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
	{"output-type",        1, NULL, 't'},
	{"foreground-string",  1, NULL, 'f'},
	{"one-rgb-for-string", 0, NULL, 'e'},
	{"new-line",           0, NULL, 'n'},
	{"bold",               0, NULL, 'B'},
	{"delay",              1, NULL, 'd'},
	{"clear-action",       1, NULL, 'c'},
	{"background-color",   1, NULL, 'b'},
	{"image-size",         1, NULL, 's'},
	{"loop",               0, NULL, 'l'},
	{"help",               0, NULL, 'h'},
	{0, 0, NULL, 0}
};

static void show_help(int err) {
	fprintf(err == 1 ? stderr : stdout,
		"Usage: %s [options] <png 1> [png 2] [png 3] ...\n" \
		"\n" \
		"Available options:\n" \
		"  -t, --output-type=<value>         - output color type.\n" \
		"                                      Must be one of:\n" \
		"                                      (0) Monochrome;\n" \
		"                                      (1) Truecolor A;\n" \
		"                                      (2) Truecolor B;\n" \
		"                                      (3) Truecolor C.\n" \
		"  -f, --foreground-string=<char>    - works only with output type truec.\n" \
		"  -e, --one-rgb-for-string          - draw one RGB pixel for each string,\n" \
		"                                      otherwise for each char.\n" \
		"  -n, --new-line                    - print new line after displaying frame.\n" \
		"  -B, --bold                        - bold font style.\n" \
		"  -d, --delay=<value>               - fixed delay between frames\n" \
		"                                      (must be of type float).\n" \
		"  -c, --clear-action=<value>        - clear terminal before each frame.\n" \
		"                                      Clear actions:\n" \
		"                                      (0) Don't clear;\n" \
		"                                      (1) Use 'clear' command to clear;\n" \
		"                                      (2) Move cursor to the left-top of the\n" \
		"                                          terminal.\n" \
		"  -b, --background-color=<r,g,b>    - background color.\n" \
		"  -s, --image-size=<geometry>       - output image size of animation,\n" \
		"                                      e.g. 100x50, 30x20...\n" \
		"  -l, --loop                        - enable loop.\n" \
		"  -h, --help                        - show a help and exit.\n", \
		program_name);
}

int main(int argc, char *argv[]) {
	if(argv[0] == NULL)
		program_name = "asc";
	else
		program_name = argv[0];

	int output_type = OUTPUT_MONO;
	char *foreground_string = "#";
	int bold = 0;
	char *delay = NULL;
	int clear_action = CLEAR_COMMAND;
	int loop = 0;
	struct image_size output_size;
	int one_rgb_for_string = 0;
	int lf = 0;
	struct triple_rgb background_color = {0, 0, 0};

	memset(&output_size, 0, sizeof(struct image_size));

	int c;
	while((c = getopt_long(argc, argv, "t:f:enBd:c:b:s:lh", longopts, NULL)) != -1) {

		switch(c) {
			int i;
			char *str;

			case 't': // --output-type
				if(optarg == NULL) break;
				i = atoi(optarg);
				if(i >= sizeof(enum output_type)) {
					fprintf(stderr, "%s: Output type - %s: %s\n\n", program_name, optarg, strerror(errno = EINVAL));
					return errno;
				}
				output_type = i;
				break;

			case 'f': // --foreground-string
				if(optarg == NULL) break;
				foreground_string = strdup(optarg);
				break;

			case 'e': // --one-rgb-for-string
				one_rgb_for_string = 1;
				break;

			case 'n': // --new-line
				lf = 1;
				break;

			case 'B': // --bold
				bold = 1;
				break;

			case 'd': // --delay
				if(optarg == NULL) break;
				delay = strdup(optarg);
				break;

			case 'c': // --clear-action
				if(optarg == NULL) {
					errno = EINVAL;
					return errno;
				}

				i = atoi(optarg);
				if(i >= sizeof(enum clear_action)) {
					fprintf(stderr, "%s: Clear action - %s: %s\n\n", program_name, optarg, strerror(errno = EINVAL));
					return errno;
				}
				clear_action = i;
				break;

			case 'b': // --background-color
				if(optarg == NULL) {
					errno = EINVAL;
					return errno;
				}
				
				str = NULL;

				// Parsing red
				str = strtok(optarg, ",");
				background_color.red = atoi(str);
				if(str == NULL) {
					fprintf(stderr, "%s: Wrong '--background-color' syntax: %s\n", program_name, strerror(errno = EINVAL));
					return errno;
				}

				// Parsing green
				str = strtok(NULL, ",");
				background_color.green = atoi(str);
				if(str == NULL) {
					fprintf(stderr, "%s: Wrong '--background-color' syntax: %s\n", program_name, strerror(errno = EINVAL));
					return errno;
				}
				
				// Parsing blue
				str = strtok(NULL, ",");
				background_color.blue = atoi(str);
				if(str == NULL) {
					fprintf(stderr, "%s: Wrong '--background-color' syntax: %s\n", program_name, strerror(errno = EINVAL));
					return errno;
				}

				break;

			case 's': // --image-size
				if(optarg == NULL) {
					errno = EINVAL;
					return errno;
				}

				str = NULL;

				// Parsing new width
				str = strtok(optarg, "x");
				output_size.width = atoi(str);
				if(str == NULL) {
					fprintf(stderr, "%s: Wrong '--image-size' syntax: %s\n", program_name, strerror(errno = EINVAL));
					return errno;
				}

				// Parsing new height
				str = strtok(NULL, "x");
				output_size.height = atoi(str);
				if(str == NULL) {
					fprintf(stderr, "%s: Wrong '--image-size' syntax: %s\n", program_name, strerror(errno = EINVAL));
					return errno;
				}
				
				break;

			case 'l': // --loop
				loop = 1;
				break;

			case 'h': // --help
				show_help(0);
				return 0;

			default:
				show_help(1);
				return 1;
		}

	}

	argv += optind;
	argc -= optind;
	// Okay, we are done parsing bunch

	// Check all files in argv if they are exist
	int num_images = 0;
	int error = 0;
	for(int i = 0; i < argc; i++) {

		struct stat st;
		stat(argv[i], &st);

		if(access(argv[i], F_OK | R_OK) != 0) {
			fprintf(stderr, "%s: %s: %s\n", program_name, argv[i], strerror(errno));
			argv[i] = NULL;
			error = 1;
			goto skip;
		}

		if(S_ISDIR(st.st_mode)) {
			fprintf(stderr, "%s: %s: %s\n", program_name, argv[i], strerror(errno = EISDIR));
			argv[i] = NULL;
			error = 1;
		}

		num_images++;

skip:

	}

	if(error) {
		return 1;
	}

	// If no image files were passed into argv, print help to stderr and exit
	if(num_images == 0) {
		show_help(1);
		return 1;
	}

	ascii_gen_init(output_type, foreground_string, bold, output_size, one_rgb_for_string, lf, background_color);

	// First set up delay variable in our art script
	if(delay) printf("d=%s\n", delay);

	// If there's loop then set up that
	if(loop) printf("while true\ndo\n");

	for(int i = 0; i < argc; i++) {

		if(argv[i] != NULL) {
			// Clear routine...
			switch(clear_action) {
				case CLEAR_NONE:
					break;

				case CLEAR_COMMAND:
					printf("clear\n");
					break;
				
				case CLEAR_MOVE_CURSOR_ONLY:
					printf("echo -ne \"\\033[0H\"\n");
					break;
			}

			// Finally ASCII generating
			if(ascii_gen_frame(argv[i])) {
				fprintf(stderr, "%s: At file - %s: %s\n", program_name, argv[i], strerror(errno = EINVAL));
				return errno;
			}

			// Ok
			if(delay) printf("sleep $d\n");

		}

	}

	// Ok
	if(loop) printf("done\n");

	return 0;
}
