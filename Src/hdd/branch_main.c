#include "../system_config/Timers/timers.h"
#include "../globals.h"
#include "pid.h"
#include "../peripherals/IMU/ASM330LHH.h"

#define ESC_PERIOD_MICROSEC      5000
#define ESC_MIN_DUTYCYCLE_PERCNT 10
#define ESC_MID_DUTYCYCLE_PERCNT 15
#define ESC_MAX_DUTYCYCLE_PERCNT 20

#define PID_KP 0.3f
#define PID_KI 0.0f
#define PID_KD 0.0f

#define IMU_TIME 0.003f
#define PID_TIME 0.0003f //10 times faster than IMU time

#define PID_TAU 0.8f //value between 0 and 1, low pass filter

#define PID_INTEGRATOR_MIN 0.0f
#define PID_INTEGRATOR_MAX 0.0f

#define PID_OUTPUT_MIN -5.0f
#define PID_OUTPUT_MAX 5.0f

#define PID_SETPOINT 0.0f

#define GYRO_RATE 3333
#define GYRO_SCALE 4000



extern bool button1;

void branch_main(){

	op1_imu_gyro_ctrl(GYRO_RATE, GYRO_SCALE);
	init_pwm_timer(ESC_PERIOD_MICROSEC); //parameter is microseconds, on pd12

	PWM_TIMER_ON();
    set_duty_cycle(ESC_MAX_DUTYCYCLE_PERCNT); //parameter
    nop(80000000);    // wait for arming  <-- This line waits for 1 sec
	//Runs infinitely
	PIDController* IMUPID;
	//Parameters: PID, Kp, Ki, Kd, Time(10 times faster than IMU measurement cycle, in SECONDS)
	PIDController_SetTerms(IMUPID, PID_KP, PID_KI, PID_KD, PID_TIME);//Last parameter, time, should be 10 times less than the IMU period
	//Parameters: PID
	PIDController_Init(IMUPID);
	//Parameters: PID, low pass filter constant tau (between 0 and 1), integrator min, integrator max, output min, output max
	PIDController_SetLimits(IMUPID, PID_TAU, PID_INTEGRATOR_MIN, PID_INTEGRATOR_MAX, PID_OUTPUT_MIN, PID_OUTPUT_MAX);

	while(1){
		//To activate on button

		if(button1){
			button1 = false;
		}
		nop(800000);//waits 10 milliseconds
		//Clockwise increases duty cycle percentage while Counterclockwise decreases duty cycle percentage
		set_duty_cycle(ESC_MID_DUTYCYCLE_PERCNT + (int)PIDController_Update(IMUPID, PID_SETPOINT, (float)op1_imu_read_gyro_z()));
	}

}
