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
int Element_Per_Process(int size_element,int *size_process)
{
    int SIZE;
    MPI_Comm_size(MPI_COMM_WORLD,&SIZE);
    *size_process = SIZE;
    return size_element/SIZE;
    
}

double *calculation(Matrixs *recv1,Matrixs *recv2,int element_size){
    double  *result = malloc(sizeof(double)*element_size);
    int i;
    for(i=0;i<element_size;i++){
        result[i] = recv1->data[i]+recv2->data[i];
    }
    return result; 
}
void splitdata()
{
    int rank,i,size_process;
    int element_per_process;
    double *results =NULL;
    MPI_Status status;
    Matrixs mat1, mat2,recv1,recv2,result;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    if (rank == 0)
    {
        Readfile("matAsmall.txt", &mat1);
        Readfile("matBsmall.txt", &mat2);
        element_per_process = Element_Per_Process(mat1.sizeC*mat1.sizeR,&size_process);
        for(i =1;i<size_process;i++){
            MPI_Send(&element_per_process, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
        }
    }else{
        MPI_Recv(&element_per_process, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
    }
    arrays_reservation_Matrixs(&recv1,element_per_process);
    arrays_reservation_Matrixs(&recv2,element_per_process);
    MPI_Scatter(mat1.data,element_per_process,MPI_DOUBLE,recv1.data,element_per_process,MPI_DOUBLE,0,MPI_COMM_WORLD);
    MPI_Scatter(mat2.data,element_per_process,MPI_DOUBLE,recv2.data,element_per_process,MPI_DOUBLE,0,MPI_COMM_WORLD);
    result.data=calculation(&recv1,&recv2,element_per_process);
    if(rank == 0){
        results = malloc(sizeof(double)*element_per_process*size_process);
    }
    MPI_Gather(result.data,element_per_process,MPI_DOUBLE,results,element_per_process,MPI_DOUBLE,0,MPI_COMM_WORLD);
    
}
int main(int argc, char **argv)
{
    int world_size, world_rank;
    int i, buf = 0, *pt;
    int *sum;
    MPI_Init(&argc, &argv);
    splitdata();
    MPI_Finalize();
}