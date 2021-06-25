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
	int		idx = -1;


	if ( ev->type () == QEvent::KeyPress )
	{
		keyEvent = static_cast<QKeyEvent *>(ev);
		QWidget * widget = qobject_cast<QWidget *>(obj);
		idx = gridLayout->indexOf ( widget );
	}

	if ( idx >= 0 )
	{
		int dr = 0;
		int dc = 0;
		int r = 0;
		int c = 0;


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
			int		j;


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
	mtKit::UserPrefs	& prefs,
	char	const * const	key,
	QByteArray		& qba
	)
{
	int	const	len = qba.size ();
	char	const	* const hex = "0123456789abcdef";
	char	const	* const	src = qba.constData ();

	std::string	out;

	for ( int i = 0; i < len; i++ )
	{
		out += hex[ src[i] & 15 ];
		out += hex[ (src[i] >> 4) & 15 ];
	}

	prefs.set ( key, out );

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
	mtKit::UserPrefs	& prefs,
	char	const * const	key,
	QByteArray		& qba
	)
{
	std::string	const	& str = prefs.get_string ( key );
	char	const * const	txt = str.c_str ();


	if ( txt[0] == 0 )
	{
		return 1;
	}

	qba.clear ();


	size_t	const	len = str.size () / 2;
	char	const	* src = txt;


	for ( size_t i = 0; i < len; i++ )
	{
		int const a = read_hex ( *src++ );
		if ( a < 0 )
		{
			return 1;
		}

		int const b = read_hex ( *src++ );
		if ( b < 0 )
		{
			return 1;
		}

		qba.append ( (char)( a + (b<<4) ) );
	}

	return 0;
}

QPixmap * mtQEX::qpixmap_from_pixypixmap (
	mtPixmap const * const pm
	)
{
	if ( ! pm )
	{
		return NULL;
	}

	std::unique_ptr<QImage> const qi ( new QImage (
		(uchar const *)pixy_pixmap_get_canvas ( pm ),
		pixy_pixmap_get_width ( pm ), pixy_pixmap_get_height ( pm ),
		pixy_pixmap_get_width ( pm ) * 3, QImage::Format_RGB888 ) );

	QPixmap * const qpixmap = new QPixmap;
	qpixmap->convertFromImage ( *qi.get() );

	return qpixmap;
}

mtPixmap * mtQEX::pixypixmap_from_qpixmap (
	QPixmap	 const * const	pm
	)
{
	if ( ! pm )
	{
		return NULL;
	}

	int	const	w = pm->width ();
	int	const	h = pm->height ();

	mtPixy::Pixmap im ( pixy_pixmap_new_rgb ( w, h ) );
	if ( ! im.get() )
	{
		return NULL;
	}

	unsigned char * const dst = pixy_pixmap_get_canvas ( im.get() );
	if ( ! dst )
	{
		return NULL;
	}

	QImage		qi = pm->toImage ();

	for ( int y = 0; y < h; y++ )
	{
		QRgb	const	* s = (QRgb const *)qi.constScanLine ( y );
		if ( ! s )
		{
			break;
		}

		unsigned char	* d = dst + 3 * w * y;

		for ( int x = 0; x < w; x++ )
		{
			*d++ = (unsigned char)qRed ( s[0] );
			*d++ = (unsigned char)qGreen ( s[0] );
			*d++ = (unsigned char)qBlue ( s[0] );

			s++;
		}
	}

	return im.release();
}

QAction * mtQEX::menu_init (
	QObject		* const	parent,
	char	const	* const	txt,
	char	const	* const	shcut,
	char	const	* const	icon
	)
{
	QAction * act;

	if ( icon )
	{
		act = new QAction ( QIcon::fromTheme ( icon ), txt, parent );
		act->setIconVisibleInMenu ( true );
	}
	else
	{
		act = new QAction ( txt, parent );
	}

	if ( shcut )
	{
		act->setShortcut ( QString ( shcut ) );
	}

	return act;
}

void mtQEX::process_qt_pending ()
{
	QCoreApplication::processEvents ();
}

