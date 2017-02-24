/*
	Copyright (C) 2013-2017 Mark Tyler

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

#include "tk_qt4.h"



void MainWindow::pressFocusOutCellRef ()
{
	updateEntryCellref ();
}

void MainWindow::pressUpdateCellRef ()
{
	CedCellRef	r1,
			r2;
	QByteArray	tmp = editCellref->text ().toUtf8 ();
	char	const	* txt = tmp.data ();


	if ( ! ced_strtocellref ( txt, &r1, NULL, 1 ) )
	{
		if (	r1.row_m	||
			r1.col_m	||
			r1.row_d < 1	||
			r1.col_d < 1
			)
		{
			return;
		}

		setCursorRange ( r1.row_d, r1.col_d, r1.row_d, r1.col_d, 1, 1,
			1 );
	}
	else if ( ! ced_strtocellrange ( txt, &r1, &r2, NULL, 1 ) )
	{
		if (	r1.row_m	||
			r1.col_m	||
			r1.row_d < 1	||
			r1.col_d < 1	||
			r2.row_m	||
			r2.col_m	||
			r2.row_d < 1	||
			r2.col_d < 1
			)
		{
			return;
		}

		setCursorRange ( r1.row_d, r1.col_d, r2.row_d, r2.col_d, 1, 1,
			1 );
	}
	else
	{
		return;
	}

	viewMain->setFocus ();
}

void MainWindow::pressTabCellRef (
	int	const	ARG_UNUSED ( t )
	)
{
	editCelltext->setFocus ();
}

void MainWindow::pressStopCellRef ()
{
	viewMain->setFocus ();		// Update happens in focus out
}

void MainWindow::setCellFromInput (
	int	const	rowd,
	int	const	cold
	)
{
	int		res,
			r,
			c;
	char		* newtxt;
	CedSheet	* sheet;


	sheet = mainwindow->projectGetSheet ();
	if ( ! sheet )
	{
		return;
	}


	QByteArray	tmp = editCelltext->text ().toUtf8 ();

	newtxt = tmp.data ();

	if ( newtxt && newtxt[0] == 0 )
	{
		newtxt = NULL;
	}

	res = cui_sheet_set_cell ( cedFile->cubook, sheet,
		sheet->prefs.cursor_r1, sheet->prefs.cursor_c1, newtxt );
	projectReportUpdates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return;
	}

	viewMain->setFocus ();

	/*
	Manually setting these limits here is safe because the scrollbars have
	already changed thanks to set_cursor_range ().  This is far more
	efficient than wasting time getting the new geometry from libmtcelledit.
	update_changes_chores () helpfully updates the rest of the GUI for us.
	*/

	sheetRows = MAX ( (int)sheetRows, sheet->prefs.cursor_r1 );
	sheetCols = MAX ( (int)sheetCols, sheet->prefs.cursor_c1 );

	r = sheet->prefs.cursor_r1 + rowd;
	c = sheet->prefs.cursor_c1 + cold;

	setCursorRange ( r, c, r, c, 1, 0, 0 );

	updateChangesChores ( 0, 0 );
}

void MainWindow::pressUpdateCellText ()
{
	setCellFromInput ( 1, 0 );
}

void MainWindow::pressTabCellText (
	int	const	t
	)
{
	setCellFromInput ( 0, t );
}

void MainWindow::pressArrowCellText (
	int	const	r
	)
{
	setCellFromInput ( r, 0 );
}

void MainWindow::pressStopCellText ()
{
	updateEntryCelltext ();
	viewMain->setFocus ();
}

bool MyLineEdit::event (
	QEvent		* const	ev
	)
{
	QKeyEvent	* const keyEvent = static_cast<QKeyEvent *>( ev );


	if ( ev->type () != QEvent::KeyPress || ! keyEvent )
	{
		// Nothing to do so let the base class handle this event instead
		return QLineEdit::event ( ev );
	}

	switch ( keyEvent->key () )
	{
	case Qt::Key_Escape:
		emit pressStopKey ();
		return true;		// Processed event

	case Qt::Key_Enter:
	case Qt::Key_Return:
		emit pressUpdateKey ();
		return true;		// Processed event

	case Qt::Key_Tab:
		if ( keyEvent->modifiers () & Qt::ControlModifier )
		{
			emit pressTabKey ( -1 );
		}
		else
		{
			emit pressTabKey ( 1 );
		}
		return true;		// Processed event

	case Qt::Key_Up:
		emit pressArrowKey ( -1 );
		return true;

	case Qt::Key_Down:
		emit pressArrowKey ( 1 );
		return true;
	}

	// Nothing to do so let the base class handle this event instead
	return QLineEdit::event ( ev );
}

void MyLineEdit::focusOutEvent (
	QFocusEvent	* const	e
	)
{
	emit pressFocusOut ();

	QLineEdit::focusOutEvent ( e );
}



typedef struct
{
	int		changed;
	int		r;
	int		c;
} jumpSTATE;



static int jump_to_active_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	ARG_UNUSED ( cell ),
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	jumpSTATE	* const	jstate = (jumpSTATE *)user_data;


	jstate->r = row;
	jstate->c = col;

	return 1;			// STOP
}

void MainWindow::keyPressEvent (
	QKeyEvent	* const	ev
	)
{
	int		row_change = 0,
			col_change = 0,
			home = 0,
			end = 0,
			key_shift,
			key_ctrl,
			key_alt,
			return_true = 0;
	CedSheet	* sheet;
	jumpSTATE	jstate = { 0, 0, 0 };
	QString		text = ev->text ();
	CedView		* activeView = viewMain;


	sheet = mainwindow->projectGetSheet ();
	if ( ! sheet )
	{
		goto no_action;
	}

	if ( ev->key () == Qt::Key_Escape )
	{
		viewMain->setFocus ();
		return;
	}

	if ( editFindText->hasFocus () || findTable->hasFocus () )
	{
		goto no_action;
	}

	jstate.r = sheet->prefs.cursor_r2;
	jstate.c = sheet->prefs.cursor_c2;

	if ( viewTab->hasFocus () )
	{
		activeView = viewTab;
	}

	key_shift = (int)(ev->modifiers () & Qt::ShiftModifier);
	key_ctrl = (int)(ev->modifiers () & Qt::ControlModifier);
	key_alt = (int)(ev->modifiers () & Qt::AltModifier);

	switch ( ev->key () )
	{
	case Qt::Key_Up:
		if ( key_ctrl )
		{
			return_true = 1;

			row_change = 1 - sheet->prefs.cursor_r2; // Default
			if ( jstate.r > 1 )
			{
				if ( ced_sheet_scan_area_backwards ( sheet,
					jstate.r - 1, CED_MAX_COLUMN,
					jstate.r - 1, CED_MAX_COLUMN,
					jump_to_active_cb, &jstate ) == 2 )
				{
					row_change = jstate.r -
						sheet->prefs.cursor_r2;
				}
			}
		}
		else
		{
			row_change = -1;
		}
		break;

	case Qt::Key_Enter:
	case Qt::Key_Return:
	case Qt::Key_Down:
		if ( key_ctrl )
		{
			return_true = 1;

			row_change = CED_MAX_ROW - sheet->prefs.cursor_r2;
					// Default

			if ( jstate.r < CED_MAX_ROW )
			{
				if ( ced_sheet_scan_area ( sheet,
					jstate.r + 1, 0, 0, 0,
					jump_to_active_cb, &jstate ) == 2 )
				{
					row_change = jstate.r -
						sheet->prefs.cursor_r2;
				}
			}
		}
		else
		{
			row_change = 1;
		}
		break;

	case Qt::Key_Left:
		if ( key_ctrl )
		{
			return_true = 1;

			col_change = 1 - sheet->prefs.cursor_c2; // Default
			if ( jstate.c > 1 )
			{
				if ( ced_sheet_scan_area_backwards ( sheet,
					jstate.r, jstate.c - 1,
					1, jstate.c - 1,
					jump_to_active_cb, &jstate ) == 2 )
				{
					col_change = jstate.c -
						sheet->prefs.cursor_c2;
				}
			}
		}
		else
		{
			col_change = -1;
		}
		break;

	case Qt::Key_Right:
		if ( key_ctrl )
		{
			return_true = 1;

			col_change = CED_MAX_COLUMN - sheet->prefs.cursor_c2;
					// Default

			if ( jstate.c < CED_MAX_COLUMN )
			{
				if ( ced_sheet_scan_area ( sheet,
					jstate.r, jstate.c + 1, 1, 0,
					jump_to_active_cb, &jstate ) == 2 )
				{
					col_change = jstate.c -
						sheet->prefs.cursor_c2;
				}
			}
		}
		else
		{
			col_change = 1;
		}
		break;

	case Qt::Key_PageUp:
		if ( key_alt )
		{
			// ALT key pressed so shift a page to the left
			col_change = activeView->getPageColumnsLeft (
				sheet->prefs.cursor_c2 );
		}
		else
		{
			row_change = -activeView->getVisibleRows ();
		}
		break;

	case Qt::Key_PageDown:
		if ( key_alt )
		{
			// ALT key pressed so shift a page to the right
			col_change = activeView->getPageColumnsRight (
				sheet->prefs.cursor_c2 );
		}
		else
		{
			row_change = activeView->getVisibleRows ();
		}
		break;

	case Qt::Key_Home:
		home = 1;
		col_change = 1 - sheet->prefs.cursor_c2;
		if ( key_ctrl )
		{
			row_change = 1 - sheet->prefs.cursor_r2;
		}
		break;

	case Qt::Key_End:
		end = 1;
		col_change = sheetCols - sheet->prefs.cursor_c2;

		if ( key_ctrl )
		{
			row_change = sheetRows - sheet->prefs.cursor_r2;
		}
		break;

	case Qt::Key_Pause:
		buttonSheet->popup ();
		break;
	}

	if ( row_change || col_change || home || end )
	{
		int		newrow = row_change,
				newcol = col_change;

		if ( key_shift )
		{
			newrow += sheet->prefs.cursor_r2;
			newcol += sheet->prefs.cursor_c2;

			if ( newrow < 1 )
			{
				newrow = 1;
			}

			if ( newcol < 1 )
			{
				newcol = 1;
			}

			setCursorRange ( sheet->prefs.cursor_r1,
				sheet->prefs.cursor_c1,
				newrow, newcol, 0, 0, 0 );

			activeView->ensureVisible ( sheet,
				sheet->prefs.cursor_r2,
				sheet->prefs.cursor_c2 );
		}
		else
		{
			if ( key_ctrl || home || end )
			{
				newrow += sheet->prefs.cursor_r2;
				newcol += sheet->prefs.cursor_c2;
			}
			else
			{
				newrow += sheet->prefs.cursor_r1;
				newcol += sheet->prefs.cursor_c1;
			}

			if ( newrow < 1 )
			{
				newrow = 1;
			}

			if ( newcol < 1 )
			{
				newcol = 1;
			}

			setCursorRange ( newrow, newcol, newrow, newcol,
				1, 0, 0 );
		}

		return;
	}

	if ( return_true )
	{
		return;
	}

	if ( key_alt || key_ctrl || text.isEmpty () )
	{
		// Avoid interpreting Alt + F, Ctrl + O etc. as Unicode keys
		goto no_action;
	}

	// Unicode input so try to set in entry box

	if ( sheet->prefs.locked )
	{
		QMessageBox::critical ( this, "Error", "Sheet locked." );

		return;
	}

	if ( editCelltext->isEnabled () )
	{
		editCelltext->setText ( text );
		editCelltext->setFocus ();

		return;			// Actioned
	}

no_action:
	// Nothing to do so let the base class handle this event instead
	QWidget::keyPressEvent ( ev );
}

bool CedViewArea::event (
	QEvent		* const	ev
	)
{
	QKeyEvent	* const keyEvent = static_cast<QKeyEvent *>( ev );


	// This event is needed to catch the tab key to stop it being used
	// to change focus from the CedViewArea.

	if ( ev->type () != QEvent::KeyPress || ! keyEvent )
	{
		// Nothing to do so let the base class handle this event instead
		return QWidget::event ( ev );
	}

	switch ( keyEvent->key () )
	{
	case Qt::Key_Tab:
		int		r, c;
		CedSheet	* sheet;


		sheet = mainwindow->projectGetSheet ();

		if ( ! sheet )
		{
			return true;	// Processed event
		}

		r = sheet->prefs.cursor_r2;
		c = sheet->prefs.cursor_c2;

		if ( keyEvent->modifiers () & Qt::ControlModifier )
		{
			c--;
		}
		else
		{
			c++;
		}

		mainwindow->setCursorRange ( r, c, r, c, 1, 0, 0 );

		return true;		// Processed event
	}

	// Nothing to do so let the base class handle this event instead
	return QWidget::event ( ev );
}

void CedView::wheelEvent (
	QWheelEvent	* const	ev
	)
{
	if ( ev->orientation () == Qt::Horizontal )
	{
		ev->ignore ();

		return;
	}


	int		delta = getVisibleRows () / 8;


	delta = MAX ( delta, 1 );

	if ( ev->delta () > 0 )
	{
		delta = -delta;
	}

	vscroll->setSliderPosition ( vscroll->value () + delta );
}

int MainWindow::isEditingCellText ()
{
	if ( editCelltext->hasFocus () )
	{
		return 1;
	}

	return 0;
}

void MainWindow::insertCellText (
	char	const * const	text
	)
{
	if ( ! text || 0 == text[0] )
	{
		return;
	}

	editCelltext->insert ( mtQEX::qstringFromC ( text ) );
}

void CedViewArea::mouseEventRouter (
	QMouseEvent	* const	ev,
	int			caller	// 0 = Press 2 = Move
	)
{
	int		row_start = 0, col_start = 0, b = 0,
			xx, yy, vscroll, hscroll, r, c,
			r1 = 1, c1 = 1, r2 = 1, c2 = 1;
	CedSheet	* sheet;
	CuiRender	* crender;


	sheet = mainwindow->projectGetSheet ();
	if ( ! sheet )
	{
		return;
	}

	if ( ev->buttons () & Qt::LeftButton )
	{
		b = 1;
	}
	else if ( ev->buttons () & Qt::RightButton )
	{
		b = 3;
	}
	else
	{
		// We only look at left/right clicks/drags

		return;
	}

	if ( 2 == caller && mainwindow->isEditingCellText () )
	{
		return;
	}

	crender = mainwindow->projectGetRender ();
	vscroll = cedview->getVScrollValue ();
	hscroll = cedview->getHScrollValue ();
	xx = ev->x ();
	yy = ev->y ();

	switch ( areaID )
	{
	case CEDVIEW_AREA_BR:
		row_start = vscroll;
		col_start = hscroll;
		break;
	case CEDVIEW_AREA_BL:
		row_start = vscroll;
		col_start = sheet->prefs.split_c1;
		break;
	case CEDVIEW_AREA_TL:
		row_start = sheet->prefs.split_r1;
		col_start = sheet->prefs.split_c1;
		break;
	case CEDVIEW_AREA_TR:
		row_start = sheet->prefs.split_r1;
		col_start = hscroll;
		break;

	case CEDVIEW_TITLE_C1:
		col_start = sheet->prefs.split_c1;
		caller ++;
		break;
	case CEDVIEW_TITLE_C2:
		col_start = hscroll;
		caller ++;
		break;
	case CEDVIEW_TITLE_R1:
		row_start = sheet->prefs.split_r1;
		caller ++;
		break;
	case CEDVIEW_TITLE_R2:
		row_start = vscroll;
		caller ++;
		break;

	default:
		return;
	}

	switch ( caller )
	{
	case 0:	// Mouse press on sheet area
		r = cui_ren_row_from_y ( row_start, crender, yy );
		c = cui_ren_column_from_x ( col_start, crender, xx );

		if ( mainwindow->isEditingCellText () )
		{
			char		txt[128] = {0}, * tp = txt;
			CedCellRef	ref;


			// Get cell reference and put it into the cell formula

			if ( 3 == b )
			{
				*tp++ = ':';
			}

			if ( ev->modifiers () & Qt::ShiftModifier )
			{
				// Absolute column
				ref.row_m = 0;
				ref.row_d = r;
			}
			else
			{
				// Relative row
				ref.row_m = 1;
				ref.row_d = r - sheet->prefs.cursor_r1;
			}

			if ( ev->modifiers () & Qt::ControlModifier )
			{
				// Absolute column
				ref.col_m = 0;
				ref.col_d = c;
			}
			else
			{
				// Relative column
				ref.col_m = 1;
				ref.col_d = c - sheet->prefs.cursor_c1;
			}

			ced_cellreftostr ( tp, &ref );

			mainwindow->insertCellText ( txt );

			return;
		}

		cedview->setFocus ();

		if ( 1 == b )		// Left click - set first corner
		{
			mainwindow->setCursorRange ( r, c, r, c, 1, 0, 0 );
		}
		else if ( 3 == b )	// Right click - set second corner
		{
			mainwindow->setCursorRange ( sheet->prefs.cursor_r1,
				sheet->prefs.cursor_c1, r, c, 0, 0, 0 );
		}
		break;

	case 1:	// Mouse press on header area
		cedview->setFocus ();

		if ( 1 == b )
		{
			if ( col_start )
			{
				mainwindow->projectGetSheetGeometry ( &r2,
					NULL );

				c1 = c2 = cui_ren_column_from_x ( col_start,
					crender, xx );
			}
			else if ( row_start )
			{
				r1 = r2 = cui_ren_row_from_y ( row_start,
					crender, yy );

				mainwindow->projectGetSheetGeometry ( NULL,
					&c2 );
			}
			else
			{
				return;
			}
		}

		if ( 3 == b ) // Right click - change second corner only
		{
			if ( col_start )
			{
				c1 = sheet->prefs.cursor_c1;

				mainwindow->projectGetSheetGeometry ( &r2,
					NULL );

				c2 = cui_ren_column_from_x ( col_start,
					crender, xx );
			}
			else if ( row_start )
			{
				r1 = sheet->prefs.cursor_r1;
				r2 = cui_ren_row_from_y ( row_start, crender,
					yy );
				mainwindow->projectGetSheetGeometry ( NULL,
					&c2 );
			}
			else
			{
				return;
			}
		}

		if ( r1 < 1 ) { r1 = 1; }
		if ( r2 < 1 ) { r2 = 1; }
		if ( c1 < 1 ) { c1 = 1; }
		if ( c2 < 1 ) { c2 = 1; }

		mainwindow->setCursorRange ( r1, c1, r2, c2, 0, 0, 0 );
		break;

	case 2:	// Mouse movement on sheet area
		r = cui_ren_row_from_y ( row_start, crender, yy );
		c = cui_ren_column_from_x ( col_start, crender, xx );

		mainwindow->setCursorRange ( sheet->prefs.cursor_r1,
			sheet->prefs.cursor_c1, r, c, 0, 0, 0 );
		break;

	case 3: // Mouse movement on header area
		if ( col_start )
		{
			r1 = 1;
			c1 = sheet->prefs.cursor_c1;

			mainwindow->projectGetSheetGeometry ( &r2, NULL );

			c2 = cui_ren_column_from_x ( col_start, crender, xx );
		}
		else if ( row_start )
		{
			r1 = sheet->prefs.cursor_r1;
			c1 = 1;
			r2 = cui_ren_row_from_y ( row_start, crender, yy );

			mainwindow->projectGetSheetGeometry ( NULL, &c2 );
		}
		else
		{
			return;
		}

		if ( r1 < 1 ) { r1 = 1; }
		if ( r2 < 1 ) { r2 = 1; }
		if ( c1 < 1 ) { c1 = 1; }
		if ( c2 < 1 ) { c2 = 1; }

		mainwindow->setCursorRange ( r1, c1, r2, c2, 0, 0, 0 );
		break;
	}
}

void CedViewArea::mousePressEvent (
	QMouseEvent	* const	ev
	)
{
	mouseEventRouter ( ev, 0 );
}

void CedViewArea::mouseMoveEvent (
	QMouseEvent	* const	ev
	)
{
	mouseEventRouter ( ev, 2 );
}

