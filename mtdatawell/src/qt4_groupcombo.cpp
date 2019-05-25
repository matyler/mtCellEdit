/*
	Copyright (C) 2018-2019 Mark Tyler

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

#include "qt4.h"



groupCombo::groupCombo (
	mtKit::Prefs		&preferences,
	QString		const	&title,
	QWidget		* const	parent
	)
	:
	QGroupBox	( title, parent ),
	prefs		( preferences ),
	m_dir_combo	( NULL )
{
	setAcceptDrops ( true );

	// Expand this box horizontally but not vertically
	setSizePolicy(QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed));

	QVBoxLayout * vbox = new QVBoxLayout ( this );

	QWidget * w = new QWidget;
	vbox->addWidget ( w );

	QHBoxLayout * gb_layh = new QHBoxLayout ( w );

	m_dir_combo = new QComboBox;
	gb_layh->addWidget ( m_dir_combo );

	m_dir_combo->setEditable ( true );
	m_dir_combo->lineEdit()->setAcceptDrops ( false );

	QPushButton * button;

	button = new QPushButton( QIcon::fromTheme ("document-open"), "Select");
	gb_layh->addWidget ( button );
	connect( button, SIGNAL(clicked()), this, SLOT( press_select()));
	button->setSizePolicy ( QSizePolicy ( QSizePolicy::Fixed,
		QSizePolicy::Fixed ) );
}

groupCombo::~groupCombo ()
{
}

void groupCombo::dragEnterEvent ( QDragEnterEvent * const event )
{
	if ( event->mimeData ()->hasUrls () )
	{
		event->acceptProposedAction ();
	}
}

void groupCombo::dropEvent ( QDropEvent * const event )
{
	QList<QUrl> const list = event->mimeData ()->urls ();

	for ( int i = 0; i < 1; i++ )
	{
		QString const &st = list.at ( i ).toLocalFile ();

		m_dir_combo->lineEdit ()->setText ( st );
	}

	event->acceptProposedAction ();
}

void groupCombo::press_select ()
{
	char const * last_dir = prefs.getString ( PREFS_LAST_DIRECTORY );

	if ( ! last_dir || last_dir[0] == 0 )
	{
		last_dir = mtkit_file_home ();
	}

	QString const filename = QFileDialog::getExistingDirectory ( this,
		"Select Output Directory", mtQEX::qstringFromC ( last_dir ),
		QFileDialog::DontUseNativeDialog );

	if ( filename.isEmpty () )
	{
		return;
	}

	m_dir_combo->lineEdit ()->setText ( filename );

	prefs.set ( PREFS_LAST_DIRECTORY, filename.toUtf8 ().data () );
}

void groupCombo::repopulate ( mtKit::RecentFile const &recent_dir )
{
	m_dir_combo->clear ();

	for ( int i = 0; i < PREFS_RECENT_DIR_TOTAL; i++ )
	{
		char const * const filename = recent_dir.get_filename ( i );

		if ( ! filename || filename[0] == 0 )
		{
			continue;
		}

		m_dir_combo->addItem ( filename );
	}
}

QString groupCombo::get_directory ()
{
	return m_dir_combo->lineEdit ()->text ();
}

