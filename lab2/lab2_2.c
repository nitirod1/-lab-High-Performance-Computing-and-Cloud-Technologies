#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    int temp1 , temp2;
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
    data = malloc(sizeof(double *) * ((*SizeRow) * (*SizeCol)));
    while (fgets(buffer, MAX_LENGTH, fp))
    {
        pch = strtok(buffer, " ");
        while (pch != NULL)
        {
            data[i++]= atof(pch);
            pch = strtok(NULL, " ");
        }
    }
    // close the file
    fclose(fp);
    return data;
}
int main(int argc, char **argv)
{
    // Initialize the MPI environment. The two arguments to MPI Init are not
    // currently used by MPI implementations, but are there in case future
    // implementations might need the arguments.
    MPI_Init(&argc, &argv);

    // Get the number of processes
    int world_size;
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    if (world_size != 4)
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
    char *str1;
    if (world_rank == 0)
    {
        printf("process read A\n");
        int SizeR,SizeC;
        double *temp1 = readfile("matAsmall.txt",&SizeR,&SizeC);
        printf(" %lf , %d , %d",temp1[0],SizeR,SizeC);
        // double **temp2 = readfile("matBsmall.txt");
    }
    // int buffer = (world_rank== 0)?12345:67890;
    // int tag_send = 0;
    // int tag_recv = tag_send;
    // int peer = (world_rank == 0)?1:0;
    // printf("MPI process rank %d sends value %d to MPI process %d.\n", world_rank, buffer, peer);
    // MPI_Sendrecv_replace(&buffer,1,MPI_INT,peer,tag_send,peer,tag_recv,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    // printf("MPI process rank %d received value %d from MPI process %d.\n", world_rank, buffer, peer);
    // Finalize the MPI environment. No more MPI calls can be made after this
    MPI_Finalize();
}