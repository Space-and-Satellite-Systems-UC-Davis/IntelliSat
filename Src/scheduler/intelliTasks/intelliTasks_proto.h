#ifndef INTELLITASKS_PROTO_H_
#define INTELLITASKS_PROTO_H_
#include "globals.h"

/* Scheduling methods */

bool low_pwr_time();
bool detumble_time();
bool comms_time();
bool experiment_time();
bool ecc_time();
bool pizza_time();
bool idle_time();



/* Configure methods */

void config_low_pwr();
void config_detumble();
void config_comms();
void config_experiment();
void config_ecc();
void config_pizza();
void config_idle();


/* Run methods */

void low_pwr();
void run_detumble();
void comms();
void experiment();
void ecc();
void pizza();
void idle();


/* Clean methods */

void clean_low_pwr();
void clean_detumble();
void clean_comms();
void clean_experiment();
void clean_ecc();;
void clean_pizza();void clean_idle();


#endif//INTELLITASKS_PROTO_H_
