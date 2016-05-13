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

#include "private.h"



qexImage :: qexImage ()
	:
	image		(),
	zoom		( 1 )
{
	setAlignment ( Qt::AlignHCenter | Qt::AlignVCenter );

	area = new qexImageArea ( this );
	setWidget ( area );

	setBackgroundRole ( QPalette::Dark );
}

qexImage :: ~qexImage ()
{
	mtkit_image_destroy ( image );
	image = NULL;
}

void qexImage :: setImage (
	mtImage		* const	im
	)
{
	mtkit_image_destroy ( image );
	image = im;

	if ( im )
	{
		area->setGeometry ( 0, 0,
			mtkit_image_get_width ( image ) * zoom,
			mtkit_image_get_height ( image ) * zoom );
	}
	else
	{
		area->setGeometry ( 0, 0, 0, 0 );
	}

	// Note: both of these are needed to cover all scenarios
	area->update ();
	area->updateGeometry ();
}

mtImage * qexImage :: getImage ()
{
	return image;
}

int qexImage :: setZoom (
	int	const	z
	)
{
	if ( z < 1 || z > 100 )
	{
		return 1;
	}

	if ( z != zoom )
	{
		zoom = z;

		if ( image )
		{
			area->setGeometry ( 0, 0,
				mtkit_image_get_width ( image ) * zoom,
				mtkit_image_get_height ( image ) * zoom );

			area->update ();
			area->updateGeometry ();
		}
	}

	return 0;
}

int qexImage :: getZoom ()
{
	return zoom;
}

void qexImage :: update ()
{
	area->update ();
}

qexImageArea :: qexImageArea (
	qexImage	* const	parent
	)
	:
	qi		( parent )
{
	setAttribute ( Qt::WA_OpaquePaintEvent );
	setAttribute ( Qt::WA_NoSystemBackground );
}

void qexImageArea :: paintEvent (
	QPaintEvent	* const	event
	)
{
	mtImage		* destImage,
			* srcImage;
	int		px, py, pw, ph;
	unsigned char	* src, * rgb;


	px = event->rect ().x ();
	py = event->rect ().y ();
	pw = event->rect ().width ();
	ph = event->rect ().height ();

	destImage = mtkit_image_new_rgb ( pw, ph );
	rgb = mtkit_image_get_rgb ( destImage );

	if ( ! rgb )
	{
		mtkit_image_destroy ( destImage );
		return;
	}

	if (	qi						&&
		(srcImage = qi->getImage () )			&&
		(src = mtkit_image_get_rgb ( srcImage ) )
		)
	{
		unsigned char	* pix, * dest;
		int		pw2,
				ph2,
				nix = 0,
				niy = 0,
				zoom = qi->getZoom (),
				iw,
				ih,
				px2,
				py2,
				x,
				y
				;


		px2 = px;
		py2 = py;

		iw = mtkit_image_get_width ( srcImage );
		ih = mtkit_image_get_height ( srcImage );

		pw2 = pw;
		ph2 = ph;

		if ( px2 < 0 )
		{
			nix = -px2;
		}

		if ( py2 < 0 )
		{
			niy = -py2;
		}

		if ( ( px2 + pw2 ) >= iw * zoom )
		{
			// Update image + blank space outside

			pw2 = iw * zoom - px2;
		}
		if ( ( py2 + ph2 ) >= ih * zoom )
		{
			// Update image + blank space outside

			ph2 = ih * zoom - py2;
		}

		for ( y = niy; y < ph2; y++ )
		{
			dest = rgb + 3 * ( y * pw + nix );
			for ( x = nix; x < pw2; x++ )
			{
				pix = src + 3 * ( ( px2 + x ) / zoom +
					( py2 + y ) / zoom * iw );

				*dest++ = pix[0];
				*dest++ = pix[1];
				*dest++ = pix[2];
			}
		}
	}

	QImage * qim = new QImage ( (const uchar *)rgb, pw, ph, pw * 3,
		QImage::Format_RGB888 );

	if ( qim )
	{
		QPainter p ( this );
		p.drawImage ( QPoint ( px, py ), qim[0] );

		delete qim;		// Delete before rgb image destroy
	}

	mtkit_image_destroy ( destImage );
}

