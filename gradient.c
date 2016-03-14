#include <stdio.h>
#include <stdlib.h>

#include "gradient.h"

static inline char HEX(char x) {
    char res = 0;
    if (x>='0' && x<='9') {
        res = x-'0';
    }
    if (x>='A' && x<='F') {
        res = x-'A' + 10;
    }
    if (x>='a' && x<='f') {
        res = x-'a' + 10;
    }
    return res;
}

gcolor from_hex(char *hex) {
    gcolor c = {0, 0, 0};
    if (!hex || !(*hex == '#')) {
        return c;
    }
    c.r = (16*HEX(hex[1]) + HEX(hex[2]));
    c.g = (16*HEX(hex[3]) + HEX(hex[4]));
    c.b = (16*HEX(hex[5]) + HEX(hex[6]));
    if (c.r<0 || c.g<0 || c.b<0) {
        c = (gcolor){0,0,0};
    }
    return c;
}

void printcolor(gcolor c) {
    printf("#%x%x%x\n", c.r, c.g, c.b);
}

gradient *triple_gradient(size_t range, char *a, char *bb, char *c) {
    gradient *gradient = malloc(sizeof(gradient));
    gradient->size = range*3;
    gradient->colors = calloc(sizeof(gcolor), range*3);
    gcolor A = from_hex(a);
    gcolor B = from_hex(bb);
    gcolor C = from_hex(c);
    float R = (float)range;

    float r,g,b;
    r = (float)(B.r - A.r);
    g = (float)(B.g - A.g);
    b = (float)(B.b - A.b);
    r/=R;
    g/=R;
    b/=R;
    for(size_t i=0;i<range;i++){
        (gradient->colors)[i].r = (unsigned char)((A.r) + r*i);
        (gradient->colors)[i].g = (unsigned char)((A.g) + g*i);
        (gradient->colors)[i].b = (unsigned char)((A.b) + b*i);
    }

    r = (float)(C.r - B.r);
    g = (float)(C.g - B.g);
    b = (float)(C.b - B.b);
    r/=R;
    g/=R;
    b/=R;
    for(size_t i=0;i<range;i++){
        (gradient->colors)[i+range].r = (unsigned char)((B.r) + r*i);
        (gradient->colors)[i+range].g = (unsigned char)((B.g) + g*i);
        (gradient->colors)[i+range].b = (unsigned char)((B.b) + b*i);
    }

    r = (float)(A.r - C.r);
    g = (float)(A.g - C.g);
    b = (float)(A.b - C.b);
    r/=R;
    g/=R;
    b/=R;
    for(size_t i=0;i<range;i++){
        (gradient->colors)[i+range+range].r = (unsigned char)((C.r) + r*i);
        (gradient->colors)[i+range+range].g = (unsigned char)((C.g) + g*i);
        (gradient->colors)[i+range+range].b = (unsigned char)((C.b) + b*i);
    }
    return gradient;
}


gradient *hex_gradient(size_t range, char *from, char *to) {
    gradient *gradient = malloc(sizeof(gradient));
    gradient->size = range*2;
    gradient->colors = calloc(sizeof(gcolor), range*2);
    gcolor f = from_hex(from);
    gcolor t = from_hex(to);
    float r = (float)(t.r - f.r);
    float g = (float)(t.g - f.g);
    float b = (float)(t.b - f.b);
    float R = (float)range;
    r/=R;
    g/=R;
    b/=R;
    for(size_t i=0;i<range;i++){
        (gradient->colors)[i].r = (unsigned char)((f.r) + r*i);
        (gradient->colors)[i].g = (unsigned char)((f.g) + g*i);
        (gradient->colors)[i].b = (unsigned char)((f.b) + b*i);
    }
    for(size_t i=0;i<range;i++){
        (gradient->colors)[i+range].r = (unsigned char)((t.r) - r*i);
        (gradient->colors)[i+range].g = (unsigned char)((t.g) - g*i);
        (gradient->colors)[i+range].b = (unsigned char)((t.b) - b*i);
    }
    return gradient;
}

