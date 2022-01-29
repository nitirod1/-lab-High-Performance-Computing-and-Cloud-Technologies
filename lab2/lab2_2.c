#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
double *readfile(char *filename, int *SizeCol, int *SizeRow)
{
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
    {
        printf("Error: could not open file %s", filename);
    }
    const unsigned MAX_LENGTH = 256;
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
//ใช้ malloc แล้วมีปัญหา
int main(int argc, char **argv)
{
    int elements_per_process, index;
    double *temp2;
    double *temp1;
   
    MPI_Status status;
    // Initialize the MPI environment. The two arguments to MPI Init are not
    // currently used by MPI implementations, but are there in case future
    // implementations might need the arguments.
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_size > 4)
    {
        printf("error ");
        MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
    }
    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // Get the name of the processor
    char processor_name[MPI_MAX_PROCESSOR_NAME];
    int name_len;
    MPI_Get_processor_name(processor_name, &name_len);

    // Print off a hello world message
    printf("Hello world %s, rank %d out of %d processors\n",
           processor_name, world_rank, world_size);
    int i;
    if (world_rank == 0)
    {
        int SizeR, SizeC, SizeR_, SizeC_;
        double test = 0;
        int index;
        temp1 = readfile("matAsmall.txt", &SizeR, &SizeC);
        temp2 = readfile("matBsmall.txt", &SizeR_, &SizeC_);
        elements_per_process = (SizeR * SizeC) / world_size;
        for (i = 1; i < world_size; i++)
        {
            index = i * elements_per_process;
            printf("show index : %d\n",index);
            // send size element_per_process
            MPI_Send(&elements_per_process, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
            MPI_Send(&temp1[index], elements_per_process, MPI_DOUBLE, i, 1, MPI_COMM_WORLD);     // MPI_Send(&temp2, elements_per_process, MPI_DOUBLE, i, 2, MPI_COMM_WORLD);
        }
    }
    else
    {
        MPI_Recv(&elements_per_process, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, &status);
        double *element = malloc(elements_per_process * sizeof(double));
        MPI_Recv(element, elements_per_process, MPI_DOUBLE, 0, 1, MPI_COMM_WORLD, &status);
        printf("%lf",element[199]);
        // MPI_Recv(&temp2,elements_per_process,MPI_DOUBLE,0,2,MPI_COMM_WORLD,&status);
    }
    // element = malloc(sizeof(double *) * (elements_per_process));
    // printf("rank %d data %lf element %d processors\n",world_rank,temp1[0], elements_per_process);
    // printf("rank %d data %lf element %d processors\n",world_rank,temp2[0], elements_per_process);
    // int buffer = (world_rank== 0)?12345:67890;
    // int tag_send = 0;
    // int tag_recv = tag_send;
    // int peer = (world_rank == 0)?1:0;
    // printf("MPI process rank %d sends value %d to MPI process %d.\n", world_rank, buffer, peer);
    // MPI_Sendrecv_replace(&buffer,1,MPI_INT,peer,tag_send,peer,tag_recv,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    // printf("MPI process rank %d received value %d from MPI process %d.\n", world_rank, buffer, peer);
    // Finalize the MPI environment. No more MPI calls can be made after this
    free(temp1);
    free(temp2);
    MPI_Finalize();
}