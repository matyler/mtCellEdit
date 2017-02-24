/*
	Copyright (C) 2012-2016 Mark Tyler

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



typedef struct
{
	char	const	* name;
	void		* ip;
} charVOIDp;



static int match_chvoidp (
	char		const	* const	txt,
	charVOIDp	const	*	chvoidp,
	void		*	* const	result
	)
{
	if ( ! txt || ! chvoidp || ! result )
	{
		return 1;
	}

	for ( ; chvoidp->name; chvoidp ++ )
	{
		if ( strcmp ( chvoidp->name, txt ) == 0 )
		{
			result[0] = chvoidp->ip;

			return 0;
		}
	}

	return 1;			// Not found
}



int jtf_set_prefs_book (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	CedBookPrefs	* bp = &CEDCLI_FILE->cubook->book->prefs;
	int		num, * ip;
	void		* vp;
	charVOIDp const table_s[] = {
			{ "active_sheet", &bp->active_sheet },
			{ "active_graph", &bp->active_graph },
			{ "author", &bp->author },
			{ "comment", &bp->comment },
			{ NULL, NULL }
			};
	charVOIDp const table_i[] = {
			{ "disable_locks", &bp->disable_locks },
			{ "auto_recalc", &bp->auto_recalc },
			{ NULL, NULL }
			};


	// Try strings first
	if ( match_chvoidp ( args[0], table_s, &vp ) == 0 )
	{
		mtkit_strfreedup ( (char **)vp, args[1] );

		return 0;
	}

	// Its not a string so it must be an int
	if ( match_chvoidp ( args[0], table_i, &vp ) != 0 )
	{
		fprintf ( stderr, "Unable to set book preference\n\n" );

		return 1;
	}

	if ( mtKit::cli_parse_int ( args[1], num, 0, -1 ) )
	{
		return 1;
	}

	ip = (int *)vp;
	ip[0] = num;

	return 0;
}

int jtf_set_prefs_cell (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	CedSheet	* sheet,
			* tmp_sheet = NULL;
	char	const	* charp = NULL;
	int		r1, r2, c1, c2,
			pref_id,
			num = 0,
			res;
	mtKit::CharInt	const ctab[] = {
		{ "align_horizontal",	CUI_CELLPREFS_align_horizontal },
		{ "color_background",	CUI_CELLPREFS_color_background },
		{ "color_foreground",	CUI_CELLPREFS_color_foreground },
		{ "format",		CUI_CELLPREFS_format },
		{ "width",		CUI_CELLPREFS_width },
		{ "num_decimal_places",	CUI_CELLPREFS_num_decimal_places },
		{ "num_zeros",		CUI_CELLPREFS_num_zeros },
		{ "text_style",		CUI_CELLPREFS_text_style },
		{ "locked",		CUI_CELLPREFS_locked },
		{ "border_type",	CUI_CELLPREFS_border_type },
		{ "border_color",	CUI_CELLPREFS_border_color },
		{ "format_datetime",	CUI_CELLPREFS_format_datetime },
		{ "num_thousands",	CUI_CELLPREFS_num_thousands },
		{ "text_prefix",	CUI_CELLPREFS_text_prefix },
		{ "text_suffix",	CUI_CELLPREFS_text_suffix },
		{ NULL, 0 }
		};


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return 1;
	}

	if ( mtKit::cli_parse_charint ( args[0], ctab, pref_id ) )
	{
		fprintf ( stderr, "No such cell preference name" );

		return 1;
	}

	if (	pref_id == CUI_CELLPREFS_format_datetime	||
		pref_id == CUI_CELLPREFS_num_thousands		||
		pref_id == CUI_CELLPREFS_text_prefix		||
		pref_id == CUI_CELLPREFS_text_suffix
		)
	{
		charp = args[1];
	}
	else
	{
		if ( mtKit::cli_parse_int ( args[1], num, 0, -1 ) )
		{
			return 1;
		}
	}

	if ( cui_cellprefs_init ( sheet, &r1, &c1, &r2, &c2, &tmp_sheet ) )
	{
		fprintf ( stderr, "Error setting up temp sheet\n\n" );

		return 1;
	}

	res = cui_cellprefs_change ( CEDCLI_FILE->cubook, sheet,
		r1, c1, r2, c2, tmp_sheet, pref_id, charp, num );

	ced_sheet_destroy ( tmp_sheet );

	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 1;
	}

	if ( res > 0 )
	{
		fprintf ( stderr, "Error setting cell preferences\n\n" );

		return 1;
	}

	return 0;
}

int jtf_set_prefs_cellborder (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	int		num,
			res;


	if ( ! cedcli_get_sheet ( state ) )
	{
		return 1;
	}

	if ( mtKit::cli_parse_int ( args[0], num, CUI_CELLBORD_MIN,
		CUI_CELLBORD_MAX )
		)
	{
		return 1;
	}

	res = cui_cellprefs_border ( CEDCLI_FILE, num );
	undo_report_updates ( res );
	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 1;
	}

	if ( res > 0 )
	{
		fprintf ( stderr, "Error setting cell border\n\n" );

		return 1;
	}

	return 0;
}

int jtf_set_prefs_sheet (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	int		num,
			* ip;
	void		* vp;
	CedSheet	* sheet;


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return 1;
	}

	// The sheet prefs are purely int
	if ( mtKit::cli_parse_int ( args[1], num, 0, -1 ) )
	{
		return 1;
	}
	else
	{
		CedSheetPrefs	* sp = &sheet->prefs;
		charVOIDp const table[] = {
				{ "cursor_r1", &sp->cursor_r1 },
				{ "cursor_c1", &sp->cursor_c1 },
				{ "cursor_r2", &sp->cursor_r2 },
				{ "cursor_c2", &sp->cursor_c2 },
				{ "split_r1", &sp->split_r1 },
				{ "split_r2", &sp->split_r2 },
				{ "split_c1", &sp->split_c1 },
				{ "split_c2", &sp->split_c2 },
				{ "start_row", &sp->start_row },
				{ "start_col", &sp->start_col },
				{ "locked", &sp->locked },
				{ NULL, NULL }
				};


		if ( match_chvoidp ( args[0], table, &vp ) )
		{
			fprintf ( stderr,
				"Unable to set sheet preference\n\n" );

			return 1;
		}
	}

	ip = (int *)vp;
	ip[0] = num;

	return 0;
}

int jtf_set_prefs_state (
	CedCli_STATE	* const	state,
	char		** const args
	)
{
	if ( 0 == strcmp ( args[0], MAIN_FONT_NAME ) )
	{
		if ( mtkit_prefs_set_str ( state->prefs, args[0], args[1] ) )
		{
			goto error;
		}
	}
	else
	{
		int		num;


		// The state is purely ints or options with no limits
		if ( mtKit::cli_parse_int ( args[1], num, 0, -1 ) )
		{
			return 1;
		}

		if ( mtkit_prefs_set_int ( state->prefs, args[0], num ) )
		{
			goto error;
		}
	}

	return 0;

error:
	fprintf ( stderr, "Error setting state preference\n\n" );

	return 1;
}

int jtf_print_prefs_book (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	CedBookPrefs	* bp;


	bp = &CEDCLI_FILE->cubook->book->prefs;

	if ( bp->active_sheet )
	{
		printf ( "active_sheet = '%s'\n", bp->active_sheet );
	}

	if ( bp->active_graph )
	{
		printf ( "active_graph = '%s'\n", bp->active_graph );
	}

	if ( bp->author )
	{
		printf ( "author = '%s'\n", bp->author );
	}

	if ( bp->comment )
	{
		printf ( "comment = '%s'\n", bp->comment );
	}

	printf ( "disable_locks = %i\n", bp->disable_locks );
	printf ( "auto_recalc = %i\n", bp->auto_recalc );

	return 0;
}

int jtf_print_prefs_sheet (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	CedSheetPrefs	* sp;
	CedSheet	* sheet;


	sheet = cedcli_get_sheet ( state );
	if ( ! sheet )
	{
		return 1;
	}

	sp = &sheet->prefs;

	printf ( "cursor_r1 = %i\n", sp->cursor_r1 );
	printf ( "cursor_c1 = %i\n", sp->cursor_c1 );
	printf ( "cursor_r2 = %i\n", sp->cursor_r2 );
	printf ( "cursor_c2 = %i\n", sp->cursor_c2 );
	printf ( "split_r1 = %i\n", sp->split_r1 );
	printf ( "split_r2 = %i\n", sp->split_r2 );
	printf ( "split_c1 = %i\n", sp->split_c1 );
	printf ( "split_c2 = %i\n", sp->split_c2 );
	printf ( "start_row = %i\n", sp->start_row );
	printf ( "start_col = %i\n", sp->start_col );
	printf ( "locked = %i\n", sp->locked );

	return 0;
}

static int cb_scan_state (
	mtTreeNode	* const	node,
	void		* const	ARG_UNUSED ( user_data )
	)
{
	mtPrefValue	* iv = (mtPrefValue *)node->data;


	printf ( "%s = '%s'\n", (char *)node->key, iv->value );

	return 0;	// continue
}

int jtf_print_prefs_state (
	CedCli_STATE	* const	state,
	char		** const ARG_UNUSED ( args )
	)
{
	if ( mtkit_tree_scan ( mtkit_prefs_get_tree ( state->prefs ),
		cb_scan_state, NULL, 0 ) )
	{
		fprintf ( stderr, "Problem scanning state preferences\n\n" );
	}

	return 0;
}

int cedcli_prefs_init (
	CedCli_STATE	* state
	)
{
	mtPrefTable	const prefs_table[] = {
{ MAIN_ROW_PAD,			MTKIT_PREF_TYPE_INT, "1", NULL, NULL, 0, NULL, NULL },
{ MAIN_FONT_NAME,		MTKIT_PREF_TYPE_STR, "Sans", NULL, NULL, 0, NULL, NULL },
{ MAIN_FONT_SIZE,		MTKIT_PREF_TYPE_INT, "12", NULL, NULL, 0, NULL, NULL },
{ CUI_INIFILE_PAGE_WIDTH,	MTKIT_PREF_TYPE_INT, "297", NULL, NULL, 0, NULL, NULL },
{ CUI_INIFILE_PAGE_HEIGHT,	MTKIT_PREF_TYPE_INT, "210", NULL, NULL, 0, NULL, NULL },
{ CUI_INIFILE_PAGE_MARGIN_X,	MTKIT_PREF_TYPE_INT, "10", NULL, NULL, 0, NULL, NULL },
{ CUI_INIFILE_PAGE_MARGIN_Y,	MTKIT_PREF_TYPE_INT, "10", NULL, NULL, 0, NULL, NULL },
{ CUI_INIFILE_PAGE_HEADER_LEFT, MTKIT_PREF_TYPE_OPTION, "2", NULL, NULL, 0, NULL, NULL },
{ CUI_INIFILE_PAGE_HEADER_CENTRE, MTKIT_PREF_TYPE_OPTION, "0", NULL, NULL, 0, NULL, NULL },
{ CUI_INIFILE_PAGE_HEADER_RIGHT, MTKIT_PREF_TYPE_OPTION, "3", NULL, NULL, 0, NULL, NULL },
{ CUI_INIFILE_PAGE_FOOTER_LEFT, MTKIT_PREF_TYPE_OPTION, "6", NULL, NULL, 0, NULL, NULL },
{ CUI_INIFILE_PAGE_FOOTER_CENTRE, MTKIT_PREF_TYPE_OPTION, "0", NULL, NULL, 0, NULL, NULL },
{ CUI_INIFILE_PAGE_FOOTER_RIGHT, MTKIT_PREF_TYPE_OPTION, "4", NULL, NULL, 0, NULL, NULL },
{ NULL, 0, NULL, NULL, NULL, 0, NULL, NULL }
};


	state->prefs = mtkit_prefs_new ( prefs_table );
	if ( ! state->prefs )
	{
		fprintf ( stderr, "Unable to create state preferences.\n\n" );

		return 1;
	}

	return 0;
}

int cedcli_prefs_free (
	CedCli_STATE	* const	state
	)
{
	mtkit_prefs_destroy ( state->prefs );
	state->prefs = NULL;

	return 0;
}

