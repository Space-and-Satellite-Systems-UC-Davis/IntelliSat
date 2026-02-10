#include "virtual_intellisat.h"

#include <SunSensors/sun_sensors.h>
#include <globals.h>
#include <print_scan.h>
#include <Buttons/buttons.h>

#include "../hdd/hdd_drive.h"
#include "../hdd/hdd_init.h"
#include "../system_config/Timers/timers.h"


int round_number(float num) {
	int result = num * 10000;
	if (result % 10 >= 5) {
		result += 10;
	}

	return result / 10;
}

float sample(uint64_t sampletime) {
	uint64_t reference_time = 0;
	vi_get_curr_millis(&reference_time);
	uint64_t curr_time=0;
	vi_get_curr_millis(&curr_time);
	//printMsg("Sampling at Time: %u\r\n", (uint32_t) curr_time);
	uint64_t time_remaining = sampletime;
	float sample = 0;
	float total_sample = 0;
	float voltage;
	while (curr_time <= (reference_time + time_remaining) ) {
		vi_get_curr_millis(&curr_time);
		//printMsg("Current Time: %u, ", (uint32_t) curr_time);
		voltage = sun_sensors_readVoltage(PANEL0, DIODE0);
		total_sample = total_sample + voltage;
		sample = sample + 1;
		//printMsg("Voltage: %f, Time Remaining: %f\r\n", (float) voltage, (float) time_remaining);
	}
	float average;
	average = total_sample / (sample);
	printMsg("Sample Average: %f V\r\n", (float) average);
	return average;
}

float find_max_sample(float set_sample_time) {
	float check_for_maximum;
	float initial_sample = sample(set_sample_time/3);
	float middle_sample = sample(set_sample_time/3);
	float final_sample = sample(set_sample_time/3);
	if (middle_sample > initial_sample && middle_sample > final_sample) {
		check_for_maximum = 1;
		printMsg("-->Maximum found at %f V\r\n", middle_sample);
	} else {
		check_for_maximum = 0;
		printMsg("No Maximum Found \r\n");
	}
	return check_for_maximum;
}



void testFunction_Diode_Reads() {
    printMsg("\r\nStarting Diode reading\r\n");
    float period = 0;
    //float temp = sun_sensors_readVoltage(PANEL0, DIODE0);
    //temp=(round_number(temp));
    uint64_t prev_max_ms;
    uint64_t curr_max_ms;
    uint32_t num_max;
    uint32_t HDD_demarcations = 1;

    const int targetPWM = 0;
    //const uint64_t DUTY_STEP = 0.5;

    hdd_arm(targetPWM);
    		//printMsg("Testing slipping.\r\n");
    		//pwm_setDutyCycle(targetPWM, SLIP_DUTY);
    		//delay_ms(5000);
    		//pwm_setDutyCycle(targetPWM, MID_DUTY);

	printMsg("Testing max duty \r\n");
	//pwm_setDutyCycle(targetPWM, MAX_DUTY);
	delay_ms(5000);

	uint64_t step_duty_increment = 2;
	const uint64_t MID_DUTY = 50;
	uint64_t curr_duty = MID_DUTY;
	uint64_t reference_timer = 0;
	vi_get_curr_millis(&reference_timer);
	uint64_t global_timer;
	vi_get_curr_millis(&global_timer);

    while (1) {

    	vi_get_curr_millis(&global_timer);
    	if (global_timer > global_timer + reference_timer) {
    		curr_duty = curr_duty + step_duty_increment;
    		pwm_setDutyCycle(targetPWM, curr_duty);
    		printMsg("Increasing Duty by %u", step_duty_increment);
    	}

    	if (find_max_sample(300) == 1) {
    		vi_get_curr_millis(num_max > 0 ? &curr_max_ms : &prev_max_ms);
    		num_max = num_max + 1;
    		//printMsg("Maximum received! \r\n");
    	}
    	if (num_max < 2) {
    		continue;
    	}
    	period = curr_max_ms - prev_max_ms;
    	float Frequency = 1000*(1/period);
    	printMsg("\n=>Frequency, %f and Period,%f\n\r", Frequency,period);
		float photoRPM = 60*HDD_demarcations*Frequency;
		printMsg("=>RPM: %f\r\n\n", photoRPM);
		num_max = 0;
    }
}


		//float photoTimer=0;
		    //float newPhotoTimer=0;
		    //float photoDelay=500;
		    //float photoPeriod=0;
		    //int processedVolt=0;
            //if (1) {
                ////delay_ms(photoDelay); // 1000
                ////photoTimer = photoTimer + photoDelay;
            	//float voltage = sun_sensors_readVoltage(PANEL0, DIODE0);
            	//find_max_sample(0.3);
                //vi_get_curr_millis(&newPhotoTimer);
                ////photoTimer = newPhotoTimer - photoTimer;
                //printMsg("Reading Voltage\r\n");
                //printMsg("V1: %f \n", voltage);
                ////voltage = sun_sensors_readVoltage(PANEL0, DIODE1);
                ////printMsg("V2: %f\r\n", voltage);
                //processedVolt = round_number(voltage);
                //printMsg("To Match %f",temp);
                //if (processedVolt>=0.3) {
                	//photoTimer = newPhotoTimer - photoTimer;
                	//photoPeriod = photoTimer;
                	//printMsg("%f matches %f",processedVolt,temp);
                	//printMsg("photo Period: %f\n",photoPeriod);
                	//float Frequency = 1/photoPeriod;
                	//float photoRPM = 60*4*2*Frequency;
                	//printMsg("RPM: %f\n",photoRPM);
                	//photoTimer=0;

                //}
