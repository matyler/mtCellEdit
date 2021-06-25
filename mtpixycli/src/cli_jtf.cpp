/*
	Copyright (C) 2016-2020 Mark Tyler

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



int Backend::jtf_help (
	char	const * const *	const	args
	)
{
	return get_help ( args );
}

int Backend::jtf_info (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
#define PFHEAD "%-25s : "

	char const * const fn = file().get_filename ();
	char const * const ft = pixy_file_type_text ( file().get_filetype () );

	printf ( PFHEAD "%i\n", "File slot #", get_file_number () );
	printf ( PFHEAD "'%s'\n", "File name", fn ? fn : "" );
	printf ( PFHEAD "%s\n", "File type", ft ? ft : "" );
	printf ( PFHEAD "%i\n", "Modified", file().get_modified () );

	char		buf[128];
	mtPixmap	const * pixmap = file().get_pixmap ();

	pixy_pixmap_print_geometry ( pixmap, buf, sizeof ( buf ) );
	printf ( PFHEAD "%s\n", "Image geometry", buf );

	if ( pixmap )
	{
		printf ( PFHEAD "%i + %i\n", "Undo + Redo",
			file().get_undo_steps (),
			file().get_redo_steps () );

		printf ( PFHEAD "%.1f MB\n", "Undo Memory",
			file().get_undo_mb () );

		int	urp, pnip, pt;
		int	pf [ PIXY_PALETTE_COLOR_TOTAL_MAX ];

		if ( pixy_pixmap_get_information( pixmap, &urp, &pnip, pf, &pt))
		{
			printf ( "Error analysing image\n" );
		}
		else
		{
			int const ca = file().brush.get_color_a_index();
			int const cb = file().brush.get_color_b_index();
			char * mask = file().palette_mask.color;
			mtColor const * const col =
				&pixy_pixmap_get_palette_const ( pixmap )->
				color[0];


			printf ( PFHEAD "%i\n", "Unique RGB Pixels", urp );
			printf ( PFHEAD "%i\n", "Pixels not in palette", pnip );

			printf ( "------------------------------------\n" );
			printf ( "   i       RGB       Mask AB  Freq\n" );
			printf ( "------------------------------------\n" );

			for ( int i = 0; i < pt; i++ )
			{
				buf[0] = 0;

				if ( ca == i )
				{
					mtkit_strnncat ( buf, "A", sizeof(buf));
				}

				if ( cb == i )
				{
					mtkit_strnncat ( buf, "B", sizeof(buf));
				}

				int const msk = mask[i];

				printf ( "%4i  " "%4i%4i%4i" "%5i" "  %2s   "
					"%i\n", i, col[i].red, col[i].green,
					col[i].blue, msk, buf, pf[i] );
			}

			printf ( "------------------------------------\n" );
		}
	}

	buf[0] = 0;
	pixmap = clipboard.get_pixmap ();
	pixy_pixmap_print_geometry ( pixmap, buf, sizeof(buf) );
	printf ( PFHEAD "%s\n", "Clipboard", buf );

	return 0;
}

int Backend::jtf_load (
	char	const * const *	const	args
	)
{
	return file().load_image ( args[0], 1, 3, "" );
}

int Backend::jtf_new (
	char	const * const *	const	args
	)
{
	int			w, h, im_type;
	mtKit::CharInt	const	chint_tab[] = {
				{ "rgb",	PIXY_PIXMAP_BPP_RGB },
				{ "indexed",	PIXY_PIXMAP_BPP_INDEXED },
				{ NULL, 0 }
				};


	if (	mtKit::cli_parse_int ( args[0], w, PIXY_PIXMAP_WIDTH_MIN,
			PIXY_PIXMAP_WIDTH_MAX )
		|| mtKit::cli_parse_int ( args[1], h, PIXY_PIXMAP_HEIGHT_MIN,
			PIXY_PIXMAP_HEIGHT_MAX )
		|| mtKit::cli_parse_charint ( args[2], chint_tab, im_type )
		)
	{
		return 1;
	}

	return file().new_image ( im_type, w, h, 1, 3, "" );
}

int Backend::jtf_quit (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	exit.abort ();

	return 0;
}

int Backend::jtf_redo (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return operation_update ( file().redo () );
}

int Backend::prep_save (
	int		&comp,
	int		&ft
	)
{
	comp = 6;
	ft = file().get_filetype ();

	switch ( ft )
	{
	case PIXY_FILE_TYPE_JPEG:
		comp = 85;
		break;

	case PIXY_FILE_TYPE_BMP:
	case PIXY_FILE_TYPE_GIF:
	case PIXY_FILE_TYPE_PNG:
		comp = 6;
		break;

	default:
		return 1;
	}

	return 0;
}

int Backend::jtf_save (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	int	comp;
	int	ft;

	if ( prep_save ( comp, ft ) )
	{
		return 1;
	}

	char	const	* fn = file().get_filename ();

	return file().save_image ( fn, ft, comp );
}

int Backend::jtf_save_as (
	char	const * const *	const	args
	)
{
	int	comp;
	int	ft;

	prep_save ( comp, ft );

	if ( args[1] )
	{
		mtKit::CharInt	const	chint_tab[] = {
				{ "bmp",	PIXY_FILE_TYPE_BMP },
				{ "gif",	PIXY_FILE_TYPE_GIF },
				{ "jpeg",	PIXY_FILE_TYPE_JPEG },
				{ "png",	PIXY_FILE_TYPE_PNG },
				{ NULL, 0 }
				};
		int		fti;

		if ( mtKit::cli_parse_charint ( args[1], chint_tab, fti ) )
		{
			return 1;
		}

		ft = fti;

		if ( ! args[2] )
		{
			if ( ft == PIXY_FILE_TYPE_JPEG )
			{
				comp = 85;
			}
		}
		else
		{
			int	cmin = 0, cmax = 9;

			if ( ft == PIXY_FILE_TYPE_JPEG )
			{
				cmax = 100;
			}

			if ( mtKit::cli_parse_int( args[2], comp, cmin, cmax ) )
			{
				return 1;
			}
		}
	}

	return file().save_image ( args[0], ft, comp );
}

int Backend::jtf_save_undo (
	char	const * const *	const	args
	)
{
	return file().export_undo_images ( args[0] );
}

int Backend::jtf_text (
	char	const * const *	const	args
	)
{
	int size, eff[4] = { 0, 0, 0, 0 };
	mtKit::CharInt	const	chint_tab[] = {
			{ "bold",		0 },
			{ "italic",		1 },
			{ "underline",		2 },
			{ "strikethrough",	3 },
			{ NULL, 0 }
			};


	if ( mtKit::cli_parse_int ( args[2], size, 1, 100 ) )
	{
		return 1;
	}

	for ( int i = 3; i < 7 && args[i]; i++ )
	{
		int res;

		if ( mtKit::cli_parse_charint( args[i], chint_tab, res ) )
		{
			return 1;
		}

		eff[res] = 1;
	}

	return file().clipboard_render_text ( clipboard,
		args[0], args[1], size, eff[0], eff[1], eff[2], eff[3] );
}

int Backend::jtf_undo (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return operation_update ( file().undo () );
}

