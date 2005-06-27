#include <iostream>
#include <valarray>
#include <vector>
#include <stdexcept>
#include <functional>
#include <algorithm>
#include "avaspec.hpp"
#include "sstr.hpp"
#include <usb.h>
#include "time.hpp"
#include "gnuplot.hpp"

const unsigned kProduct = 0x0471;
const unsigned kVendor = 0x0666;
using namespace shevek;
using namespace std;

vector<float> frequencies(const avaspec &s, int chan)
{
	vector<float> cal;	
	for (int i=0; i<7 ;i++) cal.push_back(s.get_calibration(chan,i));

	vector<float> x(s.num_pixels());
	
	for (unsigned i = 0; i != s.num_pixels(); ++i) {
		unsigned i2 = i*i;
		x[i]=cal[0]+cal[1]*i+cal[2]*(i2)+(cal[3]*i)*(i2)+(cal[4]*i2)*i2;	
	}
	
	return x;
}

vector<short> get_dynamic_spectrum(const avaspec &s, int chan)
{
	short dd = 0;
	
	for (unsigned i = 0; i != s.extra_pixels(); ++i) dd += s[chan].extra(i);

	dd /= 14;
	
	vector<short> y(s.num_pixels());

	for (unsigned i = 0; i != s.num_pixels(); ++i) {
		y[i]=s[chan][i]-dd;
	}

	return y;
}


int main (int argc, char * const argv[]) {
	vector<float>::iterator fp;
	
	string theConfig = "";
	
	//usb_set_debug(1);

	try {
		
		vector<float> cal;	
		vector<float> nl;
		vector<float> ijk;
		vector<float> x,xx;
		vector<short int> y,y1,y2,diff;
		
			
		{		// connect to the spectrometer...
			
			avaspec s(theConfig, kProduct, kVendor, 0);
			
			cout << "Avaspec driver test, v1.0" << endl;
			cout << "May 27,2005" << endl;
			
			cout << "Spectrometer found." <<endl;
			cout << " - # channels: "<<s.num_channels()<<endl;
			cout << " - # pixels:   "<<s.num_pixels()<<endl;
			cout << " - # extra_pix:"<<s.extra_pixels()<<endl;
			
			for (size_t j=0; j<s.num_channels(); j++ ){
				
				for (int i=0; i<7 ;i++) cal.push_back(s.get_calibration(j,i));
				cout << " - Channel "  << j <<endl;
				cout << "   - Start: " << s.get_start(j)<<endl;
				cout << "   - Stop:  " << s.get_stop(j)<<endl;
				cout << "   - Range: " << s[j].get_range_min() << "-"
					<< s[j].get_range_max()<<endl;
				cout << "   - Calibration: ";
				for (fp = cal.begin(); fp != cal.end(); fp++) cout << *fp << ", ";
				cout << endl;
			}
			
			// change the settings
			
			relative_time int_time(0,50000000);  // 1 second, 0 nanosec
			
			s.set_integration_time(int_time);
			s.set_average(1); // don't average spectra...
			
			//	s.external_trigger(true); // turn off external triggering
			
			s.set_strobe(false); // turn off 1 kHz strobe output
			
			s.set_digital(1,true); // set pin 1 output on
			
			// do a spectrum
			
			cout << "Take a spectrum!" << endl;
			
			s.start_read();
			s.end_read();
			
			cout << "Done." << endl;
			
//			cout << "Raw data .." << endl;
//			for (int j=0; j<50 ; j++) cout << s[0][j] << " ";
//			cout << endl;
			
			valarray<unsigned short int> extra(s.extra_pixels());
			
			for (unsigned i = 0; i != s.extra_pixels(); ++i) extra[i]=s[0].extra(i);
			
			short dd = extra.sum() / 14;
			
			cout << "Dark pixels: " << extra.sum() << endl;
			for (unsigned i = 0; i != extra.size(); ++i) cout << extra[i] << ", ";
			cout << endl;
			
			x.resize(s.num_pixels());
			y.resize(s.num_pixels());
			
			for (unsigned i = 0; i != s.num_pixels(); ++i) {
				unsigned i2 = i*i;
				x[i]=cal[0]+cal[1]*i+cal[2]*(i2)+(cal[3]*i)*(i2)+(cal[4]*i2)*i2;	
				y[i]=s[0][i]-dd;
			}
			
			xx = frequencies(s,0);
			y1 = get_dynamic_spectrum(s,0);
			s.start_read();
			s.end_read();
			y2 = get_dynamic_spectrum(s,0);
			
			diff = y2;
			std::minus<short> minus;
			std::transform(y2.begin(),y2.end(),y1.begin(),diff.begin(), minus );
			
		} // disconnect the spectrometer
	
		string title = "Spectrum";
		string xtitle = "Wavelength";
		string ytitle = "Counts";
		string style;
		
//		gnuplot::gnuplot g(title,style,xtitle,ytitle,x,y);
		
		gnuplot::gnuplot g;
		g.set_xtitle(xtitle);
		g.set_ytitle(ytitle);
		g.plot_xy(title,xx,y1);
		g.plot_xy("Corrected",xx,diff);
//		std::string s;
//		g.plot_xy("Corrected",diff,s);
		
		return 0;
	}
	
	catch (string err) {
		cout << "Exception: " << err << endl;
		return 1;
	}
	
	catch (runtime_error err) {
		cout << "Runtime error: " << err.what() << endl;
		return -1;
	}
}
