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

#include "qt5.h"



#define MTCELLEDIT_CLIP_NAME	"application/x-mtcelledit-clipboard"



void MainWindow::clipboardFlushInternal ()
{
	if ( 0 != mprefs.clipboard_use_system )
	{
		QClipboard	* const c = QApplication::clipboard ();


		if ( c && c->ownsClipboard () )
		{
			c->clear ();
		}
		else
		{
			// We don't own the clipboard so only flush internal
		}
	}

	cui_clip_flush ( cedClipboard );
}

void MainWindow::clipboardSetOwner ()
{
	if ( 0 == mprefs.clipboard_use_system )
	{
		return;
	}


	std::unique_ptr<QMimeData> mime_data ( new QMimeData );


	if ( cui_clip_save_temp ( cedClipboard ) )
	{
		QMessageBox::critical ( this, "Error",
			"Unable to create temp file." );
	}
	else
	{
		QByteArray	dat ( cedClipboard->timestamp,
					CUI_CLIPBOARD_TIMESTAMP_SIZE );


		mime_data->setData ( mtQEX::qstringFromC (MTCELLEDIT_CLIP_NAME),
			dat );
	}

	if (	cui_clip_export_text ( cedClipboard ) ||
		! cedClipboard->tsv )
	{
		QMessageBox::critical ( this, "Error",
			"Unable to create TSV data." );
	}
	else
	{
		mime_data->setText ( mtQEX::qstringFromC ( cedClipboard->tsv ));
	}

	QClipboard	* const c = QApplication::clipboard ();

	if ( c )
	{
		c->setMimeData ( mime_data.release() );
	}
}

int MainWindow::clipboardCopySelection ( int const mode )
{
	clipboardFlushInternal ();

	if ( cui_clip_copy ( cedFile, cedClipboard ) )
	{
		QMessageBox::critical ( this, "Error",
			"Unable to copy selection to clipboard." );

		return 1;
	}

	int res = 0;

	switch ( mode )
	{
	case 1:	res = be_clip_copy_values ( cedClipboard->sheet );
		break;
	case 2: res = be_clip_copy_output ( cedClipboard->sheet );
		break;
	}

	if ( res )
	{
		QMessageBox::critical ( this, "Error",
			"Problem adjusting clipboard." );
	}

	clipboardSetOwner ();

	return 0;
}

int MainWindow::clipboardClearSelection ( int const mode )
{
	int const res = be_clip_clear_selection ( cedFile, projectGetSheet (),
		mode );

	if ( res > 0 )
	{
		return 1;
	}

	projectReportUpdates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 1;
	}

	updateChangesChores ( 1, 0 );

	return 0;			// Success
}

void MainWindow::press_EditCut ()
{
	if ( clipboardCopySelection ( 0 ) )
	{
		return;
	}

	if ( clipboardClearSelection ( 0 ) )
	{
		return;
	}
}

void MainWindow::clipboardCopyRouter (
	int		mode
	)
{
	if ( clipboardCopySelection ( mode ) )
	{
		return;
	}

	viewMain->setBell ();
}

void MainWindow::press_EditCopy ()
{
	clipboardCopyRouter ( 0 );
}

void MainWindow::press_EditCopyVal ()
{
	clipboardCopyRouter ( 1 );
}

void MainWindow::press_EditCopyOutput ()
{
	clipboardCopyRouter ( 2 );
}

void MainWindow::clipboardTransform ( int const mode )
{
	if ( ! clipboardObtainPaste () )
	{
		return;			// No paste to act on
	}

	CedSheet * sheet = be_clip_transform_start ( cedClipboard, mode );

	if ( ! sheet )
	{
		goto error;
	}

	// Remove old, install new
	clipboardFlushInternal ();

	if ( be_clip_transform_finish ( cedClipboard, sheet, mode ) )
	{
		// be_* clears up sheet on error
		goto error;
	}

	clipboardSetOwner ();		// Claim ownership

	return;				// Success

error:
	QMessageBox::critical ( this, "Error",
		"Unable to transform clipboard." );
}

void MainWindow::press_EditTransformTrans ()
{
	clipboardTransform ( 0 );
}

void MainWindow::press_EditTransformFlipH ()
{
	clipboardTransform ( 1 );
}

void MainWindow::press_EditTransformFlipV ()
{
	clipboardTransform ( 2 );
}

void MainWindow::press_EditTransformRotClock ()
{
	clipboardTransform ( 3 );
}

void MainWindow::press_EditTransformRotAnti ()
{
	clipboardTransform ( 4 );
}

int MainWindow::clipboardGetMtcelledit ( QClipboard * const clipboard )
{
	QMimeData	const * const	mime = clipboard->mimeData ();


	if ( ! mime )
	{
		return 0;		// Not sent by mtCellEdit
	}

	QByteArray	dat = mime->data ( mtQEX::qstringFromC (
				MTCELLEDIT_CLIP_NAME ) );

	if ( dat.size () != CUI_CLIPBOARD_TIMESTAMP_SIZE )
	{
		return 0;		// Not sent by mtCellEdit
	}

	if ( clipboard->ownsClipboard () )
	{
		// This instance created this clipboard and its the mtCellEdit
		// sheet format.

		return 1;
	}

	if (	! cedClipboard->sheet ||
		memcmp( cedClipboard->timestamp, dat.data (),
			CUI_CLIPBOARD_TIMESTAMP_SIZE ) )
	{
		if ( cui_clip_load_temp ( cedClipboard ) )
		{
			return 0;	// Unable to load clipboard - error
		}

		// Set the new timestamp
		cui_clip_set_timestamp ( cedClipboard, (char *)dat.data () );
	}

	return 1;			// mtCellEdit clipboard is available
}

int MainWindow::clipboardReadSystem ()
{
	if ( 0 == mprefs.clipboard_use_system )
	{
		return 0;
	}


	QClipboard	* const c = QApplication::clipboard ();


	if ( ! c )
	{
		return 1;
	}

	if ( clipboardGetMtcelledit ( c ) )
	{
		// This clipboard was created by this or another mtCellEdit
		// instance.

		return 0;
	}

	if ( cui_clip_import_text ( cedClipboard, c->text().toUtf8().data() ) )
	{
		return 1;		// Error importing
	}

	return 0;			// Success
}

int MainWindow::clipboardObtainPaste ()
{
	if ( ! projectGetSheet () )
	{
		return 0;
	}

	if ( clipboardReadSystem () )
	{
		return 0;		// Nothing read in
	}

	if ( ! cedClipboard->sheet )
	{
		return 0;		// Nothing to paste
	}

	return 1;	// Successfully transferred something to clipboard
}

int MainWindow::clipboardPasteAtCursor ( int const mode )
{
	if ( ! clipboardObtainPaste () )
	{
		return 1;		// No paste found
	}

	int const res = cui_clip_paste ( cedFile, cedClipboard, mode );
	if ( res == 1 )
	{
		return 1;
	}

	projectReportUpdates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL	||
		res == CUI_ERROR_LOCKED_SHEET	||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 1;		// Nothing changed
	}

	updateChangesChores ( 1, 0 );

	return 0;			// Paste committed
}

void MainWindow::press_EditPaste ()
{
	clipboardPasteAtCursor ( 0 );
}

void MainWindow::press_EditPasteContent ()
{
	clipboardPasteAtCursor ( CED_PASTE_CONTENT );
}

void MainWindow::press_EditPastePrefs ()
{
	clipboardPasteAtCursor ( CED_PASTE_PREFS );
}

void MainWindow::press_EditClear ()
{
	if ( clipboardClearSelection ( 0 ) )
	{
		return;
	}
}

void MainWindow::press_EditClearContent ()
{
	if ( clipboardClearSelection ( CED_PASTE_CONTENT ) )
	{
		return;
	}
}

void MainWindow::press_EditClearPrefs ()
{
	if ( clipboardClearSelection ( CED_PASTE_PREFS ) )
	{
		return;
	}
}

void MainWindow::press_EditUseSystemClipboard ()
{
	if ( act_EditUseSystemClipboard->isChecked () )
	{
		uprefs.set ( PREFS_CLIPBOARD_USE_SYSTEM, 1 );
	}
	else
	{
		uprefs.set ( PREFS_CLIPBOARD_USE_SYSTEM, 0 );
	}
}

