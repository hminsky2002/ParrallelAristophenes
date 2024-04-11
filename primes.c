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

    double t1,t2;
    MPI_Init(NULL, NULL);
    
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    int subset_size, offset = 0;
    if(N < comm_sz){
        offset = comm_sz-N;
        subset_size = 1;
        N = comm_sz;
    }
    else{
        offset = N % comm_sz;
        N += offset;
        subset_size = N / comm_sz;
    }
    bool *base_array = NULL;
    if (my_rank == 0 || true) {
        t1 = MPI_Wtime(); 
        base_array = createBaseArray(N);
    }
    

    int limit = (int)ceil(sqrt(N));
    int start;
    bool *sub_array = createSubArray(subset_size);

    for(int i = 2; i < limit+1; i++){
        start = (subset_size*my_rank);

        MPI_Scatter(base_array, subset_size, MPI_C_BOOL, sub_array,
            subset_size,  MPI_C_BOOL, 0, MPI_COMM_WORLD);

        for(int j = 0; j < subset_size; j++){
            if(start+j > 2  && (start+j) != i && ((start+j) % i) == 0 ){
                 sub_array[j] = false;
            }
        }
        if(my_rank == 0){
             MPI_Gather(sub_array, subset_size,  MPI_C_BOOL, base_array, subset_size, MPI_C_BOOL, 0, MPI_COMM_WORLD);
        }
        else{
            MPI_Gather(sub_array, subset_size,  MPI_C_BOOL, NULL, subset_size, MPI_C_BOOL, 0, MPI_COMM_WORLD);
        }

    }
    
    if (my_rank == 0) {
        t2 = MPI_Wtime();
        printf("Array contents \n");
        for (int i=2; i < N - offset; i++){
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