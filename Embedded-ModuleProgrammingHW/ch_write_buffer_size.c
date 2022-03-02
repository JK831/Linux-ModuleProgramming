#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>

#define DEVICE_FILE_NAME "/dev/MyBufferedMem"

int main(int argc, char *argv[])
{
	int device;
	int n = atoi(argv[1]);

	device = open(DEVICE_FILE_NAME, O_RDWR|O_NDELAY);
	if (device >=0)
	{
		printf("Device file Open\n");
		ioctl(device, 0, n);
		printf("Changed N: %d\n", n);
	}
	else
		perror("Device file open fail");
	close(device);
	return 0;
}
