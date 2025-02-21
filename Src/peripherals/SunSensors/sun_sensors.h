
#include <globals.h>
#include <ADC/adc.h>

typedef struct PANEL_STRUCT {
    ADC_TypeDef* adcDiode0;
    ADC_TypeDef* adcDiode1;
    int diode0Channel;
    int diode1Channel;
} PANEL_INFO;

typedef enum DIODE_ENUM {DIODE0, DIODE1} DIODES;


typedef enum PANELS_ENUM {
    PANEL0,
    PANEL1,
    PANEL2,
    PANEL3,
    PANEL4,
    PANEL5
} PANELS;


void sun_sensor_init();

float sun_sensors_readVoltage(PANELS panelNumber, DIODES diodeNumber);


