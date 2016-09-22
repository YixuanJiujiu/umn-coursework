// Name: Maxwell Pung
// Login: pungx013
// Assignment 1

#include <stdio.h>	
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>	// for errno, perror
#include <signal.h>	// for signal
#include <sys/time.h>
#include <string.h>
#include <sys/ipc.h>	// for shared memory
#include <sys/shm.h>	// for shared memory

#define BUF_SIZE 1024
#define DEBUG

int ProducerProcess(char *fname, pid_t cpid);
int ConsumerProcess(pid_t ppid, key_t key);

struct timeval totaltime;
struct timeval inittv;
struct timeval posttv;
char buffer[BUF_SIZE];		// Stores data tranferred between producer & consumer.
int count = 0;			// Indicates number of bytes present in buffer. -1 indicates EOF.
static int preceived = 0;
static int creceived = 0;
key_t key = 4455;

void SigHandler(int signum)
{
	if (signum == SIGUSR1)
	{
		creceived =  1;
		printf("Recieved SIGUSR1!\n");
	}
	else if (signum == SIGUSR2)
	{
		preceived = 1;
		printf("Recieved SIGUSR2!\n");
	}
}

int main (int argc, char *argv[])
{
	// Verify usage is correct.
	if (argc < 2)
	{
		printf("Usage: prodcon <some_file_name>\n");
		return EXIT_FAILURE;
	}

	signal(SIGUSR1, SigHandler);
	signal(SIGUSR2, SigHandler);

	// Configure processes.
	char *fname = argv[1];
	pid_t ppid = getpid();
	pid_t cpid;
	cpid = fork();

	if (cpid < 0) 
	{ 
		perror("(m) ERROR");
		return EXIT_FAILURE;
	}
	else if (cpid == 0) 
	{ 
		#ifdef DEBUG
		printf("(m) fork return value: %d\n", (int)cpid);
		#endif

		printf("(m) Launching consumer\n");
		int cres = ConsumerProcess(ppid, key);
		if (cres == 1)
		{
			printf("(m) Consumer process failed\n");
		}
	}
	else 
	{ 
		#ifdef DEBUG
		printf("(m) fork return value: %d\n", (int)cpid);
		#endif

		printf("(m) Launching producer\n");
		int pres = ProducerProcess(fname, cpid);
		if (pres == 1)
		{
			printf("(m) Producer process failed\n");
		}
	}
	
	return EXIT_SUCCESS;
}

int ProducerProcess(char *fname, pid_t cpid)
{

	#ifdef DEBUG
	printf("(p) input file name: %s\n", fname);
	#endif 

	// Open file specified by argv[1] for reading.
	FILE *fp;	

	fp = fopen(fname, "rw"); 
	if (fp == NULL)
	{
		perror("(p) ERROR [fopen failed]");
		return 1;
	}

	// Create the segment.
	int shmid = 0;
	int shmflag = 1023;
	int shmsize = sizeof(buffer) + sizeof(int) + sizeof(struct timeval);

	#ifdef DEBUG
	printf("(p) fp: %p\n", fp);
	printf("(p) buffer size is %d\n", sizeof(buffer));
	printf("(p) size of shm is %d\n", shmsize);
	#endif
 
	shmid = shmget(key, shmsize, shmflag);
	if (shmid  < 0)
	{
		perror("ERROR [shmget failed]");
		fclose(fp);
		return 1;
	}

	#ifdef DEBUG
	printf("(p) shmid: %d\n", shmid);
	#endif

	// Attach segment to producer data space.
	char *shm;	// Location of shared memory space (precisely, the buffer)
	int *count;	// Location of count in shared memory space
	char *time;	// Location of timeval struct in shared memory space

	shm = shmat(shmid, (void *) NULL, shmflag);
	if (shm  == (char *) - 1)
	{
		perror("(c) ERROR [shmat failed]");
		fclose(fp);
		return 1;
	}

	// Put input into memory for consumer to read.
	char *prodbuf = NULL;
	size_t bytesread;
	int bytesshared = 0;

	count = shm + sizeof(buffer);

	#ifdef DEBUG
	printf("(p) shm: %p\n", shm);
	printf("(p) count: %p\n", count);
	printf("(p) diff: %d\n", (int)count - (int)shm);
	#endif

	// Goto end of file.
	int seekres = 0;
	printf("(p) seek result init: %d\n", seekres);
	printf("(p) fp: %p\n", fp);
	seekres = fseek(fp, 0L, SEEK_END); 
	
	#ifdef DEBUG
	printf("(p) seek result: %d\n", seekres);
	#endif

	if (seekres == -1)	
	{
		perror("(p) ERROR [fseek failed]");
		fclose(fp);
		shmdt((void *) shm);
		return 1;
	}

	#ifdef DEBUG
	printf("(p) [fseek successful]\n");
	#endif

	// Get size of file.
	long filesize = ftell(fp);	
	if (filesize == -1) 
	{ 
		perror("(p) ERROR [ftell failed]");	
		fclose(fp);
		shmdt((void *) shm);
		return 1;
	}

	#ifdef DEBUG
	printf("(p) filesize %ld\n", filesize);
	#endif 
	
	// Create space for file contents in producer buffer.
	prodbuf = malloc(sizeof(char) * (filesize + 1));
	if (prodbuf == NULL)
	{
		perror("(p) ERROR [malloc failed]");
		fclose(fp);
		shmdt((void *) shm);
		return 1;
	}
	
	// Goto start of file.
	int fseekres = fseek(fp, 0L, SEEK_SET);  
	if (fseekres != 0)		{
		perror("(p) ERROR [fseek failed]");
		fclose(fp);
		shmdt((void *) shm);
		free(prodbuf);
		return 1;
	}

	// Read entire file into producer memory.
	bytesread = fread(prodbuf, sizeof(char), filesize, fp);
	if (bytesread == 0)
	{
		printf("(p) WARNING: fread returned 0");
	}

	if (ferror(fp) != 0)
	{
		perror("(p) ERROR [ferror failed]");
		fclose(fp);
		shmdt((void *) shm);
		free(prodbuf);
		return 1;
	}
	else
	{
		prodbuf[bytesread++] = '\0';
	}
	
	#ifdef DEBUG
	printf("(p) bytes read: %i\n", bytesread);
	#endif
	fclose(fp);

	while (bytesread > bytesshared)
	{
		#ifdef DEBUG
		printf("(p) bytes read: %d\n", (int)bytesread);
		printf("(p) bytes shared: %d\n", bytesshared);
		#endif
		// Write the data into the buffer in the shared memory.
		if (bytesread < BUF_SIZE)
		{	
			memmove(shm, prodbuf, bytesread);
			*count = bytesread;
			gettimeofday(&inittv, NULL);
			bytesshared += bytesread;
		}
		else 
		{
			memmove(shm, prodbuf, BUF_SIZE);
			*count = BUF_SIZE;
			gettimeofday(&inittv, NULL);
			bytesshared += BUF_SIZE;
		}

		#ifdef DEBUG
		printf("(p) bytesshared: %d\n", bytesshared);
		printf("(p) shm string: %c\n", (char)*shm);
		printf("(p) shm string: %c\n", (char)*(shm + 1));
		printf("(p) shm string: %c\n", (char)*(shm + 2));
		printf("(p) shm count: %d\n", (int)*count);
		#endif 
		
		// Send SIGUSR1 to consumer.
		int killres = kill(cpid, SIGUSR1);

		#ifdef DEBUG
		printf("(p) kill result: %d\n", killres);
		#endif
		if (killres != 0)
		{
			perror("(p) ERROR [kill SIGUSR1 failed]");
			fclose(fp);
			shmdt((void *) shm);
			free(prodbuf);
			return 1;
		}

		#ifdef DEBUG
		printf("(p) SIGUSR1 sent to consumer.\n");
		#endif 

		// Wait for SIGUSR2 from consumer process.
		while (*count != 0) 
		{ 
			while (!preceived) { }
		}	

		#ifdef DEBUG
		printf("(p) SIGUSR2 received from consumer.\n");
		#endif 
	}

	free(prodbuf);
	shmdt((void *) shm);
	count = -1;
	return 0;
}

int ConsumerProcess(pid_t ppid, key_t key)
{
	
	// Open file to write buffer contents to.
	FILE *outfp;
	outfp = fopen("output.txt", "w");
	if (outfp == NULL)
	{
		perror("ERROR [fopen failed]");
		return 1;
	}
	printf("(c): output file open\n");

	// Wait for SIGUSR1 from producer process.
	printf("(c): waiting for signal\n");
	//while (*count == 0)
	//{
		while (!creceived);
	//}
	printf("(c) received signal");		
	// Get segment id of shared memory segment created by parent.
	int id = shmget(key, 0, 0);
	if (id == -1)
	{
		perror("(c) [shmget failed]");
		fclose(outfp);
		return 1;
	}

	#ifdef DEBUG
	printf("(c) shmid: %d\n", id);
	#endif

	// Attach shared memory segment to address space.
	char *shmptr = shmat(id, (void *) NULL, 1023);
	if (shmptr == (void *) - 1)
	{
		perror("(c) [shmat failed]");
		fclose(outfp);
		return 1;
	}

	int *count = shmptr + sizeof(buffer);
	
	#ifdef DEBUG
	printf("(c): received signal from producer.");
	printf("(c) shmptr: %p\n", shmptr);
	printf("(c) shm string: %c\n", (char) *shmptr);
	printf("(c) shm string: %c\n", (char) *(shmptr + 1));
	printf("(c) shm string: %c\n", (char) *(shmptr + 2));
	printf("(c) shm count: %d\n", (int) *count);
	#endif

	// Write buffer in shared memory to output file.
	int wres = write(fileno(outfp), shmptr, (sizeof(char) * (*count) - 1));
	if (wres > 0)
	{
		perror("(c) ERROR [write failed]");
	}
	
	fclose(outfp);
	gettimeofday(&posttv, NULL);
	totaltime.tv_usec = posttv.tv_usec - inittv.tv_usec;
	printf("(c) time in queue (microseconds): %ld\n", (long int)totaltime.tv_usec);
	*count = 0;
	
	#ifdef DEBUG
	printf("(c) finished writing output\n");
	#endif
	kill(ppid, SIGUSR2);

	while (*count != -1) { }	// Wait for producer process to terminate.
	shmdt((void *)shmptr);
	shmctl(id, IPC_RMID, NULL);

	return 0;
}
