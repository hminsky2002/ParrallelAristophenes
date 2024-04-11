/* File:     primes.c
 *
 * Purpose:  Implement parallel implementation of the sieve of Aristophenes
 *           algorithim for finding prime numbers from 2 to N
 *
 * Compile:  mpicc -g -std=c99 -Wall -lm -o  primes primes.c
 * Run:      mpiexec -n <number of processes> ./primes <Value of N>
 *
 * Input:    N, the limit(inclusive) to calculate prime numbers to
 * Output:   A file containing all prime numbers from 2 to N
 *
 *
 * Notes:
 *    1. Compile with flag -lm to accomodate ceil function used in caclulating
 *       limit
 *
 * Author: Harry Minsky, hm2536@nyu.edu
 *         Created for Mohammed Zahran's Parallel Computing Class
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <mpi.h>

// This method creates an array of size N with all values initialized to 1,
// to represent their potential prime-ness.
// It's used to generate the 'global' array which is scattered from and gathered
// to
bool *createBaseArray(int N)
{
    bool *base_array = (bool *)malloc(sizeof(bool) * N);
    for (int i = 0; i < N; i++)
    {
        base_array[i] = true;
    }
    return base_array;
}

// This method creates a sub array of size N with all values uninitialized
// These are the local arrays used by sub processes to calculate primeness
bool *createSubArray(int n)
{
    bool *subArray = (bool *)malloc(sizeof(bool) * n);
    return subArray;
}

int main(int argc, char *argv[])
{
    // Initialize input variable N, add 1 to get inclusive value

    int N = atoi(argv[1]) + 1;

    int my_rank, comm_sz;

    // Initialize variables for tracking computation time
    double t1, t2;

    // Initialize MPI space
    MPI_Init(NULL, NULL);

    // Gain access to local process number and overall number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    // These values are used to calculate
    // 1) The size/count of values each sub process will be processing
    // 2) The offset(if any) if the number of values is either not evenly divisible
    //    by the number of processes, or the number of processes is greater than the
    //    number of values.
    int subset_size, offset = 0;

    // Case where number of values is less than number of processes
    if (N < comm_sz)
    {
        offset = comm_sz - N;
        subset_size = 1;
        N = comm_sz;
    }
    else
    {
        // If the number of values is not evenly divisible by the number of processes,
        // increase it by the necessary offset.
        // If it is evenly divisible, offset is 0 so the code still works. Hooray!
        offset = N % comm_sz;
        N += offset;
        subset_size = N / comm_sz;
    }

    bool *base_array = NULL;

    // If we are in the master process, allocate memory for the global array
    if (my_rank == 0)
    {
        t1 = MPI_Wtime();
        base_array = createBaseArray(N);
    }

    // In the sieve, we don't need to check any value greater than the square
    // root of the limit
    int limit = (int)ceil(sqrt(N));
    // Variable to calculate the numerical value of a given array value,
    // as index is lost on scatter
    int start;
    // Allocate local memory
    bool *sub_array = createSubArray(subset_size);

    // Scatter the global data from master process, recieve it
    // in sub processes. Zoom!
    MPI_Scatter(base_array, subset_size, MPI_C_BOOL, sub_array,
                subset_size, MPI_C_BOOL, 0, MPI_COMM_WORLD);

    // This loop runs for all values from 2 to the sqrt of N, checking if the values
    // in the processes array chunk are divisible by those values
    for (int i = 2; i < limit + 1; i++)
    {
        // Calculate the numerical value that corresponds to index 0
        // of the sub array for a process
        start = (subset_size * my_rank);

        // Do the sieve thing
        for (int j = 0; j < subset_size; j++)
        {
            if (start + j > 2 && (start + j) != i && ((start + j) % i) == 0)
            {
                sub_array[j] = false;
            }
        }
    }

    // Gather the data if the root process, send it if sub
    if (my_rank == 0)
    {
        MPI_Gather(sub_array, subset_size, MPI_C_BOOL, base_array, subset_size, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    }
    else
    {
        MPI_Gather(sub_array, subset_size, MPI_C_BOOL, NULL, subset_size, MPI_C_BOOL, 0, MPI_COMM_WORLD);
    }

    // If we're back to the master process, time to write out the data
    if (my_rank == 0)
    {
        t2 = MPI_Wtime();
        FILE *ptr;
        int count = 0;
        char filename[20];
        int originalN = atoi(argv[1]);
        sprintf(filename, "./%d.txt", originalN);
        ptr = fopen(filename, "w+");
        if (!ptr)
        {
            perror("File opening failed");
            return EXIT_FAILURE;
        }
        for (int i = 2; i < originalN; i++)
        {
            if (base_array[i])
            {
                fprintf(ptr, "%d ", i);
                count++;
            }
        }
        fclose(ptr);
        printf("Operation Complete. Time Elapsed: %f  \n", t2 - t1);
        printf("Number of primes found: %d  \n", count);
        printf("\n");
    }

    // Give the data back to god (computer)
    free(sub_array);
    free(base_array);
    MPI_Finalize();

    return 0;
}