#include <unistd.h>	// for fork, execl, read..
#include <stdio.h>      // for fprintf..
#include <string.h>	// for printing errors (strerror)..
#include <errno.h>	// for printing errors (errno)..
#include <stdlib.h> 	// for exit, atoi..
#include <fcntl.h>	// for..
#include <sys/stat.h>	// for open, close..
#include <time.h>	// for time..

void wait_for(unsigned int secs)
{
	unsigned int return_time = time(0) + secs;
	while (time(0) < return_time);
}

int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Usage error: consumer [item_count]\n");
		exit(1);
	}

	int item_count = atoi(argv[1]);

	int fd = open("/dev/buffer", O_RDONLY);
	if (fd < 0)
	{
		fprintf(stderr, "Error: failed to open the device in consumer due to...%s\n", strerror(errno));
		exit(1);
	}
	printf("Consumer opened device successfully...\n");
	wait_for(2);
	for (int index = 0; index < item_count; index++)
	{
		char item[32];
		printf("Consumer attempting to read...\n");
		if (read(fd, &item, 32) < 0)
		{
			fprintf(stderr, "Error: consumer read failed due to...%s\n", strerror(errno));
			close(fd);
			exit(1);
		}
		printf("Consumer has read %s\n", item);
	}

	close(fd);
	printf("Consumer closed device successfully...\n");

	exit(0);
}
