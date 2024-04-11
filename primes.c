#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <mpi.h>
 
//this method creates an array of size N with all values initialized to 1,
//to represent their potential prime-ness
bool *createBaseArray(int N) {
  bool *base_array = (bool *)malloc(sizeof(bool) * N);
  for (int i = 0; i < N; i++) {
    base_array[i] = true;
  }
  return base_array;
}

//this method creates a sub array of size N with all values uninitialized
bool *createSubArray(int n) {
  bool *subArray = (bool *)malloc(sizeof(bool) * n);
  return subArray;
}

int main(int argc, char* argv[])
{
    int N = atoi(argv[1])+1;
    
    int my_rank, comm_sz;

    MPI_Status status;

    double t1,t2;
    MPI_Init(NULL, NULL);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    bool *base_array = NULL;
    if (my_rank == 0 || true) {
        t1 = MPI_Wtime(); 
        base_array = createBaseArray(N);
    }
    int *send_counts = malloc(sizeof(int)*comm_sz);
    int *displs = malloc(sizeof(int)*comm_sz);
    if(N < comm_sz){
        for(int i = 0; i < comm_sz; i++){
            if(i <= N){
               send_counts[i] = 1;
            }
            else{
               send_counts[i] = 0;
            }
        }
        for(int i = 0; i < comm_sz; i++){
            displs[i]=0;
        }

    }
    else{
        int subset_offset = N % comm_sz;
        int trimmed_count = N - subset_offset;
        int set_size = trimmed_count/comm_sz;

        for(int i = 0; i < comm_sz; i++){
            if(i == (comm_sz-1) ){
                send_counts[i] = set_size+subset_offset;
            }
            else{
                send_counts[i] = set_size;
            }
        }
        displs[0] = 0;
        for(int i = 1; i < comm_sz; i++){
            displs[i]= displs[i-1] + send_counts[i-1];
        }
    }

    int limit = (int)ceil(sqrt(N));
    int start;
    bool *sub_array = createSubArray(send_counts[my_rank]);

    for(int i = 2; i < limit+1; i++){
        start = displs[my_rank];

        MPI_Scatterv(base_array, send_counts, displs, MPI_C_BOOL, sub_array,
            send_counts[my_rank],  MPI_C_BOOL, 0, MPI_COMM_WORLD);

        for(int j = 0; j < send_counts[my_rank]; j++){
            if(start+j > 2  && (start+j) != i && ((start+j) % i) == 0 ){
                 sub_array[j] = false;
            }
        }
        if(my_rank == 0){
             MPI_Gatherv(sub_array, send_counts[my_rank],  MPI_C_BOOL, base_array, send_counts,displs, MPI_C_BOOL, 0, MPI_COMM_WORLD);
        }
        else{
            MPI_Gatherv(sub_array, send_counts[my_rank],  MPI_C_BOOL, NULL, NULL, NULL,  MPI_C_BOOL, 0, MPI_COMM_WORLD);
        }

    }
    
    if (my_rank == 0) {
        t2 = MPI_Wtime();
        printf("Array contents \n");
        for (int i=0; i < N; i++){
            if(base_array[i]){
                printf("%d ",i);
            }
        }
            
        printf("\n");
        printf("__________________________________\n");
        printf("Time Elapsed: %f  \n",t2-t1);
    }

    MPI_Finalize();
    
    return 0;
    }