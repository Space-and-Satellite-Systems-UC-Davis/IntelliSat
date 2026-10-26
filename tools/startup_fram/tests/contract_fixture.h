/*
 * contract_fixture.h
 *
 * Literal copy of the frozen wire-contract vector, committed with these tests so
 * the Suite is self-contained: no build step reads ../SPEC.md or
 * ../CONTRACT-VECTOR.json, and a change to either shows up as a reviewable diff
 * here instead of silently re-baselining the tests.
 *
 * Source of truth (read-only for this lane): CONTRACT-VECTOR.json +
 * SPEC.md "Exact minimal wire contract".
 *
 *   fresh unarmed image : boot counter 0, ADCS mask 0, both slots sequence 1,
 *                        INITIAL_CHECKS, attempts 0, remaining 1800000 ms, marker 0xA5
 *   seq-max image      : boot counter 0, ADCS mask 0, both slots sequence
 *                        0xFFFFFFFF (the last legally publishable record),
 *                        MAIN_OPS, attempts 1, remaining 0 ms, armed, marker 0xA5
 */

#ifndef CONTRACT_FIXTURE_H
#define CONTRACT_FIXTURE_H

/* 48 bytes: image_hex from CONTRACT-VECTOR.json. */
#define CONTRACT_IMAGE_HEX \
    "0000000000000000535350310100000000000040771b00421c4937a5" \
    "535350310100000000000040771b00421c4937a5"

/* 20 bytes: slot_hex from CONTRACT-VECTOR.json. */
#define CONTRACT_SLOT_HEX "535350310100000000000040771b00421c4937a5"

/* 48 bytes: two identical sequence-0xFFFFFFFF records (see above). */
#define SEQ_MAX_IMAGE_HEX \
    "000000000000000053535031ffffffff06010100000000ccafcf94a5" \
    "53535031ffffffff06010100000000ccafcf94a5"

/* 20 bytes: one of those records. */
#define SEQ_MAX_SLOT_HEX "53535031ffffffff06010100000000ccafcf94a5"

#endif /* CONTRACT_FIXTURE_H */
