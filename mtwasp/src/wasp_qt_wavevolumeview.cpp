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

#include "wasp_qt.h"



enum
{
	NODE_SIZE	= 16,
	NODE_SIZE_2	= 8
};




WaveVolumeView::WaveVolumeView (
	MainWindow	&mw,
	mtWasp::Project	&project
	)
	:
	m_mainwindow	( mw ),
	m_project	( project ),
	m_color_background ( 0, 0, 0, 255 ),
	m_color_axis	( 220, 220, 220, 255 ),
	m_color_area	( 200, 0, 0, 160 ),
	m_color_nodes	( 200, 140, 140, 160 )
{
	setMouseTracking ( true );

	auto create_vol_spin = [this]( double min, double max )
	{
		QDoubleSpinBox * spin = new QDoubleSpinBox;

		spin->setDecimals ( 3 );
		spin->setRange ( min, max );
		spin->setSingleStep ( m_project.WAVE_VOLUME_SINGLE_STEP () );

		connect ( spin, SIGNAL ( valueChanged (double) ), this,
			SLOT ( changed_spin_volume (double) ) );

		return spin;
	};

	m_spin_p1x = create_vol_spin (
		m_project.WAVE_VOLUME_X_MIN (),
		m_project.WAVE_VOLUME_X_MAX () );

	m_spin_p1y = create_vol_spin (
		m_project.WAVE_VOLUME_Y_MIN (),
		m_project.WAVE_VOLUME_Y_MAX () );

	m_spin_p2x = create_vol_spin (
		m_project.WAVE_VOLUME_X_MIN (),
		m_project.WAVE_VOLUME_X_MAX () );

	m_spin_p2y = create_vol_spin (
		m_project.WAVE_VOLUME_Y_MIN (),
		m_project.WAVE_VOLUME_Y_MAX () );

	load_values_from_project ();
}

void WaveVolumeView::changed_spin_volume ( double const ARG_UNUSED(x) )
{
	double		op1x, op1y, op2x, op2y;

	m_project.get_wave_volume_points ( op1x, op1y, op2x, op2y );

	double		p1x = m_spin_p1x->value ();
	double		p1y = m_spin_p1y->value ();
	double		p2x = m_spin_p2x->value ();
	double		p2y = m_spin_p2y->value ();

	// Ensure p1x <= p2x
	if ( p1x > p2x )
	{
		// Work out if p1 or p2 was changed and drag the other along
		if ( p1x != op1x || p1y != op1y )
		{
			p2x = p1x;
		}
		else
		{
			p1x = p2x;
		}
	}

	// Update the back end numbers, update the volume widgets

	m_project.set_wave_volume_points ( p1x, p1y, p2x, p2y );

	load_values_from_project ();
}

void WaveVolumeView::paintEvent ( QPaintEvent * const ARG_UNUSED(ev) )
{
	int	const	w = this->width ();
	int	const	h = this->height ();
	int	const	x2 = w-1;
	int	const	y2 = h-1;

	QPainter	paint ( this );

	paint.fillRect ( 0, 0, w, h, m_color_background );

	paint.setPen ( m_color_axis );

	paint.drawLine ( 0, (int)(h*0.25+0.5), x2, (int)(h*0.25+0.5) );
	paint.drawLine ( 0, (int)(h*0.50+0.5), x2, (int)(h*0.50+0.5) );
	paint.drawLine ( 0, (int)(h*0.75+0.5), x2, (int)(h*0.75+0.5) );

	paint.drawLine ( (int)(w*0.25+0.5), 0, (int)(w*0.25+0.5), y2 );
	paint.drawLine ( (int)(w*0.50+0.5), 0, (int)(w*0.50+0.5), y2 );
	paint.drawLine ( (int)(w*0.75+0.5), 0, (int)(w*0.75+0.5), y2 );

	double vx1, vy1, vx2, vy2;

	m_project.get_wave_volume_points ( vx1, vy1, vx2, vy2 );

	int const node_x1 = (int)(x2*vx1 + 0.5);
	int const node_y1 = (int)(y2 - y2*vy1 + 0.5);
	int const node_x2 = (int)(x2*vx2 + 0.5);
	int const node_y2 = (int)(y2 - y2*vy2 + 0.5);

	QPointF points[4] = {
		QPointF ( 0, y2 ),
		QPointF ( node_x1, node_y1 ),
		QPointF ( node_x2, node_y2 ),
		QPointF ( x2, y2 )
	};

	paint.setPen ( Qt::NoPen );
	paint.setBrush ( m_color_area );
	paint.drawConvexPolygon ( points, 4 );

	paint.setPen ( Qt::NoPen );
	paint.setBrush ( m_color_nodes );
	paint.drawEllipse ( node_x1 - NODE_SIZE_2, node_y1 - NODE_SIZE_2,
		NODE_SIZE, NODE_SIZE );
	paint.drawEllipse ( node_x2 - NODE_SIZE_2, node_y2 - NODE_SIZE_2,
		NODE_SIZE, NODE_SIZE );
}

void WaveVolumeView::mousePressEvent ( QMouseEvent * const ev )
{
	if ( ev->buttons () & Qt::LeftButton )
	{
		int	const	max_x = this->width() - 1;
		int	const	max_y = this->height() - 1;
		int	const	x = mtkit_int_bound ( ev->x (), 0, max_x );
		int	const	y = mtkit_int_bound ( max_y - ev->y (), 0, max_y );

		double	const	xp = (double)x / max_x;
		double	const	yp = (double)y / max_y;

		double		p1x, p1y, p2x, p2y;

		m_project.get_wave_volume_points ( p1x, p1y, p2x, p2y );

		double	const	p1d = (xp-p1x)*(xp-p1x) + (yp-p1y)*(yp-p1y);
		double	const	p2d = (xp-p2x)*(xp-p2x) + (yp-p2y)*(yp-p2y);

		// Was the click closer to node 1 or 2?

		if ( p1d <= p2d )
		{
			m_drag_node = 1;
		}
		else
		{
			m_drag_node = 2;
		}

		// Get the node to move in the widget
		mouseMoveEvent ( ev );
	}
}

void WaveVolumeView::mouseMoveEvent ( QMouseEvent * const ev )
{
	if ( ev->buttons () & Qt::LeftButton )
	{
		int	const	max_x = this->width() - 1;
		int	const	max_y = this->height() - 1;
		int	const	x = mtkit_int_bound ( ev->x (), 0, max_x );
		int	const	y = mtkit_int_bound ( max_y - ev->y (), 0, max_y );

		double	const	xp = (double)x / max_x;
		double	const	yp = (double)y / max_y;

		double		p1x, p1y, p2x, p2y;

		m_project.get_wave_volume_points ( p1x, p1y, p2x, p2y );

		int updated = 0;

		if ( 1 == m_drag_node )
		{
			p1x = xp;
			p1y = yp;

			if ( p1x > p2x )
			{
				p2x = p1x;
			}

			updated = 1;
		}
		else if ( 2 == m_drag_node )
		{
			p2x = xp;
			p2y = yp;

			if ( p1x > p2x )
			{
				p1x = p2x;
			}

			updated = 1;
		}

		if ( updated )
		{
			m_project.set_wave_volume_points ( p1x, p1y, p2x, p2y );

			load_values_from_project ();
		}
	}
}

void WaveVolumeView::mouseReleaseEvent ( QMouseEvent * const ev )
{
	if ( ev->button () & Qt::LeftButton )
	{
		m_drag_node = 0;
	}
}

void WaveVolumeView::load_values_from_project ()
{
	double x1, y1, x2, y2;

	m_project.get_wave_volume_points ( x1, y1, x2, y2 );

	set_volume_spin_values ( x1, y1, x2, y2 );

	this->update();
}

void WaveVolumeView::set_volume_spin_values (
	double	const	p1x,
	double	const	p1y,
	double	const	p2x,
	double	const	p2y
	)
{
	auto set_signal_block = [this]( bool state )
	{
		m_spin_p1x->blockSignals ( state );
		m_spin_p1y->blockSignals ( state );
		m_spin_p2x->blockSignals ( state );
		m_spin_p2y->blockSignals ( state );
	};

	set_signal_block ( true );

	m_spin_p1x->setValue ( p1x );
	m_spin_p1y->setValue ( p1y );
	m_spin_p2x->setValue ( p2x );
	m_spin_p2y->setValue ( p2y );

	set_signal_block ( false );

	m_mainwindow.update_titlebar ();
}

