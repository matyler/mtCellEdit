/*
	Copyright (C) 2020 Mark Tyler

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

#ifndef QT5_MW_H
#define QT5_MW_H



#include "qt5_cloudview.h"



class DialogLabelEdit;
class Frontend;
class Mainwindow;



class Frontend
{
public:
	Crul::Camera * get_camera ( int id );
	int add_camera ( Crul::Camera & camera );	// Next free id is used

	Crul::Ruler * get_ruler ( int id );
	int add_ruler ( Crul::Ruler & ruler );		// Next free id is used

	void save_db_state ();
	void load_db_state ();

/// ----------------------------------------------------------------------------

	Backend		backend;

	Crul::DB	crul_db;
	Crul::Cloud	cloud;
	Crul::Model	model;

	std::map<int, Crul::Camera> camera_map;
	std::map<int, Crul::Ruler> ruler_map;

	mtGin::GL::Points	cloud_gl;
	RulerQtGL		ruler_gl;
	mtGin::GL::Triangles	model_gl;
};



class Mainwindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit Mainwindow ( Frontend & fe );
	~Mainwindow ();

/// ----------------------------------------------------------------------------

private slots:

///	MENUS

	void press_file_open_db ();
	void press_file_recent ( size_t i );
	void press_file_quit ();

	void press_edit_mode_camera ();
	void press_edit_mode_ruler ();
	void press_edit_import_pts ();
	void press_edit_import_model ();
	void press_edit_prefs ();

	void press_view_select_a ();
	void press_view_select_b ();
	void press_view_show_antialiasing ();
	void press_view_show_crosshair ();
	void press_view_show_statusbar ();
	void press_view_show_rulers ();
	void press_view_show_ruler_plane ();
	void press_view_show_cloud ();
	void press_view_show_model ();
	void press_view_split ();
	void press_view_split_switch ();
	void press_view_reset_camera ();
	void press_view_spin_180 ();
	void press_view_xyz_snap_nudge ();
	void press_view_clone_ab ();
	void press_view_clone_ba ();
	void press_view_nudge_up ();
	void press_view_nudge_down ();
	void press_view_res_low ();
	void press_view_res_medium ();
	void press_view_res_high ();

	void press_ruler_swap_ab ();
	void press_ruler_deselect ();

	void press_help_about_qt ();
	void press_help_about ();

///	BUTTONS etc

	void press_camera_new ();
	void press_camera_delete ();
	void press_camera_edit ();
	void press_camera_a_xyz ();
	void press_camera_b_xyz ();
	void press_camera_list_row (
		int row,
		int column,
		int old_row,
		int old_column
		);
	void press_camera_list_item_changed ( QTableWidgetItem * item );

	void press_ruler_rgb_set ();
	void press_ruler_new ();
	void press_ruler_delete ();
	void press_ruler_copy ();
	void press_ruler_edit ();
	void press_ruler_hide_all ();
	void press_ruler_list_row (
		int row,
		int column,
		int old_row,
		int old_column
		);
	void press_ruler_list_item_changed ( QTableWidgetItem * item );
	void change_ruler_plane ( int i );
	void keypress_ruler ( CloudView * view, QKeyEvent * event );
	void mouse_ruler (
		CloudView * view,
		QMouseEvent * event,
		int mx,
		int my
		);

	// Used for accurate sub-integer changes (updates slider by blocking)
	void set_xrotation_a ( double angle );
	void set_zrotation_a ( double angle );
	void set_xrotation_b ( double angle );
	void set_zrotation_b ( double angle );

	void update_view_a ();
	void update_view_b ();
	void update_gl_view ();
	void update_ruler ();

	void set_nudge ( int i );
	void set_point_size ( int i );
	void set_line_butt_size ( int i );
	void set_line_thickness ( int i );

private:
	void create_menu ();
	void create_main_ui ();

	int get_selected_resolution () const;

	void init_db_success ( int fail );
	int database_load ( std::string const & path );
	void load_cloud_from_db ( int type = Crul::DB::CACHE_TYPE_ERROR );
	void load_model_from_db ();

	void set_edit_mode ( int mode );

	void set_view_split_on ( bool on );
	void set_view_split_vert ( bool vert );
	void set_view_resolution ( int type );

	void update_recent_db_menu ();
	void update_info ();
	void update_extents_clear ();
	void update_view_show_items ();

	void populate_camera_list ();
	void enable_camera_buttons ( Crul::Camera const * cam );

	void populate_gl_rulers ();
	void populate_ruler_list ();
	void update_ruler_info ();
	void clear_ruler_info ();
	void enable_ruler_buttons ( Crul::Ruler const * rul );

	Crul::Ruler * get_active_ruler ();	// 0 = Nothing is selected
	Crul::Camera * get_active_camera ();	// 0 = Nothing is selected

	CloudView * get_active_view ();

	void set_slider_angle ( QSlider * slider, double angle );

/// ----------------------------------------------------------------------------

	QMenuBar	* m_menu_bar;

	QSplitter	* m_split_main;
	QSplitter	* m_split_view;

	CloudView	* m_cloud_view_a;
	CloudView	* m_cloud_view_b;

	QMenu		* m_menu_ruler;

	QAction		* act_db_recent [ PREFS_CRUL_RECENT_DB_TOTAL ];

	QAction		* act_edit_import_pts;
	QAction		* act_edit_import_model;

	QAction		* act_view_show_antialiasing;
	QAction		* act_view_show_crosshair;
	QAction		* act_view_show_statusbar;
	QAction		* act_view_show_rulers;
	QAction		* act_view_show_ruler_plane;
	QAction		* act_view_show_cloud;
	QAction		* act_view_show_model;
	QAction		* act_view_split;
	QAction		* act_view_split_switch;
	QAction		* act_view_res_low;
	QAction		* act_view_res_medium;
	QAction		* act_view_res_high;

	QTabWidget	* m_tabs_mode;
	QWidget		* m_tab_camera;
	QWidget		* m_tab_ruler;

	QSlider		* m_slider_ax;
	QSlider		* m_slider_az;
	QSlider		* m_slider_bx;
	QSlider		* m_slider_bz;
	QSlider		* m_slider_nudge;
	QSlider		* m_slider_pt_size;
	QSlider		* m_slider_line_butt_size;
	QSlider		* m_slider_line_thickness;

	QLabel		* m_label_nudge;
	QLabel		* m_label_pt_size;
	QLabel		* m_label_line_butt_size;
	QLabel		* m_label_line_thickness;

	QLabel		* m_label_rul_axyz;
	QLabel		* m_label_rul_bxyz;
	QLabel		* m_label_rul_length;
	QLabel		* m_label_rul_unit_vec;
	QLabel		* m_label_rul_angle_xy;
	QLabel		* m_label_rul_angle_z;
	QLabel		* m_label_rul_rgb;

	QPushButton	* m_button_cam_delete;
	QPushButton	* m_button_cam_edit;
	QPushButton	* m_button_cam_a_xyz;
	QPushButton	* m_button_cam_b_xyz;
	QTableWidget	* m_camera_table;

	QTableWidget	* m_ruler_table;
	mtQEX::ButtonMenu * m_button_rul_plane;
	QPushButton	* m_button_rul_rgb_set;
	QPushButton	* m_button_rul_delete;
	QPushButton	* m_button_rul_copy;
	QPushButton	* m_button_rul_edit;
	QPushButton	* m_button_rul_hide_all;

	QLineEdit	* m_info_db;
	QLabel		* m_info_points;
	QLabel		* m_info_resolution;
	QLabel		* m_info_x;
	QLabel		* m_info_y;
	QLabel		* m_info_z;
	QLabel		* m_info_r;
	QLabel		* m_info_g;
	QLabel		* m_info_b;

	Frontend	& m_fe;

	mtKit::UserPrefs	& m_uprefs;
	MemPrefs	const	& m_mprefs;
	mtKit::RecentFile	& recent_crul_db;
};



class DialogLabelEdit : public QDialog
{
	Q_OBJECT

public:
	DialogLabelEdit (
		Mainwindow	* mw,
		char	const	* title,
		char	const	* label
		);

	int get_text ( std::string & text );	// 0=Cancelled 1=Confirmed

private:
	QLineEdit	* m_edit;
};



#endif		// QT5_MW_H

