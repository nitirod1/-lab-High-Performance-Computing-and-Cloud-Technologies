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
    arrays_reservation_Matrixs(mat, element_size);
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
int Row_Per_Process(Matrixs *mat)
{
    int SIZE, mod, rank;
    int *row_p, i, output;
    MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    row_p = malloc(sizeof(int) * SIZE);
    if (rank == SIZE - 1)
    {

        for (i = 0; i < SIZE; i++)
        {
            row_p[i] = mat->sizeR / SIZE;
        }
    }
    MPI_Scatter(row_p, 1, MPI_INT, &output, 1, MPI_INT, SIZE - 1, MPI_COMM_WORLD);
    return output;
}

void m_to_r_size(Matrixs *m1, Matrixs *m2)
{
    m2->sizeC = m1->sizeC;
    m2->sizeR = m1->sizeR;
    m2->commute = m1->commute;
}
void Bcast_process(int process_n, Matrixs *mat)
{
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    
    MPI_Bcast(&mat->sizeC, 1, MPI_INT, process_n, MPI_COMM_WORLD);
    MPI_Bcast(&mat->sizeR, 1, MPI_INT, process_n, MPI_COMM_WORLD);
    if (rank != process_n)
    {
        arrays_reservation_Matrixs(mat, mat->sizeC * mat->sizeR);
    }
    MPI_Bcast(mat->data, mat->sizeC * mat->sizeR, MPI_DOUBLE, process_n, MPI_COMM_WORLD);
}
int check_mul(Matrixs *mat1, Matrixs *mat2)
{
    if (mat1->sizeC == mat2->sizeR)
    {
        return 1;
    }
    return -1;
}

int calculation(Matrixs *mat1, Matrixs *mat2,Matrixs *result, int start, int end)
{
    int i, j, k, index_mat = 0,rank;
    MPI_Comm_rank(MPI_COMM_WORLD,&rank);
    arrays_reservation_Matrixs(result, (end - start)*mat2->sizeC);
    if (check_mul(mat1, mat2))
    {
        // i=0,i<mat1->sizeR
        for (i = start; i < end; i++)
        {
            for (j = 0; j < mat2->sizeC; j++)
            {
                for (k = 0; k < mat1->sizeC; k++)
                {
                    result->data[index_mat] += (mat1->data[k + (i * mat1->sizeC)] * mat2->data[j + (k * mat2->sizeC)]);
                }
                
                index_mat++;
            }
        }
    }
    result->sizeC = mat1->sizeR;
    result->sizeR = mat2->sizeC;
    return index_mat;
}

void deliver()
{
    Matrixs mat1, mat2, output_all_p, result;
    double *results = NULL;
    int rank, start, end, row_size, SIZE, elem_size,mod,*count = NULL,*disp=NULL,i;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
    if (SIZE > 1)
    {
        select_process_read("matAsmall.txt", 0, &mat1);
        select_process_read("matBsmall.txt", 1, &mat2);
        Bcast_process(0, &mat1);
        Bcast_process(1, &mat2);
        MPI_Barrier(MPI_COMM_WORLD);
    }
    else
    {
        select_process_read("matAsmall.txt", 0, &mat1);
        select_process_read("matBsmall.txt", 0, &mat2);
        Bcast_process(0, &mat1);
        Bcast_process(0, &mat2);
    }
    row_size = Row_Per_Process(&mat1);
    start = rank * row_size ;
    if (rank == SIZE - 1)
    {
        mod = mat1.sizeR % SIZE;
        row_size = mod + row_size;
    }
    end = start + row_size ;
    elem_size=calculation(&mat1,&mat2,&result,start,end);
    MPI_Barrier(MPI_COMM_WORLD);
    if(rank==0){
        disp = malloc(sizeof(int)*SIZE);
        count = malloc(sizeof(int)*SIZE);
    }
    MPI_Gather(&elem_size,1,MPI_INT,count,1,MPI_INT,0,MPI_COMM_WORLD);
    if(rank == 0){
        disp[0]=0;
        for(i=1;i<SIZE;i++){
            disp[i]=disp[i-1]+count[i-1];
        }
        output_all_p.sizeR = mat1.sizeR;
        output_all_p.sizeC = mat2.sizeC;
        arrays_reservation_Matrixs(&output_all_p,output_all_p.sizeR *output_all_p.sizeC );
        
    }
    MPI_Gatherv(result.data,elem_size,MPI_DOUBLE,output_all_p.data,count,disp,MPI_DOUBLE,0,MPI_COMM_WORLD);
    if(rank == 0){
        writefile("test.txt",&output_all_p);
    }
    // row_p[SIZE - 1] = mod + (mat->sizeR / SIZE);

    // element_size = mat1.sizeC * mat1.sizeR + mat1.commute;
    // elem_p_r1 = Element_Per_Process(element_size);
    // arrays_reservation_Matrixs(&recv1, element_size);
    // element_size = mat2.sizeC * mat2.sizeR + mat2.commute;
    // elem_p_r2 = Element_Per_Process(element_size);
    // printf("after %d %d\n",recv2.sizeR,recv2.sizeC);
    // arrays_reservation_Matrixs(&recv2, element_size);
    // if(SIZE>1){
    //     MPI_Scatter(mat1.data, elem_p_r1, MPI_DOUBLE, recv1.data, elem_p_r1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    //     MPI_Scatter(mat2.data, elem_p_r2, MPI_DOUBLE, recv2.data, elem_p_r2, MPI_DOUBLE, 1, MPI_COMM_WORLD);
    // }
    // else{
    //     MPI_Scatter(mat1.data, elem_p_r1, MPI_DOUBLE, recv1.data, elem_p_r1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    //     MPI_Scatter(mat2.data, elem_p_r2, MPI_DOUBLE, recv2.data, elem_p_r2, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    // }

    // result.data =calculation(&recv1, &recv2);
    // if(rank == 0 ){
    //     results = malloc(sizeof(double) * element_per_process * size_process);
    // }
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