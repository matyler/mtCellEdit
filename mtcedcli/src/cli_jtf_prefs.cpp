/*
	Copyright (C) 2012-2020 Mark Tyler

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



void Backend::prefs_init ()
{
	m_uprefs.add_int ( MAIN_ROW_PAD, m_row_pad, 1 );
	m_uprefs.add_string ( MAIN_FONT_NAME, m_font_name, "Sans" );
	m_uprefs.add_int ( MAIN_FONT_SIZE, m_font_size, 12 );
	m_uprefs.add_int ( CUI_INIFILE_PAGE_WIDTH, m_page_width, 297 );
	m_uprefs.add_int ( CUI_INIFILE_PAGE_HEIGHT, m_page_height, 210 );
	m_uprefs.add_int ( CUI_INIFILE_PAGE_MARGIN_X, m_page_margin_x, 10 );
	m_uprefs.add_int ( CUI_INIFILE_PAGE_MARGIN_Y, m_page_margin_y, 10 );
	m_uprefs.add_int ( CUI_INIFILE_PAGE_HEADER_LEFT, m_page_header_left,
		2, 0, 6 );
	m_uprefs.add_int ( CUI_INIFILE_PAGE_HEADER_CENTRE, m_page_header_centre,
		0, 0, 6 );
	m_uprefs.add_int ( CUI_INIFILE_PAGE_HEADER_RIGHT, m_page_header_right,
		3, 0, 6 );
	m_uprefs.add_int ( CUI_INIFILE_PAGE_FOOTER_LEFT, m_page_footer_left,
		6, 0, 6 );
	m_uprefs.add_int ( CUI_INIFILE_PAGE_FOOTER_CENTRE, m_page_footer_centre,
		0, 0, 6 );
	m_uprefs.add_int ( CUI_INIFILE_PAGE_FOOTER_RIGHT, m_page_footer_right,
		4, 0, 6 );
}

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

int Backend::jtf_set_prefs_book (
	char	const * const * const	args
	)
{
	CedBookPrefs	* const bp = &file()->cubook->book->prefs;
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

		return 2;
	}

	int		num;

	if ( mtKit::cli_parse_int ( args[1], num, 0, -1 ) )
	{
		return 2;
	}

	int * const ip = (int *)vp;
	ip[0] = num;

	return 0;
}

int Backend::jtf_set_prefs_cell (
	char	const * const * const	args
	)
{
	CedSheet * const sh = sheet ();

	if ( ! sh )
	{
		return 2;
	}

	int		pref_id;
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

	if ( mtKit::cli_parse_charint ( args[0], ctab, pref_id ) )
	{
		fprintf ( stderr, "No such cell preference name" );

		return 1;
	}

	int		num = 0;
	char	const	* charp = NULL;

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
			return 2;
		}
	}

	CedSheet	* tmp_sheet = NULL;
	int		r1, r2, c1, c2;

	if ( cui_cellprefs_init ( sh, &r1, &c1, &r2, &c2, &tmp_sheet ) )
	{
		fprintf ( stderr, "Error setting up temp sheet\n\n" );

		return 2;
	}

	int const res = cui_cellprefs_change ( file()->cubook, sh,
		r1, c1, r2, c2, tmp_sheet, pref_id, charp, num );

	ced_sheet_destroy ( tmp_sheet );
	tmp_sheet = NULL;

	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 2;
	}

	if ( res > 0 )
	{
		fprintf ( stderr, "Error setting cell preferences\n\n" );

		return 2;
	}

	return 0;
}

int Backend::jtf_set_prefs_cellborder (
	char	const * const * const	args
	)
{
	if ( ! sheet() )
	{
		return 2;
	}

	int		num;

	if ( mtKit::cli_parse_int ( args[0], num, CUI_CELLBORD_MIN,
		CUI_CELLBORD_MAX )
		)
	{
		return 2;
	}

	int const res = cui_cellprefs_border ( file(), num );

	undo_report_updates ( res );

	if (	res == CUI_ERROR_LOCKED_CELL ||
		res == CUI_ERROR_NO_CHANGES
		)
	{
		return 2;
	}

	if ( res > 0 )
	{
		fprintf ( stderr, "Error setting cell border\n\n" );

		return 2;
	}

	return 0;
}

int Backend::jtf_set_prefs_sheet (
	char	const * const * const	args
	)
{
	CedSheet * const sh = sheet ();

	if ( ! sh )
	{
		return 2;
	}

	// The sheet prefs are purely int
	int		num;

	if ( mtKit::cli_parse_int ( args[1], num, 0, -1 ) )
	{
		return 2;
	}

	void		* vp;
	CedSheetPrefs	* const sp = &sh->prefs;
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
		fprintf ( stderr, "Unable to set sheet preference\n\n" );

		return 2;
	}

	int * const ip = (int *)vp;

	ip[0] = num;

	return 0;
}

int Backend::jtf_set_prefs_state (
	char	const * const * const	args
	)
{
	if ( 0 == strcmp ( args[0], MAIN_FONT_NAME ) )
	{
		prefs().set ( args[0], args[1] );
	}
	else
	{
		int num;

		// The state is purely ints or options with no limits
		if ( mtKit::cli_parse_int ( args[1], num, 0, -1 ) )
		{
			return 2;
		}

		prefs().set ( args[0], num );
	}

	return 0;
}

int Backend::jtf_print_prefs_book (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	CedBookPrefs	* const bp = &file()->cubook->book->prefs;

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

int Backend::jtf_print_prefs_sheet (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	CedSheet * const sh = sheet ();

	if ( ! sh )
	{
		return 2;
	}

	CedSheetPrefs * const p = &sh->prefs;

	printf ( "cursor_r1 = %i\n", p->cursor_r1 );
	printf ( "cursor_c1 = %i\n", p->cursor_c1 );
	printf ( "cursor_r2 = %i\n", p->cursor_r2 );
	printf ( "cursor_c2 = %i\n", p->cursor_c2 );
	printf ( "split_r1 = %i\n", p->split_r1 );
	printf ( "split_r2 = %i\n", p->split_r2 );
	printf ( "split_c1 = %i\n", p->split_c1 );
	printf ( "split_c2 = %i\n", p->split_c2 );
	printf ( "start_row = %i\n", p->start_row );
	printf ( "start_col = %i\n", p->start_col );
	printf ( "locked = %i\n", p->locked );

	return 0;
}

int Backend::jtf_print_prefs_state (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	prefs().scan_prefs ( []( mtKit::PrefType ARG_UNUSED(type),
		std::string	const & key,
		std::string	const & ARG_UNUSED(type_name),
		std::string	const & var_value,
		bool			ARG_UNUSED(var_default)
		)
		{
			std::cout << key << " = '" << var_value << "'\n";
		} );

	return 0;
}

