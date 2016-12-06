#include <pthread.h>    // for posix threads
#include <unistd.h>
#include <stdio.h>      // for file IO
#include <string.h>	// for printing errors (strerror)
#include <errno.h>	// for printing errors (errno)
#include <stdlib.h> 	// for exit
#include <fcntl.h>	// for...
#include <sys/stat.h>	// for open

#define BUFFER_LENGTH 1024

/*void *producer(void *arg)
{
	printf("Producing a item...\n");
	int fd = open("/dev/buffer", O_WRONLY);
	if (fd < 0)
	{
		perror("Failed to open the device in producer...");
	}
	print("Producer opened device successfully...\n");
	close(fd);
}

void *consumer(void *arg)
{
	printf("Consuming an item...\n");
	int fd = open("/dev/buffer", O_RDONLY);
	if (fd < 0)
	{
		perror("Failed to open device in consumer...");
	}
	print("Consumer opened device successfully...\n");
	close(fd);
}*/

int main(int argc, char* argv[])
{
	printf("Producing a item...\n");
        int pfd = open("/dev/buffer", O_WRONLY);
        if (pfd < 0)
        {
                perror("Failed to open the device in producer...");
        }
        printf("Producer opened device successfully...\n");
	
	printf("Consuming an item...\n");
        int cfd = open("/dev/buffer", O_RDONLY);
        if (cfd < 0)
        {
                perror("Failed to open device in consumer...");
        }
        printf("Consumer opened device successfully...\n");

        close(cfd);
        close(pfd);

}



