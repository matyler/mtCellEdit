/*
	Copyright (C) 2008-2016 Mark Tyler

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

#include "private.h"



enum
{
	RRPD_ROUTE_average,
	RRPD_ROUTE_count,
	RRPD_ROUTE_counta,
	RRPD_ROUTE_countblank,
	RRPD_ROUTE_countif,
	RRPD_ROUTE_large,
	RRPD_ROUTE_max,
	RRPD_ROUTE_median,
	RRPD_ROUTE_min,
	RRPD_ROUTE_percentile,
	RRPD_ROUTE_percentrank,
	RRPD_ROUTE_rank,
	RRPD_ROUTE_small,
	RRPD_ROUTE_sum,
	RRPD_ROUTE_sumif,

	RRPD_ROUTE_tot
};

enum
{
	COND_LT,	// <
	COND_LE,	// <=
	COND_GT,	// >
	COND_GE,	// >=
	COND_EQ,	// =
	COND_NE		// <>
};

/*
	This code goes through the tree checking for rows/cols in the range
	which is much better than laboriously checking each cell (especially
	for large sheets with large gaps).
*/

typedef struct
{
	CedSheet	* sheet;
	int		func;
	CedCellRef	* cr1,
			* cr2;
	CedFuncState	* fn_state;
	double		* d_input;		// Input array, or NULL

	int		r1,
			c1,
			r2,
			c2,
			f,
			pass;
	double		d,
			* d_list;

	int		r_ref,
			c_ref,
			cond_type;		// COND_* condition type

	double		cond_val;

	CedSheet	* sheet_b;
} router_state;



static int validate_cellref (
	CedCellRef	* const	ref,
	CedParser	* const	state,
	int		* const	r,
	int		* const	c
	)
{
	r[0] = state->row    * ref->row_m + ref->row_d;
	c[0] = state->column * ref->col_m + ref->col_d;

	if ( r[0] < 1 || c[0] < 1 )
	{
		return 1;
	}

	return 0;
}

static int validate_cellrange (
	CedCellRef	* const	ref1,
	CedCellRef	* const	ref2,
	CedParser	* const	state,
	int		* const	r1,
	int		* const	c1,
	int		* const	r2,
	int		* const	c2
	)
{
	int		ir,
			ic;


	if ( validate_cellref ( ref1, state, r1, c1 ) )
	{
		return 1;
	}

	if ( validate_cellref ( ref2, state, r2, c2 ) )
	{
		return 1;
	}

	// Ensure that r1 <= r2 & c1 <= c2
	if ( r2[0] < r1[0] )
	{
		ir	= r1[0];
		r1[0]	= r2[0];
		r2[0]	= ir;
	}

	if ( c2[0] < c1[0] )
	{
		ic	= c1[0];
		c1[0]	= c2[0];
		c2[0]	= ic;
	}

	return 0;
}

static void router_count_sum (
	router_state	* const	rstate,
	CedCell		* const	cell,
	int		const	row,
	int		const	col
	)
{
	int		cond = 0;


	switch ( rstate->cond_type )
	{
	case COND_LT:
		cond = (cell->value < rstate->cond_val);
		break;

	case COND_LE:
		cond = (cell->value <= rstate->cond_val);
		break;

	case COND_GT:
		cond = (cell->value > rstate->cond_val);
		break;

	case COND_GE:
		cond = (cell->value >= rstate->cond_val);
		break;

	case COND_EQ:
		cond = (cell->value == rstate->cond_val);
		break;

	case COND_NE:
		cond = (cell->value != rstate->cond_val);
		break;
	}

	if ( cond )
	{
		switch ( rstate->func )
		{
		case RRPD_ROUTE_countif:
			rstate->d ++;
			break;

		case RRPD_ROUTE_sumif:
			{
			CedCell		* rcell;


			rcell = ced_sheet_get_cell ( rstate->sheet_b,
				row - rstate->r1 + rstate->r_ref,
				col - rstate->c1 + rstate->c_ref
				);

			if (	rcell &&
				rcell->type != CED_CELL_TYPE_TEXT &&
				rcell->type != CED_CELL_TYPE_TEXT_EXPLICIT &&
				rcell->type != CED_CELL_TYPE_ERROR
				)
				{
					rstate->d += rcell->value;
				}

			break;
			}
		}
	}
}

static int router_scan_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	router_state	* const	rstate = (router_state *)user_data;


	if ( cell->type != CED_CELL_TYPE_NONE )
	{
		if ( rstate->func == RRPD_ROUTE_countblank )
		{
			rstate->d --;
		}
		else if ( rstate->func == RRPD_ROUTE_counta )
		{
			rstate->d ++;
		}
		else if (	cell->type != CED_CELL_TYPE_TEXT &&
				cell->type != CED_CELL_TYPE_TEXT_EXPLICIT &&
				cell->type != CED_CELL_TYPE_ERROR
				)
		{
			switch ( rstate->func )
			{
			case RRPD_ROUTE_average:
				rstate->d += cell->value;
				rstate->f ++;
				break;

			case RRPD_ROUTE_count:
				rstate->d ++;
				break;

			case RRPD_ROUTE_countif:
			case RRPD_ROUTE_sumif:
				router_count_sum ( rstate, cell, row, col );
				break;

			case RRPD_ROUTE_max:
				if ( ! rstate->f )
				{
					rstate->f = 1;
					rstate->d = cell->value;
				}
				else
				{
					if ( cell->value > rstate->d )
					{
						rstate->d = cell->value;
					}
				}
				break;

			case RRPD_ROUTE_median:
			case RRPD_ROUTE_percentrank:
			case RRPD_ROUTE_percentile:
			case RRPD_ROUTE_rank:
			case RRPD_ROUTE_small:
			case RRPD_ROUTE_large:
				if ( rstate->pass == 1 )
				{
					rstate->d_list[rstate->f] = cell->value;
				}

				rstate->f += 1;
				break;

			case RRPD_ROUTE_min:
				if ( ! rstate->f )
				{
					rstate->f = 1;
					rstate->d = cell->value;
				}
				else
				{
					if ( cell->value < rstate->d )
					{
						rstate->d = cell->value;
					}
				}
				break;

			case RRPD_ROUTE_sum:
				rstate->d += cell->value;
				break;
			}
		}
	}

	return 0;
}

static int qsort_median_cmp (
	void	const * const	a,
	void	const * const	b
	)
{
	double	const	* da = (double const *)a;
	double	const	* db = (double const *)b;


	if ( da[0] < db[0] )
	{
		return -1;
	}

	if ( da[0] > db[0] )
	{
		return 1;
	}

	return 0;
}

static double state_percentile (
	router_state	* const	rstate,
	double		const	p
	)
{
	double		pos,		// Position in array
			fr;		// Ffractional position

/*
e.g. for rstate->f = 10

	p	pos
	0.0	0
	0.5	4.5
	1.0	9

	When pos != 0, use fractions of 2 closest points
*/

	pos = p * (rstate->f - 1);
	fr = pos - (int)pos;

	if ( fr == 0 )
	{
		return rstate->d_list[ (int)pos ];
	}

	return	(1 - fr)	* rstate->d_list[ (int)pos     ] +
		fr		* rstate->d_list[ (int)pos + 1 ];
}

// Get the first occurence of val (r1 = r2) OR the 2 points either side of it
// (r1 < r2)
static int get_rank (
	router_state	* const	rstate,
	double		const	val,
	int		* const	r1,
	int		* const	r2
	)
{
	int		lo,
			hi,
			mid;


	if (	val < rstate->d_list[0] ||
		val > rstate->d_list[rstate->f - 1]
		)
	{
		return 1;		// Outside range
	}

	// Simple binary search as the list has been sorted
	lo = 0;
	hi = rstate->f - 1;
	do
	{
		mid = (lo + hi) / 2;

		if ( rstate->d_list[ mid ] == val )
		{
			// Get the first match
			while ( mid > 0 && rstate->d_list[mid - 1] == val )
			{
				mid --;
			}

			r1[0] = mid;
			r2[0] = mid;

			return 0;
		}
		else if ( rstate->d_list[ mid ] > val )
		{
			hi = mid - 1;
			if ( hi < lo )
			{
				r1[0] = mid - 1;
				r2[0] = mid;

				return 0;
			}
		}
		else	// rstate->d_list[ mid ] < val
		{
			lo = mid + 1;
			if ( hi < lo )
			{
				r1[0] = mid;
				r2[0] = mid + 1;

				return 0;
			}
		}

	} while ( 1 );

	return 0;	// Should never get here
}

static int rrpd_router (
	router_state	* const	rost
	)
{
	int		r1,
			r2;


	if ( ! rost->sheet )
	{
		return 1;
	}

	if ( validate_cellrange ( rost->cr1, rost->cr2, rost->fn_state->parser,
		&rost->r1, &rost->c1, &rost->r2, &rost->c2 )
		)
	{
		return 1;
	}

	switch ( rost->func )
	{
	case RRPD_ROUTE_countblank:
		// Assume all blank for now, and when each cell is encountered
		// subtract one
		rost->d = (rost->c2 - rost->c1 + 1) * (rost->r2 - rost->r1 + 1);
		break;

	case RRPD_ROUTE_sumif:
	case RRPD_ROUTE_countif:
		if ( ! strncmp ( "<\"", rost->fn_state->arg[1].u.str, 2 ) )
		{
			rost->cond_type = COND_LT;
		}
		else if ( ! strncmp ( "<=\"", rost->fn_state->arg[1].u.str, 3 ))
		{
			rost->cond_type = COND_LE;
		}
		else if ( ! strncmp ( ">\"", rost->fn_state->arg[1].u.str, 2 ) )
		{
			rost->cond_type = COND_GT;
		}
		else if ( ! strncmp ( ">=\"", rost->fn_state->arg[1].u.str, 3 ))
		{
			rost->cond_type = COND_GE;
		}
		else if ( ! strncmp ( "=\"", rost->fn_state->arg[1].u.str, 2 ) )
		{
			rost->cond_type = COND_EQ;
		}
		else if ( ! strncmp ( "<>\"", rost->fn_state->arg[1].u.str, 3 ))
		{
			rost->cond_type = COND_NE;
		}
		else
		{
			goto error;
		}

		rost->cond_val = rost->fn_state->arg[2].u.val;

		break;
	}

	if ( rost->sheet->rows && rost->sheet->rows->root )
	{
		if ( ced_sheet_scan_area ( rost->sheet, rost->r1, rost->c1,
			rost->r2 - rost->r1 + 1, rost->c2 - rost->c1 + 1,
			router_scan_cb, rost )
			)
		{
			goto error;
		}
	}

	switch ( rost->func )
	{
	case RRPD_ROUTE_average:
		rost->fn_state->result[0] = rost->d / rost->f;
		break;

	// These functions all require a second pass of recursion
	case RRPD_ROUTE_median:
	case RRPD_ROUTE_percentrank:
	case RRPD_ROUTE_percentile:
	case RRPD_ROUTE_rank:
	case RRPD_ROUTE_small:
	case RRPD_ROUTE_large:
		if ( rost->f < 1 )
		{
			goto error;
		}

		rost->d_list = (double *)calloc ( sizeof ( double ),
			(size_t)rost->f );
		if ( ! rost->d_list )
		{
			goto error;
		}

		rost->pass += 1;		// Next pass adds numbers
		rost->f = 0;
		if ( ced_sheet_scan_area ( rost->sheet, rost->r1, rost->c1,
			rost->r2 - rost->r1 + 1, rost->c2 - rost->c1 + 1,
			router_scan_cb, rost )
			)
		{
			goto error;
		}

		qsort ( rost->d_list, (size_t)rost->f, sizeof ( double ),
			qsort_median_cmp );

		switch ( rost->func )
		{
		case RRPD_ROUTE_median:
			rost->fn_state->result[0] =
				state_percentile ( rost, 0.5 );
			break;

		case RRPD_ROUTE_large:
			if ( rost->d_input[0] > rost->f )
			{
				// < 1 is done by caller
				goto error;
			}

			rost->fn_state->result[0] =
				rost->d_list[ rost->f - (int)rost->d_input[0] ];
			break;

		case RRPD_ROUTE_small:
			if ( rost->d_input[0] > rost->f )
			{
				// < 1 is done by caller
				goto error;
			}

			rost->fn_state->result[0] =
				rost->d_list[ (int)rost->d_input[0] - 1 ];
			break;

		case RRPD_ROUTE_rank:
			if ( get_rank ( rost, rost->d_input[0], &r1, &r2 ) )
			{
				goto error;
			}

			if ( r1 != r2 )
			{
				goto error;
			}

			if ( rost->d_input[1] != 0 )
			{
				rost->fn_state->result[0] = r1 + 1;
			}
			else
			{
				rost->fn_state->result[0] = rost->f - r1;
			}

			break;

		case RRPD_ROUTE_percentrank:
			if ( get_rank ( rost, rost->d_input[0], &r1, &r2 ) )
			{
				goto error;
			}

			if ( r1 == r2 )
			{
				rost->fn_state->result[0] =
					( (double)r1 ) / ( (double)rost->f - 1);
			}
			else
			{
				double		gap;


				gap = ( rost->d_input[0] - rost->d_list[ r1 ] )
					/ ( rost->d_list[ r2 ] -
						rost->d_list[ r1 ] );

				rost->fn_state->result[0] =
					( (double)r1 + gap ) /
					( (double)rost->f - 1 );
			}
			break;

		case RRPD_ROUTE_percentile:
			rost->fn_state->result[0] =
				state_percentile ( rost, rost->d_input[0] );
			break;
		}

		break;

	default:
		rost->fn_state->result[0] = rost->d;
		break;
	}

	free ( rost->d_list );

	return 0;

error:
	free ( rost->d_list );

	return 1;
}


/*
	------  My Functions  ------
*/



#define ARGTYPE( N )	( funcs->arg[N].type )
#define ARGNUM( N )	( funcs->arg[N].u.val )
#define ARGSTR( N )	( funcs->arg[N].u.str )
#define ARGREF( N, R )	( &funcs->arg[N].u.cellref[R] )
#define ARGSHEET( N )	( funcs->arg[N].sheet )
#define FUNC_RESULT	( funcs->result[0] )



static int average (
	CedFuncState	* const	funcs
	)
{
	router_state	state = { ARGSHEET ( 0 ), RRPD_ROUTE_average,
				ARGREF ( 0, 0 ), ARGREF ( 0, 1 ), funcs,
				NULL, 0, 0, 0, 0, 0, 0, 0.0, NULL, 0, 0, 0,
				0.0, NULL
				};


	return rrpd_router ( &state );
}

static int count (
	CedFuncState	* const	funcs
	)
{
	router_state	state = { ARGSHEET ( 0 ), RRPD_ROUTE_count,
				ARGREF ( 0, 0 ), ARGREF ( 0, 1 ), funcs,
				NULL, 0, 0, 0, 0, 0, 0, 0.0, NULL, 0, 0, 0,
				0.0, NULL
				};


	return rrpd_router ( &state );
}

static int counta (
	CedFuncState	* const	funcs
	)
{
	router_state	state = { ARGSHEET ( 0 ), RRPD_ROUTE_counta,
				ARGREF ( 0, 0 ), ARGREF ( 0, 1 ), funcs,
				NULL, 0, 0, 0, 0, 0, 0, 0.0, NULL, 0, 0, 0,
				0.0, NULL
				};


	return rrpd_router ( &state );
}

static int countblank (
	CedFuncState	* const	funcs
	)
{
	router_state	state = { ARGSHEET ( 0 ), RRPD_ROUTE_countblank,
				ARGREF ( 0, 0 ), ARGREF ( 0, 1 ), funcs,
				NULL, 0, 0, 0, 0, 0, 0, 0.0, NULL, 0, 0, 0,
				0.0, NULL
				};


	return rrpd_router ( &state );
}

static int countif (
	CedFuncState	* const	funcs
	)
{
	router_state	state = { ARGSHEET ( 0 ), RRPD_ROUTE_countif,
				ARGREF ( 0, 0 ), ARGREF ( 0, 1 ), funcs,
				NULL, 0, 0, 0, 0, 0, 0, 0.0, NULL, 0, 0, 0,
				0.0, NULL
				};


	return rrpd_router ( &state );
}

static int degrees (
	CedFuncState	* const	funcs
	)
{
	FUNC_RESULT = 180 * ARGNUM ( 0 ) / M_PI;

	return 0;
}

static int frac (
	CedFuncState	* const	funcs
	)
{
	FUNC_RESULT = modf ( ARGNUM ( 0 ), funcs->result );

	return 0;
}

static int if_func (
	CedFuncState	* const	funcs
	)
{
	if ( ARGNUM ( 0 ) )
	{
		FUNC_RESULT = ARGNUM ( 1 );
	}
	else
	{
		FUNC_RESULT = ARGNUM ( 2 );
	}

	return 0;
}

static int rgb_func (
	CedFuncState	* const	funcs
	)
{
	uint32_t	r,
			g,
			b;


	r = (uint32_t)ARGNUM ( 0 );
	g = (uint32_t)ARGNUM ( 1 );
	b = (uint32_t)ARGNUM ( 2 );

	r &= 255;
	g &= 255;
	b &= 255;

	FUNC_RESULT = (r << 16) + (g << 8) + b;

	return 0;
}

static int int_func (
	CedFuncState	* const	funcs
	)
{
	modf ( ARGNUM ( 0 ), funcs->result );

	return 0;
}

static int large (
	CedFuncState	* const	funcs
	)
{
	router_state	state = { ARGSHEET ( 0 ), RRPD_ROUTE_large,
				ARGREF ( 0, 0 ), ARGREF ( 0, 1 ), funcs,
				&ARGNUM ( 1 ), 0, 0, 0, 0, 0, 0, 0.0, NULL, 0,
				0, 0, 0.0, NULL
				};


	if ( ARGNUM ( 1 ) < 1 )
	{
		return 1;
	}

	return rrpd_router ( &state );
}

static int small (
	CedFuncState	* const	funcs
	)
{
	router_state	state = { ARGSHEET ( 0 ), RRPD_ROUTE_small,
				ARGREF ( 0, 0 ), ARGREF ( 0, 1 ), funcs,
				&ARGNUM ( 1 ), 0, 0, 0, 0, 0, 0, 0.0, NULL, 0,
				0, 0, 0.0, NULL
				};


	if ( ARGNUM ( 1 ) < 1 )
	{
		return 1;
	}

	return rrpd_router ( &state );
}

static int percentile (
	CedFuncState	* const	funcs
	)
{
	router_state	state = { ARGSHEET ( 0 ), RRPD_ROUTE_percentile,
				ARGREF ( 0, 0 ), ARGREF ( 0, 1 ), funcs,
				&ARGNUM ( 1 ), 0, 0, 0, 0, 0, 0, 0.0, NULL, 0,
				0, 0, 0.0, NULL
				};


	if (	ARGNUM ( 1 ) < 0.0 ||
		ARGNUM ( 1 ) > 1.0
		)
	{
		return 1;
	}

	return rrpd_router ( &state );
}

static int percentrank (
	CedFuncState	* const	funcs
	)
{
	router_state	state = { ARGSHEET ( 0 ), RRPD_ROUTE_percentrank,
				ARGREF ( 0, 0 ), ARGREF ( 0, 1 ), funcs,
				&ARGNUM ( 1 ), 0, 0, 0, 0, 0, 0, 0.0, NULL, 0,
				0, 0, 0.0, NULL
				};


	return rrpd_router ( &state );
}

static int rank (
	CedFuncState	* const	funcs
	)
{
	double		d_input[2] = { ARGNUM ( 0 ), ARGNUM ( 2 ) };
	router_state	state = { ARGSHEET ( 1 ), RRPD_ROUTE_rank,
				ARGREF ( 1, 0 ), ARGREF ( 1, 1 ), funcs,
				d_input, 0, 0, 0, 0, 0, 0, 0.0, NULL, 0, 0, 0,
				0.0, NULL
				};


	if (	ARGNUM ( 2 ) != 0 &&
		ARGNUM ( 2 ) != 1
		)
	{
		return 1;
	}

	return rrpd_router ( &state );
}

static int median (
	CedFuncState	* const	funcs
	)
{
	router_state	state = { ARGSHEET ( 0 ), RRPD_ROUTE_median,
				ARGREF ( 0, 0 ), ARGREF ( 0, 1 ), funcs,
				NULL, 0, 0, 0, 0, 0, 0, 0.0, NULL, 0, 0, 0,
				0.0, NULL
				};


	return rrpd_router ( &state );
}

static int max (
	CedFuncState	* const	funcs
	)
{
	double		curmax = 0;
	int		argi,
			res;
	router_state	state = { NULL, RRPD_ROUTE_max, NULL, NULL, funcs,
				NULL, 0, 0, 0, 0, 0, 0, 0.0, NULL, 0, 0, 0,
				0.0, NULL
				};


	for ( argi = 0; ARGTYPE ( argi ) != 0; argi ++ )
	{
		if ( ARGTYPE ( argi ) == CED_FARG_TYPE_NUM )
		{
			FUNC_RESULT = ARGNUM ( argi );
		}
		else if ( ARGTYPE ( argi ) == CED_FARG_TYPE_CELLRANGE )
		{
			state.sheet = ARGSHEET ( argi );
			state.cr1 = ARGREF ( argi, 0 );
			state.cr2 = ARGREF ( argi, 1 );

			res = rrpd_router ( &state );
			if ( res )
			{
				return res;	// Fail
			}
		}
		else
		{
			// Wrong argument type
			funcs->parser->ced_errno =
				CED_ERROR_BAD_FUNCTION_ARGUMENTS;

			return 1;		// Fail
		}

		if ( argi == 0 )
		{
			curmax = FUNC_RESULT;

			continue;
		}

		if ( FUNC_RESULT > curmax )
		{
			curmax = FUNC_RESULT;
		}
	}

	FUNC_RESULT = curmax;

	return 0;				// Success
}

static int min (
	CedFuncState	* const	funcs
	)
{
	double		curmin = 0;
	int		argi,
			res;
	router_state	state = { NULL, RRPD_ROUTE_min, NULL, NULL, funcs,
				NULL, 0, 0, 0, 0, 0, 0, 0.0, NULL, 0, 0, 0,
				0.0, NULL
				};


	for ( argi = 0; ARGTYPE ( argi ) != 0; argi ++ )
	{
		if ( ARGTYPE ( argi ) == CED_FARG_TYPE_NUM )
		{
			FUNC_RESULT = ARGNUM ( argi );
		}
		else if ( ARGTYPE ( argi ) == CED_FARG_TYPE_CELLRANGE )
		{
			state.sheet = ARGSHEET ( argi );
			state.cr1 = ARGREF ( argi, 0 );
			state.cr2 = ARGREF ( argi, 1 );

			res = rrpd_router ( &state );
			if ( res )
			{
				return res;	// Fail
			}
		}
		else
		{
			// Wrong argument type
			funcs->parser->ced_errno =
				CED_ERROR_BAD_FUNCTION_ARGUMENTS;

			return 1;		// Fail
		}

		if ( argi == 0 )
		{
			curmin = FUNC_RESULT;

			continue;
		}

		if ( FUNC_RESULT < curmin )
		{
			curmin = FUNC_RESULT;
		}
	}

	FUNC_RESULT = curmin;

	return 0;				// Success
}

static int vlookup_scan_cb (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	ARG_UNUSED ( cell ),
	int		const	row,
	int		const	col,
	void		* const	user_data
	)
{
	CedCellRef	* const	cellref = (CedCellRef *)user_data;


	cellref->row_d = row;
	cellref->col_d = col;

	return 1;
}

static int vlookup (
	CedFuncState	* const	funcs
	)
{
	CedCell		* cell;
	CedCellRef	cellref = { 0, 0, 0, 0 };
	int		r1, c1, r2, c2;


	if ( validate_cellrange ( ARGREF ( 1, 0 ), ARGREF ( 1, 1 ),
		funcs->parser, &r1, &c1, &r2, &c2 )
		)
	{
		return 1;
	}

	if ( ced_sheet_find_value ( ARGSHEET ( 1 ), ARGNUM ( 0 ),
		CED_FIND_MODE_IG_ERROR | CED_FIND_MODE_IG_TEXT,
		r1, c1, r2 - r1 + 1, c2 - c1 + 1, vlookup_scan_cb, &cellref )
		!= 2
		)
	{
		return 1;		// Error or not found
	}

	cellref.col_d += (int)ARGNUM ( 2 );
	if ( cellref.col_d < 1 )
	{
		return 1;		// Offset off the sheet
	}

	cell = ced_sheet_get_cell ( ARGSHEET ( 1 ), cellref.row_d,
		cellref.col_d );

	if ( cell )
	{
		FUNC_RESULT = cell->value;
	}
	else
	{
		FUNC_RESULT = 0;
	}

	return 0;
}

static int strvlookup (
	CedFuncState	* const	funcs
	)
{
	CedCell		* cell;
	CedCellRef	cellref = { 0, 0, 0, 0 };
	int		r0, c0, r1, c1, r2, c2;


	if (	validate_cellref ( ARGREF ( 0, 0 ), funcs->parser, &r0, &c0 ) ||
		validate_cellrange ( ARGREF ( 1, 0 ), ARGREF ( 1, 1 ),
			funcs->parser, &r1, &c1, &r2, &c2 )
		)
	{
		return 1;
	}

/*
printf ( "strvlookup 2 - sheet = %i sheet_b = %i r = %i c = %i\n",
	(int)funcs->sheet, (int)funcs->sheet_b, r0, c0 );
*/

	// Find the text string we are looking to match
	cell = ced_sheet_get_cell ( ARGSHEET ( 0 ), r0, c0 );
	if ( ! cell || ! cell->text )
	{
		return 1;
	}

	if ( ced_sheet_find_text ( ARGSHEET ( 1 ), cell->text,
		CED_FIND_MODE_CASE | CED_FIND_MODE_ALLCHARS,
		r1, c1, r2 - r1 + 1, c2 - c1 + 1, vlookup_scan_cb, &cellref )
		!= 2
		)
	{
		return 1;		// Error or not found
	}

	cellref.col_d += (int)ARGNUM ( 2 );
	if ( cellref.col_d < 1 )
	{
		return 1;		// Offset off the sheet
	}

	cell = ced_sheet_get_cell ( ARGSHEET ( 1 ), cellref.row_d,
		cellref.col_d );

	if ( cell )
	{
		FUNC_RESULT = cell->value;
	}
	else
	{
		FUNC_RESULT = 0;
	}

	return 0;
}

static int offset (
	CedFuncState	* const	funcs
	)
{
	int		r,
			c;
	CedCell		* cell = NULL;
	CedCellRef	* cr = ARGREF ( 0, 0 );


	r = cr->row_m * funcs->parser->row    + cr->row_d + (int)ARGNUM ( 1 );
	c = cr->col_m * funcs->parser->column + cr->col_d + (int)ARGNUM ( 2 );

	if ( ! ARGSHEET ( 0 ) )
	{
		return 1;		// No sheet available
	}

	if ( r < 1 || c < 1 )
	{
		return 1;		// Bad cell reference
	}

	cell = ced_sheet_get_cell ( ARGSHEET ( 0 ), r, c );

	if ( ! cell )			// No such cell so return zilch
	{
		FUNC_RESULT = 0;

		return 0;
	}

	if ( cell->type == CED_CELL_TYPE_ERROR )
	{
		return 1;
	}

	FUNC_RESULT = cell->value;

	return 0;
}

static int pi_func (
	CedFuncState	* const	funcs
	)
{
	FUNC_RESULT = M_PI;

	return 0;
}

static int radians (
	CedFuncState	* const	funcs
	)
{
	FUNC_RESULT = M_PI * ARGNUM ( 0 ) / 180;

	return 0;
}

static int rnd (
	CedFuncState	* const	funcs
	)
{
	FUNC_RESULT = ( rand () ) / ( (double)RAND_MAX );

	return 0;
}

static int sum_cellrange (
	CedFuncState	* const	funcs
	)
{
	router_state	state = { ARGSHEET ( 0 ), RRPD_ROUTE_sum,
				ARGREF ( 0, 0 ), ARGREF ( 0, 1 ), funcs,
				NULL, 0, 0, 0, 0, 0, 0, 0.0, NULL, 0, 0, 0,
				0.0, NULL
				};


	return rrpd_router ( &state );
}

static int sumif (
	CedFuncState	* const	funcs
	)
{
	router_state	state = { ARGSHEET ( 0 ), RRPD_ROUTE_sumif,
				ARGREF ( 0, 0 ), ARGREF ( 0, 1 ), funcs,
				NULL, 0, 0, 0, 0, 0, 0, 0.0, NULL, 0, 0, 0,
				0.0, NULL
				};


	state.sheet_b = ARGSHEET ( 3 );
	state.r_ref = funcs->arg[3].u.cellref[0].row_d;
	state.c_ref = funcs->arg[3].u.cellref[0].col_d;

	return rrpd_router ( &state );
}

static int get_time_now (
	int		const	full,
	CedFuncState	* const	funcs
	)
{
	time_t		now;
	struct tm const	* now_tm;


	now = time ( NULL );
	now_tm = localtime ( &now );

	if ( full )
	{
		mtkit_itoddt ( now_tm->tm_mday, now_tm->tm_mon + 1,
			now_tm->tm_year + 1900, now_tm->tm_hour,
			now_tm->tm_min, now_tm->tm_sec, funcs->result );
	}
	else
	{
		mtkit_itoddt ( now_tm->tm_mday, now_tm->tm_mon + 1,
			now_tm->tm_year + 1900,
			0, 0, 0, funcs->result );
	}

	return 0;
}

static int now_func (
	CedFuncState	* const	funcs
	)
{
	return get_time_now ( 1, funcs );
}

static int today_func (
	CedFuncState	* const	funcs
	)
{
	return get_time_now ( 0, funcs );
}

static int date_func (
	CedFuncState	* const	funcs
	)
{
	mtkit_itoddt ( (int)ARGNUM ( 2 ), (int)ARGNUM ( 1 ), (int)ARGNUM ( 0 ),
		0, 0, 0, funcs->result );

	return 0;
}

static int time_func (
	CedFuncState	* const	funcs
	)
{
	mtkit_itoddt ( 1, 1, 0, (int)ARGNUM ( 0 ), (int)ARGNUM ( 1 ),
		(int)ARGNUM ( 2 ), funcs->result );

	return 0;
}

static int weekday_func (
	CedFuncState	* const	funcs
	)
{
	FUNC_RESULT = 1 + mtkit_ddt_weekday ( ARGNUM ( 0 ) );

	return 0;
}

static int year_func (
	CedFuncState	* const	funcs
	)
{
	int		result = 0;


	mtkit_ddttoi ( ARGNUM ( 0 ), NULL, NULL, &result, NULL, NULL, NULL );
	FUNC_RESULT = result;

	return 0;
}

static int month_func (
	CedFuncState	* const	funcs
	)
{
	int		result = 0;


	mtkit_ddttoi ( ARGNUM ( 0 ), NULL, &result, NULL, NULL, NULL, NULL );

	FUNC_RESULT = result;

	return 0;
}

static int day_func (
	CedFuncState	* const	funcs
	)
{
	int		result = 0;


	mtkit_ddttoi ( ARGNUM ( 0 ), &result, NULL, NULL, NULL, NULL, NULL );

	FUNC_RESULT = result;

	return 0;
}

static int hour_func (
	CedFuncState	* const	funcs
	)
{
	int		result = 0;


	mtkit_ddttoi ( ARGNUM ( 0 ), NULL, NULL, NULL, &result, NULL, NULL );

	FUNC_RESULT = result;

	return 0;
}

static int minute_func (
	CedFuncState	* const	funcs
	)
{
	int		result = 0;


	mtkit_ddttoi ( ARGNUM ( 0 ), NULL, NULL, NULL, NULL, &result, NULL );

	FUNC_RESULT = result;

	return 0;
}

static int second_func (
	CedFuncState	* const	funcs
	)
{
	int		result = 0;


	mtkit_ddttoi ( ARGNUM ( 0 ), NULL, NULL, NULL, NULL, NULL, &result );

	FUNC_RESULT = result;

	return 0;
}

static int fmod_func (
	CedFuncState	* const	funcs
	)
{
	FUNC_RESULT = fmod ( ARGNUM ( 0 ), ARGNUM ( 1 ) );

	return 0;
}

static int round_trunc_real (
	CedFuncState	* const	funcs,
	int		const	type		// 0 = trunc, 1 = round
	)
{
	double		subval,
			mult,
			rh = 0;
	int		dp;


	if (	isnan ( ARGNUM ( 0 ) ) ||
		isinf ( ARGNUM ( 0 ) )
		)
	{
		FUNC_RESULT = ARGNUM ( 0 );

		return 0;
	}

	if (	isnan ( ARGNUM ( 1 ) ) ||
		isinf ( ARGNUM ( 1 ) )
		)
	{
		FUNC_RESULT = ARGNUM ( 1 );

		return 0;
	}

	dp = (int)ARGNUM ( 1 );
	mult = pow ( 10, dp );

	if (	isnan ( mult ) ||
		isinf ( mult )
		)
	{
		return 1;
	}

	if ( dp < 0 && fabs ( 1/mult ) > fabs ( ARGNUM ( 0 ) ) )
	{
		// Don't bother doing calculations for zero result
		FUNC_RESULT = 0;

		return 0;
	}

	subval = modf ( ARGNUM ( 0 ) * mult, &subval );

	if ( type == 1 )	// Rounding tweak
	{
		if ( subval < -0.5 )
		{
			rh = -1 / mult;
		}
		else if ( subval >  0.5 )
		{
			rh =  1 / mult;
		}
	}

	subval = subval / mult;

	if ( isnan ( subval ) || isinf ( subval ) )
	{
		return 1;
	}

	if ( fabs ( subval ) > fabs ( ARGNUM ( 0 ) ) )
	{
		subval = ARGNUM ( 0 );
	}

	FUNC_RESULT = ARGNUM ( 0 ) - subval + rh;

	return 0;
}

static int round_func (
	CedFuncState	* const	funcs
	)
{
	return round_trunc_real ( funcs, 1 );
}


static int trunc_func (
	CedFuncState	* const	funcs
	)
{
	return round_trunc_real ( funcs, 0 );
}

#define MATH_FUNC( NAME ) \
	static int NAME##_func ( CedFuncState * const	funcs ) \
	{ \
		FUNC_RESULT = NAME ( ARGNUM ( 0 ) ); \
		return 0; \
	}

MATH_FUNC ( fabs )
MATH_FUNC ( acos )
MATH_FUNC ( asin )
MATH_FUNC ( atan )
MATH_FUNC ( ceil )
MATH_FUNC ( cos )
MATH_FUNC ( cosh )
MATH_FUNC ( exp )
MATH_FUNC ( floor )
MATH_FUNC ( log )
MATH_FUNC ( sin )
MATH_FUNC ( sinh )
MATH_FUNC ( sqrt )
MATH_FUNC ( tan )
MATH_FUNC ( tanh )

static int const token_funcs_args[ CED_ARGSET_TOTAL ][ CED_FUNC_ARG_MAX + 1 ] =
{
{ -1 },								// CED_ARGSET_NONE

{ 0 },								// CED_ARGSET_VOID
{ CED_FARG_TYPE_NUM },						// CED_ARGSET_NUM,
{ CED_FARG_TYPE_NUM, CED_FARG_TYPE_NUM },			// CED_ARGSET_NUM_NUM,
{ CED_FARG_TYPE_NUM, CED_FARG_TYPE_NUM, CED_FARG_TYPE_NUM },	// CED_ARGSET_NUM_NUM_NUM,
{ CED_FARG_TYPE_CELLRANGE },					// CED_ARGSET_CELLRANGE,
{ CED_FARG_TYPE_CELLRANGE, CED_FARG_TYPE_NUM },			// CED_ARGSET_CELLRANGE_NUM,
{ CED_FARG_TYPE_NUM, CED_FARG_TYPE_CELLRANGE, CED_FARG_TYPE_NUM }, // CED_ARGSET_NUM_CELLRANGE_NUM,
{ CED_FARG_TYPE_CELLREF, CED_FARG_TYPE_NUM, CED_FARG_TYPE_NUM }, // CED_ARGSET_CELLREF_NUM_NUM,
{ CED_FARG_TYPE_CELLRANGE, CED_FARG_TYPE_STRING, CED_FARG_TYPE_NUM }, // CED_ARGSET_CELLRANGE_STR_NUM,
{ CED_FARG_TYPE_CELLRANGE, CED_FARG_TYPE_STRING, CED_FARG_TYPE_NUM, CED_FARG_TYPE_CELLREF },	// CED_ARGSET_CELLRANGE_STR_NUM_CELLREF,
{ CED_FARG_TYPE_CELLREF, CED_FARG_TYPE_CELLRANGE, CED_FARG_TYPE_NUM } // CED_ARGSET_CELLREF_CELLRANGE_NUM,

};

/*
List must be in alpha order w.r.t. name for ced_token_get.
All names must be in lower case for the lexer.
*/
static CedToken const token_funcs[] = {
{"abs",		CED_ARGSET_NUM,		fabs_func,	0 },
{"acos",	CED_ARGSET_NUM,		acos_func,	0 },
{"asin",	CED_ARGSET_NUM,		asin_func,	0 },
{"atan",	CED_ARGSET_NUM,		atan_func,	0 },
{"average",	CED_ARGSET_CELLRANGE,	average,	CED_TOKEN_FLAG_VOLATILE },
{"ceil",	CED_ARGSET_NUM,		ceil_func,	0 },
{"cos",		CED_ARGSET_NUM,		cos_func,	0 },
{"cosh",	CED_ARGSET_NUM,		cosh_func,	0 },
{"count",	CED_ARGSET_CELLRANGE,	count,		CED_TOKEN_FLAG_VOLATILE },
{"counta",	CED_ARGSET_CELLRANGE,	counta,		CED_TOKEN_FLAG_VOLATILE },
{"countblank",	CED_ARGSET_CELLRANGE,	countblank,	CED_TOKEN_FLAG_VOLATILE },
{"countif",	CED_ARGSET_CELLRANGE_STR_NUM, countif,	CED_TOKEN_FLAG_VOLATILE },
{"date",	CED_ARGSET_NUM_NUM_NUM,	date_func,	0 },
{"day",		CED_ARGSET_NUM,		day_func,	0 },
{"degrees",	CED_ARGSET_NUM,		degrees,	0 },
{"exp",		CED_ARGSET_NUM,		exp_func,	0 },
{"floor",	CED_ARGSET_NUM,		floor_func,	0 },
{"frac",	CED_ARGSET_NUM,		frac,		0 },
{"hour",	CED_ARGSET_NUM,		hour_func,	0 },
{"if",		CED_ARGSET_NUM_NUM_NUM,	if_func,	0 },
{"int",		CED_ARGSET_NUM,		int_func,	0 },
{"large",	CED_ARGSET_CELLRANGE_NUM, large,	CED_TOKEN_FLAG_VOLATILE },
{"ln",		CED_ARGSET_NUM,		log_func,	0 },
{"max",		CED_ARGSET_VARIABLE,	max,		CED_TOKEN_FLAG_VOLATILE },
{"median",	CED_ARGSET_CELLRANGE,	median,		CED_TOKEN_FLAG_VOLATILE },
{"min",		CED_ARGSET_VARIABLE,	min,		CED_TOKEN_FLAG_VOLATILE },
{"minute",	CED_ARGSET_NUM,		minute_func,	0 },
{"mod",		CED_ARGSET_NUM_NUM,	fmod_func,	0 },
{"month",	CED_ARGSET_NUM,		month_func,	0 },
{"now",		CED_ARGSET_VOID,	now_func,	CED_TOKEN_FLAG_VOLATILE },
{"offset",	CED_ARGSET_CELLREF_NUM_NUM, offset,	CED_TOKEN_FLAG_VOLATILE },
{"percentile",	CED_ARGSET_CELLRANGE_NUM, percentile,	CED_TOKEN_FLAG_VOLATILE },
{"percentrank",	CED_ARGSET_CELLRANGE_NUM, percentrank,	CED_TOKEN_FLAG_VOLATILE },
{"pi",		CED_ARGSET_VOID,	pi_func,	0 },
{"radians",	CED_ARGSET_NUM,		radians,	0 },
{"rand",	CED_ARGSET_VOID,	rnd,		CED_TOKEN_FLAG_VOLATILE },
{"rank",	CED_ARGSET_NUM_CELLRANGE_NUM, rank,	CED_TOKEN_FLAG_VOLATILE },
{"rgb",		CED_ARGSET_NUM_NUM_NUM,	rgb_func,	0 },
{"round",	CED_ARGSET_NUM_NUM,	round_func,	0 },
{"second",	CED_ARGSET_NUM,		second_func,	0 },
{"sin",		CED_ARGSET_NUM,		sin_func,	0 },
{"sinh",	CED_ARGSET_NUM,		sinh_func,	0 },
{"small",	CED_ARGSET_CELLRANGE_NUM, small,	CED_TOKEN_FLAG_VOLATILE },
{"sqrt",	CED_ARGSET_NUM,		sqrt_func,	0 },
{"strvlookup",	CED_ARGSET_CELLREF_CELLRANGE_NUM, strvlookup, CED_TOKEN_FLAG_VOLATILE },
{"sum",		CED_ARGSET_CELLRANGE,	sum_cellrange,	CED_TOKEN_FLAG_VOLATILE },
{"sumif",	CED_ARGSET_CELLRANGE_STR_NUM_CELLREF, sumif, CED_TOKEN_FLAG_VOLATILE },
{"tan",		CED_ARGSET_NUM,		tan_func,	0 },
{"tanh",	CED_ARGSET_NUM,		tanh_func,	0 },
{"time",	CED_ARGSET_NUM_NUM_NUM,	time_func,	0 },
{"today",	CED_ARGSET_VOID,	today_func,	CED_TOKEN_FLAG_VOLATILE },
{"trunc",	CED_ARGSET_NUM_NUM,	trunc_func,	0 },
{"vlookup",	CED_ARGSET_NUM_CELLRANGE_NUM, vlookup,	CED_TOKEN_FLAG_VOLATILE },
{"weekday",	CED_ARGSET_NUM,		weekday_func,	0 },
{"year",	CED_ARGSET_NUM,		year_func,	0 },
};


CedToken const * ced_token_get (
	char	const * const	text
	)
{
	int		p1 = 0,
			p2,
			p3,
			cmp;


	p2 = sizeof ( token_funcs ) / sizeof ( token_funcs[0] ) - 1;

	while ( p1 <= p2 )
	{
		p3 = (p1 + p2) / 2;
		cmp = strcmp ( text, token_funcs[p3].name );
		if ( cmp == 0 )
		{
			return &token_funcs[p3];	// Found
		}

		if ( cmp < 0 )
		{
			p2 = p3 - 1;
		}
		else
		{
			p1 = p3 + 1;			// cmp > 0
		}
	}

	return NULL;
}

static int check_arg_types (
	CedFuncState	* const	state
	)
{
	int		i;


	for ( i = 0; i <= CED_FUNC_ARG_MAX; i++ )
	{
		if ( state->arg[i].type !=
			token_funcs_args[ state->token->type ][i] )
		{
			// The argument types don't match
			state->parser->ced_errno =
				CED_ERROR_BAD_FUNCTION_ARGUMENTS;
			state->parser->flag = CED_PARSER_FLAG_ERROR;

			return 1;	// Fail
		}

		if ( state->arg[i].type == 0 )
		{
			break;		// Terminator
		}
	}

	return 0;			// Arguments valid
}

int ced_token_exe (
	CedFuncState	* const	state
	)
{
	if ( state->token->type != CED_ARGSET_VARIABLE )
	{
		if ( check_arg_types ( state ) )
		{
			return 1;
		}
	}

	state->parser->ced_errno = 0;

	if ( state->token->func ( state ) )
	{
		if ( state->parser->ced_errno == 0 )
		{
			// Function error not determined

			state->parser->ced_errno =
				CED_ERROR_BAD_FUNCTION_OPERATION;
		}

		state->parser->flag = CED_PARSER_FLAG_ERROR;

		return 1;
	}

	return 0;
}

