#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Matrix
{
    double *data;
    int commute;
    int sizeR;
    int sizeC;
} Matrixs;
void arrays_reservation_Matrixs(Matrixs *mat, int element_size)
{
    mat->data = (double *)calloc(element_size, sizeof(double *));
}
void Readfile(char *filename, Matrixs *mat)
{
    int i = 0, SIZE = 0, element_size;
    int temp1, temp2;
    FILE *fp = fopen(filename, "r");
    MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
    if (fp == NULL)
    {
        printf("Error can't open file\n");
    }
    fscanf(fp, "%d %d", &mat->sizeR, &mat->sizeC);
    element_size = mat->sizeR * mat->sizeC;
    mat->commute = element_size % SIZE;
    element_size += mat->commute;
    arrays_reservation_Matrixs(mat, element_size + mat->commute);
    for (i = 0; i < element_size; i++)
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
        printf("file can't open %s\n", filename);
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

void send_size_mat(int process_n, Matrixs *mat)
{
    MPI_Bcast(&mat->sizeC, 1, MPI_INT, process_n, MPI_COMM_WORLD);
    MPI_Bcast(&mat->sizeR, 1, MPI_INT, process_n, MPI_COMM_WORLD);
    MPI_Bcast(&mat->commute, 1, MPI_INT, process_n, MPI_COMM_WORLD);
}
void m_to_r_size(Matrixs *m1, Matrixs *m2)
{
    m2->sizeC = m1->sizeC;
    m2->sizeR = m1->sizeR;
    m2->commute = m1->commute;
}
void Bcast_process(Matrixs *m1, Matrixs *m2, int process_n)
{
    send_size_mat(process_n, m1);
    m_to_r_size(m1, m2);
}
int check_mul(Matrixs *mat1, Matrixs *mat2)
{
    if (mat1->sizeC == mat2->sizeR)
    {
        return 1;
    }
    return -1;
}

void calculation(Matrixs *mat1, Matrixs *mat2,Matrixs *temp)
{
    int i, j, k, index_mat = 0;
    arrays_reservation_Matrixs(temp, mat1->sizeR * mat2->sizeC);
    if (check_mul(mat1, mat2))
    {
        for (i = 0; i < mat1->sizeR; i++)
        {
            for (j = 0; j < mat2->sizeC; j++)
            {
                for (k = 0; k < mat1->sizeC; k++)
                {
                    temp->data[index_mat] += (mat1->data[k + (i * mat1->sizeC)] * mat2->data[j + (k * mat2->sizeC)]);
                }
                index_mat++;
            }
        }
    }
    temp->sizeR = mat1->sizeR;
    temp->sizeC = mat2->sizeC;
    printf("sucess\n");
}

void deliver()
{
    Matrixs mat1, mat2, recv1, recv2,result;
    int rank, elem_p_r1, elem_p_r2, element_size, SIZE;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
    if (SIZE > 1)
    {
        select_process_read("matAsmall.txt", 0, &mat1);
        select_process_read("matBsmall.txt", 1, &mat2);
        Bcast_process(&mat1, &recv1, 0);
        Bcast_process(&mat2, &recv2, 1);
        MPI_Barrier(MPI_COMM_WORLD);
    }
    else{
        select_process_read("matAsmall.txt", 0, &mat1);
        select_process_read("matBsmall.txt", 0, &mat2);
        Bcast_process(&mat1, &recv1, 0);
        Bcast_process(&mat2, &recv2, 0);
    }
    element_size = mat1.sizeC * mat1.sizeR + mat1.commute;
    printf("before :%d\n",element_size);
    elem_p_r1 = Element_Per_Process(element_size);
    arrays_reservation_Matrixs(&recv1, element_size);
    element_size = mat2.sizeC * mat2.sizeR + mat2.commute;
    elem_p_r2 = Element_Per_Process(element_size);
    printf("after %d %d\n",recv2.sizeR,recv2.sizeC);
    arrays_reservation_Matrixs(&recv2, element_size);
    if(SIZE>1){
        MPI_Scatter(mat1.data, elem_p_r1, MPI_DOUBLE, recv1.data, elem_p_r1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Scatter(mat2.data, elem_p_r2, MPI_DOUBLE, recv2.data, elem_p_r2, MPI_DOUBLE, 1, MPI_COMM_WORLD);
    }
    else{
        MPI_Scatter(mat1.data, elem_p_r1, MPI_DOUBLE, recv1.data, elem_p_r1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
        MPI_Scatter(mat2.data, elem_p_r2, MPI_DOUBLE, recv2.data, elem_p_r2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    }
    calculation(&recv1, &recv2,&result);
    
    // printf("%d %d\n",result->sizeR,result->sizeC);
}
int main(int argc, char **argv)
{
    double StartTime, EndTime;
    StartTime = MPI_Wtime();
    MPI_Init(&argc, &argv);
    EndTime = MPI_Wtime();
    deliver();
}