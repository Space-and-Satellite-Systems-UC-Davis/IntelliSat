#include "virtual_intellisat.h"
#include <print_scan.h>
#include <globals.h>
#include "../peripherals/IMU/ASM330LHH.h"
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
    int *hour,
    int *minute,
    int *second
){
	//TODO: Implement

	return GET_EPOCH_FAILURE;
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
	enum IMU_SELECT imu_select = (imuSensor.choice == ONE) ? IMU0 : IMU1;

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

	return VI_GET_MAG_FAILURE;
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
	double* temp
){
	//TODO: Implement

	return VI_GET_TEMP_FAILURE;
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
	return VI_GET_SOLAR_PANEL_CURRENT_FAILURE;
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




