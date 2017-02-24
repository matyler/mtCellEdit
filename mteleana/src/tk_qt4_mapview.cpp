/*
	Copyright (C) 2013-2016 Mark Tyler

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




MapCanvas::MapCanvas ()
	:
	oldMouseX	( 0 ),
	oldMouseY	( 0 )
{
	setAttribute ( Qt::WA_OpaquePaintEvent );
	setAttribute ( Qt::WA_NoSystemBackground );
}

void MapCanvas::paintEvent (
	QPaintEvent	* const	ev
	)
{
	int		px, py, pw, ph, format, stride;
	unsigned char	* rgb;
	cairo_t		* cr;
	cairo_surface_t	* cr_surface;


	px = ev->rect ().x ();
	py = ev->rect ().y ();
	pw = ev->rect ().width ();
	ph = ev->rect ().height ();

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

void MapCanvas::mouseEventRouter (
	QMouseEvent	* const	ev,
	int		const	caller	// 0 = Press 2 = Move
	)
{
	int		xx, yy;


	if (	! ( ev->buttons () & Qt::LeftButton ) &&
		! ( ev->buttons () & Qt::RightButton )
		)
	{
		// We only look at left/right clicks/drags

		return;
	}

	xx = ev->x ();
	yy = ev->y ();

	switch ( caller )
	{
	case 0:	// Mouse press
		oldMouseX = xx;
		oldMouseY = yy;

		if ( ev->buttons () & Qt::RightButton )
		{
			mainwindow->clickMap ( xx, yy );

			return;
		}

		break;

	case 2:	// Mouse movement
		if ( ev->buttons () & Qt::LeftButton )
		{
			int	const	dx = oldMouseX - xx;
			int	const	dy = oldMouseY - yy;


			mainwindow->polymapScroll ( dx, dy );
		}
		break;
	}
}

void MapCanvas::mousePressEvent (
	QMouseEvent	* const	ev
	)
{
	mouseEventRouter ( ev, 0 );
}

void MapCanvas::mouseMoveEvent (
	QMouseEvent	* const	ev
	)
{
	mouseEventRouter ( ev, 2 );
}

void MapCanvas::wheelEvent (
	QWheelEvent	* const	ev
	)
{
	if ( ev->orientation () == Qt::Horizontal )
	{
		ev->ignore ();

		return;
	}

	mainwindow->wheelZoom ( ev->x (), ev->y (), ev->delta () );
}

void MapView::resizeEvent (
	QResizeEvent	* const	ARG_UNUSED ( ev )
	)
{
	mainwindow->resetMapSize ();
}

