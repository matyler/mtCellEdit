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



mtQEX::ButtonMenu::ButtonMenu ()
	:
	itemCurrent	( -1 )
{
	setMenu ( new QMenu );

	signalMapper = new QSignalMapper ( this );
	connect ( signalMapper, SIGNAL ( mapped ( int ) ),
		this, SLOT ( optionClicked ( int ) ) );

	setSizePolicy ( QSizePolicy ( QSizePolicy::Maximum,
		QSizePolicy::Preferred ) );

	setMinimumSize ( minimumSizeHint ().width (), 0 );

/*
	I disconnect the original Qt signal so I can pre-select the current
	menu item via the popup slot.
*/
	disconnect ( this, SIGNAL ( pressed () ), 0, 0 );
	connect ( this, SIGNAL ( pressed () ), this, SLOT ( popup () ) );
}

mtQEX::ButtonMenu::~ButtonMenu ()
{
	QMenu * m = menu();

	setMenu ( NULL );

	delete m;
}

void mtQEX::ButtonMenu::keyPressEvent (
	QKeyEvent	* const	ev
	)
{
	switch ( ev->key () )
	{
	case Qt::Key_Up:
		setCurrentIndex ( itemCurrent - 1 );
		return;

	case Qt::Key_Down:
		setCurrentIndex ( itemCurrent + 1 );
		return;
	}

	// Nothing to do so let the base class handle this event instead
	QPushButton::keyPressEvent ( ev );
}

void mtQEX::ButtonMenu::setCurrentIndex (
	int	const	i
	)
{
	QList<QAction*>	actionList = menu()->actions ();
	QAction		* action = actionList.value ( i, NULL );


	if ( ! action )
	{
		return;			// Unable to change
	}

	itemCurrent = i;
	setText ( action->text () );

	currentIndexChanged ( i );
}

void mtQEX::ButtonMenu::popup ()
{
	QList<QAction*>	actionList = menu()->actions ();


	menu()->setActiveAction ( actionList.value ( itemCurrent, NULL ) );

#ifdef U_TK_QT5
	// Hack to stop Qt5 displaying wrong decoration (GTK+ prelight) after
	// the user closes the menu with the mouse elsewhere.
	setAttribute ( Qt::WA_UnderMouse, false );
#endif

	showMenu ();
}

int mtQEX::ButtonMenu::count ()
{
	QList<QAction*>	actionList = menu()->actions ();


	return actionList.size ();
}

int mtQEX::ButtonMenu::findText ( QString t )
{
	QList<QAction*>	actionList = menu()->actions ();
	QAction		* action;


	t.replace ( "&", "&&" );

	for ( int i = 0; i < actionList.size (); i++ )
	{
		action = actionList.value ( i, NULL );

		if ( action && action->text () == t )
		{
			return i;
		}
	}

	return -1;			// Not found
}

/*
We need text() to reverse the & replacements we do in addItem to avoid Qt
thinking & is used as a shortcut.
*/
QString mtQEX::ButtonMenu::text ()
{
	return QPushButton::text().replace ( "&&", "&" );
}

void mtQEX::ButtonMenu::optionClicked (
	int	const	i
	)
{
	setCurrentIndex ( i );
}

void mtQEX::ButtonMenu::addItem (
	QString		t
	)
{
	QAction * action = menu()->addAction ( t.replace ( "&", "&&" ) );


	if ( ! action )
	{
		return;
	}

	if ( -1 == itemCurrent )
	{
		setCurrentIndex ( 0 );
	}

	connect ( action, SIGNAL( triggered ()), signalMapper, SLOT( map () ) );
	signalMapper->setMapping ( action, count () - 1 );
}

int mtQEX::ButtonMenu::currentIndex ()
{
	return itemCurrent;
}

void mtQEX::ButtonMenu::clear ()
{
	itemCurrent = -1;

	menu()->clear ();

	setText ( "" );
}

