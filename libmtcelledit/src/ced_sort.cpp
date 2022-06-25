/*
	Copyright (C) 2009-2021 Mark Tyler

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

#include "ced_sort.h"



int Sort_State::cmp_cells (
	CedCell	const * const	c1,
	CedCell	const * const	c2
	) const
{
	static int const ctp[CED_CELL_TYPE_TOTAL] = { 0, -10, -100, -100, -100,
		0, -100 };


	if ( ! c1 && ! c2 )
	{
		return 0;		// equal as neither cells exist
	}

	if ( ! c1 )
	{
		if ( c2->text )
		{
			return 1;	// empty a, active b => a>b
		}

		return 0;		// Empty cell same as non-existent cell
	}
	else if ( ! c2 )
	{
		if ( c1->text )
		{
			return -1;	// active a, empty b => a<b
		}

		return 0;		// Empty cell same as non-existent cell
	}

	if ( ! c1->text && ! c2->text )
	{
		return 0;		// equal as neither cell has text
	}

	if ( ! c1->text )
	{
		return 1;		// empty a => a>b
	}
	else if ( ! c2->text )
	{
		return -1;		// empty b => a<b
	}


	// This should never happen unless corruption has occurred
	if (	c1->type < CED_CELL_TYPE_NONE ||
		c1->type >= CED_CELL_TYPE_TOTAL ||
		c2->type < CED_CELL_TYPE_NONE ||
		c2->type >= CED_CELL_TYPE_TOTAL
		)
	{
		return 0;
	}

	// Check cell type precedence
	if ( ctp[ c1->type ] < ctp[ c2->type ] )
	{
		return ( -1 * m_order );
	}
	if ( ctp[ c1->type ] > ctp[ c2->type ] )
	{
		return ( 1 * m_order );
	}

	// At this point we know that the 2 cells are of the same type so
	// directly compare
	switch ( c1->type )
	{
	case CED_CELL_TYPE_TEXT:
	case CED_CELL_TYPE_TEXT_EXPLICIT:
		return ( m_order * cmp_cstring ( c1->text, c2->text ) );

	case CED_CELL_TYPE_VALUE:
	case CED_CELL_TYPE_FORMULA:
	case CED_CELL_TYPE_FORMULA_EVAL:
	case CED_CELL_TYPE_DATE:
		if ( c1->value < c2->value )
		{
			return ( -1 * m_order );
		}
		else if ( c1->value > c2->value )
		{
			return ( 1 * m_order );
		}
		break;

	case CED_CELL_TYPE_ERROR:	// All errors are equal
	default:
		return 0;		// Unknown type - should never happen
	}

	return 0;
}

void Sort_State::set_mode_from_list ( int const i )
{
	if ( m_mode_list )
	{
		if ( m_mode_list[ i ] & CED_SORT_MODE_DESCENDING )
		{
			m_order = -1;
		}
		else
		{
			m_order = 1;
		}

		if ( m_mode_list[ i ] & CED_SORT_MODE_CASE )
		{
			m_cmp_func = strcmp;
		}
		else
		{
			m_cmp_func = strcasecmp;
		}
	}
}

