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


typedef enum {
	ADCS_DETUMBLE,
	ADCS_HDD_EXP_ANGVEL,
	ADCS_HDD_EXP_TRIAD,
    ADCS_HDD_EXP_RAMP,
    ADCS_TESTING,
    ADCS_COILS_TESTING,
    ADCS_HDD_TESTING,
    ADCS_ROTISSERIE
} adcs_mode; //TODO: full list

typedef enum {
   ADCS_MAIN_SUCCESS,
   ADCS_MAIN_UNKNOWN,
   ADCS_MAIN_ECLIPSE,
   ADCS_MAIN_HDD_ERR,
   ADCS_MAIN_IMU_ERR,
   ADCS_MAIN_DETUMBLE_ERR,
   ADCS_MAIN_COILS_TESTING_ERR,
   ADCS_ROTISSERIE_ERR
} adcs_main_status; //TODO: full list


#ifdef __cplusplus
extern "C" {
#endif//__cplusplus

/**@brief Command ADCS to run a mode.
 *
 * Blocks during mode execution.
 *
 * @param mode The mode to run.
 *
 * @return The mode's status code.
 */
adcs_main_status
ADCS_MAIN(adcs_mode mode);


/**@brief Ask ADCS to recommend a mode to run.
 *
 * @return The recommended mode.
 */
adcs_mode
ADCS_recommend_mode();

#ifdef __cplusplus
}
#endif//__cplusplus


#endif//ADCS_H
