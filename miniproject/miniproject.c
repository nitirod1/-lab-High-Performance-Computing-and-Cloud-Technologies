// Quick sort in C
#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
int data[] = {75, 91, 15, 64, 21, 8, 88, 54, 50, 12, 47, 72, 65, 54, 66, 22, 83, 66, 67, 0, 70, 98};
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
int partition(int array[], int low, int high)
{

    // select the rightmost element as pivot
    int pivot = array[high];

    // pointer for greater element
    int i = (low - 1);

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


void quickSort(int array[], int low, int high)
{
    if (low < high)
    {
        // find the pivot element such that
        // elements smaller than pivot are on left of pivot
        // elements greater than pivot are on right of pivot
        int pi = partition(array, low, high);

        // recursive call on the left of pivot
        quickSort(array, low, pi - 1);

        // recursive call on the right of pivot
        quickSort(array, pi + 1, high);
    }
}

// function to print array elements
void printArray(int array[], int size)
{
    for (int i = 0; i < size; ++i)
    {
        printf("%d  ", array[i]);
    }
    printf("\n");
}
int intial_size(){ // ประกาศ size ของ data
    return sizeof(data) / sizeof(data[0]);
}
void allocation_mem(int **num,int size){
    *num = (int*)malloc(size*sizeof(int));
}
void intial_send_process(){//ส่งจำนวน data ในแต่ละ process
    int  i,element_process;
    int *num;
    #pragma omp parallel for private(element_process)
    for(i =0 ;i<SIZE;i++){
        if(i == SIZE-1){
            // ถ้าเป็นตัวสุดท้ายต้องส่งทั้งหมดใน arrays global
            element_process = (n /SIZE) + (n%SIZE);
        }
        else{
            element_process = n / SIZE;
        }
        MPI_Send(&element_process,1,MPI_INT,i,0,MPI_COMM_WORLD);
    }
}
int intial_recv_process(){ // รับจำนวน data ในแต่ละ process
    int element_process;
    MPI_Recv(&element_process,1,MPI_INT,0,0,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    printf(" %d %d \n",element_process,RANK);
    return element_process;
}
void position(int *pos,int element_process){// return position start กับ end ของ data
    
    if(RANK == SIZE-1){
        pos[0] = ((element_process-(n%SIZE))*RANK);
        printf("pos[0] :%d\n",((element_process-(n%SIZE))*RANK));
        pos[1] = (element_process-(n%SIZE))*(RANK+1) + (n%SIZE);
    }else{
        pos[0] = element_process*RANK;
        pos[1] = element_process*(RANK+1);
    }
}
void deliver(){
    int element_process ;
    int *pos;
    allocation_mem(&(pos),2);
    if(RANK==0){
        intial_send_process();
    }
    element_process= intial_recv_process();
    position(pos,element_process);
    printf("start %d end %d\n",pos[0],pos[1]);
    // quickSort(data, 0, n - 1);
    // printf("Sorted array in ascending order: \n");
    // printArray(data, n);
}

int main(int argc, char *argv[])
{
    MPI_Init(&argc, &argv);
    n = intial_size();
    MPI_Comm_size(MPI_COMM_WORLD,&SIZE);
    MPI_Comm_rank(MPI_COMM_WORLD,&RANK);
    if(RANK == 0){
        printf("Unsorted arrays\n");
        // printArray(data, n);
    }
    deliver();
    MPI_Finalize();
}