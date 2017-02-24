/*
	Copyright (C) 2011-2016 Mark Tyler

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



#define MAX_GRAPH_ID		10000



char const * be_graph_new (
	CedBook		* const	book
	)
{
	static char	const	* graph_text_default =
"/*\n"
"	mtCellEdit Graph Template\n"
"\n"
"	Created		%i-%i-%i %02i:%02i:%02i\n"
"*/\n"
"\n"
"\n"
"{ page\n"
"	width=\"640\"\n"
"	height=\"320\"\n"
"	line_color=\"-1\"\n"
"	fill_color=\"-1\"\n"
"}\n"
"\n"
"{ graph\n"
"	fill_color=\"0xffffff\"\n"
"	line_color=\"0\"\n"
"	x_pad=\"10\"\n"
"	y_pad=\"5\"\n"
"	antialias=\"0\"\n"
"}\n"
"\n"
"{ plot\n"
"	fill_color=\"0xdddddd\"\n"
"	x_pad=\"5\"\n"
"	y_pad=\"5\"\n"
"	line_color=\"0\"\n"
"	antialias=\"0\"\n"
"}\n"
"\n"
"{ x_axis\n"
"	min=\"0\"\n"
"	max=\"100\"\n"
"}\n"
"\n"
"{ y_axis\n"
"	min=\"10\"\n"
"	max=\"-10\"\n"
"}\n"
"\n"
"{ plot_x_axis_grid\n"
"	gap=\"1\"\n"
"	line_color=\"rgb(200,200,200)\"\n"
"	antialias=\"0\"\n"
"}\n"
"\n"
"{ plot_y_axis_grid\n"
"	gap=\"2\"\n"
"	line_color=\"rgb(200,200,200)\"\n"
"	antialias=\"0\"\n"
"}\n"
"\n"
"{ plot_x_axis_grid\n"
"	gap=\"5\"\n"
"	line_color=\"rgb(150,150,150)\"\n"
"	antialias=\"0\"\n"
"}\n"
"\n"
"{ plot_y_axis_grid\n"
"	gap=\"10\"\n"
"	line_color=\"rgb(150,150,150)\"\n"
"	antialias=\"0\"\n"
"}\n"
"\n"
"{ plot_x_axis_top\n"
"	text=\"Graph Title\"\n"
"	text_size=\"20\"\n"
"	x_justify=\"0.5\"\n"
"	y_justify=\"0.5\"\n"
"	y_pad=\"10\"\n"
"}\n"
"\n"
"{ plot_x_axis\n"
"	size=\"5\"\n"
"	gap=\"10\"\n"
"	text_size=\"10\"\n"
"	x_justify=\"0\"\n"
"	y_pad=\"5\"\n"
"	antialias=\"0\"\n"
"}\n"
"\n"
"{ plot_y_axis\n"
"	size=\"5\"\n"
"	gap=\"2\"\n"
"	text_size=\"10\"\n"
"	y_justify=\"0\"\n"
"	x_pad=\"5\"\n"
"	antialias=\"0\"\n"
"}\n"
"\n"
"{ plot_x_axis\n"
"	text=\"Input\"\n"
"	text_size=\"14\"\n"
"	y_pad=\"0\"\n"
"}\n"
"\n"
"{ plot_y_axis\n"
"	text=\"Output\"\n"
"	text_size=\"14\"\n"
"	x_pad=\"0\"\n"
"}\n"
"\n"
"\n"
;
	char		graph_name[256],
			* text,
			txt[2048];
	int		i;
	time_t		now;
	struct tm	* now_tm;


	now = time ( NULL );
	now_tm = localtime ( &now );

	for ( i = 1; i < MAX_GRAPH_ID; i++ )
	{
		snprintf ( graph_name, sizeof ( graph_name ), "Graph %i", i );
		if ( ! cui_graph_get ( book, graph_name ) )
		{
			break;
		}

		// Keep looping until we find an unused name
	}

	if ( i == MAX_GRAPH_ID )
	{
		return NULL;
	}

	snprintf ( txt, sizeof ( txt ),
		graph_text_default, now_tm->tm_year + 1900,
		now_tm->tm_mon + 1, now_tm->tm_mday, now_tm->tm_hour,
		now_tm->tm_min, now_tm->tm_sec );

	text = strdup ( txt );
	if ( ! text )
	{
		return NULL;
	}

	if ( cui_graph_new ( book, text, (int)strlen ( text ), graph_name )
		== NULL )
	{
		free ( text );

		return NULL;
	}

	mtkit_strfreedup ( &book->prefs.active_graph, graph_name );

	return book->prefs.active_graph;
}

char * be_graph_duplicate (
	CuiBook		* const	cubook
	)
{
	char	const	* oldname = cubook->book->prefs.active_graph;
	char		* newname;


	if ( ! oldname )
	{
		return NULL;
	}

	if ( cui_graph_duplicate ( cubook, oldname, &newname ) )
	{
		return NULL;
	}

	return newname;
}

int be_graph_selection_clip (
	CedSheet	* const	sheet,
	char		* const	buf,
	size_t		const	buflen
	)
{
	if ( ! sheet || ! sheet->book_tnode || ! sheet->book_tnode->key )
	{
		return 1;
	}

	int		r1, c1, r2, c2;
	char		sheetname [ buflen ];
	char		cellrange [ buflen ];
	char		* smax, * dest;
	char	const	* src;


	// Convert sheet name to valid utree string
	src = (char const *)sheet->book_tnode->key;
	smax = sheetname + buflen - 3;

	for ( dest = sheetname; src[0] != 0 && dest < smax; dest ++ )
	{
		if (	src[0] == '\\' ||
			src[0] == '"'
			)
		{
			*dest++ = '\\';
		}

		dest[0] = *src++;
	}

	dest[0] = 0;

	ced_sheet_cursor_max_min ( sheet, &r1, &c1, &r2, &c2 );

	snprintf ( cellrange, buflen, "r%ic%i:r%ic%i", r1, c1, r2, c2 );
	cellrange [ buflen - 1 ] = 0;

	snprintf ( buf, buflen,
		"{ plot_graph_bar\n"
		"	fill_color=\"0x9999FF\"\n"
		"	gap=\"1\"\n"
		"	sheet=\"%s\"\n"
		"	data=\"%s\"\n"
		"	antialias=\"0\"\n"
		"}\n",
		sheetname, cellrange );

	return 0;			// Success
}

