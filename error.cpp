/* error.hh - error and warning handling
 * Copyright (C) 2004 Bas Wijnen <b.wijnen@phys.rug.nl>
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

#include "error.hpp"
//#include <glibmm.h>
#include <iostream>
#include <stdexcept>

namespace shevek
{
  static bool _break_on_error = true;

  void _error_impl (std::string const &message, bool is_error)
  {
    std::cerr << (is_error ? "Error: " : "Warning: ") << message << '\n';
    if (is_error && _break_on_error)
      {
		 throw std::runtime_error(message);
      }
    /*
    else
      {
	std::cerr << (is_error ? "Error: " : "Warning: ") << message << '\n';
      }
    */
  }

  bool fatal_errors (bool fatal)
  {
    bool retval = _break_on_error;
    _break_on_error = fatal;
    return retval;
  }
}
