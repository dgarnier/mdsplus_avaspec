/* debug.hh - debugging macros
 * Copyright (C) 2003 Bas Wijnen <b.wijnen@phys.rug.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * The GNU General Public License is enclosed in the distribution as a text
 * file named "COPYING".
 */

#ifndef SHEVEK_DEBUG_HH
#define SHEVEK_DEBUG_HH

#include <iostream>
#include <iomanip>
#include <string>
#include <ctype.h>

#define startfunc \
do { if (DEBUG_STARTFUNC) std::cerr << "Debug: entering " \
  << __PRETTY_FUNCTION__ << '\n'; } while (0)
#ifndef DEBUG_STARTFUNC
#define DEBUG_STARTFUNC false
#endif

#define dbg(x) do { if (DEBUG_DBG) std::cerr << __FILE__ << ':' << __LINE__ \
  << '(' << __FUNCTION__ << "): " << x << '\n'; } while (0)
#ifndef DEBUG_DBG
#define DEBUG_DBG false
#endif

#ifdef __GNUC__
#define UNUSED(x) x //__attribute__ ((unused))
#else
#define UNUSED(x) x
#endif

namespace shevek
{
  // some functions for debugging

  // hexdump
  inline void dump (std::string const &data, std::ostream &target,
		    char def = '.')
  {
    unsigned l = data.size ();
    for (unsigned y = 0; y < l; y += 16)
      {
	target << std::setfill (' ') << std::setw (3) << std::hex << y << ':'
	       << std::setfill ('0');
	for (unsigned x = 0; x < 16; ++x)
	  {
	    if (y + x >= l) target << "   ";
	    else target << " " << std::setw (2)	<< std::hex
			<< unsigned (data[y + x] & 0xff);
	  }
	target << '\t';
	for (unsigned x = 0; x < 16; ++x)
	  {
	    if (y + x >= l) break;
	    if (::isprint (data[y + x]) ) std::cerr << data[y + x];
	    else std::cerr << def;
	  }
	target << std::setfill (' ') << '\n';
      }
  }
}

#endif // defined (SHEVEK_DEBUG_HH)
