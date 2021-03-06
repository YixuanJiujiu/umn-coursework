#include <stdio.h>	// for logging (printf) and IO (fopen, fclose)
#include <string.h>	// for printing errors (strerror)
#include <errno.h>	// for printing errors (errno)
#include <stdlib.h>	// for exit, atoi
#include <sys/shm.h>	// for shmget, shmat
#include <sys/ipc.h>	// for shm flagsi
#include <pthread.h>	// for mutex & conditon variables

#define IPC_ALLOC 0
#define MAX_ITEM_SIZE ((5 * sizeof(char)) + (2 * sizeof(char)) + sizeof(long int))

static const char consumer_fn[] = "consumer.txt";

// See comments for this structure in main.c
typedef struct buffer
{
        char item_buffer[2][MAX_ITEM_SIZE];
        pthread_mutex_t lock;
        pthread_cond_t space_available, item_available;
        int buf_count;
} shm_buffer;

int main (int argc, char* argv[])
{
	// attach to existing shared memory created by parent
	int shmid = 0;
	shm_buffer *shmptr;

	if ((shmid = shmget(atoi(argv[1]), 0, IPC_ALLOC)) == -1)
	{
		fprintf(stderr, "error in consumer getting shared mem id: %s\n", strerror(errno));
		exit(1);
	}
	if ((shmptr = (shm_buffer *) shmat(shmid, (void *) NULL, 1023)) == (void *) -1)
	{
		fprintf(stderr, "error in consumer attaching to shared mem: %s\n", strerror(errno));
		exit(1);
	}

	// create/open log file
	FILE *consumer_fp;
	if ((consumer_fp = fopen(consumer_fn, "a")) == NULL)
	{
		fprintf(stderr, "error opening %s: %s\n", consumer_fn, strerror(errno));
		shmdt((void *) shmptr);
		exit(1);
	}		

	// consume items from buffer until total consumed items is 3000
	int items_removed = 0;
	do
        {
                pthread_mutex_lock(&(shmptr->lock));
                while (shmptr->buf_count == 0)
                        while (pthread_cond_wait(&(shmptr->item_available), &(shmptr->lock)) != 0);

                char item_str[MAX_ITEM_SIZE];
                strcpy(item_str, shmptr->item_buffer[shmptr->buf_count - 1]);
                *(shmptr->item_buffer[shmptr->buf_count - 1]) = NULL;
                shmptr->buf_count--;
		items_removed++;

                pthread_mutex_unlock(&(shmptr->lock));
                pthread_cond_signal(&(shmptr->space_available));
                fprintf(consumer_fp, "%s\n", item_str);
        } while (items_removed < 3000);

	// close log file
	if (fclose(consumer_fp) != 0)
	{
		fprintf(stderr, "error closing %s: %s\n", consumer_fn, strerror(errno));
		shmdt((void *) shmptr);
		exit(1);
	}

	// detach from shared memory & return
	shmdt((void *) shmptr);
	return 0;
}
