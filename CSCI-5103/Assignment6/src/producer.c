#include <unistd.h>	// for fork, execl...
#include <stdio.h>      // for fprintf..
#include <string.h>	// for strerror, strlen
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
	// verify argument input is correct.
	if (argc != 3)
	{
		printf("Usage error: producer [item_color] [item_count].\n");
		exit(1);
	}

	char *color_name = argv[1];		// get color of item to produce
	
	if (strlen(color_name) != 5)
	{
		printf("Error: invalid color argument, must be 5 characters long (green, black, etc.).\n");
		exit(1);
	}

	int item_count = atoi(argv[2]);		// get number of items to produce
	char items_to_produce[item_count][32];	
	int items_produced = 0;

	// create items to produce
	for (int index = 0; index < item_count; index++)
	{
		char item[32];
		if (items_produced < 9)
		{
			sprintf(item, "%s00000000000000000000000000%d", color_name, index + 1);
		}
		else 
		{
			sprintf(item, "%s0000000000000000000000000%d", color_name, index + 1);
		}
		strcpy(items_to_produce[index], item);
		items_produced++;
		//printf("Producer: created item %s of size %zu bytes\n", item, sizeof(item));
	}
	
	int fd = open("/dev/buffer", O_WRONLY);
	if (fd < 0)
	{
		printf("Error: failed to open the device in producer.\n");
		exit(1);
	}
	printf("Producer opened device successfully...\n");
	wait_for(2);
	for (int index = 0; index < item_count; index++)
	{
		char item[32];
		strcpy(item, items_to_produce[index]);
		printf("Producer is writing %s\n", item);
		if (write(fd, item, sizeof(char) * 32) < 0)
		{
			printf("Error: write by producer failed\n");	// could use perror
			exit(1);
		}
	}
	printf("Producer finished writing to device...\n");

	close(fd);
	printf("Producer closed device successfully...\n");

	exit(0);
}
