/* DIGITS.c
 *   by Tim Müller
 *
 * Created:
 *   21/04/2020, 11:46:37
 * Last edited:
 *   19/11/2020, 17:30:04
 * Auto updated?
 *   Yes
 *
 * Description:
 *   This file uses the NeuralNetwork class to try and predict the correct
 *   digit from a training set of hand-drawn digits.
**/

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include "CSV.h"
#include "NeuralNetwork.h"

/* Converts the expanded macros to string. */
#define STR_IMPL_(x) #x
/* Is able to convert macros to their string counterpart. */
#define STR(x) STR_IMPL_(x)

/* If left uncommented, reports and writes the costs. */
// #define PLOT 1
/* Percentage of data that will be used for training, the rest is for testing. */
#define TRAIN_RATIO 0.8
/* Number of iterations that the neural network will be trained on. */
#define TRAIN_ITERATIONS 20000
/* Learning rate of the Neural Network. */
#define TRAIN_ETA 0.005



/***** TOOLS *****/

/* Deallocates a list of arrays, up to the sfirst NULL it encounters. */
void destroy_array_list(size_t n_samples, double** list) {
    for (size_t i = 0; i < n_samples; i++) {
        if (list[i] == NULL) { break; }
        free(list[i]);
    }
    free(list);
}



/***** MAIN *****/

int main(int argc, char** argv) {
    // Check argument validity
    if (argc < 2) {
        printf("Usage: %s <path_to_digits_datafile> [params]\n", argv[0]);
        return -1;
    }

    // Parse any optional arguments using the NeuralNetwork parser
    parse_opt_args(argc - 2, argv + 2);

    #ifndef BENCHMARK
    printf("\n*** NEURAL NETWORK training DIGITS ***\n\n");

    // Call the list of used arguments
    printf("Configuration:\n");
    print_opt_args();
    printf(" - Number of epochs        : " STR(TRAIN_ITERATIONS) "\n");
    printf(" - Learning rate           : " STR(TRAIN_ETA) "\n");
    printf(" - Train / test ratio      : " STR(TRAIN_RATIO) "\n");
    printf("\n");

    printf("Loading digit dataset \"%s\"...\n", argv[1]);
    #endif
    
    // Try to open the file
    FILE* data = fopen(argv[1], "r");
    if (data == NULL) {
        fprintf(stderr, "ERROR: could not open file: %s\n", strerror(errno));
        exit(errno);
    }

    // The resulting linkedlist of matrices
    int n_samples = -1;
    int n_classes = -1;
    int sample = -1;
    double** digits = NULL;
    double** classes = NULL;

    // Loop through all elements (numbers enclosed in ',')
    int num;
    int status = get_num(data, &num);
    int i;
    for (i = 0; status > 0; i++) {
        int i_row = (i - 2) % 65;
        if (i == 0) {
            // Fill the number of samples first so we can allocate the array
            n_samples = num;
            digits = malloc(sizeof(double*) * n_samples);
            classes = malloc(sizeof(double*) * n_samples);
            for (int i = 0; i < n_samples; i++) {
                digits[i] = NULL;
                classes[i] = NULL;
            }
        } else if (i == 1) {
            // Fill in the number of classes for the output layer
            n_classes = num;
        } else if (i_row == 0) {
            // First every 65: new sample
            sample++;
            // Make sure the num is within range
            if (num >= n_classes) {
                destroy_array_list(n_samples, digits);
                destroy_array_list(n_samples, classes);
                fclose(data);
                fprintf(stderr, "ERROR: (%u:%u): given class is too high (%d > %d)\n",
                        row, col, num, n_classes - 1);
                return -1;
            }
            // Create an empty array in the digits class
            digits[sample] = malloc(sizeof(double) * 64);

            // Allocate an output array and set the correct index to 1
            classes[sample] = malloc(sizeof(double) * n_classes);
            for (int i = 0; i < n_classes; i++) {
                classes[sample][i] = i == num ? 1.0 : 0.0;
            }
        } else {
            // One of the 64 datapoints: fill in the matrix
            if (num > 16) {
                destroy_array_list(n_samples, digits);
                destroy_array_list(n_samples, classes);
                fclose(data);
                fprintf(stderr, "ERROR: (%u:%u): given datapoint is too high for pixel value (%d > 16)\n",
                        row, col, num);
                return -1;
            }
            digits[sample][i_row - 1] = num;
        }
        
        status = get_num(data, &num);
    }
    // Stop if an error occured
    if (status < 0) {
        destroy_array_list(n_samples, digits);
        destroy_array_list(n_samples, classes);
        fclose(data);
        return -1;
    }
    
    // Make sure n_samples and n_classes are set
    if (n_samples == -1) {
        fclose(data);
        fprintf(stderr, "ERROR: (%u:%u): number of samples not specified\n",
                row, col);
        return -1;
    }
    if (n_classes == -1) {
        free(digits);
        fclose(data);
        fprintf(stderr, "ERROR: (%u:%u): number of classes not specified\n",
                row, col);
        return -1;
    }

    // Throw a complaint about too few data if that's the case
    if (sample != n_samples - 1) {
        destroy_array_list(n_samples, digits);
        destroy_array_list(n_samples, classes);
        fclose(data);
        fprintf(stderr, "ERROR: (%u:%u): not enough samples provided (got %d, expected %d)\n",
                row, col, sample + 1, n_samples);
        return -1;
    } else if ((i - 2) % 65 != 0) {
        destroy_array_list(n_samples, digits);
        destroy_array_list(n_samples, classes);
        fclose(data);
        fprintf(stderr, "ERROR: (%u:%u): not enough datapoints provided for last sample (got %d, expected 64)\n",
                row, col, (i - 2) % 65);
        return -1;
    }

    #ifndef BENCHMARK
    printf("Done loading (loaded %d samples)\n\n", n_samples);
    
    printf("Training network...\n");
    #endif

    // Create training and testing subsets of the digits and classes matrix
    #ifndef BENCHMARK
    printf("  Splitting test and train sets...\n");
    #endif
    size_t training_size = n_samples * TRAIN_RATIO;
    size_t testing_size = n_samples - training_size;
    double** digits_train = digits;
    double** digits_test = digits + training_size;
    double** classes_train = classes;
    double** classes_test = classes + training_size;

    // Create a new neural network
    #ifndef BENCHMARK
    printf("  Initializing Neural Network...\n");
    #endif
    size_t hidden_layer_nodes[] = {20};
    neural_net* nn = create_nn(64, 1, hidden_layer_nodes, n_classes);

    // Train the neural network for ITERATIONS iterations
    #ifndef BENCHMARK
    struct timeval start_ms, end_ms;
    clock_t start, end;
    #endif
    #ifdef PLOT
    printf("  Training...\n");
    gettimeofday(&start_ms, NULL);
    start = clock();
    double* costs = nn_train_costs(nn, training_size, digits_train, classes_train, TRAIN_ETA, TRAIN_ITERATIONS);
    end = clock();
    gettimeofday(&end_ms, NULL);
    printf("  Time taken: %f seconds / CPU time taken: %f seconds\n",
           ((end_ms.tv_sec - start_ms.tv_sec) * 1000000 + (end_ms.tv_usec - start_ms.tv_usec)) / 1000000.0,
           (end - start) / (double) CLOCKS_PER_SEC);
    printf("  Writing costs...\n\n");
    // Write the costs for plotting
    write_costs(costs, TRAIN_ITERATIONS);
    #else
    #ifndef BENCHMARK
    printf("  Training...\n");
    gettimeofday(&start_ms, NULL);
    start = clock();
    #endif
    nn_train(nn, training_size, digits_train, classes_train, TRAIN_ETA, TRAIN_ITERATIONS);
    #ifndef BENCHMARK
    end = clock();
    gettimeofday(&end_ms, NULL);
    printf("  Time taken: %f seconds / CPU time taken: %f seconds\n\n",
           ((end_ms.tv_sec - start_ms.tv_sec) * 1000000 + (end_ms.tv_usec - start_ms.tv_usec)) / 1000000.0,
           (end - start) / (double) CLOCKS_PER_SEC);
    #endif
    #endif

    #ifndef BENCHMARK
    printf("Validating network...\n");

    // Test the network
    size_t last_nodes = nn->nodes_per_layer[nn->n_layers - 1];
    double outputs[testing_size * last_nodes];
    nn_forward(nn, testing_size, outputs, digits_test);

    // Flatten the results
    flatten_output(testing_size, last_nodes, outputs);

    // Compute the accuracy
    double accuracy = compute_accuracy(testing_size, last_nodes, outputs, classes_test);
    printf("  Network accuracy: %f\n\n", accuracy);
    #endif
    
    // Cleanup
    #ifndef BENCHMARK
    printf("Cleaning up...\n");
    #endif
    destroy_array_list(n_samples, digits);
    destroy_array_list(n_samples, classes);
    #ifdef PLOT
    free(costs);
    #endif
    destroy_nn(nn);
    fclose(data);

    #ifndef BENCHMARK
    printf("Done.\n\n");
    #endif
}
