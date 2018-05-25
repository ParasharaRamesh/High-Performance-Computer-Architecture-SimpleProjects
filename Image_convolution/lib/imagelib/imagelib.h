#ifndef T0_LIB_IMAGE
#define T0_LIB_IMAGE

struct color
{
	float R;
	float G;
	float B;
};

typedef struct color Color;

struct image
{
	
	Color** pixels;
	int width;
	int height;
};

typedef struct image Image;


Image* img_png_read_from_file (char* filename);

void   img_png_write_to_file  (Image* img, char* filename);

void img_destroy(Image *img);

#endif 
