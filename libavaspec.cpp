/*
 *  libavaspec.cpp
 *  avaspec
 *
 *  Created by Darren Garnier on 6/28/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

//#include "libavaspec.h"

#include "avaspec.hpp"
#include <pthread.h>
#include <math.h>
#include <vector>
#include <map>
#include <iostream>
//#include <boost/array.hpp>

// export these functions
#include "libavaspec.h"

class multispec : public avaspec {
public:
    pthread_t m_multispec_thread;
    bool      m_multispec_cancel;
    bool      m_dynamic_dark;
    bool      m_cancelled;
    unsigned int m_max_spectra;
    pthread_t m_dacq_thread;
    bool      m_dacq_thread_running;
        
    std::vector< short > m_dark;
    
    std::vector< std::vector< short > >   m_spectra;

    static const unsigned kProduct = 0x0471;
    static const unsigned kVendor  = 0x0666;

    multispec(int skip, float integration_time, int average, int dynamic_dark, size_t max_spectra);
    multispec(int skip, float integration_time, int average, int dynamic_dark, size_t spectra, int raw);
    
    ~multispec(void);
    
    std::vector<float>   get_wavelengths(unsigned chan);
    std::vector<short>   get_spectrum(unsigned chan);
    bool            run_dacq(void);
    std::vector< std::vector< short > > run_dacq_no_trig(void);

    bool            stop_dacq(void);

};



std::vector<float> multispec::get_wavelengths(unsigned chan)
{
    std::vector<float> cal;	
    
    for (int i=0; i<5 ;i++) cal.push_back(get_calibration(chan,i));
    
    std::vector<float> x(num_pixels());
    
    for (unsigned i = 0; i != num_pixels(); ++i) {
        unsigned i2 = i*i;
        x[i]=cal[0]+cal[1]*i+cal[2]*(i2)+(cal[3]*i)*(i2)+(cal[4]*i2)*i2;	
    }
    
    return x;
}

std::vector<short> multispec::get_spectrum(unsigned chan)
{
    short dd = 0;
    
    if ((m_dynamic_dark) && (extra_pixels()!=0)) {
        for (unsigned i = 0; i != extra_pixels(); ++i) dd += (*this)[chan].extra(i);
        dd /= extra_pixels();
    }
    
    std::vector<short> y(num_pixels());
    
    for (unsigned i = 0; i != num_pixels(); ++i) {
        short val=(*this)[chan][i];
        y[i]=(val < (1 << 14)) ? val-dd : val;
    }
    
    return y;
}

static void * StartDacqThread(void *vp)
{
    multispec *sp = reinterpret_cast<multispec *>(vp);
    
    sp->run_dacq();
    
    return NULL;
}

multispec::multispec(int skip, float integration_time, int average, int dynamic_dark, size_t max_spectra) :
     avaspec("",kProduct,kVendor,skip) 
{
    
    // change the settings
    unsigned int sec = (unsigned int) floor(integration_time);
    unsigned int nsec = (unsigned int) floor(1e9*(integration_time-sec));
    shevek::relative_time int_time(sec,nsec);  // 1 second, 0 nanosec
    
    set_integration_time(int_time);
    set_average(average); // don't average spectra...
    set_strobe(false); // turn off 1 kHz strobe output
    set_digital(1,true); // set pin 1 output on
    
    m_cancel_read = false;
    m_cancelled = false;
    m_max_spectra = max_spectra;
    
    avaspec::channel *cp = &(*this)[0];
    cp->set_range (cp->get_range_min (), cp->get_range_max ());
    
    if (0 == pthread_create(&m_dacq_thread,NULL,StartDacqThread,reinterpret_cast<void *>(this))) {
        m_dacq_thread_running = true;
    }
}

multispec::multispec(int skip, float integration_time, int average, int dynamic_dark, 
                     size_t max_spectra, int raw):
avaspec("",kProduct,kVendor,skip) 
{
    
    // change the settings
    unsigned int sec = (unsigned int) floor(integration_time);
    unsigned int nsec = (unsigned int) floor(1e9*(integration_time-sec));
    shevek::relative_time int_time(sec,nsec);  // 1 second, 0 nanosec
    
    set_integration_time(int_time);
    set_average(average); // don't average spectra...
    set_strobe(false); // turn off 1 kHz strobe output
    set_digital(1,true); // set pin 1 output on
    
    m_cancel_read = false;
    m_cancelled = false;
    m_max_spectra = max_spectra;
    
    avaspec::channel *cp = &(*this)[0];
    cp->set_range (cp->get_range_min (), cp->get_range_max ());
    
//    pthread_create(&m_dacq_thread,NULL,StartDacqThread,reinterpret_cast<void *>(this));
}



multispec::~multispec(void)
{
    if (m_dacq_thread_running) {
        void *result;
        pthread_cancel(m_dacq_thread);
        pthread_join(m_dacq_thread, &result);
        m_dacq_thread_running = false;
    }
}

bool multispec::stop_dacq(void)
{
    void * result;
    if (!m_dacq_thread_running) return false;
    m_cancel_read = true;
    
    pthread_join(m_dacq_thread, &result);
    
    m_dacq_thread_running = false;
    
    return !m_cancelled;
}

std::vector< std::vector< short > > multispec::run_dacq_no_trig(void)
{
     std::vector< std::vector< short > > spectra;
     
     for (int i=0; i<m_max_spectra; i++) {
         start_read();
         end_read();
         spectra.push_back(get_spectrum(0));
     }
     
     return spectra;
}
    

bool multispec::run_dacq(void)
{
// do a background spectrum

    external_trigger(false);
    start_read();
    end_read();
    
    m_dark = get_spectrum(0);
    
//  now do the data spectra
    
    external_trigger(true);
    
    for (unsigned i=0;i<m_max_spectra;i++) {
//        std::cout<<"taking spectrum."<<std::endl;
        start_read();
        if (run_read_async()) {
            m_spectra.push_back(get_spectrum(0));
        } else {
            m_cancelled = true;
            return false;
        }
    }
    return true;
}

static std::map< int , multispec * > gSpects;

int Init(int spect, float integration_time, char *trig_event, int *triggers, int average, int dynamic_dark, unsigned max_spectra)
{
    multispec *sp;
    sp = new multispec(spect, integration_time, average, dynamic_dark, max_spectra);
    if (sp == NULL) return -1;
    gSpects[spect] = sp;
    return spect;
}

void Destroy(int spect)
{
    multispec *sp = gSpects[spect];
    delete sp;
    gSpects.erase(spect);
}

int  Stop(int spect)
{
    multispec *sp  = gSpects[spect];
    return  sp->stop_dacq();
}

int    NumChannels(int spect)
{
    multispec *sp =  gSpects[spect];
    return  sp->num_channels();
}

int    NumSpectra(int spect)
{
    multispec *sp  = gSpects[spect];
    return  sp->m_spectra.size();
}

int    NumWavelengths(int spect)
{
    multispec *sp =  gSpects[spect];
    return  sp->num_pixels();
}

void   ReadSpectra(int spect, int chan, short int *data)
{
    multispec *sp =  gSpects[spect];
    
    for (size_t i=0; i != sp->m_spectra.size(); ++i)
        for (size_t j=0; j != sp->num_pixels(); j++)
            *data++ = sp->m_spectra[i][j];
}

void   ReadDark(int spect, int chan, short int *data)
{
    multispec *sp =  gSpects[spect];
    
    for (size_t i=0; i != sp->m_dark.size(); ++i)
        *data++ = sp->m_dark[i];
}


void   ReadWavelengths(int spect, int chan, float *wavel)
{
    multispec *sp =  gSpects[spect];

    std::vector<float> waves;
    
    waves = sp->get_wavelengths(chan);

    for (size_t i=0; i != waves.size(); ++i)
        *wavel++ = waves[i];
}

void RunRaw(float integration_time, int average, int dynamic_dark, int num_spectra)
{
    
    multispec *sp;
    sp = new multispec(0, integration_time, average, dynamic_dark, num_spectra, 1);
    if (sp == NULL) return ;

    std::vector<float> waves = sp->get_wavelengths(0);
    
    std::vector< std::vector< short > > spectra = sp->run_dacq_no_trig();
    
    for (unsigned i=0; i<waves.size() ; i++) std::cout << waves[i] << ", ";
    std::cout << std::endl;
    
    for(unsigned j=0; j<spectra.size() ; j++) {
        for (unsigned i=0; i<waves.size() ; i++) std::cout << spectra[j][i] << ", ";
        std::cout << std::endl;
    }
    
    delete sp;
    
}
