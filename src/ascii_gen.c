#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <png.h>

#include "ascii_gen.h"

#define MAP_SIZE 39

const unsigned char brightness_map[MAP_SIZE] = {
	0x00, 0x16, 0x19, 0x1b, 0x21, 0x2c, 0x2f, 0x32, 0x3d, 0x40, 0x4b, 0x50, 0x53, 0x5c, 0x61, 0x64,
	0x67, 0x6a, 0x6c, 0x6f, 0x72, 0x75, 0x77, 0x7a, 0x7d, 0x80, 0x83, 0x85, 0x8b, 0x8e, 0x91, 0x96,
	0x9c, 0xa1, 0xa4, 0xb5, 0xba, 0xbd, 0xef
};

const unsigned char char_map[MAP_SIZE] = {
	' ', '.', '-', ':', '\'', '^', '~', ',', '!', ';', '*', '+', 'v', 'T', 'z', '1',
	'i', 'n', '}', '[', 'k', '5', 'w', 'U', 'h', '4', 'A', '9', 'H', 'b', 'p', 'D',
	'8', '0', '#', '%', '&', 'g', '@'
};

const struct png_color_16_struct color_black = {0, 0, 0, 0, 0};

static struct image_size output_size;
static int output_type;
static unsigned char foreground_char;

static unsigned char *resize_image(unsigned char *org_image, int org_width, int org_height, int new_width, int new_height) {
	unsigned char *new_image = malloc(new_width * new_height * 3);

	double x_inc = (double)org_width / (double)new_width;
	double y_inc = (double)org_height / (double)new_height;

	double tx = 0;
	double ty = 0;

	for(int y = 0; y < new_height; y++) {
		for(int x = 0; x < new_width; x++) {
			new_image[y * new_width * 3 + x * 3] = org_image[(int)ty * org_width * 3 + (int)tx * 3];
			new_image[y * new_width * 3 + x * 3 + 1] = org_image[(int)ty * org_width * 3 + (int)tx * 3 + 1];
			new_image[y * new_width * 3 + x * 3 + 2] = org_image[(int)ty * org_width * 3 + (int)tx * 3 + 2];
			tx += x_inc;
		}
		ty += y_inc;
		tx = 0;
	}

	return new_image;
}

static unsigned char *pixel_address(char *image, int x, int y) {
	return &image[y * output_size.width * 3 + x * 3];
}

static unsigned char bw_pixel(char *image, int x, int y) {
	unsigned char *rgb = pixel_address(image, x, y);

	return (
		*rgb + 
		*(rgb + 1) + 
		*(rgb + 2)
	) / 3;
}

void ascii_gen_init(int ot, char fc, struct image_size is) {
	output_type = ot;
	foreground_char = fc;
	output_size = is;
}

int ascii_gen_frame(char *file) {
	unsigned char *image = NULL;
	unsigned char **rows = NULL;
	png_structp png_ctx = NULL;
	png_infop png_info = NULL;
	unsigned int width, height;
	int bd, ct, im, cm, fm;

	FILE *fd = fopen(file, "rb");

	if(fd == NULL) {
		return 1;
	}

	if((png_ctx = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL)) == NULL) {
		fclose(fd);
		return 1;
	}

	if((png_info = png_create_info_struct(png_ctx)) == NULL) {
		png_destroy_read_struct(&png_ctx, NULL, NULL);
		fclose(fd);
		return 1;
	}

	if(setjmp(png_jmpbuf(png_ctx))) {
		if(image != NULL) free(image);
		if(rows != NULL) free(rows);
		png_destroy_read_struct(&png_ctx, &png_info, NULL);
		fclose(fd);
		return 1;
	}

	png_init_io(png_ctx, fd);

	png_read_info(png_ctx, png_info);
	png_get_IHDR(png_ctx, png_info, &width, &height, &bd, &ct, &im, &cm, &fm);

	if(bd < 8) png_set_expand_gray_1_2_4_to_8(png_ctx);
	if(ct & PNG_COLOR_MASK_PALETTE) png_set_palette_to_rgb(png_ctx);
	if(!(ct & PNG_COLOR_MASK_COLOR)) png_set_gray_to_rgb(png_ctx);
	png_set_strip_alpha(png_ctx);
	png_set_background(png_ctx, &color_black, PNG_BACKGROUND_GAMMA_SCREEN, 0, 0);

	if((image = malloc(width * height * 3)) == NULL) {
		longjmp(png_jmpbuf(png_ctx), 1);
	}

	if((rows = malloc(sizeof(png_bytep) * height)) == NULL) {
		longjmp(png_jmpbuf(png_ctx), 1);
	}

	for(int i = 0; i < height; i++) {
		rows[i] = (width * 3) * i + image;
	}

	png_read_image(png_ctx, rows);
	free(rows);
	rows = NULL;

	if(output_size.width == 0 || output_size.height == 0) {
		output_size.width = width;
		output_size.height = height;
	}

	char *new_image = resize_image(image, width, height, output_size.width, output_size.height);
	free(image);
	image = new_image;

	png_read_end(png_ctx, png_info);

	png_destroy_read_struct(&png_ctx, &png_info, NULL);
	fclose(fd);

	if(output_type == OUTPUT_MONO) {

		for(int y = 0; y < output_size.height; y++) {
			printf("echo \"");
			for(int x = 0; x < output_size.width; x++) {

				unsigned char bw = bw_pixel(image, x, y);

				int i = 0;
				while(i < MAP_SIZE) {
					if(i >= MAP_SIZE - 1) break;
					if(bw >= brightness_map[i] && bw <= brightness_map[i + 1]) break;
					i++;
				}

				printf("%c", char_map[i]);

			}
			printf("\"\n");
		}

	} else if(output_type == OUTPUT_TRUE_A) {

		for(int y = 0; y < output_size.height; y++) {
			printf("echo -e \"");
			for(int x = 0; x < output_size.width; x++) {

				unsigned char *rgb = pixel_address(image, x, y);

				printf("\\e[48;2;%d;%d;%dm ",
					*rgb,
					*(rgb + 1),
					*(rgb + 2)
				);

			}
			printf("\\e[0m\"\n");
		}

	} else if(output_type == OUTPUT_TRUE_B) {

		for(int y = 0; y < output_size.height; y++) {
			printf("echo -e \"");
			for(int x = 0; x < output_size.width; x++) {

				unsigned char *rgb = pixel_address(image, x, y);
				unsigned char bw = bw_pixel(image, x, y);

				int i = 0;
				while(i < MAP_SIZE) {
					if(i >= MAP_SIZE - 1) break;
					if(bw >= brightness_map[i] && bw <= brightness_map[i + 1]) break;
					i++;
				}

				printf("\\e[38;2;%d;%d;%dm%c",
					*rgb,
					*(rgb + 1),
					*(rgb + 2),
					char_map[i]
				);

			}
			printf("\\e[0m\"\n");
		}

	} else if(output_type == OUTPUT_TRUE_C) {

		for(int y = 0; y < output_size.height; y++) {
			printf("echo -e \"");
			for(int x = 0; x < output_size.width; x++) {

				unsigned char *rgb = pixel_address(image, x, y);

				printf("\\e[38;2;%d;%d;%dm%c",
					*rgb,
					*(rgb + 1),
					*(rgb + 2),
					foreground_char
				);

			}
			printf("\\e[0m\"\n");
		}

	}

	free(image);

	return 0;
}
