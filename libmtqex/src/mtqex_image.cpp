/*
	Copyright (C) 2013-2020 Mark Tyler

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

#include "private.h"



mtQEX::Image::Image ()
	:
	m_area		( new ImageArea ( this ) ),
	m_zoom		( 1 )
{
	setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );

	setWidget ( m_area );

	setBackgroundRole ( QPalette::Dark );
}

mtQEX::Image::~Image ()
{
}

void mtQEX::Image::setPixmap ( mtPixmap * const pixmap )
{
	m_pixmap.reset ( pixmap );

	int const w = pixy_pixmap_get_width ( pixmap );
	int const h = pixy_pixmap_get_height ( pixmap );

	m_area->setGeometry ( 0, 0, w * m_zoom, h * m_zoom );

	// Note: both of these are needed to cover all scenarios
	m_area->update ();
	m_area->updateGeometry ();
}

mtPixmap * mtQEX::Image::getPixmap ()
{
	return m_pixmap.get();
}

int mtQEX::Image::setZoom (
	int	const	z
	)
{
	if ( z < 1 || z > 100 )
	{
		return 1;
	}

	if ( z != m_zoom )
	{
		m_zoom = z;

		int const w = pixy_pixmap_get_width ( getPixmap () );
		int const h = pixy_pixmap_get_height ( getPixmap () );

		m_area->setGeometry ( 0, 0, w * m_zoom, h * m_zoom );
		m_area->update ();
		m_area->updateGeometry ();
	}

	return 0;
}

int mtQEX::Image::getZoom ()
{
	return m_zoom;
}

void mtQEX::Image::update ()
{
	m_area->update ();
}

mtQEX::ImageArea::ImageArea (
	Image		* const	par
	)
	:
	qi		( par )
{
	setAttribute ( Qt::WA_OpaquePaintEvent );
	setAttribute ( Qt::WA_NoSystemBackground );
}

void mtQEX::ImageArea::paintEvent (
	QPaintEvent	* const	ev
	)
{
	int const px = ev->rect ().x ();
	int const py = ev->rect ().y ();
	int const pw = ev->rect ().width ();
	int const ph = ev->rect ().height ();

	mtPixy::Pixmap const dest ( pixy_pixmap_new_rgb ( pw, ph ) );
	unsigned char * const rgb = pixy_pixmap_get_canvas ( dest.get() );

	if ( ! rgb )
	{
		return;
	}

	mtPixmap const * const src = qi->getPixmap();

	if ( src )
	{
		mtPalette const * const pal =pixy_pixmap_get_palette_const(src);

		pixy_pixmap_blit_rgb ( src, pal, rgb, -px, -py, pw, ph,
			qi->getZoom () );
	}

	std::unique_ptr<QImage> const qim ( new QImage ( (const uchar *)rgb, pw,
		ph, pw * 3, QImage::Format_RGB888 ) );

	if ( qim.get() )
	{
		QPainter p ( this );
		p.drawImage ( QPoint ( px, py ), *qim.get() );
	}
}

