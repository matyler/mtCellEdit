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

#include "private.h"



QString mtQEX :: qstringFromC (
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
		result = QString :: fromUtf8 ( cstring, size );
	}
	else
	{
		result = QString :: fromLatin1 ( cstring, size );
	}

	return result;
}



static mtPrefTable const default_prefs_table[] = {
{ "prefs.col1",		MTKIT_PREF_TYPE_INT, "150", NULL, NULL, 0, NULL, NULL },
{ "prefs.col2",		MTKIT_PREF_TYPE_INT, "70", NULL, NULL, 0, NULL, NULL },
{ "prefs.col3",		MTKIT_PREF_TYPE_INT, "70", NULL, NULL, 0, NULL, NULL },
{ "prefs.col4",		MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },

{ "prefs.window_x",	MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
{ "prefs.window_y",	MTKIT_PREF_TYPE_INT, "50", NULL, NULL, 0, NULL, NULL },
{ "prefs.window_w",	MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },
{ "prefs.window_h",	MTKIT_PREF_TYPE_INT, "500", NULL, NULL, 0, NULL, NULL },

{ NULL, 0, NULL, NULL, NULL, 0, NULL, NULL }

	};


int mtQEX :: prefsInitPrefs (
	mtPrefs		* const	prefs
	)
{
	return mtkit_prefs_add ( prefs, default_prefs_table, 0 );
}

int mtQEX :: prefsWindowMirrorPrefs (
	mtPrefs		* const	dest,
	mtPrefs		* const	src
	)
{
	return mtkit_prefs_value_mirror ( dest, src, default_prefs_table );
}

qexArrowFilter :: qexArrowFilter (
	QGridLayout	* const	layout
	)
	:
	gridLayout	( layout )
{
}

bool qexArrowFilter :: eventFilter (
	QObject		* const	obj,
	QEvent		* const	event
	)
{
	QKeyEvent	* keyEvent = NULL;
	QWidget		* widget = NULL;
	int		idx = -1;


	if ( event->type () == QEvent::KeyPress )
	{
		keyEvent = static_cast<QKeyEvent *>(event);
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
	return QObject :: eventFilter ( obj, event );
}

