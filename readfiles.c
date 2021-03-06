#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>


static struct timeval tp1,tp2;
static size_t block_size = 64 * 1024;
static int show_latency;
static double min_latency, max_latency, total_time;
static unsigned num_blocks;

static void start_timer()
{
	gettimeofday(&tp1,NULL);
}

static double end_timer()
{
	gettimeofday(&tp2,NULL);
	return((tp2.tv_sec - tp1.tv_sec) + 
	       (tp2.tv_usec - tp1.tv_usec)*1.0e-6);
}


static void read_file(char *fname)
{
	int fd;
	static double total, thisrun;
	int n;
	char *buf;
	double last_time;

	buf = malloc(block_size);

	if (!buf) {
		printf("Malloc of %d failed\n", block_size);
		exit(1);
	}

	fd = open(fname, O_RDONLY);
	if (fd == -1) {
		perror(fname);
		free(buf);
		return;
	}

	if (show_latency) {
		last_time = end_timer();
	}

	while ((n = read(fd, buf, block_size)) > 0) {
		total += n;
		thisrun += n;
		if (show_latency) {
			double t = end_timer();
			double latency = t - last_time;
			if (latency > max_latency) {
				max_latency = latency;
			}
			if (min_latency == 0 ||
			    latency < min_latency) {
				min_latency = latency;
			}
			last_time = t;
		}
		if (end_timer() >= 1.0) {
			if (show_latency) {
				printf("%d MB    %g MB/sec  minlat=%.3f ms  maxlat=%.3f ms\n", 
				       (int)(total/1.0e6),
				       (thisrun*1.0e-6)/end_timer(),
				       min_latency * 1000, max_latency * 1000);
			} else {
				printf("%d MB    %g MB/sec\n", 
				       (int)(total/1.0e6),
				       (thisrun*1.0e-6)/end_timer());
			}

			start_timer();
			last_time = 0;
			thisrun = 0;
			min_latency = max_latency = 0;
		}
	}

	free(buf);
	close(fd);
}


static void usage(void)
{
	printf("\n" \
"readfiles - reads from a list of files, showing read throughput\n" \
"\n" \
"Usage: readfiles [options] <files>\n" \
"\n" \
"Options:\n" \
"    -B size        set the block size in bytes\n" \
"    -L             show latencies\n" \
"");
}

int main(int argc, char *argv[])
{
	int i;
	extern char *optarg;
	extern int optind;
	int c;

	while ((c = getopt(argc, argv, "B:hL")) != -1) {
		switch (c) {
		case 'B':
			block_size = strtol(optarg, NULL, 0);
			break;
		case 'L':
			show_latency = 1;
			break;
		case 'h':
		default:
			usage();
			exit(1);
		}
	}

	argc -= optind;
	argv += optind;

	if (argc == 0) {
		usage();
		exit(1);
	}

	start_timer();

	while (1) {
		for (i=0; i < argc; i++) {
			read_file(argv[i]);
		}
	}

	return 0;
}

