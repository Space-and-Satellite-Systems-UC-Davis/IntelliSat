"""Byte-exact codec for the 48-byte Startup persistence image (format tag ``SSP1``).

This module is the single host-side spelling of the frozen wire contract:

===================  ======  ===============================================
byte address         bytes   meaning
===================  ======  ===============================================
0..3                 4       uint32 LE boot counter
4..7                 4       uint32 LE ADCS sensor enable mask
8..27                20      Startup slot A
28..47               20      Startup slot B
===================  ======  ===============================================

Slot layout (little endian, offsets relative to the slot start):

====  ====  ==================================================================
off   size  meaning
====  ====  ==================================================================
0     4     ASCII ``SSP1``
4     4     uint32 save sequence, initial 1, incremented per publication
8     1     resume state, existing ``MCUState`` numeric value
9     1     attempts used, 0..7
10    1     flags, bit0 = armed, every other bit zero
11    4     uint32 remaining wait in milliseconds
15    4     CRC32 of slot bytes 0..14 (reflected 0xEDB88320, init/xorout 0xFFFFFFFF)
19    1     commit marker 0xA5, 0x00 while uncommitted
====  ====  ==================================================================

Bytes 48..32767 of the FRAM device are never part of this image; the provisioning
tools restrict themselves to 0..47.
"""

from __future__ import annotations

import dataclasses
import enum
import struct
import zlib

BOOT_COUNTER_ADDR = 0
ADCS_MASK_ADDR = 4
SLOT_A_ADDR = 8
SLOT_B_ADDR = 28
SLOT_SIZE = 20
IMAGE_SIZE = 48
SLOT_ADDRS = (SLOT_A_ADDR, SLOT_B_ADDR)

#: Highest byte address any provisioning write may touch, inclusive.
LAST_IMAGE_ADDR = IMAGE_SIZE - 1

TAG = b"SSP1"
COMMIT_MARKER = 0xA5
INVALID_MARKER = 0x00

SEQ_OFFSET = 4
STATE_OFFSET = 8
ATTEMPTS_OFFSET = 9
FLAGS_OFFSET = 10
REMAINING_OFFSET = 11
CRC_OFFSET = 15
MARKER_OFFSET = 19

SEQ_INITIAL = 1
#: Highest sequence a record may legally carry. The firmware loader accepts a
#: record at this sequence; only the *next* save reports exhaustion (no wrap).
SEQ_MAX = 0xFFFFFFFF
BOOT_COUNT_MAX = 0xFFFFFFFE  # the firmware increments it, so UINT32_MAX is unusable

FLAG_ARMED = 0x01
FLAG_MASK = FLAG_ARMED

#: Reserved ADCS mask bits 21..31, which must stay zero.
ADCS_RESERVED_MASK = 0xFFE00000
ADCS_MASK_MAX = 0x001FFFFF

INHIBIT_MS = 1_800_000
RETRY_MS = 172_800_000
RETRY_MS_SATURATED = 604_800_000
ATTEMPT_LIMIT = 7

#: ``SensorOffset`` order from ``Src/inc/sensor_offsets.h``; bit == offset.
ADCS_BITS = (
    ("Sun0", 0),
    ("Sun1", 1),
    ("Sun2", 2),
    ("Sun3", 3),
    ("Sun4", 4),
    ("Sun5", 5),
    ("Sun6", 6),
    ("Sun7", 7),
    ("Sun8", 8),
    ("Sun9", 9),
    ("Sun10", 10),
    ("Sun11", 11),
    ("Coil0", 12),
    ("Coil1", 13),
    ("Coil2", 14),
    ("Imu0", 15),
    ("Imu1", 16),
    ("Mag0", 17),
    ("Mag1", 18),
    ("Hdd0", 19),
    ("Hdd1", 20),
)
ADCS_NAME_TO_BIT = dict(ADCS_BITS)


class StartupImageError(ValueError):
    """The bytes or arguments do not satisfy the wire contract."""


class StartupState(enum.IntEnum):
    """Existing ``MCUState`` numeric values; durable resume points reuse them."""

    INITIAL_CHECKS = 0
    WAIT_POWER_ON = 1
    WAITING_FOR_UPLINK = 2
    SAT_TO_GROUND_1 = 3
    GROUND_TO_SAT_1 = 4
    SAT_TO_GROUND_2 = 5
    MAIN_OPS = 6
    DIAGNOSTICS_MODE = 7


#: States that may appear in FRAM; the rest stay transient in RAM.
PERSISTENT_STATES = frozenset(
    {
        StartupState.INITIAL_CHECKS,
        StartupState.WAIT_POWER_ON,
        StartupState.WAITING_FOR_UPLINK,
        StartupState.SAT_TO_GROUND_1,
        StartupState.MAIN_OPS,
    }
)

_TRANSIENT_ONLY = (StartupState.GROUND_TO_SAT_1, StartupState.SAT_TO_GROUND_2,
                   StartupState.DIAGNOSTICS_MODE)


def state_name(value: int) -> str:
    try:
        return StartupState(value).name
    except ValueError:
        return f"UNKNOWN({value})"


def crc32(payload: bytes) -> int:
    """CRC32 matching zlib and the reflected 0xEDB88320 contract."""
    return zlib.crc32(payload) & 0xFFFFFFFF


def retry_remaining_ms(attempts_used: int) -> int:
    """Retry wait saved *before* a burn, derived from the consumed attempt count."""
    if not 1 <= attempts_used <= ATTEMPT_LIMIT:
        raise StartupImageError(
            f"attempts_used={attempts_used} is outside 1..{ATTEMPT_LIMIT}"
        )
    return RETRY_MS if attempts_used < ATTEMPT_LIMIT else RETRY_MS_SATURATED


def adcs_mask_from_names(names) -> int:
    """Build the enable mask from ``SensorOffset`` names, rejecting the unknown."""
    mask = 0
    for name in names:
        try:
            bit = ADCS_NAME_TO_BIT[name]
        except KeyError:
            raise StartupImageError(
                f"unknown ADCS sensor {name!r}; valid names: "
                + ", ".join(ADCS_NAME_TO_BIT)
            ) from None
        mask |= 1 << bit
    return mask


def adcs_names(mask: int) -> list[str]:
    """Enabled ``SensorOffset`` names for a mask; empty when nothing is enabled."""
    return [name for name, bit in ADCS_BITS if mask & (1 << bit)]


def validate_adcs_mask(mask: int) -> None:
    if not 0 <= mask <= 0xFFFFFFFF:
        raise StartupImageError(f"ADCS mask 0x{mask:X} is not a uint32")
    if mask & ADCS_RESERVED_MASK:
        raise StartupImageError(
            f"ADCS mask 0x{mask:08X} sets reserved bits 21..31 "
            f"(0x{mask & ADCS_RESERVED_MASK:08X}); "
            f"allowed range is 0x00000000..0x{ADCS_MASK_MAX:08X}"
        )


def mask_from_names_text(text: str) -> int:
    """``"Sun0,Mag1"`` -> mask, for the CLI; empty entries are rejected."""
    names = [part.strip() for part in text.split(",")]
    if not names or any(not name for name in names):
        raise StartupImageError(f"malformed ADCS sensor list {text!r}")
    return adcs_mask_from_names(names)


def slot_field_problems(
    sequence: int,
    state: int,
    attempts_used: int,
    armed: bool,
    remaining_ms: int,
) -> list[str]:
    """Field-range rules for one slot; empty list means the fields are acceptable."""
    problems: list[str] = []
    if not SEQ_INITIAL <= sequence <= SEQ_MAX:
        problems.append(
            f"sequence {sequence} outside {SEQ_INITIAL}..{SEQ_MAX}"
            " (0 is never valid; 0xFFFFFFFF is the last publishable record)"
        )
    if state not in {int(s) for s in PERSISTENT_STATES}:
        if state in {int(s) for s in _TRANSIENT_ONLY}:
            problems.append(
                f"state {state} ({state_name(state)}) is transient and must not be persisted"
            )
        else:
            problems.append(f"state {state} is not an MCUState value")
    if not 0 <= attempts_used <= ATTEMPT_LIMIT:
        problems.append(f"attempts_used {attempts_used} outside 0..{ATTEMPT_LIMIT}")
    if not 0 <= remaining_ms <= 0xFFFFFFFF:
        problems.append(f"remaining_ms {remaining_ms} is not a uint32")

    if state == StartupState.INITIAL_CHECKS:
        if attempts_used != 0:
            problems.append(
                f"INITIAL_CHECKS requires attempts_used 0, got {attempts_used}"
            )
        if remaining_ms > INHIBIT_MS:
            problems.append(
                f"INITIAL_CHECKS remaining_ms {remaining_ms} exceeds {INHIBIT_MS}"
            )
    elif state == StartupState.WAIT_POWER_ON:
        if remaining_ms != 0:
            problems.append(
                f"WAIT_POWER_ON requires remaining_ms 0, got {remaining_ms}"
            )
    elif state == StartupState.WAITING_FOR_UPLINK:
        if not 1 <= attempts_used <= ATTEMPT_LIMIT:
            problems.append(
                "WAITING_FOR_UPLINK requires attempts_used 1.."
                f"{ATTEMPT_LIMIT}, got {attempts_used}"
            )
        elif remaining_ms > retry_remaining_ms(attempts_used):
            problems.append(
                f"WAITING_FOR_UPLINK remaining_ms {remaining_ms} exceeds "
                f"{retry_remaining_ms(attempts_used)} for {attempts_used} attempts"
            )
    elif state in (StartupState.SAT_TO_GROUND_1, StartupState.MAIN_OPS):
        if remaining_ms != 0:
            problems.append(
                f"{state_name(state)} requires remaining_ms 0, got {remaining_ms}"
            )

    if state != StartupState.INITIAL_CHECKS and not armed:
        problems.append(
            f"{state_name(state)} requires the armed flag; an unarmed record can"
            " only sit in INITIAL_CHECKS"
        )
    return problems


@dataclasses.dataclass(frozen=True)
class StartupSlot:
    """One 20-byte startup slot. ``armed`` carries flags bit0 only."""

    state: int = StartupState.INITIAL_CHECKS
    attempts_used: int = 0
    remaining_ms: int = INHIBIT_MS
    armed: bool = False
    sequence: int = SEQ_INITIAL

    @property
    def flags(self) -> int:
        return FLAG_ARMED if self.armed else 0

    def encode(self) -> bytes:
        """The committed 20-byte record, including the 0xA5 marker."""
        problems = slot_field_problems(
            self.sequence, int(self.state), self.attempts_used, self.armed,
            self.remaining_ms,
        )
        if problems:
            raise StartupImageError("; ".join(problems))
        body = (
            TAG
            + struct.pack("<I", self.sequence)
            + bytes([int(self.state), self.attempts_used, self.flags])
            + struct.pack("<I", self.remaining_ms)
        )
        return body + struct.pack("<I", crc32(body)) + bytes([COMMIT_MARKER])

    @classmethod
    def decode(cls, raw: bytes) -> "StartupSlot":
        """Validate one 20-byte slot and return it; raises on any defect."""
        problems = slot_problems(raw)
        if problems:
            raise StartupImageError("; ".join(problems))
        body = raw[:CRC_OFFSET]
        return cls(
            state=raw[STATE_OFFSET],
            attempts_used=raw[ATTEMPTS_OFFSET],
            remaining_ms=struct.unpack("<I", body[REMAINING_OFFSET:CRC_OFFSET])[0],
            armed=bool(raw[FLAGS_OFFSET] & FLAG_ARMED),
            sequence=struct.unpack("<I", body[SEQ_OFFSET:STATE_OFFSET])[0],
        )


def slot_problems(raw: bytes) -> list[str]:
    """Every defect in one raw 20-byte slot: marker, tag, CRC, flags, field ranges."""
    if len(raw) != SLOT_SIZE:
        return [f"slot is {len(raw)} bytes, expected {SLOT_SIZE}"]
    problems: list[str] = []
    if raw[MARKER_OFFSET] != COMMIT_MARKER:
        problems.append(
            f"commit marker 0x{raw[MARKER_OFFSET]:02X} is not 0x{COMMIT_MARKER:02X}"
            " (uncommitted or invalid record)"
        )
    if raw[:4] != TAG:
        problems.append(f"tag {raw[:4].hex()} is not {TAG.hex()} ('SSP1')")
    stored_crc = struct.unpack("<I", raw[CRC_OFFSET:MARKER_OFFSET])[0]
    computed_crc = crc32(raw[:CRC_OFFSET])
    if stored_crc != computed_crc:
        problems.append(
            f"CRC32 0x{stored_crc:08X} does not match 0x{computed_crc:08X}"
        )
    flags = raw[FLAGS_OFFSET]
    if flags & ~FLAG_MASK:
        problems.append(
            f"flags 0x{flags:02X} sets bits outside bit0 (armed)"
        )
    sequence = struct.unpack("<I", raw[SEQ_OFFSET:STATE_OFFSET])[0]
    remaining_ms = struct.unpack("<I", raw[REMAINING_OFFSET:CRC_OFFSET])[0]
    problems.extend(
        slot_field_problems(
            sequence,
            raw[STATE_OFFSET],
            raw[ATTEMPTS_OFFSET],
            bool(flags & FLAG_ARMED),
            remaining_ms,
        )
    )
    return problems


def fresh_image(
    boot_count: int,
    adcs_mask: int,
    *,
    armed: bool = False,
) -> bytes:
    """The 48-byte factory image: identical sequence-1 INITIAL_CHECKS slots.

    ``armed`` is the only caller-selectable policy bit and defaults to False, so
    no call path arms a vehicle implicitly.
    """
    if not 0 <= boot_count <= BOOT_COUNT_MAX:
        raise StartupImageError(
            f"boot_count {boot_count} outside 0..{BOOT_COUNT_MAX} (UINT32_MAX is"
            " reserved for a saturated counter)"
        )
    validate_adcs_mask(adcs_mask)
    slot = StartupSlot(state=StartupState.INITIAL_CHECKS, armed=armed).encode()
    return (
        struct.pack("<I", boot_count)
        + struct.pack("<I", adcs_mask)
        + slot
        + slot
    )


def boot_count(image: bytes) -> int:
    return struct.unpack("<I", image[BOOT_COUNTER_ADDR:ADCS_MASK_ADDR])[0]


def image_adcs_mask(image: bytes) -> int:
    return struct.unpack("<I", image[ADCS_MASK_ADDR:SLOT_A_ADDR])[0]


def slots(image: bytes) -> tuple[bytes, bytes]:
    return (
        image[SLOT_A_ADDR:SLOT_A_ADDR + SLOT_SIZE],
        image[SLOT_B_ADDR:SLOT_B_ADDR + SLOT_SIZE],
    )


@dataclasses.dataclass(frozen=True)
class DecodedImage:
    """Decoded view of a 48-byte image plus the aggregate record problems."""

    boot_count: int
    adcs_mask: int
    raw_slots: tuple[bytes, bytes]
    slot_errors: tuple[list[str], list[str]]
    record_errors: list[str]

    @property
    def valid(self) -> bool:
        """Complete image: no record-level problem and both slots readable."""
        return not self.record_errors and not self.slot_errors[0] and not self.slot_errors[1]

    @property
    def usable(self) -> bool:
        """True when the firmware loader would find at least one committed slot."""
        return not self.record_errors and (not self.slot_errors[0] or not self.slot_errors[1])

    @property
    def adcs_enabled(self) -> list[str]:
        return adcs_names(self.adcs_mask)

    @property
    def armed(self) -> bool:
        return any(
            not errors and StartupSlot.decode(raw).armed
            for raw, errors in zip(self.raw_slots, self.slot_errors)
        )

    @property
    def active_slot_index(self) -> int | None:
        """Index of the slot a loader would use: highest valid committed sequence."""
        candidates = [
            (StartupSlot.decode(raw).sequence, index)
            for index, (raw, errors) in enumerate(zip(self.raw_slots, self.slot_errors))
            if not errors
        ]
        if not candidates:
            return None
        return max(candidates)[1]


def validate_image(image: bytes) -> DecodedImage:
    """Decode a 48-byte image and report every contract violation.

    Checks the boot counter, the ADCS reserved bits and both slots, plus the
    loader rule that equal sequences must be byte-identical. Per-slot defects go
    into :attr:`DecodedImage.slot_errors`; image-wide defects go into
    :attr:`DecodedImage.record_errors`. Only a structurally wrong length raises
    :class:`StartupImageError`, so an inspector can explain a broken device.
    """
    if len(image) != IMAGE_SIZE:
        raise StartupImageError(
            f"image is {len(image)} bytes, expected {IMAGE_SIZE} (addresses 0..47)"
        )
    record_errors: list[str] = []
    count = boot_count(image)
    if count > BOOT_COUNT_MAX:
        record_errors.append(
            f"boot counter {count} is 0x{count:08X}; the firmware cannot increment a"
            " saturated counter, use 0..0xFFFFFFFE"
        )
    mask = image_adcs_mask(image)
    try:
        validate_adcs_mask(mask)
    except StartupImageError as exc:
        record_errors.append(str(exc))

    raw_slots = slots(image)
    slot_errors = [slot_problems(raw) for raw in raw_slots]
    if not slot_errors[0] and not slot_errors[1]:
        seq_a = StartupSlot.decode(raw_slots[0]).sequence
        seq_b = StartupSlot.decode(raw_slots[1]).sequence
        if seq_a == seq_b and raw_slots[0] != raw_slots[1]:
            record_errors.append(
                f"both slots claim sequence {seq_a} but differ byte-wise:"
                " equal sequences must be identical records"
            )
    if slot_errors[0] and slot_errors[1]:
        record_errors.append(
            "no usable Startup slot: both records are invalid, the firmware would"
            " treat this device as an I/O error"
        )

    return DecodedImage(
        boot_count=count,
        adcs_mask=mask,
        raw_slots=raw_slots,
        slot_errors=(slot_errors[0], slot_errors[1]),
        record_errors=record_errors,
    )
