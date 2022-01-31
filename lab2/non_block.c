#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int world_rank;
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
    fscanf(fp, "%d %d", &mat->sizeR, &mat->sizeC);
    arrays_reservation_Matrixs(mat, (mat->sizeC) * (mat->sizeR));
    for (i = 0; i < (mat->sizeC) * (mat->sizeR); i++)
    {
        fscanf(fp, "%lf", &mat->data[i]);
    }
    fclose(fp);
}
void writefile(char *filename, Matrixs *mat)
{
    FILE *fp = fopen(filename, "w");
    int i;
    if (fp == NULL)
    {
        printf("file can't open\n");
    }
    fprintf(fp, "%d %d", mat->sizeR, mat->sizeC);
    for (i = 0; i < mat->sizeR * mat->sizeC; i++)
    {
        if (i % mat->sizeC == 0)
        {
            fprintf(fp, "\n");
        }
        fprintf(fp, "%lf ", mat->data[i]);
    }
}

void showmatrix(Matrixs *mat)
{
    int i, j;
    printf("%d %d\n", mat->sizeR, mat->sizeC);
    for (i = 1; i <= mat->sizeR * mat->sizeC; i++)
    {
        printf("%lf ", mat->data[i - 1]);
        if (i % mat->sizeC == 0)
        {
            printf("\n");
        }
    }
}
int send_process(Matrixs *mat, int size)
{
    int i, index;
    int element_per_process = ((mat->sizeR) * (mat->sizeC)) / size;
    for (i = 1; i < size; i++)
    {
        index = element_per_process * i;
        MPI_Send(&element_per_process, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        MPI_Send(&mat->data[index], element_per_process, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
    }
    return element_per_process;
}
int recv_process(Matrixs *mat, int size)
{
    int i = 0, element_per_process, ready;
    MPI_Request req[2];
    MPI_Status status;
    MPI_Irecv(&element_per_process, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD, &req[0]);
    MPI_Wait(&req[0], &status);
    arrays_reservation_Matrixs(mat, element_per_process);
    MPI_Irecv(mat->data, element_per_process, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &req[1]);
    MPI_Wait(&req[1], &status);
    MPI_Test(&req[1], &ready, &status);
    if (ready)
    {
        return element_per_process;
    }
    return 0;
}
double *plus(Matrixs **matA, Matrixs **matB, int element_per_process)
{
    int i;
    double temp1, temp2;
    double *cal = (double *)malloc(sizeof(double *) * element_per_process);
    for (i = 0; i < element_per_process; i++)
    {
        cal[i] = (*matA)->data[i] + (*matB)->data[i];
    }
    return cal;
}
double *cal_matrix(Matrixs *matA, Matrixs *matB, int element_per_process)
{

    double *result = plus(&matA, &matB, element_per_process);
    return result;
}
void concat_result_by_master(Matrixs *mat, double *cal, int element_size, int size)
{
    int i, j, pos = 0;
    MPI_Request req;
    MPI_Status status;
    arrays_reservation_Matrixs(mat, mat->sizeR * mat->sizeC);
    for (j = 0; j < element_size; j++)
    {
        mat->data[pos++] = cal[j];
    }
    for (i = 1; i < size; i++)
    {
        int ready;
        double *calculated = malloc(element_size * sizeof(double));
        MPI_Irecv(calculated, element_size, MPI_DOUBLE, i, 3, MPI_COMM_WORLD, &req);
        MPI_Wait(&req, &status);
        MPI_Test(&req, &ready, &status);
        if (ready)
        {
            for (j = 0; j < element_size; j++)
            {
                mat->data[pos++] = calculated[j];
            }
        }
        else
        {
            printf("not recv\n");
        }
        free(calculated);
    }
}
void concat_result_by_slave(double *cal, int element_size)
{
    MPI_Send(&cal[0], element_size, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD);
}
void master_process(int size)
{
    Matrixs matA, matB, result;
    double *cal = 0;
    int element_per_process;
    Readfile("matAlarge.txt", &matA);
    Readfile("matBlarge.txt", &matB);
    element_per_process = send_process(&matA, size);
    element_per_process = send_process(&matB, size);
    cal = cal_matrix(&matA, &matB, element_per_process);
    result.sizeR = matA.sizeR;
    result.sizeC = matA.sizeC;
    concat_result_by_master(&result, cal, element_per_process, size);
    writefile("result_Large.txt", &result);
}
void slave_process(int size)
{
    Matrixs matA, matB, result;
    double *cal;
    int element_per_process;
    element_per_process = recv_process(&matA, size);
    element_per_process = recv_process(&matB, size);
    cal = cal_matrix(&matA, &matB, element_per_process);
    concat_result_by_slave(cal, element_per_process);
}
void deliver(int rank, int size)
{
    if (rank == 0)
    {
        master_process(size);
    }
    else
    {
        slave_process(size);
    }
}
int main(int argc, char **argv)
{
    int world_size;

    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    deliver(world_rank, world_size);
    MPI_Finalize();
    return 0;
}