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
    int Init(int spec, float int_time, int average, 
                int dynamic_dark, unsigned max_spectra);
    int    Stop(int spec);
    int    NumChannels(int spec);
    int    NumSpectra(int spec);
    int    NumWavelengths(int spec);
    void   ReadSpectra(int spec, int chan, short int *data);
    void   ReadDark(int spec, int chan, short int *data);
    void   ReadWavelengths(int spec, int chan, float *wave);
    void   Destroy(int spec);
    void RunRaw(float integration_time, int average, int dynamic_dark, int num_spectra);
#if __cplusplus
};
#endif
