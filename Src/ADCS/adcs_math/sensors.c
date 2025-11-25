#include "sensors.h"

getMag_status getMag(vi_sensor sensor, vec3 prevVal, vec3 *currVal)
{

    float sensor_offset, sensor_scalar, sensor_filter_constant;
    vi_MAG_choice choice = sensor.field.mag_choice;
    vec3 mag;           // Local varible to store the magnotometer reading
    int errorCount = 0; // Local varible to store error occurances

    errorCount = 0;
    while (vi_get_mag(choice, &(mag.x), &(mag.y), &(mag.z))) {
        errorCount++;
        if (errorCount >= 3)
            return GET_MAG_FAILURE;
    };

    /*################ PROTOTYPE CODE ################*/
    // I think it works but I have noooo idea if it works
    // If it breaks then fall back to BACKUP CODE

    sensor.component = VI_COMP_MAG_VALUE;
    sensor.field.mag_value = choice == VI_MAG1 ? VI_MAG_X1 : VI_MAG_X2;

    // Starting from the X - axis
    double *magCurrPtr = (double *)&mag;
    double *magPrevPtr = (double *)&prevVal;

    // Perform calibration on each axis
    for (int i = 0; i < 3; i++) {

        int errorCount = 0;
        while(vi_get_sensor_calibration(sensor, &sensor_offset, &sensor_scalar, 
                                        &sensor_filter_constant))
        {
            errorCount++;
            if (errorCount >= 3)
                return MAG_CALIBRATION_FAILURE;
        }

        double currVal = *(magCurrPtr + i);
        double prevVal = *(magPrevPtr + i);

        currVal = get_sensor_calibration(currVal, prevVal, sensor_offset,
                                         sensor_scalar, sensor_filter_constant);
        *(magCurrPtr + i) = currVal;

        sensor.field.mag_value += 1;
    }

    return GET_MAG_SUCCESS;

    /*################ BACKUP CODE ################*/
    // To be used in case Chunho's pointer magic fails
    /*

    sensor.field.mag_value = VI_MAG_X1;
    if (vi_get_sensor_calibration(sensor, &sensor_offset,
                                    &sensor_scalar, &sensor_filter_constant))
            return MAG_CALIBRATION_FAILURE;
    mag.x = get_sensor_calibration(mag.x, prevVal.x, sensor_offset,
                                    sensor_scalar, sensor_filter_constant);

    sensor.field.mag_value = VI_MAG_Y1;
    if (vi_get_sensor_calibration(sensor, &sensor_offset,
                                    &sensor_scalar, &sensor_filter_constant))
            return MAG_CALIBRATION_FAILURE;
    mag.y = get_sensor_calibration(mag.y, prevVal.y, sensor_offset,
                                        sensor_scalar, sensor_filter_constant);

    sensor.field.mag_value = VI_MAG_Z1;
    if (vi_get_sensor_calibration(sensor, &sensor_offset,
                                    &sensor_scalar, &sensor_filter_constant))
            return MAG_CALIBRATION_FAILURE;
    mag.z = get_sensor_calibration(mag.z, prevVal.z, sensor_offset,
                                        sensor_scalar, sensor_filter_constant);

    return GET_MAG_SUCCESS;

    */
}

getIMU_status getIMU(vi_sensor sensor, vec3 prevVal, vec3 *currVal)
{

    float sensor_offset, sensor_scalar, sensor_filter_constant;
    vi_IMU_choice choice = sensor.field.imu_choice;
    vec3 reading; // Local varible to store sensor
    int errorCount = 0;


      while(vi_get_angvel(choice, &(reading.x), &(reading.y), &(reading.z)))
    {
        errorCount++;
        if (errorCount >= 3) return GET_IMU_FAILURE;
    };



    sensor.component = VI_COMP_IMU_VALUE;
    sensor.field.imu_value = choice == VI_IMU1 ? VI_IMU1_X : VI_IMU2_X;
    
    //
    double *magCurrPtr = (double *)&reading;
    double *magPrevPtr = (double *)&prevVal;

    for (int i = 0; i < 3; i++){

        int errorCount = 0;
        while(vi_get_sensor_calibration(sensor, &sensor_offset, &sensor_scalar, 
                                        &sensor_filter_constant)){
            errorCount++;
            if(errorCount >= 3) return GET_IMU_FAILURE;

                                  }
        
        double currVal = *(magCurrPtr + i);
        double prevVal = *(magPrevPtr + i);

        currVal = get_sensor_calibration(currVal, prevVal, sensor_offset, 
                                       sensor_scalar, sensor_filter_constant);

        *(magCurrPtr + i) = currVal;

        sensor.field.imu_value += 1;

    }

    return GET_IMU_SUCCESS;

    /*
    Old code: 

    sensor.field.imu_value = VI_IMU1_X;
    if (vi_get_sensor_calibration(sensor, &sensor_offset, &sensor_scalar,
                                  &sensor_filter_constant))
        return IMU_CALIBRATION_FAILURE;
    reading.x = get_sensor_calibration(reading.x, prevVal.x, sensor_offset,
                                       sensor_scalar, sensor_filter_constant);

    sensor.field.imu_value = VI_IMU1_Y;
    if (vi_get_sensor_calibration(sensor, &sensor_offset, &sensor_scalar,
                                  &sensor_filter_constant))
        return IMU_CALIBRATION_FAILURE;
    reading.y = get_sensor_calibration(reading.y, prevVal.y, sensor_offset,
                                       sensor_scalar, sensor_filter_constant);

    sensor.field.imu_value = VI_IMU1_Z;
    if (vi_get_sensor_calibration(sensor, &sensor_offset, &sensor_scalar,
                                  &sensor_filter_constant))
        return IMU_CALIBRATION_FAILURE;
    reading.z = get_sensor_calibration(reading.z, prevVal.z, sensor_offset,
                                       sensor_scalar, sensor_filter_constant);
    */

}

getCSS_status getCSS(vi_sensor sensor, vi_CSS_face face, double prevVal,
                     double *currVal)
{

    float sensor_offset, sensor_scalar, sensor_filter_constant;
    vi_CSS_choice choice = sensor.field.css_choice;

    double reading; // Local varible to store sensor

    // Todo: Implement retries
    if (vi_get_css(choice, &reading)) {
        return GET_CSS_FAILURE;
    }

    // Instruct which sensor to read from
    sensor.field.css_value = choice == VI_CSS1 ? VI_CSS_PX1 : VI_CSS_PX2;

    // Instruct which face to read from
    sensor.field.css_value += face;

    if (vi_get_sensor_calibration(sensor, &sensor_offset, &sensor_scalar,
                                  &sensor_filter_constant))
        return CSS_CALIBRATION_FAILURE;
    reading = get_sensor_calibration(reading, prevVal, sensor_offset,
                                     sensor_scalar, sensor_filter_constant);

    return GET_CSS_SUCCESS;
}

int is_in_eclipse()
{

    static int iteration = 0;
    //     px1, px2, nx1, nx2, py1, py2, ny1, ny2, pz1, pz2, nz1, nz2;
    double readingsOne[6];
    double readingsTwo[6];
    double prevValOne, prevValTwo = 0; // TODO: MUST CHANGE

    vi_sensor sensorOne, sensorTwo;
    sensorOne.component = VI_COMP_CSS_CHOICE;
    sensorTwo.component = VI_COMP_CSS_CHOICE;

    sensorOne.field.css_choice = VI_CSS1;
    sensorTwo.field.css_choice = VI_CSS2;

    for (int face = VI_CSS_PX; face <= VI_CSS_NZ; face++) {
        // Get Readings from sensor
        getCSS(sensorOne, face, prevValOne, &readingsOne[face - 1]);
        getCSS(sensorTwo, face, prevValTwo, &readingsTwo[face - 1]);

        // Increment to next set of sensors
        // sensorOne.field.css_choice++;
        // sensorTwo.field.css_choice++;
    }

    double sum = 0;
    for (int i = 0; i < 6; i++) {
        sum += pow(readingsOne[i], 2) + pow(readingsTwo[i], 2);
    }

    double magnitude = sqrt(sum);

    iteration++;

    if (magnitude <= 0.25) {
        return 1;
    }

    return 0;
}

static const char alternations[256] = {
    0b01100111, 0b01110101, 0b10000000, 0b11101011, 0b01100100, 0b11111111,
    0b11011100, 0b10100100, 0b01011011, 0b01010111, 0b01010101, 0b00010000,
    0b00100100, 0b11100011, 0b00110110, 0b00011001, 0b11101010, 0b10100111,
    0b10000011, 0b01010011, 0b10001100, 0b10100010, 0b11100110, 0b11101100,
    0b11010011, 0b10111001, 0b00001001, 0b11010010, 0b00100000, 0b01111110,
    0b01010010, 0b11100010, 0b01010001, 0b11101000, 0b10000110, 0b10101100,
    0b00111111, 0b10010000, 0b10110101, 0b10100000, 0b00100110, 0b10011000,
    0b11010111, 0b11000010, 0b11010110, 0b01001110, 0b01111001, 0b00111011,
    0b01001100, 0b01000100, 0b00011110, 0b01110000, 0b11000001, 0b00110101,
    0b00101010, 0b01101110, 0b01110100, 0b10111010, 0b01101111, 0b00101001,
    0b10010101, 0b00110010, 0b11010100, 0b01000101, 0b10110000, 0b10010110,
    0b00011011, 0b00110001, 0b11100101, 0b10010100, 0b11100000, 0b10111000,
    0b11111110, 0b11111000, 0b11001010, 0b00110000, 0b01111010, 0b11110100,
    0b10011110, 0b11101110, 0b10101111, 0b00011000, 0b00001010, 0b10100011,
    0b11011010, 0b00111100, 0b01110111, 0b00011111, 0b00001110, 0b00111010,
    0b00011101, 0b11110011, 0b11001111, 0b00010011, 0b01101101, 0b10101110,
    0b01100110, 0b10011101, 0b00011100, 0b10101011, 0b00100101, 0b00101011,
    0b00010100, 0b10011011, 0b00000101, 0b01010000, 0b00010010, 0b00111101,
    0b01011111, 0b01011010, 0b00101110, 0b11011101, 0b01111101, 0b10001011,
    0b10101000, 0b10100001, 0b10110011, 0b11111101, 0b11011111, 0b11000111,
    0b00000111, 0b11110010, 0b01110010, 0b01101100, 0b10000100, 0b10101001,
    0b10100101, 0b01111011, 0b10000111, 0b10010001, 0b01111100, 0b10110111,
    0b01011000, 0b01011101, 0b01100101, 0b01101000, 0b10000101, 0b10001111,
    0b10010011, 0b10101010, 0b00111000, 0b01000011, 0b10101101, 0b11001011,
    0b10110001, 0b01010110, 0b01001111, 0b10111111, 0b11000100, 0b01100011,
    0b11100100, 0b01100001, 0b00101000, 0b01001001, 0b01111000, 0b11101001,
    0b00111001, 0b01100000, 0b11111100, 0b11000000, 0b10000001, 0b00010111,
    0b00010101, 0b11000101, 0b00011010, 0b10111011, 0b00110011, 0b11011110,
    0b11111011, 0b11110111, 0b10111100, 0b11011000, 0b00100111, 0b10001101,
    0b01110110, 0b01101011, 0b01101001, 0b01000010, 0b01001101, 0b11010101,
    0b01001000, 0b10011010, 0b00101101, 0b11111010, 0b11100111, 0b10001000,
    0b11011001, 0b00101100, 0b00100001, 0b10010111, 0b01111111, 0b11000011,
    0b10010010, 0b01011001, 0b00010110, 0b01011100, 0b11001100, 0b11110000,
    0b01101010, 0b01110011, 0b11111001, 0b01000111, 0b00110100, 0b01001010,
    0b11001110, 0b01000001, 0b11101111, 0b00100010, 0b10001001, 0b10111101,
    0b10110100, 0b11010000, 0b01110001, 0b10100110, 0b10110110, 0b10000010,
    0b00000011, 0b01000110, 0b11011011, 0b11001000, 0b01001011, 0b10001010,
    0b11100001, 0b01000000, 0b00010001, 0b01011110, 0b10110010, 0b11110001,
    0b11001101, 0b00001101, 0b00001100, 0b11110110, 0b00101111, 0b01010100,
    0b11101101, 0b11110101, 0b00000001, 0b00000010, 0b00001000, 0b00100011,
    0b10001110, 0b11000110, 0b11001001, 0b11010001, 0b00111110, 0b10011111,
    0b00000100, 0b10011001, 0b00110111, 0b10011100, 0b10111110, 0b00000110,
    0b00001011, 0b00001111, 0b01100010, 0b00000000
};

int sensor_pair_choice(vi_sensor sensor, int generation)
{
    int mask = 0;
    switch (sensor.component) {
        case VI_COMP_CSS_CHOICE:
            switch (sensor.field.css_choice) {
                case VI_CSS_PX:
                    mask = 1;
                    break;
                case VI_CSS_NX:
                    mask = 2;
                    break;
                case VI_CSS_PY:
                    mask = 3;
                    break;
                case VI_CSS_NY:
                    mask = 4;
                    break;
                case VI_CSS_PZ:
                    mask = 5;
                    break;
                case VI_CSS_NZ:
                    mask = 6;
                    break;
            }
            break;

        case VI_COMP_MAG_CHOICE:
            mask = 7;
            break;

        case VI_COMP_IMU_CHOICE:
            mask = 8;
            break;

        case VI_COMP_HDD_CHOICE:
        case VI_COMP_TMP_CHOICE:
        case VI_COMP_SOL_CHOICE:
        case VI_COMP_CSS_VALUE:
        case VI_COMP_IMU_VALUE:
        case VI_COMP_MAG_VALUE:
            return 0;
    }

    if ((alternations[generation % 256] | (1 << mask)) != 0) {
        return 2;
    } else {
        return 1;
    }
}
