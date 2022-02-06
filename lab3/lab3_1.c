#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
typedef struct Matrix
{
    double *data;
    int sizeR;
    int sizeC;
} Matrixs;
void arrays_reservation_Matrixs(Matrixs *mat, int element_size)
{
    mat->data = (double *)malloc(sizeof(double *) * element_size);
}

void Readfile(char *filename, Matrixs *mat)
{
    int i = 0;
    int temp1, temp2;
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error can't open file\n");
    }
    fscanf(fp, "%d %d", &mat->sizeR, &mat->sizeC);
    arrays_reservation_Matrixs(mat, (mat->sizeC) * (mat->sizeR));
    for (i = 0; i < (mat->sizeC) * (mat->sizeR); i++)
    {
        fscanf(fp, "%lf", &mat->data[i]);
    }
    fclose(fp);
}
int main(int argc, char **argv)
{
    int world_size, world_rank;
    int i, buf = 0, *pt;
    int *sum;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    MPI_Finalize();
}