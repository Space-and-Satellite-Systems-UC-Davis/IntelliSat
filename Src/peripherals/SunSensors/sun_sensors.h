
#include <globals.h>
#include <ADC/adc.h>

typedef struct PANEL_STRUCT {
    ADC_TypeDef* adcDiode0;
    ADC_TypeDef* adcDiode1;
    int diode0Channel;
    int diode1Channel;
} panel_t;

typedef enum DIODE_ENUM {DIODE0, DIODE1} DIODES;


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


