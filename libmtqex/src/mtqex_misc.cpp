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



QString mtQEX::qstringFromC (
	char	const	* const	cstring,
	int			size
	)
{
	QString		result;


	if ( size < 0 )
	{
		size = -1;
	}

	if ( ! cstring )
	{
		result = "";
	}
	else if ( mtkit_utf8_string_legal ( (unsigned char const *)cstring,
		size < 0 ? 0 : (size_t)size ) )
	{
		result = QString::fromUtf8 ( cstring, size );
	}
	else
	{
		result = QString::fromLatin1 ( cstring, size );
	}

	return result;
}

mtQEX::ArrowFilter::ArrowFilter (
	QGridLayout	* const	layout
	)
	:
	gridLayout	( layout )
{
}

bool mtQEX::ArrowFilter::eventFilter (
	QObject		* const	obj,
	QEvent		* const	ev
	)
{
	QKeyEvent	* keyEvent = NULL;
	QWidget		* widget = NULL;
	int		idx = -1;


	if ( ev->type () == QEvent::KeyPress )
	{
		keyEvent = static_cast<QKeyEvent *>(ev);
		widget = qobject_cast<QWidget *>(obj);
		idx = gridLayout->indexOf ( widget );
	}

	if ( idx >= 0 )
	{
		int		dr = 0,
				dc = 0,
				r = 0,
				c = 0,
				j;


		switch ( keyEvent->key () )
		{
		case Qt::Key_Up:	dr = -1;	break;
		case Qt::Key_Down:	dr = 1;		break;
		case Qt::Key_Left:	dc = -1;	break;
		case Qt::Key_Right:	dc = 1;		break;
		}

		if ( dr || dc )
		{
			QLayoutItem	* item;


			gridLayout->getItemPosition ( idx, &r, &c, &j, &j );

			r += dr;
			c += dc;

			item = gridLayout->itemAtPosition ( r, c );

			if ( item && item->widget () )
			{
				item->widget ()->setFocus (
					Qt::OtherFocusReason );
			}

			return true;
		}
	}

	// standard event processing
	return QObject::eventFilter ( obj, ev );
}

int mtQEX::qt_set_state (
	mtKit::Prefs	* const	pr,
	char	const	* const	key,
	QByteArray	* const	qb
	)
{
	int	const	l = qb->size ();


	if ( l < 1 )
	{
		return 1;
	}

	char * txt = (char *)malloc ( (size_t)(l) * 2 + 1 );
	if ( ! txt )
	{
		return 1;
	}


	char	const	* const hex = "0123456789abcdef";
	char	const	*	src = qb->constData ();
	char		*	dest = txt;


	for ( int i = 0; i < l; i++, src++ )
	{
		*dest++ = hex[ src[0] & 15 ];
		*dest++ = hex[ (src[0] >> 4) & 15 ];
	}

	dest[0] = 0;

	pr->set ( key, txt );
	free ( txt );

	return 0;
}

static int read_hex ( char const ch )
{
	if ( ch >= '0' && ch <= '9' )
	{
		return ( ch - '0' );
	}

	if ( tolower ( ch ) >= 'a' && tolower ( ch ) <= 'f' )
	{
		return ( 10 + (tolower ( ch ) - 'a') );
	}

	return -1;
}

int mtQEX::qt_get_state (
	mtKit::Prefs	* const	pr,
	char	const	* const	key,
	QByteArray	* const	qb
	)
{
	char	const	* const txt = pr->getString ( key );


	if ( ! txt )
	{
		return 1;
	}

	qb->clear ();


	size_t		const	l = strlen ( txt ) / 2;
	char	const	*	src = txt;
	int			a, b;


	for ( size_t i = 0; i < l; i++ )
	{
		a = read_hex ( *src++ );
		if ( a < 0 )
		{
			return 1;
		}

		b = read_hex ( *src++ );
		if ( b < 0 )
		{
			return 1;
		}

		qb->append ( (char)( a + (b<<4) ) );
	}

	return 0;
}

QPixmap * mtQEX::qpixmap_from_pixyimage (
	mtPixy::Image	* i
	)
{
	if ( ! i || ! i->get_canvas () )
	{
		return NULL;
	}


	QPixmap		* pixmap;
	QImage		* qi = new QImage ( (const uchar *)i->get_canvas (),
				i->get_width (), i->get_height (),
				i->get_width () * 3, QImage::Format_RGB888 );


	pixmap = new QPixmap;
	if ( pixmap )
	{
		pixmap->convertFromImage ( *qi );
	}

	delete qi;

	return pixmap;
}

mtPixy::Image * mtQEX::pixyimage_from_qpixmap (
	QPixmap		* const	pm
	)
{
	if ( ! pm )
	{
		return NULL;
	}


	int	const	w = pm->width ();
	int	const	h = pm->height ();
	mtPixy::Image	* im = image_create ( mtPixy::Image::RGB, w, h );
	if ( ! im )
	{
		return NULL;
	}

	unsigned char * const dst = im->get_canvas ();
	if ( ! dst )
	{
		delete im;
		return NULL;
	}


	QImage		qi = pm->toImage ();
	QRgb	const	* s;
	int		x, y;
	unsigned char	* d;


	for ( y = 0; y < h; y++ )
	{
		s = (QRgb const *)qi.constScanLine ( y );
		if ( ! s )
		{
			break;
		}

		d = dst + 3 * w * y;

		for ( x = 0; x < w; x++ )
		{
			*d++ = (unsigned char)qRed ( s[0] );
			*d++ = (unsigned char)qGreen ( s[0] );
			*d++ = (unsigned char)qBlue ( s[0] );

			s++;
		}
	}

	return im;
}

