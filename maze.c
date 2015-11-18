#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

int WIDTH = 1920;
int HEIGHT = 1080;

struct pixl {
    float r;
    float g;
    float b;
    char d;
};

struct pixl *pixles;
#define p(x, y) (pixles + (x) + ((y) * WIDTH))

int write_bmp(const char *filename, int width, int height, struct pixl *rgb);

int randdir(int x, int y) {
    int i = rand();
    int k[5] = {0};
    int ct = 0;

    if (x==0 && y==0) { // top left
        k[2] = 2;
        k[3] = 3;
        ct = 2;
    } else if (x==WIDTH-1 && y==HEIGHT-1) { // bottom right
        k[1] = 1;
        k[4] = 4;
        ct = 2;
    } else if (x==0 && y==HEIGHT-1) { // bottom left
        k[1] = 1;
        k[2] = 2;
        ct = 2;
    } else if (x==WIDTH-1 && y==0) { // top right
        k[3] = 3;
        k[4] = 4;
        ct = 2;
    } else if (x==0) { // left wall
        k[1] = 1;
        k[2] = 2;
        k[3] = 3;
        ct = 3;
    } else if (x==WIDTH-1) { // right wall
        k[1] = 1;
        k[3] = 3;
        k[4] = 4;
        ct = 3;
    } else if (y==0) { // top
        k[2] = 2;
        k[3] = 3;
        k[4] = 4;
        ct = 3;
    } else if (y==HEIGHT-1) { // bottom
        k[1] = 1;
        k[2] = 2;
        k[4] = 4;
        ct = 3;
    } else { // any
        k[1] = 1;
        k[2] = 2;
        k[3] = 3;
        k[4] = 4;
        ct = 4;
    }

    if (k[1] == 1) {
        k[1] = !(p(x,y-1)->d);
        ct-=(k[1]?0:1);
    }

    if (k[2] == 2) {
        k[2] = !(p(x+1,y)->d);
        ct-=(k[2]?0:1);
    }

    if (k[3] == 3) {
        k[3] = !(p(x,y+1)->d);
        ct-=(k[3]?0:1);
    }

    if (k[4] == 4) {
        k[4] = !(p(x-1,y)->d);
        ct-=(k[4]?0:1);
    }

    if (ct == 0) {
        return 0;
    }

    i %= ct;
    for(int q=1;q<5;q++) {
        if (k[q]) {
            if (i==0) {
                return q;
            }
            i--;
        }
    }
    return 0;
}

void color(struct pixl *dest, struct pixl *src) {
    dest->r = (src->r+3.0/(WIDTH*HEIGHT));
    if (dest->r > 1) {
        dest->r = 0;
    }
    dest->g = (src->g+3.0/(WIDTH*HEIGHT));
    if (dest->g > 1) {
        dest->g = 0;
    }
    dest->b = (src->b+3.0/(WIDTH*HEIGHT));
    if (dest->b > 1) {
        dest->b = 0;
    }
}


int main(int argc, char **argv) {
    int arg = 1;
    srand(time(NULL));
    char *filename = "generated.bmp";
    while(arg < argc) {
        if (!strcmp(argv[arg], "-w") || !strcmp(argv[arg], "--width")) {
            WIDTH=atoi(argv[arg+1]);
            arg+=2;
        } else if (!strcmp(argv[arg], "-h") || !strcmp(argv[arg], "--height")) {
            HEIGHT=atoi(argv[arg+1]);
            arg+=2;
        } else if (!strcmp(argv[arg], "-s") || !strcmp(argv[arg], "--seed")) {
            srand(atoi(argv[arg+1]));
            arg+=2;
        } else if (!strcmp(argv[arg], "-o") || !strcmp(argv[arg], "--output")) {
            filename = argv[arg+1];
            arg+=2;
        }
    }
    pixles = calloc(sizeof(struct pixl), WIDTH*HEIGHT);

    int x = 0;
    int y = 0;

    p(x,y)->r = (float)rand()/(float)(RAND_MAX)/1.2;
    p(x,y)->g = (float)rand()/(float)(RAND_MAX)/1.2;
    p(x,y)->b = (float)rand()/(float)(RAND_MAX)/1.2;
    p(x,y)->d = 5;

    int write = 0;
    do {
        int z = randdir(x,y);
        if (z) {write++;}
        switch(z) {
            case 0:
                switch(p(x,y)->d) {
                    case 5:
                        goto done;
                    case 1:
                        y--; break;
                    case 2:
                        x++; break;
                    case 3:
                        y++; break;
                    case 4:
                        x--; break;
                }
                break;
            case 1: // go up
                color(p(x,y-1), p(x,y));
                p(x,y-1)->d = 3;
                y--;
                break;
            case 3: // go down
                color(p(x,y+1), p(x,y));
                p(x,y+1)->d = 1;
                y++;
                break;
            case 2: // go right
                color(p(x+1,y), p(x,y));
                p(x+1,y)->d = 4;
                x++;
                break;
            case 4: // go left
                color(p(x-1,y), p(x,y));
                p(x-1,y)->d = 2;
                x--;
                break;
        }
    }
    while(p(x,y)->d != 5);
done:
    write_bmp(filename, WIDTH, HEIGHT, pixles);
}

struct BMPHeader {
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

int write_bmp(const char *filename, int width, int height, struct pixl *rgb) {
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

    line = (unsigned char*)malloc(bytesPerLine);
    if (line == NULL) {
        fprintf(stderr, "Can't allocate memory for BMP file.\n");
        return(0);
    }

    for (i = height - 1; i >= 0; i--) {
        for (j = 0; j < width; j++) {
            ipos = (width * i + j);
            line[3*j] = rgb[ipos].b * 255;
            line[3*j+1] = rgb[ipos].g * 255;
            line[3*j+2] = rgb[ipos].r * 255;
        }
        fwrite(line, bytesPerLine, 1, file);
    }

    //free(line);
    fclose(file);

    return(1);
}
