/* CSV.h
 *   by Lut99
 *
 * Created:
 *   19/11/2020, 17:25:22
 * Last edited:
 *   19/11/2020, 17:28:42
 * Auto updated?
 *   Yes
 *
 * Description:
 *   This file contains function(s) that are useful for reading or writing
 *   to CSV files.
**/

#ifndef CSV_H
#define CSV_H

#include <stdio.h>

// Static track of the current row number while reading
static unsigned int row = 1;
// Static track of current column number while reading
static unsigned int col = 1;

/* Fetches a number from the file and returns it in the given num argument. If something goes wrong, prints an error and returns -1. If EOF is reached, it return 0. Otherwise, 1 is returned, indicating that more is to be read. */
int get_num(FILE* f, int* num);

/* Cleans a given string by removing all spaces and # and everything after (as comment). This is done in-place, so after this operation the string will be shorter or the same size as it was before. */
void clean_input(char* input);

/* Writes the data to a .dat file so GNUplot can plot it later. */
void write_costs(double* costs, size_t size);

#endif
