// asignment 3
/*
  list-forming.c: 
  Each thread generates a data node, attaches it to a global list. This is reapeated for K times.
  There are num_threads threads. The value of "num_threads" is input by the student.
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sched.h>

#define K 200

struct Node
{
    int data;
    struct Node* next;
};

struct list
{
     struct Node * header;
     struct Node * tail;
};

pthread_mutex_t mutex_lock;

struct list *List;

// Thread argument structure - IMPORTANT FIX
typedef struct {
    int cpu_id;
} thread_arg_t;

void bind_thread_to_cpu(int cpuid) 
{
     cpu_set_t mask;
     CPU_ZERO(&mask);

     CPU_SET(cpuid, &mask);
     if (sched_setaffinity(0, sizeof(cpu_set_t), &mask)) 
     {
         fprintf(stderr, "sched_setaffinity failed\n");

     }
}

struct Node* generate_data_node()
{
    struct Node *ptr;
    ptr = (struct Node *)malloc(sizeof(struct Node));    

    if( NULL != ptr ){
        ptr->next = NULL;
        ptr->data = 0;
    }
    else {
        printf("Node allocation failed!\n");
    }
    return ptr;
}

void* producer_thread(void *arg)
{
    thread_arg_t *targ = (thread_arg_t *)arg;
    
    // Bind this thread to a CPU
    bind_thread_to_cpu(targ->cpu_id);

    struct Node *ptr;
    struct Node *local_head = NULL;
    struct Node *local_tail = NULL;
    int counter = 0;  

    while(counter < K)
    {
        ptr = generate_data_node();

        if(NULL != ptr)
        {
            ptr->data = 1;

            if(local_head == NULL)
            {
                local_head = local_tail = ptr;
            }
            else
            {
                local_tail->next = ptr;
                local_tail = ptr;
            }
        }
        ++counter;
    }
    pthread_mutex_lock(&mutex_lock);

    if(local_head != NULL)
    {
        if(List->header == NULL)
        {
            List->header = local_head;
            List->tail = local_tail;
        }
        else
        {
            List->tail->next = local_head;
            List->tail = local_tail;
        }
    }

    pthread_mutex_unlock(&mutex_lock);

    free(targ); // Free the argument structure
    return NULL;
}

int main(int argc, char* argv[])
{
    int i, num_threads;
    int NUM_PROCS;
    struct Node *tmp, *next;
    struct timeval starttime, endtime;

    if(argc < 2) 
    {
       // printf("Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }

    num_threads = atoi(argv[1]);
    
    if(num_threads <= 0) {
        //printf("Invalid number of threads\n");
        return 1;
    }

    pthread_t producer[num_threads];
    
    NUM_PROCS = sysconf(_SC_NPROCESSORS_CONF);
    //printf("Number of CPUs: %d\n", NUM_PROCS);
    //printf("Number of threads: %d\n", num_threads);
    //printf("Nodes per thread: %d\n", K);

    pthread_mutex_init(&mutex_lock, NULL);

    List = (struct list *)malloc(sizeof(struct list));
    if(NULL == List)
    {
       printf("List allocation failed\n");
       exit(1);	
    }
    List->header = List->tail = NULL;

    gettimeofday(&starttime, NULL);
    
    // Create threads with proper argument passing
    for(i = 0; i < num_threads; i++)
    {
        thread_arg_t *arg = (thread_arg_t *)malloc(sizeof(thread_arg_t));
        if(arg == NULL) 
        {
            printf("Failed to allocate thread argument\n");
            exit(1);
        }
        arg->cpu_id = i % NUM_PROCS;
        
        if(pthread_create(&producer[i], NULL, producer_thread, arg) != 0) {
            printf("Failed to create thread %d\n", i);
            free(arg);
            exit(1);
        }
    }

    // join all threads
    for(i = 0; i < num_threads; i++)
    {
        pthread_join(producer[i], NULL);
    }

    gettimeofday(&endtime, NULL);

    // Count nodes to verify
    int node_count = 0;
    tmp = List->header;
    while(tmp != NULL) 
    {
        node_count++;
        tmp = tmp->next;
    }
    printf("Total nodes created: %d (expected: %d)\n", node_count, num_threads * K);

    // Free the list
    if(List->header != NULL)
    {
        tmp = List->header;
        while(tmp != NULL)
        {  
           next = tmp->next;
           free(tmp);
           tmp = next;
        }            
    }
    
    free(List);
    pthread_mutex_destroy(&mutex_lock);
    
    //printf("Total run time is %ld microseconds.\n", (endtime.tv_sec - starttime.tv_sec) * 1000000 + (endtime.tv_usec - starttime.tv_usec));
    
    return 0; 
}
