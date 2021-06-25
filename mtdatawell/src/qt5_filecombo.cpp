/*
	Copyright (C) 2018-2020 Mark Tyler

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



fileCombo::fileCombo (
	mtKit::UserPrefs		& preferences,
	char		const * const	title,
	QFileDialog::FileMode	const	filename_type
	)
	:
	QGroupBox	( title ),
	uprefs		( preferences ),
	m_dir_combo	( new QComboBox ),
	m_title		( title ),
	m_filemode	( filename_type )
{
	setAcceptDrops ( true );

	// Expand this box horizontally but not vertically
	setSizePolicy(QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Fixed));

	QVBoxLayout * vbox = new QVBoxLayout ( this );

	QWidget * w = new QWidget;
	vbox->addWidget ( w );

	QHBoxLayout * gb_layh = new QHBoxLayout ( w );

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

void fileCombo::dragEnterEvent ( QDragEnterEvent * const event )
{
	if ( event->mimeData ()->hasUrls () )
	{
		event->acceptProposedAction ();
	}
}

void fileCombo::dropEvent ( QDropEvent * const event )
{
	QList<QUrl> const list = event->mimeData ()->urls ();

	for ( int i = 0; i < 1; i++ )
	{
		QString const &st = list.at ( i ).toLocalFile ();

		m_dir_combo->lineEdit ()->setText ( st );
	}

	event->acceptProposedAction ();
}

void fileCombo::press_select ()
{
	std::string const & dir = uprefs.get_string ( PREFS_LAST_DIRECTORY );
	char const * const last_dir = dir.size() > 0 ? dir.c_str() :
		mtkit_file_home();

	std::string const title = "Select " + m_title;

	QString filename;
	QString qlast_dir = mtQEX::qstringFromC ( last_dir );

	mtQEX::SaveFileDialog dialog ( this, title.c_str () );

	if ( QFileDialog::ExistingFile == m_filemode )
	{
		dialog.setOption ( QFileDialog::DontConfirmOverwrite );
	}

	dialog.selectFile ( qlast_dir );
	dialog.setFileMode ( m_filemode );

	if ( dialog.exec () )
	{
		QStringList fileList = dialog.selectedFiles ();
		filename = fileList.at ( 0 );
	}

	if ( filename.isEmpty () )
	{
		return;
	}

	m_dir_combo->lineEdit ()->setText ( filename );

	// Even if this is a filename, still store it as we need the directory.
	uprefs.set ( PREFS_LAST_DIRECTORY, filename.toUtf8 ().data () );
}

void fileCombo::repopulate ( mtKit::RecentFile const &recent_dir )
{
	m_dir_combo->clear ();

	for ( size_t i = 0; i < PREFS_RECENT_DIR_TOTAL; i++ )
	{
		std::string const filename ( recent_dir.filename (i+1) );

		if ( filename.empty() )
		{
			continue;
		}

		m_dir_combo->addItem ( filename.c_str() );
	}
}

QString fileCombo::get_directory ()
{
	return m_dir_combo->lineEdit ()->text ();
}

