enum output_type {
    OUTPUT_MONO,
    OUTPUT_TRUE_A,
    OUTPUT_TRUE_B,
    OUTPUT_TRUE_C
};

enum clear_action {
	CLEAR_NONE,            // Don't clear 
	CLEAR_COMMAND,         // Use 'clear' command
	CLEAR_MOVE_CURSOR_ONLY // Move cursor to left-top of the terminal
};

struct image_size {
    int width;
    int height;
};

struct triple_rgb {
	int red;
	int green;
	int blue;
};

extern void ascii_gen_init(int, char *, int, struct image_size, int, int, struct triple_rgb);
extern int ascii_gen_frame(char *);
