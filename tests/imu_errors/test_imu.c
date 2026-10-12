#include <assert.h>
#include <math.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "IMU/ASM330LHH.h"
#include "adcs_math/sensors.h"
#include "unit_tests/IMU_Tests/IMU_Tests.h"

extern float imu_acelFullScaleIMU0, imu_acelFullScaleIMU1;
extern float imu_gyroFullScaleIMU0, imu_gyroFullScaleIMU1;
bool imu_hasExpectedValuesAccel(void);
bool imu_hasExpectedValuesGyro(void);

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "%s:%d: %s\n", __func__, __LINE__, #condition); \
    return false; \
} } while (0)

static uint8_t registers[256];
static enum IMU_SELECT expected_imu;
static int starts, stops, position, address, byte_count, after_failure;
static int fail_address, fail_byte, failures_left, fail_on_start;
static int calibration_calls, assertion_failures;
static bool active, failed, write_failed_byte;
static char printed[1024];

static void reset(enum IMU_SELECT selected) {
    memset(registers, 0, sizeof(registers));
    registers[0x0f] = 0x6b;
    expected_imu = selected;
    set_IMU(selected);
    imu_acelFullScaleIMU0 = 0.000061f;
    imu_acelFullScaleIMU1 = 0.000122f;
    imu_gyroFullScaleIMU0 = 0.00875f;
    imu_gyroFullScaleIMU1 = 0.035f;
    starts = stops = byte_count = after_failure = 0;
    calibration_calls = assertion_failures = 0;
    active = failed = write_failed_byte = false;
    fail_address = fail_byte = -1;
    failures_left = -1;
    fail_on_start = 0;
    printed[0] = '\0';
}

static void raw_value(uint8_t reg, int16_t value) {
    registers[reg] = (uint16_t)value & 0xff;
    registers[reg + 1] = (uint16_t)value >> 8;
}

void spi_startCommunication(GPIO_TypeDef *port, int pin) {
    assert(!active && port == GPIOB);
    assert(pin == (expected_imu == IMU0 ? 6 : 12));
    active = true;
    failed = false;
    position = 0;
    starts++;
}

void spi_stopCommunication(GPIO_TypeDef *port, int pin) {
    assert(active && port == GPIOB);
    assert(pin == (expected_imu == IMU0 ? 6 : 12));
    active = false;
    stops++;
}

// Inject failure at a byte within the CS transaction. A failed transfer can
// already have written part (or all) of its receive buffer.
bool spi_transmitReceive(SPI_TypeDef *spi, uint8_t *tx, uint8_t *rx,
                         uint16_t size, bool dma) {
    assert(active && !dma);
    assert(spi == (expected_imu == IMU0 ? SPI3 : SPI2));
    if (failed) after_failure++;
    for (unsigned i = 0; i < size; i++) {
        if (position == 0) {
            assert(tx && (tx[i] & 0x80));
            address = tx[i] & 0x7f;
        }
        bool inject = position == fail_byte && failures_left != 0
            && (fail_address < 0 || fail_address == address)
            && (!fail_on_start || starts == fail_on_start);
        if (rx && (!inject || write_failed_byte))
            rx[i] = registers[address + position - 1];
        position++;
        byte_count++;
        if (inject) {
            failed = true;
            if (failures_left > 0) failures_left--;
            return false;
        }
    }
    return true;
}

void nop(long long count) { (void)count; }
void delay_ms(uint64_t ms) { (void)ms; }

calibration_status calibrateVec3(vi_sensor sensor, vec3 prev, vec3 *curr) {
    (void)sensor;
    (void)prev;
    assert(isfinite(curr->x) && isfinite(curr->y) && isfinite(curr->z));
    calibration_calls++;
    return CALIBRATION_SUCESS;
}

bool assert_equal_int(int actual, int expected) {
    if (actual != expected) assertion_failures++;
    return actual == expected;
}
bool assert_greater_than_equal_float(float a, float b, float margin) {
    bool ok = a >= b - margin;
    if (!ok) assertion_failures++;
    return ok;
}
bool assert_less_than_equal_float(float a, float b, float margin) {
    bool ok = a <= b + margin;
    if (!ok) assertion_failures++;
    return ok;
}
int printMsg(const char *format, ...) {
    va_list args;
    va_start(args, format);
    size_t used = strlen(printed);
    int n = vsnprintf(printed + used, sizeof(printed) - used, format, args);
    va_end(args);
    return n;
}

static const struct {
    float (*read)(void);
    uint8_t reg;
} readings[] = {
    {imu_readAcel_X, 0x28}, {imu_readAcel_Y, 0x2a}, {imu_readAcel_Z, 0x2c},
    {imu_readGyro_X, 0x22}, {imu_readGyro_Y, 0x24}, {imu_readGyro_Z, 0x26},
    {imu_readTemp, 0x20}
};

static bool read_failures(void) {
    for (int imu = IMU0; imu <= IMU1; imu++)
        for (unsigned r = 0; r < 7; r++)
            for (int byte = 0; byte < 3; byte++)
                for (int written = 0; written < 2; written++) {
                    reset(imu);
                    raw_value(readings[r].reg, 0x1234);
                    fail_byte = byte;
                    write_failed_byte = written;
                    CHECK(isnan(readings[r].read()));
                    CHECK(starts == 1 && stops == 1 && !active);
                    CHECK(byte_count == byte + 1 && after_failure == 0);
                }
    return true;
}

static bool valid_readings(void) {
    const int16_t values[] = {0, 1, -1, 0x1234, INT16_MIN, INT16_MAX};
    for (int imu = IMU0; imu <= IMU1; imu++)
        for (unsigned r = 0; r < 7; r++)
            for (unsigned v = 0; v < sizeof(values) / sizeof(values[0]); v++) {
                reset(imu);
                raw_value(readings[r].reg, values[v]);
                float expected = r == 6 ? values[v] / 256.0f + 25.0f
                    : values[v] * (r < 3
                        ? (imu == IMU0 ? 0.000061f : 0.000122f)
                        : (imu == IMU0 ? 0.00875f : 0.035f));
                CHECK(readings[r].read() == expected);
                CHECK(starts == 1 && stops == 1 && byte_count == 3);
            }
    return true;
}

static bool identity(void) {
    for (int imu = IMU0; imu <= IMU1; imu++) {
        reset(imu);
        CHECK(imu_isCommunicationWorking());
        registers[0x0f] = 0;
        CHECK(!imu_isCommunicationWorking());
        for (int byte = 0; byte < 2; byte++) {
            reset(imu);
            fail_byte = byte;
            write_failed_byte = true; // Even a received 0x6b is invalid on failure.
            CHECK(!imu_isCommunicationWorking());
            CHECK(starts == 1 && stops == 1 && after_failure == 0);
        }
    }
    return true;
}

static vi_sensor sensor(int imu) {
    return (vi_sensor){.component = IMU, .choice = imu == IMU0 ? ONE : TWO, .axis = NA};
}

static bool vector_failures(void) {
    for (int imu = IMU0; imu <= IMU1; imu++)
        for (int axis = 0; axis < 3; axis++)
            for (int byte = 0; byte < 3; byte++) {
                reset(imu);
                set_IMU(imu == IMU0 ? IMU1 : IMU0); // The ADCS selection must win.
                fail_address = 0x22 + 2 * axis;
                fail_byte = byte;
                double x = 11, y = 22, z = 33; // Previously valid sample.
                CHECK(vi_get_angvel(sensor(imu), &x, &y, &z) == GET_ANGVEL_FAILURE);
                CHECK(isnan(x) && isnan(y) && isnan(z));
                CHECK(starts == axis + 1 && stops == starts && !active);
            }
    return true;
}

static bool vector_success(void) {
    for (int imu = IMU0; imu <= IMU1; imu++) {
        reset(imu);
        raw_value(0x22, 100);
        raw_value(0x24, -200);
        double x, y, z;
        float scale = imu == IMU0 ? 0.00875f : 0.035f;
        CHECK(vi_get_angvel(sensor(imu), &x, &y, &z) == GET_ANGVEL_SUCCESS);
        CHECK(x == 100 * scale && y == -200 * scale && z == 0);
    }
    return true;
}

static bool exhausted_retries(void) {
    for (int imu = IMU0; imu <= IMU1; imu++)
        for (int axis = 0; axis < 3; axis++) {
            reset(imu);
            fail_address = 0x22 + 2 * axis;
            fail_byte = 2;
            vec3 value = {11, 22, 33};
            CHECK(getIMU(sensor(imu), undefined_vec3, &value) == GET_IMU_FAILURE);
            CHECK(calibration_calls == 0);
            CHECK(starts == 3 * (axis + 1) && stops == starts);
            CHECK(isnan(value.x) && isnan(value.y) && isnan(value.z));
        }
    return true;
}

static bool retry_then_success(void) {
    for (int imu = IMU0; imu <= IMU1; imu++) {
        reset(imu);
        raw_value(0x22, 200);
        raw_value(0x24, -100);
        fail_byte = 2;
        failures_left = 2;
        vec3 value = {11, 22, 33};
        CHECK(getIMU(sensor(imu), undefined_vec3, &value) == GET_IMU_SUCCESS);
        CHECK(calibration_calls == 1 && starts == 5 && stops == 5);
        float scale = imu == IMU0 ? 0.00875f : 0.035f;
        CHECK(value.x == 200 * scale && value.y == -100 * scale && value.z == 0);
    }
    return true;
}

static bool diagnostics(void) {
    reset(IMU0);
    CHECK(imu_hasExpectedValuesGyro());
    reset(IMU0);
    fail_byte = 1;
    fail_on_start = 10; // Nine good samples must not hide a failed one.
    CHECK(!imu_hasExpectedValuesGyro());
    reset(IMU1);
    raw_value(0x2c, 8197);
    CHECK(imu_hasExpectedValuesAccel());
    fail_byte = 2;
    CHECK(!imu_hasExpectedValuesAccel());
    return true;
}

static bool stability_failure(void) {
    for (int axis = 0; axis < 3; axis++) {
        reset(IMU0);
        fail_byte = 2;
        fail_address = 0x22 + 2 * axis;
        test_initial_state();
        CHECK(assertion_failures > 0);
        CHECK(starts == 3 && stops == 3); // Stop at the first invalid sample.
    }
    return true;
}

static bool stability_success(void) {
    reset(IMU0);
    test_initial_state();
    CHECK(assertion_failures == 0);
    CHECK(starts == 15 && stops == 15);
    return true;
}

static bool diagnostic_output(void) {
    reset(IMU0);
    fail_byte = 0;
    imu_printAllValues();
    CHECK(strstr(printed, "X: nan") && !strstr(printed, "25.000000"));
    return true;
}

int main(void) {
    const struct { const char *name; bool (*run)(void); } tests[] = {
        {"read_failures", read_failures}, {"valid_readings", valid_readings},
        {"identity", identity}, {"vector_failures", vector_failures},
        {"vector_success", vector_success}, {"exhausted_retries", exhausted_retries},
        {"retry_then_success", retry_then_success}, {"diagnostics", diagnostics},
        {"stability_failure", stability_failure}, {"stability_success", stability_success},
        {"diagnostic_output", diagnostic_output}
    };
    int failures = 0;
    for (unsigned i = 0; i < sizeof(tests) / sizeof(tests[0]); i++) {
        bool ok = tests[i].run();
        printf("%s %s\n", ok ? "PASS" : "FAIL", tests[i].name);
        if (!ok) failures++;
    }
    return failures != 0;
}
