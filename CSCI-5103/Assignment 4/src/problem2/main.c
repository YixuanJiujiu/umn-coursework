/* CSci5103 Fall 2016i
* Assignment# 4
* name: Maxwell Pung
* student id: 4238845
* x500 id: pungx013
* CSELABS machine: <xyz.cselabs.umn.edu>
*/

#include <unistd.h>	// for fork, execl...
#include <stdio.h>	// for file IO
#include <sys/shm.h>	// for shared memory segment
#include <sys/ipc.h>	// for ftok
#include <string.h>	// for printing errors (strerror)
#include <errno.h>	// for printing errors (errno)
#include <stdlib.h>	// for exit
#include <sys/types.h>	// for wait
#include <sys/wait.h>	// for wait
#include <pthread.h>	// for condition variables & mutex

#define DEBUG
#define PRODUCER "producer"
#define CONSUMER "consumer"
#define SEM_NAME "lock"
#define MAX_ITEM_SIZE (sizeof(black) + (2 * sizeof(char)) + sizeof(long int)) 

// colors & arguments for passing to producer/consumer
static const char black[] = "BLACK";
static const char white[] = "WHITE";
static const char red[] = "RED";
static const char shmkey[] = "4455";

typedef struct buffer
{
	char item_buffer[2][MAX_ITEM_SIZE];
	pthread_mutex_t lock;
	pthread_cond_t space_available, item_available;
	int buf_count;
	int finished_producers;	
} shm_buffer;

int main(int argc, char *argv[])
{


	// create shared memory
	key_t key = 4455;	// arbitrary integer
	int shmflg = 1023;	// all permissions & modes set
	int shmid;		// shared memory identifier
	shm_buffer *shmptr;		// pointer to shared memory segment
	//int size = 2 * (MAX_ITEM_SIZE + sizeof(int));
	
	if ((shmid = shmget(key, sizeof(shm_buffer), shmflg)) == -1)
	{
		fprintf(stderr, "parent shmget failed: %s\n", strerror(errno));
		exit(1);
	}
	if ((shmptr = (shm_buffer *) shmat(shmid, (void *) NULL, 1023)) == (void *) -1)
	{
		fprintf(stderr, "parent shmat failed: %s\n", strerror(errno));
		exit(1);
	}

	pthread_mutexattr_t mutex_attr;
	pthread_condattr_t cond_attr;
	pthread_mutexattr_init(&mutex_attr);
	pthread_condattr_init(&cond_attr);
	pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED);
	pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED);
	pthread_mutex_init(&(shmptr->lock), &mutex_attr);
	pthread_cond_init(&(shmptr->space_available), &cond_attr);
	pthread_cond_init(&(shmptr->item_available), &cond_attr);
	pthread_mutexattr_destroy(&mutex_attr);
	pthread_condattr_destroy(&cond_attr);

	// fork processes
	pid_t producer0_pid, producer1_pid, producer2_pid, consumer_pid;
	int exec_result = 0;
	
	// fork black producer process
	if ((producer0_pid = fork()) == 0)
	{
		//printf("producer black child!!!\n");
		if ((exec_result = execl("./producer", "producer", black, shmkey, NULL)) < 0)
		{
			fprintf(stderr, "black producer execl failed: %s\n", strerror(errno));
			exit(1);
		}
	}
	else if (producer0_pid < 0)
	{
		fprintf(stderr, "black producer fork failed: %s\n", strerror(errno));
		exit(1);
	}
	else 
	{
		// fork consumer process 
		if ((consumer_pid = fork()) == 0)
		{
			//printf("consumer child!!!\n");
			if ((exec_result = execl("./consumer", "consumer", shmkey, NULL)) < 0)
			{
				fprintf(stderr, "consumer execl failed: %s\n", strerror(errno));
				exit(1);
			}
		}
		else if (consumer_pid < 0)
		{
			fprintf(stderr, "consumer fork failed: %s\n", strerror(errno));
			exit(1);
		}
		else
		{
			// fork white producer process
			if ((producer1_pid = fork()) == 0)
			{
				//printf("producer white child!!!\n");
				if ((exec_result = execl("./producer", "producer", white, shmkey, NULL)) < 0)
				{
					fprintf(stderr, "white producer execl failed: %s\n", strerror(errno));
					exit(1);
				}
			}
			else if (producer1_pid < 0)
			{
				fprintf(stderr, "white producer fork failed: %s\n", strerror(errno));
				exit(1);
			}
			else
			{

				// fork red producer process
				if ((producer2_pid = fork()) == 0)
				{
					//printf("producer red child!!!\n");
					if ((exec_result = execl("./producer", "producer", red, shmkey, NULL)) < 0)
					{
						fprintf(stderr, "red producer execl failed: %s\n", strerror(errno));
						exit(1);
					}
				}
				else if (producer2_pid < 0)
				{
					fprintf(stderr, "red producer fork failed: %s\n", strerror(errno));
					exit(1);
				}
			}
			
			printf("Forking finished!!!\n");
		}
	}
	pid_t wait_pid;
	int status = 0;
	while ((wait_pid = wait(&status)) > 0) { /* wait for all child processes to finish */ }

	pthread_mutex_destroy(&(shmptr->lock));
	pthread_cond_destroy(&(shmptr->space_available));
	pthread_cond_destroy(&(shmptr->item_available));
	shmdt((void *) shmptr);
	shmctl(shmid, IPC_RMID, NULL);	
	printf("parent finished executing\n");
	
return 0;
}

