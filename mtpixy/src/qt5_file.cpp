/*
	Copyright (C) 2016-2023 Mark Tyler

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



void Mainwindow::press_file_new ()
{
	if ( ! ok_to_lose_changes () )
	{
		return;
	}

	mtPixmap const * const	clip_im = backend.clipboard.get_pixmap ();
	DialogImageNew		dialog ( this,
		mprefs.file_new_width,
		mprefs.file_new_height,
		mprefs.file_new_type,
		clip_im ? true : false );


	if ( dialog.exec () != QDialog::Accepted )
	{
		return;
	}

	backend.uprefs.set ( PREFS_FILE_NEW_WIDTH, dialog.get_width () );
	backend.uprefs.set ( PREFS_FILE_NEW_HEIGHT, dialog.get_height () );

	int	const	t = dialog.get_type ();

	if ( t < 0 && clip_im )
	{
		project_new ( pixy_pixmap_duplicate ( clip_im ) );
	}
	else
	{
		backend.uprefs.set ( PREFS_FILE_NEW_TYPE, t );
		project_new ();
	}
}

void Mainwindow::press_file_open ()
{
	if ( ! ok_to_lose_changes () )
	{
		return;
	}

	QString filename = QFileDialog::getOpenFileName ( this,
		"Load Image File", mtQEX::qstringFromC (
			backend.mprefs.recent_image.filename().c_str() ),
		NULL, NULL, QFileDialog::DontUseNativeDialog );


	if ( ! filename.isEmpty () )
	{
		project_load ( filename.toUtf8 ().data () );
	}
}

void Mainwindow::press_file_save ()
{
	char const * const filename = backend.file.get_filename ();

	if ( NULL == filename || 0 == filename[0] )
	{
		press_file_save_as ();
		return;
	}

	project_save ( filename, backend.file.get_filetype () );
}

void Mainwindow::press_file_save_as ()
{
	int		ft_item = 0;
	int		ftnow = backend.file.get_filetype ();
	QStringList	list_qs;
	int		list_ft_tot = 0;
	int		list_ft[ PIXY_FILE_TYPE_MAX ] = { 0 };


	if (	ftnow < PIXY_FILE_TYPE_MIN ||
		ftnow > PIXY_FILE_TYPE_MAX
		)
	{
		ftnow = PIXY_FILE_TYPE_PNG;
	}

	// Populate the table according to the image type
	if ( pixy_pixmap_get_bytes_per_pixel ( backend.file.get_pixmap() )
		== PIXY_PIXMAP_BPP_RGB )
	{
		list_ft [ list_ft_tot++ ] = PIXY_FILE_TYPE_BMP;
		list_ft [ list_ft_tot++ ] = PIXY_FILE_TYPE_JPEG;

		list_ft [ list_ft_tot++ ] = PIXY_FILE_TYPE_PNG;
		ft_item = list_ft_tot - 1;	// Default to PNG
	}
	else	// Indexed
	{
		list_ft [ list_ft_tot++ ] = PIXY_FILE_TYPE_BMP;
		list_ft [ list_ft_tot++ ] = PIXY_FILE_TYPE_GIF;

		list_ft [ list_ft_tot++ ] = PIXY_FILE_TYPE_PNG;
		ft_item = list_ft_tot - 1;	// Default to PNG
	}

	// Populate the text list and set the current format
	for ( int i = 0; i < list_ft_tot; i++ )
	{
		list_qs << pixy_file_type_text ( list_ft[i] );

		if ( ftnow == list_ft[i] )
		{
			ft_item = i;
		}
	}

	mtQEX::SaveFileDialog dialog ( this, "Save Image File", list_qs,
		ft_item, backend.file.get_filename () );

	dialog.setOption ( QFileDialog::DontConfirmOverwrite );

	if ( ! backend.file.get_filename () )
	{
		std::string const last_dir ( backend.get_last_directory () );

		dialog.setDirectory ( mtQEX::qstringFromC ( last_dir.c_str()) );
	}

	// Loop until successful save or user cancel
	while ( dialog.exec () )
	{
		QString filename = mtQEX::get_filename ( dialog );

		if ( filename.isEmpty () )
		{
			continue;
		}

		int const format = dialog.getFormat ();
		int ft = PIXY_FILE_TYPE_PNG;

		// Retrieve file type from table
		for ( int i = 0; i < list_ft_tot; i++ )
		{
			if ( format == i )
			{
				ft = list_ft[i];
				break;
			}
		}

		char * correct = mtPixyUI::File::get_correct_filename (
			filename.toUtf8().data(), ft );

		if ( correct )
		{
			filename = QString::fromUtf8 ( correct );
			free ( correct );
			correct = NULL;
		}

		if ( mtQEX::message_file_overwrite ( this, filename ) )
		{
			continue;
		}

		if ( 0 == project_save( filename.toUtf8().data(), ft ) )
		{
			break;
		}
	}
}

void Mainwindow::press_file_export_undo ()
{
	QStringList		list;


	mtQEX::SaveFileDialog dialog ( this, "Export Undo Images", list,
		0, backend.file.get_filename () );

	if ( ! backend.file.get_filename () )
	{
		dialog.setDirectory ( mtQEX::qstringFromC (
			backend.mprefs.recent_image.filename().c_str() ) );
	}

	while ( dialog.exec () )
	{
		QString const filename = mtQEX::get_filename ( dialog );

		if ( filename.isEmpty () )
		{
			break;
		}

		if ( 0 == backend.file.export_undo_images (
			filename.toUtf8 ().data () ) )
		{
			break;
		}

		QMessageBox::critical ( this, "Error",
			"Unable to save undo images." );
	}
}

void Mainwindow::press_file_quit ()
{
	close ();
}

void Mainwindow::press_file_recent ( size_t const i )
{
	if ( ! ok_to_lose_changes () )
	{
		return;
	}

	project_load ( backend.mprefs.recent_image.filename (i).c_str() );
}

int Mainwindow::project_load ( char const * const fn )
{
	if ( backend.file.load_image ( fn,
		mprefs.file_new_palette_type,
		mprefs.file_new_palette_num,
		mprefs.file_new_palette_file )
		)
	{
		QMessageBox::critical ( this, "Error",
			QString ( "Unable to load file:\n%1" ).arg (
			mtQEX::qstringFromC ( fn ) ) );

		return 1;	// Problem loading image file
	}

	backend.mprefs.recent_image.set ( fn );

	update_ui ( UPDATE_ALL | UPDATE_RECENT_FILES );

	set_scroll_position_h ( m_scroll_main, 0.5 );
	set_scroll_position_v ( m_scroll_main, 0.5 );

	return 0;
}

int Mainwindow::project_save (
	char	const * const	fn,
	int		const	ft
	)
{
	int		comp = 0;


	switch ( ft )
	{
	case PIXY_FILE_TYPE_JPEG:
		comp = mprefs.file_compression_jpeg;
		break;

	case PIXY_FILE_TYPE_PNG:
		comp = mprefs.file_compression_png;
		break;

	default:
		break;
	}

	if ( backend.file.save_image ( fn, ft, comp ) )
	{
		QMessageBox::critical ( this, "Error",
			QString ( "Unable to save file:\n%1" ).arg (
			mtQEX::qstringFromC ( fn ) ) );

		return 1;	// Problem saving image file
	}

	backend.mprefs.recent_image.set ( fn );

	update_ui ( UPDATE_TITLEBAR | UPDATE_RECENT_FILES );

	return 0;
}

int Mainwindow::project_new ( mtPixmap * const im )
{
	if ( im )
	{
		backend.file.set_pixmap ( im );
	}
	else
	{
		if ( backend.file.new_image (
			backend.mprefs.file_new_type,
			mprefs.file_new_width,
			mprefs.file_new_height,
			mprefs.file_new_palette_type,
			mprefs.file_new_palette_num,
			mprefs.file_new_palette_file )
			)
		{
			int const type = PIXY_PIXMAP_BPP_RGB;
			int const w = 1;
			int const h = 1;

			backend.file.new_image ( type, w, h,
				mprefs.file_new_palette_type,
				mprefs.file_new_palette_num,
				mprefs.file_new_palette_file );

			backend.uprefs.set ( PREFS_FILE_NEW_WIDTH, w );
			backend.uprefs.set ( PREFS_FILE_NEW_HEIGHT, h );
			backend.uprefs.set ( PREFS_FILE_NEW_TYPE, type );
		}
	}

	update_ui ( UPDATE_ALL );

	return 0;
}

int Mainwindow::ok_to_lose_changes ()
{
	finish_tool_mode ();

	if ( backend.file.get_modified () )
	{
		int const res = QMessageBox::warning ( this, "Warning",
			"This image has been modified. Do you really want to "
			"lose these changes?",
			QMessageBox::Cancel | QMessageBox::Discard,
			QMessageBox::Cancel );

		if ( res == QMessageBox::Cancel )
		{
			return 0;	// Not OK to lose changes
		}
	}

	return 1;			// OK to lose changes
}

mtPixmap * Mainwindow::get_screenshot ()
{
	QScreen	* const	screen = QGuiApplication::primaryScreen ();
	if ( screen )
	{
		QPixmap const pixmap = screen->grabWindow ( 0 );

		return mtQEX::pixypixmap_from_qpixmap ( &pixmap );
	}

	return NULL;
}

