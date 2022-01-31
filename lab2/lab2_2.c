#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

double *readfile(char *filename, int *SizeRow, int *SizeCol)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error: could not open file %s", filename);
    }
    const unsigned MAX_LENGTH = 1048576;
    char buffer[MAX_LENGTH];
    char *pch;
    int i = 0;
    int temp1, temp2;
    double *data;
    // split size
    fgets(buffer, MAX_LENGTH, fp);
    pch = strtok(buffer, " ");
    temp1 = atoi(pch);
    *SizeRow = temp1;
    pch = strtok(NULL, " ");
    temp2 = atoi(pch);
    *SizeCol = temp2;
    pch = strtok(NULL, " ");
    // split size
    data = (double *)malloc(sizeof(double *) * ((*SizeRow) * (*SizeCol)));
    while (fgets(buffer, MAX_LENGTH, fp))
    {
        pch = strtok(buffer, " ");
        while (pch != NULL)
        {
            data[i++] = atof(pch);
            pch = strtok(NULL, " ");
        }
    }
    // close the file
    fclose(fp);
    return data;
}
void writefile(char *filename, double *result, int SizeR, int SizeC)
{
    FILE *fp = fopen(filename, "w");
    int i;
    if (fp == NULL)
    {
        printf("file can't open\n");
    }
    fprintf(fp, "%d %d", SizeR, SizeC);
    for (i = 0; i < SizeR * SizeC; i++)
    {
        if (i % SizeC == 0)
        {
            fprintf(fp, "\n");
        }
        fprintf(fp, "%lf ", result[i]);
    }
}

//ใช้ malloc แล้วมีปัญหา
int main(int argc, char **argv)
{
    int elements_per_process, index, pos = 0;
    double *temp2;
    double *temp1;
    double *result;
    MPI_Status status;
    MPI_Init(&argc, &argv);

    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_size > 4)
    {
        printf("error ");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    printf("Hello world %s, rank %d out of %d processors\n",
           processor_name, world_rank, world_size);
    int i, j;
    if (world_rank == 0)
    {
        int SizeR, SizeC, SizeR_, SizeC_;
        double test = 0;
        int index;
        temp1 = readfile("matAlarge.txt", &SizeR, &SizeC);
        temp2 = readfile("matBlarge.txt", &SizeR_, &SizeC_);
        elements_per_process = (SizeR * SizeC) / world_size;
        result = malloc((SizeR * SizeC) * sizeof(double));
        for (i = 0; i < elements_per_process; i++)
        {
            result[pos++] = temp1[i] + temp2[i];
        }
        for (i = 1; i < world_size; i++)
        {
            index = i * elements_per_process;
            // send size element_per_process
            MPI_Send(&elements_per_process, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&temp1[index], elements_per_process, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);
            MPI_Send(&temp2[index], elements_per_process, MPI_DOUBLE, i, 2, MPI_COMM_WORLD);
            double *calculated = malloc(elements_per_process * sizeof(double)); // MPI_Send(&temp2, elements_per_process, MPI_DOUBLE, i, 2, MPI_COMM_WORLD);
            MPI_Recv(calculated, elements_per_process, MPI_DOUBLE, i, 3, MPI_COMM_WORLD, &status);
            for (j = 0; j < elements_per_process; j++)
            {
                result[pos++] = calculated[j];
            }
            free(calculated);
        }
        writefile("result.txt", result, SizeR, SizeC);
    }
    else
    {
        MPI_Recv(&elements_per_process, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        double *element_1 = malloc(elements_per_process * sizeof(double));
        MPI_Recv(element_1, elements_per_process, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
        double *element_2 = malloc(elements_per_process * sizeof(double));
        MPI_Recv(element_2, elements_per_process, MPI_DOUBLE, 0, 2, MPI_COMM_WORLD, &status);
        double *calculated = malloc(elements_per_process * sizeof(double));
        for (i = 0; i < elements_per_process; i++)
        {
            calculated[i] = element_1[i] + element_2[i];
        }
        free(element_1);
        free(element_2);
        MPI_Send(&calculated[0], elements_per_process, MPI_DOUBLE, 0, 3, MPI_COMM_WORLD);
    }
    free(temp1);
    free(temp2);
    MPI_Finalize();
}