#include <omp.h>
#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#define FILENAME "test.txt"
int n = 21;
double *ReadFile(char *filename, int *size)
{
  double *arr;
  FILE *fp = fopen(filename, "r");
  if (fp != NULL)
  {
    fscanf(fp, "%d", size);
    arr = calloc(*size, sizeof(double));
    for (int i = 0; i < *size; i++)
    {
      fscanf(fp, "%lf", (arr + i));
    }
  }
  fclose(fp);
  return arr;
}
void swap(double *a, double *b)
{
  int t = *a;
  *a = *b;
  double *ReadFile(char *filename, int *size)
  {
    double *arr;
    FILE *fp = fopen(filename, "r");
    if (fp != NULL)
    {
      fscanf(fp, "%d", size);
      arr = calloc(*size, sizeof(double));
      for (int i = 0; i < *size; i++)
      {
        fscanf(fp, "%lf", (arr + i));
      }
    }
    fclose(fp);
    return arr;
  }
  *b = t;
}
void printArray(double array[], int size)
{
  for (int i = 0; i < size; ++i)
  {
    printf("%lf  ", array[i]);
  }
  printf("\n");
}
// function to find the partition position
int partition(double array[], int low, int high, double pivot)
{
  printf("pi %lf\n",pivot);
  // select the rightmost element as pivot
  // int pivot = array[high];
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
  // swap(&array[i + 1], &array[high]);
  // return the partition point
  return (i);
}

void quickSort(double array[], int low, int high, double pivot)
{
  if (low < high)
  {
    // printArray(array,n);
    // find the pivot element such that
    // elements smaller than pivot are on left of pivot
    // elements greater than pivot are on right of pivot
    int pi = partition(array, low, high, pivot);
    // printArray(array,n);

    // printf("%d %d %d\n",pivot,pi,low);
    // recursive call on the left of pivot

    quickSort(array, low, pi, array[pi - 1]);
    // recursive call on the right of pivot
    quickSort(array, pi + 1, high, array[high - 1]);
  }
}

// function to print array elements

void allocation_mem(int **num, int size)
{
  *num = (int *)malloc(size * sizeof(int));
}
// root ไว้ใกล้ตัวเสมอ
int send_elem(int root_p, int element)
{
  MPI_Send(&element, 1, MPI_INT, root_p, 4, MPI_COMM_WORLD);
  return element;
}
void send_data(int root_p, int scale, double data[], int size[])
{
  int i,rank,intial =0 ;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  for (i = root_p; i < scale; i++)
  {
    if(i >0){
      intial = size[i-1]+intial;
    }
    MPI_Send(&data[intial], size[i], MPI_DOUBLE, i, 4, MPI_COMM_WORLD);
  }
}
void receive(int root_p, double data[],int scale, int size[])
{
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Recv(data, size[rank], MPI_DOUBLE, root_p,4, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}
void intial_send_process(int root_p,int elem[], int scale, int size)
{ //ส่งจำนวน data ในแต่ละ process
  int i, disp = 0;
  for (i = root_p; i < scale; i++)
  {
    if (i + 1 == scale)
    {
      // ถ้าเป็นตัวสุดท้ายต้องส่งทั้งหมดใน arrays global
      elem[i] = (n / scale) + (n % scale);
    }
    else
    {
      elem[i] = n / scale;
      disp = disp + elem[i];
    }
    // MPI_Send(&element_process, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
  }
}

int intial_recv_process(int root_p)
{ // รับจำนวน data ในแต่ละ process
  int element_process, rank, scale;
  MPI_Comm_size(MPI_COMM_WORLD, &scale);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // MPI_Recv(&element_process, 1, MPI_INT, root_p, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  return element_process;
}
double recv_pi(){
  double pi;
  int i;
  MPI_Recv(&pi,1,MPI_DOUBLE,0,7,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
  return pi;
}
void send_pi(int start, int end , double pi ){
  int i ;
  for(i=start;i<=end;i++){
    MPI_Send(&pi,1,MPI_DOUBLE,i,7,MPI_COMM_WORLD);
  }
}
void scatter_process(int start,int end , int n){

}
/* tag = 6 is recv gather
  tag = 5 is recv n
*/
int gathercv_process(int start , int end ,int n){
  int i,j=0 ,count=0,size,mn;
  double *dataR = calloc(n, sizeof(double));
  for(int i =start;i<=end;i++){
    MPI_Recv(&size,1,MPI_INT,i,5,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    double *dataE = calloc(size,sizeof(double));
    MPI_Recv(dataE,size,MPI_DOUBLE,i,6,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    for(j=0;j<size;j++){
      dataR[count++] = dataE[j];
    }
    free(dataE);
  }
  mn = count;
  for(int i =start;i<=end;i++){
    MPI_Recv(&size,1,MPI_INT,i,7,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    double *dataE = calloc(size,sizeof(double));
    MPI_Recv(dataE,size,MPI_DOUBLE,i,8,MPI_COMM_WORLD,MPI_STATUS_IGNORE);
    for(j=0;j<size;j++){
      dataR[count++] = dataE[j];
    }
    free(dataE);
  }
  printArray(dataR,n);
  return mn;
}
/*rank = 0 , 1 , 2 , 3 */
/*if rank in range root_p , scale*/
void split_data_p(int root_p , int scale ,double dataR[],int element_process[],int n){
  int count_p = scale - root_p +1;
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if(rank ==  root_p){
    // master process ทำหน้าที่ส่งไป
    if(count_p == 1 ){
      // แบ่งทั้งหมด เข้า process เดียวแล้ว qsort

    }
    else{
      // แบ่งทั้งหมด ตามค่าเฉลี่ยของ process
      intial_send_process(root_p,element_process,scale,n);
    }
  }
  else{
    // slave process รับ proess ที่ได้รับจากการ แบ่ง จาก root
    intial_recv_process(root_p);
  }
}

void processing_all(int root_p, int scale,double dataR[],int n){
  double *dataE , pi;
  int rank, *element_process,mn;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  element_process = calloc(scale,sizeof(int));
  split_data_p(root_p,scale,dataR,element_process,n);
  // if (rank == root_p)
  // {
  //   intial_send_process(root_p,element_process, scale, n);
  // }
  // intial_recv_process(root_p);
  MPI_Barrier(MPI_COMM_WORLD);
  MPI_Bcast(dataR, n, MPI_DOUBLE, root_p, MPI_COMM_WORLD);
  MPI_Bcast(element_process,scale,MPI_INT,root_p,MPI_COMM_WORLD);
  /*กระจายข้อมูลไปให้ทุก process*/
  if(rank == root_p){
    send_data(root_p,scale,dataR,element_process);
    pi = dataR[n-1];
    send_pi(root_p,scale-1,pi);
  }
  dataE = calloc(element_process[rank], sizeof(double));
  MPI_Barrier(MPI_COMM_WORLD);
  receive(root_p,dataE,scale,element_process);
  // sort 
  pi = recv_pi();
  int cluster = partition(dataE,0,element_process[rank],pi);
  cluster+=1;
  int cluster_mx = element_process[rank]-cluster;
  printf("rank %d min %d ,max %d\n",rank,cluster , cluster_mx);
  //จะรวมค่า ไปที่ master process โดย แบ่ง ฝั่งซ่าย น้อยก่วา ฝั่งขวามากกว่า
  MPI_Send(&cluster,1,MPI_INT,root_p,5,MPI_COMM_WORLD);
  MPI_Send(dataE,cluster,MPI_DOUBLE,root_p,6,MPI_COMM_WORLD);
  MPI_Send(&cluster_mx,1,MPI_INT,root_p,7,MPI_COMM_WORLD);
  MPI_Send(&dataE[cluster],cluster_mx,MPI_DOUBLE,root_p,8,MPI_COMM_WORLD);
  //
  if(rank == root_p){
    mn = gathercv_process(root_p,scale-1,n);
    printf("mn %d \n",mn);
  }
  // control_Process();
}
int main(int argc, char *argv[])
{
  MPI_Init(&argc, &argv);
  int scale,rank;
  double *data = calloc(n, sizeof(double));
  MPI_Comm_size(MPI_COMM_WORLD, &scale);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if(rank == 0){
    data = ReadFile(FILENAME, &n);
  }
  processing_all(0,scale,data,n);
  MPI_Finalize();
  /*____________________*/
  // printf("recv rank %d",rank);
  // printArray(dataE,element_process);
  // printf("size :%d\n", size);
  // printf("%d %d Unsorted\n", rank, element_process);
  // quickSort(dataR, rank * element_process, (rank + 1) * element_process, dataR[((rank + 1) * element_process) - 1]);
  // printf("sorted Array :%d\n",rank);
  // printArray(dataR, n);
  // printf("%d\n",rank);

  // // perform quicksort on data
  // quickSort(data, 0, n - 1);

  // printf("Sorted array in ascending order: \n");
  // printArray(data, n);
  
}