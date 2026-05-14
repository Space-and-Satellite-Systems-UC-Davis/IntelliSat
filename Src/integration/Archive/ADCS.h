/**@file ADCS.h
 *
 * @brief Intellisat's interface to the ADCS software.
 *
 * @author Jacob Tkeio (jacobtkeio@gmail.com)
 *
 * @date 7/30/2023
 */

#ifndef ADCS_H
#define ADCS_H

#include "adcs_math/matrix.h"
#include "virtual_intellisat.h"


//State variables
mat3 realop_attitude;


typedef enum {
	ADCS_DETUMBLE,
	ADCS_HDD_EXPERIMENT,
	ADCS_MRW_EXPERIMENT
} adcs_mode;        //TODO: full list

typedef enum {
   ADCS_MAIN_SUCCESS,
   ADCS_MAIN_UNKNOWN,
   ADCS_MAIN_ECLIPSE,
   ADCS_MAIN_HDD_ERR,
   ADCS_MAIN_MRW_ERR,
   ADCS_MAIN_IMU_ERR
} adcs_main_status; //TODO: full list

// adcs_main_status
// ADCS_MAIN(adcs_mode mode);

// adcs_mode
// ADCS_recommend_mode();


#endif//ADCS_H





