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



cairo_t * MainWindow::prepRender (
	int	const	px,
	int	const	py,
	int	const	pw,
	int	const	ph
	)
{
	if ( pw < 1 || ph < 1 || ! election )
	{
		return NULL;
	}

	int	const	w = polymapScrollArea->viewport ()->width ();
	int	const	h = polymapScrollArea->viewport ()->height ();
	cairo_t		* cr = NULL;


	election->renderPolymap ( &cr, px - w, py - h, pw, ph, zoomMap,
		NULL, NULL, comboMapMode->currentIndex (),
		comboDiagramLeft->currentText().toUtf8().data() );

	return cr;
}

void MainWindow::clickMap (
	int		xx,
	int		yy
	)
{
	int		seatnum, i;


	if ( ! election )
	{
		// No data loaded

		return;
	}

	xx -= polymapScrollArea->viewport ()->width ();
	yy -= polymapScrollArea->viewport ()->height ();

	seatnum = election->getSeatFromMap ( xx, yy, zoomMap );

	if ( seatnum < 0 )
	{
		clearSeatTable ();

		return;
	}

	if ( 0 == election->getTableValue ( seatnum, EL_TAB_SHEET_ROW, &i ) )
	{
		setupSeatTable ( i );
	}
}

void MainWindow::wheelZoom (
	int	const	xx,
	int	const	yy,
	int	const	delta
	)
{
	if ( ! polymapScrollArea || ! mapCanvas || ! sliderMapZoom )
	{
		return;
	}


	double		aw, ah, cx, cy, zd = 0, newzoom;


	aw = polymapScrollArea->viewport ()->width ();
	ah = polymapScrollArea->viewport ()->height ();

	cx = ( (double)(xx - aw) ) / (mapCanvas->width () - 2 * aw );
	cy = ( (double)(yy - ah) ) / (mapCanvas->height () - 2 * ah );

	if ( cx < 0 )
	{
		cx = 0;
	}
	else if ( cx > 1 )
	{
		cx = 1;
	}

	if ( cy < 0 )
	{
		cy = 0;
	}
	else if ( cy > 1 )
	{
		cy = 1;
	}

	zd =  zoomMap / 10;

	if ( delta < 0 )
	{
		zd = -zd;
	}

	newzoom = zoomMap + zd;

	if ( newzoom < POLYMAP_ZOOM_MIN )
	{
		newzoom = POLYMAP_ZOOM_MIN;
	}

	if ( newzoom > POLYMAP_ZOOM_MAX )
	{
		newzoom = POLYMAP_ZOOM_MAX;
	}

	if ( zoomMap == newzoom )
	{
		return;
	}


	int		w,
			h,
			nv_h = 0,
			nv_v = 0,	// New positions of scrollbar
			ox,		// Offset coord from visible area
			oy;


	w = (int)lrint ( POLYMAP_WIDTH * newzoom );
	h = (int)lrint ( POLYMAP_HEIGHT * newzoom );

	ox = xx - polymapScrollArea->horizontalScrollBar ()->value ();
	oy = yy - polymapScrollArea->verticalScrollBar ()->value ();

	zoomMap = newzoom;

	sliderMapZoom->blockSignals ( true );
	sliderMapZoom->setValue ( (int)lrint ( newzoom * 100 ) );
	sliderMapZoom->blockSignals ( false );

	mapCanvas->setGeometry ( 0, 0, (int)(w + aw * 2), (int)(h + ah * 2) );

	nv_h = (int)( rint ( w * cx - ox ) + aw );
	nv_v = (int)( rint ( h * cy - oy ) + ah );

	polymapScrollArea->horizontalScrollBar ()->setValue ( nv_h );
	polymapScrollArea->verticalScrollBar ()->setValue ( nv_v );
}

void MainWindow::moveMapFocus (
	int	const	row
	)
{
	if ( ! polymapScrollArea )
	{
		return;
	}


	double		xx, yy;


	if ( ! election || election->getPolyMinXY ( row, &xx, &yy ) )
	{
		return;
	}

	xx *= zoomMap;
	yy *= zoomMap;

	xx += polymapScrollArea->viewport ()->width ();
	yy += polymapScrollArea->viewport ()->height ();

	polymapScrollArea->horizontalScrollBar ()->setValue ( (int)xx );
	polymapScrollArea->verticalScrollBar ()->setValue ( (int)yy );
}

void MainWindow::resetMapSize ()
{
	if ( ! polymapScrollArea || ! mapCanvas )
	{
		return;
	}


	int		w, h;


	w = polymapScrollArea->viewport ()->width ();
	h = polymapScrollArea->viewport ()->height ();

	mapCanvas->setGeometry ( 0, 0,
		(int)( POLYMAP_WIDTH * zoomMap + 2 * w ),
		(int)( POLYMAP_HEIGHT * zoomMap + 2 * h ) );

	mapCanvas->update ();
}

void MainWindow::resetMapZoomPos ()
{
	if ( ! polymapScrollArea )
	{
		return;
	}


	int		w, h;


	resetMapSize ();

	w = polymapScrollArea->horizontalScrollBar ()->maximum ();
	h = polymapScrollArea->verticalScrollBar ()->maximum ();

	// This ensures the x / y map position is as I want
	polymapScrollArea->horizontalScrollBar ()->setValue ( w / 2 );
	polymapScrollArea->verticalScrollBar ()->setValue ( h / 2 );
}

void MainWindow::polymapScroll (
	int	const	dx,
	int	const	dy
	)
{
	QScrollBar * const hscroll = polymapScrollArea->horizontalScrollBar ();
	QScrollBar * const vscroll = polymapScrollArea->verticalScrollBar ();


	if ( dx )
	{
		hscroll->setValue ( dx + hscroll->value () );
	}

	if ( dy )
	{
		vscroll->setValue ( dy + vscroll->value () );
	}
}

void MainWindow::mapRedraw ()
{
	if ( ! election )
	{
		return;
	}


	unsigned char	* dest;
	unsigned char	* d1;
	int		r, xx, yy, rgb;


	if ( mapCanvas )
	{
		mapCanvas->update ();
	}

	d1 = cartogramWidget->getImage ()->get_canvas ();
	if ( ! d1 )
	{
		return;
	}

	// Clear the old rubbish
	memset ( d1, 0, CARTOGRAM_WIDTH * CARTOGRAM_HEIGHT * 3 );

	for ( r = 0; r < election->getSeats (); r++ )
	{
		if (	election->getCartogramXY ( r, &xx, &yy )	||
			election->getSeatRGB ( r, &rgb,
				comboMapMode->currentIndex (),
				comboDiagramLeft->currentText().toUtf8().data())
			)
		{
			continue;
		}

		if ( xx < 0 || xx >= CARTOGRAM_WIDTH )
		{
			continue;
		}

		if ( yy < 0 || yy >= CARTOGRAM_HEIGHT )
		{
			continue;
		}

		dest = d1 + 3 * (xx + yy * CARTOGRAM_WIDTH);
		*dest++ = (unsigned char)mtPixy::int_2_red ( rgb );
		*dest++ = (unsigned char)mtPixy::int_2_green ( rgb );
		*dest++ = (unsigned char)mtPixy::int_2_blue ( rgb );
	}

	cartogramWidget->update ();
}

void MainWindow::mapModeChanged (
	int	const	ARG_UNUSED ( i )
	)
{
	mapRedraw ();
}

