/*
 *  avaspec_raw.cpp
 *  avaspec
 *
 *  Created by Darren Garnier on 7/1/05.
 *  Copyright 2005 __MyCompanyName__. All rights reserved.
 *
 */

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
#include <iostream>
#include <vector>

using namespace std;


int main (int argc, char * const argv[]) {
    // int h;
    // size_t nw, ns;
    // vector< float > waves;
    // short *spectra;
    
    cout<<"Avaspec Raw"<<endl;
    
    RunRaw(.25,1,1,50);
    return 0;
}
