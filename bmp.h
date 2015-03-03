#ifndef __bmp_h__
#define __bmp_h__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int read_bmp(const char *filename, int *width, int *height, unsigned char *rgb);
int write_bmp(const char *filename, int width, int height, char *rgb);

#endif

