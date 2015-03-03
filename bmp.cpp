#include "bmp.h"

struct BMPHeader
{
    char bfType[2];       /* "BM" */
    int bfSize;           /* Size of file in bytes */
    int bfReserved;       /* set to 0 */
    int bfOffBits;        /* Byte offset to actual bitmap data (= 54) */
    int biSize;           /* Size of BITMAPINFOHEADER, in bytes (= 40) */
    int biWidth;          /* Width of image, in pixels */
    int biHeight;         /* Height of images, in pixels */
    short biPlanes;       /* Number of planes in target device (set to 1) */
    short biBitCount;     /* Bits per pixel (24 in this case) */
    int biCompression;    /* Type of compression (0 if no compression) */
    int biSizeImage;      /* Image size, in bytes (0 if no compression) */
    int biXPelsPerMeter;  /* Resolution in pixels/meter of display device */
    int biYPelsPerMeter;  /* Resolution in pixels/meter of display device */
    int biClrUsed;        /* Number of colors in the color table (if 0, use
                             maximum allowed by biBitCount) */
    int biClrImportant;   /* Number of important colors.  If 0, all colors
                             are important */
};

int
read_bmp(const char *filename, int *width, int *height, unsigned char *rgb)
{
    fprintf(stderr, "Sorry, reading of .bmp files isn't supported yet.\n");
    return(0);
}

int
write_bmp(const char *filename, int width, int height, char *rgb)
{
    int i, j, ipos;
    int bytesPerLine;
    unsigned char *line;

    FILE *file;
    struct BMPHeader bmph;

    /* The length of each line must be a multiple of 4 bytes */

    bytesPerLine = (3 * (width + 1) / 4) * 4;

    strcpy(bmph.bfType, "BM");
    bmph.bfOffBits = 54;
    bmph.bfSize = bmph.bfOffBits + bytesPerLine * height;
    bmph.bfReserved = 0;
    bmph.biSize = 40;
    bmph.biWidth = width;
    bmph.biHeight = height;
    bmph.biPlanes = 1;
    bmph.biBitCount = 24;
    bmph.biCompression = 0;
    bmph.biSizeImage = bytesPerLine * height;
    bmph.biXPelsPerMeter = 0;
    bmph.biYPelsPerMeter = 0;
    bmph.biClrUsed = 0;
    bmph.biClrImportant = 0;

    file = fopen (filename, "wb");
    if (file == NULL) return(0);

    fwrite(&bmph.bfType, 2, 1, file);
    fwrite(&bmph.bfSize, 4, 1, file);
    fwrite(&bmph.bfReserved, 4, 1, file);
    fwrite(&bmph.bfOffBits, 4, 1, file);
    fwrite(&bmph.biSize, 4, 1, file);
    fwrite(&bmph.biWidth, 4, 1, file);
    fwrite(&bmph.biHeight, 4, 1, file);
    fwrite(&bmph.biPlanes, 2, 1, file);
    fwrite(&bmph.biBitCount, 2, 1, file);
    fwrite(&bmph.biCompression, 4, 1, file);
    fwrite(&bmph.biSizeImage, 4, 1, file);
    fwrite(&bmph.biXPelsPerMeter, 4, 1, file);
    fwrite(&bmph.biYPelsPerMeter, 4, 1, file);
    fwrite(&bmph.biClrUsed, 4, 1, file);
    fwrite(&bmph.biClrImportant, 4, 1, file);

    line =(unsigned char*) malloc(bytesPerLine);
    if (line == NULL)
    {
        fprintf(stderr, "Can't allocate memory for BMP file.\n");
        return(0);
    }

    for (i = height - 1; i >= 0; i--)
    {
        for (j = 0; j < width; j++)
        {
            ipos = 4 * (width * i + j);
            line[3*j+0] = rgb[ipos + 2];
            line[3*j+1] = rgb[ipos + 1];
            line[3*j+2] = rgb[ipos + 0];
        }
        fwrite(line, bytesPerLine, 1, file);
    }

    free(line);
    fclose(file);

    return(1);
}
