/*
	Copyright (C) 2013-2015 Mark Tyler

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

#include "tk_qt4.h"




MapCanvas :: MapCanvas ()
	:
	oldMouseX	( 0 ),
	oldMouseY	( 0 )
{
	setAttribute ( Qt::WA_OpaquePaintEvent );
	setAttribute ( Qt::WA_NoSystemBackground );
}

void MapCanvas :: paintEvent (
	QPaintEvent	* const	event
	)
{
	int		px, py, pw, ph, format, stride;
	unsigned char	* rgb;
	cairo_t		* cr;
	cairo_surface_t	* cr_surface;


	px = event->rect ().x ();
	py = event->rect ().y ();
	pw = event->rect ().width ();
	ph = event->rect ().height ();

//printf("%i,%i wh %i,%i\n", px,py, pw,ph );

	cr = mainwindow->prepRender ( px, py, pw, ph );
	if ( ! cr )
	{
		return;
	}

	cr_surface = cairo_get_target ( cr );
	if ( ! cr_surface )
	{
		goto finish;
	}

	rgb = cairo_image_surface_get_data ( cr_surface );
	if ( ! rgb )
	{
		goto finish;
	}

	format = cairo_image_surface_get_format ( cr_surface );
	if (	format != CAIRO_FORMAT_ARGB32 &&
		format != CAIRO_FORMAT_RGB24
		)
	{
		goto finish;
	}

	stride = cairo_image_surface_get_stride ( cr_surface );

	{
		QImage * im = new QImage ( (const uchar *)rgb, pw, ph, stride,
			QImage::Format_ARGB32 );

		QPainter p ( this );
		p.drawImage ( QPoint ( px, py ), im[0] );

		delete im;		// Delete before rgb image destroy
	}

finish:
	cairo_destroy ( cr );
	cairo_surface_destroy ( cr_surface );
}

void MapCanvas :: mouseEventRouter (
	QMouseEvent	* const	event,
	int		const	caller	// 0 = Press 2 = Move
	)
{
	int		x, y;


	if (	! ( event->buttons () & Qt::LeftButton ) &&
		! ( event->buttons () & Qt::RightButton )
		)
	{
		// We only look at left/right clicks/drags

		return;
	}

	x = event->x ();
	y = event->y ();

	switch ( caller )
	{
	case 0:	// Mouse press
		oldMouseX = x;
		oldMouseY = y;

		if ( event->buttons () & Qt::RightButton )
		{
			mainwindow->clickMap ( x, y );

			return;
		}

		break;

	case 2:	// Mouse movement
		if ( event->buttons () & Qt::LeftButton )
		{
			int	const	dx = oldMouseX - x;
			int	const	dy = oldMouseY - y;


			mainwindow->polymapScroll ( dx, dy );
		}
		break;
	}
}

void MapCanvas :: mousePressEvent (
	QMouseEvent	* const	event
	)
{
	mouseEventRouter ( event, 0 );
}

void MapCanvas :: mouseMoveEvent (
	QMouseEvent	* const	event
	)
{
	mouseEventRouter ( event, 2 );
}

void MapCanvas :: wheelEvent (
	QWheelEvent	* const	event
	)
{
	if ( event->orientation () == Qt::Horizontal )
	{
		event->ignore ();

		return;
	}

	mainwindow->wheelZoom ( event->x (), event->y (), event->delta () );
}

void MapView :: resizeEvent (
	QResizeEvent	* const	ARG_UNUSED ( event )
	)
{
	mainwindow->resetMapSize ();
}

