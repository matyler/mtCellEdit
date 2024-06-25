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

#ifndef MTWASP_H_
#define MTWASP_H_



#include <mtdatawell_math.h>
#include <mtgin_sdl.h>



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifndef DEBUG
#pragma GCC visibility push ( hidden )
#endif



#ifdef __cplusplus
extern "C" {
#endif

// C API




#ifdef __cplusplus
}

// C++ API

namespace mtWasp
{

class Project;



class Project
{
public:
	enum
	{
		// Audiospec
		WAVE_OUTPUT_HZ		= 48000,
		WAVE_OUTPUT_KHZ		= 48,		// Output KHz
		WAVE_OUTPUT_CHANNELS	= 1,		// Mono
		WAVE_OUTPUT_SAMPLES	= 4096,		// For SDL

		// Wave creation model
		WAVE_OCTAVE_MIN		= -4,
		WAVE_OCTAVE_DEFAULT	= 4,		// Middle C
		WAVE_OCTAVE_MAX		= 12,

		AUDIO_DEVICE_DEFAULT	= -1,
	};

	static double WAVE_DURATION_MIN () { return 0.1; }
	static double WAVE_DURATION_DEFAULT () { return 1.0; }
	static double WAVE_DURATION_MAX () { return 100.0; }

	static double WAVE_VOLUME_X_MIN () { return 0.0; }
	static double WAVE_VOLUME_X_MAX () { return 1.0; }
	static double WAVE_VOLUME_Y_MIN () { return 0.0; }
	static double WAVE_VOLUME_Y_MAX () { return 1.0; }
	static double WAVE_VOLUME_SINGLE_STEP () { return 0.1; }

	Project ();
	~Project ();

/// Actions --------------------------------------------------------------------

	int new_file ();
	int load_file ( char const * filename );
	int save_file ( char const * filename );
	int export_wave_file ( char const * filename );

	int play_wave_audio ();
	int stop_wave_audio ();

/// Setters --------------------------------------------------------------------

	int set_wave_function ( char const * function );
		// e.g. "sin(x)" where x = [0.0 - 360.0]
	int set_wave_duration_seconds ( double secs );
	int set_wave_octave ( int octave );

	int set_wave_volume_points (
		double p1x,		// 0.0 -> 1.0
		double p1y,		// 0.0 -> 1.0
		double p2x,		// 0.0 -> 1.0
		double p2y		// 0.0 -> 1.0
		);

	void set_audio_device ( int device )
	{
		m_audio_device = device;
		m_audio_device_ready = 0;
	}

/// Getters --------------------------------------------------------------------

	int has_file_changed () const
	{
		return m_changes_not_saved;
	}

	std::string get_titlebar_text () const;

	std::string const get_filename () const
	{
		return m_filename;
	}

	std::string const & get_wave_function () const
	{
		return m_wave_function;
	}

	int get_wave_function_value (
		double x,		// x = 0.0 -> 360.0
		double &result		// On failure gives text position
		);

	double get_wave_function_hz () const	// Function cycles per second
	{
		return 440.0 / ( ::pow(2.0, (9.0/12.0)) ) *
			( ::pow(2.0, (m_wave_octave - 4)) );
	}

	double get_wave_duration_seconds () const
	{
		return m_wave_duration;
	}

	int get_wave_octave () const
	{
		return m_wave_octave;
	}

	void get_wave_volume_points (
		double &p1x,	// 0.0 -> 1.0
		double &p1y,	// 0.0 -> 1.0
		double &p2x,	// 0.0 -> 1.0
		double &p2y	// 0.0 -> 1.0
		) const
	{
		p1x = m_wave_volume_p1x;
		p1y = m_wave_volume_p1y;
		p2x = m_wave_volume_p2x;
		p2y = m_wave_volume_p2y;
	}

	double get_wave_volume_at ( double const x ) const;
		// Input: 0.0 to 1.0, Output: 0.0 to 1.0

	int get_wave_buffer ( short const *& buf, size_t & samples )
	{
		if ( m_changes_not_rebuilt )
		{
			if ( build_wave () )
			{
				return 1;
			}
		}

		buf = m_wave_buf;
		samples = m_wave_buf_samples;

		return 0;
	}

	SDL_AudioSpec const & get_audiospec () const
	{
		return m_wave_audiospec;
	}

private:
	void set_wave_buf ( short * buf, size_t samples );
	void settings_have_changed ()
	{
		m_changes_not_saved = 1;
		m_changes_not_rebuilt = 1;
	}

	int init_audio_device ();
	int build_wave ();

/// ----------------------------------------------------------------------------

	// File info - set in constructor / new_file()
	int		m_changes_not_saved;
	int		m_changes_not_rebuilt;
	int		m_audio_device_ready;
	int		m_audio_device;
	std::string	m_filename;

	// File data - set in constructor / new_file()
	std::string	m_wave_function;
	double		m_wave_duration;
	int		m_wave_octave;

	double		m_wave_volume_p1x;
	double		m_wave_volume_p1y;
	double		m_wave_volume_p2x;
	double		m_wave_volume_p2y;

	// Parser
	mtDW::DoubleParser m_parser;

	// Wave info & player (SDL)
	SDL_AudioSpec	m_wave_audiospec;
	mtGin::AudioPlay m_audio_player;

	// Wave data - created by build_wave() via get_wave_buffer()
	short		* m_wave_buf = nullptr;
	size_t		m_wave_buf_samples = 0;

	// Raw pointers used
	MTKIT_RULE_OF_FIVE ( Project )
};



} // mtWasp



#endif		// C++ API



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// MTWASP_H_

