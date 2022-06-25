/*
	Copyright (C) 2011-2021 Mark Tyler

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



class cwidSTATE
{
public:
	explicit cwidSTATE ( CuiRender const * const viewren )
		:
		m_font_width	( viewren->font_width() ),
		m_default_width	( viewren->default_cell_width() )
	{
		if ( m_default_width < 1 )
		{
			m_default_width = 1;
		}

		if ( m_font_width < 1 )
		{
			m_font_width = 1;
		}
	}

	~cwidSTATE ()
	{
		free ( m_ax );
//		m_ax = nullptr;
		free ( m_aw );
//		m_aw = nullptr;
	}

	inline int init_array ( size_t const size )
	{
		free ( m_ax );
		m_ax = (int *)calloc ( size, sizeof ( m_ax[0] ) );
		if ( ! m_ax )
		{
			return 1;
		}

		free ( m_aw );
		m_aw = (int *)calloc ( size, sizeof ( m_aw[0] ) );
		if ( ! m_aw )
		{
			return 1;
		}

		return 0;
	}

	inline void release_array ( int ** const x, int ** const w )
	{
		x[0] = m_ax;
		w[0] = m_aw;

		m_ax = nullptr;
		m_aw = nullptr;
	}

	void populate_array ( CuiRender const * viewren );

	inline int ax ( int i ) const { return m_ax[i]; }
	inline int aw ( int i ) const { return m_aw[i]; }
	inline void set_ax ( int i, int v ) const { m_ax[i] = v; }
	inline void set_aw ( int i, int v ) const { m_aw[i] = v; }

	void check_cell_width ( CedCell const * cell );

/// ----------------------------------------------------------------------------

	int	m_c1		= 0;
	int	m_c2		= 0;	// Min / Max columns

	int	m_col_prev	= 0;	// Last column calculated
	int	m_col_x		= 0;	// Current X pixel position

	int	m_ai		= 0;	// Current array index
	int	m_as		= 0;	// Current array size

	int	m_cellw		= 0;	// Cell widths

	int	m_font_width;		// Character width
	int	m_default_width;

private:
	int	* m_ax		= nullptr;
	int	* m_aw		= nullptr;	// Arrays
};


void cwidSTATE::check_cell_width ( CedCell const * const cell )
{
	if ( cell->prefs )
	{
		m_cellw = cell->prefs->width * m_font_width;

		if ( m_cellw == 0 )
		{
			m_cellw = m_default_width;
		}
		else if	( m_cellw < 0 )
		{
			m_cellw = 0;
		}
	}
	else
	{
		m_cellw = m_default_width;
	}
}

static int col_wid_array_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	col,
	void		* const	user_data
	)
{
	auto * const state = static_cast<cwidSTATE *>(user_data);

	state->check_cell_width ( cell );

	// Add in any skipped cells (fill with default widths)
	for (	;
		state->m_ai < (col - state->m_c1);
		state->m_ai ++
		)
	{
		state->set_ax ( state->m_ai, state->m_col_x );
		state->set_aw ( state->m_ai, state->m_default_width );

		state->m_col_x += state->m_default_width;
	}

	// Put width of this cell into the array
	state->set_ax ( state->m_ai, state->m_col_x );
	state->set_aw ( state->m_ai, state->m_cellw );
	state->m_ai ++;

	state->m_col_x += state->m_cellw;
	state->m_col_prev = col;

	return 0;		// Continue
}

void cwidSTATE::populate_array ( CuiRender const * const viewren )
{
	set_aw ( 0, viewren->cell_width ( m_c1 ) );

	m_ai = 1;
	m_col_prev = m_c1;
	m_col_x = ax( 0 ) + aw( 0 );

	if ( m_c1 < m_c2 )
	{
		ced_sheet_scan_area ( viewren->sheet(), 0, m_c1 + 1, 1,
			m_c2 - m_c1, col_wid_array_cb, this );

		// Calculate final gap to c2 - all defaults as we have no cells
		// left.

		for (	;
			m_ai < m_as;
			m_ai ++
			)
		{
			set_ax ( m_ai, m_col_x );
			set_aw ( m_ai, m_default_width );

			m_col_x += m_default_width;
		}
	}
}

int CuiRender::init_column_array (
	int		const	c1,
	size_t		const	coltot,
	int	** const	col_x,
	int	** const	col_w
	)
{
	cwidSTATE	state ( this );

	state.m_as = (int)coltot;
	state.m_c1 = c1;
	state.m_c2 = c1 + state.m_as - 1;

	if ( state.init_array ( coltot ) )
	{
		return 1;
	}

	state.set_ax ( 0, 0 );
	state.populate_array ( this );

	state.release_array ( col_x, col_w );

	return 0;
}

int CuiRender::init_column_array (
	int	const		col_start,
	int	const		x,
	int	const		w,
	int	&		c1,
	int	&		c2,
	int	** const	col_x,
	int	** const	col_w
	)
{
	cwidSTATE	state ( this );

	state.m_c1 = column_from_x ( col_start, x );
	state.m_c2 = column_from_x ( col_start, (x + w - 1) );
	state.m_as = state.m_c2 - state.m_c1 + 1;

	if ( state.init_array ( (size_t)state.m_as ) )
	{
		return 1;
	}

	state.set_ax ( 0, x_from_column( col_start, state.m_c1 ) );
	state.populate_array ( this );

	state.release_array ( col_x, col_w );

	c1 = state.m_c1;
	c2 = state.m_c2;

	return 0;
}

static int col_xfc_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	col,
	void		* const	user_data
	)
{
	auto * const state = static_cast<cwidSTATE *>(user_data);

	state->check_cell_width ( cell );

	// Add in any skipped cells (fill with default widths)
	state->m_col_x += state->m_default_width * (col - state->m_col_prev - 1)
			+ state->m_cellw;
	state->m_col_prev = col;

	return 0;			// Continue
}

int CuiRender::x_from_column (
	int	const	col_start,
	int	const	column
	) const
{
	cwidSTATE	state ( this );

	state.m_c1 = column;

	if ( column <= col_start )
	{
		return 0;
	}

	state.m_col_prev = col_start - 1;

	ced_sheet_scan_area ( sheet(), 0, col_start, 1,
		column - col_start, col_xfc_cb, &state );

	// Calculate final gap to column
	state.m_col_x += state.m_default_width * (column - state.m_col_prev -1);

	return state.m_col_x;
}

int CuiRender::y_from_row (
	int	const	row_start,
	int	const	row
	) const
{
	return (row - row_start) * row_height();
}

static int colfx_backwards_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	col,
	void		* const	user_data
	)
{
	auto * const state = static_cast<cwidSTATE *>(user_data);

	state->check_cell_width ( cell );

	// Add in any skipped cells (fill with default widths)
	for ( ; state->m_col_prev > col; state->m_col_prev -- )
	{
		state->m_col_x += state->m_default_width;

		if ( state->m_col_x >= state->m_c1 )
		{
			// Limit reached/exceeded
			state->m_col_prev --;

			return 1;
		}
	}

	state->m_col_x += state->m_cellw;
	state->m_col_prev --;

	if ( state->m_col_x >= state->m_c1 )
	{
		return 1;		// Limit reached/exceeded
	}

	return 0;			// Continue
}

int CuiRender::column_from_x_backwards (
	int	const	col_start,
	int	const	x
	) const
{
	cwidSTATE	state ( this );

	state.m_c1 = x;

	if ( x < 0 )
	{
		return col_start;
	}

	state.m_col_prev = col_start;

	ced_sheet_scan_area_backwards ( sheet(), 0, col_start, 1, 0,
		colfx_backwards_cb, &state );

	// Calculate final gap
	for ( ; state.m_col_x < x; state.m_col_prev -- )
	{
		state.m_col_x += state.m_default_width;
	}

	if ( state.m_col_x > x )
	{
		state.m_col_prev ++;
	}

	return state.m_col_prev;
}

static int colfx_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	col,
	void		* const	user_data
	)
{
	auto * const state = static_cast<cwidSTATE *>(user_data);

	state->check_cell_width ( cell );

	// Add in any skipped cells (fill with default widths)
	for ( ; state->m_col_prev < col; state->m_col_prev ++ )
	{
		state->m_col_x += state->m_default_width;

		if ( state->m_col_x >= state->m_c1 )
		{
			state->m_col_prev ++;

			return 1;	// Limit reached/exceeded
		}
	}

	state->m_col_x += state->m_cellw;
	state->m_col_prev ++;

	if ( state->m_col_x >= state->m_c1 )
	{
		return 1;		// Limit reached/exceeded
	}

	return 0;			// Continue
}

int CuiRender::column_from_x (
	int	const	col_start,
	int	const	x
	) const
{
	cwidSTATE	state ( this );

	state.m_c1 = x;

	if ( x < 0 )
	{
		return col_start;
	}

	state.m_col_prev = col_start;

	ced_sheet_scan_area ( sheet(), 0, col_start, 1, 0, colfx_cb, &state );

	// Calculate final gap
	for ( ; state.m_col_x < x; state.m_col_prev ++ )
	{
		state.m_col_x += state.m_default_width;
	}

	if ( state.m_col_x > x )
	{
		state.m_col_prev--;
	}

	return state.m_col_prev;
}

int CuiRender::row_from_y (
	int	const	row_start,
	int	const	y
	) const
{
	if ( y < 0 )
	{
		return row_start;
	}

	return ( row_start + y / row_height() );
}

