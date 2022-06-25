/*
	Copyright (C) 2013-2021 Mark Tyler

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



#define	CEDVIEW_FRZ_PANE_TOT	7



CedView::CedView ( MainWindow * mw )
	:
	m_mainwindow	( mw )
{
	int const rcs [ CEDVIEW_AREA_TOTAL ][ 4 ] = {

		// Order as per CEDVIEW_*

		{ 1, 1, QSizePolicy::Preferred, QSizePolicy::Preferred },
		{ 1, 3, QSizePolicy::Preferred, QSizePolicy::Preferred },
		{ 3, 1, QSizePolicy::Preferred, QSizePolicy::Preferred },
		{ 3, 3, QSizePolicy::Expanding, QSizePolicy::Expanding },

		{ 0, 0, QSizePolicy::Preferred, QSizePolicy::Preferred },

		// Column / Row headers

		{ 0, 1, QSizePolicy::Preferred, QSizePolicy::Fixed },
		{ 0, 3, QSizePolicy::Preferred, QSizePolicy::Fixed },
		{ 1, 0, QSizePolicy::Fixed, QSizePolicy::Preferred },
		{ 3, 0, QSizePolicy::Fixed, QSizePolicy::Preferred },

		// Frozen pane areas

		{ 0, 2, QSizePolicy::Fixed, QSizePolicy::Preferred },
		{ 2, 0, QSizePolicy::Preferred, QSizePolicy::Fixed },
		{ 1, 2, QSizePolicy::Preferred, QSizePolicy::Preferred },
		{ 3, 2, QSizePolicy::Preferred, QSizePolicy::Preferred },
		{ 2, 1, QSizePolicy::Preferred, QSizePolicy::Preferred },
		{ 2, 3, QSizePolicy::Preferred, QSizePolicy::Preferred },
		{ 2, 2, QSizePolicy::Fixed, QSizePolicy::Fixed }

			};


	setSizePolicy ( QSizePolicy ( QSizePolicy::Expanding,
		QSizePolicy::Expanding ) );

	layout = new QGridLayout;
	setLayout ( layout );
	layout->setContentsMargins ( 0, 0, 0, 0 );
	layout->setSpacing ( 0 );

	vscroll = new QScrollBar ( Qt::Vertical );
	layout->addWidget ( vscroll, 3, 4, 1, 1 );
	connect ( vscroll, SIGNAL ( valueChanged ( int ) ),
		this, SLOT ( vscrollMove ( int ) ) );
	connect ( vscroll, SIGNAL ( actionTriggered ( int ) ),
		this, SLOT ( vscrollAction ( int ) ) );

	hscroll = new QScrollBar ( Qt::Horizontal );
	layout->addWidget ( hscroll, 4, 3, 1, 1 );
	connect ( hscroll, SIGNAL ( valueChanged ( int ) ),
		this, SLOT ( hscrollMove ( int ) ) );
	connect ( hscroll, SIGNAL ( actionTriggered ( int ) ),
		this, SLOT ( hscrollAction ( int ) ) );

	for ( int i = 0; i < CEDVIEW_AREA_TOTAL; i++ )
	{
		area [ i ] = new CedViewArea ( this, i );

		layout->addWidget ( area [ i ], rcs[i][0], rcs[i][1], 1, 1 );

		area [ i ]->setSizePolicy ( QSizePolicy (
			(QSizePolicy::Policy) rcs[i][2],
			(QSizePolicy::Policy) rcs[i][3] ) );
	}

	// Allow corner to have focus
	area [ CEDVIEW_AREA_CORNER ]->setFocusPolicy ( Qt::WheelFocus );

	for ( int i = 0; i < CEDVIEW_FRZ_PANE_TOT; i++ )
	{
		area [ CEDVIEW_FRZ_COL + i ]->setMinimumSize ( 1, 1 );
	}
}

void CedView::setFocus ()
{
	area [ CEDVIEW_AREA_CORNER ]->setFocus ( Qt::OtherFocusReason );
}

bool CedView::hasFocus ()
{
	return area [ CEDVIEW_AREA_CORNER ]->hasFocus ();
}

void CedView::vscrollMove ( int value )
{
	int		delta;
	CuiRender	* crender = m_mainwindow->projectGetRender ();


	if ( ! crender->sheet() )
	{
		return;
	}

	delta = value - vscrollLast;
	if ( delta == 0 )
	{
		return;		// Nothing to do
	}

	if ( m_mainwindow->isMainView ( this ) )
	{
		crender->sheet()->prefs.start_row = value;
	}


	/*
	Find out how many rows fit on the screen.
	NOTE - This is slightly different to columns because rows are always of
	a fixed height and less work is needed.
	*/

	if ( abs ( delta ) < getVisibleRows ( crender ) )
	{
		int		ph;


		// We are optimizing by scrolling
		// (accelerated on most systems)

		ph = crender->y_from_row ( vscrollLast, vscrollLast +
			abs ( delta ) );

		if ( delta > 0 )
		{
			ph = -ph;
		}

		area [ CEDVIEW_TITLE_R2 ]->scroll ( 0, ph );
		area [ CEDVIEW_AREA_BR ]->scroll ( 0, ph );
		area [ CEDVIEW_AREA_BL ]->scroll ( 0, ph );
	}
	else
	{
		area [ CEDVIEW_TITLE_R2 ]->update ();
		area [ CEDVIEW_AREA_BL ]->update ();
		area [ CEDVIEW_AREA_BR ]->update ();
	}

	vscrollLast = value;
}

void CedView::vscrollAction (
	int	const	action
	)
{
	int		delta = 0,
			value = vscroll->value ();	// Original value
	CuiRender	* const	crender = m_mainwindow->projectGetRender ();


	if ( ! crender->sheet() )
	{
		return;
	}

	// Find out how many rows fit on the screen
	int const page = crender->row_from_y ( value,
		area [ CEDVIEW_AREA_BR ]->height () ) - value;

	switch ( action )
	{
	case QAbstractSlider::SliderPageStepAdd:
		delta = page;
		break;

	case QAbstractSlider::SliderPageStepSub:
		delta = -page;
		break;

	default:
		// We only need to manually intervene for page changes
		return;
	}

	value += delta;

	vscroll->setSliderPosition ( value );
}

void CedView::hscrollMove (
	int	const	value
	)
{
	int		max_col, delta, pw = 0;
	CuiRender	* const	crender = m_mainwindow->projectGetRender ();


	if ( ! crender->sheet() )
	{
		return;
	}

	delta = value - hscrollLast;
	if ( delta == 0 )
	{
		return;		// Nothing to do
	}

	if ( m_mainwindow->isMainView ( this ) )
	{
		crender->sheet()->prefs.start_col = value;
	}

	if ( delta > 0 )
	{
		// Scrolling to the right
		max_col = crender->column_from_x ( hscrollLast,
			area [ CEDVIEW_AREA_BR ]->width () - 1 );

		if ( max_col > value )
		{
			// Old & new area rectangles overlap, so scroll
			pw = -crender->x_from_column ( hscrollLast, value );
		}
	}
	else	// delta < 0
	{
		// Scrolling to the left
		max_col = crender->column_from_x ( value,
			area [ CEDVIEW_AREA_BR ]->width () - 1 );

		if ( max_col > hscrollLast )
		{
			// Old & new area rectangles overlap, so scroll
			pw = crender->x_from_column ( value, hscrollLast );
		}
	}

	if ( pw != 0 )
	{
		area [ CEDVIEW_TITLE_C2 ]->scroll ( pw, 0 );
		area [ CEDVIEW_AREA_BR ]->scroll ( pw, 0 );
		area [ CEDVIEW_AREA_TR ]->scroll ( pw, 0 );
	}
	else
	{
		area [ CEDVIEW_TITLE_C2 ]->update ();
		area [ CEDVIEW_AREA_TR ]->update ();
		area [ CEDVIEW_AREA_BR ]->update ();
	}

	hscrollLast = value;
}

int CedView::getPageColumnsLeft (
	int	const	column
	)
{
	int delta = m_mainwindow->projectGetRender ()->column_from_x_backwards (
		column, area [ CEDVIEW_AREA_BR ]->width () - 1 ) - column;

	if ( delta > -1 )
	{
		delta = -1;
	}

	return delta;
}

int CedView::getPageColumnsRight (
	int	const	column
	)
{
	int delta = m_mainwindow->projectGetRender ()->column_from_x ( column,
		area [ CEDVIEW_AREA_BR ]->width () - 1 ) - column;

	if ( delta < 1 )
	{
		delta = 1;
	}

	return delta;
}

void CedView::hscrollAction (
	int	const	action
	)
{
	int		value = hscroll->value (),	// Original value
			delta = 0;
	CuiRender	* const	crender = m_mainwindow->projectGetRender ();


	if ( ! crender->sheet() )
	{
		return;
	}

	switch ( action )
	{
	case QAbstractSlider::SliderPageStepAdd:
		delta = getPageColumnsRight ( value );
		break;

	case QAbstractSlider::SliderPageStepSub:
		delta = getPageColumnsLeft ( value );
		break;

	default:
		// We only need to manually intervene for page changes
		return;
	}

	value += delta;

	hscroll->setSliderPosition ( value );
}

void CedView::setPanes ()
{
	CedSheet	* const sheet = m_mainwindow->projectGetSheet ();

	if ( ! sheet )
	{
		return;
	}

	CuiRender	* const crender = m_mainwindow->projectGetRender ();
	int		const	min_row = sheet->prefs.split_r1;
	int		const	max_row = sheet->prefs.split_r2;
	int		const	min_col = sheet->prefs.split_c1;
	int		const	max_col = sheet->prefs.split_c2;

	if (	min_row > max_row ||
		min_col > max_col
		)
	{
		return;
	}

	if (	min_col == 0 ||
		min_row == 0
		)
	{
		area[ CEDVIEW_AREA_TL ]->hide ();
	}
	else
	{
		area[ CEDVIEW_AREA_TL ]->show ();
	}

	if (	min_col == 0 &&
		min_row == 0
		)
	{
		area[ CEDVIEW_FRZ_ROW ]->hide ();
		area[ CEDVIEW_FRZ_COL ]->hide ();
		area[ CEDVIEW_FRZ_V_TOP ]->hide ();
		area[ CEDVIEW_FRZ_V_BOTTOM ]->hide ();
		area[ CEDVIEW_FRZ_H_LEFT ]->hide ();
		area[ CEDVIEW_FRZ_H_RIGHT ]->hide ();
		area[ CEDVIEW_FRZ_CORNER ]->hide ();
	}
	else
	{
		if ( min_col == 0 )
		{
			area[ CEDVIEW_FRZ_H_LEFT ]->hide ();
		}
		else
		{
			int const w = crender->x_from_column ( min_col, max_col
				+ 1 );

			area[ CEDVIEW_FRZ_H_LEFT ]->setMinimumWidth ( w );
			area[ CEDVIEW_FRZ_H_LEFT ]->show ();
		}

		if ( min_row == 0 )
		{
			area[ CEDVIEW_FRZ_V_TOP ]->hide ();
		}
		else
		{
			int const h = (max_row - min_row + 1) *
				crender->row_height();

			area[ CEDVIEW_FRZ_V_TOP ]->setMinimumHeight ( h );
			area[ CEDVIEW_FRZ_V_TOP ]->show ();
		}

		area[ CEDVIEW_FRZ_ROW ]->show ();
		area[ CEDVIEW_FRZ_COL ]->show ();
		area[ CEDVIEW_FRZ_V_BOTTOM ]->show ();
		area[ CEDVIEW_FRZ_H_RIGHT ]->show ();
		area[ CEDVIEW_FRZ_CORNER ]->show ();
	}

	if ( min_col == 0 )
	{
		area[ CEDVIEW_AREA_BL ]->hide ();
		area[ CEDVIEW_TITLE_C1 ]->hide ();
	}
	else
	{
		area[ CEDVIEW_AREA_BL ]->show ();
		area[ CEDVIEW_TITLE_C1 ]->show ();
	}

	if ( min_row == 0 )
	{
		area[ CEDVIEW_AREA_TR ]->hide ();
		area[ CEDVIEW_TITLE_R1 ]->hide ();
	}
	else
	{
		area[ CEDVIEW_AREA_TR ]->show ();
		area[ CEDVIEW_TITLE_R1 ]->show ();
	}

	area[ CEDVIEW_TITLE_C1 ]->setMinimumHeight ( crender->row_height() );
	area[ CEDVIEW_TITLE_C2 ]->setMinimumHeight ( crender->row_height() );
}



#define ADJ_PAGE_SIZE 20



void CedView::setScrollbars ()
{
	CedSheet	* const	sheet = m_mainwindow->projectGetSheet ();


	if ( ! sheet )
	{
		return;
	}

	int		r, c;

	m_mainwindow->projectGetSheetGeometry ( &r, &c );

	// Vertical
	int min = sheet->prefs.split_r2 + 1;

	int max = MAX ( sheet->prefs.cursor_r1, sheet->prefs.cursor_r2 );
	max = MAX ( r, max );
	max = MAX ( min, max );

	vscroll->setMinimum ( min );
	vscroll->setMaximum ( max );
	vscroll->setPageStep ( ADJ_PAGE_SIZE );

	vscrollLast = vscroll->value ();

	// Horizontal
	min = sheet->prefs.split_c2 + 1;

	max = MAX ( sheet->prefs.cursor_c1, sheet->prefs.cursor_c2 );
	max = MAX ( c, max );
	max = MAX ( min, max );

	hscroll->setMinimum ( min );
	hscroll->setMaximum ( max );
	hscroll->setPageStep ( ADJ_PAGE_SIZE );

	hscrollLast = hscroll->value ();

	setRowHeaderWidth ();
}

void CedView::setRowHeaderWidth ()
{
	CuiRender * const crender = m_mainwindow->projectGetRender ();

	be_cedrender_set_header_width ( crender,
		vscroll->maximum () + vscroll->pageStep () );

	area [ CEDVIEW_TITLE_R1 ]->setMinimumWidth (
		crender->row_header_width() );
	area [ CEDVIEW_TITLE_R2 ]->setMinimumWidth (
		crender->row_header_width() );
}

void CedView::updateRedraw ()
{
	for ( int i = 0; i < CEDVIEW_AREA_TOTAL; i++ )
	{
		area [ i ]->update ();
	}
}

void CedView::reconfigure ()
{
	CedSheet	* const sheet = m_mainwindow->projectSetSheet ();

	if ( sheet )
	{
		hscroll->blockSignals ( true );
		vscroll->blockSignals ( true );

		setScrollbars ();

		hscroll->blockSignals ( false );
		vscroll->blockSignals ( false );

		setRowHeaderWidth ();
		setPanes ();

		hscrollLast = MAX (sheet->prefs.start_col, hscroll->minimum ());
		vscrollLast = MAX (sheet->prefs.start_row, vscroll->minimum ());

		hscroll->setValue ( hscrollLast );
		vscroll->setValue ( vscrollLast );
	}

	updateRedraw ();
}

void CedView::setBell ()
{
	int	const	tmp = bellState[1];

	bellState[1] = bellState[0];
	bellState[0] = tmp;

	area [ CEDVIEW_AREA_CORNER ]->update ();
}

int CedView::getBell ()
{
	return bellState[0];
}

int CedView::getVScrollValue ()
{
	return vscroll->value ();
}

int CedView::getHScrollValue ()
{
	return hscroll->value ();
}

CedViewArea::CedViewArea (
	CedView		* const	cv,
	int		const	id
	)
	:
	cedview		( cv ),
	areaID		( id )
{
	setAttribute ( Qt::WA_OpaquePaintEvent );
	setAttribute ( Qt::WA_NoSystemBackground );
}

static void expose_cb (
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	unsigned char	const * const	rgb,
	int		const	pix_size,
	void		* const	user_data
	)
{
	auto const format = (pix_size == 4) ? QImage::Format_RGB32 :
		QImage::Format_RGB888;

	QImage		im ( (uchar const *)rgb, w, h, w * pix_size, format );
	QPainter	p ( static_cast<CedViewArea *>(user_data) );

	p.drawImage ( QPoint ( x, y ), im );
}

void CedViewArea::paintEvent (
	QPaintEvent	* const	ev
	)
{
	int const px = ev->rect ().x ();
	int const py = ev->rect ().y ();
	int const pw = ev->rect ().width ();
	int const ph = ev->rect ().height ();

	if ( pw < 1 || ph < 1 )
	{
		return;
	}

	CuiRender	* const ren = cedview->mainwindow()->projectGetRender();
	CedSheet	* const sheet = ren->sheet();
	int		const	start_row = cedview->getVScrollValue ();
	int		const	start_col = cedview->getHScrollValue ();
	int		col = 0;

	if ( ! sheet )
	{
		// Don't try to render if anything is missing - leave as grey

		col = 256 + 182;
	}
	else switch ( areaID )
	{
	case CEDVIEW_AREA_TL:
		ren->expose_sheet( sheet->prefs.split_r1, sheet->prefs.split_c1,
			px, py, pw, ph, expose_cb, this );
		break;

	case CEDVIEW_AREA_TR:
		ren->expose_sheet ( sheet->prefs.split_r1, start_col,
			px, py, pw, ph, expose_cb, this );
		break;

	case CEDVIEW_AREA_BL:
		ren->expose_sheet ( start_row, sheet->prefs.split_c1,
			px, py, pw, ph, expose_cb, this );
		break;

	case CEDVIEW_AREA_BR:
		ren->expose_sheet ( start_row, start_col,
			px, py, pw, ph, expose_cb, this );
		break;

	case CEDVIEW_TITLE_R1:
		ren->expose_row_header ( sheet->prefs.split_r1,
			px, py, pw, ph, expose_cb, this );
		break;

	case CEDVIEW_TITLE_R2:
		ren->expose_row_header ( start_row,
			px, py, pw, ph, expose_cb, this );
		break;

	case CEDVIEW_TITLE_C1:
		ren->expose_column_header ( sheet->prefs.split_c1,
			px, py, pw, ph, expose_cb, this );
		break;

	case CEDVIEW_TITLE_C2:
		ren->expose_column_header ( start_col,
			px, py, pw, ph, expose_cb, this );
		break;

	case CEDVIEW_AREA_CORNER:
		col = 256 + cedview->getBell ();
		break;

	default:
		col = 256;		// Frozen pane areas
	}

	if ( col )
	{
		void * const rgb = malloc ( (unsigned)pw * (unsigned)ph * 3 );

		if ( ! rgb )
		{
			return;
		}
		else
		{
			memset ( rgb, col & 255, (unsigned)pw * (unsigned)ph*3);

			std::unique_ptr<QImage> const im (
				new QImage ( (const uchar *)rgb, pw, ph, pw * 3,
				QImage::Format_RGB888 ) );

			QPainter p ( this );
			p.drawImage ( QPoint ( px, py ), *im.get() );

		}	// Delete "im" before rgb image destroy

		free ( rgb );
	}
}

int MainWindow::isMainView (
	CedView		* const	v
	)
{
	if ( v == viewMain )
	{
		return 1;
	}

	return 0;
}

int CedView::getVisibleRows (
	CuiRender	* crender
	)
{
	if ( ! crender )
	{
		crender = m_mainwindow->projectGetRender ();
	}

	int const tot = area [ CEDVIEW_AREA_BR ]->height () /
		crender->row_height();

	return MAX ( tot, 1 );
}

void CedView::updateGeometry ()
{
	setScrollbars ();
}

void CedView::redrawArea (
	CedSheet	* const	sheet,
	int		const	nr1,
	int		const	nc1,
	int		const	nr2,
	int		const	nc2
	)
{
	int		r1_top = -1,
			r1_bot = -1,	// Row min for top/bottom area
			c1_left = -1,
			c1_right = -1,
			px1_left, pw2_left, px1_right, pw2_right,
			py1_top, ph2_top, py1_bot, ph2_bot;

	int	const	vscroll_val = vscroll->value ();
	int	const	hscroll_val = hscroll->value ();
	CuiRender	* const	crender = m_mainwindow->projectGetRender ();


	// Ensure r1, c1 = min r2, c2 = max
	int	const	r1 = MIN ( nr1, nr2 );
	int	const	r2 = MAX ( nr2, nr1 );
	int	const	c1 = MIN ( nc1, nc2 );
	int	const	c2 = MAX ( nc2, nc1 );

	// If area is poorly set, update all
	if (	r1 < 1 ||
		c1 < 1 ||
		r2 < 1 ||
		c2 < 1
		)
	{
		updateRedraw ();

		return;
	}

	// Calculate maximum visible row/column
	int	max_col = hscroll_val;
	int	max_row = vscroll_val;

	{
		CedViewArea * w = area[ CEDVIEW_AREA_BR ];

		if ( w->width () > 0 )
		{
			max_col = crender->column_from_x(max_col, w->width()-1);
		}

		if ( w->height () > 0 )
		{
			max_row = crender->row_from_y( max_row, w->height() -1);
		}
	}

	// Calculate extent of cursor on visible areas (updating headers)

	if (	c1 <= sheet->prefs.split_c2 &&
		c2 >= sheet->prefs.split_c1
		)
	{
		// Cursor overlaps left vertical area

		c1_left = MAX ( c1, sheet->prefs.split_c1 );
		int const c2_left = MIN ( c2, sheet->prefs.split_c2 );

		px1_left = crender->x_from_column ( sheet->prefs.split_c1,
			c1_left );
		pw2_left = crender->x_from_column ( sheet->prefs.split_c1,
			c2_left + 1 ) - px1_left;

		CedViewArea * w = area[ CEDVIEW_TITLE_C1 ];
		w->update ( px1_left, 0, pw2_left, w->height () );
	}

	if (	c1 <= max_col &&
		c2 >= hscroll_val
		)
	{
		// Cursor overlaps right vertical area

		c1_right = MAX ( c1, hscroll_val );
		int const c2_right = MIN ( c2, max_col );

		px1_right = crender->x_from_column ( hscroll_val, c1_right );
		pw2_right = crender->x_from_column ( hscroll_val, c2_right + 1 )
			- px1_right;

		CedViewArea * w = area[ CEDVIEW_TITLE_C2 ];
		w->update ( px1_right, 0, pw2_right, w->height () );
	}

	if (	r1 <= sheet->prefs.split_r2 &&
		r2 >= sheet->prefs.split_r1
		)
	{
		// Cursor overlaps top horizontal area

		r1_top = MAX ( r1, sheet->prefs.split_r1 );
		int const r2_top = MIN ( r2, sheet->prefs.split_r2 );

		py1_top = crender->y_from_row ( sheet->prefs.split_r1, r1_top );
		ph2_top = crender->y_from_row ( sheet->prefs.split_r1, r2_top +
			1 ) - py1_top;

		CedViewArea * w = area[ CEDVIEW_TITLE_R1 ];
		w->update ( 0, py1_top, w->width (), ph2_top );
	}

	if (	r1 <= max_row &&
		r2 >= vscroll_val
		)
	{
		// Cursor overlaps bottom horizontal area

		r1_bot = MAX ( r1, vscroll_val );
		int const r2_bot = MIN ( r2, max_row );

		py1_bot = crender->y_from_row ( vscroll_val, r1_bot );
		ph2_bot = crender->y_from_row ( vscroll_val, r2_bot + 1 ) -
			py1_bot;

		CedViewArea * w = area[ CEDVIEW_TITLE_R2 ];
		w->update ( 0, py1_bot, w->width (), ph2_bot );
	}

	if (	r1_bot != -1 &&
		c1_right != -1
		)
	{
		area[ CEDVIEW_AREA_BR ]->update ( px1_right, py1_bot, pw2_right,
			ph2_bot );
	}

	if (	r1_top != -1 &&
		c1_right != -1
		)
	{
		area[ CEDVIEW_AREA_TR ]->update ( px1_right, py1_top, pw2_right,
			ph2_top );
	}

	if (	r1_bot != -1 &&
		c1_left != -1
		)
	{
		area[ CEDVIEW_AREA_BL ]->update ( px1_left, py1_bot, pw2_left,
			ph2_bot );
	}

	if (	r1_top != -1 &&
		c1_left != -1
		)
	{
		area[ CEDVIEW_AREA_TL ]->update ( px1_left, py1_top, pw2_left,
			ph2_top );
	}
}

void CedView::ensureVisible (
	CedSheet	* const	sheet,
	int		const	row,
	int		const	column
	)
{
	int	const	vscroll_val = vscroll->value ();
	int	const	hscroll_val = hscroll->value ();
	CuiRender * const crender = m_mainwindow->projectGetRender ();


	if (	! sheet		||
		row < 1		||
		column < 1
		)
	{
		return;
	}

	if ( row >= vscroll->minimum () )
	{
		// Move the scrollbars if the cursor is not visible
		if ( row < vscroll_val )
		{
			vscroll->setValue ( row );
		}
		else
		{
			// Last row visible
			int last_row = crender->row_from_y( vscroll_val,
				area[ CEDVIEW_AREA_BR ]->height () );

			if ( last_row <= row )
			{
				int new_row = row - 1;

				for ( ; ; new_row -- )
				{
					last_row = crender->row_from_y( new_row,
						area[CEDVIEW_AREA_BR]->height()
						);

					if ( last_row <= row )
					{
						break;
					}
				}

				new_row ++;

				vscroll->setValue ( new_row );
			}
		}
	}

	if ( column >= hscroll->minimum () )
	{

		if ( column < hscroll_val )
		{
			hscroll->setValue ( column );
		}
		else
		{
			// Last row visible
			int last_col = crender->column_from_x ( hscroll_val,
				area [ CEDVIEW_AREA_BR ]->width () );

			if ( last_col <= column )
			{
				int new_col = column - 1;

				for ( ; ; new_col -- )
				{
					last_col = crender->column_from_x (
						new_col, area[ CEDVIEW_AREA_BR
						]->width () );

					if ( last_col <= column )
					{
						break;
					}
				}

				new_col ++;

				hscroll->setValue ( new_col );
			}
		}
	}
}

