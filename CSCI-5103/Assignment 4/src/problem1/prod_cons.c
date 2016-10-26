/* CSci5103 Fall 2016
* Assignment# 4
* name: Maxwell Pung
* student id: 4238845
* x500 id: pungx013
* CSELABS machine: <xyz.cselabs.umn.edu>
*/

#include <pthread.h>	// for posix threads
#include <unistd.h>
#include <stdio.h>	// for file IO
#include <sys/shm.h>	// for shared memory segment
#include <sys/time.h>	// for timestamping items (gettimeofday)
#include <string.h>	// for printing errors (strerror)
#include <errno.h>	// for printing errors (errno)
#include <stdlib.h>	// for exit

static const char black_fn[] = "producer_black.txt";
static const char white_fn[] = "producer_white.txt";
static const char red_fn[] = "producer_red.txt";
static const char consumer_fn[] = "consumer.txt";
static const char black[] = "BLACK";
static const char white[] = "WHITE";
static const char red[] = "RED";
#define  MAX_ITEM_SIZE (sizeof(black) + (2 * sizeof(char)) + sizeof(long int))

char buffer[2][MAX_ITEM_SIZE];
pthread_mutex_t lock;
pthread_cond_t space_available, item_available;
int buf_count = 0;

void *producer_black(void *arg)
{
	FILE* log = (FILE*)arg;
	for (int item_index = 0; item_index < 1000; ++item_index)
	{
		pthread_mutex_lock(&lock);
		while (buf_count == 2)
			while (pthread_cond_wait(&space_available, &lock) != 0);
		
		char item_str[MAX_ITEM_SIZE];
		struct timeval timestamp;
		gettimeofday(&timestamp, NULL);
		snprintf(item_str, MAX_ITEM_SIZE, "%s %li", black, timestamp.tv_usec);
		strcpy(buffer[buf_count], item_str);		
		buf_count++;
		
		pthread_mutex_unlock(&lock);
		pthread_cond_signal(&item_available);
		fprintf(log, "%s\n", item_str);
	}
}

void *producer_white(void *arg)
{
	FILE* log = (FILE*)arg;
	for (int item_index = 0; item_index < 1000; ++item_index)
	{
		pthread_mutex_lock(&lock);
		while (buf_count == 2)
			while (pthread_cond_wait(&space_available, &lock) != 0);
		
		char item_str[MAX_ITEM_SIZE];
		struct timeval timestamp;
		gettimeofday(&timestamp, NULL);
		snprintf(item_str, MAX_ITEM_SIZE, "%s %li", white, timestamp.tv_usec);
		strcpy(buffer[buf_count], item_str);		
		buf_count++;
		
		pthread_mutex_unlock(&lock);
		pthread_cond_signal(&item_available);
		fprintf(log, "%s\n", item_str);
	}
}

void *producer_red(void *arg)
{
	FILE* log = (FILE*)arg;
	for (int item_index = 0; item_index < 1000; ++item_index)
	{
		pthread_mutex_lock(&lock);
		while (buf_count == 2)
			while (pthread_cond_wait(&space_available, &lock) != 0);
		
		char item_str[MAX_ITEM_SIZE];
		struct timeval timestamp;
		gettimeofday(&timestamp, NULL);
		snprintf(item_str, MAX_ITEM_SIZE, "%s %li", red, timestamp.tv_usec);
		strcpy(buffer[buf_count], item_str);		
		//printf("Buffer after add: %s\n", buffer[buf_count]);
		buf_count++;
		
		pthread_mutex_unlock(&lock);
		pthread_cond_signal(&item_available);
		fprintf(log, "%s\n", item_str);
	}
}

void *consumer(void *arg)
{
	FILE* log = (FILE*)arg;
	int items_removed = 0;
	do
	{
		pthread_mutex_lock(&lock);
		while (buf_count == 0)
			while (pthread_cond_wait(&item_available, &lock) != 0);

		char item_str[MAX_ITEM_SIZE];
		strcpy(item_str, buffer[buf_count - 1]);
		*buffer[buf_count - 1] = NULL;
		buf_count--;
		items_removed++;

		pthread_mutex_unlock(&lock);
		pthread_cond_signal(&space_available);
		fprintf(log, "%s\n", item_str);
	} while (items_removed < 3000);
}

int main(int argc, char *argv[])
{
	// create/open log files for processes
	FILE *black_fp, *white_fp, *red_fp, *consumer_fp;
	if ((black_fp = fopen(black_fn, "a")) == NULL)
	{
		fprintf(stderr, "error opening %s: %s\n", black_fn, strerror(errno));
		exit(1);
	}

	if ((white_fp = fopen(white_fn, "a")) == NULL)
	{
		fprintf(stderr, "error opening %s: %s\n", white_fn, strerror(errno));
		exit(1);
	}
	
	if ((red_fp = fopen(red_fn, "a")) == NULL)
	{
		fprintf(stderr, "error opening %s: %s\n", red_fn, strerror(errno));	
		exit(1);
	}
	
	if ((consumer_fp = fopen(consumer_fn, "a")) == NULL)
	{
		fprintf(stderr, "error opening %s: %s\n", consumer_fn, strerror(errno));
		exit(1);
	}

	// create posix locks & threads
	pthread_t black_producer_thr, white_producer_thr, red_producer_thr, consumer_thr;
	pthread_attr_t attr;

	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&space_available, NULL);
	pthread_cond_init(&item_available, NULL);
	int pthread_error_num = 0;

	if ((pthread_error_num = pthread_create(&black_producer_thr, NULL, producer_black, (void *) black_fp)))
	{
		fprintf(stderr, "error creating black thread: %s\n", strerror(pthread_error_num));
		exit(1);
	}

	if ((pthread_error_num = pthread_create(&white_producer_thr, NULL, producer_white, (void *) white_fp)))
	{
		fprintf(stderr, "error creating black thread: %s\n", strerror(pthread_error_num));
		exit(1);
	}

	if ((pthread_error_num = pthread_create(&red_producer_thr, NULL, producer_red, (void *) red_fp)))
	{
		fprintf(stderr, "error creating black thread: %s\n", strerror(pthread_error_num));
		exit(1);
	}

	if ((pthread_error_num = pthread_create(&consumer_thr, NULL, consumer, (void *) consumer_fp)))
	{
		fprintf(stderr, "error creating black thread: %s\n", strerror(pthread_error_num));
		exit(1);
	}

	// wait for producers and consumer_thrsumer to finish
	if ((pthread_error_num = pthread_join(consumer_thr, NULL)))
	{
		fprintf(stderr, "error joining consumer thread: %s\n", strerror(pthread_error_num));
		exit(1);
	}

	// close log files
	if (fclose(black_fp) != 0)
	{
		fprintf(stderr, "error closing %s: %s\n", black_fn, strerror(errno));
		exit(1);
	}
	if (fclose(white_fp) != 0)
	{
		fprintf(stderr, "error closing %s: %s\n", white_fn, strerror(errno));
		exit(1);
	}
	if (fclose(red_fp) != 0)
	{
		fprintf(stderr, "error closing %s: %s\n", red_fn, strerror(errno));
		exit(1);
	}
	if (fclose(consumer_fp) != 0)
	{
		fprintf(stderr, "error closing %s: %s\n", consumer_fn, strerror(errno));
		exit(1);
	}

	return 0;
}

