#include <sys/time.h>	// for timestamp (gettimeofday)
#include <stdio.h>	// for file IO (fopen, fclose)
#include <sys/shm.h>	// for attaching to shared memory
#include <sys/ipc.h>	// for shared memory flags
#include <string.h>	// for printing errors (strerrior), strcmp
#include <errno.h>	// for printing errors (errno)
#include <stdlib.h>	// for atoi, exit
#include <pthread.h>	// for mutex & conditon variables

const char black_fn[] = "producer_black.txt";
static const char white_fn[] = "producer_white.txt";
static const char red_fn[] = "producer_red.txt";
static const char black[] = "BLACK";
static const char white[] = "WHITE";
static const char red[] = "RED";

#define MAX_ITEM_SIZE (sizeof(black) + (2 * sizeof(char)) + sizeof(long int)) 
#define IPC_ALLOC 0	// for getting a shared mem segment that already exists

// See comments on this structure in main.c
typedef struct buffer
{
        char item_buffer[2][MAX_ITEM_SIZE];
        pthread_mutex_t lock;
        pthread_cond_t space_available, item_available;
        int buf_count;
} shm_buffer;

int main(int argc, char* argv[])
{
	char *color_name = argv[1];	// parent passed us name of color give producer should produce
	int shmid = 0;			// shared memory identifier
	shm_buffer *shmptr;		// pointer to shared memory segment
	
	// attach to existing shared memory
	if ((shmid = shmget(atoi(argv[2]), 0, IPC_ALLOC)) == -1)
	{
		fprintf(stderr, "error in producer getting shared mem id: %s\n", strerror(errno));
		exit(1);
	}
	if ((shmptr = (shm_buffer *) shmat(shmid, (void *) NULL, 1023)) == (void *) -1)
	{
		fprintf(stderr, "error in producer attaching to shared mem: %s\n", strerror(errno));
		exit(1);
	}

	// determine which color to produce and produce it
	int cmp_result = 0;
	if ((cmp_result = strcmp(color_name, black)) == 0)	
	{
		FILE *black_fp;
		if ((black_fp = fopen(black_fn, "a")) == NULL)
		{
			fprintf(stderr, "error opening %s: %s\n", black_fn, strerror(errno));
			shmdt((void *) shmptr);
			exit(1);
		}

		for (int item_index = 0; item_index < 1000; ++item_index)
		{
			pthread_mutex_lock(&(shmptr->lock));
			while (shmptr->buf_count == 2)
				while (pthread_cond_wait(&(shmptr->space_available), &(shmptr->lock)) != 0);

			char item_str[MAX_ITEM_SIZE];
			struct timeval timestamp;
			gettimeofday(&timestamp, NULL);
			snprintf(item_str, MAX_ITEM_SIZE, "%s %li", black, timestamp.tv_usec);	// create item
			strcpy(shmptr->item_buffer[shmptr->buf_count], item_str);		// place item in buffer
			shmptr->buf_count++;

			pthread_mutex_unlock(&(shmptr->lock));
			pthread_cond_signal(&(shmptr->item_available));
			fprintf(black_fp, "%s\n", item_str);	// write to log file
		}

		if (fclose(black_fp) != 0)
		{
			fprintf(stderr, "error closing %s: %s\n", black_fn, strerror(errno));
			shmdt((void *) shmptr);
			exit(1);
		}
	}
	else if ((cmp_result = strcmp(color_name, white)) == 0)
	{
		FILE *white_fp;
		if ((white_fp = fopen(white_fn, "a")) == NULL)
		{
			fprintf(stderr, "error opening %s: %s\n", white_fn, strerror(errno));
			shmdt((void *) shmptr);
			exit(1);
		}

		for (int item_index = 0; item_index < 1000; ++item_index)
		{
			pthread_mutex_lock(&(shmptr->lock));
			while (shmptr->buf_count == 2)
				while (pthread_cond_wait(&(shmptr->space_available), &(shmptr->lock)) != 0);

			char item_str[MAX_ITEM_SIZE];
			struct timeval timestamp;
			gettimeofday(&timestamp, NULL);
			snprintf(item_str, MAX_ITEM_SIZE, "%s %li", white, timestamp.tv_usec);
			strcpy(shmptr->item_buffer[shmptr->buf_count], item_str);            
			shmptr->buf_count++;

			pthread_mutex_unlock(&(shmptr->lock));
			pthread_cond_signal(&(shmptr->item_available));
			fprintf(white_fp, "%s\n", item_str);
		}
		
		if (fclose(white_fp) != 0)
		{
			fprintf(stderr, "error closing %s: %s\n", white_fn, strerror(errno));
			shmdt((void *) shmptr);
			exit(1);
		}
	}
	else if ((cmp_result = strcmp(color_name, red)) == 0)
	{
		FILE *red_fp;
		if ((red_fp = fopen(red_fn, "a")) == NULL)
		{
			fprintf(stderr, "error opening %s: %s\n", red_fn, strerror(errno));
			shmdt((void *) shmptr);
			exit(1);
		}

		for (int item_index = 0; item_index < 1000; ++item_index)
		{
			pthread_mutex_lock(&(shmptr->lock));
			while (shmptr->buf_count == 2)
				while (pthread_cond_wait(&(shmptr->space_available), &(shmptr->lock)) != 0);
			char item_str[MAX_ITEM_SIZE];
			struct timeval timestamp;
			gettimeofday(&timestamp, NULL);
			snprintf(item_str, MAX_ITEM_SIZE, "%s %li", red, timestamp.tv_usec);
			strcpy(shmptr->item_buffer[shmptr->buf_count], item_str);

			shmptr->buf_count++;
			pthread_mutex_unlock(&(shmptr->lock));
			pthread_cond_signal(&(shmptr->item_available));
			fprintf(red_fp, "%s\n", item_str);
		}
		
		if (fclose(red_fp) != 0)
		{
			fprintf(stderr, "error opening %s: %s\n", red_fn, strerror(errno));
			shmdt((void *) shmptr);
			exit(1);
		}
	}
	else 	
	{
		printf("Producer color does not exist!\n");
		shmdt((void *) shmptr);
		exit(1);
	}

	// detach from shared memory segment & return
	shmdt((void *) shmptr);

	return 0;
}

