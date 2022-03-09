// Quick sort in C
#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
int data[] = {7, 13, 18, 2, 17, 1, 14, 20, 6, 10, 15, 9, 3, 16, 19, 4, 11, 12, 5, 8};
// function to swap elements
int n;
int SIZE;
int RANK;
void swap(int *a, int *b)
{
    int t = *a;
    *a = *b;
    *b = t;
}

int partition(int array[], int pivot, int low, int high)
{

    // select the rightmost element as pivot

    // pointer for greater element
    int i = (low - 1);
    if (low < high)
    {
        // traverse each element of the array
        // compare them with the pivot
        for (int j = low; j < high; j++)
        {
            if (array[j] <= pivot)
            {

                // if element smaller than pivot is found
                // swap it with the greater element pointed by i
                i++;

                // swap element at i with element at j
                swap(&array[i], &array[j]);
            }
        }

        // swap the pivot element with the greater element at i
        swap(&array[i + 1], &array[high]);

        // return the partition point
        return (i + 1);
    }
    return -1;
}

// void quickSort(int array[], int low, int high)
// {
//     if (low < high)
//     {
//         // find the pivot element such that
//         // elements smaller than pivot are on left of pivot
//         // elements greater than pivot are on right of pivot
//         int pi = partition(array, low, high);

//         // // recursive call on the left of pivot
//         // quickSort(array, low, pi - 1);

//         // // recursive call on the right of pivot
//         // quickSort(array, pi + 1, high);
//     }
// }

// function to print array elements
void printArray(int array[], int size)
{
    for (int i = 0; i < size; ++i)
    {
        printf("%d  ", array[i]);
    }
    printf("\n");
}
int intial_size()
{ // ประกาศ size ของ data
    return sizeof(data) / sizeof(data[0]);
}
void allocation_mem(int **num, int size)
{
    *num = (int *)malloc(size * sizeof(int));
}
void intial_send_process()
{ //ส่งจำนวน data ในแต่ละ process
    int i, element_process;
#pragma omp parallel for private(element_process)
    for (i = 0; i < SIZE; i++)
    {
        if (i == SIZE - 1)
        {
            // ถ้าเป็นตัวสุดท้ายต้องส่งทั้งหมดใน arrays global
            element_process = (n / SIZE) + (n % SIZE);
        }
        else
        {
            element_process = n / SIZE;
        }
        MPI_Send(&element_process, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
    }
}
int intial_recv_process()
{ // รับจำนวน data ในแต่ละ process
    int element_process;
    MPI_Recv(&element_process, 1, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    return element_process;
}
void position(int *pos, int element_process)
{ // return position start กับ end ของ data
    if (RANK == SIZE - 1)
    {
        pos[0] = ((element_process - (n % SIZE)) * RANK);
        pos[1] = (element_process - (n % SIZE)) * (RANK + 1) + (n % SIZE);
    }
    else
    {
        pos[0] = element_process * RANK;
        pos[1] = element_process * (RANK + 1);
    }
    pos[1] =pos[1]-1;
}
void disp(int displace[], int count[],int dis)
{ // return displace arrays
    int i;
    displace[0] = dis;
    for (i = 1; i < SIZE; i++)
    {
        displace[i] = displace[i - 1] + count[i - 1];
    }
}
int select_pivot(int last_element)
{
    return data[last_element];
}
int gather_data(int size,int arrays[],int dis){
    int *count,*count_pos_quick,*displace,i,all=0;
    allocation_mem(&(displace), SIZE);
    allocation_mem(&(count_pos_quick), SIZE);
    allocation_mem(&(count), SIZE);
    MPI_Gather(&size, 1, MPI_INT, count_pos_quick, 1, MPI_INT, 0, MPI_COMM_WORLD);
    for (i = 0; i < SIZE; i++)
    {
        count[i] = count_pos_quick[i];
        all = all+count[i];
        
    }
    disp(displace, count,dis);
    MPI_Gatherv(arrays, count[0], MPI_INT, data, count, displace, MPI_INT, 0, MPI_COMM_WORLD);
    return all;
}
void gather_data_slave(int size,int arrays[]){
    int *count,*count_pos_quick,*displace,i;
    allocation_mem(&(displace), SIZE);
    allocation_mem(&(count_pos_quick), SIZE);
    allocation_mem(&(count), SIZE);
    MPI_Gather(&size, 1, MPI_INT, NULL, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Gatherv(arrays, size, MPI_INT,NULL, NULL, NULL, MPI_INT, 0, MPI_COMM_WORLD);
}   
void prepare_data_less(int temp[],int start,int end){
    int i , j=0;
    for (i = start; i < end; i++)
    {
        temp[j++] = data[i];
    }
}
void prepare_data_more(int temp[],int end,int start){
    int i , j=0;
    for (i = start; i > end-1; i--)
    {
        temp[j++] = data[i];
    }
}
void root_process()
{
    int element_process, pivot, s;
    int *pos,   i = 0,temp[10],num,j=0;
    pivot = select_pivot(n - 1);
    MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_WORLD);
    // printf("RANK %d pivot %d\n", RANK, pivot);
    allocation_mem(&(pos), 2);
    intial_send_process();
    element_process = intial_recv_process();
    position(pos, element_process);
    s = partition(data, pivot, pos[0], pos[1]);
    // printf("Sorted array in ascending order: \n");
    prepare_data_more(temp,s,pos[1]);
    num = gather_data(s,data,0);
    s = pos[1]-s+1;
    gather_data(s,temp,num);
}
void slave_process()
{
    int element_process, pivot, temp[10], s;
    int *pos, *displace, *count, i, j = 0, *count_pos_quick;
    MPI_Bcast(&pivot, 1, MPI_INT, 0, MPI_COMM_WORLD);
    allocation_mem(&(pos), 2);
    element_process = intial_recv_process();
    position(pos, element_process);
    s = partition(data, pivot, pos[0], pos[1]);
    s = s - pos[0];
    prepare_data_less(temp,pos[0],pos[1]);
    gather_data_slave(s,temp);
    s = s+pos[0];
    prepare_data_more(temp,s,pos[1]);
    s = pos[1]-s+1;
    gather_data_slave(s,temp);
    
}
void deliver()
{
    if (RANK == 0)
    {
        root_process();
    }
    else
    {
        slave_process();
    }
    if (RANK == 0)
    {
        printArray(data, n);
    }
    
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    n = intial_size();
    printf("%d\n",n);
    MPI_Comm_size(MPI_COMM_WORLD, &SIZE);
    MPI_Comm_rank(MPI_COMM_WORLD, &RANK);
    if (RANK == 0)
    {
        // printf("UNSORTED\n");
        // printArray(data, n);
    }
    deliver();
    MPI_Finalize();
}