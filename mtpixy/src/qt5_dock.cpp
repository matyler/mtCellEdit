/*
	Copyright (C) 2016-2020 Mark Tyler

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

#include "qt5.h"



void Mainwindow::create_dock ()
{
	m_palette_dock = new QDockWidget ( "Palette", this );
	m_palette_dock->setObjectName ( "palette_dock" );

	m_palette_holder = new PaletteHolder ( *this );
	m_palette_dock->setWidget ( m_palette_holder );

	addDockWidget ( Qt::RightDockWidgetArea, m_palette_dock );
}

static void set_icon_image (
	mtPixmap	* const	i,
	QAction		* const	qa
	)
{
	if ( ! i )
	{
		return;
	}

	mtPixy::Pixmap const img (i);
	std::unique_ptr<QPixmap> const pixmap (
		mtQEX::qpixmap_from_pixypixmap (i) );
	if ( pixmap )
	{
		qa->setIcon ( QIcon ( *pixmap.get() ) );
	}
}

void Mainwindow::update_brush_colors ()
{
	backend.file.update_brush_colors ();
}

void Mainwindow::update_toolbars ()
{
	mtPixmap	* i;
	int	const	z = backend.get_ui_scale ();


	i = backend.file.brush.build_color_swatch ( z );
	set_icon_image ( i, tb_brush_color );

	i = backend.file.brush.build_shape_swatch ( z );
	set_icon_image ( i, tb_brush_shape );

	i = backend.file.brush.build_pattern_swatch ( z );
	set_icon_image ( i, tb_brush_pattern );

	i = backend.file.brush.build_preview_swatch ( z );
	set_icon_image ( i, tb_brush_settings );

	if ( backend.file.get_tool_mode () == mtPixyUI::File::TOOL_MODE_PASTE &&
		backend.clipboard.is_text_paste () &&
		0 == render_text_paste () )
	{
		update_ui ( UPDATE_CANVAS );
	}
}

void Mainwindow::finish_tool_mode ()
{
	if ( m_paste_committed )
	{
		// Update the undo step if needed
		tool_action_paste_set_undo ();
		m_paste_committed = 0;
	}
}

void Mainwindow::set_tool_mode (
	mtPixyUI::File::ToolMode const	m,
	int			const	updt
	)
{
	finish_tool_mode ();

	backend.file.set_tool_mode ( m );

	m_cursor.redraw ();

	QCursor		cur;

	switch ( m )
	{
	case mtPixyUI::File::TOOL_MODE_PAINT:
	case mtPixyUI::File::TOOL_MODE_PAINTING:
		cur = Qt::ArrowCursor;
		break;

	case mtPixyUI::File::TOOL_MODE_LINE:
	case mtPixyUI::File::TOOL_MODE_LINING:
		cur = Qt::UpArrowCursor;
		break;

	case mtPixyUI::File::TOOL_MODE_SELECT_RECTANGLE:
	case mtPixyUI::File::TOOL_MODE_SELECTING_RECTANGLE:
	case mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE:
	case mtPixyUI::File::TOOL_MODE_SELECT_POLYGON:
	case mtPixyUI::File::TOOL_MODE_SELECTING_POLYGON:
	case mtPixyUI::File::TOOL_MODE_SELECTED_POLYGON:
		cur = Qt::CrossCursor;
		break;

	case mtPixyUI::File::TOOL_MODE_PASTE:
		cur = Qt::OpenHandCursor;
		break;

	case mtPixyUI::File::TOOL_MODE_PASTING:
		cur = Qt::ClosedHandCursor;
		break;

	case mtPixyUI::File::TOOL_MODE_FLOODFILL:
		cur = Qt::PointingHandCursor;
		break;
	}

	m_canvas_main->setCursor ( cur );
	m_canvas_split->setCursor ( cur );

	update_ui ( updt );
}

void Mainwindow::press_paint ()
{
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_PAINT );
}

void Mainwindow::press_line ()
{
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_LINE );
}

void Mainwindow::press_flood_fill ()
{
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_FLOODFILL );
}

void Mainwindow::press_select_rectangle ()
{
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_SELECT_RECTANGLE );
}

void Mainwindow::press_select_polygon ()
{
	set_tool_mode ( mtPixyUI::File::TOOL_MODE_SELECT_POLYGON );
}

void Mainwindow::create_toolbar ()
{
	m_toolbar = new QToolBar ( "Toolbar" );
	m_toolbar->setObjectName ( "toolbar" );

	addToolBar ( Qt::LeftToolBarArea, m_toolbar );

	QActionGroup * act_group = new QActionGroup ( this );

	tb_paint = new QAction ( "Paint", act_group );
	tb_paint->setCheckable ( true );
	m_toolbar->addAction ( tb_paint );
	connect ( tb_paint, SIGNAL ( triggered () ), this,
		SLOT ( press_paint () ) );

	tb_line = new QAction ( "Line", act_group );
	tb_line->setCheckable ( true );
	m_toolbar->addAction ( tb_line );
	connect ( tb_line, SIGNAL ( triggered () ), this,
		SLOT ( press_line () ) );

	tb_flood_fill = new QAction ( "Flood Fill", act_group );
	tb_flood_fill->setCheckable ( true );
	m_toolbar->addAction ( tb_flood_fill );
	connect ( tb_flood_fill, SIGNAL ( triggered () ), this,
		SLOT ( press_flood_fill () ) );

	tb_select_rectangle = new QAction ( "Select Rectangle", act_group );
	tb_select_rectangle->setCheckable ( true );
	m_toolbar->addAction ( tb_select_rectangle );
	connect ( tb_select_rectangle, SIGNAL ( triggered () ), this,
		SLOT ( press_select_rectangle () ) );

	tb_select_polygon = new QAction ( "Select Polygon", act_group );
	tb_select_polygon->setCheckable ( true );
	m_toolbar->addAction ( tb_select_polygon );
	connect ( tb_select_polygon, SIGNAL ( triggered () ), this,
		SLOT ( press_select_polygon () ) );

	m_toolbar->addSeparator ();

	tb_brush_color = m_toolbar->addAction ( "Colour" );
	connect ( tb_brush_color, SIGNAL ( triggered () ), this,
		SLOT ( press_brush_color () ) );

	tb_brush_shape = m_toolbar->addAction ( "Shape" );
	connect ( tb_brush_shape, SIGNAL ( triggered () ), this,
		SLOT ( press_brush_shape () ) );

	tb_brush_pattern = m_toolbar->addAction ( "Pattern" );
	connect ( tb_brush_pattern, SIGNAL ( triggered () ), this,
		SLOT ( press_brush_pattern () ) );

	tb_brush_settings = m_toolbar->addAction ( "Settings" );
	connect ( tb_brush_settings, SIGNAL ( triggered () ), this,
		SLOT ( press_brush_settings () ) );
}

void Mainwindow::create_icons ()
{
	// Default size of image before scaling = 144x96
	int const w = 144;
	int const h = 96;

	std::string path;
	mtKit::get_data_dir ( path, DATA_INSTALL "/" DATA_NAME "/icons/" );

	int const ui = backend.get_ui_scale ();

	path += "default.svg";
	QPixmap pm = QIcon ( path.c_str() ).pixmap ( w * ui, h * ui );

	if ( pm.width() < 1 || pm.height() < 1 )
	{
		QMessageBox::critical( this, "Error", "Unable to load icons." );

		return;
	}

	int const sz = 24 * ui;
	m_toolbar->setIconSize ( QSize ( sz, sz ) );

	tb_paint->setIcon		( pm.copy ( 0*sz, 3*sz, sz, sz ) );
	tb_line->setIcon		( pm.copy ( 1*sz, 3*sz, sz, sz ) );
	tb_flood_fill->setIcon		( pm.copy ( 2*sz, 3*sz, sz, sz ) );
	tb_select_rectangle->setIcon	( pm.copy ( 3*sz, 3*sz, sz, sz ) );
	tb_select_polygon->setIcon	( pm.copy ( 4*sz, 3*sz, sz, sz ) );

	backend.file.brush.rebuild_shapes_palette ( ui );
	backend.file.brush.rebuild_patterns_palette ( ui );

	update_ui ( UPDATE_TOOLBAR );
}

void Mainwindow::main_view_moved ()
{
	main_view_move_h ( 0 );
	main_view_move_v ( 0 );
}

void Mainwindow::main_view_move_h (
	int	const	ARG_UNUSED ( n )
	)
{
	if ( act_options_split_focus->isChecked () )
	{
		double		xp;


		get_scroll_position_h ( m_scroll_main, &xp );
		set_scroll_position_h ( m_scroll_split, xp );
	}
}

void Mainwindow::main_view_move_v (
	int	const	ARG_UNUSED ( n )
	)
{
	if ( act_options_split_focus->isChecked () )
	{
		double		yp;


		get_scroll_position_v ( m_scroll_main, &yp );
		set_scroll_position_v ( m_scroll_split, yp );
	}
}

void Mainwindow::main_view_change_h (
	int	const	ARG_UNUSED ( a ),
	int	const	ARG_UNUSED ( b )
	)
{
	main_view_move_h ( 0 );
}

void Mainwindow::main_view_change_v (
	int	const	ARG_UNUSED ( a ),
	int	const	ARG_UNUSED ( b )
	)
{
	main_view_move_v ( 0 );
}

void Mainwindow::create_easel ()
{
	m_file_split = new QSplitter;
	setCentralWidget ( m_file_split );

	m_easel_split = new QSplitter;
	m_file_split->addWidget ( m_easel_split );

	m_files_table = new QTableWidget;
	m_file_split->addWidget ( m_files_table );
	m_files_table->hide ();
	m_files_table->setSelectionBehavior ( QAbstractItemView::SelectRows );
	m_files_table->setSelectionMode ( QAbstractItemView::SingleSelection );
	m_files_table->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	m_files_table->setColumnCount ( 1 );
	m_files_table->setShowGrid ( false );
	m_files_table->horizontalHeader ()->hide ();
	m_files_table->horizontalHeader ()->setStretchLastSection ( true );
	m_files_table->setHorizontalScrollMode ( QAbstractItemView::
		ScrollPerPixel );

	m_scroll_main = new QScrollArea;
	m_canvas_main = new CanvasView ( m_scroll_main, *this );
	m_scroll_split = new QScrollArea;
	m_canvas_split = new CanvasView ( m_scroll_split, *this );

	m_easel_split->addWidget ( m_scroll_main );
	m_easel_split->addWidget ( m_scroll_split );

	split_hide ();

	connect ( m_scroll_main->horizontalScrollBar (), SIGNAL ( valueChanged (
		int ) ), this, SLOT ( main_view_move_h ( int ) ) );
	connect ( m_scroll_main->verticalScrollBar (), SIGNAL ( valueChanged (
		int ) ), this, SLOT ( main_view_move_v ( int ) ) );

	connect ( m_scroll_main->horizontalScrollBar (), SIGNAL ( rangeChanged (
		int, int ) ), this, SLOT ( main_view_change_h ( int, int ) ) );
	connect ( m_scroll_main->verticalScrollBar (), SIGNAL ( rangeChanged (
		int, int ) ), this, SLOT ( main_view_change_v ( int, int ) ) );
}

void Mainwindow::split_hide ()
{
	m_scroll_split->hide ();
	update_ui ( UPDATE_STATUS_GEOMETRY );
}

void Mainwindow::split_show ()
{
	m_scroll_split->show ();
	update_ui ( UPDATE_STATUS_GEOMETRY );
}

void Mainwindow::split_switch ()
{
	if ( m_easel_split->orientation () == Qt::Vertical )
	{
		m_easel_split->setOrientation ( Qt::Horizontal );
	}
	else
	{
		m_easel_split->setOrientation ( Qt::Vertical );
	}
}

bool Mainwindow::is_split_visible ()
{
	return m_scroll_split->isVisible ();
}

void Mainwindow::update_canvas (
	int	const	xx,
	int	const	yy,
	int	const	w,
	int	const	h
	)
{
	m_canvas_main->update_canvas ( xx, yy, w, h );
	m_canvas_split->update_canvas ( xx, yy, w, h );
}

void Mainwindow::set_canvas_rgb_transform (
	int	const	g,
	int	const	b,
	int	const	c,
	int	const	s,
	int	const	h,
	int	const	p
	)
{
	mtPixmap const * const i = backend.file.get_pixmap ();


	if ( pixy_pixmap_get_bytes_per_pixel (i) == PIXY_PIXMAP_BPP_RGB )
	{
		m_canvas_main->set_canvas_rgb_transform ( g, b, c, s, h, p );
		m_canvas_split->set_canvas_rgb_transform ( g, b, c, s, h, p );
	}
}


void Mainwindow::unset_canvas_rgb_transform ()
{
	m_canvas_main->unset_canvas_rgb_transform ();
	m_canvas_split->unset_canvas_rgb_transform ();
}

