/* avaspec.cc - implementation of the driver for the Avaspec
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


#include <iostream>
#include <fstream>
#include "avaspec.hpp"
#include <fcntl.h>     // open
#include <sys/stat.h>  // open
#include <sys/types.h> // open
#include <unistd.h>    // close
#include <termios.h>   // setting up the serial port
#include <errno.h>     // errno == -EINTR
#include "ieee754.h"   // float
#include <sys/poll.h>  // poll
#include "debug.hpp" // startfunc, dbg
#include "error.hpp"


avaspec::channel &avaspec::operator[] (unsigned idx)
{
  startfunc;
  if (idx >= m_channel.size () )
    {
      shevek_error ("channel index out of range (" << idx << " >= "
		    << m_channel.size () << ")");
      return m_channel[0];
    }
  return m_channel[idx];
}

avaspec::channel const &avaspec::operator[] (unsigned idx) const
{
  startfunc;
  if (idx >= m_channel.size () )
    {
      shevek_error ("channel index out of range (" << idx << " >= "
		    << m_channel.size () << ")");
      return m_channel[0];
    }
  return m_channel[idx];
}

void avaspec::set_integration_time (shevek::relative_time value)
{
  startfunc;
  m_integration_time = value;
}

shevek::relative_time avaspec::get_integration_time () const
{
  startfunc;
  return m_integration_time;
}

shevek::relative_time avaspec::get_measured_integration_time () const
{
  startfunc;
  return m_measured_time;
}

void avaspec::set_average (unsigned value)
{
  startfunc;
  m_average = value;
}

unsigned avaspec::get_average () const
{
  startfunc;
  return m_average;
}

void avaspec::set_digital (unsigned which, bool value)
{
  startfunc;
  if (which > MAX_DIGITAL)
    {
      shevek_error ("invalid digital output (" << which << " > " << MAX_DIGITAL
		    << ")");
      return;
    }
  m_digital[which] = value;
  std::string command ("\007\000\000", 3);
  command[1] = which;
  command[2] = value ? 1 : 0;
  l_readwrite (command, 0x87, 1);
}

bool avaspec::get_digital (unsigned which) const
{
  startfunc;
  if (which > MAX_DIGITAL)
    {
      shevek_error ("invalid digital output (" << which << " > " << MAX_DIGITAL
		    << ")");
      return bool ();
    }
  return m_digital[which];
}

bool avaspec::get_digital ()
{
  startfunc;
  std::string response = l_readwrite (std::string ("\006", 1), 0x86, 2);
  if (response[1] > 1)
    {
      shevek_error ("invalid response on get_digital_in from device ("
		    << response[1] << " > 1");
      return bool ();
    }
  return response[1];
}

void avaspec::external_trigger (bool enable)
{
  startfunc;
  m_external = enable;
  std::string command ("\011\000", 2);
  command[1] = enable ? 1 : 0;
  l_readwrite (command, 0x89, 1);
}

bool avaspec::get_external_trigger () const
{
  startfunc;
  return m_external;
}

void avaspec::set_fixed_strobe (bool value)
{
  startfunc;
  m_fixed = value;
  std::string command ("\012\000", 2);
  command[1] = value ? 1 : 0;
  l_readwrite (command, 0x8a, 1);
}

bool avaspec::get_fixed_strobe () const
{
  startfunc;
  return m_fixed;
}

void avaspec::set_strobe (unsigned number)
{
  startfunc;
  std::string command ("\013\000\000", 3);
  command[1] = number & 0xff;
  command[2] = (number >> 8) & 0xff;
  l_readwrite (command, 0x8b, 1);
  m_strobe = number;
}

unsigned avaspec::get_strobe () const
{
  startfunc;
  return m_strobe;
}

void avaspec::start_read ()
{
  startfunc;
  // don't read if a reading is in progress
  if (m_saved_integration_time != shevek::relative_time () )
    return;
  std::string command = std::string ("\003\000\000\000\000", 5);
  unsigned time_ms = m_integration_time.total () * 1000
    + m_integration_time.nanoseconds () / 1000000;
  dbg (time_ms);
  command[1] = time_ms & 0xff;
  command[2] = (time_ms >> 8) & 0xff;
  command[3] = m_average & 0xff;
  command[4] = (m_average >> 8) & 0xff;
  m_hardware->write_message (command);
  m_saved_integration_time = m_integration_time;
}

//unsigned avaspec::send_get_first_chan(void)
//{
//    std::string command();
//    for (unsigned channel = 0; channel < m_channel.size (); ++channel)
//    {
//        if (m_channel[channel].get_range_max ()
//            <= m_channel[channel].get_range_min () ) continue;
//        command[1] = channel & 0xff;
//        m_hardware->write_message(command);
//        return first_chan;
//    }
//}
//                                     
//bool avaspec::check_read(unsigned timeout, unsigned first)
//{
//    // read message (timeout_ms,replylen,reply)
//    std::string data = m_hardware->read_message(timeout,0,0x83);
//    if (data.size()==0) return false;
//    
//    m_channel[first].new_data(data);
//    
//    for (unsigned channel=first+1; channel < m_channel.size(); ++channel) {
//        if (m_channel[channel].get_range_max ()
//            <= m_channel[channel].get_range_min () ) continue;
//        command = std::string("\004\000",2);
//        data = l_readwrite(command, 0x83, 0, 0);
//        m_channel[first].new_data(data);
//    }
//    return true;
//}
//
bool avaspec::run_read_async(void)
{
    std::string command;
    std::string data;
    unsigned channel;
    
    unsigned time_ms = m_integration_time.total () * 1000
        + m_integration_time.nanoseconds () / 1000000;

    poll(0,0,time_ms - 10);
    
    for (channel = 0; channel < m_channel.size (); ++channel) {
        if (m_channel[channel].get_range_max ()
                <= m_channel[channel].get_range_min () ) continue;
            command[1] = channel & 0xff;
            break ;
    }

    m_hardware->write_message(command);

    do {
        // read message (timeout_ms,replylen,reply)
        data = m_hardware->read_message(20,0,0x83);
    } while ((data.size()==0)&&(m_cancel_read==false));
    
    if (m_cancel_read) return false;
    
    m_channel[channel].new_data(data);
    
    for (++channel; channel < m_channel.size(); ++channel) {
        if (m_channel[channel].get_range_max ()
            <= m_channel[channel].get_range_min () ) continue;
        command = std::string("\004\000",2);
        data = l_readwrite(command, 0x83, 0, 0);
        m_channel[channel].new_data(data);
    }
    return true;
}



static void * async_read_thread_wrapper(void * p)
{
    static bool complete;
    avaspec *s;
    
    s = reinterpret_cast<avaspec *>( p );
    
    complete = s->run_read_async();
    
    return reinterpret_cast<void *>( &complete );
}

void avaspec::end_read_async()
{
    int err;
    
    m_cancel_read = false;
    
    
    if ((err = pthread_create(&m_thread,NULL,
                             async_read_thread_wrapper,
                             reinterpret_cast<void *>(this)))) {
        m_thread = NULL;
    }
}

bool avaspec::cancel_read_async()
{
    bool *result_p;
    
    if (m_thread == NULL) return false;
    
    m_cancel_read = true;
    
    pthread_join(m_thread, reinterpret_cast<void **>(&result_p));
    
    m_thread = NULL;
    
    return *result_p;
}

void avaspec::end_read ()
{
  startfunc;
  bool first = true;
  std::string command; // start with empty command: only read
  unsigned pausetime = m_saved_integration_time.total () * 1000
    + m_saved_integration_time.nanoseconds () / 1000000;
  for (unsigned channel = 0; channel < m_channel.size (); ++channel)
    {
      if (m_channel[channel].get_range_max ()
	  <= m_channel[channel].get_range_min () ) continue;
      if (!first)
	command[1] = channel & 0xff;
      std::string data = l_readwrite (command, 0x83, 0, pausetime);
      // record the time
      if (first)
	m_time = shevek::absolute_time ();
      m_channel[channel].new_data (data);
      command = std::string ("\004\000", 2);
      first = false;
      pausetime = 0;
    }
  m_measured_time = m_saved_integration_time;
  m_saved_integration_time = shevek::relative_time ();
}

void avaspec::write_eeprom (std::string const &password)
{
  startfunc;
  if (password.size () != 15)
    {
      shevek_error ("incorrect password size (" << password.size ()
		    << " != 15)");
      return;
    }
  std::string command ("\002", 1); // size = 0x01
  command += password; // size = 0x0f
  command += '\000'; // size = 0x10
  command += m_eeprom.version; // size = 0x50
  command += std::string ("\000\000\000\000\000\000", 6); //size = 0x56
  command[0x51] = m_eeprom.id & 0xff;
  command[0x52] = (m_eeprom.id >> 8) & 0xff;
  command[0x53] = m_channel.size ();
  command[0x54] = (m_numpixels + m_extra_pixels) & 0xff;
  command[0x55] = ( (m_numpixels + m_extra_pixels) >> 8) & 0xff;
  command[0x56] = m_eeprom.sensor; // whatever that is
  for (unsigned i = 0; i < 8; ++i)
    {
      for (unsigned f = 0; f < 7; ++f)
	{
	  union ieee754_float num;
	  num.f = m_eeprom.channel[i].calibration[f];
	  char fl[4];
	  fl[0] = fl[1] = fl[2] = fl[3] = 0;
	  fl[3] |= num.ieee.negative << 7;
	  fl[3] |= (num.ieee.exponent & 0xfe) >> 1;
	  fl[2] |= (num.ieee.exponent & 0x01) << 7;
	  fl[2] |= (num.ieee.mantissa >> 16) & 0x7f;
	  fl[1] = (num.ieee.mantissa >> 8) & 0xff;
	  fl[0] = num.ieee.mantissa & 0xff;
	  command += std::string (fl, 4);
	}
      char limits[4];
      limits[0] = m_eeprom.channel[i].start & 0xff;
      limits[1] = (m_eeprom.channel[i].start >> 8) & 0xff;
      limits[2] = (m_eeprom.channel[i].stop - 1) & 0xff;
      limits[3] = ( (m_eeprom.channel[i].stop - 1) >> 8) & 0xff;
      command += std::string (limits, 4);
    }
  // write the whole thing to the device, handle throws (incorrect password)
  try
    {
      l_readwrite (command, 0x82, 1);
    }
  catch (char const *msg)
    {
      if (std::string ("incorrect reply") == msg)
	{
	  shevek_error ("incorrect password");
	  return;
	}
      shevek_error (msg);
      return;
    }
}

unsigned avaspec::num_channels () const
{
  startfunc;
  return m_channel.size ();
}

unsigned avaspec::num_pixels () const
{
  startfunc;
  return m_numpixels;
}

unsigned avaspec::extra_pixels () const
{
  startfunc;
  return m_extra_pixels;
}

void avaspec::init (std::string const &config)
{
  startfunc;
  std::ifstream configfile (config.c_str () );
  if (!configfile)
    {
      m_integration_time = shevek::relative_time (0, 0, 0, 0, 100000000);
      m_average = 1;
    }
  else
    {
      configfile >> m_integration_time >> m_average;
      std::string line;
      std::getline (configfile, line);
      if (!configfile)
	{
	  shevek_error ("invalid configfile");
	  return;
	}
    }
  std::string status = l_readwrite (std::string ("\001", 1), 0x81, 327);
  //0x40 bytes version
  m_eeprom.version = std::string (&status[0x01], 0x40);
  //0x02 bytes device id
  m_eeprom.id = (status[0x41] & 0xff) + ( (status[0x42] & 0xff) << 8);
  //0x01 byte number of channels in this device
  unsigned numchannels = status[0x43];
  if (numchannels > 7)
    {
      shevek_error ("invalid number of channels (" << numchannels << " > 7)");
      return;
    }
  m_channel.resize (numchannels);
  //0x02 bytes number of pixels per channel
  m_numpixels = (status[0x44] & 0xff) + ( (status[0x45] & 0xff) << 8);
  if (m_numpixels > 0x800)
    {
      shevek_error ("invalid number of pixels (" << m_numpixels << " > 0x800)");
      return;
    }
  if (m_numpixels == 0x800)
    m_extra_pixels = 14;
  else
    m_extra_pixels = 0;
  m_numpixels -= m_extra_pixels;
  //0x01 byte sensor (?)
  m_eeprom.sensor = status[0x46] & 0xff;
  //for each channel, 0x20 bytes
  for (unsigned i = 0; i < 8; ++i)
    {
      // 0x04 * 0x07 bytes calibration data (5 fit, 1 gain, 1 offset)
      for (unsigned f = 0; f < 7; ++f)
	{
	  union ieee754_float num;
	  char const *data = status.data () + 0x47 + i * 0x20 + 4 * f;
	  num.ieee.negative = (data[3] & 0x80) != 0;
	  num.ieee.exponent = ( (data[3] & 0x7f) << 1)
	    + ( ( (data[2] & 0xff) & 0x80) >> 7);
	  num.ieee.mantissa = ( (data[2] & 0x7f) << 16)
	    + ( (data[1] & 0xff) << 8) + (data[0] & 0xff);
	  m_eeprom.channel[i].calibration[f] = num.f;
	}
      //0x02 bytes start pixel
      m_eeprom.channel[i].start = (status[0x63 + i * 0x20] & 0xff)
	+ ( (status[0x63 + i * 0x20 + 1] & 0xff) << 8);
      //0x02 bytes stop pixel, +1 because of the weird format (incl. endpoint)
      m_eeprom.channel[i].stop = (status[0x65 + i * 0x20] & 0xff)
	+ ( (status[0x65 + i * 0x20 + 1] & 0xff) << 8) + 1;
      //use ijking
      std::vector <float> ijkvector;
      shevek::relative_time ijktime;
      std::vector <float> nl;
      if (configfile) do
	{
	  std::string line;
	  std::getline (configfile, line);
	  std::istringstream nlstream (line);
	  nl.resize (8);
	  nlstream >> nl[0] >> nl[1] >> nl[2] >> nl[3] >> nl[4] >> nl[5]
		   >> nl[6] >> nl[7];
	  if (!nlstream)
	    nl.resize (0);
	  std::getline (configfile, line);
	  if (!configfile)
	    break;
	  std::ifstream ijk (line.c_str () );
	  unsigned start;
	  ijk >> ijktime >> start;
	  if (start >= m_numpixels)
	    {
	      shevek_error ("invalid start for ijk: " << start << " >= "
			    << m_numpixels);
	      return;
	    }
	  ijkvector.resize (m_numpixels - start);
	  unsigned i;
	  for (i = start; i < m_numpixels; ++i)
	    {
	      float ijking;
	      ijk >> ijking;
	      if (!ijk)
		break;
	      ijkvector[i - start] = ijking;
	    }
	  ijkvector.resize (i - start);
	} while (0);
      //setup the channel structure with all the info
      if (i < numchannels)
	m_channel[i].setup (this, i, ijkvector, ijktime, nl);
    }
  // disable external trigger
  l_readwrite (std::string ("\011\000", 2), 0x89, 1);
}

avaspec::avaspec (std::string const &config, std::string const &device)
{
  startfunc;
  m_hardware = new serial (device);
  try
    {
      init (config);
    }
  catch (...)
    {
      delete m_hardware;
      throw;
    }
}

avaspec::avaspec (std::string const &config,
		  unsigned vendor, unsigned product, unsigned skip)
{
  startfunc;
  m_hardware = new usb (vendor, product, skip);
  try
    {
      init (config);
    }
  catch (...)
    {
      delete m_hardware;
      throw;
    }
}

avaspec::avaspec (std::string const &config)
{
  startfunc;
  m_hardware = new emulation ();
  try
    {
      init (config);
    }
  catch (...)
    {
      delete m_hardware;
      throw;
    }
}

avaspec::~avaspec ()
{
  startfunc;
  delete m_hardware;
}

void avaspec::set_calibration (unsigned channel, unsigned which, float value)
{
  startfunc;
  if (which >= 7 || channel >= 8)
    {
      shevek_error ("invalid argument for calibration (" << which
		    << " >= 7 || " << channel << " >= 8)");
      return;
    }
  m_eeprom.channel[channel].calibration[which] = value;
}

float avaspec::get_calibration (unsigned channel, unsigned which) const
{
  startfunc;
  if (which >= 7 || channel >= 8)
    {
      shevek_error ("invalid argument for calibration (" << which
		    << " >= 7 || " << channel << " >= 8)");
      return float ();
    }
  return m_eeprom.channel[channel].calibration[which];
}

void avaspec::set_start (unsigned channel, unsigned value)
{
  startfunc;
  if (channel >= 8)
    {
      shevek_error ("invalid channel (" << channel << " >= 8)");
      return;
    }
  m_eeprom.channel[channel].start = value;
}

unsigned avaspec::get_start (unsigned channel) const
{
  startfunc;
  if (channel >= 8)
    {
      shevek_error ("invalid channel (" << channel << " >= 8)");
      return unsigned ();
    }
  return m_eeprom.channel[channel].start;
}

void avaspec::set_stop (unsigned channel, unsigned value)
{
  startfunc;
  if (channel >= 8)
    {
      shevek_error ("invalid channel (" << channel << " >= 8)");
      return;
    }
  m_eeprom.channel[channel].stop = value;
}

unsigned avaspec::get_stop (unsigned channel) const
{
  startfunc;
  if (channel >= 8)
    {
      shevek_error ("invalid channel (" << channel << " >= 8)");
      return unsigned ();
    }
  return m_eeprom.channel[channel].stop;
}

std::string avaspec::l_readwrite (std::string const &message, char reply,
				  unsigned replysize, unsigned pausetime)
{
  startfunc;
  std::string result;
  if (!message.empty () ) m_hardware->write_message (message);
  if (pausetime)
    poll (0, 0, pausetime);
  result = m_hardware->read_message (1000, replysize, reply);
  if (result.size () == 0)
    {
      shevek_error ("empty reply");
      return std::string ();
    }
  if ( (replysize && replysize != result.size () )
      && (result.size () != 2 || result[0] != 0) )
    {
      shevek_error ("incorrect reply size (" << result.size () << " != "
		    << replysize << ")");
      return std::string ();
    }
  if (result[0] == 0)
    {
      shevek_error ("device returned error: " << unsigned (result[1]) );
      return std::string ();
    }
  if (result[0] != reply)
    {
      shevek_warning ("expected " << unsigned (reply & 0xff) << ", got "
		      << unsigned (result[0] & 0xff) );
      // this must be a throw, not a shevek_error, to make it catchable for
      // incorrect passwords.  The message must also be the same as there.
      throw "incorrect reply";
    }
  return result;
}

bool avaspec::usb::l_find_device (unsigned vendor, unsigned product,
				  unsigned skip)
{
  startfunc;
  unsigned skipped = 0;
  for (struct usb_bus *bus = usb_get_busses (); bus; bus = bus->next)
    {
      for (struct usb_device *dev = bus->devices; dev; dev = dev->next)
	{
	  if (dev->descriptor.idVendor == vendor
	      && dev->descriptor.idProduct == product)
	    {
	      if (skip > skipped++)
		continue;
	      m_handle = usb_open (dev);
	      m_interface
		= dev->config->interface->altsetting->bInterfaceNumber;
	      bool have_in = false, have_out = false;
	      for (unsigned i = 0;
		   i < dev->config->interface->altsetting->bNumEndpoints; ++i)
		{
		  if ( (dev->config->interface->altsetting
			->endpoint[i].bmAttributes & USB_ENDPOINT_TYPE_MASK)
		       != USB_ENDPOINT_TYPE_BULK)
		    continue;
		  if ( (dev->config->interface->altsetting
			->endpoint[i].bEndpointAddress
			& USB_ENDPOINT_DIR_MASK) )
		    {
		      // input
		      if (have_in)
		        {
			  shevek_error ("invalid device: "
					"more than one input endpoint");
			  return false;
			}
		      have_in = true;
		      m_in_ep
			= dev->config->interface->altsetting
			->endpoint[i].bEndpointAddress
			& USB_ENDPOINT_ADDRESS_MASK;
		    }
		  else
		    {
		      // output
		      if (have_out)
		        {
			  shevek_error ("invalid device: "
					"more than one output endpoint");
			  return false;
			}
		      have_out = true;
		      m_out_ep
			= dev->config->interface->altsetting
			->endpoint[i].bEndpointAddress
			& USB_ENDPOINT_ADDRESS_MASK;
		    }
		} // loop over all endpoints
	      return true;
	    } // if this device has good vendor/product id
	} // loop over all devices connected to the bus
    } // loop over all usb busses
  return false;
}

avaspec::usb::usb (unsigned vendor, unsigned product, unsigned skip)
{
  startfunc;
  usb_init ();
  if (usb_find_busses () < 0)
    {
      shevek_error ("unable to find usb busses");
      return;
    }
  if (usb_find_devices () < 0)
    {
      shevek_error ("unable to find usb devices");
      return;
    }
  if (l_find_device (vendor, product, skip) )
    {
      int err;
      err = usb_reset (m_handle);
      if (err)
	shevek_error ("unable to reset usb device: " << strerror (-err) );
		err = usb_close(m_handle);
      if (l_find_device (vendor, product, skip) )
	{
	  int err = usb_claim_interface (m_handle, m_interface);
	  if (err < 0)
	    {
	      usb_close (m_handle);
	      shevek_warning ("unable to claim usb interface: "
			    << usb_strerror () );
	      return;
	    }
	  return;
        }
    }
  shevek_error ("unable to find specified usb device 0x" << std::hex << vendor
		<< "/0x" << product << ';' << std::dec << skip);
  return;
}

avaspec::usb::~usb ()
{
  startfunc;
  usb_release_interface (m_handle, m_interface);
  usb_close (m_handle);
}

void avaspec::usb::write_message (std::string const &message)
{
  startfunc;
  unsigned done = 0;
  while (true)
    {
      if (message.size () == done)
	return;
      int l = usb_bulk_write (m_handle, m_out_ep,
			      // I hate const casts, but the library wants
			      // a char *, not a char const *.  I don't
			      // expect it to change the data at all,
			      // though.
			      const_cast <char *> (message.data () ) + done,
			      message.size () - done, 1000);
      if (l < 0)
	{
	  shevek_error_errno ("unable to write message to usb device");
	  return;
	}
      done += l;
    }
}

std::string avaspec::usb::read_message (unsigned timeout, unsigned, char)
{
  startfunc;
  char buffer[6000];
  int l = usb_bulk_read (m_handle, m_in_ep, buffer, sizeof (buffer), timeout);
  if (l <= 0)
    {
//      shevek_error ("unable to read from usb device: " << usb_strerror());
      return std::string ();
    }
  return std::string (buffer, l);
}

unsigned avaspec::serial::m_id = 0;

avaspec::serial::serial (std::string const &device_file)
{
  startfunc;
  m_buffer_size = 0;
  m_fd = ::open (device_file.c_str(), O_RDWR | O_NOCTTY);
  if (m_fd < 0)
    {
      shevek_error_errno ("unable to open device file");
      return;
    }
  struct termios tio;
  ::memset (&tio, 0, sizeof(tio) );
  ::tcflush (m_fd, TCIOFLUSH); // drop all pending data
  tio.c_iflag = IGNBRK | IGNPAR;
  tio.c_oflag = 0;
  tio.c_cflag = CREAD | CRTSCTS | CS8 | HUPCL | CLOCAL;
  tio.c_lflag = IEXTEN;
  tio.c_cc[VTIME] = 0;
  tio.c_cc[VMIN] = 1;
  ::cfsetispeed (&tio, B115200);
  ::cfsetospeed (&tio, B115200);
  ::tcsetattr (m_fd, TCSANOW, &tio);
}

avaspec::serial::~serial ()
{
  startfunc;
  ::close (m_fd);
}

void avaspec::serial::write_message (std::string const &message)
{
  startfunc;
  std::string data;
  // duplicate all 0x10's in the message
  std::string::size_type done = 0, found;
  while ( (found = message.find (0x10, done) ) != std::string::npos) 
    {
      data += message.substr (done, found - done);
      data += std::string ("\020\020", 2);
      done = found + 1;
    }
  data += message.substr (done);
  // add header and footer
  unsigned len = data.size ();
  data = std::string ("\020\002\000\000\000\000", 6) + data
    + std::string ("\020\003", 2);
  data[2] = m_id++ & 0xff;
  data[3] = 0; // node number, not used
  data[4] = len & 0xff;
  data[5] = (len >> 8) & 0xff;
  if (len >> 16)
    {
      shevek_error ("message too long (" << len << " >= 1 << 16)");
      return;
    }
  // write it to serial port
  done = 0;
  while (done < data.size () )
    {
      int l = ::write (m_fd, data.data () + done, data.size () - done);
      if (l < 0)
	{
	  if (errno == -EINTR) continue;
	  shevek_error_errno ("error writing to serial device");
	  return;
	}
      done += l;
    }
}

std::string avaspec::serial::read_message (unsigned timeout, unsigned, char)
{
  startfunc;
  std::string message;
  while (true)
    {
      dbg ("reading message");
      message = l_read_message (timeout);
      if (unsigned (message[0] & 0xff) == ( (m_id - 1) & 0xff) ) break;
      dbg ("invalid id, trying to read next message");
    }
  unsigned len = (message[2] & 0xff) + ( (message[3] & 0xff) << 8);
  if (len == 0) // error message
    {
      if (message.size () != 5)
	shevek_error ("invalid length for error message (" << message.size ()
		      << " != 5)");
      else
        shevek_error ("error received from device: "
		      << unsigned (message[4] & 0xff) );
      return std::string ();
    }
  if (len != message.size () - 4)
    {
      shevek_error ("incorrect message length (" << len << " != "
		    << (message.size () - 4) << ")");
      return std::string ();
    }
  return message.substr (4);
}

std::string avaspec::serial::l_read_message (unsigned timeout)
{
  startfunc;
  // read the data
  while (true)
    {
      struct pollfd pfd;
      pfd.fd = m_fd;
      pfd.events = POLLIN;
      int t;
      while (1 != (t = ::poll (&pfd, 1, timeout) ) )
	{
	  if (t == 0)
	    {
	      shevek_error ("timeout on serial device");
	      return std::string ();
	    }
	  if (errno != -EINTR)
	    {
	      shevek_error_errno ("poll returned error");
	      return std::string ();
	    }
	  // FIXME: timeout should be updated
	}
      if (!(pfd.revents & POLLIN) )
	{
	  shevek_error ("error on socket");
	  return std::string ();
	}
      int l = ::read (m_fd, &m_buffer[m_buffer_size],
		      BUFFERSIZE - m_buffer_size);
      if (l <= 0)
	{
	  if (errno == -EINTR) continue;
	  shevek_error ("read error");
	  return std::string ();
	}
      m_buffer_size += l;
      while (true)
	{
	  char *p = reinterpret_cast <char *> (memchr (m_buffer, 0x10,
						       m_buffer_size) );
	  if (p != m_buffer)
	    {
	      if (p == 0)
		{
		  m_buffer_size = 0;
		  dbg ("no 0x10 in buffer");
		  break; // read more
		}
	      memmove (m_buffer, p, m_buffer_size - (p - m_buffer) );
	      m_buffer_size -= p - m_buffer;
	    }
	  if (m_buffer_size < 2) break; // read more
	  if (m_buffer[1] != 0x02) // this is not a message head after all
	    {
	      m_buffer[0] = 0;
	      dbg ("not a message head, retrying");
	      continue; // retry parsing
	    }
	  std::string data;
	  for (unsigned i = 2; i < m_buffer_size; ++i)
	    {
	      if (m_buffer[i] == 0x10)
		{
		  if (i + 1 == m_buffer_size) break; // read more
		  switch (m_buffer[i + 1])
		    {
		    case 0x10: // escaped 0x10, insert only one in data
		      data += 0x10;
		      ++i;
		      break;
		    case 0x03: // end of message
		      if (i + 1 != m_buffer_size)
			memmove (m_buffer, &m_buffer[i + 2],
				 m_buffer_size - (i + 1) );
		      m_buffer_size -= i + 1;
		      return data;
		    default: // weird message
		      m_buffer[0] = 0; // break header, look for next one
		      continue;
		    }
		}
	      else
		{
		  data += m_buffer[i];
		}
	    }
	  // this exit means the message did not yet finish, but we saw the
	  // whole buffer: read more
	  break;
	}
    }
}

avaspec::emulation::emulation ()
{
  startfunc;
}

avaspec::emulation::~emulation ()
{
  startfunc;
}

void avaspec::emulation::write_message (std::string const &message)
{
  startfunc;
}

std::string avaspec::emulation::read_message (unsigned timeout,
					      unsigned replysize, char reply)
{
  startfunc;
  std::string result;
  if ( (reply & 0xff) == 0x81)
    {
      if (replysize != 327)
        {
	  shevek_error ("unable to return status: "
			"incorrect reply length (bug)");
	  return std::string ();
	}
      result = std::string ("\201" // reply
			    "emulation device                "
			    "                                " // 0x40B version
			    "\000\000" //0x02B device id
			    "\002" //0x01B number of channels in this device
			    "\000\010" //0x02B number of pixels per channel
			    "\000" //0x01B sensor (?)
			    , 0x47);
      std::string channel ("\001\000\000\000" // calibration
			   "\000\000\000\000" // calibration
			   "\000\000\000\000" // calibration
			   "\000\000\000\000" // calibration
			   "\000\000\000\000" // calibration
			   "\000\000\000\000" // calibration (gain)
			   "\000\000\000\000" // calibration (offset)
			   "\000\000" //0x02 bytes start pixel
			   "\361\007" //0x02 bytes stop pixel+1(incl. endpoint)
			   , 0x20);
      for (unsigned i = 0; i < 8; ++i)
	result += channel;
    }
  else if (replysize == 0)
    {
      result.resize (0x800 * 2 + 6);
      result[0] = reply;
      result[2] = 0;
      result[3] = 0;
      result[4] = 0xf1;
      result[5] = 0x07;
    }
  else
    {
      result.resize (replysize);
      result[0] = reply;
    }
  return result;
}

void avaspec::channel::new_data (std::string const &message)
{
  startfunc;
  m_mindata = (message[2] & 0xff) + ( (message[3] & 0xff) << 8);
  m_maxdata = (message[4] & 0xff) + ( (message[5] & 0xff) << 8) + 1;
  if (m_mindata != m_min || m_maxdata != m_max)
    {
      shevek_error ("consistency check failed for data range: min = " << m_min
		    << ", mindata = " << m_mindata << ", max = " << m_max
		    << ", maxdata = " << m_maxdata);
      return;
    }
  unsigned offset = 6 + m_parent->m_extra_pixels;
  for (unsigned i = m_mindata; i < m_maxdata; ++i)
    {
      int value = (message[offset + 2 * (i - m_mindata)] & 0xff)
	+ ( (message[offset + 2 * (i - m_mindata) + 1] & 0xff) << 8);
      // Contrary to the documentation, this number should be divided by 4.
      // A consistancy check is added, remove the division if it fails.
      if ( (value & 3) != 0)
        {
	  shevek_error ("Raw data is not a multiple of 4.  Change the source.");
	  return;
	}
      value >>= 2;
      m_data[i] = value;
    }
  offset = 6;
  for (unsigned i = 0; i < m_parent->m_extra_pixels; ++i)
    {
      int value = (message[offset + 2 * i] & 0xff)
	+ ( (message[offset + 2 * i + 1] & 0xff) << 8);
      // Contrary to the documentation, this number should be divided by 4.
      // A consistancy check is added, remove the division if it fails.
      if ( (value & 3) != 0)
        {
	  shevek_error ("Raw data is not a multiple of 4.  Change the source.");
	  return;
	}
      value >>= 2;
      m_extra[i] = value;
    }
}

void avaspec::channel::setup (avaspec *parent, unsigned id,
			      std::vector <float> const &ijkvector,
			      shevek::relative_time ijktime,
			      std::vector <float> const &nonlinear)
{
  startfunc;
  m_parent = parent;
  m_mindata = 1;
  m_maxdata = 1;
  m_data.resize (m_parent->m_numpixels);
  m_extra.resize (m_parent->m_extra_pixels);
  m_id = id;
  m_ijk = ijkvector;
  m_ijktime = ijktime;
  m_nonlinear = nonlinear;
  set_range (m_parent->m_eeprom.channel[id].start,
	     m_parent->m_eeprom.channel[id].stop);
}

void avaspec::channel::set_range (unsigned min, unsigned max)
{
  startfunc;
  // if there are 2048 pixels, "dark-correction-data" is sent with the
  // measurement.  This is 14 pixels, therefore you cannot read further
  // than 2034.
  if (min >= m_parent->m_numpixels || max == 0 || max > m_parent->m_numpixels)
    {
      shevek_error ("invalid range: [" << min << ", " << max << ")");
      return;
    }
  std::string message = std::string ("\010\000\000\000\000\000", 6);
  message[1] = m_id;
  message[2] = min & 0xff;
  message[3] = (min >> 8) & 0xff;
  message[4] = (max - 1) & 0xff;
  message[5] = ( (max - 1) >> 8) & 0xff;
  m_parent->l_readwrite (message, 0x88, 1);
  m_min = min;
  m_max = max;
  m_parent->set_start (m_id, min);
  m_parent->set_stop (m_id, max);
}

unsigned avaspec::channel::get_range_min () const
{
  startfunc;
  return m_min;
}

unsigned avaspec::channel::get_range_max () const
{
  startfunc;
  return m_max;
}

unsigned avaspec::channel::operator[] (unsigned idx) const
{
  startfunc;
  if (idx < m_mindata || idx >= m_maxdata)
    {
      shevek_error ("index out of range");
      return unsigned ();
    }
  return m_data[idx];
}

unsigned avaspec::channel::extra (unsigned idx) const
{
  startfunc;
  if (idx >= m_parent->m_extra_pixels)
    {
      shevek_error ("extra index out of range");
      return unsigned ();
    }
  return m_extra[idx];
}

std::vector <float> const &avaspec::channel::nonlinear () const
{
	return m_nonlinear;
}

std::vector <float> const &avaspec::channel::ijking () const
{
	return m_ijk;
}

shevek::relative_time avaspec::channel::ijktime () const
{
	return m_ijktime;
}

shevek::absolute_time avaspec::time () const
{
  startfunc;
  return m_time;
}
