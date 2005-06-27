/* time.icc - inline function implementations for time.hh         -*- C++ -*-
 * $Header$
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

#include "time.hpp"
#include "debug.hpp"
#include "error.hpp"
#include <sys/time.h>
#include <time.h>
#include <iomanip>
//#include <glibmm.h>

namespace shevek
{
  // now
  absolute_time::absolute_time ()
  {
    startfunc;
    struct timeval tv;
    if (gettimeofday (&tv, 0) )
      shevek_error ("error returned from gettimeofday");
    m_seconds = tv.tv_sec;
    m_nanoseconds = tv.tv_usec * 1000;
  }

  // a specific time.  days may be 0-365, with months 0.
  // if months > 0, both days and months have a base of 1.
  absolute_time::absolute_time (unsigned years, unsigned months, unsigned days,
				unsigned hours, unsigned minutes,
				unsigned seconds, unsigned nanoseconds)
    : m_seconds (seconds), m_nanoseconds (nanoseconds)
  {
    startfunc;
    timetype y = years;
    y -= 1970;
    if (months != 0)
      {
	static unsigned const daysofmonth[12]
	  = {0,
	     31,
	     31 + 28,
	     31 + 28 + 31,
	     31 + 28 + 31 + 30,
	     31 + 28 + 31 + 30 + 31,
	     31 + 28 + 31 + 30 + 31 + 30,
	     31 + 28 + 31 + 30 + 31 + 30 + 31,
	     31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
	     31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
	     31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
	     31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30};
	if (months > 2 && (y & 3) == 2) ++days;
	// months and days are 1-based, not 0
	--months;
	--days;
	days += daysofmonth[months];
      }
    days += 365 * y;
    days += (y - 3) / 4; // leap years
    hours += days * 24;
    minutes += hours * 60;
    m_seconds += minutes * 60;
  }

  // fast constructor
  absolute_time::absolute_time (timetype seconds, unsigned nanoseconds)
    : m_seconds (seconds), m_nanoseconds (nanoseconds)
  {
    startfunc;
  }

  // do computations
  absolute_time absolute_time::operator+ (relative_time that) const
  {
    startfunc;
    absolute_time t (*this);
    if (that.isnegative () )
      {
	if (t.m_nanoseconds < -that.nanoseconds () )
	  {
	    t.m_nanoseconds += 1000000000;
	    --t.m_seconds;
	  }
	t.m_nanoseconds -= that.nanoseconds ();
      }
    else
      {
	t.m_nanoseconds += that.nanoseconds ();
	if (t.m_nanoseconds > 1000000000)
	  {
	    t.m_nanoseconds -= 1000000000;
	    ++t.m_seconds;
	  }
      }
    t.m_seconds += that.total ();
    return t;
  }

  absolute_time absolute_time::operator- (relative_time that) const
  {
    startfunc;
    absolute_time t (*this);
    if (that.isnegative () )
      {
	t.m_nanoseconds += that.nanoseconds ();
	if (t.m_nanoseconds > 1000000000)
	  {
	    t.m_nanoseconds -= 1000000000;
	    ++t.m_seconds;
	  }
      }
    else
      {
	if (t.m_nanoseconds < that.nanoseconds () )
	  {
	    t.m_nanoseconds += 1000000000;
	    --t.m_seconds;
	  }
	t.m_nanoseconds -= that.nanoseconds ();
      }
    t.m_seconds -= that.total ();
    return t;
  }

  relative_time absolute_time::operator- (absolute_time that) const
  {
    startfunc;
    timetype s = m_seconds;
    int ns = m_nanoseconds;
    ns -= that.m_nanoseconds;
    s -= that.m_seconds;
    return relative_time (s, ns);
  }

  absolute_time &absolute_time::operator+= (relative_time that)
  {
    startfunc;
    return *this = *this + that;
  }

  absolute_time &absolute_time::operator-= (relative_time that)
  {
    startfunc;
    return *this = *this - that;
  }

  bool absolute_time::operator< (absolute_time that) const
  {
    if (m_seconds == that.m_seconds) return m_nanoseconds < that.m_nanoseconds;
    else return m_seconds < that.m_seconds;
  }

  bool absolute_time::operator> (absolute_time that) const
  {
    if (m_seconds == that.m_seconds) return m_nanoseconds > that.m_nanoseconds;
    else return m_seconds > that.m_seconds;
  }

  bool absolute_time::operator<= (absolute_time that) const
  {
    if (m_seconds == that.m_seconds)
      {
	return m_nanoseconds <= that.m_nanoseconds;
      }
    else return m_seconds <= that.m_seconds;
  }

  bool absolute_time::operator>= (absolute_time that) const
  {
    if (m_seconds == that.m_seconds)
      {
	return m_nanoseconds >= that.m_nanoseconds;
      }
    else return m_seconds >= that.m_seconds;
  }

  bool absolute_time::operator== (absolute_time that) const
  {
    return m_seconds == that.m_seconds && m_nanoseconds == that.m_nanoseconds;
  }

  bool absolute_time::operator!= (absolute_time that) const
  {
    return m_seconds != that.m_seconds || m_nanoseconds != that.m_nanoseconds;
  }

  unsigned absolute_time::nanoseconds () const
  {
    startfunc;
    return m_nanoseconds;
  }

  unsigned absolute_time::second () const
  {
    startfunc;
    struct tm st;
    time_t t = m_seconds;
    if (!gmtime_r (&t, &st) )
      shevek_error ("call to gmtime_r failed");
    return st.tm_sec;
  }

  unsigned absolute_time::minute () const
  {
    startfunc;
    struct tm st;
    time_t t = m_seconds;
    if (!gmtime_r (&t, &st) )
      shevek_error ("call to gmtime_r failed");
    return st.tm_min;
  }

  unsigned absolute_time::hour () const
  {
    startfunc;
    struct tm st;
    time_t t = m_seconds;
    if (!gmtime_r (&t, &st) )
      shevek_error ("call to gmtime_r failed");
    return st.tm_hour;
  }

  unsigned absolute_time::days () const // 0-365
  {
    startfunc;
    struct tm st;
    time_t t = m_seconds;
    if (!gmtime_r (&t, &st) )
      shevek_error ("call to gmtime_r failed");
    return st.tm_yday;
  }

  unsigned absolute_time::day () const // 1-31
  {
    startfunc;
    struct tm st;
    time_t t = m_seconds;
    if (!gmtime_r (&t, &st) )
      shevek_error ("call to gmtime_r failed");
    return st.tm_mday;
  }

  unsigned absolute_time::month () const
  {
    startfunc;
    struct tm st;
    time_t t = m_seconds;
    if (!gmtime_r (&t, &st) )
      shevek_error ("call to gmtime_r failed");
    return st.tm_mon;
  }

  unsigned absolute_time::year () const
  {
    startfunc;
    struct tm st;
    time_t t = m_seconds;
    if (!gmtime_r (&t, &st) )
      shevek_error ("call to gmtime_r failed");
    return st.tm_year + 1900;
  }

  // total number of seconds, as encoded
  timetype absolute_time::total () const
  {
    startfunc;
    return m_seconds;
  }

  // schedule wrapper, to change function return type
//  bool absolute_time::l_schedule (sigc::slot0 <void> callback)
//  {
//    callback ();
    // don't reschedule
//    return false;
//  }

  // schedule a callback when the main loop is ready
//  sigc::connection schedule (sigc::slot0 <void> callback,
//			     Glib::RefPtr <Glib::MainContext> context)
//  {
//    return context->signal_idle ().
//      connect (sigc::bind (sigc::ptr_fun (&absolute_time::l_schedule),
//			   callback), Glib::PRIORITY_HIGH);
//  }

  // schedule a callback
//  sigc::connection absolute_time::schedule (sigc::slot0 <void> callback,
//					    Glib::RefPtr <Glib::MainContext>
//					    context)
//  {
//    relative_time t = *this - absolute_time ();
//    unsigned time;
//    if (t.isnegative () ) time = 0;
//    else time = t.total () * 1000 + t.nanoseconds () / 1000000;
//    dbg ("Scheduling an event in " << time << " milliseconds.");
//    return context->signal_timeout ().
//      connect (sigc::bind (sigc::ptr_fun (&l_schedule), callback), time);
//  }

  // no timedifference (that is, 0)
  relative_time::relative_time ()
    : m_seconds (0), m_nanoseconds (0)
  {
    startfunc;
    l_clean ();
  }

  // a specific time.
  relative_time::relative_time (timetype days, int hours,
				int minutes, int seconds,
				int nanoseconds)
    : m_seconds (seconds), m_nanoseconds (nanoseconds)
  {
    startfunc;
    hours += 24 * days;
    minutes += 60 * hours;
    m_seconds += 60 * minutes;
    l_clean ();
  }

  // fast constructor
  relative_time::relative_time (timetype seconds, unsigned nanoseconds)
    : m_seconds (seconds), m_nanoseconds (nanoseconds)
  {
    startfunc;
    l_clean ();
  }

  // do computations
  relative_time relative_time::operator+ (relative_time that) const
  {
    startfunc;
    relative_time t (*this);
    t.m_nanoseconds += that.m_nanoseconds;
    t.m_seconds += that.m_seconds;
    t.l_clean ();
    return t;
  }

  absolute_time relative_time::operator+ (absolute_time that) const
  {
    startfunc;
    return that + *this;
  }

  relative_time relative_time::operator- (relative_time that) const
  {
    startfunc;
    relative_time t (*this);
    t.m_nanoseconds -= that.m_nanoseconds;
    t.m_seconds -= that.total ();
    t.l_clean ();
    return t;
  }

  relative_time relative_time::operator- () const
  {
    startfunc;
    relative_time t (*this);
    t.m_seconds = -t.m_seconds;
    t.m_nanoseconds = -t.m_nanoseconds;
    t.l_clean ();
    return t;
  }

  relative_time relative_time::operator* (float c) const
  {
    startfunc;
    timetype s = m_seconds;
    int ns = m_nanoseconds;
    ns = int (ns * c);
    double part = s * c;
    s = timetype (s * c);
    part -= s;
    ns += int (1000000000 * part);
    relative_time t (s, ns);
    return t;
  }

  relative_time relative_time::operator/ (float c) const
  {
    startfunc;
    timetype s = m_seconds;
    int ns = m_nanoseconds;
    ns = int (ns / c);
    double part = s / c;
    s = timetype (s / c);
    part -= s;
    ns += int (1000000000 * part);
    relative_time t (s, ns);
    return t;
  }

  double relative_time::operator/ (relative_time that) const
  {
    startfunc;
    return (m_seconds * 1000000000.0 + m_nanoseconds) * 1.0
      / (that.m_seconds * 1000000000.0 + that.m_nanoseconds);
  }

  relative_time relative_time::operator% (relative_time that) const
  {
    startfunc;
    if (that == relative_time () )
      shevek_error ("division by zero");
    int witherror = int (*this / that);
    relative_time ret = that * witherror;
    while (ret.m_seconds < 0)
      if (that.m_seconds < 0)
	ret -= that;
      else
	ret += that;
    while (ret > that)
      if (that.m_seconds < 0)
	ret += that;
      else
	ret -= that;
    return ret;
  }

  relative_time &relative_time::operator+= (relative_time that)
  {
    startfunc;
    return *this = *this + that;
  }

  relative_time &relative_time::operator-= (relative_time that)
  {
    startfunc;
    return *this = *this - that;
  }

  relative_time &relative_time::operator*= (float c)
  {
    startfunc;
    return *this = *this * c;
  }

  relative_time &relative_time::operator/= (float c)
  {
    startfunc;
    return *this = *this / c;
  }

  relative_time &relative_time::operator%= (relative_time that)
  {
    startfunc;
    return *this = *this % that;
  }

  bool relative_time::operator< (relative_time that) const
  {
    if (m_seconds == that.m_seconds) return m_nanoseconds < that.m_nanoseconds;
    else return m_seconds < that.m_seconds;
  }

  bool relative_time::operator> (relative_time that) const
  {
    if (m_seconds == that.m_seconds) return m_nanoseconds > that.m_nanoseconds;
    else return m_seconds > that.m_seconds;
  }

  bool relative_time::operator<= (relative_time that) const
  {
    if (m_seconds == that.m_seconds)
      {
	return m_nanoseconds <= that.m_nanoseconds;
      }
    else return m_seconds <= that.m_seconds;
  }

  bool relative_time::operator>= (relative_time that) const
  {
    if (m_seconds == that.m_seconds)
      {
	return m_nanoseconds >= that.m_nanoseconds;
      }
    else return m_seconds >= that.m_seconds;
  }

  bool relative_time::operator== (relative_time that) const
  {
    return m_seconds == that.m_seconds && m_nanoseconds == that.m_nanoseconds;
  }

  bool relative_time::operator!= (relative_time that) const
  {
    return m_seconds != that.m_seconds || m_nanoseconds != that.m_nanoseconds;
  }

  unsigned relative_time::nanoseconds () const
  {
    startfunc;
    return m_nanoseconds < 0 ? -m_nanoseconds : m_nanoseconds;
  }

  unsigned relative_time::seconds () const
  {
    startfunc;
    timetype s = m_seconds;
    if (s < 0) s = -s;
    return s % 60;
  }

  unsigned relative_time::minutes () const
  {
    startfunc;
    timetype s = m_seconds;
    if (s < 0) s = -s;
    return (s / 60) % 60;
  }

  unsigned relative_time::hours () const
  {
    startfunc;
    timetype s = m_seconds;
    if (s < 0) s = -s;
    return (s / (60 * 60) ) % 24;
  }

  unsigned relative_time::days () const
  {
    startfunc;
    timetype s = m_seconds;
    if (s < 0) s = -s;
    return s / (60 * 60 * 24);
  }

  bool relative_time::isnegative () const
  {
    startfunc;
    return m_seconds < 0 || m_nanoseconds < 0;
  }

  // total number of seconds, as encoded
  timetype relative_time::total () const
  {
    startfunc;
    return m_seconds;
  }

  void relative_time::l_clean ()
  {
    if (m_nanoseconds >= 1000000000)
      {
	while (m_nanoseconds >= 1000000000)
	  {
	    m_nanoseconds -= 1000000000;
	    ++m_seconds;
	  }
	if (m_seconds < 0)
	  {
	    m_nanoseconds -= 1000000000;
	    ++m_seconds;
	  }
      }
    else if (m_nanoseconds <= -1000000000)
      {
	while (m_nanoseconds <= -1000000000)
	  {
	    m_nanoseconds += 1000000000;
	    --m_seconds;
	  }
	if (m_seconds > 0)
	  {
	    m_nanoseconds += 1000000000;
	    --m_seconds;
	  }
      }
    else if (m_nanoseconds < 0 && m_seconds > 0)
      {
	m_nanoseconds += 1000000000;
	--m_seconds;
      }
    else if (m_nanoseconds > 0 && m_seconds < 0)
      {
	m_nanoseconds -= 1000000000;
	++m_seconds;
      }
  }

  namespace
  {
    int get_next (std::istream &s, char before, bool last = false,
		  bool allow_negative = false)
    {
      if (!s)
	return 0;
      if (before != 0)
	{
	  char c;
	  s >> c;
	  if (c != before)
	    {
	      s.setstate (std::ios::failbit);
	      return 0;
	    }
	}
      if (last)
	return 0;
      int retval;
      s >> retval;
      if (!s)
	return 0;
      if (retval < 0 && !allow_negative)
	{
	  s.setstate (std::ios::failbit);
	  return 0;
	}
      return retval;
    }
  }

  std::ostream &operator<< (std::ostream &s, absolute_time t)
  {
    s << '[';
    s << std::setfill ('0') << std::setw (4) << t.year () << '-'
      << std::setw (2) << (t.month () + 1) << '-'
      << std::setw (2) << t.day () << '/'
      << std::setw (2) << t.hour () << ':'
      << std::setw (2) << t.minute () << ':'
      << std::setw (2) << t.second () << '.'
      << std::setw (9) << t.nanoseconds () << ']'
      << std::setfill (' ');
    return s;
  }

  std::istream &operator>> (std::istream &s, absolute_time &t)
  {
    unsigned year = get_next (s, '[');
    unsigned month = get_next (s, '-');
    unsigned day = get_next (s, '-');
    unsigned hour = get_next (s, '/');
    unsigned minute = get_next (s, ':');
    unsigned second = get_next (s, ':');
    unsigned nanosecond = get_next (s, '.');
    get_next (s, ']', true);
    if (s)
      t = absolute_time (year, month, day, hour, minute, second, nanosecond);
    return s;
  }

  std::ostream &operator<< (std::ostream &s, relative_time t)
  {
    s << '{';
    if (t.isnegative () ) s << '-';
    if (t.days () != 0) s << t.days () << '/';
    s << std::setfill ('0') << std::setw (2) << t.hours () << ':'
      << std::setw (2) << t.minutes () << ':'
      << std::setw (2) << t.seconds () << '.'
      << std::setw (9) << t.nanoseconds () << '}'
      << std::setfill (' ');
    return s;
  }

  std::istream &operator>> (std::istream &s, relative_time &t)
  {
    int day = get_next (s, '{', false, true);
    int hour;
    char c;
    s >> c;
    if (c == '/')
      {
	hour = get_next (s, 0);
	get_next (s, ':', true);
      }
    else if (c == ':')
      {
	hour = day;
	day = 0;
      }
    else
      {
	s.setstate (std::ios::failbit);
	return s;
      }
    int minute = get_next (s, 0);
    int second = get_next (s, ':');
    int nanosecond = get_next (s, '.');
    get_next (s, '}', true);
    if (s)
      t = relative_time (day, hour, minute, second, nanosecond);
    return s;
  }
}
