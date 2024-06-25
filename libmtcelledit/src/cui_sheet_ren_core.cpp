/*
	Copyright (C) 2011-2024 Mark Tyler

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

#include "cui_sheet.h"



int SheetRenCore::init_expose (
	int	const	row_start,
	int	const	col_start,
	int	const	x,
	int	const	y,
	int	const	w,
	int	const	h
	)
{
	m_row_start = row_start;
	m_col_start = col_start;
	m_x = x;
	m_y = y;
	m_w = w;
	m_h = h;

	m_r1 = viewren()->row_from_y ( row_start, y );
	m_r2 = viewren()->row_from_y ( row_start, (y + h - 1) );

	free_column_arrays ();

	if ( viewren()->init_column_array ( col_start, x, w, m_c1, m_c2,
		&m_col_x_px, &m_col_w_px )
		)
	{
		return 1;
	}

	m_cell_act = (unsigned char *)calloc( (size_t)(m_c2 - m_c1 + 1),
		sizeof(m_cell_act[0]) );

	if ( ! m_cell_act )
	{
		return 1;
	}

	/*
	NOTE: we don't use ced_sheet_cursor_max_min () because it disrespects a
	cursor of 0,0 which is useful for hiding the cursor when editing a
	background colour preference.
	*/

	m_cur_row1 = MIN (viewren()->sheet()->prefs.cursor_r1,
		viewren()->sheet()->prefs.cursor_r2 );

	m_cur_row2 = MAX (viewren()->sheet()->prefs.cursor_r1,
		viewren()->sheet()->prefs.cursor_r2 );

	m_cur_col1 = MIN (viewren()->sheet()->prefs.cursor_c1,
		viewren()->sheet()->prefs.cursor_c2 );

	m_cur_col2 = MAX (viewren()->sheet()->prefs.cursor_c1,
		viewren()->sheet()->prefs.cursor_c2 );

	if (	m_cur_row1 <= m_r2 && m_cur_row2 >= m_r1 &&
		m_cur_col1 <= m_c2 && m_cur_col2 >= m_c1
		)
	{
		m_draw_cursor = 1;
	}
	else
	{
		m_draw_cursor = 0;
	}

	return 0;
}

int SheetRenCore::init_expose_sheet (
	int	const	r1,
	int	const	c1,
	int	const	r2,
	int	const	c2
	)
{
	if ( init_column_arrays ( c1, (size_t)(c2 - c1 + 1) ) )
	{
		return 1;
	}

	m_r1 = r1;
	m_c1 = c1;
	m_r2 = r2;
	m_c2 = c2;

	return 0;
}

void SheetRenCore::free_column_arrays ()
{
	free ( m_cell_act );
	m_cell_act = nullptr;

	free ( m_col_x_px );
	m_col_x_px = nullptr;

	free ( m_col_w_px );
	m_col_w_px = nullptr;
}

int SheetRenCore::init_column_arrays (
	int	const	c1,
	size_t	const	tot
	)
{
	free_column_arrays ();

	m_cell_act = (unsigned char *)calloc ( tot, sizeof(m_cell_act[0]) );
	if ( ! m_cell_act )
	{
		return 1;
	}

	if ( m_viewren->init_column_array ( c1, tot, &m_col_x_px, &m_col_w_px ))
	{
		return 1;
	}

	return 0;
}

void SheetRenCore::prepare_ren_expansion (
	CedCell const * const	cell,
	int		const	row,
	int		const	col,
	int		const	col_exp
	)
{
	int tc1 = col_exp;
	int tc2 = col_exp;

	// Needed to reserve cell to stop text from the right encroaching.
	set_cell_act ( col_exp );

	// Expansion to the left
	if (	cell_text_justify() == CED_CELL_JUSTIFY_CENTER ||
		cell_text_justify() == CED_CELL_JUSTIFY_RIGHT
		)
	{
		for (	int i = col_exp - 1;
			i >= c1();
			i-- )
		{
			// Is this cell free for us to expand over?
			if ( cell_act_col ( i ) != 0 )
			{
				break;
			}

			// We are going leftwards so we don't need to reserve
			// the cell as the start cell was reserved above and
			// we render from left to right.
//			set_cell_act ( i );

			// Adjust X coords one cell to the left, expand width
			int const delta = col_w_pix( i );
			set_cell_x_px ( cell_x_px() - delta );
			set_cell_width_px ( cell_width_px() + delta );

			tc1 = i;		// The new leftmost column

			if ( text_x_px() >= cell_x_px() )
			{
				// Text is now completely visible so stop
				break;
			}
		}
	}

	// Expansion to the right
	if (	cell_text_justify() == CED_CELL_JUSTIFY_CENTER ||
		cell_text_justify() == CED_CELL_JUSTIFY_LEFT
		)
	{
		// Only ever attempt to expand if the text has already been
		// truncated to the right.

		if (	(cell_x_px() + cell_width_px()) <
			(text_x_px() + text_width_px()) )
		{
			for ( int i = col_exp + 1; i <= c2(); i++ )
			{
				// Is this cell free for us to expand over?
				if ( cell_act_col ( i ) != 0 )
				{
					break;
				}

				// Reserve this cell to stop text coming from
				// the right and overwriting.
				set_cell_act ( i );

				tc2 = i;	// The new rightmost column

				// Expand width
				set_cell_width_px ( cell_width_px() +
					col_w_pix( i ) );

				// Have we expanded beyond the text width?
				if (	(cell_x_px() + cell_width_px()) >=
					(text_x_px() + text_width_px())
					)
				{
					break;
				}
			}
		}
	}

	if (	row < cur_row1() ||
		row > cur_row2() ||
		tc2 < cur_col1() ||
		tc1 > cur_col2()
		)
	{
		// No selection area means we render in one go
		be_ren_text ( cell, row, tc1 );
	}
	else
	{
		int		ci[3] = { -1, -1, -1 };
		int		xi[3];
		int		wi[3];
/*
Expansion took place AND cursor interfering: split required.
We now have up to 3 renders as we have the cursor area to traverse:
?i[0]	Text to left of cursor
?i[1]	Text over cursor (or skipped if the row/col sits outside the selection)
?i[2]	Text to right of cursor
*/

		if ( tc1 < cur_col1() )
		{
			// Text exists to left

			ci[0] = tc1;
			xi[0] = col_x_pix( tc1 );
			wi[0] = col_x_pix( cur_col1() ) - xi[0];
		}

		// Only render white text on the cursor section if the cell is
		// selected itself.

		if ( col >= cur_col1() && col <= cur_col2() )
		{
			// Text exists over the selection (by virtue of being
			// inside this 'else')

			if ( ci[0] >= 0 )
			{
/*
It's safe to use cur_col1 as array reference in this section because it has to
be >= c1 and also <= c2 due to text existing to the left of the cursor and the
fact that the cursor is between c1 and c2 somehwere due to the expansions
earlier.
*/
				// Start of render is flush to the cell edge on
				// the left.

				xi[1] = col_x_pix( cur_col1() );
				int const c = MIN( cur_col2(), tc2 );
				wi[1] = col_x_pix( c ) - xi[1] + col_w_pix( c );
			}
			else
			{
				// Start of render is pos_x as there is no text
				// to the left.

				xi[1] = col_x_pix( tc1 );
				int const c = MIN( cur_col2(), tc2 );
				wi[1] = col_x_pix( c ) - xi[1] + col_w_pix( c );
			}

			ci[1] = col;
		}

		if ( tc2 > cur_col2() )
		{
			// Text exists to right
			xi[2] = col_x_pix( cur_col2() ) + col_w_pix(cur_col2());
			wi[2] = col_x_pix( tc2 ) - xi[2] + col_w_pix( tc2 );
			ci[2] = cur_col2() + 1;
		}

		for ( int i = 0; i < 3; i++ )
		{
			if ( ci[i] < 0 )
			{
				continue;	// Skip this section
			}

			set_cell_x_px ( xi[i] );
			set_cell_width_px ( wi[i] );

			be_ren_text ( cell, row, ci[i] );
		}
	}
}

void SheetRenCore::get_cell_text_rgb (
	int		const	row,
	int		const	col,
	CedCell const * const	cell,
	unsigned char		color[3]
	) const
{
	if ( has_cursor ( row, col ) )
	{
		// Cursor covers this background, so use white text

		color[0] = CURSOR_R_TEXT;
		color[1] = CURSOR_G_TEXT;
		color[2] = CURSOR_B_TEXT;
	}
	else
	{
		if ( cell->prefs )
		{
			color[0] = (unsigned char)pixy_int_2_red (
				cell->prefs->color_foreground );

			color[1] = (unsigned char)pixy_int_2_green (
				cell->prefs->color_foreground );

			color[2] = (unsigned char)pixy_int_2_blue (
				cell->prefs->color_foreground );
		}
		else
		{
			color[0] = 0;
			color[1] = 0;
			color[2] = 0;
		}
	}
}

void SheetRenCore::justify_text_position ()
{
	switch ( cell_text_justify() )
	{
	case CED_CELL_JUSTIFY_RIGHT:
		set_text_x_px( text_x_px() + cell_width_px() - text_width_px() -
			viewren()->cell_pad() );
		break;

	case CED_CELL_JUSTIFY_CENTER:
		set_text_x_px ( text_x_px() + (cell_width_px() -
			text_width_px()) / 2 );
		break;

	case CED_CELL_JUSTIFY_LEFT:
	default:
		set_text_x_px ( text_x_px() + viewren()->cell_pad() );
		break;
	}
}

int SheetRenCore::render_cell_left (
	CedCell	const * const	cell,
	int		const	row,
	int		const	col
	)
{
	if ( ! cell->text )
	{
		return 0; // Nothing to render here, keep looking leftwards
	}

	// X coordinate of first visible cells left edge
	int const c1x = col_x_pix ( c1() );

	// X coordinate of this cell
	set_text_x_px ( c1x - viewren()->x_from_column ( col, c1() ) );
	set_cell_width_px ( viewren()->cell_width ( col ) );

	if ( be_ren_text_prepare ( cell ) )
	{
		return 1;		// Problem / Nothing to do
	}

	// Is the text going to be visible?
	if ( (text_x_px() + text_width_px()) > c1x )
	{
		set_cell_x_px ( c1x );
		set_cell_width_px ( col_w_pix( c1() ) );

		prepare_ren_expansion ( cell, row, col, c1() );
	}

	return 1;
}

int SheetRenCore::render_cell_text (
	CedCell	const * const	cell,
	int		const	row,
	int		const	col
	)
{
	if ( ! cell->text )
	{
		return 0;
	}

	// Check for text intruding from the right of the viewable area
	if ( col > c2() )
	{
		// We can stop scanning now if the last visible cell is active
		if ( cell_act_col( c2() ) != 0 )
		{
			return 1;
		}

		// X coordinate of last visible cells right edge + 1
		int const pos_x = col_x_pix( c2() ) + col_w_pix( c2() );
		set_text_x_px( pos_x + viewren()->x_from_column(c2() + 1, col));
		set_cell_x_px ( text_x_px() );
		set_cell_width_px ( viewren()->cell_width ( col ) );

		if ( be_ren_text_prepare ( cell ) )
		{
			return 0;
		}

		// Is the text going to be visible?
		if ( text_x_px() < pos_x )
		{
			// Clip to final visible cell and expand if necessary
			set_cell_x_px ( col_x_pix( c2() ) );
			set_cell_width_px ( col_w_pix( c2() ) );

			prepare_ren_expansion ( cell, row, col, c2() );
		}

		return 1;
	}

	set_cell_width_px ( col_w_pix( col ) );
	set_text_x_px ( col_x_pix( col ) );
	set_cell_x_px ( text_x_px() );

	if ( be_ren_text_prepare ( cell ) )
	{
		return 0;
	}

	if ( text_x_px() < ( cell_x_px() ) )
	{
		// Expand to the left (and right if needed)
		prepare_ren_expansion ( cell, row, col, col );
	}
	else if (	(text_x_px() + text_width_px()) >
			(cell_x_px() + cell_width_px())
			)
	{
		// Expand to the right
		prepare_ren_expansion ( cell, row, col, col );
	}
	else
	{
		// Render text as is
		be_ren_text ( cell, row, col );
	}

	return 0;			// Continue
}

int SheetRenCore::render_cell_background (
	CedCell	const * const	cell,
	int		const	row,
	int		const	col
	)
{
	if ( cell->text )
	{
		set_cell_act ( col );
	}

	if ( ! cell->prefs )
	{
		return 0;
	}

	if ( CED_COLOR_BACKGROUND_DEFAULT == cell->prefs->color_background )
	{
		return 0;		// Already default
	}

	if ( has_cursor ( row, col ) )
	{
		// Cursor covers this background, so do nothing

		return 0;
	}

	return be_ren_background ( cell, col );
}

int SheetRenCore::render_cell_foreground (
	CedCell	const * const	cell,
	int		const	row,
	int		const	col
	)
{
	if (	cell->prefs &&
		cell->prefs->border_type != 0 && // Border must exist
		col <= c2() &&			// Must be a visible cell
		row_height() > 3		// Row must be tall enough
		)
	{
		int const cursor = ( draw_cursor() && has_cursor(row, col) ) ?
			1 : 0;

		be_ren_borders ( cell, col, cursor );
	}

	return render_cell_text ( cell, row, col );
}

