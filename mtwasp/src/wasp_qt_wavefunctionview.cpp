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



WaveFunctionView::WaveFunctionView ( mtWasp::Project &project )
	:
	m_project	( project ),
	m_color_background ( 0, 0, 0, 255 ),
	m_color_axis	( 70, 80, 130, 255 ),
	m_color_lines	( 220, 220, 220, 255 )
{
}

void WaveFunctionView::paintEvent ( QPaintEvent * const	ARG_UNUSED(ev) )
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

	paint.setPen ( m_color_lines );

	QPolygon polygon;
	double yd;
	double const x2d = (double)x2;

	for ( int x = 0; x < w; x++ )
	{
		if ( m_project.get_wave_function_value ( (double)(360*x) / x2d,
			yd ) )
		{
			break;
		}

		int const y = (int)( y2 - ((yd+1) * h / 2) + 0.5 );

		polygon << QPoint ( x, y );
	}

	paint.drawPolyline ( polygon );
}

