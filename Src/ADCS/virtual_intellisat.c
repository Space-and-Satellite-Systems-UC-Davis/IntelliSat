#include "virtual_intellisat.h"
#include <print_scan.h>
#include <globals.h>
#include "../peripherals/IMU/ASM330LHH.h"
#include "../peripherals/MAG/QMC5883L.h"
#include "../peripherals/PWRMON/INA226.h"
#include "../peripherals/SunSensors/sun_sensors.h"
#include "../system_config/RTC/rtc.h"
#include <Timers/timers.h>

/*################## ACTUATOR COMMANDS ##################*/

vi_hdd_command_status
vi_hdd_command(
	vi_sensor hdd,
    double throttle
){
	//TODO add hdd selection
	PWM_Channels pwm;
	pwm_setDutyCycle(pwm, throttle);
	return HDD_COMMAND_SUCCESS;
}

vi_control_coil_status
vi_control_coil(
	double command_x,
	double command_y,
	double command_z
){
	//TODO: Implement

	return VI_CONTROL_COIL_FAILURE;
}

/*################### SENSOR READINGS ###################*/

vi_get_epoch_status
vi_get_epoch(
    int *year,
    int *month,
    int *day,
    int *hour, //rtc get time
    int *minute,
    int *second
){
	//TODO: Implement
	// May need to change int parameter types to uint_8
	rtc_getTime(hour, minute, second);

	return GET_EPOCH_SUCCESS;
}

vi_get_curr_millis_status
vi_get_curr_millis(
	uint64_t *curr_millis
){
	*curr_millis = getSysTime();
	return GET_CURR_MILLIS_SUCCESS;
}

vi_get_angvel_status
vi_get_angvel(
	vi_sensor imuSensor,
    double *angvel_x,
    double *angvel_y,
    double *angvel_z
){
	enum IMU_SELECT imu_select = (imuSensor.choice == One) ? IMU0 : IMU1;
	set_IMU(imu_select);

	*angvel_x = imu_readGyro_X();
	*angvel_y = imu_readGyro_Y();
	*angvel_z = imu_readGyro_Z();
	return GET_ANGVEL_SUCCESS;
}

vi_get_mag_status
vi_get_mag(
	vi_sensor magSensor,
	double *mag_x,
	double *mag_y,
	double *mag_z
){
	//TODO add mag selection
	Mag_Selector mag_selected = magSensor.choice ? MAG0 : MAG1;
	mag_select(mag_selected);

	*mag_x = mag_read_X();
	*mag_y = mag_read_Y();
	*mag_z = mag_read_Z();

	return VI_GET_MAG_SUCCESS;
}

vi_get_css_status
vi_get_css(
    vi_sensor cssSensor,
    double *magnitude
){
	//TODO: Implement

	return VI_GET_CSS_FAILURE;
}

vi_get_temp_status 
vi_get_temp(
	vi_sensor tempSensor, 
	// TEMP tempSelect,		// selects which sensor on a component to read from
	double* temp
){
	//TODO: Implement

	switch (tempSensor.component) {
		case MAG:
			Mag_Selector mag_selected = tempSensor.choice ? MAG0 : MAG1;
			mag_select(mag_selected);
			
			*temp = mag_readTemp();
			break;
		case IMU:
			enum IMU_SELECT imu_selected = (tempSensor.choice == 0) ? IMU0 : IMU1;
			set_IMU(imu_selected);

			*temp = imu_readTemp();
			break;
		//case CSS:
			// CSS header file doesn't exist yet
		//case HDD:
			// HDD header file doesn't exist yet
		case TEMP:
			int tmp_address = (tempSensor.choice == 0) ? TMP0_ADDRESS : TMP1_ADDRESS;

			// Axis is placeholder, need to find the panel select variable for temp 1-4 from parameters
			switch (tempSensor.axis) {
				case (NZ):				// PAN0
					*temp = temp_sensor_getTemp(PAN0_GPIO, PAN0_SCL_PIN, PAN0_SDA_PIN, tmp_address);
					break;
				case (PX): 				// PAN1
					*temp = temp_sensor_getTemp(PAN1_GPIO, PAN1_SCL_PIN, PAN1_SDA_PIN, tmp_address);
					break;
				case (PY): 				// PAN2
					*temp = temp_sensor_getTemp(PAN2_GPIO, PAN2_SCL_PIN, PAN2_SDA_PIN, tmp_address);
					break;
				case (PZ): 				// PAN3
					*temp = temp_sensor_getTemp(PAN3_GPIO, PAN3_SCL_PIN, PAN3_SDA_PIN, tmp_address);
					break;
				default:
					return VI_GET_TEMP_FAILURE;
			}

			break;

		case SOL:
			TEMP_SENSOR sensor = (!tempSensor.choice) ? PANEL0 : PANEL1;

			PANELS panel = PANEL0;
			switch (tempSensor.axis) {
				case NZ:
					// do nothing
					break;
				case PX:
					panel = PANEL1;
					break;
				case PY:
					panel = PANEL2;
					break;
				case PZ:
					panel = PANEL3;
					break;
				case NX:
					panel = PANEL4;
					break;
				case NY:
					panel = PANEL5;
					break;
				default:
					return VI_GET_TEMP_FAILURE;
			}

			*temp = sun_sensors_readTemp(panel, tempSensor.choice);

			break;

		default:
			return VI_GET_TEMP_FAILURE;
	}

	return VI_GET_TEMP_SUCCESS;
}

vi_get_coils_current_status 
vi_get_coils_current(
	double* currentX,
	double* currentY, 
	double* currentZ
){
	//TODO: Implement

	return VI_GET_COILS_CURRENT_FAILURE;
}

vi_get_solar_panel_current_status
vi_get_solar_panel_current(
	vi_sensor sp,
	double* current
){
	//TODO: Implement
	if (sp.component != SOL) return VI_GET_SOLAR_PANEL_CURRENT_FAILURE;


	// need to double check if these axes are correct
	PANELS panel = PANEL0;
	switch (sp.axis) {
		case NZ:
			// do nothing
			break;
		case PX:
			panel = PANEL1;
			break;
		case PY:
			panel = PANEL2;
			break;
		case PZ:
			panel = PANEL3;
			break;
		case NX:
			panel = PANEL4;
			break;
		case NY:
			panel = PANEL5;
			break;
		default:
			return VI_GET_SOLAR_PANEL_CURRENT_FAILURE;
	}
	
	*current = sun_sensors_readCurrent(panel);

	return VI_GET_SOLAR_PANEL_CURRENT_SUCCESS;
}

/*###################### CONSTANTS ######################*/

vi_get_constant_status
vi_get_sensor_calibration(
	vi_sensor sensor,
	float *offset,
	float *scalar,
	float *filter_constant
){
	//TODO: Implement

	return GET_CONSTANT_FAILURE;
}

vi_get_constant_status
vi_get_sensor_status(
	vi_sensor sensor,
	int *sensor_status
){
	//TODO: Implement

	return GET_CONSTANT_FAILURE;
}

vi_get_TLE_status
vi_get_TLE(
	char *tle_line1,
	char *tle_line2
){
	//TODO: Implement

	return GET_TLE_FAILURE;
}

int vi_get_experiment_generation(){
	//TODO: Implement

	return 0;
}

void vi_increment_experiment_generation(){
	//TODO: Implement

}

int vi_get_detumbling_generation(){
	//TODO: Implement

	return 0;
}

void vi_increment_detumbling_generation(){
	//TODO: Implement

}

int vi_get_determination_generation(){
	//TODO: Implement

	return 0;
}

void vi_increment_determination_generation(){
	//TODO: Implement

}

vi_delay_ms_status
vi_delay_ms(
    int ms
){
	uint64_t current = getSysTime();
	delay_ms(ms);
	if (getSysTime() >= current + (uint64_t)(ms)){
		return VI_DELAY_MS_SUCCESS;
	} else {
		return VI_DELAY_MS_FAILURE;
	}
}

void vi_print (const char* message){
	printMsg(message);
}




