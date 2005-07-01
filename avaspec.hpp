/* avaspec.hh - interface description of the driver for the Avaspec
 * spectrometer made by Avantes.
 * Copyright (C) 2003 Bas Wijnen <b.wijnen@phys.rug.nl>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License version 2
 * as published by the Free Software Foundation;
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

#ifndef AVASPEC_HH
#define AVASPEC_HH

#include <string>
#include <vector>
#include "time.hpp"
#include <usb.h>
#include <pthread.h>

class avaspec
{
public:
  class channel;
  // channel specific features are accessed through the channels
  channel &operator[] (unsigned idx);
  channel const &operator[] (unsigned idx) const;
  // measurement settings
  void set_integration_time (shevek::relative_time value);
  shevek::relative_time get_integration_time () const;
  shevek::relative_time get_measured_integration_time () const;
  void set_average (unsigned value);
  unsigned get_average () const;
  // other settings
  void set_digital (unsigned which, bool value);
  bool get_digital (unsigned which) const; // read digital out values
  bool get_digital (); // read software external trigger, pin 8 (DI2)
  void external_trigger (bool enable);
  bool get_external_trigger () const;
  void set_fixed_strobe (bool value);
  bool get_fixed_strobe () const;
  void set_strobe (unsigned number);
  unsigned get_strobe () const;
  // do a measurement.  The measurement is started with start_read.  When the
  // integration time has (almost) passed, end_read should be called.  It will
  // block until the data is fully received.
  void start_read ();
  void end_read ();
  void end_read_async();
  bool cancel_read_async();
  bool run_read_async(void);
  // write current data to eeprom.  Not advised to do often
  // (although the windows driver does it on every change)
  void write_eeprom (std::string const &password);
  // get general info about the device
  unsigned num_channels () const;
  unsigned num_pixels () const;
  unsigned extra_pixels () const;
  // constructor and destructor.
  avaspec (std::string const &config,
	   std::string const &device_file); // serial port
  avaspec (std::string const &config,
	   unsigned product, unsigned vendor, unsigned skip); // usb
  avaspec (std::string const &config); // emulation
  ~avaspec ();
  // read and change calibration values.  They are not written back at
  // the device until avaspec::write_eeprom is called.  It doesn't do anything
  // with them anyway (except remembering)
  void set_calibration (unsigned channel, unsigned which, float value);
  float get_calibration (unsigned channel, unsigned which) const;
  void set_start (unsigned channel, unsigned value);
  unsigned get_start (unsigned channel) const;
  void set_stop (unsigned channel, unsigned value);
  unsigned get_stop (unsigned channel) const;
  // get time of measurement (stored by end_read)
  shevek::absolute_time time () const;
  enum { MAX_DIGITAL = 10 };
protected:
      bool m_cancel_read;
private:
  struct
  {
    std::string version;
    unsigned id;
    unsigned sensor;
    // not in m_channel, because it exists for nonexisting channels as well
    struct
    {
      float calibration[7];
      unsigned start, stop;
    } channel[8];
  } m_eeprom;
  // objects of this class cannot be copied
  avaspec (avaspec const &);
  void operator= (avaspec const &);
  // internal functions
  // write a command, and wait for the reply, of which the first
  // character and size must match the given ones.  Size is not
  // checked if replysize == 0
  std::string l_readwrite (std::string const &message, char reply,
			   unsigned replysize, unsigned pausetime = 0);
  // the actual constructor code
  void init (std::string const &config);
  // data members
  shevek::relative_time m_integration_time, m_saved_integration_time,
    m_measured_time;
  shevek::absolute_time m_time; // last measuremnt
  unsigned m_average;
  unsigned m_numpixels, m_extra_pixels;
  bool m_digital[MAX_DIGITAL];
  bool m_external;
  bool m_fixed;
  pthread_t m_thread;
  unsigned m_strobe;
  std::vector <channel> m_channel;
  // hardware is a virtual class.  usb, serial and emulation inherit from it.
  // m_hardware does the hardware-specific parts.
  class hardware;
  class usb;
  class serial;
  class emulation;
  hardware *m_hardware;
};

// this class is where all channel specific features are accessed.
class avaspec::channel
{
public:
  // pixel values for the ranges, minimum inclusive, maximum exclusive
  void set_range (unsigned min, unsigned max);
  unsigned get_range_min () const;
  unsigned get_range_max () const;
  // read the measured data (taken when avaspec::read was called)
  unsigned operator[] (unsigned idx) const;
  unsigned extra (unsigned idx) const;
  std::vector <float> const &nonlinear () const;
  std::vector <float> const &ijking () const;
  shevek::relative_time ijktime () const;
private:
  // settings
  unsigned m_min, m_max, m_id;
  // which data is actually availabe (from last read)
  unsigned m_mindata, m_maxdata;
  // parent, to access m_hardware
  avaspec *m_parent;
  // measured spectrum
  std::vector <unsigned> m_data;
  std::vector <unsigned> m_extra;
  // ijking data
  std::vector <float> m_ijk;
  shevek::relative_time m_ijktime;
  std::vector <float> m_nonlinear;
  // function called by parent to load new data to m_data
  void new_data (std::string const &message);
  friend void avaspec::end_read ();
  friend bool avaspec::run_read_async();
  // because setup is not done in constructor, objects can be used in a vector
  void setup (avaspec *parent, unsigned id,
	      std::vector <float> const &ijkvector,
	      shevek::relative_time ijktime,
	      std::vector <float> const &nonlinear);
  friend void avaspec::init (std::string const &config);
};

class avaspec::hardware
{
  // not copyable
  hardware (hardware const &);
  void operator= (hardware const &);
public:
  hardware () {}
  virtual ~hardware () {}
  virtual void write_message (std::string const &message) = 0;
  virtual std::string read_message (unsigned timeout, unsigned replysize,
				    char reply) = 0;
};

class avaspec::usb : public avaspec::hardware
{
  int m_in_ep, m_out_ep;
  usb_dev_handle *m_handle;
  int m_interface;
  bool l_find_device (unsigned vendor, unsigned product, unsigned skip);
public:
  usb (unsigned vendor, unsigned product, unsigned skip);
  virtual ~usb ();
  virtual void write_message (std::string const &message);
  virtual std::string read_message (unsigned timeout, unsigned replysize,
				    char reply);
};

class avaspec::serial : public avaspec::hardware
{
  int m_fd;
  // id of next message
  static unsigned m_id;
  // buffer where the message is read into
  enum { BUFFERSIZE = 20000 };
  char m_buffer[BUFFERSIZE];
  // actual buffer size
  unsigned m_buffer_size;
  // function reading the device and replacing escape codes.
  // It returns the message without header and footer
  std::string l_read_message (unsigned timeout);
public:
  serial (std::string const &device_file);
  virtual ~serial ();
  virtual void write_message (std::string const &message);
  virtual std::string read_message (unsigned timeout, unsigned replysize,
				    char reply);
};

class avaspec::emulation : public avaspec::hardware
{
public:
  emulation ();
  virtual ~emulation ();
  virtual void write_message (std::string const &message);
  virtual std::string read_message (unsigned timeout, unsigned replysize,
				    char reply);
};

#endif // defined AVASPEC_HH
