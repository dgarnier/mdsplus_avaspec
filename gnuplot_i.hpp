////////////////////////////////////////////
//
// A C++ interface to gnuplot. 
//
// This is a direct translation from the C interface
// written by N. Devillard (which is available from
// http://ndevilla.free.fr/gnuplot/).
//
// As in the C interface this uses pipes and so won't
// run on a system that doesn't have POSIX pipe 
// support
//
// Rajarshi Guha
// <rajarshi@presidency.com>
//
// 07/03/03
//
// 26/01/04 - Gnuplot::cmd() was rewritten to accept a
// char* rather than a std::std::string, thus avoiding the
// va_start warning at compile time
// /////////////////////////////////////////

#ifndef _GNUPLOT_PIPES_H_
#define _GNUPLOT_PIPES_H_

#include <stdarg.h>
#include <unistd.h>

#include <cstdlib>
#include <cstdio>
#include <cstring>

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <list>
#include <vector>
#include <stdexcept>

#define GP_MAX_TMP_FILES    64
#define GP_TMP_NAME_SIZE    512
#define GP_CMD_SIZE         1024
#define GP_TITLE_SIZE       80

//using namespace std;

#pragma interface

namespace Gnuplot {
	
	class GnuplotException : public std::runtime_error {
	public:
		GnuplotException(const std::string &msg) : std::runtime_error(msg){}
	};
	
	class Gnuplot {
	private:
		FILE            *gnucmd;
		std::string           pstyle;
		std::vector<std::string>   to_delete;
		int              nplots;
		bool             get_program_path(const std::string);
		bool             valid;
	public:
			Gnuplot();
		
		// set a style during construction
		Gnuplot(const std::string &);
		
		// The equivilant of gnuplot_plot_once, the two forms
		// allow you to plot either (x,y) pairs or just a single
		// vector at one go
		// takes any STL compliant container with an iterator...
		
		template <typename ContainerX, typename ContainerY>
			Gnuplot(const std::string &, // title
					  const std::string &, // style
					  const std::string &, // xlabel
					  const std::string &, // ylabel
					  const ContainerX &x, //x
					  const ContainerY &y  //y 
					  );
		
		template <typename Container>
			Gnuplot(const std::string &, //title
					  const std::string &, //style
					  const std::string &, //xlabel
					  const std::string &, //ylabel
					  const Container &d //x
					  );
		
		~Gnuplot();
		
		// send a command to gnuplot
		void cmd(const char*, ...);
		void cmd(const std::string &);
		
		// set line style
		void set_style(const std::string &);
		
		// set y and x axis labels
		void set_ylabel(const std::string &);
		void set_xlabel(const std::string &);
		
		// plot a single vector
		template <typename Container>
			void plot_x(const Container &d, 
							const std::string & // title
							);
		
		// plot x,y pairs
		template <typename ContainerX, typename ContainerY>
			void plot_xy(const ContainerX &x, const ContainerY &y, 
							 const std::string &t // title
							 );
		
		// plot an equation of the form: y = ax + b
		// You supply a and b
		void plot_slope(
							 double, // a
							 double, // b 
							 const std::string & // title
							 );
		
		// plot an equation supplied as a std::string
		void plot_equation(
								 const std::string &, // equation 
								 const std::string &  // title
								 );
		
		// if multiple plots are present it will clear the plot area
		void reset_plot(void);
		
		bool is_valid(void);
		
	};	
}

#endif
