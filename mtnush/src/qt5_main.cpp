/*
	Copyright (C) 2022-2023 Mark Tyler

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

#include "qt5.h"



int main (
	int			const	argc,
	char	const * const * const	argv
	)
{
	Cline cline;

	if ( cline.parse_command_line ( argc, argv ) )
	{
		return cline.exit.value ();
	}

	// I don't want Qt snooping or changing my command line.
	int		dummy_argc	= 1;
	char		dummy_str[1]	= { 0 };
	char		* dummy_argv	= dummy_str;
	QApplication	app ( dummy_argc, &dummy_argv );
	MainWindow	window ( cline );

	if ( 0 == cline.exit.value () )
	{
		app.exec ();
	}

	return cline.exit.value ();
}



#define ADD_CALCULATOR_TAB( TYPE_LABEL, TYPE_NAME )	\
							\
	tab = new QWidget;				\
	tabWidget->addTab ( tab, TYPE_LABEL );		\
							\
	vbox = new QVBoxLayout;				\
	tab->setLayout ( vbox );			\
							\
	w = new QWidget;				\
	vbox->addWidget ( w );				\
							\
	grid = new QGridLayout;				\
	w->setLayout ( grid );				\
							\
	groupBox = new QGroupBox ( "Input" );		\
	grid->addWidget ( groupBox, 0, 0 );		\
							\
	sgvb = new QVBoxLayout;				\
	groupBox->setLayout ( sgvb );			\
							\
	m_text_ ## TYPE_NAME ## _input = new QPlainTextEdit; \
	sgvb->addWidget ( m_text_ ## TYPE_NAME ## _input ); \
							\
	groupBox = new QGroupBox ( "Output" );		\
	m_group_box_ ## TYPE_NAME  = groupBox;		\
	grid->addWidget ( groupBox, 0, 1 );		\
							\
	sgvb = new QVBoxLayout;				\
	groupBox->setLayout ( sgvb );			\
							\
	m_text_ ## TYPE_NAME ## _output = new QPlainTextEdit; \
	sgvb->addWidget ( m_text_ ## TYPE_NAME ## _output ); \
	m_text_ ## TYPE_NAME ## _output->setReadOnly ( true ); \
							\
/* Buttons ---------------------------------------------------------------- */ \
							\
	w = new QWidget;				\
	vbox->addWidget ( w );				\
							\
	hbox = new QHBoxLayout;				\
	w->setLayout ( hbox );				\
							\
	button = new QPushButton ( QIcon::fromTheme ( "view-refresh" ), \
		"&Evaluate" );				\
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred ); \
	hbox->addWidget ( button );			\
	connect ( button, SIGNAL ( clicked () ), this,	\
		SLOT ( press_evaluate_ ## TYPE_NAME () ) ); \
							\
	button = new QPushButton ( "Show Variables" );	\
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred ); \
	hbox->addWidget ( button );			\
	connect ( button, SIGNAL ( clicked () ), this,	\
		SLOT ( press_show_ ## TYPE_NAME ## _vars () ) ); \
							\
	button = new QPushButton ( "Show Functions" );	\
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred ); \
	hbox->addWidget ( button );			\
	connect ( button, SIGNAL ( clicked () ), this,	\
		SLOT ( press_show_ ## TYPE_NAME ## _funcs () ) ); \
							\
	button = new QPushButton ( QIcon::fromTheme ( "edit-copy" ), \
		"Copy Output" );			\
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred ); \
	hbox->addWidget ( button );			\
	connect ( button, SIGNAL ( clicked () ), this,	\
		SLOT ( press_copy_ ## TYPE_NAME ## _output () ) ); \
							\
	button = new QPushButton ( QIcon::fromTheme ( "document-save" ), \
		"Save Output" );			\
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred ); \
	hbox->addWidget ( button );			\
	connect ( button, SIGNAL ( clicked () ), this,	\
		SLOT ( press_save_ ## TYPE_NAME ## _output () ) ); \
							\
	/* Squashes buttons together to the left */	\
	hbox->addWidget ( new QWidget );		\
							\



MainWindow::MainWindow ( Cline & cline )
	:
	uprefs		( cline.uprefs ),
	mprefs		( cline.mprefs )
{
	QPushButton	* button;
	QGroupBox	* groupBox;
	QHBoxLayout	* hbox;
	QVBoxLayout	* vbox;
	QVBoxLayout	* sgvb;
	QWidget		* tab, * w;
	QGridLayout	* grid;

	setWindowTitle ( VERSION );

	std::string path;
	mtKit::get_data_dir ( path, DATA_INSTALL "/icons/hicolor/scalable/apps/"
		BIN_NAME ".svg" );
	setWindowIcon ( QIcon ( path.c_str () ) );

	QWidget * widget = new QWidget;
	setCentralWidget ( widget );

	create_menu ();

	QVBoxLayout * vbox_main = new QVBoxLayout;
	widget->setLayout ( vbox_main );

	QTabWidget * tabWidget = new QTabWidget;
	vbox_main->addWidget ( tabWidget );

///	Calculator tabs --------------------------------------------------------

	ADD_CALCULATOR_TAB( "Float", float )
	ADD_CALCULATOR_TAB( "Integer", int )
	ADD_CALCULATOR_TAB( "Rational", rational )

/// ----------------------------------------------------------------------------

	setMinimumSize ( 160, 160 );
	setGeometry ( mprefs.window_x, mprefs.window_y,
		mprefs.window_w, mprefs.window_h );

	if ( 1 == mprefs.window_maximized )
	{
		showMaximized ();
	}
	else
	{
		show ();
	}
}

MainWindow::~MainWindow ()
{
	if ( isMaximized () )
	{
		uprefs.set ( PREFS_WINDOW_MAXIMIZED, 1 );
	}
	else
	{
		uprefs.set ( PREFS_WINDOW_MAXIMIZED, 0 );

		uprefs.set ( PREFS_WINDOW_X, geometry().x () );
		uprefs.set ( PREFS_WINDOW_Y, geometry().y () );
		uprefs.set ( PREFS_WINDOW_W, geometry().width () );
		uprefs.set ( PREFS_WINDOW_H, geometry().height () );
	}
}

