/*
 * Linker stubs for symbols that live outside this repo (virtual_intellisat)
 * or in source files with heavyweight dependencies (sensors.c).
 * These are never called by pure-function tests but must be defined
 * so the linker can resolve references inside the compiled source files.
 */
#include "virtual_intellisat.h"
#include "adcs_math/vector.h"

/* ── virtual_intellisat stubs ──────────────────────────────────────────────── */

vi_get_constant_status vi_get_sensor_calibration(
    vi_sensor sensor,
    float *offset,
    float *scalar,
    float *filter_constant)
{
    (void)sensor;
    *offset          = 0.0f;
    *scalar          = 1.0f;
    *filter_constant = 0.5f;
    return GET_CONSTANT_SUCCESS;
}

vi_delay_ms_status vi_delay_ms(int ms)
{
    (void)ms;
    return VI_DELAY_MS_SUCCESS;
}

/* ── sensors.c stub ────────────────────────────────────────────────────────── */

// Sentinel value for "no previous sensor reading", normally defined in sensors.c.
const vec3 undefined_vec3 = {0.0, 0.0, 0.0};
