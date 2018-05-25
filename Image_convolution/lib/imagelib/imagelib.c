
#include "imagelib.h"

#include <math.h>
#include <png.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

void img_destroy(Image *img) {
  for (int i = 0; i < img->height; i++) {
    free(img->pixels[i]);
  }
  free(img->pixels);
  free(img);
}

static void img_normalize(Image *img) {
  // return;
  double max_R = -INFINITY;
  double max_G = -INFINITY;
  double max_B = -INFINITY;
  double min_R = +INFINITY;
  double min_G = +INFINITY;
  double min_B = +INFINITY;

  for (int row = 0; row < img->height; row++) {
    for (int col = 0; col < img->width; col++) {
      Color c = img->pixels[row][col];

      max_R = fmax(c.R, max_R);
      max_G = fmax(c.G, max_G);
      max_B = fmax(c.B, max_B);
      min_R = fmin(c.R, min_R);
      min_G = fmin(c.G, min_G);
      min_B = fmin(c.B, min_B);
    }
  }

  for (int row = 0; row < img->height; row++) {
    for (int col = 0; col < img->width; col++) {
      Color *c = &(img->pixels[row][col]);

      if (max_R - min_R)
        c->R = (c->R - min_R) / (max_R - min_R);
      if (max_G - min_G)
        c->G = (c->G - min_G) / (max_G - min_G);
      if (max_B - min_B)
        c->B = (c->B - min_B) / (max_B - min_B);
    }
  }
}

Image *img_png_read_from_file(char *filename) {
  Image *img = malloc(sizeof(Image));

  FILE *fp = fopen(filename, "rb");

  png_structp png =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png)
    abort();

  png_infop info = png_create_info_struct(png);
  if (!info)
    abort();

  if (setjmp(png_jmpbuf(png)))
    abort();

  png_init_io(png, fp);

  png_read_info(png, info);

  int width = png_get_image_width(png, info);
  int height = png_get_image_height(png, info);
  uint8_t color_type = png_get_color_type(png, info);
  uint8_t bit_depth = png_get_bit_depth(png, info);

  if (bit_depth == 16)
    png_set_strip_16(png);

  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png);

  // PNG_COLOR_TYPE_GRAY_ALPHA is always 8 or 16bit depth.
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png);

  if (png_get_valid(png, info, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png);

  // These color_type don't have an alpha channel then fill it with 0xff.
  if (color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_filler(png, 0xFF, PNG_FILLER_AFTER);

  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png);

  png_read_update_info(png, info);

  uint8_t **row_pointers = calloc(height, sizeof(uint8_t *));
  for (int row = 0; row < height; row++) {
    row_pointers[row] = calloc(width * 4, sizeof(uint8_t));
  }

  png_read_image(png, row_pointers);

  img->pixels = calloc(height, sizeof(Color *));
  for (int row = 0; row < height; row++) {
    img->pixels[row] = calloc(width, sizeof(Color));
    for (int col = 0; col < width; col++) {
      uint8_t *px = &(row_pointers[row][col * 4]);
      img->pixels[row][col].R = ((float)(px[0])) / 255.f;
      img->pixels[row][col].G = ((float)(px[1])) / 255.f;
      img->pixels[row][col].B = ((float)(px[2])) / 255.f;
    }
    free(row_pointers[row]);
  }
  free(row_pointers);

  fclose(fp);

  png_destroy_read_struct(&png, &info, NULL);

  img->width = width;
  img->height = height;

  return img;
}


void img_png_write_to_file(Image *img, char *filename) {
  img_normalize(img);

  FILE *fp = fopen(filename, "wb");
  if (!fp)
    abort();

  png_structp png =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png)
    abort();

  png_infop info = png_create_info_struct(png);
  if (!info)
    abort();

  if (setjmp(png_jmpbuf(png)))
    abort();

  png_init_io(png, fp);

  // Output is 8bit depth, RGBA format.
  png_set_IHDR(png, info, img->width, img->height, 8, PNG_COLOR_TYPE_RGBA,
               PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT,
               PNG_FILTER_TYPE_DEFAULT);
  png_write_info(png, info);

  uint8_t **row_pointers = calloc(img->height, sizeof(uint8_t *));
  for (int row = 0; row < img->height; row++) {
    row_pointers[row] = calloc(img->width * 4, sizeof(uint8_t));
    for (int col = 0; col < img->width; col++) {
      uint8_t *px = &(row_pointers[row][col * 4]);
      px[0] = (uint8_t)(img->pixels[row][col].R * 255.f);
      px[1] = (uint8_t)(img->pixels[row][col].G * 255.f);
      px[2] = (uint8_t)(img->pixels[row][col].B * 255.f);
      px[3] = 255;
    }
  }

  png_write_image(png, row_pointers);
  png_write_end(png, NULL);

  for (int row = 0; row < img->height; row++) {
    free(row_pointers[row]);
  }
  free(row_pointers);

  fclose(fp);

  if (png && info)
    png_destroy_write_struct(&png, &info);
}
