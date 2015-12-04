/*
 * Copyright (C) 2015 Ted Meyer
 *
 * see LICENSING for details
 *
 */
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <linux/fb.h>
#include <stdint.h>
#include <sys/mman.h>
#include <unistd.h>
#include <math.h>

#define p(x, y) (pixels + (x) + ((y) * WIDTH))

#define colormode 2

struct pixl {
    float r;
    float g;
    float b;
    char d;
};

struct rgb {
	unsigned char r;
	unsigned char g;
	unsigned char b;
};

int WIDTH = 1920;
int HEIGHT = 1080;
struct pixl *pixels = 0;

int write_bmp(const char *filename, int width, int height, struct pixl *rgb);
void rgb_create(double r, double g, double b, struct rgb *to);
void rgb_from_hsl(double h, double s, double l, struct rgb *to);
void rgb_from_hsl2(double h, double s, double l, struct rgb *to);
void rgb_color(struct pixl *dest, struct pixl *src);
void hsl_color(struct pixl *dest, struct pixl *src);

#ifdef FRAMEBUFFER
int fb_fd = 0;
int sleep_t = 0;
struct fb_fix_screeninfo finfo;
struct fb_var_screeninfo vinfo;
uint8_t *fbp = 0;

inline uint32_t pixel_color(uint8_t r, uint8_t g, uint8_t b, struct fb_var_screeninfo *vinfo) {
    return (r<<vinfo->red.offset) | (g<<vinfo->green.offset) | (b<<vinfo->blue.offset);
}
#endif

void mapcolor(struct pixl *from, struct rgb *to) {
    switch(colormode) {
        case 1: //rgb
            rgb_create(from->r
                      ,from->g
                      ,from->b
                      ,to);
            break;
        case 2: //hsl
        rgb_from_hsl2(from->r
                    ,from->g
                    ,from->b
                    ,to);
        break;
    }
}

void color(struct pixl *dest, struct pixl *src) {
    switch(colormode) {
        case 1: //rgb
            rgb_color(dest, src);
            break;
        case 2:
            hsl_color(dest, src);
            break;
    }
}


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

void hsl_color(struct pixl *dest, struct pixl *src) {
    dest->r = 0;
    dest->g = 10;
    dest->b = src->b+.002;
    if (dest->b > 100) {
        dest->b = 0;
    }
}

void rgb_color(struct pixl *dest, struct pixl *src) {
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

#ifdef FRAMEBUFFER
void framebuffer(int x, int y) {
    struct rgb draw;
    mapcolor(p(x,y), &draw);
    if (sleep_t) {
        usleep(sleep_t);
    }
    long location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8)
                  + (y+vinfo.yoffset) * finfo.line_length;
    
    *((uint32_t*)(fbp + location))=pixel_color(draw.r, draw.g, draw.b, &vinfo);
}
#endif

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
#ifdef FRAMEBUFFER
        } else if (!strcmp(argv[arg], "-t") || !strcmp(argv[arg], "--timeout")) {
            sleep_t=atoi(argv[arg+1]);
            arg+=2;
#endif
        }
    }
#ifdef FRAMEBUFFER
    fb_fd = open("/dev/fb0",O_RDWR);
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);
    vinfo.grayscale=0;
    vinfo.bits_per_pixel=32;

    ioctl(fb_fd, FBIOPUT_VSCREENINFO, &vinfo);
    ioctl(fb_fd, FBIOGET_VSCREENINFO, &vinfo);

    ioctl(fb_fd, FBIOGET_FSCREENINFO, &finfo);
    long screensize = vinfo.yres_virtual * finfo.line_length;
    WIDTH = vinfo.xres;
    HEIGHT = vinfo.yres;

    fbp = mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED, fb_fd, (off_t)0);
done:
    if(pixels) {
        free(pixels);
    }
#endif
    pixels = calloc(sizeof(struct pixl), WIDTH*HEIGHT);

    int x = rand() % WIDTH;
    int y = rand() % HEIGHT;

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
                y--;
                p(x,y)->d = 3;
#ifdef FRAMEBUFFER
                framebuffer(x, y);
#endif
                break;
            case 3: // go down
                color(p(x,y+1), p(x,y));
                y++;
                p(x,y)->d = 1;
#ifdef FRAMEBUFFER
                framebuffer(x, y);
#endif
                break;
            case 2: // go right
                color(p(x+1,y), p(x,y));
                x++;
                p(x,y)->d = 4;
#ifdef FRAMEBUFFER
                framebuffer(x, y);
#endif
                break;
            case 4: // go left
                color(p(x-1,y), p(x,y));
                x--;
                p(x,y)->d = 2;
#ifdef FRAMEBUFFER
                framebuffer(x, y);
#endif
                break;
        }
    }
    while(p(x,y)->d != 5);
    goto done;
#ifndef FRAMEBUFFER
done:
      write_bmp(filename, WIDTH, HEIGHT, pixels);
#endif
}

#ifndef FRAMEBUFFER
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

    struct rgb cur;

    for (i = height - 1; i >= 0; i--) {
        for (j = 0; j < width; j++) {
            ipos = (width * i + j);
            mapcolor(&(rgb[ipos]), &cur);
            line[3*j+0] = cur.b;
            line[3*j+1] = cur.g;
            line[3*j+2] = cur.r;
        }
        fwrite(line, bytesPerLine, 1, file);
    }

    fclose(file);

    return(1);
}
#endif

#define HUE_UPPER_LIMIT 360.0

void rgb_create(double r, double g, double b, struct rgb *to) {
    to->r = 255*r;
    to->g = 255*g;
    to->b = 255*b;
}

void rgb_from_hsl(double h, double s, double l, struct rgb *to) {
    double c = 0.0, m = 0.0, x = 0.0;
    c = (1.0 - fabsf(2 * l - 1.0)) * s;
    m = 1.0 * (l - 0.5 * c);
    x = c * (1.0 - fabs(fmod(h / 60.0, 2) - 1.0));
    if (h >= 0.0 && h < (HUE_UPPER_LIMIT / 6.0)) {
        rgb_create(c + m, x + m, m, to);
    }
    else if (h >= (HUE_UPPER_LIMIT / 6.0) && h < (HUE_UPPER_LIMIT / 3.0)) {
        rgb_create(x + m, c + m, m, to);
    }
    else if (h < (HUE_UPPER_LIMIT / 3.0) && h < (HUE_UPPER_LIMIT / 2.0)) {
        rgb_create(m, c + m, x + m, to);
    }
    else if (h >= (HUE_UPPER_LIMIT / 2.0)
            && h < (2.0f * HUE_UPPER_LIMIT / 3.0)) {
        rgb_create(m, x + m, c + m, to);
    }
    else if (h >= (2.0 * HUE_UPPER_LIMIT / 3.0)
            && h < (5.0 * HUE_UPPER_LIMIT / 6.0)) {
        rgb_create(x + m, m, c + m, to);
    }
    else if (h >= (5.0 * HUE_UPPER_LIMIT / 6.0) && h < HUE_UPPER_LIMIT) {
        rgb_create(c + m, m, x + m, to);
    }
    else {
        rgb_create(m, m, m, to);
    }
}

float h2rgb(float p, float q, float h) {
    if (h<0) {
        h+=1;
    }
    if (h>1) {
        h -= 1;
    }
    if (6 * h < 1){
        return p + ((q - p) * 6 * h);
    }
    if (2 * h < 1 ){
        return  q;
    }
    if (3 * h < 2){
        return p + ( (q - p) * 6 * ((2.0f / 3.0f) - h) );
    }
    return p;
}

void rgb_from_hsl2(double h, double s, double l, struct rgb *to) {
    h /= 360.0;
    s /= 100.0;
    l /= 100.0;

    float q = 0;

    if (l < 0.5)
        q = l * (1 + s);
    else
        q = (l + s) - (s * l);

    float p = 2 * l - q;

    float r = fmaxf(0, h2rgb(p, q, h + (1.0f / 3.0f)));
    float g = fmaxf(0, h2rgb(p, q, h));
    float b = fmaxf(0, h2rgb(p, q, h - (1.0f / 3.0f)));

    r = fminf(r, 1.0f);
    g = fminf(g, 1.0f);
    b = fminf(b, 1.0f);

    rgb_create(r, g, b, to);
}
