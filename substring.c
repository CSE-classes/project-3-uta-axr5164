// assignment 2
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 5

char buffer[BUFFER_SIZE];
int in = 0;   // producer 
int out = 0;  // consumer 
int count = 0; // items in buffer

pthread_mutex_t mutex;
pthread_cond_t not_full;  
pthread_cond_t not_empty; 

FILE *fp;

void* producer(void* arg) 
{
    char ch;
    
    fp = fopen("message.txt", "r");
    if (fp == NULL) 
	{
        printf("Error opening file\n");
        return NULL;
    }
    
    while ((ch = fgetc(fp)) != EOF) 
	{
        pthread_mutex_lock(&mutex);
        
        // if buffer is full wait till consumer does its thing
        while (count == BUFFER_SIZE) 
		{
            pthread_cond_wait(&not_full, &mutex);
        }
        
        // add char to buffer
        buffer[in] = ch;
        in = (in + 1) % BUFFER_SIZE;
        count++;
        
        // buffer isnt empty
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mutex);
    }
    
    fclose(fp);
    
    pthread_mutex_lock(&mutex);
    while (count == BUFFER_SIZE) 
	{
        pthread_cond_wait(&not_full, &mutex);
    }
    buffer[in] = EOF;
    in = (in + 1) % BUFFER_SIZE;
    count++;
    pthread_cond_signal(&not_empty);
    pthread_mutex_unlock(&mutex);
    
    return NULL;
}

void* consumer(void* arg) 
{
    char ch;
    
    while (1) 
	{
        pthread_mutex_lock(&mutex);
        
        // makes sure its empty b4 consuming
        while (count == 0) 
		{
            pthread_cond_wait(&not_empty, &mutex);
        }
        
        // remove char
        ch = buffer[out];
        out = (out + 1) % BUFFER_SIZE;
        count--;
        
        // buffer isnt full
        pthread_cond_signal(&not_full);
        
        pthread_mutex_unlock(&mutex);
        
        if (ch == EOF) 
		{
            break;
        }

		// print out char by char, more visual idea of whats going on
        printf("%c\n", ch);
        fflush(stdout);
    }
    
    return NULL;
}

int main() 
{
    pthread_t prod_thread, cons_thread;
    
    pthread_mutex_init(&mutex, NULL);
    pthread_cond_init(&not_full, NULL);
    pthread_cond_init(&not_empty, NULL);
    
    pthread_create(&prod_thread, NULL, producer, NULL);
    pthread_create(&cons_thread, NULL, consumer, NULL);
    
    pthread_join(prod_thread, NULL);
    pthread_join(cons_thread, NULL);
    
    pthread_mutex_destroy(&mutex);
    pthread_cond_destroy(&not_full);
    pthread_cond_destroy(&not_empty);
    
    printf("\n");
    return 0;
}











