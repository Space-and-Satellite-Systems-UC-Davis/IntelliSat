/*
 * QMC5883L.c  (MAG interface)
 *
 *	- Jan  6, 2024
 *		Author		 : Darsh
 *		Log			 : Edited function names to follow consistent naming
 *
 *  - Apr 29, 2023 (Creation)
 *      Author       : Tim S.
 *      Contributors : nithinsenthil , Raphael
 *      Log          : Magnetometer Control functions written
 */


#include "QMC5883L.h"

/*************************** MAG Parameters *************************/

static uint16_t mag_magFullScale = 0;
static Mag_Selector current_mag_selected = MAG0;

/*************************** MAG Helper Functions *************************/

int16_t ScaleData(int16_t data, int scale){
	return ((data * scale) / (uint16_t)(-1));
}


/**
 * Configures the control registers of the OP1/2/3 magnetometer.
 *
 * @param rate The output data rate of the magnetometer.
 * @param scale The full scale range of the magnetometer.
 * @param oversample The number of samples to average for each measurement.
 *
 * @returns None
 */
void magCtrl(int rate, int scale, int oversample) {
    #if OP_REV == 2 || OP_REV == 1
        if (rate) {
            softi2c_writeReg(MAG_I2C_SCL_GPIO, MAG_I2C_SCL_PIN, MAG_I2C_SDA_GPIO, MAG_I2C_SDA_PIN, MAG_ADDR, 0x09, oversample << 6 | scale << 4 | (rate-1) << 2 | 1);
        } else {
            softi2c_writeReg(MAG_I2C_SCL_GPIO, MAG_I2C_SCL_PIN, MAG_I2C_SDA_GPIO, MAG_I2C_SDA_PIN, MAG_ADDR, 0x09, 0);
        }
    #elif OP_REV == 3
        if (rate) {
            softi2c_writeReg(MAG0_I2C_SCL_GPIO, MAG0_I2C_SCL_PIN, MAG0_I2C_SDA_GPIO, MAG0_I2C_SDA_PIN, MAG_ADDR, 0x09, oversample << 6 | scale << 4 | (rate-1) << 2 | 1);
            softi2c_writeReg(MAG1_I2C_SCL_GPIO, MAG1_I2C_SCL_PIN, MAG1_I2C_SDA_GPIO, MAG1_I2C_SDA_PIN, MAG_ADDR, 0x09, oversample << 6 | scale << 4 | (rate-1) << 2 | 1);
        } else {
            softi2c_writeReg(MAG0_I2C_SCL_GPIO, MAG0_I2C_SCL_PIN, MAG0_I2C_SDA_GPIO, MAG0_I2C_SDA_PIN, MAG_ADDR, 0x09, 0);
            softi2c_writeReg(MAG1_I2C_SCL_GPIO, MAG1_I2C_SCL_PIN, MAG1_I2C_SDA_GPIO, MAG1_I2C_SDA_PIN, MAG_ADDR, 0x09, 0);
        }
    #endif

    switch (scale) {
        case MAG_FS_2_G:
            mag_magFullScale = 2;
            break;
        case MAG_FS_8_G:
            mag_magFullScale = 8;
            break;
        default:
            mag_magFullScale = 0;
            break;
    }
}

/*************************** MAG Interface Functions *************************/

void mag_init(int rate, int scale, int oversample) {
    #if OP_REV == 2 || OP_REV == 1
        softi2c_init(MAG_I2C_SCL_GPIO, MAG_I2C_SCL_PIN, MAG_I2C_SDA_GPIO, MAG_I2C_SDA_PIN);

        softi2c_writeReg(MAG_I2C_SCL_GPIO, MAG_I2C_SCL_PIN, MAG_I2C_SDA_GPIO, MAG_I2C_SDA_PIN, MAG_ADDR, 0x0A, 0x80); // soft reset
        nop(50);
        softi2c_writeReg(MAG_I2C_SCL_GPIO, MAG_I2C_SCL_PIN, MAG_I2C_SDA_GPIO, MAG_I2C_SDA_PIN, MAG_ADDR, 0x0A, 0x00);
        softi2c_writeReg(MAG_I2C_SCL_GPIO, MAG_I2C_SCL_PIN, MAG_I2C_SDA_GPIO, MAG_I2C_SDA_PIN, MAG_ADDR, 0x0B, 1);
        nop(50);
        magCtrl(rate, scale, oversample);
    #elif OP_REV == 3
        softi2c_init(MAG0_I2C_SCL_GPIO, MAG0_I2C_SCL_PIN, MAG0_I2C_SDA_GPIO, MAG0_I2C_SDA_PIN);
        softi2c_init(MAG1_I2C_SCL_GPIO, MAG1_I2C_SCL_PIN, MAG1_I2C_SDA_GPIO, MAG1_I2C_SDA_PIN);
        softi2c_writeReg(MAG0_I2C_SCL_GPIO, MAG0_I2C_SCL_PIN, MAG0_I2C_SDA_GPIO, MAG0_I2C_SDA_PIN, MAG_ADDR, 0x0A, 0x80); // soft reset
        softi2c_writeReg(MAG1_I2C_SCL_GPIO, MAG1_I2C_SCL_PIN, MAG1_I2C_SDA_GPIO, MAG1_I2C_SDA_PIN, MAG_ADDR, 0x0A, 0x80); // soft reset
        nop(50);
        softi2c_writeReg(MAG0_I2C_SCL_GPIO, MAG0_I2C_SCL_PIN, MAG0_I2C_SDA_GPIO, MAG0_I2C_SDA_PIN, MAG_ADDR, 0x0A, 0x00);
        softi2c_writeReg(MAG1_I2C_SCL_GPIO, MAG1_I2C_SCL_PIN, MAG1_I2C_SDA_GPIO, MAG1_I2C_SDA_PIN, MAG_ADDR, 0x0A, 0x00);

        softi2c_writeReg(MAG0_I2C_SCL_GPIO, MAG0_I2C_SCL_PIN, MAG0_I2C_SDA_GPIO, MAG0_I2C_SDA_PIN, MAG_ADDR, 0x0B, 1);
        softi2c_writeReg(MAG1_I2C_SCL_GPIO, MAG1_I2C_SCL_PIN, MAG1_I2C_SDA_GPIO, MAG1_I2C_SDA_PIN, MAG_ADDR, 0x0B, 1);
        nop(50);
        magCtrl(rate, scale, oversample);
    #endif

}

int16_t mag_read_X() {
    #if OP_REV == 2 || OP_REV == 1
        int16_t x = softi2c_readRegHighLow(MAG_I2C_SCL_GPIO, MAG_I2C_SCL_PIN, MAG_I2C_SDA_GPIO, MAG_I2C_SDA_PIN, MAG_ADDR, 0x01, 0x00);
    #elif OP_REV == 3
        int16_t x = 0;
        if (mag_select == MAG0)
            x = softi2c_readRegHighLow(MAG0_I2C_SCL_GPIO, MAG0_I2C_SCL_PIN, MAG0_I2C_SDA_GPIO, MAG0_I2C_SDA_PIN, MAG_ADDR, 0x01, 0x00);
        else
            x = softi2c_readRegHighLow(MAG1_I2C_SCL_GPIO, MAG1_I2C_SCL_PIN, MAG1_I2C_SDA_GPIO, MAG1_I2C_SDA_PIN, MAG_ADDR, 0x01, 0x00);
    #endif

	return ScaleData(x, mag_magFullScale);
}

int16_t mag_read_Y() {
    #if OP_REV == 2 || OP_REV == 1
        int16_t y = softi2c_readRegHighLow(MAG_I2C_SCL_GPIO, MAG_I2C_SCL_PIN, MAG_I2C_SDA_GPIO, MAG_I2C_SDA_PIN, MAG_ADDR, 0x03, 0x02);
    #elif OP_REV == 3
        int16_t y = 0;
        if (mag_select == MAG0)
            y = softi2c_readRegHighLow(MAG0_I2C_SCL_GPIO, MAG0_I2C_SCL_PIN, MAG0_I2C_SDA_GPIO, MAG0_I2C_SDA_PIN, MAG_ADDR, 0x03, 0x02);
        else 
            y = softi2c_readRegHighLow(MAG1_I2C_SCL_GPIO, MAG1_I2C_SCL_PIN, MAG1_I2C_SDA_GPIO, MAG1_I2C_SDA_PIN, MAG_ADDR, 0x03, 0x02);
    #endif

    return ScaleData(y, mag_magFullScale);
}

int16_t mag_read_Z() {
    #if OP_REV == 2 || OP_REV == 1
        int16_t z = softi2c_readRegHighLow(MAG_I2C_SCL_GPIO, MAG_I2C_SCL_PIN, MAG_I2C_SDA_GPIO, MAG_I2C_SDA_PIN, MAG_ADDR, 0x05, 0x04);
    #elif OP_REV == 3
        int16_t z = 0;
        if (mag_select == MAG0)
            z = softi2c_readRegHighLow(MAG0_I2C_SCL_GPIO, MAG0_I2C_SCL_PIN, MAG0_I2C_SDA_GPIO, MAG0_I2C_SDA_PIN, MAG_ADDR, 0x05, 0x04);
        else   
            z = softi2c_readRegHighLow(MAG1_I2C_SCL_GPIO, MAG1_I2C_SCL_PIN, MAG1_I2C_SDA_GPIO, MAG1_I2C_SDA_PIN, MAG_ADDR, 0x05, 0x04);
    #endif

    return ScaleData(z, mag_magFullScale);
}

int16_t mag_readTemp() {
    #if OP_REV == 2 || OP_REV == 1
        int16_t temp = softi2c_readRegHighLow(MAG_I2C_SCL_GPIO, MAG_I2C_SCL_PIN, MAG_I2C_SDA_GPIO, MAG_I2C_SDA_PIN, MAG_ADDR, 0x07, 0x06);
    #elif OP_REV == 3
        int16_t temp = 0;
        if (mag_select == MAG0)
            temp = softi2c_readRegHighLow(MAG0_I2C_SCL_GPIO, MAG0_I2C_SCL_PIN, MAG0_I2C_SDA_GPIO, MAG0_I2C_SDA_PIN, MAG_ADDR, 0x07, 0x06);
        else
            temp = softi2c_readRegHighLow(MAG1_I2C_SCL_GPIO, MAG1_I2C_SCL_PIN, MAG1_I2C_SDA_GPIO, MAG1_I2C_SDA_PIN, MAG_ADDR, 0x07, 0x06);
    #endif

	return temp;
}

void mag_select(Mag_Selector mag_picker) {
    current_mag_selected = mag_picker;
}
