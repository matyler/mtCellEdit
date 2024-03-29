/*
	Copyright (C) 2013-2022 Mark Tyler

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



void MainWindow::press_OptionsGraph ()
{
	if ( graphTextEdit->hasFocus () )
	{
		tabWidget->hide ();
		viewMain->setFocus ();
	}
	else
	{
		tabWidget->show ();
		tabWidget->setCurrentIndex ( TAB_GRAPH );
		graphTextEdit->setFocus ( Qt::OtherFocusReason );
	}
}

void MainWindow::projectGraphStoreChanges ()
{
	// Checks for changes, if so save them to the current graph
	// (active_graph).

	if ( ! graphTextEdit->document ()->isModified () )
	{
		// No changes so nothing to store

		return;
	}


	CedBookFile	* bookfile = cui_file_get_graph ( cedFile );


	if ( ! bookfile )
	{
		return;
	}


	QByteArray	tmp = graphTextEdit->toPlainText ().toUtf8 ();
	char		* txt = tmp.data ();

	if ( ! txt )
	{
		return;
	}


	size_t	const	len = strlen ( txt );


	if ( len > INT_MAX )
	{
		return;
	}

	txt = strdup ( txt );

	// Free old memory and use newly allocated text
	free ( bookfile->mem );
	bookfile->mem = txt;

	if ( txt )
	{
		bookfile->size = (int)len;
	}

	ced_book_timestamp_file ( bookfile );

	updateChangesChores ( 0, 1 );

	graphTextEdit->document ()->setModified ( false );

	projectGraphRedraw ();		// Draw the new graph
}

void MainWindow::graphChanged (
	int	const	ARG_UNUSED ( i )
	)
{
	QString		text;
	char	const	* graph_text = NULL;
	int		graph_text_len = 0;
	CedBookFile	* bookfile;
	CedBook		* book;


	projectGraphStoreChanges ();

	text = buttonGraph->text ();
	book = cedFile->cubook->book;

	if ( text.isEmpty () )
	{
		mtkit_strfreedup ( &book->prefs.active_graph, NULL );
	}
	else
	{
		bookfile = cui_graph_get ( book, text.toUtf8 ().data () );
		if ( bookfile && bookfile->mem )
		{
			graph_text = bookfile->mem;
			graph_text_len = bookfile->size;
		}

		mtkit_strfreedup ( &book->prefs.active_graph,
			text.toUtf8 ().data () );
	}

	graphTextEdit->setPlainText ( mtQEX::qstringFromC ( graph_text,
		graph_text_len ) );

	graphTextEdit->document ()->setModified ( false );

	projectGraphRedraw ();		// Draw the new graph
}

void MainWindow::press_GraphNew ()
{
	char	const	* newname;


	projectGraphStoreChanges ();

	newname = be_graph_new ( cedFile->cubook->book );

	if ( newname )
	{
		updateGraph ( newname );
	}
	else
	{
		QMessageBox::critical ( this, "Error",
			"Unable to create a new graph." );
	}

	updateChangesChores ( 0, 1 );
}

void MainWindow::press_GraphDuplicate ()
{
	projectGraphStoreChanges ();

	char * newname = be_graph_duplicate ( cedFile->cubook );

	if ( newname )
	{
		updateGraph ( newname );
		free ( newname );
		newname = NULL;
	}
	else
	{
		QMessageBox::critical ( this, "Error",
			"Unable to duplicate the current graph." );
	}

	updateChangesChores ( 0, 1 );
}

int MainWindow::projectRenameGraph (
	QString		const	newName
	)
{
	char	const	* old_name;
	char	const	* new_name;
	CedBookFile	* old;
	CedBook		* book = cedFile->cubook->book;
	QByteArray	tmp = newName.toUtf8 ();


	new_name = tmp.data ();

	if ( ! new_name || new_name[0] == 0 )
	{
		QMessageBox::critical ( this, "Error", "Bad graph name." );

		return 1;
	}
	else if ( cui_graph_get ( book, new_name ) )
	{
		QMessageBox::critical ( this, "Error",
			"Graph name already exists." );

		return 1;
	}

	old_name = cedFile->cubook->book->prefs.active_graph;
	old = cui_graph_get ( book, old_name );

	if (	! old ||
		! cui_graph_new ( book, old->mem, old->size, new_name )
		)
	{
		QMessageBox::critical ( this, "Error",
			"Unable to rename graph." );

		return 1;
	}
	else
	{
		old->mem = NULL;
		cui_graph_destroy ( book, old_name );
	}

	updateGraph ( new_name );
	updateChangesChores ( 0, 1 );

	return 0;			// Success
}

void MainWindow::press_GraphRename ()
{
	char	const	* old_name;


	old_name = cedFile->cubook->book->prefs.active_graph;

	if ( ! old_name )
	{
		return;
	}

	projectGraphStoreChanges ();

	for ( int res = 1; res; )
	{
		bool	ok;
		QString new_name = QInputDialog::getText ( this,
			"Rename Graph",
			"New Graph Name:",
			QLineEdit::Normal,
			mtQEX::qstringFromC ( old_name ),
			&ok );


		if ( ok )
		{
			res = projectRenameGraph ( new_name );
		}
		else
		{
			break;
		}
	}
}

void MainWindow::press_GraphDelete ()
{
	int		res,
			gnum;
	CedBook		* book;


	res = QMessageBox::question ( this, "Question",
		"Do you really want to delete this graph?",
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No );

	if ( res != QMessageBox::Yes )
	{
		return;
	}

	gnum = buttonGraph->currentIndex ();

	book = cedFile->cubook->book;
	if ( cui_graph_destroy ( book, book->prefs.active_graph ) )
	{
		QMessageBox::critical ( this, "Error",
			"Unable to delete this graph." );

		return;
	}

	updateGraph ( NULL );

	if ( gnum > 0 )
	{
		gnum --;
	}

	buttonGraph->setCurrentIndex ( gnum );

	updateChangesChores ( 0, 1 );
}

void MainWindow::projectGraphRedraw ()
{
	int		err = -1;
	CedBook		* const book = cedFile->cubook->book;

	mtPixmap * const pixmap = cui_graph_render_pixmap ( book,
		book->prefs.active_graph, &err, mprefs.graph_scale,
		mprefs.font_pango_name.c_str() );
	graphWidget->setPixmap ( pixmap );

	if ( err >= 0 )
	{
		QTextCursor	c = graphTextEdit->textCursor ();


		c.movePosition ( QTextCursor::End );
		c.setPosition ( err, QTextCursor::KeepAnchor );

		graphTextEdit->setTextCursor ( c );
	}
}

void MainWindow::press_GraphRedraw ()
{
	projectGraphStoreChanges ();
	projectGraphRedraw ();
}

void MainWindow::press_GraphExport ()
{
	char const * const graphname= cedFile->cubook->book->prefs.active_graph;

	if ( ! graphname )
	{
		return;
	}

	projectGraphStoreChanges ();

	QStringList	list;
	QString		last = mtQEX::qstringFromC ( mprefs.last_dir.c_str() )
				+ MTKIT_DIR_SEP + "export";


	// Order as per CED_GRAPH_TYPE_*
	list	<< "EPS"
		<< "PDF"
		<< "PNG"
		<< "PS"
		<< "SVG"
		;

	mtQEX::SaveFileDialog dialog ( this, "Export Graph", list,
		lastExportGraphType, last.toUtf8 ().data () );

	dialog.setOption ( QFileDialog::DontConfirmOverwrite );

	while ( dialog.exec () )
	{
		QString filename = mtQEX::get_filename ( dialog );

		if ( filename.isEmpty () )
		{
			break;
		}

		lastExportGraphType = dialog.getFormat ();

		char * correct = cui_get_correct_graph_filename (
			filename.toUtf8().data(), lastExportGraphType );

		if ( correct )
		{
			filename = correct;
			free ( correct );
			correct = NULL;
		}

		if ( mtQEX::message_file_overwrite ( this, filename ) )
		{
			continue;
		}

		if ( cui_graph_render_file ( cedFile->cubook->book,
			graphname, filename.toUtf8 ().data (),
			lastExportGraphType, NULL,
			mprefs.graph_scale, mprefs.font_pango_name.c_str() )
			)
		{
			QMessageBox::critical ( this, "Error",
				"Unable to export graph." );

			continue;
		}

		backend->remember_last_dir( filename.toUtf8 ().data() );

		break;
	}
}

void MainWindow::press_GraphSClipboard ()
{
	QClipboard * const c = QApplication::clipboard ();

	if ( c )
	{
		char * txt = be_graph_selection_clip ( projectGetSheet () );

		if ( txt )
		{
			c->setText ( mtQEX::qstringFromC ( txt ) );

			free ( txt );
			txt = NULL;
		}
		else
		{
			c->setText ( "No sheet available" );
		}
	}
}

