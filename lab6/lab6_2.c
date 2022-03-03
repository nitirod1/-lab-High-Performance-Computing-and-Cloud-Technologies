#include <omp.h> 
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef struct Matrix
{
    double *data;
    int sizeR;
    int sizeC;
} Matrixs;
Matrixs x1[2];
void arrays_reservation_Matrixs(Matrixs *mat, int element_size)
{
    mat->data = (double *)calloc(element_size, sizeof(double *));
}
void Readfile(char *filename, Matrixs *mat)
{
    int i = 0, SIZE = 0, element_size;
    int temp1, temp2;
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error can't open file\n");
    }
    fscanf(fp, "%d %d", &mat->sizeR, &mat->sizeC);
    element_size = mat->sizeR * mat->sizeC;
    arrays_reservation_Matrixs(mat, element_size);
    #pragma omp parallel for
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
    fclose(fp);
}
double *plus(Matrixs **matA, Matrixs **matB)
{
    int i;
    double temp1, temp2;
    int size = (*matA)->sizeR*(*matB)->sizeC;
    double *cal = (double *)malloc(sizeof(double *) *size);
#pragma omp parallel for
    for (i = 0; i < size; i++)
    {
        cal[i] = (*matA)->data[i] + (*matB)->data[i];
    }
    return cal;
}
double *cal_matrix(Matrixs *matA, Matrixs *matB)
{

    double *result = plus(&matA, &matB);
    return result;
}
void Data_input(){
    int i =0;
    #pragma omp parallel for
    for(i=0;i<2;i++){
        if(i==0){
            Readfile("matAlarge.txt",&x1[0]);
        }else{
            Readfile("matBlarge.txt",&x1[1]);
        }
        
    }
    
    
}
void same_size(Matrixs *matA,Matrixs *matB){
    matA->sizeR = matB->sizeR;
    matA->sizeC = matB->sizeC;
}
void deliver(){
    Matrixs result;
    Data_input();   
    result.data=cal_matrix(&x1[0],&x1[1]);
    same_size(&result,&x1[0]);
    #pragma omp single 
    writefile("test.txt",&result);
    
}

int main(int argc, char **argv)
{
    clock_t start = clock();
    int rank;
    deliver();
    clock_t stop = clock();
    printf("max threads: %d time: %lf\n", omp_get_max_threads(), ((double)(stop - start)) / CLOCKS_PER_SEC);
}