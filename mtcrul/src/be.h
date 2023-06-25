/*
	Copyright (C) 2020-2022 Mark Tyler

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

#ifndef BE_H
#define BE_H



// C
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// System
#include <mtdatawell_sqlite.h>

// Internal
#include "static.h"



#define PREFS_CRUL_RECENT_DB		"crul.recent.db"
#define PREFS_CRUL_RECENT_DB_TOTAL	10
#define PREFS_CRUL_SPLIT_MAIN		"crul.split.main"

#define PREFS_GL_POINT_RANGE		"gl.point.range"
#define PREFS_GL_POINT_SIZE		"gl.point.size"
#define PREFS_GL_LINE_BUTT_SIZE		"gl.line.butt.size"
#define PREFS_GL_LINE_THICKNESS		"gl.line.thickness"
#define PREFS_GL_LIGHT_CAMERA		"gl.light.camera"
#define PREFS_GL_LIGHT_X		"gl.light.x"
#define PREFS_GL_LIGHT_Y		"gl.light.y"
#define PREFS_GL_LIGHT_Z		"gl.light.z"

#define PREFS_CLOUD_RATE_LOW		"cloud.rate.low"
#define PREFS_CLOUD_RATE_MEDIUM		"cloud.rate.medium"

#define PREFS_VIEW_A			"view.a."
#define PREFS_VIEW_B			"view.b."
#define PREFS_VIEW_SPLIT_ON		"view.split.on"
#define PREFS_VIEW_SPLIT_POS		"view.split.pos"
#define PREFS_VIEW_SPLIT_VERT		"view.split.vert"
#define PREFS_VIEW_NUDGE_SIZE		"view.nudge.size"

#define PREFS_VIEW_SHOW_ANTIALIASING	"view.show.antialiasing"
#define PREFS_VIEW_SHOW_CLOUD		"view.show.cloud"
#define PREFS_VIEW_SHOW_CROSSHAIR	"view.show.crosshair"
#define PREFS_VIEW_SHOW_MODEL		"view.show.model"
#define PREFS_VIEW_SHOW_RULER_PLANE	"view.show.ruler.plane"
#define PREFS_VIEW_SHOW_RULERS		"view.show.rulers"
#define PREFS_VIEW_SHOW_STATUSBAR	"view.show.statusbar"

#define PREFS_CAM_X			"cam.x"
#define PREFS_CAM_Y			"cam.y"
#define PREFS_CAM_Z			"cam.z"
#define PREFS_CAM_XROT			"cam.xrot"
#define PREFS_CAM_YROT			"cam.yrot"
#define PREFS_CAM_ZROT			"cam.zrot"

#define PREFS_WINDOW_MAXIMIZED		"main.window_max"
#define PREFS_WINDOW_X			"main.window_x"
#define PREFS_WINDOW_Y			"main.window_y"
#define PREFS_WINDOW_W			"main.window_w"
#define PREFS_WINDOW_H			"main.window_h"



class Backend;
class MemPrefs;



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



class MemPrefs
{
public:
	MemPrefs () {}

	std::string		crul_split_main;
	double			gl_point_range		= 0.0;
	double			gl_point_size		= 0.0;
	double			gl_line_butt_size	= 0.0;
	double			gl_line_thickness	= 0.0;
	int			gl_light_camera		= 0;
	double			gl_light_x		= 0.0;
	double			gl_light_y		= 0.0;
	double			gl_light_z		= 0.0;

	int			cloud_rate_low		= 0;
	int			cloud_rate_medium	= 0;
	int			view_nudge_size		= 0;
	int			view_split_on		= 0;
	std::string		view_split_pos;
	int			view_split_vert		= 0;

	double			view_a_cam_x		= 0.0;
	double			view_a_cam_y		= 0.0;
	double			view_a_cam_z		= 0.0;
	double			view_a_cam_xrot		= 0.0;
	double			view_a_cam_yrot		= 0.0;
	double			view_a_cam_zrot		= 0.0;

	double			view_b_cam_x		= 0.0;
	double			view_b_cam_y		= 0.0;
	double			view_b_cam_z		= 0.0;
	double			view_b_cam_xrot		= 0.0;
	double			view_b_cam_yrot		= 0.0;
	double			view_b_cam_zrot		= 0.0;

	int			view_show_antialiasing	= 0;
	int			view_show_cloud		= 0;
	int			view_show_crosshair	= 0;
	int			view_show_model		= 0;
	int			view_show_ruler_plane	= 0;
	int			view_show_rulers	= 0;
	int			view_show_statusbar	= 0;

	int			window_x		= 0;
	int			window_y		= 0;
	int			window_w		= 0;
	int			window_h		= 0;
	int			window_maximized	= 0;

	mtKit::UPrefUIEdit	ui_editor;
	mtKit::RecentFile	recent_crul_db;

private:
	MTKIT_RULE_OF_FIVE( MemPrefs )
};



class Backend
{
public:
	Backend ();

	int command_line ( int argc, char const * const * argv );
		// 0 = Continue running (start UI)
		// 1 = Terminate program, returning exit.value()

	void cline_add_filename ( char const * filename );

	inline std::vector<char const *> const & get_cline_files () const
		{ return m_cline_files; }

	void get_data_dir ( std::string & path ) const;

/// ----------------------------------------------------------------------------

	mtKit::Exit		exit;

	// uprefs must be destroyed before mprefs, so it appears below it:
	MemPrefs		mprefs;
	mtKit::UserPrefs	uprefs;

private:
	void prefs_init ();

/// ----------------------------------------------------------------------------

	char		const *	m_db_path		= nullptr;
	char		const *	m_prefs_filename	= nullptr;

	std::vector<char const *> m_cline_files;

	MTKIT_RULE_OF_FIVE( Backend )
};



#endif		// BE_H

