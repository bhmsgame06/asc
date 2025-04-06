#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <getopt.h>
#include <unistd.h>
#include <sys/stat.h>

#include "ascii_gen.h"

static char *program_name;

static struct option longopts[] = {
	{"output-type",     1, NULL, 't'},
	{"foreground-char", 1, NULL, 'f'},
	{"delay",           1, NULL, 'd'},
	{"do-not-clear",    0, NULL, 'c'},
	{"image-size",      1, NULL, 's'},
	{"loop",            0, NULL, 'l'},
	{"help",            0, NULL, 'h'},
	{0, 0, NULL, 0}
};

static void show_help(int err) {
	fprintf(err == 1 ? stderr : stdout,
		"Usage: [options] [png1] [png2] [png3] ...\n"
		"\n"
		"Available options:\n"
		"\t-t, --output-type=<value>       - color type; must be one of: (0) mono, (1) truea, (2) trueb, (3) truec\n"
		"\t-f, --foreground-char=<char>    - works only with output type truec\n"
		"\t-d, --delay=<value>             - fixed delay between frames (must be of type float)\n"
		"\t-c, --do-not-clear              - don't clear terminal before each frame\n"
		"\t-s, --image-size=<geometry>     - output image size of animation; e.g. 100x50, 30x20...\n"
		"\t-l, --loop                      - enable loop\n"
		"\t-h, --help                      - show a help and exit\n");
}

int main(int argc, char *argv[]) {
	if(argv[0] == NULL)
		program_name = "asc";
	else
		program_name = argv[0];

	static int output_type = OUTPUT_MONO;
	static char foreground_char = '#';
	static char *delay = NULL;
	static int clear = 1;
	static int loop = 0;
	static struct image_size output_size;

	memset(&output_size, 0, sizeof(struct image_size));

	int c;
	while((c = getopt_long(argc, argv, "t:f:d:cs:lh", longopts, NULL)) != -1) {
		switch(c) {
			case 't':
				if(strcmp("mono", optarg) == 0 || strcmp("0", optarg) == 0)
					output_type = OUTPUT_MONO;
				else if(strcmp("truea", optarg) == 0 || strcmp("1", optarg) == 0)
					output_type = OUTPUT_TRUE_A;
				else if(strcmp("trueb", optarg) == 0 || strcmp("2", optarg) == 0)
					output_type = OUTPUT_TRUE_B;
				else if(strcmp("truec", optarg) == 0 || strcmp("3", optarg) == 0)
					output_type = OUTPUT_TRUE_C;
				else {
					fprintf(stderr, "%s: Output type - %s: %s\n\n", program_name, optarg, strerror(errno = EINVAL));
					return 1;
				}
				break;

			case 'f':
				if(optarg == NULL) break;
				foreground_char = *optarg;
				break;

			case 'd':
				delay = optarg;
				break;

			case 'c':
				clear = 0;
				break;

			case 's':
				if(optarg == NULL) break;
				
				char *str;

				str = strtok(optarg, "x");
				if(str == NULL) {
					fprintf(stderr, "%s: Wrong '--image-size' syntax: %s\n", program_name, strerror(errno = EINVAL));
					break;
				}
				output_size.width = atoi(str);

				str = strtok(NULL, "x");
				if(str == NULL) {
					fprintf(stderr, "%s: Wrong '--image-size' syntax: %s\n", program_name, strerror(errno = EINVAL));
					break;
				}
				output_size.height = atoi(str);
				
				break;

			case 'l':
				loop = 1;
				break;

			case 'h':
				show_help(0);
				return 0;

			default:
				show_help(1);
				return 1;
		}
	}

	argv += optind;
	argc -= optind;

	int num_images = 0;
	for(int i = 0; i < argc; i++) {

		struct stat st;
		stat(argv[i], &st);

		if(S_ISDIR(st.st_mode)) {
			fprintf(stderr, "%s: %s: %s\n", program_name, argv[i], strerror(errno = EISDIR));
			argv[i] = NULL;
		}

		if(access(argv[i], F_OK | R_OK) != 0) {
			fprintf(stderr, "%s: %s: %s\n", program_name, argv[i], strerror(errno));
			argv[i] = NULL;
		}

		num_images++;

	}

	if(num_images == 0) {
		show_help(1);
		return 1;
	}

	if(delay) printf("d=%s\n\n", delay);

	ascii_gen_init(output_type, foreground_char, output_size);
	if(loop) printf("while true; do\n\n");

	for(int i = 0; i < argc; i++) {

		if(argv[i] != NULL) {
			if(clear) printf("clear\n");
			if(ascii_gen_frame(argv[i])) {
				fprintf(stderr, "%s: At file - %s: %s\n", program_name, argv[i], strerror(errno = EINVAL));
				return 1;
			}
			if(delay) printf("sleep $d\n");
		}

	}

	if(loop) printf("done\n");

	return 0;
}
