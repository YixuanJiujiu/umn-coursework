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

#define DEBUG

static const char blackf[] = "producer_black.txt";
static const char whitef[] = "producer_white.txt";
static const char redf[] = "producer_red.txt";
static const char conf[] = "consumer.txt";
static const char black[] = "BLACK";
static const char white[] = "WHITE";
static const char red[] = "RED";
struct timeval timestamp;

int main(int argc, char *argv[])
{
	// create/open log files for processes
	FILE *black_prod_fp, *white_prod_fp, *red_prod_fp, *con_fp;
	if ((black_prod_fp = fopen(blackf, "w")) == NULL)
	{
		printf("error opening %s: %s\n", blackf, strerror(errno));
	}
	if ((white_prod_fp = fopen(whitef, "w")) == NULL)
	{
		printf("error opening %s: %s\n", whitef, strerror(errno));
	}
	
	if ((red_prod_fp = fopen(redf, "w")) == NULL)
	{
		printf("error opening %s: %s\n", redf, strerror(errno));
	}
	
	if ((con_fp = fopen(conf, "w")) == NULL)
	{
		printf("error opening %s: %s\n", conf, strerror(errno));
	}

	// create shared memory
	key_t key = 4455;	// arbitrary integer
	int shmflg = 1023;	// all permissions & modes set
	int shmid;		// shared memory identifier
	int *shmptr;		// pointer to shared memory segment
	int size = 2 * (sizeof(black) + sizeof(char) + sizeof(timestamp.tv_usec));
	
	if ((shmid = shmget(key, size, shmflg)) == -1)
	{
		perror("shmget failed");
		exit(1);
	}
	#ifdef DEBUG
	printf("Size of shm: %d\n", size);
	printf("shmem aquired id: %d\n", shmid);
	#endif

	if ((shmptr = shmat(shmid, (void *) NULL, 1023)) == (void *) -1)
	{
		perror("shmat failed");
		exit(2);
	}
	#ifdef DEBUG
	printf("shmptr set: %p\n", shmptr);
	#endif

	// create posix locks & threads

	// close log files
	if (fclose(black_prod_fp) != 0)
	{
		printf("error closing %s: %s\n", blackf, strerror(errno));
	}
	if (fclose(white_prod_fp) != 0)
	{
		printf("error closing %s: %s\n", whitef, strerror(errno));
	}
	if (fclose(red_prod_fp) != 0)
	{
		printf("error closing %s: %s\n", redf, strerror(errno));
	}
	if (fclose(con_fp) != 0)
	{
		printf("error closing %s: %s\n", conf, strerror(errno));
	}
	return 0;
}

