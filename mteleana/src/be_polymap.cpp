/*
	Copyright (C) 2009-2016 Mark Tyler

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

#include "be.h"



typedef struct mapPOLYdata	mapPOLYdata;
typedef struct mapPOLYkey	mapPOLYkey;
typedef struct mapPOLYpoint	mapPOLYpoint;



struct mapPOLYpoint
{
	double		x,
			y;
};

// Polygon key & data items for an mtTree

struct mapPOLYkey			// Sorted by miny first, then seat
{
	int		seat;		// Seat index for global.table[?]
					// position
	double		minx,
			maxx,		// bounding
			miny,
			maxy;
};

struct mapPOLYdata
{
	int		points;		// Number of points in this polygon
	mapPOLYpoint	* array;	// Array of points
};



static int ptree_cmp (
	void	const * const	k1,
	void	const * const	k2
	)
{
	mapPOLYkey	const * const key1 = (mapPOLYkey const *)k1;
	mapPOLYkey	const * const key2 = (mapPOLYkey const *)k2;


	if ( key1->miny < key2->miny )
	{
		return -1;
	}

	if ( key1->miny > key2->miny )
	{
		return 1;
	}

	if ( key1->seat < key2->seat )
	{
		return -1;
	}

	if ( key1->seat > key2->seat )
	{
		return 1;
	}

	return 0;			// identical
}

static void ptree_del (
	mtTreeNode	* node
	)
{
	mapPOLYkey	* key = (mapPOLYkey *)node->key;
	mapPOLYdata	* data = (mapPOLYdata *)node->data;


	free ( data->array );
	free ( data );
	free ( key );
}

static int parse_poly (
	mtTree		* const	tree,
	int		const	seat,
	char	const	* const	text
	)
{
	mapPOLYkey	* key;
	mapPOLYdata	* data;
	int		i,
			nodes,
			literal;
	char	const	* st;
	char		* unsafe;


	nodes = mtkit_strtok_count ( text, "," );
	if ( nodes < 3 )
	{
		// Nothing to do as there aren't enough nodes

		return 0;
	}

	key = (mapPOLYkey *)calloc ( 1, sizeof ( mapPOLYkey ) );
	if ( ! key )
	{
		return 1;	// Memory error
	}

	data = (mapPOLYdata *)calloc ( 1, sizeof ( mapPOLYdata ) );
	if ( ! data )
	{
		free ( key );

		return 1;	// Memory error
	}

	data->array = (mapPOLYpoint *)calloc ( (size_t)nodes,
		sizeof ( mapPOLYpoint ) );

	if ( ! data->array )
	{
		goto fail;
	}

	data->points = nodes - 1;
	st = text;
	literal = 0;

	for ( i = 0; i < nodes; i++ )
	{
		while ( isspace ( st[0] ) )
		{
			st ++;
		}

		switch ( st[0] )
		{
		case 'Z':
		case 'z':
			goto finish;

		case 'm':
		case 'l':
			literal = 0;
			st ++;
			break;

		case 'M':
		case 'L':
			literal = 1;
			st ++;
			break;
		}

		while ( isspace ( st[0] ) )
		{
			st ++;
		}

		if ( mtkit_strtod ( st, &data->array[i].x, &unsafe, 0 ) )
		{
			goto fail;
		}

		st = unsafe + 1;	// ','

		if ( mtkit_strtod ( st, &data->array[i].y, &unsafe, 0 ) )
		{
			goto fail;
		}

		st = unsafe;

		if ( ! literal && i > 0 )
		{
			data->array[i].x += data->array[i - 1].x;
			data->array[i].y += data->array[i - 1].y;
		}

		if ( i == 0 )
		{
			key->minx = key->maxx = data->array[i].x;
			key->miny = key->maxy = data->array[i].y;
		}
		else
		{
			key->minx = MIN ( key->minx, data->array[i].x );
			key->maxx = MAX ( key->maxx, data->array[i].x );
			key->miny = MIN ( key->miny, data->array[i].y );
			key->maxy = MAX ( key->maxy, data->array[i].y );
		}
	}

finish:

	key->seat = seat;
	if ( mtkit_tree_node_add ( tree, key, data ) == 0 )
	{
		goto fail;
	}

	return 0;			// Success

fail:
	free ( data->array );
	free ( data );
	free ( key );

	return 1;
}



typedef struct
{
	cairo_t		* cr;
	cairo_surface_t	* cr_surface;
	cairo_t		** cr_dest;

	int		x;		// Offset that the image origin starts
	int		y;		// at.
	mtTree		* tree;		// Polygon tree
	double		zoom;		// Zoom factor for the polygons

	elRenderFunc	render_cb;
	void		* render_cb_user_data;

	double		winx1;
	double		winx2;		// Visible window on polygons
	double		winy1;
	double		winy2;		// At the original polygon scale

	mapPOLYkey	* key;
	mapPOLYdata	* data;

	char	const	* filename;
	int		wmax;
	int		hmax;

	int		rgba_party[5][4];

	eleanaElection	* election;
	int		map_mode;
	char	const	* map_party_name;
} polySTATE;



static int default_render_cb (
	int		const		seat,
	int		** const	rgba_fill,
	int		** const	rgba_outline,
	void		* const		user_data
	)
{
	int		rgb = 0;
	polySTATE	* const	state = (polySTATE *)user_data;


	state->election->getSeatRGB ( seat, &rgb, state->map_mode,
		state->map_party_name );

	state->rgba_party[0][0] = mtPixy::int_2_red ( rgb );
	state->rgba_party[0][1] = mtPixy::int_2_green ( rgb );
	state->rgba_party[0][2] = mtPixy::int_2_blue ( rgb );

	rgba_fill[0] = state->rgba_party[0];
	rgba_outline[0] = state->rgba_party[4];

	return 0;			// Always render
}



#define GAP 1



static int poly_recurse (
	polySTATE	* const	state,
	mtTreeNode	* const	node
	)
{
	int		res = 0;
	int		* rgba_fill;
	int		* rgba_outline;


	if ( node->left )
	{
		res = poly_recurse ( state, node->left );

		if ( res )
		{
			return res;
		}
	}

	state->key = (mapPOLYkey *)node->key;
	state->data = (mapPOLYdata *)node->data;

	rgba_fill = NULL;
	rgba_outline = NULL;

	if (	( state->key->maxx + GAP ) >= state->winx1 &&
		( state->key->minx - GAP ) <= state->winx2 &&
		( state->key->maxy + GAP ) >= state->winy1 &&
		( state->key->miny - GAP ) <= state->winy2 &&
		0 == (res = state->render_cb ( state->key->seat, &rgba_fill,
			&rgba_outline, state->render_cb_user_data ) )
		)
	{
		int		i;
		double		x, y;


		cairo_new_path ( state->cr );

		for ( i = 0; i < state->data->points; i++ )
		{
			x = state->data->array[i].x * state->zoom - state->x;
			y = state->data->array[i].y * state->zoom - state->y;

			cairo_line_to ( state->cr, x, y );
		}

		cairo_close_path ( state->cr );

		if ( rgba_fill )
		{
			cairo_set_source_rgb ( state->cr,
				((double)rgba_fill[0]) / 255,
				((double)rgba_fill[1]) / 255,
				((double)rgba_fill[2]) / 255 );
			cairo_fill_preserve ( state->cr );
		}

		if ( rgba_outline )
		{
			cairo_set_source_rgb ( state->cr,
				((double)rgba_outline[0]) / 255,
				((double)rgba_outline[1]) / 255,
				((double)rgba_outline[2]) / 255 );
			cairo_stroke_preserve ( state->cr );
		}
	}

	if ( res == 2 )
	{
		return 2;		// User requested stop
	}

	if ( node->right && state->key->miny <= state->winy2 )
	{
		res = poly_recurse ( state, node->right );
		if ( res )
		{
			return res;
		}
	}

	return 0;
}

static int tree_scan_missing_cb (
	mtTreeNode	* const	node,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	CedIndexItem	* item;


	item = (CedIndexItem *)node->data;
	if ( ! item )
	{
		return 1;		// Unexpected error
	}

	if ( item->row == 0 )
	{
		// This item hasn't been matched

		printf ( "'%s'\n", (char *)node->key );
	}

	return 0;
}



typedef struct
{
	int		i;
	CedIndex	* idx_seats;
	CedIndexItem	* index_item;
} prepSTATE;



static int result_scan_seats (
	CedSheet	* const	ARG_UNUSED ( sheet ),
	CedCell		* const	cell,
	int		const	ARG_UNUSED ( row ),
	int		const	ARG_UNUSED ( col ),
	void		* const	user_data
	)
{
	prepSTATE	* state = (prepSTATE *)user_data;


	// Match seat name in cell with index
	if ( ced_index_query ( state->idx_seats, 0, cell->text,
		&state->index_item ) != 1
		)
	{
		return 1;		// Should never happen
	}

	state->index_item->row = 0;
	state->index_item->col = state->i;
	state->i ++;

	return 0;			// Continue
}

static CedIndex * prepare_idx_seats (
	CedSheet	* const	results
	)
{
	prepSTATE	state = { 0, NULL, NULL };


	state.idx_seats = ced_index_new ( CED_INDEX_TYPE_TEXT );
	if ( ! state.idx_seats )
	{
		return NULL;
	}

	if ( ced_index_add_items ( state.idx_seats, results, FULL_ROW_PARTY1,
		FULL_COL_SEAT_NAME, 0, 1 )
		)
	{
		goto error;
	}

	// For each seat in the index, change row to 0, change col to its index
	state.i = 0;
	if ( ced_sheet_scan_area ( results, FULL_ROW_PARTY1,
		FULL_COL_SEAT_NAME, 0, 1, result_scan_seats, &state )
		)
	{
		goto error;
	}

	return state.idx_seats;

error:
	ced_index_destroy ( state.idx_seats );

	return NULL;
}

int eleanaElection::loadPolymap (
	char	const	* const	filename
	)
{
	char		* svg;
	char		* st;
	char		* d;
	char		* d2;
	char		* id;
	char		* id2;
	int		svg_size;
	int		tot;
	int		not_found = 0;
	int		res;
	CedIndexItem	* index_item;


	if ( ! filename )
	{
		return 1;
	}

	svg = mtkit_file_load ( filename, &svg_size,
		MTKIT_FILE_ZERO | MTKIT_FILE_GUNZIP, NULL );

	if ( ! svg )
	{
		return 1;
	}

	treePolymap = mtkit_tree_new ( ptree_cmp, ptree_del );
	if ( ! treePolymap )
	{
		free ( svg );

		return 1;
	}

	st = svg;

	indexSeatID = prepare_idx_seats ( sheetResults );
	if ( ! indexSeatID )
	{
		goto fail;
	}

	for ( tot = 0; ; tot ++ )
	{
		st = mtkit_strcasestr ( st, "<path" );
		if ( ! st )
		{
			break;
		}

		d = mtkit_strcasestr ( st, "d=\"" );
		if ( ! d )
		{
			break;
		}

		d += 3;

		id = mtkit_strcasestr ( st, "id=\"" );
		if ( ! id )
		{
			break;
		}

		id += 4;

		d2 = strchr ( d, '"' );
		if ( ! d2 )
		{
			break;
		}

		d2[0] = 0;

		id2 = strchr ( id, '"' );
		if ( ! id2 )
		{
			break;
		}

		id2[0] = 0;

		if ( d2 > id2 )
		{
			st = d2 + 1;
		}
		else
		{
			st = id2 + 1;
		}

		res = ced_index_query ( indexSeatID, 0, id, &index_item );
		if ( res < 0 )
		{
			goto fail;
		}

		if ( res == 0 )
		{
			fprintf ( stderr, "ERROR eleana_polymap_load - not in "
				"TSV - '%s'\n", id );

			not_found ++;

			continue;
		}
		else
		{
			// Seat matched so set the found flag
			index_item->row = 1;

			if ( parse_poly ( treePolymap, index_item->col, d ) )
			{
				goto fail;
			}
		}
	}

	if ( not_found )
	{
		fprintf ( stderr, "SVG items not_found = %i\n", not_found );
		mtkit_tree_scan ( indexSeatID->tree, tree_scan_missing_cb,
			NULL, 0 );
	}

	if ( ! treePolymap->root )
	{
		// Nothing found

		goto fail;
	}

	free ( svg );

	return 0;

fail:
	free ( svg );

	ced_index_destroy ( indexSeatID );
	indexSeatID = NULL;

	mtkit_tree_destroy ( treePolymap );
	treePolymap = NULL;

	return 1;
}



typedef struct
{
	mtTreeNode	* node;
	int		idx;
} ePolyState;



static int ePolyCB (
	mtTreeNode	* const node,
	void		* const user_data
	)
{
	ePolyState	* state = (ePolyState *)user_data;
	mapPOLYkey	* poly_key = (mapPOLYkey *)node->key;


	if ( state->idx == poly_key->seat )
	{
		state->node = node;

		return 1;	// Stop
	}

	return 0;		// Continue
}

static mtTreeNode * poly_match (
	mtTree		* const	poly_tree,
	int		const	idx
	)
{
	ePolyState	state = { NULL, idx };


	mtkit_tree_scan ( poly_tree, ePolyCB, &state, 0 );

	return state.node;
}

int eleanaElection::getPolyMinXY (
	int		const	row,
	double		* const	x,
	double		* const	y
	)
{
	if ( ! x || ! y )
	{
		return 1;		// Fail
	}

	int		idx;
	CedCell		* cell;
	CedIndexItem	* item;
	mtTreeNode	* tnode;
	mapPOLYkey	* poly_key;


	cell = ced_sheet_get_cell ( sheetResults, row, FULL_COL_SEAT_NAME );
	if ( ! cell || ! cell->text )
	{
		return 1;		// Fail
	}

	if ( 1 != ced_index_query ( indexSeatID, 0, cell->text, &item ) )
	{
		return 1;		// Fail
	}

	idx = item->col;

	tnode = poly_match ( treePolymap, idx );
	if ( ! tnode )
	{
		return 1;		// Fail
	}

	poly_key = (mapPOLYkey *)tnode->key;

	x[0] = poly_key->minx;
	y[0] = poly_key->miny;

	return 0;			// Success
}



typedef struct
{
	int		last_seat;
	int		rgba[4];
	cairo_t		* cr;
	cairo_surface_t	* cr_surface;
	int		found;
	unsigned char	* rgb;
} uptabSTATE;



static int found_seat (
	uptabSTATE	* const	state
	)
{
	state->cr_surface = cairo_get_target ( state->cr );
	if ( ! state->cr_surface )
	{
		return 0;
	}

	state->rgb = cairo_image_surface_get_data ( state->cr_surface );
	if ( ! state->rgb )
	{
		return 0;
	}

	if ( state->rgb[1] < 128 )
	{
		return 1;	// Seat rendered
	}

	return 0;		// Not found
}

static int uptab_cb (
	int		const		seat,
	int		** const	rgba_fill,
	int		** const	rgba_outline,
	void		* const		user_data
	)
{
	uptabSTATE	* const	state = (uptabSTATE *)user_data;


	if ( found_seat ( state ) )
	{
		// Terminate if the last rendering found a seat
		state->found = 1;

		return 2;
	}

	state->last_seat = seat;

	rgba_fill[0] = state->rgba;
	rgba_outline[0] = state->rgba;

	return 0;			// Render & continue
}

int eleanaElection::getSeatFromMap (
	int		const	x,
	int		const	y,
	double		const	map_zoom
	)
{
	uptabSTATE	state = { -1, {0,0,0,255}, NULL, NULL, 0, NULL };


	if ( renderPolymap ( &state.cr, x, y, 1, 1, map_zoom, uptab_cb,
		&state, 0, NULL ) )
	{
		return -1;
	}

	// Check final seat rendered
	if ( state.found == 0 )
	{
		state.found = found_seat ( &state );
	}

	if ( state.found == 0 )
	{
		// No seat was found

		state.last_seat = -1;
	}

	state.cr_surface = cairo_get_target ( state.cr );

	cairo_destroy ( state.cr );
	cairo_surface_destroy ( state.cr_surface );

	return state.last_seat;
}

static int renderKr (
	polySTATE	* const	state,
	int		const	filetype
	)
{
	int		res	= 1;
	double	const	x	= state->x;
	double	const	y	= state->y;
	double	const	z	= state->zoom;
	double		w	= POLYMAP_WIDTH  * z + 1;
	double		h	= POLYMAP_HEIGHT * z + 1;


	if ( state->wmax )
	{
		w = state->wmax;
	}

	if ( state->hmax )
	{
		h = state->hmax;
	}

	if ( w < 1 || h < 1 )
	{
		return 1;
	}

	state->winx1 = x/z - z/2;
	state->winx2 = (x + w)/z + z/2;
	state->winy1 = y/z - z/2;
	state->winy2 = (y + h)/z + z/2;

	// Create new surface based on new geometry and target file
	switch ( filetype )
	{
#ifdef USE_CAIRO_EPS
	case POLYMAP_FILETYPE_EPS:
		state->cr_surface = cairo_ps_surface_create ( state->filename,
			w, h );
		cairo_ps_surface_set_eps ( state->cr_surface, 1 );
		break;
#endif

#ifdef CAIRO_HAS_PDF_SURFACE
	case POLYMAP_FILETYPE_PDF:
		state->cr_surface = cairo_pdf_surface_create ( state->filename,
			w, h );
		break;
#endif

#ifdef CAIRO_HAS_PNG_FUNCTIONS
	case POLYMAP_FILETYPE_PNG:
#endif
	case POLYMAP_FILETYPE_NONE:
		state->cr_surface = cairo_image_surface_create (
			CAIRO_FORMAT_RGB24, (int)w, (int)h );
		break;

#ifdef CAIRO_HAS_PS_SURFACE
	case POLYMAP_FILETYPE_PS:
		state->cr_surface = cairo_ps_surface_create ( state->filename,
			w, h );
		break;
#endif

#ifdef CAIRO_HAS_SVG_SURFACE
	case POLYMAP_FILETYPE_SVG:
		state->cr_surface = cairo_svg_surface_create ( state->filename,
			w, h );
		break;
#endif
	default:
		goto error;		// Unknown
	}

	if ( cairo_surface_status ( state->cr_surface ) != CAIRO_STATUS_SUCCESS)
	{
		goto error;
	}

	state->cr = cairo_create ( state->cr_surface );

	if ( state->cr_dest )
	{
		state->cr_dest[0] = state->cr;
	}

	// Set background to white
	cairo_set_source_rgb ( state->cr, 1, 1, 1 );
	cairo_paint ( state->cr );

	cairo_set_line_join ( state->cr, CAIRO_LINE_JOIN_ROUND );
	cairo_set_line_width ( state->cr, 1.0 );

	poly_recurse ( state, state->tree->root );


#ifdef CAIRO_HAS_PNG_FUNCTIONS
	if ( filetype == POLYMAP_FILETYPE_PNG )
	{
		if ( cairo_surface_write_to_png ( state->cr_surface,
			state->filename ) != CAIRO_STATUS_SUCCESS )
		{
			goto error;
		}
	}
#endif

	res = 0;
error:

	if ( 0 != res || ! state->cr_dest )
	{
		if ( state->cr_dest )
		{
			state->cr_dest[0] = NULL;
		}

		cairo_destroy ( state->cr );
		cairo_surface_destroy ( state->cr_surface );
	}

	return res;
}

int eleanaElection::renderPolymap (
	cairo_t	**	const	cr,
	int		const	x,
	int		const	y,
	int		const	wmax,
	int		const	hmax,
	double		const	zoom,
	elRenderFunc	const	callback,
	void		* const	user_data,
	int		const	map_mode,
	char	const * const	map_party_name
	)
{
	polySTATE	state = { NULL, NULL, cr, x, y, treePolymap,
				zoom, callback, user_data, 0.0, 0.0, 0.0, 0.0,
				NULL, NULL, NULL, wmax, hmax,
				{
					{0,0,0,255},
					{0,0,0,255},
					{0,0,0,255},
					{0,0,0,255},
					{0,0,0,255}
				}, this, map_mode, map_party_name };


	if ( ! callback )
	{
		state.render_cb = default_render_cb;
		state.render_cb_user_data = &state;
	}

	return renderKr ( &state, POLYMAP_FILETYPE_NONE );
}

int eleanaElection::savePolymap (
	char	const * const	filename,
	double		const	zoom,
	int		const	filetype,
	int		const	map_mode,
	char	const * const	map_party_name
	)
{
	polySTATE	state = { NULL, NULL, NULL, 0, 0, treePolymap,
				zoom, default_render_cb, &state,
				0.0, 0.0, 0.0, 0.0,
				NULL, NULL, filename, 0, 0,
				{
					{0,0,0,255},
					{0,0,0,255},
					{0,0,0,255},
					{0,0,0,255},
					{0,0,0,255}
				}, this, map_mode, map_party_name };


	return renderKr ( &state, filetype );
}

