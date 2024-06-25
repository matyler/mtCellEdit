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

#ifndef WASP_QT_H_
#define WASP_QT_H_



#include "static.h"
#include "be.h"



#ifdef U_TK_QT5
	#include <mtqex5.h>
#endif

#ifdef U_TK_QT6
	#include <mtqex6.h>
#endif



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



class DialogAudioDevicePicker;
class MainWindow;
class WaveFunctionView;
class WaveVolumeView;



class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow ( CommandLine & cline );
	~MainWindow ();

/// UPDATES

	void update_titlebar ();
	void update_recent_files ();
	void update_statusbar ();
	void update_inputs ();

private slots:
	void press_file_new ();
	void press_file_open ();
	void press_file_save ();
	void press_file_save_as ();
	void press_file_export_wave ();
	void press_file_recent ( size_t i );
	void press_file_quit ();

	void press_edit_prefs ();

	void press_audio_play ();
	void press_audio_stop ();
	void press_audio_set_audio_device ();

	void press_help_about_qt ();
	void press_help_about ();

	void changed_spin_wave_duration ( double seconds );
	void changed_spin_wave_octave ( int octave );
	void changed_text_wave_function ();

protected:
	void closeEvent ( QCloseEvent * ev );

private:
	void create_menu ();
	void create_statusbar ();
	void create_prefs_callbacks ();

	int project_new ();
	int project_load ( char const * filename );
	int project_save ( char const * filename );
	int project_export_wave ( char const * filename );
	int ok_to_lose_changes ();	// 0=No 1=Yes

	void store_window_geometry ();

/// ----------------------------------------------------------------------------

/// Low level private references

	mtKit::UserPrefs	& m_uprefs;
	MemPrefs	const	& m_mprefs;
	mtKit::RecentFile	& m_recent_files;

/// Low level private members

	// Create before anything else
	mtWasp::Project	m_project;

	// Create SDL app last, and destroy it first
	mtGin::AudioPlay m_audio_player;
	mtGin::App	m_gin;

/// Menus
	QAction		* act_file_recent_separator;
	QAction		* act_file_recent [ RECENT_MENU_TOTAL ];

/// Control

	QDoubleSpinBox	* m_spin_wave_duration;
	QSpinBox	* m_spin_wave_octave;

/// Volume envelope

	WaveVolumeView	* m_volume_view;

/// Waveform

	QTextEdit	* m_text_wave_function;
	WaveFunctionView * m_wave_function_view;

/// Status bar

	QLabel		* m_statusbar_left;
	QLabel		* m_statusbar_middle_left;
	QLabel		* m_statusbar_middle_right;
	QLabel		* m_statusbar_right;

	MTKIT_RULE_OF_FIVE( MainWindow )
};



class WaveFunctionView : public QWidget
{
	Q_OBJECT

public:
	explicit WaveFunctionView ( mtWasp::Project &project );

private:
	void paintEvent ( QPaintEvent * ev );

/// ----------------------------------------------------------------------------

	mtWasp::Project	&m_project;

	QColor	const	m_color_background;
	QColor	const	m_color_axis;
	QColor	const	m_color_lines;
};



class WaveVolumeView : public QWidget
{
	Q_OBJECT

public:
	WaveVolumeView (
		MainWindow	&mw,
		mtWasp::Project	&project
		);

	QDoubleSpinBox * get_spin_p1x () { return m_spin_p1x; };
	QDoubleSpinBox * get_spin_p1y () { return m_spin_p1y; };
	QDoubleSpinBox * get_spin_p2x () { return m_spin_p2x; };
	QDoubleSpinBox * get_spin_p2y () { return m_spin_p2y; };

	void load_values_from_project ();

private slots:
	void changed_spin_volume ( double x );

private:
	void paintEvent ( QPaintEvent * ev );
	void mousePressEvent ( QMouseEvent * ev );
	void mouseMoveEvent ( QMouseEvent * ev );
	void mouseReleaseEvent ( QMouseEvent * ev );

	void set_volume_spin_values (
		double p1x,
		double p1y,
		double p2x,
		double p2y
		);

/// ----------------------------------------------------------------------------

	MainWindow	&m_mainwindow;
	mtWasp::Project	&m_project;

	QDoubleSpinBox	* m_spin_p1x, * m_spin_p1y;
	QDoubleSpinBox	* m_spin_p2x, * m_spin_p2y;

	QColor	const	m_color_background;
	QColor	const	m_color_axis;
	QColor	const	m_color_area;
	QColor	const	m_color_nodes;

	int		m_drag_node = 0;	// 1 or 2 during a drag
};



class DialogAudioDevicePicker : public QDialog
{
	Q_OBJECT

public:
	DialogAudioDevicePicker (
		MainWindow		&mw,
		mtKit::UserPrefs	& uprefs,
		MemPrefs	const	& mprefs
		);

private slots:
	void press_ok ();

private:
	QTableWidget		* m_table_devices;

	MainWindow		&mainwindow;
	mtKit::UserPrefs	& m_uprefs;
};



#endif		// C++ API



#ifndef DEBUG
#pragma GCC visibility pop
#endif



#endif		// WASP_QT_H_

