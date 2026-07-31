#include "ADCS.h"

void ADCS_get_n_error(int n, adcs_err *err) {
    for (int i = 0; i < n; i++) {
        err[i].error = (uint8_t) i;
        err[i].unix_timestamp = 1000u + (uint32_t) i;
    }
}
