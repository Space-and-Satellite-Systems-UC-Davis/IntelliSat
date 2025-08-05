#include "sun_sensors.h"

panel_t panel0, panel1, panel2, panel3, panel4, panel5;

panel_t* panelInfo0;
panel_t* panelInfo1 = &panel1;
panel_t* panelInfo2 = &panel2;
panel_t* panelInfo3 = &panel3;
panel_t* panelInfo4 = &panel4;
panel_t* panelInfo5 = &panel5;


static void sun_sensors_initializePanelConstants() {
    panelInfo0->adcDiode0 = ADC3;
    panelInfo0->adcDiode1 = ADC3;
    panelInfo0->diode0Channel = 6;
    panelInfo0->diode1Channel = 7;
    panelInfo0->i2cPort = GPIOG;
    panelInfo0->scl_pin = 13;
    panelInfo0->sda_pin = 14;

    panelInfo1->adcDiode0 = ADC3;
    panelInfo1->adcDiode1 = ADC2;
    panelInfo1->diode0Channel = 12;
    panelInfo1->diode1Channel = 5;
    panelInfo1->i2cPort = GPIOF;
    panelInfo1->sda_pin = 11;
    panelInfo1->scl_pin = 2;

    panelInfo2->adcDiode0 = ADC2;
    panelInfo2->adcDiode1 = ADC2;
    panelInfo2->diode0Channel = 6;
    panelInfo2->diode1Channel = 7;
    panelInfo2->i2cPort = GPIOG;
    panelInfo2->sda_pin = 0;
    panelInfo2->scl_pin = 1;

    panelInfo3->adcDiode0 = ADC3;
    panelInfo3->adcDiode1 = ADC3;
    panelInfo3->diode0Channel = 10;
    panelInfo3->diode1Channel = 9;
    panelInfo3->i2cPort = GPIOF;
    panelInfo3->sda_pin = 4;
    panelInfo3->scl_pin = 5;


    panelInfo4->adcDiode0 = ADC2;
    panelInfo4->adcDiode1 = ADC2;
    panelInfo4->diode0Channel = 1;
    panelInfo4->diode1Channel = 2;

    panelInfo5->adcDiode0 = ADC2;
    panelInfo5->adcDiode1 = ADC2;
    panelInfo5->diode0Channel = 3;
    panelInfo5->diode1Channel = 4;
}

static panel_t* sun_sensors_panelInfoSelect(PANELS panel) {
    switch (panel) {
        case PANEL0:
            return panelInfo0;
        case PANEL1:
            return panelInfo1;
        case PANEL2:
            return panelInfo2;
        case PANEL3:
            return panelInfo3;
        case PANEL4:
            return panelInfo4;
        case PANEL5:
            return panelInfo5;                 
    }
    return NULL;
}

void sun_sensor_init() {
    adc_init();
    sun_sensors_initializePanelConstants();
    pwrmon_init(AVERAGE_1, CONVERT_1100, CONVERT_1100, 68,30);
    temp_sensor_init(RESOLUTION_12);
}


float sun_sensors_readVoltage(PANELS panelNumber, DIODES diodeNumber) {
    panel_t* panel = sun_sensors_panelInfoSelect(panelNumber);

    ADC_TypeDef* adc = NULL;
    int channel = -1;

    if (diodeNumber == DIODE0) {
        adc = panel->adcDiode0;
        channel = panel->diode0Channel;
    } else {
        adc = panel->adcDiode1;
        channel = panel->diode1Channel;
    }

    return adc_readVoltage(adc_readChannel(adc, channel));
    
}

float sun_sensors_readShuntVoltage(PANELS panelNum) {
    panel_t* panel = sun_sensors_panelInfoSelect(panelNum);
    return pwrmon_getShuntVoltage(panel->i2cPort, panel->scl_pin, panel->sda_pin);
}

float sun_sensors_readBusVoltage(PANELS panelNum) {
    panel_t* panel = sun_sensors_panelInfoSelect(panelNum);
    return pwrmon_getBusVoltage(panel->i2cPort, panel->scl_pin, panel->sda_pin);
}

float sun_sensors_readPower(PANELS panelNum) {
    panel_t* panel = sun_sensors_panelInfoSelect(panelNum);
    return pwrmon_getPower(panel->i2cPort, panel->scl_pin, panel->sda_pin);
}

float sun_sensors_readCurrent(PANELS panelNum) {
    panel_t* panel = sun_sensors_panelInfoSelect(panelNum);
    return pwrmon_getCurrent(panel->i2cPort, panel->scl_pin, panel->sda_pin);
}

float sun_sensors_readTemp(PANELS panelNum, TEMP tempNum) {
    panel_t* panel = sun_sensors_panelInfoSelect(panelNum);
    int tempAddress = TMP0_ADDRESS;
    if (tempNum == TEMP1) {
        tempAddress = TMP1_ADDRESS;
    }
    return temp_sensor_getTemp(panel->i2cPort, panel->scl_pin, panel->sda_pin, tempAddress);

}

void sun_sensors_shutdownAll(){
    power_set_mode(MODE_POWERED_DOWN);
    shutdown(true);
}

void sun_sensors_repowerAll(){
    power_set_mode(MODE_CONTINUOUS);
    shutdown(false);
}





