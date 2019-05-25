/*
	Copyright (C) 2016-2018 Mark Tyler

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

#include "qt4.h"



CanvasView::CanvasView (
	QScrollArea	* const	scr,
	Mainwindow	&mw
	)
	:
	m_zoom		( ZOOM_DEFAULT ),
	m_zoom_cx	( -1 ),
	m_zoom_cy	( -1 ),
	m_zoom_grid	( 1 ),
	m_tran_rgb	( 0 ),
	m_scroll_area	( scr ),
	mainwindow	( mw )
{
	setMouseTracking ( true );

	m_scroll_area->setWidget ( this );
	m_scroll_area->setBackgroundRole ( QPalette::Dark );
	m_scroll_area->setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );

	update_canvas_easel_rgb ();
}

CanvasView::~CanvasView ()
{
}

void get_scroll_position_h (
	QScrollArea	* const sa,
	double		* const	zxp
	)
{
	int	const	sxm = sa->horizontalScrollBar ()->maximum ();
	int	const	sxpage = sa->horizontalScrollBar()->pageStep();
	int	const	sx = sa->horizontalScrollBar ()->value ();


	if ( sxm > 0 )
	{
		zxp[0] = (double)(sx + sxpage/2) / (double)(sxm + sxpage);
	}
	else
	{
		// No scrollbar so set as centre
		zxp[0] = 0.5;
	}
}

void get_scroll_position_v (
	QScrollArea	* const sa,
	double		* const	zyp
	)
{
	int	const	sym = sa->verticalScrollBar ()->maximum ();
	int	const	sypage = sa->verticalScrollBar ()->pageStep ();
	int	const	sy = sa->verticalScrollBar ()->value ();


	if ( sym > 0 )
	{
		zyp[0] = (double)(sy + sypage/2) / (double)(sym + sypage);
	}
	else
	{
		// No scrollbar so set as centre
		zyp[0] = 0.5;
	}
}

void set_scroll_position_h (
	QScrollArea	* const sa,
	double		const	zxp
	)
{
	int	const	sxm = sa->horizontalScrollBar ()->maximum ();
	int	const	sxpage = sa->horizontalScrollBar()->pageStep();


	sa->horizontalScrollBar ()->setValue (
		(int)((sxm + sxpage) * zxp) - sxpage / 2 );
}

void set_scroll_position_v (
	QScrollArea	* const sa,
	double		const	zyp
	)
{
	int	const	sym = sa->verticalScrollBar ()->maximum ();
	int	const	sypage = sa->verticalScrollBar ()->pageStep ();


	sa->verticalScrollBar ()->setValue (
		(int)((sym + sypage) * zyp) - sypage / 2 );
}

void CanvasView::set_zoom (
	int	const	z
	)
{
	int		nz;
	double		zxp, zyp;
	int	const	zs = get_zoom_scale ();


	nz = MIN ( ZOOM_MAX, z );
	nz = MAX ( ZOOM_MIN, nz );

	if ( nz == m_zoom )
	{
		return;
	}

	m_zoom = nz;

	if ( m_zoom_cx < 0 )
	{
		get_scroll_position_h ( m_scroll_area, &zxp );
		get_scroll_position_v ( m_scroll_area, &zyp );
	}
	else
	{
		double	const	zx = zs < 0 ? m_zoom_cx / -zs : m_zoom_cx * zs;
		double	const	zy = zs < 0 ? m_zoom_cy / -zs : m_zoom_cy * zs;


		// Calculate zoom centre based on user's middle button
		zxp = zx / (double)width ();
		zyp = zy / (double)height ();
	}

	mainwindow.update_ui ( Mainwindow::UPDATE_STATUS_GEOMETRY );
	reconfigure ();

	set_scroll_position_h ( m_scroll_area, zxp );
	set_scroll_position_v ( m_scroll_area, zyp );

	m_zoom_cx = -1;
	m_zoom_cy = -1;

	mainwindow.set_last_zoom_scale ( get_zoom_scale () );
}

int CanvasView::get_zoom_percent () const
{
	int	const	zs = get_zoom_scale ();


	if ( zs < 0 )
	{
		return 100 / -zs;
	}

	// zoom >= 0
	return zs * 100;
}

double CanvasView::get_centre_canvas_x () const
{
	double		p;

	get_scroll_position_h ( m_scroll_area, &p );

	return p;
}

double CanvasView::get_centre_canvas_y () const
{
	double		p;

	get_scroll_position_v ( m_scroll_area, &p );

	return p;
}

int CanvasView::get_zoom_scale () const
{
	if ( m_zoom == 0 )
	{
		return 1;
	}


	int	const	az = MIN ( ZOOM_MAX, abs ( m_zoom ) );
	int	const	aze = ( 2 << ((az - 1)/2) );


	if ( m_zoom > 0 )
	{
		if ( (az % 2) == 0 )
		{
			// az is even
			return ( (2 << (az/2)) + aze ) / 2;
		}

		// az is odd
		return aze;
	}

	// zoom < 0

	if ( (az % 2) == 0 )
	{
		// az is even
		return -( (2 << (az/2)) + aze ) / 2;
	}

	// az is odd
	return -aze;
}

void CanvasView::set_zoom_grid (
	int	const	z
	)
{
	m_zoom_grid = z;
	update ();
}

void CanvasView::zoom_in ()
{
	set_zoom ( MIN ( m_zoom + 1, ZOOM_MAX ) );
}

void CanvasView::zoom_out ()
{
	set_zoom ( MAX ( m_zoom - 1, ZOOM_MIN ) );
}

void CanvasView::reconfigure ()
{
	int		const	zs = get_zoom_scale ();
	mtPixy::Image	const * im = mainwindow.backend.file.get_image ();
	int		const	w = im ? im->get_width () : 0;
	int		const	h = im ? im->get_height () : 0;


	if ( zs < 0 )
	{
		resize ( w / -zs, h / -zs );
	}
	else	// zoom >= 0
	{
		resize ( w * zs, h * zs );
	}

	update ();
}

void CanvasView::update_canvas (
	int	const	xx,
	int	const	yy,
	int	const	w,
	int	const	h
	)
{
	int	const	zs = get_zoom_scale ();


	if ( zs < 0 )
	{
		int	const	ax = xx / -zs;
		int	const	ay = yy / -zs;
		int	const	bx = (xx + w - 1) / -zs;
		int	const	by = (yy + h - 1) / -zs;


		update ( ax, ay, bx - ax + 1, by - ay + 1 );
	}
	else	// zoom >= 0
	{
		update ( xx * zs, yy * zs, w * zs, h * zs );
	}
}

void CanvasView::update_canvas_easel_rgb ()
{
	int	const	col = mainwindow.prefs.getInt( PREFS_CANVAS_EASEL_RGB );
	QPalette	pal = m_scroll_area->palette ();


	pal.setColor ( QPalette::Dark, QColor ( mtPixy::int_2_red ( col ),
		mtPixy::int_2_green ( col ), mtPixy::int_2_blue ( col ) ) );
	m_scroll_area->setPalette ( pal );
}

void CanvasView::set_canvas_rgb_transform (
	int	const	g,
	int	const	b,
	int	const	c,
	int	const	s,
	int	const	h,
	int	const	p
	)
{
	m_tran_rgb = 1;
	m_tran_gamma = g;
	m_tran_brightness = b;
	m_tran_contrast = c;
	m_tran_saturation = s;
	m_tran_hue = h;
	m_tran_posterize = p;

	update ();
}

void CanvasView::unset_canvas_rgb_transform ()
{
	m_tran_rgb = 0;

	update ();
}

static void polygon_overlay_render (
	QPainter			&p,
	mtPixy::Brush			&bru,
	int			const	&zs,
	mtPixy::PolySelOverlay		&ovl,
	mtPixyUI::File::ToolMode const	tm
	)
{
	mtPixy::Color col = bru.get_color_a ();

	p.setPen ( QColor ( col.red, col.green, col.blue ) );

	for ( int i = 0; i < ovl.m_point_total; i++ )
	{
		int x1 = ovl.m_x[ i ], x2;
		int y1 = ovl.m_y[ i ], y2;

		if ( i < (ovl.m_point_total - 1) )
		{
			x2 = ovl.m_x[ i+1 ];
			y2 = ovl.m_y[ i+1 ];
		}
		else
		{
			if ( tm == mtPixyUI::File::TOOL_MODE_SELECTED_POLYGON )
			{
				x2 = ovl.m_x[ 0 ];
				y2 = ovl.m_y[ 0 ];
			}
			else
			{
				x2 = ovl.get_x1 ();
				y2 = ovl.get_y1 ();
			}
		}

		if ( zs < 0 )
		{
			x1 = (x1 / -zs);
			x2 = (x2 / -zs);
			y1 = (y1 / -zs);
			y2 = (y2 / -zs);
		}
		else
		{
			x1 = (x1 * zs) + zs / 2;
			x2 = (x2 * zs) + zs / 2;
			y1 = (y1 * zs) + zs / 2;
			y2 = (y2 * zs) + zs / 2;
		}

		p.drawLine ( x1, y1, x2, y2 );
	}
}

void CanvasView::paintEvent (
	QPaintEvent	* const	ev
	)
{
	int	const	px = ev->rect ().x ();
	int	const	py = ev->rect ().y ();
	int	const	pw = ev->rect ().width ();
	int	const	ph = ev->rect ().height ();
	int	const	zs = get_zoom_scale ();


	if ( pw < 1 || ph < 1 )
	{
		return;
	}


///	IMAGE

	mtPixy::Image * im = mainwindow.backend.file.render_canvas ( px, py,
		pw, ph, zs );
	if ( ! im )
	{
		return;
	}

	unsigned char * rgb = im->get_canvas ();
	if ( ! rgb )
	{
		delete im;
		return;
	}

///	TRANSFORMS

	if ( m_tran_rgb )
	{
		mtPixy::transform_color ( rgb, pw * ph, m_tran_gamma,
			m_tran_brightness, m_tran_contrast, m_tran_saturation,
			m_tran_hue, m_tran_posterize );
	}

///	GRID

	if ( m_zoom_grid && zs > 8 )
	{
		unsigned char const gry = (unsigned char)mainwindow.prefs.
			getInt ( PREFS_CANVAS_ZOOM_GRID_GREY );

		mainwindow.backend.file.render_zoom_grid ( rgb, px, py, pw, ph,
			zs, gry);
	}

///	OVERLAYS

	mtPixyUI::File::ToolMode tm = mainwindow.backend.file.get_tool_mode();

	switch ( tm )
	{
	case mtPixyUI::File::TOOL_MODE_PAINT:
		{
			unsigned char opac = (unsigned char)( mainwindow.prefs.
				getDouble( PREFS_CURSOR_BRUSH_OPACITY ) * 255 );

			if ( opac > 0 && mainwindow.m_cursor.is_on_screen () )
			{
				mainwindow.backend.file.brush.render_cursor (
					mainwindow.m_cursor.x(),
					mainwindow.m_cursor.y(),
					opac, rgb, px, py, pw, ph, zs);
			}
		}
		break;

	case mtPixyUI::File::TOOL_MODE_LINING:
		mainwindow.line_overlay.render (
			mainwindow.backend.file.brush, rgb, px, py, pw, ph, zs);
		break;

	case mtPixyUI::File::TOOL_MODE_SELECTING_RECTANGLE:
	case mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE:
		mainwindow.backend.file.rectangle_overlay.render ( rgb, px, py,
			pw, ph, zs, 0 );
		break;

	case mtPixyUI::File::TOOL_MODE_SELECTING_POLYGON:
		mainwindow.backend.file.polygon_overlay.render (
			mainwindow.backend.file.brush, rgb, px, py, pw, ph, zs);
		break;

	case mtPixyUI::File::TOOL_MODE_PASTE:
	case mtPixyUI::File::TOOL_MODE_PASTING:
		mainwindow.backend.clipboard.render ( mainwindow.backend.file.
			get_image ()->get_palette ()->get_color (),
			mainwindow.backend.file.rectangle_overlay, rgb, px, py,
			pw, ph, zs );

		mainwindow.backend.file.rectangle_overlay.render ( rgb, px, py,
			pw, ph, zs, 1 );
		break;

	default:
		break;
	}

///	PAINT ONTO WIDGET

	QImage * qi = new QImage ( (const uchar *)rgb, pw, ph, pw * 3,
		QImage::Format_RGB888 );
	QPainter p ( this );

	p.drawImage ( QPoint ( px, py ), qi[0] );

///	QT OVERLAY LINES

	switch ( tm )
	{
	case mtPixyUI::File::TOOL_MODE_SELECTING_POLYGON:
	case mtPixyUI::File::TOOL_MODE_SELECTED_POLYGON:
		polygon_overlay_render ( p, mainwindow.backend.file.
			brush, zs, mainwindow.backend.file.polygon_overlay, tm);
		break;

	default:
		break;
	}

	delete qi;

	delete im;
}

