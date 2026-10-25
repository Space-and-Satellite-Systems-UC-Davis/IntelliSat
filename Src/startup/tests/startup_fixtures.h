#pragma once
/* Independent wire fixtures, not persistence encoder calls. Addresses from v3 contract. */
static void fixture_le(uint8_t *p, uint64_t value, unsigned n) {
    for (unsigned i = 0; i < n; ++i) p[i] = (uint8_t)(value >> (i * 8));
}
static uint32_t fixture_crc(const uint8_t *p, size_t n) {
    uint32_t c = UINT32_MAX;
    while (n--) {
        c ^= *p++;
        for (unsigned b = 0; b < 8; ++b) c = (c >> 1) ^ ((c & 1) ? 0xedb88320U : 0);
    }
    return c ^ UINT32_MAX;
}
static void fixture_seed(uint8_t *fram, StPhase phase, unsigned count,
                         uint64_t elapsed, uint64_t reference, bool authorized) {
    uint8_t c[96] = {0}, p[64] = {0};
    uint64_t reservation = count ? 2 * count + 1 : 0;
    uint64_t seq = phase == ST_PHASE_PREPARED ? 0 :
                   phase == ST_PHASE_INHIBIT ? 1 :
                   phase == ST_PHASE_ELIGIBLE ? 2 :
                   phase == ST_PHASE_RESERVED ? reservation :
                   phase == ST_PHASE_RETRY_WAIT ? reservation + 1 :
                   phase == ST_PHASE_RETIRED ? (count ? reservation + 2 : 3) :
                                              (count ? reservation + 3 : 4);
    bool waiting = phase == ST_PHASE_INHIBIT || phase == ST_PHASE_RETRY_WAIT;
    uint64_t duration = phase == ST_PHASE_INHIBIT ? 1800000 :
                        count < 7 ? 172800000 : 604800000;
    memcpy(c, "STC3", 4);
    fixture_le(c + 4, 3, 2); fixture_le(c + 6, 96, 2);
    fixture_le(c + 8, 1, 8); fixture_le(c + 16, seq, 8);
    fixture_le(c + 24, reservation, 8);
    if (waiting) {
        fixture_le(c + 32, seq, 8); fixture_le(c + 40, duration, 8);
        fixture_le(c + 56, reference, 8); fixture_le(c + 64, 1, 4);
        c[75] = 1;
    }
    fixture_le(c + 68, phase >= ST_PHASE_ELIGIBLE ? 1 : 0, 4);
    c[72] = (uint8_t)phase; c[73] = (uint8_t)count; c[74] = authorized;
    fixture_le(c + 92, fixture_crc(c + 4, 88), 4);
    memset(fram + 8, 0, 320);
    memcpy(fram + 8, c, 96); memcpy(fram + 104, c, 96);
    if (waiting && elapsed) {
        memcpy(p, "STP3", 4);
        fixture_le(p + 4, 3, 2); fixture_le(p + 6, 64, 2);
        fixture_le(p + 8, 1, 8); fixture_le(p + 16, seq, 8);
        fixture_le(p + 24, 1, 8); fixture_le(p + 32, elapsed, 8);
        fixture_le(p + 40, reference, 8); fixture_le(p + 48, 1, 4); p[52] = 1;
        fixture_le(p + 60, fixture_crc(p + 4, 56), 4);
        memcpy(fram + 200, p, 64);
    }
}
