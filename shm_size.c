#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>

void *shm_setup(int size)
{
	int shmid;
	void *ret;

	shmid = shmget(IPC_PRIVATE, size, SHM_R | SHM_W);
	if (shmid == -1) {
		printf("can't get shared memory\n");
		exit(1);
	}
	ret = (void *)shmat(shmid, 0, 0);
	if (!ret || ret == (void *)-1) {
		printf("can't attach to shared memory\n");
		return NULL;
	}
	/* the following releases the ipc, but note that this process
	   and all its children will still have access to the memory, its

	   means we don't leave behind lots of shm segments after we exit 

	   See Stevens "advanced programming in unix env" for details
	   */
	shmctl(shmid, IPC_RMID, 0);


	
	return ret;
}

int main(int argc, char *argv[])
{
	volatile char *buf;
	int size;

	if (argc < 2) {
		printf("shm_size <size>\n");
		exit(1);
	}

	size = atoi(argv[1]);

	buf = shm_setup(size);

	if (!buf) {
		printf("shm_setup(%d) failed\n", size);
		exit(1);
	}
	return 0;
}

