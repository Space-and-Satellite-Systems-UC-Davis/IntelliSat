"""Contract tests for the SSP1 image codec (tools/startup_fram/startup_image.py)."""

import pathlib
import struct
import sys
import unittest

LANE_ROOT = pathlib.Path(__file__).resolve().parents[3]
sys.path.insert(0, str(LANE_ROOT / "tools" / "startup_fram"))
sys.path.insert(0, str(pathlib.Path(__file__).resolve().parent))

import contract_fixture as fixture  # noqa: E402
import startup_image as si  # noqa: E402


class GoldenVectorTest(unittest.TestCase):
    """The committed literal vector is the cross-language anchor for this lane."""

    def test_vector_is_the_fresh_unarmed_initial_image(self):
        image = si.fresh_image(0, 0)
        self.assertEqual(image.hex(), fixture.CONTRACT_IMAGE_HEX)
        self.assertEqual(len(image), si.IMAGE_SIZE)
        self.assertEqual(si.slots(image)[0].hex(), fixture.CONTRACT_SLOT_HEX)
        self.assertEqual(si.slots(image)[0], si.slots(image)[1])

    def test_vector_slot_fields(self):
        slot = si.StartupSlot.decode(bytes.fromhex(fixture.CONTRACT_SLOT_HEX))
        self.assertEqual(slot.sequence, 1)
        self.assertEqual(slot.state, si.StartupState.INITIAL_CHECKS)
        self.assertEqual(slot.attempts_used, 0)
        self.assertEqual(slot.remaining_ms, si.INHIBIT_MS)
        self.assertFalse(slot.armed)

    def test_crc_is_zlib_compatible(self):
        body = bytes.fromhex(fixture.CONTRACT_SLOT_HEX)[: si.CRC_OFFSET]
        self.assertEqual(si.crc32(body), 0x37491C42)
        self.assertEqual(si.crc32(b"123456789"), 0xCBF43926)

    def test_validate_accepts_the_vector(self):
        decoded = si.validate_image(bytes.fromhex(fixture.CONTRACT_IMAGE_HEX))
        self.assertTrue(decoded.valid)
        self.assertEqual(decoded.boot_count, 0)
        self.assertEqual(decoded.adcs_mask, 0)
        self.assertFalse(decoded.armed)
        self.assertIn(decoded.active_slot_index, (0, 1))

    def test_sequence_max_fixture_is_a_loadable_record(self):
        """0xFFFFFFFF is the last legally published sequence, not a defect."""
        raw = bytes.fromhex(fixture.SEQ_MAX_SLOT_HEX)
        slot = si.StartupSlot.decode(raw)
        self.assertEqual(slot.sequence, si.SEQ_MAX)
        self.assertEqual(slot.state, si.StartupState.MAIN_OPS)
        self.assertTrue(slot.armed)
        self.assertEqual(slot.encode().hex(), fixture.SEQ_MAX_SLOT_HEX)

        image = bytes.fromhex(fixture.SEQ_MAX_IMAGE_HEX)
        decoded = si.validate_image(image)
        self.assertTrue(decoded.valid)
        self.assertTrue(decoded.armed)
        self.assertEqual(len(image), si.IMAGE_SIZE)
        self.assertEqual(si.slots(image)[0].hex(), fixture.SEQ_MAX_SLOT_HEX)


class FreshImageTest(unittest.TestCase):
    def test_deterministic(self):
        self.assertEqual(si.fresh_image(3, 0x001FFFFF), si.fresh_image(3, 0x001FFFFF))

    def test_unarmed_by_default_and_armed_is_explicit(self):
        self.assertFalse(si.validate_image(si.fresh_image(0, 0)).armed)
        armed = si.fresh_image(0, 0, armed=True)
        self.assertTrue(si.validate_image(armed).armed)
        slot = si.slots(armed)[0]
        self.assertEqual(slot[si.FLAGS_OFFSET], si.FLAG_ARMED)

    def test_boot_count_bounds(self):
        self.assertEqual(si.boot_count(si.fresh_image(si.BOOT_COUNT_MAX, 0)),
                         si.BOOT_COUNT_MAX)
        for bad in (-1, si.BOOT_COUNT_MAX + 1, 0x1_0000_0000):
            with self.assertRaises(si.StartupImageError):
                si.fresh_image(bad, 0)

    def test_adcs_mask_bounds_and_reserved_bits(self):
        si.fresh_image(0, si.ADCS_MASK_MAX)
        for bad in (-1, si.ADCS_MASK_MAX + 1, 0x200000, 0xFFE00000, 0xFFFFFFFF):
            with self.assertRaises(si.StartupImageError):
                si.fresh_image(0, bad)

    def test_named_sensors(self):
        mask = si.adcs_mask_from_names(["Sun0", "Sun11", "Coil2", "Imu0", "Mag1", "Hdd1"])
        self.assertEqual(mask, 0b1 | (1 << 11) | (1 << 14) | (1 << 15) | (1 << 18) | (1 << 20))
        self.assertEqual(si.adcs_names(mask),
                         ["Sun0", "Sun11", "Coil2", "Imu0", "Mag1", "Hdd1"])
        self.assertEqual(si.mask_from_names_text(" Sun0 , Hdd0 "), 0b1 | (1 << 19))
        for bad in (["Sun12"], ["sun0"], [""], ["Mag0", "nope"]):
            with self.assertRaises(si.StartupImageError):
                si.adcs_mask_from_names(bad)
        for bad_text in ("", "Sun0,", ",Sun0", "Sun0,,Mag0"):
            with self.assertRaises(si.StartupImageError):
                si.mask_from_names_text(bad_text)

    def test_retry_interval_derives_from_attempt_count(self):
        for attempts in range(1, si.ATTEMPT_LIMIT):
            self.assertEqual(si.retry_remaining_ms(attempts), si.RETRY_MS)
        self.assertEqual(si.retry_remaining_ms(si.ATTEMPT_LIMIT), si.RETRY_MS_SATURATED)
        for bad in (0, si.ATTEMPT_LIMIT + 1):
            with self.assertRaises(si.StartupImageError):
                si.retry_remaining_ms(bad)


class SlotValidationTest(unittest.TestCase):
    def _image(self, slot) -> bytes:
        raw = slot.encode() if isinstance(slot, si.StartupSlot) else slot
        return struct.pack("<I", 0) + struct.pack("<I", 0) + raw + raw

    def _assert_invalid(self, image: bytes):
        decoded = si.validate_image(image)
        self.assertFalse(decoded.valid, f"expected invalid: {decoded}")
        self.assertFalse(decoded.usable, f"expected unusable: {decoded}")
        return decoded

    def test_round_trip(self):
        slot = si.StartupSlot(state=si.StartupState.WAITING_FOR_UPLINK, attempts_used=2,
                              remaining_ms=si.RETRY_MS // 3, armed=True, sequence=9)
        self.assertEqual(si.StartupSlot.decode(slot.encode()), slot)

    def test_unknown_and_transient_states_rejected(self):
        for state in (4, 5, 7, 8, 255):
            raw = bytearray(si.StartupSlot(state=si.StartupState.MAIN_OPS, attempts_used=1,
                                           remaining_ms=0, armed=True).encode())
            raw[si.STATE_OFFSET] = state
            raw[si.CRC_OFFSET:si.CRC_OFFSET + 4] = struct.pack("<I", si.crc32(bytes(raw[:si.CRC_OFFSET])))
            self._assert_invalid(self._image(bytes(raw)))

    def test_marker_tag_crc_and_flags(self):
        good = si.StartupSlot(state=si.StartupState.INITIAL_CHECKS).encode()
        mutations = {
            "uncommitted": (si.MARKER_OFFSET, si.INVALID_MARKER),
            "tag": (0, ord("X")),
            "flags bit1": (si.FLAGS_OFFSET, 0x02),
            "flags bit7": (si.FLAGS_OFFSET, 0x80),
        }
        for label, (offset, value) in mutations.items():
            raw = bytearray(good)
            raw[offset] = value
            # Re-sign so each defect is rejected on its own, not via the CRC check.
            raw[si.CRC_OFFSET:si.CRC_OFFSET + 4] = struct.pack(
                "<I", si.crc32(bytes(raw[:si.CRC_OFFSET])))
            with self.subTest(label):
                self._assert_invalid(self._image(bytes(raw)))

        raw = bytearray(good)
        raw[si.CRC_OFFSET] ^= 0x01
        self._assert_invalid(self._image(bytes(raw)))

    def test_state_specific_field_ranges(self):
        cases = [
            dict(state=si.StartupState.INITIAL_CHECKS, attempts_used=1),
            dict(state=si.StartupState.INITIAL_CHECKS, remaining_ms=si.INHIBIT_MS + 1),
            dict(state=si.StartupState.WAIT_POWER_ON, attempts_used=1, remaining_ms=1,
                 armed=True),
            dict(state=si.StartupState.WAITING_FOR_UPLINK, attempts_used=0,
                 remaining_ms=si.RETRY_MS, armed=True),
            dict(state=si.StartupState.WAITING_FOR_UPLINK, attempts_used=1,
                 remaining_ms=si.RETRY_MS + 1, armed=True),
            dict(state=si.StartupState.SAT_TO_GROUND_1, attempts_used=1, remaining_ms=1,
                 armed=True),
            dict(state=si.StartupState.MAIN_OPS, attempts_used=1, remaining_ms=1, armed=True),
            dict(state=si.StartupState.MAIN_OPS, attempts_used=1, remaining_ms=0),
            dict(state=si.StartupState.WAIT_POWER_ON, attempts_used=1, remaining_ms=0),
        ]
        for case in cases:
            with self.subTest(case=case):
                # Built directly: encode() refuses invalid field combinations.
                slot = bytearray(si.StartupSlot(state=si.StartupState.INITIAL_CHECKS).encode())
                slot[si.STATE_OFFSET] = case["state"]
                slot[si.ATTEMPTS_OFFSET] = case.get("attempts_used", 0)
                slot[si.FLAGS_OFFSET] = si.FLAG_ARMED if case.get("armed") else 0
                slot[si.REMAINING_OFFSET:si.CRC_OFFSET] = struct.pack(
                    "<I", case.get("remaining_ms", si.INHIBIT_MS))
                slot[si.CRC_OFFSET:si.CRC_OFFSET + 4] = struct.pack(
                    "<I", si.crc32(bytes(slot[:si.CRC_OFFSET])))
                self._assert_invalid(self._image(bytes(slot)))

    def test_accepted_combinations(self):
        accepted = [
            dict(state=si.StartupState.INITIAL_CHECKS),
            dict(state=si.StartupState.INITIAL_CHECKS, armed=True),
            dict(state=si.StartupState.WAIT_POWER_ON, attempts_used=1, remaining_ms=0,
                 armed=True),
            dict(state=si.StartupState.WAITING_FOR_UPLINK, attempts_used=1,
                 remaining_ms=si.RETRY_MS, armed=True),
            dict(state=si.StartupState.WAITING_FOR_UPLINK, attempts_used=7,
                 remaining_ms=si.RETRY_MS_SATURATED, armed=True),
            dict(state=si.StartupState.SAT_TO_GROUND_1, attempts_used=1, remaining_ms=0,
                 armed=True),
            dict(state=si.StartupState.MAIN_OPS, attempts_used=7, remaining_ms=0, armed=True),
        ]
        for case in accepted:
            with self.subTest(case=case):
                self.assertTrue(si.validate_image(self._image(si.StartupSlot(**case))).valid)

    def test_sequence_zero_invalid_and_max_valid(self):
        slot = bytearray(si.StartupSlot().encode())
        slot[si.SEQ_OFFSET:si.STATE_OFFSET] = struct.pack("<I", 0)
        slot[si.CRC_OFFSET:si.CRC_OFFSET + 4] = struct.pack(
            "<I", si.crc32(bytes(slot[:si.CRC_OFFSET])))
        self._assert_invalid(self._image(bytes(slot)))

        slot = bytearray(si.StartupSlot().encode())
        slot[si.SEQ_OFFSET:si.STATE_OFFSET] = struct.pack("<I", si.SEQ_MAX)
        slot[si.CRC_OFFSET:si.CRC_OFFSET + 4] = struct.pack(
            "<I", si.crc32(bytes(slot[:si.CRC_OFFSET])))
        self.assertTrue(si.validate_image(self._image(bytes(slot))).valid)


class ImageValidationTest(unittest.TestCase):
    def test_length_and_accessors(self):
        image = si.fresh_image(7, 0x00000003)
        self.assertEqual(si.boot_count(image), 7)
        self.assertEqual(si.image_adcs_mask(image), 0x00000003)
        self.assertEqual(si.adcs_names(0x00000003), ["Sun0", "Sun1"])
        for bad in (b"", image[:-1], image + b"\x00", b"\x00" * 64):
            with self.assertRaises(si.StartupImageError):
                si.validate_image(bad)

    def test_both_slots_invalid_leaves_nothing_usable(self):
        image = bytearray(si.fresh_image(0, 0))
        image[si.SLOT_A_ADDR + si.MARKER_OFFSET] = si.INVALID_MARKER
        image[si.SLOT_B_ADDR + si.MARKER_OFFSET] = si.INVALID_MARKER
        decoded = si.validate_image(bytes(image))
        self.assertFalse(decoded.valid)
        self.assertFalse(decoded.usable)
        self.assertIsNone(decoded.active_slot_index)
        self.assertTrue(any("no usable Startup slot" in problem
                            for problem in decoded.record_errors))

    def test_one_valid_slot_is_enough_for_the_loader_but_not_a_complete_image(self):
        image = bytearray(si.fresh_image(0, 0))
        image[si.SLOT_B_ADDR + si.MARKER_OFFSET] = si.INVALID_MARKER
        decoded = si.validate_image(bytes(image))
        self.assertFalse(decoded.valid)
        self.assertTrue(decoded.usable)
        self.assertEqual(decoded.active_slot_index, 0)
        self.assertIn("commit marker", "; ".join(decoded.slot_errors[1]))

    def test_equal_sequences_must_be_identical(self):
        newer = si.StartupSlot().encode()
        older = si.StartupSlot(armed=True).encode()
        image = struct.pack("<I", 0) + struct.pack("<I", 0) + newer + older
        decoded = si.validate_image(image)
        self.assertFalse(decoded.valid)
        self.assertIn("equal sequences must be identical", "; ".join(decoded.record_errors))

    def test_higher_sequence_is_the_active_slot(self):
        first = si.StartupSlot(state=si.StartupState.INITIAL_CHECKS).encode()
        second = si.StartupSlot(state=si.StartupState.MAIN_OPS, attempts_used=1,
                                remaining_ms=0, armed=True, sequence=2).encode()
        image = struct.pack("<I", 0) + struct.pack("<I", 0) + first + second
        decoded = si.validate_image(image)
        self.assertEqual(decoded.active_slot_index, 1)
        self.assertTrue(decoded.armed)

    def test_reserved_adcs_bits_and_saturated_boot_counter(self):
        image = bytearray(si.fresh_image(0, 0))
        image[si.ADCS_MASK_ADDR:si.SLOT_A_ADDR] = struct.pack("<I", 0x00200000)
        decoded = si.validate_image(bytes(image))
        self.assertFalse(decoded.valid)
        self.assertTrue(any("reserved bits 21..31" in problem
                            for problem in decoded.record_errors))

        image = bytearray(si.fresh_image(0, 0))
        image[si.BOOT_COUNTER_ADDR:si.ADCS_MASK_ADDR] = struct.pack("<I", 0xFFFFFFFF)
        decoded = si.validate_image(bytes(image))
        self.assertFalse(decoded.valid)
        self.assertTrue(any("boot counter" in problem for problem in decoded.record_errors))


if __name__ == "__main__":
    unittest.main()
