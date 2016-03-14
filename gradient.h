
#ifndef _GRADIENT_H_
#define _GRADIENT_H_

typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} gcolor;

typedef struct {
    gcolor *colors;
    size_t size;
} gradient;

gradient *hex_gradient(size_t range, char *from, char *to);
gradient *triple_gradient(size_t range, char *a, char *bb, char *c);
gcolor from_hex(char *hex);
void printcolor(gcolor c);
#endif
