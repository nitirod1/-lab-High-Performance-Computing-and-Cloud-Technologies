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
void select_process_read(char *filename, int process_n, Matrixs *mat)
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == process_n)
    {
        Readfile(filename, mat);
    }
}
int Element_Per_Process(int size_element)
{
    int SIZE;
    MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
    return size_element / SIZE;
}

void control_process()
{
}
void send_size_mat(int process_n, Matrixs *mat)
{
    MPI_Bcast(&mat->sizeC, 1, MPI_INT, process_n, MPI_COMM_WORLD);
    MPI_Bcast(&mat->sizeR, 1, MPI_INT, process_n, MPI_COMM_WORLD);
}
void m_to_r_size(Matrixs *m1, Matrixs *m2)
{
    m2->sizeC = m1->sizeC;
    m2->sizeR = m1->sizeR;
}
void Bcast_process(Matrixs *m1, Matrixs *m2, int process_n)
{
    send_size_mat(process_n, m1);
    m_to_r_size(m1, m2);
}
void deliver()
{
    Matrixs mat1, mat2, recv1, recv2;
    int rank,elem_p_r1,elem_p_r2;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    select_process_read("matAsmall.txt", 0, &mat1);
    select_process_read("matBsmall.txt", 1, &mat2);
    Bcast_process(&mat1, &recv1, 0);
    Bcast_process(&mat2, &recv2, 1);
    MPI_Barrier(MPI_COMM_WORLD);
    elem_p_r1=Element_Per_Process(recv1.sizeC*recv1.sizeR);
    elem_p_r2=Element_Per_Process(recv2.sizeC*recv2.sizeR);
    printf("%d %d %d\n",rank,elem_p_r1,elem_p_r2);
    // MPI_Scatter(sum,5,MPI_INT,recv,5,MPI_INT,0,MPI_COMM_WORLD);
}
int main(int argc, char **argv)
{
    MPI_Init(&argc, &argv);
    deliver();
}