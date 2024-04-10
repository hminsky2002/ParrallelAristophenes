#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
 
 
int main(int argc, char* argv[])
{
    int val = 12;
    int my_rank, comm_sz, elements_per_process;
    MPI_Status status;

    MPI_Init(&argc, &argv);
 
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
 
    if (my_rank == 0) {
        int start, i;
        elements_per_process = val/comm_sz;
 
        
        if (comm_sz > 1) {
         
            for (i = 1; i < comm_sz - 1; i++) {
                start = i * elements_per_process;
 
                MPI_Send(&elements_per_process,
                         1, MPI_INT, i, 0,
                         MPI_COMM_WORLD);
                MPI_Send(&start,
                         1,
                         MPI_INT, i, 0,
                         MPI_COMM_WORLD);
            }
 
           
            int p = i * elements_per_process;
            int s = val - elements_per_process;
 
            MPI_Send(&s,
                     1, MPI_INT,
                     i, 0,
                     MPI_COMM_WORLD);
            MPI_Send(&p,
                     1,
                     MPI_INT, i, 0,
                     MPI_COMM_WORLD);
        }
 
        int sum = 0;
        for (i = 1; i < elements_per_process; i++)
            sum += i;
 
        int tmp;
        for (i = 1; i < comm_sz; i++) {
            MPI_Recv(&tmp, 1, MPI_INT,
                     MPI_ANY_SOURCE, 0,
                     MPI_COMM_WORLD,
                     &status);
            int sender = status.MPI_SOURCE;
 
            sum += tmp;
        }
 
        printf("Sum of array is : %d\n", sum);
    }
    else {
        int z, e;
        MPI_Recv(&e,
                 1, MPI_INT, 0, 0,
                 MPI_COMM_WORLD,
                 &status);
 

        MPI_Recv(&z, 1,
                 MPI_INT, 0, 0,
                 MPI_COMM_WORLD,
                 &status);
 
        int partial_sum = 0;
        for (int i = 1; i < e; i++)
            partial_sum += (z+i);
 
        MPI_Send(&partial_sum, 1, MPI_INT,
                 0, 0, MPI_COMM_WORLD);
    }
 
    MPI_Finalize();
 
    return 0;
}