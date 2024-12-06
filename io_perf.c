#include <pthread.h> 
#include <stdio.h>
#include <fcntl.h> 
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define NUM_REQUESTS 100
#define REQUEST_SIZE 16384
#define RANDOM_REQUEST_SIZE 128
#define BLOCK_SIZE 4096
#define FILE_PATH "datafile"

typedef struct {
     off_t offset;
     size_t bytes;
} request_t; 


// Create a struct for all inputs for the thread-functions
typedef struct {
     request_t *in_list_for_thread;
     size_t size_of_inlist;
     char *file_path;
     char *text_buffer;
     int fd;
} ThreadArgs;

// Constructor for the inputs of the thread function
ThreadArgs initThreadArgs(request_t *in_list_for_thread, size_t size_of_inlist, char *file_path, char *text_buffer, int fd){
     ThreadArgs ta = { .file_path = file_path, .in_list_for_thread = in_list_for_thread, size_of_inlist = size_of_inlist, .fd = fd};
     return ta;
}

// @ in_list_for_thread[] is a list of structs that store the offset and amount of bytes to read
// @ num_requests is the size of the 
void *reader_thread_func(void *arg) {

     //Cast void-pointer to ThreadArgs-pointer, input needs to be void *arg for pthread_create
     ThreadArgs *ta = (ThreadArgs*) arg;

     // measure the amount of bytes needed to save the read data (might not be needed)
     size_t num_bytes = 0;
     for (size_t i = 0; i < ta->size_of_inlist; i++)
     {
          num_bytes = (ta->in_list_for_thread[i].bytes) + num_bytes;
     }
     
   
     // @Add code for reader threads
     // @Given a list of [offset1, bytes1], [offset2, bytes2], ...
     off_t offset;
     size_t bytes_to_read;
     // @for each: read bytes_i from offset_i
     for (size_t i = 0; i < ta->size_of_inlist; i++)
     {
          offset = ta->in_list_for_thread[i].offset;
          lseek(ta->fd, offset, SEEK_SET);                           // set the current place in the file to the offset given
          bytes_to_read = ta->in_list_for_thread[i].bytes;
          read(ta->fd, ta->text_buffer, bytes_to_read);     // read the correct amount of bytes according to the input and put it in text_buffer
     }
     //read_text_buffer[num_bytes] = '\0'; // add the NULL terminator for strings in c <- behövs inte tror jag


     pthread_exit(0);
     return NULL; // won't happen because pthread_exit will terminate the thread
}


void *writer_thread_func(void *arg) { 
     printf("vi började");
     //Cast void-pointer to ThreadArgs-pointer, input needs to be void *arg for pthread_create
     ThreadArgs *ta = (ThreadArgs *)arg;
     
     // @Add code for writer threads
     // @Given a list of [offset1, bytes1], [offset2, bytes2], ...
     off_t offset;
     size_t bytes_to_write;
     // @for each: write bytes_i to offset_i
     for (size_t i = 0; i < ta->size_of_inlist; i++)
     {
          offset = ta->in_list_for_thread[i].offset;
          lseek(ta->fd, offset, SEEK_SET);                    // set the current place in the file to the offset given
          bytes_to_write = ta->in_list_for_thread[i].bytes;
          write(ta->fd, ta->text_buffer, bytes_to_write);     // write bytes from the input and put it in text_buffer
     }
     
     
     printf("vi blev klara");
     pthread_exit(0);
     return NULL; // won't happen because pthread_exit will terminate the thread
}

// Takes the list and all relevant inputs, runs the writers and waits for them to finish
void run_writers(request_t *list, int n, int p, char *file_path, char *text_buffer, int fd){
     pthread_t thread_ids[p];
     size_t block_size = n/p;
     ThreadArgs ta;
     void *status = NULL;

     for (int i = 0; i < p; i++)
     {    

          ta = initThreadArgs(list+(block_size*i), block_size, file_path, text_buffer, fd);

          if (pthread_create(&thread_ids[i], NULL, writer_thread_func, &ta) != 0){
               perror("Error creating thread");
               exit(EXIT_FAILURE);
          }
          
     }

     for (int i = 0; i < p; i++)
     {
          if (pthread_join(thread_ids[i], &status) != 0) {  /*problem sitter här, vet inte varför*/
               printf("Error joining thread in writers, status %ld", (long)status);
               exit(EXIT_FAILURE);
          }
          printf("we made it\n");
     }
     

     return;  
}

// Takes the list and all relevant inputs and runs the readers
void run_readers(request_t *list, int n, int p, char *file_path, char *text_buffer, int fd){
     pthread_t thread_ids[p];
     size_t block_size = n/p;
     ThreadArgs ta;
     void *status;
     for (int i = 0; i < p; i++)
     {
          ta = initThreadArgs(list+(block_size*i), block_size, file_path, text_buffer, fd);
          thread_ids[i] = (pthread_t)i;
          if (pthread_create(&thread_ids[i], NULL, reader_thread_func, &ta) != 0){
               perror("Error creating thread");
               exit(EXIT_FAILURE);
          }
     }

     for (int i = 0; i < p; i++)
     {
          if (pthread_join(thread_ids[i], &status) != 0) {
               printf("Error joining thread in readers, status %ld", (long)status);
               return exit(EXIT_FAILURE);
          }
     }
     return;
}

int main(int argc, char *argv[])
{
     int n = atoi(argv[1]); //number of bytes
     int p = atoi(argv[2]); //number of threads

     // @create a file for saving the data
     int fd = open(FILE_PATH, O_CREAT | O_WRONLY | O_TRUNC, 0666); //Create the file if it doesn't already exist and if so overwrite it
     if (fd < 0)
     {
          perror("File not created");
          exit(EXIT_FAILURE);
     }
     
     printf("open/create file\n");
     // @allocate a buffer and initialize it with 1 extra spot for the Null-terminator
     char* ptr = (char*)calloc(n+1, sizeof(char));
     if (ptr == NULL){
          perror("Memory allocation in ptr failed, canceling code");
          exit(EXIT_FAILURE);
     }
     printf("buffer created, ptr = %d\n", *ptr);
     // @create two lists of 100 requests in the format of [offset, bytes]

     // @List 1: sequtial requests of 16384 bytes, where offset_n = offset_(n-1) + 16384
     // @e.g., [0, 16384], [16384, 16384], [32768, 16384] ...
     // @ensure no overlapping among these requests.
     //allocate memory list1
     request_t *list1 = malloc(NUM_REQUESTS * sizeof(request_t)); 
     if (list1 == NULL){
          perror("Memory allocation in list1 failed, canceling code");
          exit(EXIT_FAILURE);
     }
     //initialize list1
     for(int i = 0; i < NUM_REQUESTS; i++){
          list1[i].offset = i * REQUEST_SIZE;
          list1[i].bytes = REQUEST_SIZE;
     }
     printf("list1 initialized\n");

     // @List 2: random requests of 128 bytes, where offset_n = random[0,N/4096] * 4096
     // @e.g., [4096, 128], [16384, 128], [32768, 128], etc.
     // @ensure no overlapping among these requests.
     
     //allocate memory list2
     request_t *list2 = malloc(NUM_REQUESTS * sizeof(request_t));
     if (list2 == NULL){
          perror("Memory allocation in list2 failed, canceling code");
          exit(EXIT_FAILURE);
     }
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
     printf("list2 initialized\n");

     // @start timing 
     double start_time = (double)clock();
     printf("clock started\n");
     /* Create writer workers and pass in their portion of list1. Then wait for them to finish */ 
     run_writers(list1, n, p, FILE_PATH, ptr, fd);

     // @close the file 
     close(fd);

     // @end timing 
     double time = (double)clock() - start_time;

     double MB = n/1000000;
     //@Print out the write bandwidth
     printf("Write %f MB, use %d threads, elapsed time %f s, write bandwidth: %f MB/s \n", MB, p, time, MB/time);
     
     // @reopen the file 
     fd = open(FILE_PATH, O_RDONLY);

     // @start timing 
     start_time = (double)clock();
     /* Create reader workers and pass in their portion of list1. Then wait for them to finish */   
     run_readers(list1, n, p, FILE_PATH, ptr, fd);
     

     // @close the file 
     close(fd);

     // @end timing 

     time = (double)clock() - start_time;
     //@Print out the read bandwidth
     printf("Read %f MB, use %d threads, elapsed time %f s, write bandwidth: %f MB/s \n", MB, p, time, MB/time);


     // @Repeat the write and read test now using List2 
     fd = open(FILE_PATH, O_RDWR, 0666);
     // @start timing 
     start_time = (double)clock();

     /* Create writer workers and pass in their portion of list1. Then wait for them to finish */ 
     run_writers(list2, n, p, FILE_PATH, ptr, fd);

     // @close the file 
     close(fd);

     // @end timing 
     time = (double)clock() - start_time;

     
     //@Print out the write bandwidth
     printf("Write %f MB, use %d threads, elapsed time %f s, write bandwidth: %f MB/s \n", MB, p, time, MB/time);
     
     // @reopen the file 
     fd = open(FILE_PATH, O_RDONLY);

     // @start timing 
     start_time = (double)clock();
     /* Create reader workers and pass in their portion of list1. Then wait for them to finish */   
     run_readers(list2, n, p, FILE_PATH, ptr, fd);
     

     // @close the file 
     close(fd);

     // @end timing 

     time = (double)clock() - start_time;
     //@Print out the read bandwidth
     printf("Read %f MB, use %d threads, elapsed time %f s, write bandwidth: %f MB/s \n", MB, p, time, MB/time);

     /*free up resources properly */
     free(list1);
     free(list2);
     free(ptr);
}
