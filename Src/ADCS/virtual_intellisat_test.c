#include "virtual_intellisat.h"
#include "../tools/print_scan.h"

#define     UNSET       -1

/* 
A list of functions to test:

New Functions:
vi_get_epoch_status();
vi_get_mag_status();
vi_get_temp_status();
vi_get_solar_panel_status();

Old Functions:
vi_get_curr_millis_status();
vi_get_angvel_status();

I think everything else in virutal_intellisat.c hasn't been implemented yet, add to the list as
more functions are implemented

note: I should check if some of these existing old functions already have test code written in another test file

- Corey Nguyen (1/16/26)

*/


/*
vi_get_epoch() notes:
- On this current branch (ADCS_Software Integration) as of 1/19/26, there is no existing getter
function in Src/system_config/RTC/rtc.c for getting the year, month, and day yet. I left placeholders
in the relevant parameters in the meantime (-1). Once the getter function for those calendar values is
implemented, please make sure to update this test. 
*/
void test_vi_get_epoch() {
    int year = UNSET, month = UNSET, day = UNSET,
        hour = UNSET, minute = UNSET, second = UNSET;

    vi_get_epoch(&year, &month, &day, &hour, &minute, &second);
    
    printMsg(
        "(Note: -1 means the term failed to be set)\r\n"
        "Year: %d\r\n"
        "Month: %d\r\n"
        "Day: %d\r\n"
        "Hour: %d\r\n"
        "Minute: %d\r\n"
        "Second: %d\r\n",
        year, month, day, hour, minute, second
    );
    
    return;
}

void test_vi_get_mag() {
    vi_sensor VI_MAG_X1 = {MAG, One, PX};
    double mag_x = UNSET;
    double mag_y = UNSET;
    double mag_z = UNSET;

    vi_get_mag(VI_MAG_X1, &mag_x, &mag_y, &mag_z);

    printMsg(
        "(Note: -1 means the term failed to be set)\r\n"
        "Sensor Input: VI_MAG_X1\r\n"
        "mag_x: %f\r\n"
        "mag_y: %f\r\n"
        "mag_z: %f\r\n",
        mag_x, mag_y, mag_z
    );

    return;
}

/*
vi_get_temp() notes:
- I don't know too much about what the existing sensors' struct values are other than VI_MAG_X1
*/
void test_vi_get_temp() {
    // TODO also test IMU, TEMP, SOL sensors
    vi_sensor VI_MAG_X1 = {MAG, One, PX};
    double temp = UNSET;

    vi_get_temp(VI_MAG_X1, &temp);

    printMsg(
        "(Note: -1 means the term failed to be set)\r\n"
        "Sensor Input: VI_MAG_X1\r\n"
        "Temperature: %f\r\n", temp
    );

    return;
}

void test_vi_get_solar_panel_current() {
    // TODO
    vi_sensor VI_SOL_PX1 = {SOL, One, PX};   // not sure if this one exists yet
    double current = UNSET;

    vi_get_solar_panel_current(VI_SOL_PX1, &current);

    printMsg(
        "(Note: -1 means the term failed to be set)\r\n"
        "Sensor Input: VI_SOL_PX1 = {SOL, One, PX}\r\n"
        "Current: %f\r\n", current
    );

    return;
}

// read the note above
void test_vi_get_curr_millis() {
    uint64_t curr_millis = UNSET;

    vi_get_curr_millis(&curr_millis);

    printMsg(
        "(Note: -1 means the term failed to be set)\r\n"
        "curr_millis: %f\r\n", curr_millis
    );

    return;
}

// read the note above
void test_vi_get_angvel() {
    // TODO
    vi_sensor VI_IMU_X1 = {IMU, One, PX};
    double angvel_x = UNSET;
    double angvel_y = UNSET;
    double angvel_z = UNSET;

    vi_get_angvel(VI_IMU_X1, &angvel_x, &angvel_y, &angvel_z);

    printMsg(
        "(Note: -1 means the term failed to be set)\r\n"
        "Sensor Input: VI_IMU_X1\r\n"
        "angvel_x: %f\r\n"
        "angvel_y: %f\r\n"
        "angvel_z: %f\r\n",
        angvel_x, angvel_y, angvel_z
    );

    return;
}

void testFunction_virtual_intellisat() {
    /* Test: vi_get_epoch_status */
    printMsg("===== Testing: test_vi_get_epoch() =====\r\n");
    test_vi_get_epoch();
    printMsg("Test Concluded\r\n\n");

    /* Test: vi_get_mag_status */
    //printMsg("===== Testing: test_vi_get_mag() =====\r\n");
    //test_vi_get_mag();
    //printMsg("Test Concluded\r\n\n");

    /* Test: vi_get_temp_status */
    printMsg("===== Testing: test_vi_get_temp() =====\r\n");
    test_vi_get_temp();
    printMsg("Test Concluded\r\n\n");

    /* Test: vi_get_solar_panel_status */
    printMsg("===== Testing: test_vi_get_solar_panel_current() =====\r\n");
    test_vi_get_solar_panel_current();
    printMsg("Test Concluded\r\n\n");

    /* Test: vi_get_curr_millis_status */
    printMsg("===== Testing: test_vi_get_curr_millis() =====\r\n");
    test_vi_get_curr_millis();
    printMsg("Test Concluded\r\n\n");

    /* Test: vi_get_angvel_status */
    //printMsg("===== Testing: test_vi_get_angvel() =====\r\n");
    //test_vi_get_angvel();
    //printMsg("Test Concluded\r\n\n");

    // Add more tests below as more functions in virtual_intellisat.c are implemented

    return;
}
