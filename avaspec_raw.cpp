/*
 *  avaspec_raw.cpp
 *  avaspec
 *
 *  Created by Darren Garnier on 7/1/05.
 *
 */

#include "libavaspec.h"
#include <stdlib.h>
#include <iostream>
#include <vector>

using namespace std;

int main(int argc, char *const argv[])
{

    int h;
    size_t nw, ns;
    vector<float> waves;
    short *spectra;
    float int_time = .25;
    long num_spec = 50;

    cout << "Avaspec Raw" << endl;

    if (argc >= 2)
        int_time = atof(argv[1]);

    if (argc >= 3)
        num_spec = atoi(argv[2]);

    cout << "Integration time = " << int_time << endl;
    cout << "Num spectra = " << num_spec << endl;

    RunRaw(int_time, 1, 1, num_spec);
    return 0;
}
