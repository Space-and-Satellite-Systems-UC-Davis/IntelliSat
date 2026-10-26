"""Literal copy of the frozen wire-contract vector (self-contained test fixture).

Committed with the tests so no suite reads ``../SPEC.md`` or
``../CONTRACT-VECTOR.json``. If the contract ever changes, this file is the
reviewable diff point instead of a silent re-baseline.

Source of truth (read-only for this lane): ``CONTRACT-VECTOR.json`` and the
"Exact minimal wire contract" section of ``SPEC.md``.
"""

#: 48 bytes: ``image_hex`` of the independent vector (fresh unarmed image).
CONTRACT_IMAGE_HEX = (
    "0000000000000000535350310100000000000040771b00421c4937a5"
    "535350310100000000000040771b00421c4937a5"
)

#: 20 bytes: ``slot_hex`` of the independent vector.
CONTRACT_SLOT_HEX = "535350310100000000000040771b00421c4937a5"

#: 48 bytes: two identical sequence-0xFFFFFFFF records - the last legally
#: publishable sequence, which the loader accepts (the *next* save is what
#: reports exhaustion and never wraps).
SEQ_MAX_IMAGE_HEX = (
    "000000000000000053535031ffffffff06010100000000ccafcf94a5"
    "53535031ffffffff06010100000000ccafcf94a5"
)

#: 20 bytes: one of those records.
SEQ_MAX_SLOT_HEX = "53535031ffffffff06010100000000ccafcf94a5"
