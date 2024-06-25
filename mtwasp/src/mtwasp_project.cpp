/*
	Copyright (C) 2024 Mark Tyler

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 3 of the License, or
	(at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program in the file COPYING.
*/

#include <mtdatawell_math.h>

#include "mtwasp.h"



mtWasp::Project::Project ()
{
	SDL_zero ( m_wave_audiospec );

	m_wave_audiospec.freq = WAVE_OUTPUT_HZ;
	m_wave_audiospec.format = AUDIO_S16SYS;
	m_wave_audiospec.channels = WAVE_OUTPUT_CHANNELS;
	m_wave_audiospec.samples = WAVE_OUTPUT_SAMPLES;
	m_wave_audiospec.callback = nullptr;

	m_audio_device_ready = 0;
	m_audio_device = AUDIO_DEVICE_DEFAULT;

	new_file ();
}

mtWasp::Project::~Project ()
{
	set_wave_buf ( nullptr, 0 );
}

void mtWasp::Project::set_wave_buf (
	short	* const	buf,
	size_t	const	samples
	)
{
	free ( m_wave_buf );
	m_wave_buf = buf;
	m_wave_buf_samples = samples;
}

std::string mtWasp::Project::get_titlebar_text () const
{
	size_t	const	fullname_sep = m_filename.rfind ( MTKIT_DIR_SEP );
	std::string	txt;

	if ( m_filename.size() > 0 )
	{
		if ( fullname_sep == std::string::npos )
		{
			txt += m_filename;
		}
		else
		{
			txt += std::string ( m_filename, fullname_sep + 1 );
		}
	}
	else
	{
		txt += "Untitled";
	}

	if ( has_file_changed () )
	{
		txt += " (Modified)";
	}

	if ( fullname_sep != std::string::npos )
	{
		txt += " - ";
		txt += std::string ( m_filename, 0, fullname_sep );
	}

	txt += "    ";
	txt += VERSION;

	return txt;
}

int mtWasp::Project::get_wave_function_value (
	double	const	x,
	double		&result
	)
{
	auto & vars = m_parser.variables ();

	vars[ "x" ] = x / 180.0 * M_PI;

	char const * txt = m_wave_function.c_str();
	int const err = m_parser.evaluate ( txt );

	if ( err )
	{
		result = m_parser.error_pos ();
		return 1;
	}

	result = m_parser.result ().get_num ();

	return 0;
}

int mtWasp::Project::new_file ()
{
	m_changes_not_saved = 0;
	m_changes_not_rebuilt = 1;
	m_filename.clear();
	m_wave_function = "sin(x)";
	m_wave_duration = WAVE_DURATION_DEFAULT();
	m_wave_octave = WAVE_OCTAVE_DEFAULT;
	m_wave_volume_p1x = WAVE_VOLUME_SINGLE_STEP ();
	m_wave_volume_p1y = WAVE_VOLUME_Y_MAX ();
	m_wave_volume_p2x = WAVE_VOLUME_X_MAX () - WAVE_VOLUME_SINGLE_STEP ();
	m_wave_volume_p2y = WAVE_VOLUME_Y_MAX ();

	return 0;
}



/// ----------------------------------------------------------------------------



#define ELEMENT_NAME_MTWASP		"mtWasp"
#define ELEMENT_NAME_WAVE		"Wave"
#define ATTRIBUTE_NAME_VERSION		"version"
#define ATTRIBUTE_NAME_OCTAVE		"octave"
#define ATTRIBUTE_NAME_DURATION		"duration"
#define ATTRIBUTE_NAME_VOLUME_P1X	"volume_p1x"
#define ATTRIBUTE_NAME_VOLUME_P1Y	"volume_p1y"
#define ATTRIBUTE_NAME_VOLUME_P2X	"volume_p2x"
#define ATTRIBUTE_NAME_VOLUME_P2Y	"volume_p2y"
#define ATTRIBUTE_NAME_FUNCTION		"function"



int mtWasp::Project::load_file ( char const * const filename )
{
	if ( ! filename )
	{
		std::cerr << "mtWasp::Project::load_file ERROR: "
			"filename is NULL\n";
		return 1;
	}

	mtKit::Utree file;
	mtUtreeNode * const root = file.load ( filename );

	if ( ! root )
	{
		std::cerr << "mtWasp::Project::load_file ERROR: "
			"unable to load file '" << filename << "'\n";
		return 1;
	}

	mtUtreeNode * node;
	int version, octave;
	double duration, p1x, p1y, p2x, p2y;
	char const * function;

	if (	!( node = mtkit_utree_get_node ( root, ELEMENT_NAME_MTWASP, 0 ))
		|| mtkit_utree_get_attribute_int ( node, ATTRIBUTE_NAME_VERSION,
			&version )
		|| !( node = mtkit_utree_get_node ( node, ELEMENT_NAME_WAVE, 0))
		|| mtkit_utree_get_attribute_int ( node, ATTRIBUTE_NAME_OCTAVE,
			&octave )
		|| mtkit_utree_get_attribute_double ( node,
			ATTRIBUTE_NAME_DURATION, &duration )
		|| mtkit_utree_get_attribute_double ( node,
			ATTRIBUTE_NAME_VOLUME_P1X, &p1x )
		|| mtkit_utree_get_attribute_double ( node,
			ATTRIBUTE_NAME_VOLUME_P1Y, &p1y )
		|| mtkit_utree_get_attribute_double ( node,
			ATTRIBUTE_NAME_VOLUME_P2X, &p2x )
		|| mtkit_utree_get_attribute_double ( node,
			ATTRIBUTE_NAME_VOLUME_P2Y, &p2y )
		|| mtkit_utree_get_attribute_string ( node,
			ATTRIBUTE_NAME_FUNCTION, &function )
		)
	{
		std::cerr << "mtWasp::Project::load_file ERROR: "
			"unable to load Utree data from file\n";
		return 1;
	}

	m_wave_function = function;
	m_wave_duration = duration;
	m_wave_octave = octave;
	m_wave_volume_p1x = p1x;
	m_wave_volume_p1y = p1y;
	m_wave_volume_p2x = p2x;
	m_wave_volume_p2y = p2y;

	m_changes_not_saved = 0;
	m_changes_not_rebuilt = 1;
	m_filename = filename;

	return 0;
}

int mtWasp::Project::save_file ( char const * const filename )
{
	if ( ! filename )
	{
		std::cerr << "mtWasp::Project::save_file ERROR: "
			"filename is NULL\n";
		return 1;
	}

	mtKit::Utree file;
	mtUtreeNode * root, * node;

	if (	!( root = file.init() )
		|| !( node = mtkit_utree_new_element ( root, ELEMENT_NAME_MTWASP ) )
		|| mtkit_utree_set_attribute_int ( node, ATTRIBUTE_NAME_VERSION,
			1 )
		|| !( node = mtkit_utree_new_element ( node, ELEMENT_NAME_WAVE ) )
		|| mtkit_utree_set_attribute_int ( node, ATTRIBUTE_NAME_OCTAVE,
			m_wave_octave )
		|| mtkit_utree_set_attribute_double ( node,
			ATTRIBUTE_NAME_DURATION, m_wave_duration )
		|| mtkit_utree_set_attribute_double ( node,
			ATTRIBUTE_NAME_VOLUME_P1X, m_wave_volume_p1x )
		|| mtkit_utree_set_attribute_double ( node,
			ATTRIBUTE_NAME_VOLUME_P1Y, m_wave_volume_p1y )
		|| mtkit_utree_set_attribute_double ( node,
			ATTRIBUTE_NAME_VOLUME_P2X, m_wave_volume_p2x )
		|| mtkit_utree_set_attribute_double ( node,
			ATTRIBUTE_NAME_VOLUME_P2Y, m_wave_volume_p2y )
		|| mtkit_utree_set_attribute_string ( node,
			ATTRIBUTE_NAME_FUNCTION, m_wave_function.c_str() )
		)
	{
		std::cerr << "mtWasp::Project::save_file ERROR: "
			"unable to populate Utree\n";
		return 1;
	}

	if ( file.save ( filename ) )
	{
		std::cerr << "mtWasp::Project::save_file ERROR: "
			"unable to save file '" << filename << "'\n";
		return 1;
	}

	m_changes_not_saved = 0;
	m_filename = filename;

	return 0;
}



int mtWasp::Project::export_wave_file ( char const * const filename )
{
	if ( ! filename )
	{
		std::cerr << "mtWasp::Project::export_wave_file ERROR: "
			"filename is NULL\n";
		return 1;
	}

	short const * buf = nullptr;
	size_t samples;

	if ( get_wave_buffer ( buf, samples ) )
	{
		std::cerr << "mtWasp::Project::export_wave_file ERROR: "
			"Unable to build wave\n";
		return 1;
	}

	mtGin::AudioFileWrite	audio_file;

	if ( audio_file.open ( filename, m_wave_audiospec ) )
	{
		std::cerr << "mtWasp::Project::export_wave_file ERROR: "
			"Unable to open file: '"
			<< filename
			<< "'\n";
		return 1;
	}

	if ( audio_file.write ( buf, samples ) )
	{
		std::cerr << "mtWasp::Project::export_wave_file ERROR: "
			"Unable to write data to file\n";
		return 1;
	}

	return 0;
}

int mtWasp::Project::set_wave_function ( char const * const function )
{
	if ( ! function )
	{
		return 1;
	}

	m_wave_function = function;

	settings_have_changed ();

	return 0;
}

int mtWasp::Project::set_wave_duration_seconds ( double const secs )
{
	if ( secs < WAVE_DURATION_MIN()
		|| secs > WAVE_DURATION_MAX()
		)
	{
		return 1;
	}

	m_wave_duration = secs;

	settings_have_changed ();

	return 0;
}

int mtWasp::Project::set_wave_octave ( int const octave )
{
	if ( octave < WAVE_OCTAVE_MIN
		|| octave > WAVE_OCTAVE_MAX
		)
	{
		return 1;
	}

	m_wave_octave = octave;

	settings_have_changed ();

	return 0;
}

int mtWasp::Project::set_wave_volume_points (
	double	const	p1x,
	double	const	p1y,
	double	const	p2x,
	double	const	p2y
	)
{
	if (	p1x < WAVE_VOLUME_X_MIN ()	|| p1x > WAVE_VOLUME_X_MAX ()
		|| p1y < WAVE_VOLUME_Y_MIN ()	|| p1y > WAVE_VOLUME_Y_MAX ()
		|| p2x < WAVE_VOLUME_X_MIN ()	|| p2x > WAVE_VOLUME_X_MAX ()
		|| p2y < WAVE_VOLUME_Y_MIN ()	|| p2y > WAVE_VOLUME_Y_MAX ()
		|| p1x > p2x
		)
	{
		return 1;
	}

	m_wave_volume_p1x = p1x;
	m_wave_volume_p1y = p1y;
	m_wave_volume_p2x = p2x;
	m_wave_volume_p2y = p2y;

	settings_have_changed ();

	return 0;
}

int mtWasp::Project::build_wave ()
{
	if ( ! m_changes_not_rebuilt )
	{
		// Nothing to do as settings haven't changed since last rebuild
		return 0;
	}

	// 1 + Hz * secs:
	size_t const samples = (size_t)(1 + WAVE_OUTPUT_HZ * m_wave_duration);

	// Don't bother allocating if we already have the right sized buffer
	if ( samples != m_wave_buf_samples )
	{
		auto buf = static_cast<short *>(malloc(samples*sizeof(short)));

		if ( ! buf )
		{
			std::cerr << "mtWasp::Project::build_wave () ERROR : "
				"Unable to malloc.\n";
			return 1;
		}

		set_wave_buf ( buf, samples );
	}

	for ( size_t i = 0; i < samples; i++ )
	{
		double const hz = get_wave_function_hz ();
		double const xp = (double)i / ((double)samples - 1.0);	// X %
		double const secs_elapsed = xp * get_wave_duration_seconds ();
		double const cycles_elapsed = secs_elapsed * hz;
		double const x = 360.0 * fmod ( cycles_elapsed, 1.0 );

		double y;
		short res = 0;

		if ( get_wave_function_value ( x, y ) )
		{
			// Function failed, but no error needs to be reported,
			// just clear the buffer to create silence.
			memset( m_wave_buf, 0, samples * sizeof(m_wave_buf[0]));
			break;
		}
		else
		{
			double const volume = get_wave_volume_at ( xp );
			double const dres = mtkit_double_bound (
				y * volume * SHRT_MAX,
				SHRT_MIN, SHRT_MAX );

			res = (short)dres;
		}

		m_wave_buf[i] = res;
	}

	m_changes_not_rebuilt = 0;

	return 0;
}

double mtWasp::Project::get_wave_volume_at ( double const x ) const
{
	if ( x <= 0.0 || x >= 1.0 )
	{
		return 0.0;
	}

	if ( x <= m_wave_volume_p1x )
	{
		double p = (m_wave_volume_p1x - x) / m_wave_volume_p1x;
		return (1.0 - p) * m_wave_volume_p1y;
	}

	if ( x <= m_wave_volume_p2x )
	{
		double p = (m_wave_volume_p2x - x) /
			(m_wave_volume_p2x - m_wave_volume_p1x);
		return p * m_wave_volume_p1y + (1.0 - p) *
			m_wave_volume_p2y;
	}

	double p = (1.0 - x) / (1.0 - m_wave_volume_p2x);
	return p * m_wave_volume_p2y;
}

int mtWasp::Project::init_audio_device ()
{
	if ( m_audio_device_ready )
	{
		// Device already initialized
		return 0;
	}

	if ( m_audio_player.open_device ( m_audio_device, m_wave_audiospec ) )
	{
		std::cerr << "mtWasp::init_audio_device ERROR - Unable to "
			"open audio device.\n";
		return 1;
	}

	m_audio_device_ready = 1;

	return 0;
}

int mtWasp::Project::play_wave_audio ()
{
	if ( init_audio_device () )
	{
		return 1;
	}

	short const * buf;
	size_t samples;

	if ( get_wave_buffer ( buf, samples ) )
	{
		return 1;
	}

	m_audio_player.pause ();
	m_audio_player.queue_flush ();

	if ( m_audio_player.queue_data ( buf, samples ) )
	{
		std::cerr << "mtWasp::Project::play_wave_audio ERROR: "
			"Unable to queue data.\n";
		return 1;
	}

	if ( m_audio_player.resume () )
	{
		std::cerr << "mtWasp::Project::play_wave_audio ERROR: "
			"Unable to resume playing audio.\n";
		return 1;
	}

	return 0;
}

int mtWasp::Project::stop_wave_audio ()
{
	return m_audio_player.pause ();
}

