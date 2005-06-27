//#define DEBUG_DBG true

#include <shevek/server.hh>
#include <shevek/debug.hh>
#include <shevek/args.hh>
#include <shevek/sstr.hh>
#include <shevek/mainloop.hh>
#include <sstream>
#include "avaspec.hh"

struct client;

struct serverdata
{
	avaspec *device;
	unsigned continuous;
	void start_read ();
	void end_read ();
	std::string encode (unsigned data);
	shevek::relative_time last_time;
	shevek::absolute_time done_read;
	Glib::RefPtr <shevek::server <client, serverdata> > parent;
	sigc::connection read_handle;
	serverdata () : continuous (0) {}
};

struct client : public shevek::server <client, serverdata>::connection
{
	friend class shevek::server <client, serverdata>;
	friend class shevek::server <client, serverdata>::connection;
	void pickup (bool is_stdio) {}
	inline void read (std::string const &line);
	static Glib::RefPtr <client> create ()
	{ return Glib::RefPtr <client> (new client () ); }
	client () : continuous (false), waiting (false) {}
	~client ();
	bool continuous;
	bool waiting;
	enum { NO, YES, ONCE } view;
};

client::~client ()
{
	if (continuous)
	{
		--get_server ()->data ().continuous;
		continuous = false;
	}
}

std::string serverdata::encode (unsigned data)
{
	unsigned mask = (1 << 5) - 1;
	unsigned offset = ' ';
	std::string ret ("   ", 3);
	ret[0] = (data & mask) + offset;
	ret[1] = ( (data >> 5) & mask) + offset;
	ret[2] = ( (data >> 10) & mask) + offset;
	return ret;
}

void serverdata::start_read ()
{
	if (read_handle.connected () )
		return;
	// use previous time by default, because the default
	// constructor ("now") is much slower than the copy constructor.
	shevek::absolute_time t;
	shevek::relative_time inttime = device->get_integration_time ();
	// change things if the integration time has changed.
	if (inttime != last_time)
	{
		// new measurement: restart timer
		t += inttime;
		last_time = inttime;
	}
	else if (done_read + inttime > t)
	{
		t = done_read + inttime;
	}
	// Start reading out 10 ms before measurement finishes.
	t -= shevek::relative_time (0, 0, 0, 0, 10000000);
	device->start_read ();
	read_handle
		= t.schedule (sigc::mem_fun (*this, &serverdata::end_read) );
}

void serverdata::end_read ()
{
	read_handle.disconnect ();
	device->end_read ();
	done_read = device->time ();
	if (continuous)
		start_read (); // Start the next measurement while we're busy.
	std::ostringstream datastr;
	datastr << "newdata " << done_read << '\n';
	std::string data;
	data = datastr.str ();
	for (unsigned c = 0; c < device->num_channels (); ++c)
	{
		unsigned min = (*device)[c].get_range_min ();
		unsigned max = (*device)[c].get_range_max ();
		if (max <= min)
			continue;
		std::ostringstream head;
		head << "data " << c << ' ' << min << ' ' << max << "\n#";
		data += head.str ();
		unsigned newlinecounter = 0;
		for (unsigned i = min; i < max; ++i)
		{
			data += encode ( (*device)[c][i]);
			++newlinecounter;
			newlinecounter %= 25;
			if (newlinecounter == 0)
				data += std::string ("\n#", 2);
		}
		data += "\n#";
		newlinecounter = 0;
		unsigned numextra = device->extra_pixels ();
		for (unsigned i = 0; i < numextra; ++i)
		{
			data += encode ( (*device)[c].extra (i) );
			++newlinecounter;
			newlinecounter %= 25;
			if (newlinecounter == 0)
				data += std::string ("\n#", 2);
		}
		data += '\n';
	}
	data += "done\n";
	for (shevek::server <client, serverdata>::iterator
			i = parent->begin (); i != parent->end (); ++i)
		if ( (*i)->view != client::NO)
			(*i)->out->write (data);
	for (shevek::server <client, serverdata>::iterator
			i = parent->begin (); i != parent->end (); ++i)
	{
		if ( (*i)->view == client::ONCE)
			(*i)->view = client::NO;
		if ( (*i)->waiting)
		{
			(*i)->waiting = false;
			out->write ("\n");
			(*i)->continue_reading ();
		}
	}
}

void client::read (std::string const &line)
{
	std::string::size_type pos;
	pos = line.find_first_not_of (std::string (" \t\r\n\0", 5) );
	if (pos == std::string::npos || line[pos] == '#')
		return;
	serverdata *data = &get_server ()->data ();
	avaspec *dev = data->device;
	switch (line[pos])
	{
	case 'q': // quit
		disconnect ();
		return;
	case 'i': // get info
		if (line.size () < pos + 2)
		{
			err->write ("E: Info (i) requires an argument\n");
			return;
		}
		switch (line[pos + 1])
		{
		case 'i': // non-changing info
			out->write (sstr (dev->num_channels () << ' '
						<< dev->num_pixels () << ' '
						<< dev->extra_pixels ()
						<< '\n') );
			return;
		case 't': // Timing et al.
			out->write (sstr (dev->get_integration_time () << ' '
						<< dev->get_average ()
						<< '\n') );
			return;
		case 'd': // digital i/o settings
		{
			std::ostringstream s;
			s << dev->get_digital () << ' ';
			for (unsigned i = 0; i < dev->MAX_DIGITAL; ++i)
				s << dev->get_digital (i) << ' ';
			s << dev->get_external_trigger () << ' '
				<< dev->get_fixed_strobe () << ' '
				<< dev->get_strobe () << '\n';
			out->write (s.str () );
			return;
		}
		default:
			unsigned c = line[pos + 1] - '0';
			if (c < 8) // channel info
			{
				std::ostringstream s;
				// Don't send gain and offset: they mess up
				// the calibration.
				s << "5 " << dev->get_start (c) << ' '
					<< dev->get_stop (c);
				for (unsigned i = 0; i < 5; ++i)
					s << ' ' << dev->get_calibration (c, i);
				s << '\n';
				out->write (s.str () );
				return;
			}
			err->write ("E: Unknown argument for info\n");
			return;
		};
	case 't': // set integration time in ms
	{
		int t;
		t = atoi (&line.c_str ()[pos + 1]);
		if (t < 1 || t > 60000)
		{
			err->write ("E: Invalid integration time\n");
			return;
		}
		int s, ns;
		s = t / 1000;
		ns = (t - s * 1000) * 1000000;
		dev->set_integration_time (shevek::relative_time (s, ns) );
		out->write ("\n");
		return;
	}
	case 'a':
	{
		int a;
		a = atoi (&line.c_str ()[pos + 1]);
		if (a < 1 || a > 1000)
		{
			err->write ("E: Invalid number of averages\n");
			return;
		}
		dev->set_average (a);
		out->write ("\n");
		return;
	}
	case 'd':
	{
		int d;
		d = atoi (&line.c_str ()[pos + 1]);
		if (d < 0 || d >= dev->MAX_DIGITAL)
		{
			err->write ("E: Invalid digital pin\n");
			return;
		}
		dev->set_digital (d, false);
		out->write ("\n");
		return;
	}
	case 'D':
	{
		int D;
		D = atoi (&line.c_str ()[pos + 1]);
		if (D < 0 || D >= dev->MAX_DIGITAL)
		{
			err->write ("E: Invalid digital pin\n");
			return;
		}
		dev->set_digital (D, true);
		out->write ("\n");
		return;
	}
	case 'f':
		dev->set_fixed_strobe (false);
		out->write ("\n");
		return;
	case 'F':
		dev->set_fixed_strobe (true);
		out->write ("\n");
		return;
	case 's':
	{
		int s;
		s = atoi (&line.c_str ()[pos + 1]);
		if (s < 0 || s > 6000)
		{
			err->write ("E: Invalid number of flashes\n");
			return;
		}
		dev->set_strobe (s);
		out->write ("\n");
		return;
	}
	case 'w':
		try
		{
			dev->write_eeprom (line.substr (pos + 1) );
		}
		catch (...)
		{
			err->write ("E: Failed to write to eeprom\n");
			return;
		}
		out->write ("\n");
		return;
	case 'r': // read data (do measurement)
		if (view == NO)
			view = ONCE;
		data->start_read ();
		out->write ("\n");
		return;
	case 'c':
		if (!continuous)
		{
			out->write ("\n");
			return;
		}
		continuous = false;
		--data->continuous;
		waiting = true;
		in->unread ();
		return;
	case 'C':
		if (continuous)
		{
			out->write ("\n");
			return;
		}
		continuous = true;
		++data->continuous;
		data->start_read (); // this will do nothing if already reading
		out->write ("\n");
		return;
	case 'v':
		view = NO;
		out->write ("\n");
		return;
	case 'V':
		view = YES;
		out->write ("\n");
		return;
	case 'h':
		waiting = true;
		in->unread ();
		return;
		// TODO: read dark, asynchronous info
	default:
		unsigned c;
		c = line[pos] - '0';
		if (c >= 8)
		{
			err->write ("E: Unknown command\n");
			return;
		}
		if (line.size () < pos + 2)
		{
			err->write ("E: Channel without command\n");
			return;
		}
		switch (line[pos + 1])
		{
		case 'b':
		{
			int b;
			b = atoi (&line.c_str ()[pos + 2]);
			if (b < 0 || unsigned (b) > dev->num_pixels () )
			{
				err->write ("E: Invalid start of range\n");
				return;
			}
			dev->set_start (c, b);
			out->write ("\n");
			return;
		}
		case 'e':
		{
			int e;
			e = atoi (&line.c_str ()[pos + 2]);
			if (e < 1 || unsigned (e) > dev->num_pixels () )
			{
				err->write ("E: Invalid end of range\n");
				return;
			}
			dev->set_stop (c, e);
			out->write ("\n");
			return;
		}
		case 'i':
		{
			std::vector <float> const *v;
			v = &(*dev)[c].nonlinear ();
			out->write (sstr (v->size () << '\n') );
			for (unsigned i = 0; i < v->size (); ++i)
				out->write (sstr ( (*v)[i] << '\n') );
			v = &(*dev)[c].ijking ();
			out->write (sstr ( (*dev)[c].ijktime () << '\n'
						<< v->size () << '\n') );
			for (unsigned i = 0; i < v->size (); ++i)
				out->write (sstr ( (*v)[i] << '\n') );
			return;
		}
		default:
		{
			unsigned n;
			n = atoi (&line.c_str ()[pos + 2]);
			if (n >= 7)
			{
				err->write ("E: Invalid channel command\n");
				return;
			}
			float v;
			v = strtof (&line.c_str ()[pos + 3], 0);
			dev->set_calibration (c, n, v);
			out->write ("\n");
			return;
		}
		}
	}
}

int main (int argc, char **argv)
{
	std::string port ("/tmp/drivers/avaspec");
	std::string config;
	shevek::args::option opts[] = {
		shevek::args::option (0, "config", "configuration file", true,
				config),
		shevek::args::option (0, "port", "port to listen on", true,
				port)
	};
	shevek::args args (argc, argv, opts, 0, 0, "Server for avaspec",
			"2005");
	Glib::RefPtr <shevek::server <client, serverdata> > s;
	s = shevek::server <client, serverdata>::create ();
	s->data ().parent = s;
	s->data ().device = new avaspec (config, 0x471, 0x666, 0);
	s->open (port, 0);
	shevek::loop ();
	return 0;
}
