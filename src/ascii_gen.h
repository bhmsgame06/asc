enum {
    OUTPUT_MONO,
    OUTPUT_TRUE_A,
    OUTPUT_TRUE_B,
    OUTPUT_TRUE_C
};

struct image_size {
    int width;
    int height;
};

void ascii_gen_init(int, char, struct image_size);
int ascii_gen_frame(char *);