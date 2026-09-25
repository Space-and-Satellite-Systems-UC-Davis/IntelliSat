/*
 * print_scan.h
 *
 * 	- May 11, 2023 (Creation)
 * 		Author : Darsh
 * 		Log    : Created the printMsg function
 */

#ifndef REALOP1_PRINT_SCAN_H_
#define REALOP1_PRINT_SCAN_H_

#include <stdbool.h>

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
 * @returns The length of the formatted message. Only the first 127 characters are printed.
 */
int printMsg(const char *message, ...);

/**
 * Reads one character from the console if one has arrived. Does not wait.
 *
 * @param c Where the character is stored
 *
 * @returns true if a character was read
 */
bool scanChar(char *c);

#endif // REALOP1_PRINT_SCAN_H_
