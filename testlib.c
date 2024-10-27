/*
 *  testlib.c
 *  avaspec
 *
 *  Created by Darren Garnier on 6/30/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#include "libavaspec.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>  /* sleep */

int main (int argc, char * const argv[]) {

    int h;
    size_t nw, ns;
    float *waves;
    short *spectra;
    int trigs[5] = {0,0,0,0,0};

    printf("preinit\n");
    h = Init(0,.25,"trig", trigs, 1, 1, 5);
    printf("postinit...\n");
    
    sleep(10);
    
    printf("post sleep\n");
    
    if (Stop(h)) 
        printf("Complete.\n");
    else
        printf("cancelled.\n");
    
    
    nw = NumWavelengths(h);
    ns = NumSpectra(h);
    
    printf("nw=%ld,ns=%ld\n",nw,ns);
    
    waves = (float *) malloc(sizeof(float)*nw);
    spectra = (short *) malloc(sizeof(short)*nw*ns);
    
    ReadSpectra(h,0,spectra);
    ReadWavelengths(h,0,waves);
    
    Destroy(h);
    
    return 0;
    
}
