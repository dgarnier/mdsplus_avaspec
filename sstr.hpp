/* sstr.hh - using stringstreams inline
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

#ifndef SHEVEK_SSTR_HH
#define SHEVEK_SSTR_HH

#include <sstream>

namespace shevek
{
  class _sstr_t : public std::ostringstream
  {
  public:
    std::ostream &t () { return *this; }
    std::string s () { std::string a = str (); delete this; return a; }
  };
}

#define sstr(x) \
dynamic_cast <shevek::_sstr_t &>( (new shevek::_sstr_t ())->t () << x).s ()

#define getstr(string,out) \
	do { std::istringstream tmp (string); tmp >> out; } while (0)

#endif
