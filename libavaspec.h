/*
 *  libavaspec.h
 *  avaspec
 *
 *  Created by Darren Garnier on 6/30/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

#if __cplusplus
extern "C" {
#endif
    void * Init(int skip, float int_time, int average, 
                int dynamic_dark, unsigned max_spectra);
    int    Stop(void *handle);
    int    NumChannels(void *handle);
    int    NumSpectra(void *handle);
    int    NumWavelengths(void *handle);
    void   ReadSpectra(void *handle, int chan, short int *data);
    void   ReadWavelengths(void *handle, int chan, float *wave);
    void   Destroy(void *handle);
#if __cplusplus
};
#endif
