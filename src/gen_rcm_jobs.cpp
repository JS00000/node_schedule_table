#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <string.h>
#include "queue.h"
#include "functions.h"
#include "rcm.h"

#define SIZE 512         // number of rows and cols of sparse array


int* CuthillMckee(int* matrix) {
    int* degrees = degreesCalculation(matrix, SIZE);  // degrees points to a vector with the degree of each row

    queue* Q = queueInit();                            // Init the queue
    int* R = (int*) malloc(SIZE * sizeof(int));        // Allocate memory for the permutations array
    if(R == NULL) {
        printf("ERROR: malloc fail");
        exit(1);
    }
    int Rsize=0;                                       // The num of nodes in R

    int* notVisited = (int*) malloc(SIZE*sizeof(int)); // Allocate memory and init notVisited array
    if(notVisited == NULL) {
        printf("ERROR: malloc fail");
        exit(1);
    }

    // init notVisited array
    for (int i = 0; i < SIZE; i++) {
        *(notVisited+i) = 1;
    }

    while(Rsize != SIZE) {
        int minDegreeIndex = 0;  // The pos of min degree node in matrix
        int minDegree = SIZE+10; // A node can not have degree > SIZE

        // find the min degree
        for (int i = 0; i < SIZE; i++) {
            if(degrees[i] < minDegree && notVisited[i] == 1) {
                minDegreeIndex = i;
                minDegree = degrees[i];
            }
        }

        queueAdd(Q, minDegreeIndex);      // add in Q
        notVisited[minDegreeIndex] = 0;   // This node become visited

        while(!(Q->empty)) {
            int* currentIndex = (int*) malloc(sizeof(int));
            if(currentIndex == NULL) {
                printf("ERROR: malloc fail");
                exit(1);
            }
            queueDel(Q, currentIndex);
            int* neighbors = (int*) malloc(degrees[*currentIndex] * sizeof(int)); // array of neighbors
            if(neighbors == NULL) {
                printf("ERROR: malloc fail");
                exit(1);
            }
            int neighborsCounter=0;  // the num of neighbors

            // find all not visited neighbors
            for (int i = 0; i < SIZE; i++) {
                if(i != *currentIndex && *(matrix+(*currentIndex)*SIZE+i)==1 && notVisited[i]==1) {
                    neighbors[neighborsCounter++] = i; // add the neighbor in the array
                    notVisited[i] = 0; // and the node become visited
                }
            }

            // sort the neighbors by degree
            sortByDegree(neighbors, degrees, neighborsCounter); // sort the neighbors by degree

            // add the sorted neighbor in Q
            for (int i = 0; i < neighborsCounter; i++) {
                queueAdd(Q, neighbors[i]);
            }

            // add the current node in permutation array R
            R[Rsize++] = *currentIndex;
            free(currentIndex);
            free(neighbors);
        }
    }

    queueDelete(Q);
    free(notVisited);
    free(degrees);


    return R;
}

//  Reverse  CuthillMckee Result
int* ReverseCuthillMckee(int* matrix) {
    int* rcm = CuthillMckee(matrix);

    int n = SIZE;

    if(SIZE % 2 == 0)
        n -= 1;

    n = n / 2;

    for (int i = 0; i <= n; i++) {
        swap(&rcm[SIZE - 1 - i], &rcm[i]);
    }

    return rcm;
}


void init_matrix(int* matrix, int size, double sparsity_limit) {

    srand(time(NULL));

    int non_zeros = (size*size) - (size*size*sparsity_limit);
    int sum = 0, randX=0, randY=0;

    memset(matrix, 0, SIZE * SIZE * sizeof(int));
    // add self loop
    for (int i = 0; i < size; i++) {
        matrix[i*size+i] = 1;
    }

    for (int i = 0; i < non_zeros - size; i+=2) {
      do {
        randX = rand() % size;
        randY = rand() % size;
      } while(randX == randY);
      *(matrix+randX*size+randY) = 1;
      *(matrix+randY*size+randX) = 1;
      sum += 2;
    }
    double sparsity = 1.0 - ((double)sum)/((double)(size*size));
    printf(" >> Sparsity: %lf\n", sparsity);
    printf(" >> Nonzeros: %d\n", non_zeros);

}


// matrix: size * size
void write_rcm_jobs(int* matrix, int size, int grid_rows, int grid_cols, int id, char* file_path_prefix) {
    int grid_size = grid_rows*grid_cols;
    int block_size = ceil(double(size) / (grid_size));
    char file_name[50];
    sprintf(file_name, "%s/jobs_%dx%d_%d.txt", file_path_prefix, grid_size, grid_size, id);
    printf("write_rcm_jobs -> %s\n", file_name);
    FILE* file = fopen(file_name, "w");
    if(file == NULL)
        exit(0);
    fprintf(file, "%d %d\n", grid_size, grid_size);
    for (int gi = 0; gi < grid_size; gi++) {
        // check if any nonezero element is exist
        for (int gj = 0; gj < grid_size; gj++) {
            bool flag = 0;
            for (int i = gi * block_size; i < (gi + 1) * block_size && i < size; i++) {
                for (int j = gj * block_size; j < (gj + 1) * block_size && j < size; j++) {
                    if (matrix[i*size+j] == 1) {
                        flag = 1;
                    }
                }
            }
            if (flag)
                fprintf(file, "1 ");
            else
                fprintf(file, "0 ");
        }
        fprintf(file, "\n");
    }
    fclose(file);
}


int main(int argc, char const *argv[]) {

    // Init the sparse array
    int *matrix = (int*) calloc(SIZE * SIZE, sizeof(int));
    int* R = (int*) malloc(SIZE*sizeof(int));
    if(matrix == NULL) {
        printf("ERROR: malloc fail");
        exit(1);
    }
    if(R == NULL) {
        printf("ERROR: malloc fail");
        exit(1);
    }

    for (int grid_scale = 2; grid_scale <= 8; grid_scale+=2) {
        for (int i = 1; i <= grid_scale * grid_scale / 2; i+=grid_scale/2) {
            double sparsity = 1 - i * 2 / 1000.0;
            init_matrix(matrix, SIZE, sparsity);
            R = ReverseCuthillMckee(matrix);
            // generate 4x4 node jobs
            int* new_matrix = reorder(matrix, R, SIZE);
            char file_path_prefix[20] = "./input/rcm_jobs";
            write_rcm_jobs(new_matrix, SIZE, grid_scale, grid_scale, i, file_path_prefix);
        }
    }


    free(matrix);
    free(R);

    return 0;
}
