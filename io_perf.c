#include <pthread.h> 
#include <stdio.h>
#include <fcntl.h> 
#include <unistd.h>
#include <stdlib.h>

#define NUM_REQUESTS 100
#define REQUEST_SIZE 16384
#define RANDOM_REQUEST_SIZE 128
#define BLOCK_SIZE 4096

typedef struct {
     off_t offset;
     size_t bytes;
} request_t; 

void *reader_thread_func(request_t in_list_for_thread[], char file_path[]) { 
     
     int fd = open(file_path, O_RDONLY);
     if(fd < 0) // check if file was opened
     {
          printf("File was not opened, thread not used");
          return NULL;
     }
     int num_requests = sizeof(in_list_for_thread) / sizeof(request_t); // Calculate the number of elements in the inlist, works because *in_list_for_thread is static in thread
     size_t num_bytes = 0;
     for (size_t i = 0; i < num_requests; i++)
     {
          num_bytes = (in_list_for_thread[i]->bytes);
     }
     
     char read_text_buffer[num_bytes + 1];
   
     // @Add code for reader threads
     // @Given a list of [offset1, bytes1], [offset2, bytes2], ...
     
     for (size_t i = 0; i < num_requests; i++)
     {
          
          
     }
     
     // @for each: read bytes_i from offset_i

     pthread_exit(0);
     return NULL;
}


void *writer_thread_func(/*@input paramters*/) { 
     
     // @Add code for writer threads
     // @Given a list of [offset1, bytes1], [offset2, bytes2], ...
     // @for each: write bytes_i to offset_i

     pthread_exit(0);
}


int main(int argc, char *argv[])
{
     int n = atoi(argv[1]); //number of bytes
     int p = atoi(argv[2]); //number of threads

     // @create a file for saving the data
     int fd = open("datafile", O_RDWR | O_CREAT, 0666); //RDWR: open for read and write, CREAT: create the file if doesnt exist

     // @allocate a buffer and initialize it
     char* ptr = (char*)calloc(n, sizeof(char));

     
     // @create two lists of 100 requests in the format of [offset, bytes]

     // @List 1: sequtial requests of 16384 bytes, where offset_n = offset_(n-1) + 16384
     // @e.g., [0, 16384], [16384, 16384], [32768, 16384] ...
     // @ensure no overlapping among these requests.
     //allocate memory list1
     request_t *list1 = malloc(NUM_REQUESTS * sizeof(request_t)); 
     //initialize list1
     for(int i = 0; i < NUM_REQUESTS; i++){
          list1[i].offset = i * REQUEST_SIZE;
          list1[i].bytes = REQUEST_SIZE;
     }

     // @List 2: random requests of 128 bytes, where offset_n = random[0,N/4096] * 4096
     // @e.g., [4096, 128], [16384, 128], [32768, 128], etc.
     // @ensure no overlapping among these requests.
     
     //allocate memory list2
     request_t *list2 = malloc(NUM_REQUESTS * sizeof(request_t));

     int num_blocks = n / BLOCK_SIZE;
     
     srand(time(NULL)); //random number generator
     //array to track used blocks
     int *used_blocks = calloc(num_blocks, sizeof(int));

     int requests_generated = 0;
     while(requests_generated < NUM_REQUESTS){
          int block_index = rand() % num_blocks;
          if(used_blocks[block_index] == 0){
               used_blocks[block_index] = 1;
               list2[requests_generated].offset = block_index * BLOCK_SIZE;
               list2[requests_generated].bytes = RANDOM_REQUEST_SIZE;
               requests_generated++;
          }
          //if block is already used, loop again to find new block
     }
     free(used_blocks);
     

     // @start timing 

     /* Create writer workers and pass in their portion of list1 */   
     
     
     /* Wait for all writers to finish */ 
     

     // @close the file 


     // @end timing 


     //@Print out the write bandwidth
     printf("Write %f MB, use %d threads, elapsed time %f s, write bandwidth: %f MB/s \n", /**/);
     
     
     // @reopen the file 

     // @start timing 

     /* Create reader workers and pass in their portion of list1 */   
     
     
     /* Wait for all reader to finish */ 
     

     // @close the file 


     // @end timing 


     //@Print out the read bandwidth
     printf("Read %f MB, use %d threads, elapsed time %f s, write bandwidth: %f MB/s \n", /**/);


     // @Repeat the write and read test now using List2 


     /*free up resources properly */
}
