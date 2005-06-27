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

#ifndef _GNUPLOT_HPP_
#define _GNUPLOT_HPP_

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

namespace gnuplot {
	
//	class gnuplot_error : public std::runtime_error {
//	public:
//		explicit gnuplot_error(const std::string &msg) : std::runtime_error(msg){}
//	};
	
	typedef std::runtime_error gnuplot_error;
	
	template <typename Container>
		void stringtok (Container &container, std::string const &in,
							 const char * const delimiters = " \t\n") {
			const std::string::size_type len = in.length();
			std::string::size_type i = 0;
			
			while ( i < len )
			{
				// eat leading whitespace
				i = in.find_first_not_of (delimiters, i);
				if (i == std::string::npos)
					return;   // nothing left but white space
				
				// find the end of the token
				std::string::size_type j = in.find_first_of (delimiters, i);
				
				// push token
				if (j == std::string::npos) {
					container.push_back (in.substr(i));
					return;
				} else
					container.push_back (in.substr(i, j-i));
				
				// set up for next loop
				i = j + 1;
			}
		}
	
	class gnuplot {
	private:
		FILE								*gnucmd;
		std::string						pstyle;
		std::vector<std::string>   to_delete;
		int								nplots;
		std::ofstream					plotfile;
		void								init(void) 
		{
			if (!get_program_path("gnuplot"))
			{
				throw gnuplot_error("Can't find gnuplot in your PATH");
			}
			
			gnucmd = popen("gnuplot","w");
			
			if (!gnucmd)
			{
				throw gnuplot_error("Couldn't open connection to gnuplot");
			}
			nplots=0;
		}
			
			
	public:		
		// set a style during construction
		gnuplot(const std::string &style = "lines") { init(); set_style(style); }
		
		
		// The equivilant of gnuplot_plot_once, the two forms
		// allow you to plot either (x,y) pairs or just a single
		// vector at one go
		// takes any STL compliant container with an iterator...
		
		template <typename ContainerX, typename ContainerY>
			gnuplot(const std::string &title, // title
					  const std::string &style, // style
					  const std::string &labelx, // xlabel
					  const std::string &labely, // ylabel
					  const ContainerX &x, //x
					  const ContainerY &y  //y 
					  ) 
			{	
				init();
				
				if (x.empty())
					throw gnuplot_error("Data containers empty.");
				if ((!y.empty()) && (x.size() != y.size()))
					throw gnuplot_error("Data containers must match in length.");
					 
				if (style == "")
					set_style("lines");
				else
					set_style(style);
				
				if (labelx == "")
					set_xtitle("X");
				else
					set_xtitle(labelx);
				
				if (labely == "")
					set_ytitle("Y");
				else
					set_ytitle(labely);
				
				plot_xy(title,x,y);
			}
		
		//
		// Destructor
		// 
		~gnuplot()
		{
			// pclose actually waits for gnuplot to finish... 
			if (pclose(this->gnucmd) == -1)  // destructor should not throw!
				std::cerr << "Problem closing communication to gnuplot" << std::endl;  // but we'll complain
			
			reset_plot();
		}
		
		
		bool get_program_path(const std::string &pname)
		{
			std::list<std::string> ls;
			char *path;
			
			path = getenv("PATH");
			if (!path)
			{
				std::cerr << "Path is not set" << std::endl;
				return false;
			}
			else
			{
				stringtok(ls,path,":");
				for (std::list<std::string>::const_iterator i = ls.begin();
					  i != ls.end(); ++i)
				{
					std::string tmp = (*i) + "/" + pname;
					if (access(tmp.c_str(),X_OK) == 0)
						return true;
				}
			}
			return false;
		}
		
		void reset_plot(void)
		{       
			if (to_delete.size() > 0)
				for (size_t i = 0; i < this->to_delete.size(); i++)
					remove(this->to_delete[i].c_str());
			
			to_delete.clear();
			nplots = 0;
		}
		
		void set_style(const std::string& stylestr)
		{
			if (stylestr != "lines" &&
				 stylestr != "points" &&
				 stylestr != "linespoints" &&
				 stylestr != "impulses" &&
				 stylestr != "dots" &&
				 stylestr != "steps" &&
				 stylestr != "errorbars" &&
				 stylestr != "boxes" &&
				 stylestr != "boxerrorbars")
				pstyle = std::string("points");
			else
					pstyle = stylestr;
		}
		
		void cmd(const char *cmdstr, ...)
		{
			va_list ap;
			char local_cmd[GP_CMD_SIZE];
			
			va_start(ap, cmdstr);
			vsprintf(local_cmd, cmdstr, ap);
			va_end(ap);
			strcat(local_cmd,"\n");
			fputs(local_cmd,this->gnucmd);
			fflush(this->gnucmd);
			return;
		}
		
		void cmd(const std::string &s)
		{
			std::ostringstream cmdstr;
			cmdstr << s << std::endl;
			fputs(cmdstr.str().c_str(),gnucmd);
			fflush(gnucmd);
		}
		
		void set_xtitle(const std::string &label)
		{
			std::ostringstream cmdstr;
			
			cmdstr << "set xlabel \"" << label << "\"";
			cmd(cmdstr.str());
		}
		
		void set_ytitle(const std::string &label)
		{
			std::ostringstream cmdstr;
			
			cmdstr << "set ylabel \"" << label << "\"";
			cmd(cmdstr.str());
		}
		
		// 
		// Plots a linear equation (where you supply the
		// slope and intercept)
		//
		void plot_slope(double a, double b, const std::string &title)
		{
			std::ostringstream cmdstr;
			
			if (nplots > 0)
				cmdstr << "re";
			
			cmdstr << "plot " << a << " * x + " << b;
			
			if (!title.empty())
				cmdstr << " title \"" << title << "\"";
			
			cmdstr << " with " << pstyle;
			
			cmd(cmdstr.str());
			nplots++;
		}
		
		//
		// Plot an equation which is supplied as a string
		// 
		void plot_equation(const std::string &equation, const std::string &title)
		{
			std::ostringstream cmdstr;
			
			if (nplots > 0)
				cmdstr << "re";
			
			cmdstr << "plot " << equation;

			if (!title.empty())
				cmdstr << " title \"" << title << "\"";
			
			cmdstr << " with " << pstyle;
			
			cmd(cmdstr.str());
			nplots++;
		}
		
			
		template <class ContainerX, class ContainerY>
			void plot_xy(const std::string &title,
						 const ContainerX &x, 
						 const ContainerY &y = new ContainerX 
						 )
		{
				std::ofstream tmp;
				std::ostringstream cmdstr;
				std::string name = "/tmp/gnuplotiXXXXXX";
				typename ContainerX::const_iterator xp;
				typename ContainerY::const_iterator yp;
				
				// should raise an exception
				if ((!y.empty()) && (x.size() != y.size()))
					throw gnuplot_error("X and Y arrays should be equal sized."); 
				
				if (to_delete.size() == GP_MAX_TMP_FILES - 1)
					throw gnuplot_error("Maximum temporary files reached.");
				
				//
				//open temporary files for output
				//
				if (mkstemp(const_cast<char *>(name.c_str())) == -1)
					throw gnuplot_error("Cannot create temporary file");
				
				tmp.open(name.c_str());
				if (tmp.bad())
					throw gnuplot_error("Cannot create temporary file");
				
				//
				// Save the temporary filename
				// 
				to_delete.push_back(name);
				
				//
				// write the data to file
				//
				if (y.empty())
					for (xp=x.begin(); xp != x.end(); ++xp) 
						tmp << *xp << std::endl;
				else
					for (xp=x.begin(), yp=y.begin(); xp != x.end();) 
						tmp << *xp++ << " " << *yp++ << std::endl;
				
				tmp.flush();    
				tmp.close();
				
				//
				// command to be sent to gnuplot
				//
								
				if (nplots > 0)
					cmdstr << "re";
				
				cmdstr << "plot \"" << name << "\"";
				
				if (!title.empty())
					cmdstr << " title \"" << title << "\"";
				
				cmdstr << " with " << pstyle;
				
				cmd(cmdstr.str());
				nplots++;
			
			}
	};
	
}

#endif
