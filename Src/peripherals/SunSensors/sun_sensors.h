
#include <globals.h>
#include <ADC/adc.h>
#include "INA226.h"
#include "TMP275.h"

typedef struct PANEL_STRUCT {
    ADC_TypeDef* adcDiode0;
    ADC_TypeDef* adcDiode1;
    int diode0Channel;
    int diode1Channel;
    GPIO_TypeDef* i2cPort;
    int scl_pin;
    int sda_pin;
} panel_t;

typedef enum DIODE_ENUM {DIODE0, DIODE1} DIODES;

typedef enum TEMP_ENUM {TEMP0, TEMP1} TEMP;


typedef enum PANELS_ENUM {
    PANEL0,
    PANEL1,
    PANEL2,
    PANEL3,
    PANEL4,
    PANEL5
} PANELS;

/**
 * Initalizes the sun sensors diode reading
 */
void sun_sensor_init();

/**
 * Read the voltage from a diode
 * 
 * @param panelNumber: A PANELS_ENUM to identify which panel to read from
 * @param diodeNumber: A DIODE_ENUM to identify which diode to read from
 * 
 * @return voltage 
 */
float sun_sensors_readVoltage(PANELS panelNumber, DIODES diodeNumber);

/**
 * Read the shunt voltage from power moniter
 * 
 * @param panelNum: A PANELS_ENUM to identify which panel to read from 
 * 
 * @return shunt voltage in mV
 * 
 */
float read_shunt_voltage(PANELS panelNum);

/**
 * Read the bus voltage from power moniter
 * 
 * @param panelNum: A PANELS_ENUM to identify which panel to read from 
 * 
 * @return bus voltage in V
 * 
 */
float read_bus_voltage(PANELS panelNum);


/**
 * Read the power from power moniter
 * 
 * @param panelNum: A PANELS_ENUM to identify which panel to read from 
 * 
 * @return power in W
 * 
 */
float read_power(PANELS panelNum);

/**
 * Read the current from power moniter
 * 
 * @param panelNum: A PANELS_ENUM to identify which panel to read from 
 * 
 * @return current in A
 * 
 */
float read_current(PANELS panelNum);

/**
 * Read the shunt voltage from power moniter
 * 
 * @param panelNum: A PANELS_ENUM to identify which panel to read from 
 * @param tempNum: A TEMP_ENUM to identify which temp sensor to read from
 * 
 * @return temperature in Celsius
 * 
 */
float read_temp(PANELS panelNum, TEMP tempNum);

/**
 * Power down all temp sensors and power moniters
 * 
 */
void power_down_all();

/**
 * Repower all temp sensors and power moniters into continuous mode
 * 
 */
void repower_all();


