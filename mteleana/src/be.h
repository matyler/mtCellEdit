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

extern "C" {

	#include <stdlib.h>
	#include <string.h>
	#include <math.h>
	#include <ctype.h>
	#include <unistd.h>
	#include <sys/stat.h>
	#include <sys/types.h>

	#include <cairo.h>

	#ifdef CAIRO_HAS_PDF_SURFACE
	#include <cairo-pdf.h>
	#if (CAIRO_VERSION_MAJOR > 1) || ((CAIRO_VERSION_MAJOR == 1) && (CAIRO_VERSION_MINOR >= 6))
		#define USE_CAIRO_EPS
	#endif
	#endif

	#ifdef CAIRO_HAS_PS_SURFACE
	#include <cairo-ps.h>
	#endif

	#ifdef CAIRO_HAS_SVG_SURFACE
	#include <cairo-svg.h>
	#endif
}

#include <mtkit.h>
#include <mtcelledit.h>
#include <mtpixy.h>



class	eleanaCore;
class	eleanaIndex;
class	eleanaElection;



#define FULL_ROW_TITLES		2
#define FULL_ROW_PARTY1		4

#define FULL_COL_SEAT_NAME	1
#define FULL_COL_ELECTORATE	2
#define FULL_COL_MP_NAME	3
#define FULL_COL_PARTY		4
#define FULL_COL_VOTES		5
#define FULL_COL_COUNTY		6
#define FULL_COL_REGION		7

#define POLYMAP_WIDTH		640
#define POLYMAP_HEIGHT		1234
#define POLYMAP_ZOOM_MIN	0.3
#define POLYMAP_ZOOM_MAX	25.0
#define	CARTOGRAM_WIDTH		40
#define CARTOGRAM_HEIGHT	48



enum	// NOTE: Must be in same order as comboMapMode
{
	MAP_MODE_MIN			= 0,

	MAP_MODE_WINNER			= 0,
	MAP_MODE_PARTY_PLACING		= 1,
	MAP_MODE_PARTY_VOTE_SHARE	= 2,
	MAP_MODE_MARGINALITY		= 3,
	MAP_MODE_TURNOUT		= 4,

	MAP_MODE_MAX			= 4
};

enum
{
	ELEANA_INDEX_COL_YEAR		= 1,
	ELEANA_INDEX_COL_FULL		= 3,
	ELEANA_INDEX_COL_CARTOGRAM	= 4,
	ELEANA_INDEX_COL_POLYMAP	= 5,

	ELEANA_INDEX_COL_TOTAL
};

enum
{
	POLYMAP_FILETYPE_NONE		= -1,

#ifdef USE_CAIRO_EPS
	POLYMAP_FILETYPE_EPS,
#endif

#ifdef CAIRO_HAS_PDF_SURFACE
	POLYMAP_FILETYPE_PDF,
#endif

#ifdef CAIRO_HAS_PNG_FUNCTIONS
	POLYMAP_FILETYPE_PNG,
#endif

#ifdef CAIRO_HAS_PS_SURFACE
	POLYMAP_FILETYPE_PS,
#endif

#ifdef CAIRO_HAS_SVG_SURFACE
	POLYMAP_FILETYPE_SVG,
#endif

	POLYMAP_FILETYPE_TOTAL
};

enum
{
	EL_TAB_SHEET_ROW,	// Main results sheet for this seat

	EL_TAB_REGION,
	EL_TAB_COUNTY,
	EL_TAB_CARTOGRAM_X,
	EL_TAB_CARTOGRAM_Y,

	EL_TAB_WINNER_RGB,
	EL_TAB_TURNOUT,		// 0=30% or less	255=90% or more
	EL_TAB_MARGINALITY,	// 0=0%			255=20% or more
	EL_TAB_VOTES,

	EL_TAB_TOTAL
};

#define PREFS_LAST_MAP_FILE_NAME	"main.last_map_file_name"
#define PREFS_LAST_MAP_FILE_FORMAT	"main.last_map_file_format"

#define PREFS_MAIN_WINDOW	"main.window"
#define PREFS_WINDOW_X		"main.window_x"
#define PREFS_WINDOW_Y		"main.window_y"
#define PREFS_WINDOW_W		"main.window_w"
#define PREFS_WINDOW_H		"main.window_h"

#define PREFS_LAST_YEAR		"main.last_year"



typedef void (* elFindCB) (	// Called when using eleana_find_text
	int		seat_id,
	char	const	* seat_text,
	char	const	* cell_type,	// Seat, Candidate, Party,
					// County, Region
	char	const	* cell_text,
	void		* user
	);

typedef int (* elRenderFunc) (
	int		seat,
	int		** rgba_fill,	// Put fill RGBA here -
					// NULL => don't fill
	int		** rgba_outline, // Put outline RGBA here -
					// NULL => don't outline
	void		* user_data
	);
	// 0 = render seat
	// 1 = don't render seat
	// 2 = terminate



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



void be_cline (
	int			argc,
	char	const * const * argv
	);



class eleanaIndex
{
public:
	eleanaIndex	();
	~eleanaIndex	();

	int		load ( char const * filename );

	char	const *	getText (		// Get index contents
				int row,	// 0 = first active record
				int col		// ELEANA_INDEX_COL_*
				);
				// NULL = no data

	CedSheet	* getCountySheet ();
	CedSheet	* getRegionSheet ();
	int		getRecords () const;
	int		getPartyRGB ( char const * party );
			// -1=Error else packed RGB for party

private:
	char		* dsPath;	// Dynamic string pointing to index path
	int		iRecords;	// Number of records (rows) in the index

	CedBook		* bookIndex;	// Whole book containing all index
					// sheets
	CedSheet	* sheetIndex;	// Index sheet

	mtTree		* treePartyRGB;	// Key=Name, data=RGB
};

class eleanaElection
{
public:
	eleanaElection	();
	~eleanaElection	();

	CedSheet	* getResults ();

	CedSheet	* createSummary ();
	mtPixy::Image	* createDiagram (
				eleanaIndex * eindex,
				char const * party_a,
				char const * party_b
				);

	int		getSeats () const;

	int		load ( eleanaIndex * eindex, int election );
	void		findText (
				char const * text,
				elFindCB callback,
				void * user
			);
	int		getCartogramXY (
				int seat_id,
				int * x,
				int * y
				) const;
	int		getSeatRGB (
				int seat_id,
				int * rgb,
				int mode,	// MAP_MODE_*
				char const * party_name
				) const;
	int		getPolyMinXY (
				int row,
				double * x,
				double * y
			);
	int		getSeatFromMap (
				int x,
				int y,
				double map_zoom
			);	// Return seat ID, -1=error/not found
	int		renderPolymap (
				cairo_t ** cr,	// Caller destroys + surface
				int x,
				int y,
				int wmax,
				int hmax,
				double zoom,
				elRenderFunc callback,
				void * user_data,
				int map_mode,
				char const * map_party_name
			);
	int		savePolymap (
				char const * filename,
				double zoom,
				int filetype,	// POLYMAP_FILETYPE_*
				int map_mode,
				char const * map_party_name
			);
	int		getTableValue (
				int seat_id,
				int col,	// EL_TAB_*
				int * val
				) const;
	int		setTableValue (
				int seat_id,
				int col,	// EL_TAB_*
				int val
				);

private:
	void		clear ();
	int		loadPolymap ( char const * filename );
	void		createMapData ( mtPixy::Image * const image );

// -----------------------------------------------------------------------------

	CedSheet	* sheetResults;
	mtTree		* treePolymap;
	int		* iTable;
	CedIndex	* indexSeatID;	// Key=Seat name; item->col=SeatID
	CedSheet	* sheetSummary;

	int		iSeats;
};

