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

#include "qt5_mw.h"



Mainwindow::Mainwindow ( Frontend & fe )
	:
	m_menu_bar		( NULL ),
	m_split_main		( NULL ),
	m_split_view		( NULL ),

	m_cloud_view_a		( NULL ),
	m_cloud_view_b		( NULL ),

	m_menu_ruler		( NULL ),
	act_db_recent		( ),
	act_edit_import_pts	( NULL ),
	act_view_show_antialiasing ( NULL ),
	act_view_show_crosshair	( NULL ),
	act_view_show_statusbar	( NULL ),
	act_view_show_rulers	( NULL ),
	act_view_show_ruler_plane ( NULL ),
	act_view_show_cloud	( NULL ),
	act_view_show_model	( NULL ),
	act_view_split		( NULL ),
	act_view_split_switch	( NULL ),
	act_view_res_low	( NULL ),
	act_view_res_medium	( NULL ),
	act_view_res_high	( NULL ),
	m_tabs_mode		( NULL ),
	m_tab_camera		( NULL ),
	m_tab_ruler		( NULL ),
	m_slider_ax		( NULL ),
	m_slider_az		( NULL ),
	m_slider_bx		( NULL ),
	m_slider_bz		( NULL ),

	m_slider_nudge		( NULL ),
	m_slider_pt_size	( NULL ),
	m_slider_line_butt_size	( NULL ),
	m_slider_line_thickness	( NULL ),

	m_label_nudge		( NULL ),
	m_label_pt_size		( NULL ),
	m_label_line_butt_size	( NULL ),
	m_label_line_thickness	( NULL ),

	m_label_rul_axyz	( NULL ),
	m_label_rul_bxyz	( NULL ),
	m_label_rul_length	( NULL ),
	m_label_rul_unit_vec	( NULL ),
	m_label_rul_angle_xy	( NULL ),
	m_label_rul_angle_z	( NULL ),
	m_label_rul_rgb		( NULL ),

	m_button_cam_delete	( NULL ),
	m_button_cam_edit	( NULL ),
	m_button_cam_a_xyz	( NULL ),
	m_button_cam_b_xyz	( NULL ),
	m_camera_table		( NULL ),

	m_ruler_table		( NULL ),
	m_button_rul_plane	( NULL ),
	m_button_rul_rgb_set	( NULL ),
	m_button_rul_delete	( NULL ),
	m_button_rul_copy	( NULL ),
	m_button_rul_edit	( NULL ),
	m_button_rul_hide_all	( NULL ),

	m_info_file		( NULL ),
	m_info_points		( NULL ),
	m_info_resolution	( NULL ),
	m_info_x		( NULL ),
	m_info_y		( NULL ),
	m_info_z		( NULL ),
	m_info_r		( NULL ),
	m_info_g		( NULL ),
	m_info_b		( NULL ),
	m_fe			( fe ),
	m_prefs			( fe.backend.prefs )
{
	setWindowTitle ( VERSION );

	std::string path;
	mtKit::get_data_dir ( path, DATA_INSTALL "/icons/hicolor/256x256/apps/"
		BIN_NAME ".png" );
	setWindowIcon ( QIcon ( path.c_str () ) );

	setEnabled ( false );

	create_main_ui ();

	setMinimumSize ( 160, 160 );
	setGeometry ( m_prefs.getInt ( PREFS_WINDOW_X ),
		m_prefs.getInt ( PREFS_WINDOW_Y ),
		m_prefs.getInt ( PREFS_WINDOW_W ),
		m_prefs.getInt ( PREFS_WINDOW_H ) );

	if ( 1 == m_prefs.getInt ( PREFS_WINDOW_MAXIMIZED ) )
	{
		showMaximized ();
	}
	else
	{
		show ();
	}

	setEnabled ( true );

	// Restore positions of the splits
	QByteArray qba;

	if ( 0 == mtQEX::qt_get_state ( &m_prefs, PREFS_CRUL_SPLIT_MAIN, &qba ))
	{
		m_split_main->restoreState ( qba );
	}
	else
	{
		QList<int> list;

		list.append ( (int)(width () * 0.75) );
		list.append ( (int)(width () * 0.25) );

		m_split_main->setSizes ( list );
	}

	if ( 0 == mtQEX::qt_get_state ( &m_prefs, PREFS_VIEW_SPLIT_POS, &qba ) )
	{
		m_split_view->restoreState ( qba );
	}

	update_recent_db_menu ();

	act_view_split->setChecked ( (0 != m_prefs.getInt ( PREFS_VIEW_SPLIT_ON
		) ) );
	press_view_split ();

	set_view_split_vert ( 0 != m_prefs.getInt ( PREFS_VIEW_SPLIT_VERT ) );

	press_edit_mode_camera ();	// Prepare mode

	press_file_recent ( 1 );	// Load most recent DB

	update_view_show_items ();
}

Mainwindow::~Mainwindow ()
{
	if ( isMaximized () )
	{
		m_prefs.set ( PREFS_WINDOW_MAXIMIZED, 1 );
	}
	else
	{
		m_prefs.set ( PREFS_WINDOW_MAXIMIZED, 0 );

		m_prefs.set ( PREFS_WINDOW_X, geometry().x () );
		m_prefs.set ( PREFS_WINDOW_Y, geometry().y () );
		m_prefs.set ( PREFS_WINDOW_W, geometry().width () );
		m_prefs.set ( PREFS_WINDOW_H, geometry().height () );
	}

	QByteArray qba;

	qba = m_split_main->saveState ();
	mtQEX::qt_set_state ( &m_prefs, PREFS_CRUL_SPLIT_MAIN, &qba );

	qba = m_split_view->saveState ();
	mtQEX::qt_set_state ( &m_prefs, PREFS_VIEW_SPLIT_POS, &qba );

	m_prefs.set ( PREFS_VIEW_SPLIT_ON, act_view_split->isChecked () ? 1:0 );
	m_prefs.set ( PREFS_VIEW_SPLIT_VERT,
		m_split_view->orientation () == Qt::Vertical ? 1 : 0 );

	m_fe.save_db_state ();
}

static QVBoxLayout * tight_vbox ( QWidget * parent )
{
	QVBoxLayout * vlay = new QVBoxLayout ( parent );
	vlay->setMargin ( 0 );
	vlay->setSpacing ( 0 );

	return vlay;
}

void Mainwindow::create_main_ui ()
{
	QWidget * w = new QWidget;
	setCentralWidget ( w );

	QVBoxLayout * vbox = tight_vbox ( w );
	w->setLayout ( vbox );

	m_split_main = new QSplitter ( Qt::Horizontal );
	vbox->addWidget ( m_split_main );

	QHBoxLayout	* hbox;
	QVBoxLayout	* vl, * vlay;
	QGridLayout	* grid;
	QPushButton	* button;

///	LEFT - OpenGL ----------------------------------------------------------
	w = new QWidget;
	m_split_main->addWidget ( w );
	vlay = tight_vbox ( w );

	m_split_view = new QSplitter ( Qt::Vertical );
	vlay->addWidget ( m_split_view );

	m_cloud_view_a = new CloudView ( m_prefs, PREFS_VIEW_A, m_fe.cloud_gl,
		m_fe.ruler_gl, m_fe.model_gl, NULL );
	m_split_view->addWidget ( m_cloud_view_a );
	connect ( m_cloud_view_a, SIGNAL(ruler_changed ()),
		this, SLOT(update_ruler ()) );
	connect ( m_cloud_view_a,
		SIGNAL(keypress_ruler	(CloudView *, QKeyEvent *)), this,
		SLOT(keypress_ruler	(CloudView *, QKeyEvent *)) );
	connect ( m_cloud_view_a,
		SIGNAL(mouse_ruler	(CloudView *, QMouseEvent *, int, int)),
		this,
		SLOT(mouse_ruler	(CloudView *, QMouseEvent *, int, int))
		);

	m_cloud_view_b = new CloudView ( m_prefs, PREFS_VIEW_B, m_fe.cloud_gl,
		m_fe.ruler_gl, m_fe.model_gl, NULL );
	m_split_view->addWidget ( m_cloud_view_b );
	connect ( m_cloud_view_b, SIGNAL(ruler_changed ()),
		this, SLOT(update_ruler ()) );
	connect ( m_cloud_view_b,
		SIGNAL(keypress_ruler	(CloudView *, QKeyEvent *)), this,
		SLOT(keypress_ruler	(CloudView *, QKeyEvent *)) );
	connect ( m_cloud_view_b,
		SIGNAL(mouse_ruler	(CloudView *, QMouseEvent *, int, int)),
		this,
		SLOT(mouse_ruler	(CloudView *, QMouseEvent *, int, int))
		);

///	RIGHT
	w = new QWidget;
	m_split_main->addWidget ( w );
	vlay = tight_vbox ( w );

///	MENU BAR
	m_menu_bar = new QMenuBar;
	vlay->addWidget ( m_menu_bar );
	m_menu_bar->setSizePolicy ( QSizePolicy ( QSizePolicy::Preferred,
		QSizePolicy::Maximum ) );

	create_menu ();

	m_tabs_mode = new QTabWidget;
	vlay->addWidget ( m_tabs_mode );

	w = new QWidget;
	m_tabs_mode->addTab ( w, "Info" );
	vl = tight_vbox ( w );

	w = new QWidget;
	vl->addWidget ( w );

	grid = new QGridLayout;
	grid->setSizeConstraint ( QLayout::SetFixedSize );
	w->setLayout ( grid );

	int row = 0, col = 0;

	grid->addWidget ( new QLabel ( "File" ),	row++, col );
	grid->addWidget ( new QLabel ( "Points" ),	row++, col );
	grid->addWidget ( new QLabel ( "Resolution  " ), row++, col );
	grid->addWidget ( new QLabel ( "X" ),		row++, col );
	grid->addWidget ( new QLabel ( "Y" ),		row++, col );
	grid->addWidget ( new QLabel ( "Z" ),		row++, col );
	grid->addWidget ( new QLabel ( "R" ),		row++, col );
	grid->addWidget ( new QLabel ( "G" ),		row++, col );
	grid->addWidget ( new QLabel ( "B" ),		row++, col );

	row = 0;
	col = 1;

	grid->addWidget ( m_info_file = new QLabel,	row++, col );
	grid->addWidget ( m_info_points = new QLabel,	row++, col );
	grid->addWidget ( m_info_resolution = new QLabel ("Low"), row++, col );
	grid->addWidget ( m_info_x = new QLabel,	row++, col );
	grid->addWidget ( m_info_y = new QLabel,	row++, col );
	grid->addWidget ( m_info_z = new QLabel,	row++, col );
	grid->addWidget ( m_info_r = new QLabel,	row++, col );
	grid->addWidget ( m_info_g = new QLabel,	row++, col );
	grid->addWidget ( m_info_b = new QLabel,	row++, col );

	w = new QWidget;
	w->setSizePolicy ( QSizePolicy ( QSizePolicy::Expanding,
		QSizePolicy::Expanding ) );
	vl->addWidget ( w );

/// ----------------------------------------------------------------------------

	w = new QWidget;
	m_tabs_mode->addTab ( w, "Camera" );
	m_tab_camera = w;

	vl = tight_vbox ( w );

	w = new QWidget;
	vl->addWidget ( w );

	grid = new QGridLayout;
	w->setLayout ( grid );

	row = 0;
	col = 0;

	grid->addWidget ( new QLabel ( "A X°" ),	row++, col );
	grid->addWidget ( new QLabel ( "A Z°" ),	row++, col );

	row = 0;
	col = 1;

	m_slider_ax = new QSlider ( Qt::Horizontal );
	m_slider_ax->setRange ( 0, 359 );
	grid->addWidget ( m_slider_ax, row++, col );

	connect ( m_slider_ax, SIGNAL(valueChanged (int)),
		m_cloud_view_a, SLOT(set_xrotation (int)) );
	connect ( m_cloud_view_a, SIGNAL(xrotation_changed (double)),
		this, SLOT(set_xrotation_a (double)) );

	m_slider_az = new QSlider ( Qt::Horizontal );
	m_slider_az->setRange ( 0, 359 );
	grid->addWidget ( m_slider_az, row++, col );

	connect ( m_slider_az, SIGNAL(valueChanged (int)),
		m_cloud_view_a, SLOT(set_zrotation (int)) );
	connect ( m_cloud_view_a, SIGNAL(zrotation_changed (double)),
		this, SLOT(set_zrotation_a (double)) );

	w = new QWidget;
	vl->addWidget ( w );

	grid = new QGridLayout;
	w->setLayout ( grid );

	row = 0;
	col = 0;

	grid->addWidget ( new QLabel ( "B X°" ),	row++, col );
	grid->addWidget ( new QLabel ( "B Z°" ),	row++, col );

	row = 0;
	col = 1;

	m_slider_bx = new QSlider ( Qt::Horizontal );
	m_slider_bx->setRange ( 0, 359 );
	grid->addWidget ( m_slider_bx, row++, col );

	connect ( m_slider_bx, SIGNAL(valueChanged (int)),
		m_cloud_view_b, SLOT(set_xrotation (int)) );
	connect ( m_cloud_view_b, SIGNAL(xrotation_changed (double)),
		this, SLOT(set_xrotation_b (double)) );

	m_slider_bz = new QSlider ( Qt::Horizontal );
	m_slider_bz->setRange ( 0, 359 );
	grid->addWidget ( m_slider_bz, row++, col );

	connect ( m_slider_bz, SIGNAL(valueChanged (int)),
		m_cloud_view_b, SLOT(set_zrotation (int)) );
	connect ( m_cloud_view_b, SIGNAL(zrotation_changed (double)),
		this, SLOT(set_zrotation_b (double)) );

	connect ( m_cloud_view_a, SIGNAL(camera_changed ()),
		this, SLOT(update_view_a ()) );
	connect ( m_cloud_view_b, SIGNAL(camera_changed ()),
		this, SLOT(update_view_b ()) );


	w = new QWidget;
	vl->addWidget ( w );
	grid = new QGridLayout;
	grid->setSizeConstraint ( QLayout::SetFixedSize );
	w->setLayout ( grid );

	row = 0;
	col = 0;
	grid->addWidget ( new QLabel ( "Nudge" ),	row++, col );
	row = 0;
	col = 1;
	grid->addWidget ( m_label_nudge = new QLabel,	row++, col );

	int const nudge = m_prefs.getInt ( PREFS_VIEW_NUDGE_SIZE );
	m_slider_nudge = new QSlider ( Qt::Horizontal );
	m_slider_nudge->setRange ( Crul::VIEW_NUDGE_MIN,
		Crul::VIEW_NUDGE_MAX );
	m_slider_nudge->setValue ( nudge );
	vl->addWidget ( m_slider_nudge );
	connect ( m_slider_nudge, SIGNAL(valueChanged (int)),
		this, SLOT(set_nudge (int)) );
	set_nudge ( nudge );


	w = new QWidget;
	vl->addWidget ( w );
	grid = new QGridLayout;
	grid->setSizeConstraint ( QLayout::SetFixedSize );
	w->setLayout ( grid );

	row = 0;
	col = 0;
	grid->addWidget ( new QLabel ( "Point Size" ),	row++, col );
	row = 0;
	col = 1;
	grid->addWidget ( m_label_pt_size = new QLabel,	row++, col );

	m_slider_pt_size = new QSlider ( Qt::Horizontal );
	m_slider_pt_size->setRange ( Crul::VIEW_POINT_SIZE_MIN,
		Crul::VIEW_POINT_SIZE_MAX );
	set_point_size ( (int)m_prefs.getDouble ( PREFS_GL_POINT_SIZE ) );
	vl->addWidget ( m_slider_pt_size );
	connect ( m_slider_pt_size, SIGNAL(valueChanged (int)),
		this, SLOT(set_point_size (int)) );

	w = new QWidget;
	vl->addWidget ( w );
	grid = new QGridLayout;
	grid->setSizeConstraint ( QLayout::SetFixedSize );
	w->setLayout ( grid );

	row = 0;
	col = 0;
	grid->addWidget ( new QLabel ( "Line Butt Size" ), row++, col );
	row = 0;
	col = 1;
	grid->addWidget ( m_label_line_butt_size = new QLabel, row++, col );

	m_slider_line_butt_size = new QSlider ( Qt::Horizontal );
	m_slider_line_butt_size->setRange ( Crul::VIEW_LINE_BUTT_SIZE_MIN,
		Crul::VIEW_LINE_BUTT_SIZE_MAX );
	set_line_butt_size ( (int)m_prefs.getDouble ( PREFS_GL_LINE_BUTT_SIZE));
	vl->addWidget ( m_slider_line_butt_size );
	connect ( m_slider_line_butt_size, SIGNAL(valueChanged (int)),
		this, SLOT(set_line_butt_size (int)) );

	w = new QWidget;
	vl->addWidget ( w );
	grid = new QGridLayout;
	grid->setSizeConstraint ( QLayout::SetFixedSize );
	w->setLayout ( grid );

	row = 0;
	col = 0;
	grid->addWidget ( new QLabel ( "Line Thickness" ), row++, col );
	row = 0;
	col = 1;
	grid->addWidget ( m_label_line_thickness = new QLabel, row++, col );

	m_slider_line_thickness = new QSlider ( Qt::Horizontal );
	m_slider_line_thickness->setRange ( Crul::VIEW_LINE_THICKNESS_MIN,
		Crul::VIEW_LINE_THICKNESS_MAX );
	set_line_thickness ( (int)m_prefs.getDouble ( PREFS_GL_LINE_THICKNESS));
	vl->addWidget ( m_slider_line_thickness );
	connect ( m_slider_line_thickness, SIGNAL(valueChanged (int)),
		this, SLOT(set_line_thickness (int)) );


	set_slider_angle ( m_slider_ax, m_prefs.getDouble (
		PREFS_VIEW_A PREFS_CAM_XROT ) );
	set_slider_angle ( m_slider_az, m_prefs.getDouble (
		PREFS_VIEW_A PREFS_CAM_ZROT ) );

	set_slider_angle ( m_slider_bx, m_prefs.getDouble (
		PREFS_VIEW_B PREFS_CAM_XROT ) );
	set_slider_angle ( m_slider_bz, m_prefs.getDouble (
		PREFS_VIEW_B PREFS_CAM_ZROT ) );

	m_camera_table = new QTableWidget;
	vl->addWidget ( m_camera_table );
	m_camera_table->setSizePolicy ( QSizePolicy ( QSizePolicy::Expanding,
		QSizePolicy::Expanding ) );

	m_camera_table->setSelectionMode ( QAbstractItemView::SingleSelection );
	m_camera_table->setSelectionBehavior ( QAbstractItemView::SelectRows );
	m_camera_table->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	m_camera_table->setColumnCount ( 3 );
	m_camera_table->setShowGrid ( false );
	m_camera_table->verticalHeader ()->QEX_RESIZEMODE( QHeaderView::Fixed );
	m_camera_table->horizontalHeader ()->resizeSections (
		QHeaderView::ResizeToContents );
	m_camera_table->horizontalHeader ()->setStretchLastSection ( true );

	connect( m_camera_table, SIGNAL( currentCellChanged(int, int, int,int)),
		this, SLOT ( press_camera_list_row ( int, int, int, int ) ) );
	connect( m_camera_table, SIGNAL( itemChanged(QTableWidgetItem *)),
		this, SLOT ( press_camera_list_item_changed (QTableWidgetItem *)
		) );

	QStringList columnLabels;
	columnLabels << "ID" << "R/O" << "Label";
	m_camera_table->setHorizontalHeaderLabels ( columnLabels );

	w = new QWidget;
	vl->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	button = new QPushButton ( QIcon::fromTheme ("list-add"), "New" );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_camera_new () ) );

	button = new QPushButton ( QIcon::fromTheme ("list-remove"), "Delete" );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_camera_delete () ) );
	m_button_cam_delete = button;

	button = new QPushButton ( "Edit" );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_camera_edit () ) );
	m_button_cam_edit = button;

	button = new QPushButton ( "A>XYZ" );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_camera_a_xyz () ) );
	m_button_cam_a_xyz = button;

	button = new QPushButton ( "B>XYZ" );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_camera_b_xyz () ) );
	m_button_cam_b_xyz = button;

	enable_camera_buttons ( NULL );

/// ----------------------------------------------------------------------------

	w = new QWidget;
	m_tabs_mode->addTab ( w, "Ruler" );
	m_tab_ruler = w;
	vl = tight_vbox ( w );

	w = new QWidget;
	vl->addWidget ( w );

	grid = new QGridLayout;
	grid->setSizeConstraint ( QLayout::SetFixedSize );
	w->setLayout ( grid );

	row = 0;
	col = 0;

	grid->addWidget ( new QLabel ( "A" ),		row++, col );
	grid->addWidget ( new QLabel ( "B" ),		row++, col );
	grid->addWidget ( new QLabel ( "Length" ),	row++, col );
	grid->addWidget ( new QLabel ( "Unit vector  " ), row++, col );
	grid->addWidget ( new QLabel ( "XY°" ),		row++, col );
	grid->addWidget ( new QLabel ( "Z°" ),		row++, col );
	grid->addWidget ( new QLabel ( "RGB" ),		row++, col );
	grid->addWidget ( new QLabel ( "Plane" ),	row++, col );

	row = 0;
	col = 1;

	m_button_rul_plane = new mtQEX::ButtonMenu;

	// Index order as per Crul::PLANE_*
	m_button_rul_plane->addItem ( "XY" );
	m_button_rul_plane->addItem ( "XZ" );
	m_button_rul_plane->addItem ( "YZ" );

	connect ( m_button_rul_plane, SIGNAL ( currentIndexChanged ( int ) ),
		this, SLOT ( change_ruler_plane ( int ) ) );

	grid->addWidget ( m_label_rul_axyz = new QLabel,	row++, col );
	grid->addWidget ( m_label_rul_bxyz = new QLabel,	row++, col );
	grid->addWidget ( m_label_rul_length = new QLabel,	row++, col );
	grid->addWidget ( m_label_rul_unit_vec = new QLabel,	row++, col );
	grid->addWidget ( m_label_rul_angle_xy = new QLabel,	row++, col );
	grid->addWidget ( m_label_rul_angle_z = new QLabel,	row++, col );
	grid->addWidget ( m_label_rul_rgb = new QLabel,		row++, col );
	grid->addWidget ( m_button_rul_plane,			row++, col );

	row = 6;
	col = 2;

	grid->addWidget ( m_button_rul_rgb_set = new QPushButton (
		QIcon::fromTheme ( "stock-edit" ), "Set" ), row++, col );
	connect ( m_button_rul_rgb_set, SIGNAL ( clicked () ),
		this, SLOT ( press_ruler_rgb_set () ) );

	m_ruler_table = new QTableWidget;
	vl->addWidget ( m_ruler_table );
	m_ruler_table->setSizePolicy ( QSizePolicy ( QSizePolicy::Expanding,
		QSizePolicy::Expanding ) );

	m_ruler_table->setSelectionMode ( QAbstractItemView::SingleSelection );
	m_ruler_table->setSelectionBehavior ( QAbstractItemView::SelectRows );
	m_ruler_table->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	m_ruler_table->setColumnCount ( 4 );
	m_ruler_table->setShowGrid ( false );
	m_ruler_table->verticalHeader ()->QEX_RESIZEMODE( QHeaderView::Fixed );
	m_ruler_table->horizontalHeader ()->resizeSections (
		QHeaderView::ResizeToContents );
	m_ruler_table->horizontalHeader ()->setStretchLastSection ( true );

	connect( m_ruler_table, SIGNAL( currentCellChanged(int, int, int,int)),
		this, SLOT ( press_ruler_list_row ( int, int, int, int ) ) );
	connect( m_ruler_table, SIGNAL( itemChanged(QTableWidgetItem *)),
		this, SLOT ( press_ruler_list_item_changed (QTableWidgetItem *)
		) );

	columnLabels.clear ();
	columnLabels << "ID" << "R/O" << "Show" << "Label";
	m_ruler_table->setHorizontalHeaderLabels ( columnLabels );

	w = new QWidget;
	vl->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	button = new QPushButton ( QIcon::fromTheme ("list-add"), "New" );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_ruler_new () ) );

	button = new QPushButton ( QIcon::fromTheme ("list-remove"), "Delete" );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_ruler_delete () ) );
	m_button_rul_delete = button;

	button = new QPushButton ( QIcon::fromTheme ("edit-copy"), "Copy" );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_ruler_copy () ) );
	m_button_rul_copy = button;

	button = new QPushButton ( "Edit" );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_ruler_edit () ) );
	m_button_rul_edit = button;

	button = new QPushButton ( "Hide All" );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_ruler_hide_all () ) );
	m_button_rul_hide_all = button;

	enable_ruler_buttons ( NULL );
}

