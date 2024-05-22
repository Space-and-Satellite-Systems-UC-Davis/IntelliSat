/*
 * print_scan.h
 *
 * 	- May 11, 2023 (Creation)
 * 		Author : Darsh
 * 		Log    : Created the printMsg function
 */

#ifndef REALOP1_PRINT_SCAN_H_
#define REALOP1_PRINT_SCAN_H_

/**
 * Initializes the internal systems needed for Console Printing
 *
 * @param none
 *
 * @returns none
 */
void printer_init();

/**
 * Prints a variable length string to the console
 *
 * @param message The string (character array) to be printed
 *
 * @returns None
 */
int printMsg(const char *message, ...);

#endif // REALOP1_PRINT_SCAN_H_
