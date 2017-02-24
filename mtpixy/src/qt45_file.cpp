/*
	Copyright (C) 2016 Mark Tyler

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

#include "qt45.h"



void Mainwindow::press_file_new ()
{
	if ( ! ok_to_lose_changes () )
	{
		return;
	}

	mtPixy::Image	* const	clip_im = backend.clipboard.get_image ();
	DialogImageNew		dialog (
		prefs.getInt ( PREFS_FILE_NEW_WIDTH ),
		prefs.getInt ( PREFS_FILE_NEW_HEIGHT ),
		prefs.getInt ( PREFS_FILE_NEW_TYPE ),
		clip_im ? true : false );


	if ( dialog.exec () != QDialog::Accepted )
	{
		return;
	}

	prefs.set ( PREFS_FILE_NEW_WIDTH, dialog.get_width () );
	prefs.set ( PREFS_FILE_NEW_HEIGHT, dialog.get_height () );

	int	const	t = dialog.get_type ();

	if ( t < 0 && clip_im )
	{
		project_new ( clip_im->duplicate () );
	}
	else
	{
		prefs.set ( PREFS_FILE_NEW_TYPE, t );
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
		"Load Image File", mtQEX::qstringFromC ( prefs.getString (
			PREFS_FILE_RECENT_IMAGE ".001" ) ),
		NULL, NULL, QFileDialog::DontUseNativeDialog );


	if ( ! filename.isEmpty () )
	{
		project_load ( filename.toUtf8 ().data () );
	}
}

void Mainwindow::press_file_save ()
{
	if ( backend.file.get_filename () == NULL )
	{
		press_file_save_as ();
		return;
	}

	project_save ( backend.file.get_filename (),
		backend.file.get_filetype () );
}

void Mainwindow::press_file_save_as ()
{
	mtPixy::Image		* const	im = backend.file.get_image ();
	int			ft_item = 0;
	mtPixy::File::Type	const	ftnow = backend.file.get_filetype ();
	mtPixy::Image::Type	const	imtype = im->get_type ();
	QStringList		list;


	if ( imtype == mtPixy::Image::RGB )
	{
		list << "BMP" << "JPEG" << "PNG";

		switch ( ftnow )
		{
		case mtPixy::File::BMP:		ft_item = 0;	break;
		case mtPixy::File::JPEG:	ft_item = 1;	break;
		default:
		case mtPixy::File::PNG:		ft_item = 2;	break;
		}
	}
	else	// Indexed
	{
		list << "BMP" << "GIF" << "PNG";

		switch ( ftnow )
		{
		case mtPixy::File::BMP:		ft_item = 0;	break;
		case mtPixy::File::GIF:		ft_item = 1;	break;
		default:
		case mtPixy::File::PNG:		ft_item = 2;	break;
		}
	}

	mtQEX::SaveFileDialog dialog ( this, "Save Image File", list,
		ft_item, backend.file.get_filename () );

	if ( ! backend.file.get_filename () )
	{
		dialog.setDirectory ( mtQEX::qstringFromC ( prefs.getString (
			PREFS_FILE_RECENT_IMAGE ".001" ) ) );
	}

	while ( dialog.exec () )
	{
		QStringList	fileList = dialog.selectedFiles ();
		QString		filename = fileList.at ( 0 );
		int		format = dialog.getFormat ();
		mtPixy::File::Type ft = mtPixy::File::BMP;


		if ( imtype == mtPixy::Image::RGB )
		{
			switch ( format )
			{
			case 0:	ft = mtPixy::File::BMP;		break;
			case 1:	ft = mtPixy::File::JPEG;	break;
			default:
			case 2:	ft = mtPixy::File::PNG;		break;
			}
		}
		else	// Indexed
		{
			switch ( format )
			{
			case 0:	ft = mtPixy::File::BMP;		break;
			case 1:	ft = mtPixy::File::GIF;		break;
			default:
			case 2:	ft = mtPixy::File::PNG;		break;
			}
		}

		if ( ! filename.isEmpty () )
		{
			if ( 0 == project_save( filename.toUtf8().data(), ft ) )
			{
				break;
			}
		}
		else
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
		dialog.setDirectory ( mtQEX::qstringFromC ( prefs.getString (
			PREFS_FILE_RECENT_IMAGE ) ) );
	}

	while ( dialog.exec () )
	{
		QStringList	fileList = dialog.selectedFiles ();
		QString		filename = fileList.at ( 0 );


		if ( ! filename.isEmpty () )
		{
			if ( 0 == backend.file.export_undo_images (
				filename.toUtf8 ().data () ) )
			{
				break;
			}
			else
			{
				QMessageBox::critical ( this, "Error",
					QString("Unable to save undo images."));
			}
		}
		else
		{
			break;
		}
	}
}

void Mainwindow::press_file_quit ()
{
	close ();
}

void Mainwindow::press_file_recent (
	int	const	i
	)
{
	if ( ! ok_to_lose_changes () )
	{
		return;
	}

	project_load ( backend.recent_image.get_filename ( i ) );
}

int Mainwindow::project_load (
	char	const * const	fn
	)
{
	if ( backend.file.load_image ( fn,
		prefs.getInt ( PREFS_FILE_NEW_PALETTE_TYPE ),
		prefs.getInt ( PREFS_FILE_NEW_PALETTE_NUM ) )
		)
	{
		QMessageBox::critical ( this, "Error",
			QString ( "Unable to load file:\n%1" ).arg (
			mtQEX::qstringFromC ( fn ) ) );

		return 1;	// Problem loading image file
	}

	backend.recent_image.set_filename ( fn );

	update_ui ( UPDATE_ALL | UPDATE_RECENT_FILES );

	set_scroll_position_h ( m_scroll_main, 0.5 );
	set_scroll_position_v ( m_scroll_main, 0.5 );

	return 0;
}

int Mainwindow::project_save (
	char		const * const	fn,
	mtPixy::File::Type	const	ft
	)
{
	int		comp = 0;


	switch ( ft )
	{
	case mtPixy::File::JPEG:
		comp = prefs.getInt ( PREFS_FILE_COMPRESSION_JPEG );
		break;

	case mtPixy::File::PNG:
		comp = prefs.getInt ( PREFS_FILE_COMPRESSION_PNG );
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

	backend.recent_image.set_filename ( fn );

	update_ui ( UPDATE_TITLEBAR | UPDATE_RECENT_FILES );

	return 0;
}

int Mainwindow::project_new (
	mtPixy::Image	* const	im
	)
{
	if ( im )
	{
		backend.file.set_image ( im );
	}
	else
	{
		if ( backend.file.new_image (
		(mtPixy::Image::Type)backend.prefs.getInt(PREFS_FILE_NEW_TYPE),
			prefs.getInt ( PREFS_FILE_NEW_WIDTH ),
			prefs.getInt ( PREFS_FILE_NEW_HEIGHT ),
			prefs.getInt ( PREFS_FILE_NEW_PALETTE_TYPE ),
			prefs.getInt ( PREFS_FILE_NEW_PALETTE_NUM ) )
			)
		{
			backend.file.new_image ( mtPixy::Image::RGB, 1, 1,
				prefs.getInt ( PREFS_FILE_NEW_PALETTE_TYPE ),
				prefs.getInt ( PREFS_FILE_NEW_PALETTE_NUM ) );

			prefs.set ( PREFS_FILE_NEW_WIDTH, 1 );
			prefs.set ( PREFS_FILE_NEW_HEIGHT, 1 );
			prefs.set ( PREFS_FILE_NEW_TYPE, 2 );
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



#ifdef U_TK_QT4

mtPixy::Image * Mainwindow::get_screenshot ()
{
	QPixmap pixmap = QPixmap::grabWindow( QApplication::desktop()->winId());

	return mtQEX::pixyimage_from_qpixmap ( &pixmap );
}

#endif	// U_TK_QT4



#ifdef U_TK_QT5

mtPixy::Image * Mainwindow::get_screenshot ()
{
	QScreen	* const	screen = QGuiApplication::primaryScreen ();
	if ( screen )
	{
		QPixmap pixmap = screen->grabWindow ( 0 );

		return mtQEX::pixyimage_from_qpixmap ( &pixmap );
	}

	return NULL;
}

#endif	// U_TK_QT5


