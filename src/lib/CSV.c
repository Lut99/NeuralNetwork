/* CSV.c
 *   by Lut99
 *
 * Created:
 *   19/11/2020, 17:26:23
 * Last edited:
 *   19/11/2020, 17:28:33
 * Auto updated?
 *   Yes
 *
 * Description:
 *   <Todo>
**/

#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <limits.h>

#include "CSV.h"


/* Fetches a number from the file and returns it in the given num argument. If something goes wrong, prints an error and returns -1. If EOF is reached, it return 0. Otherwise, 1 is returned, indicating that more is to be read. */
int get_num(FILE* f, int* num) {
    // Get character-by-character until comma, newline or EOF
    int value = 0;
    bool comment = false;
    bool seen_digit = false;
    while (true) {
        char c = fgetc(f);

        // Fetch an element into the correct buffer position
        if (c == EOF) {
            if (!feof(f)) {
                fprintf(stderr, "ERROR: get_num: (%u:%u): could not read from file: %s\n",
                        row, col, strerror(errno));
                return -1;
            }

            // Check if we have got a number
            if (!seen_digit) {
                return 0;
            }

            // Otherwise, return the value
            (*num) = value;
            return 1;
        }
        
        if (c == '#') {
            // Set comment mode to skip all until newline
            comment = true;
        } else if ((!comment && c == ',') || c == '\n') {
            if (c == '\n') {
                // Reset the comment line & increment row
                comment = false;
                row++;
                col = 0;

                // If nothing's found, just continue
                if (!seen_digit) { continue; }
            } else {
                // Check if we have got a number
                if (!seen_digit) {
                    fprintf(stderr, "ERROR: get_num: (%u:%u): no number given\n",
                            row, col);
                    return -1;
                }
            }

            // Otherwise, update col and return the value
            col++;
            (*num) = value;
            return 1;
        } else if (!comment && c >= '0' && c <= '9') {
            seen_digit = true;
            // Make sure value will not overflow
            int ic = (int)(c - '0');
            if (value > INT_MAX / 10 || value * 10 > INT_MAX - ic) {
                fprintf(stderr, "ERROR: get_num: (%u:%u): integer overflow occured\n",
                        row, col);
                return -1;
            }

            // Update the value
            value = value * 10 + ic;
        } else if (!comment && c != ' ' && c != '\t') {
            fprintf(stderr, "ERROR: get_num: (%u:%u): illegal character '%c'\n",
                    row, col, c);
            return -1;
        }

        // Don't forget to update col
        col++;
    }
}

/* Cleans a given string by removing all spaces and # and everything after (as comment). This is done in-place, so after this operation the string will be shorter or the same size as it was before. */
void clean_input(char* input) {
    int write_i = 0;
    for (int i = 0; input[i] != '\0'; i++) {
        char c = input[i];
        if (c == '#') {
            // Immediately done
            break;
        } else if (c != ' ') {
            // Write this char to the write_i and update it
            input[write_i] = input[i];
            write_i++;
        }
    }

    // Set zero-termination
    input[write_i] = '\0';
}

/* Writes the data to a .dat file so GNUplot can plot it later. */
void write_costs(double* costs, size_t size) {
    FILE* dat = fopen("./nn_costs.dat", "w");
    fprintf(dat, "# Iteration / Costs\n");
    for (size_t i = 0; i < size; i++) {
        fprintf(dat, "%ld %f\n", i, costs[i]);
    }
    fclose(dat);
}
