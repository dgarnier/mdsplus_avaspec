// $Header$
/* time.hh - class definitions to work with time.  All is in UTC, timezones
 * are not implemented.
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

#ifndef SHEVEK_TIME_HH
#define SHEVEK_TIME_HH

#include <iostream>
//#include <glibmm.h>

namespace shevek
{
  // schedule a callback when the main loop has time
//  sigc::connection schedule (sigc::slot0 <void> callback,
//			     Glib::RefPtr <Glib::MainContext> context
//			     = Glib::MainContext::get_default () );

  typedef int64_t timetype;
  class relative_time;

  class absolute_time
  {
    // number of seconds since epoch.
    timetype m_seconds;
    // number of nanoseconds.  Should be less than 1000 000 000.
    unsigned m_nanoseconds;
//    static bool l_schedule (sigc::slot0 <void> callback);
    // let schedule use l_schedule
//    friend
//    sigc::connection schedule (sigc::slot0 <void> callback,
//			       Glib::RefPtr <Glib::MainContext> context);
  public:
    // now
    absolute_time ();
    // a specific time.  days may be 0-365, with months 0.
    // if months > 0, both days and months have a base of 1.
    absolute_time (unsigned years, unsigned months, unsigned days,
		   unsigned hours, unsigned minutes, unsigned seconds,
		   unsigned nanoseconds = 0);
    // fast constructor
    absolute_time (timetype seconds, unsigned nanoseconds);
    // do computations
    absolute_time operator+ (relative_time that) const;
    absolute_time operator- (relative_time that) const;
    relative_time operator- (absolute_time that) const;
    absolute_time &operator+= (relative_time that);
    absolute_time &operator-= (relative_time that);
    // comparing
    bool operator< (absolute_time that) const;
    bool operator> (absolute_time that) const;
    bool operator<= (absolute_time that) const;
    bool operator>= (absolute_time that) const;
    bool operator== (absolute_time that) const;
    bool operator!= (absolute_time that) const;
    // read out
    unsigned nanoseconds () const;
    unsigned second () const;
    unsigned minute () const;
    unsigned hour () const;
    unsigned days () const; // 0-365
    unsigned day () const; // 1-31
    unsigned month () const;
    unsigned year () const;
    // total number of seconds, as encoded
    timetype total () const;
    // schedule a callback at a certain time
//    sigc::connection schedule (sigc::slot0 <void> callback,
//				      Glib::RefPtr <Glib::MainContext> context
//				      = Glib::MainContext::get_default () );
  };
  //std::ostream &operator<< (std::ostream &s, absolute_time t);

  class relative_time
  {
    // number of seconds.
    timetype m_seconds;
    // number of nanoseconds.  Should be less than 1000000000.
    int m_nanoseconds;
  public:
    // no timedifference (that is, 0)
    relative_time ();
    // a specific time.  cannot set months or years, because they depend on
    // the offset (think leap years)
    relative_time (timetype days, int hours, int minutes, int seconds,
			  int nanoseconds = 0);
    // fast constructor
    relative_time (timetype seconds, unsigned nanoseconds);
    // do computations
    relative_time operator+ (relative_time that) const;
    absolute_time operator+ (absolute_time that) const;
    relative_time operator- (relative_time that) const;
    relative_time operator- () const;
    relative_time operator* (float c) const;
    relative_time operator/ (float c) const;
    relative_time operator% (relative_time that) const;
    double operator/ (relative_time that) const;
    relative_time &operator+= (relative_time that);
    relative_time &operator-= (relative_time that);
    relative_time &operator*= (float c);
    relative_time &operator/= (float c);
    relative_time &operator%= (relative_time that);
    // comparing
    bool operator< (relative_time that) const;
    bool operator> (relative_time that) const;
    bool operator<= (relative_time that) const;
    bool operator>= (relative_time that) const;
    bool operator== (relative_time that) const;
    bool operator!= (relative_time that) const;
    // read out
    unsigned nanoseconds () const;
    unsigned seconds () const;
    unsigned minutes () const;
    unsigned hours () const;
    unsigned days () const;
    bool isnegative () const;
    // total number of seconds, as encoded
    timetype total () const;
  private:
    // internal function to clean the seconds/nanoseconds
    void l_clean ();
  };
  std::ostream &operator<< (std::ostream &s, absolute_time t);
  std::ostream &operator<< (std::ostream &s, relative_time t);
  std::istream &operator>> (std::istream &s, absolute_time &t);
  std::istream &operator>> (std::istream &s, relative_time &t);
}


#endif
