/*
	Copyright (C) 2016-2017 Mark Tyler

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



int jtf_help (
	char	const * const *	const	args
	)
{
	return backend.get_help ( args );
}

int jtf_info (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
#define PFHEAD "%-25s : "

	char const * const fn = backend.file().get_filename ();
	char const * const ft = mtPixy::File::type_text (
					backend.file().get_filetype () );

	printf ( PFHEAD "%i\n", "File slot #", backend.get_file_number () );
	printf ( PFHEAD "'%s'\n", "File name", fn ? fn : "" );
	printf ( PFHEAD "%s\n", "File type", ft ? ft : "" );
	printf ( PFHEAD "%i\n", "Modified", backend.file().get_modified () );

	char		buf[128];
	mtPixy::Image * im = backend.file().get_image ();

	mtPixy::image_print_geometry ( im, buf, sizeof ( buf ) );
	printf ( PFHEAD "%s\n", "Image geometry", buf );

	if ( im )
	{
		printf ( PFHEAD "%i + %i\n", "Undo + Redo",
			backend.file().get_undo_steps (),
			backend.file().get_redo_steps () );

		printf ( PFHEAD "%.1f MB\n", "Undo Memory",
			backend.file().get_undo_mb () );

		int		urp, pnip, pt;
		int		pf [ mtPixy::Palette::COLOR_TOTAL_MAX ];

		if ( im->get_information ( urp, pnip, pf, pt ) )
		{
			printf ( "Error analysing image\n" );
		}
		else
		{
			int const ca = backend.file().brush.get_color_a_index();
			int const cb = backend.file().brush.get_color_b_index();
			char * mask = backend.file().palette_mask.color;
			mtPixy::Color * col = im->get_palette ()->get_color ();


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

	im = backend.clipboard.get_image ();
	mtPixy::image_print_geometry ( im, buf, sizeof(buf) );
	printf ( PFHEAD "%s\n", "Clipboard", buf );

	return 0;
}

int jtf_load (
	char	const * const *	const	args
	)
{
	return backend.file().load_image ( args[0], 1, 3 );
}

int jtf_new (
	char	const * const *	const	args
	)
{
	int			w, h, im_type;
	mtKit::CharInt	const	chint_tab[] = {
				{ "rgb",	mtPixy::Image::RGB },
				{ "indexed",	mtPixy::Image::INDEXED },
				{ NULL, 0 }
				};


	if (	mtKit::cli_parse_int ( args[0], w, mtPixy::Image::WIDTH_MIN,
			mtPixy::Image::WIDTH_MAX )
		|| mtKit::cli_parse_int ( args[1], h, mtPixy::Image::HEIGHT_MIN,
			mtPixy::Image::HEIGHT_MAX )
		|| mtKit::cli_parse_charint ( args[2], chint_tab, im_type )
		)
	{
		return 1;
	}

	return backend.file().new_image((mtPixy::Image::Type)im_type, w, h, 1,
		3 );
}

int jtf_quit (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	backend.m_exit_now = 1;

	return 0;
}

int jtf_redo (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return operation_update ( backend.file().redo () );
}

static int prep_save (
	int			&comp,
	mtPixy::File::Type	&ft
	)
{
	comp = 5;
	ft = backend.file().get_filetype ();

	switch ( ft )
	{
	case mtPixy::File::JPEG:
		comp = 85;
		break;

	case mtPixy::File::BMP:
	case mtPixy::File::GIF:
	case mtPixy::File::PNG:
		comp = 5;
		break;

	default:
		return 1;
	}

	return 0;
}

int jtf_save (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	int			comp;
	mtPixy::File::Type	ft;

	if ( prep_save ( comp, ft ) )
	{
		return 1;
	}

	char	const	* fn = backend.file().get_filename ();

	return backend.file().save_image ( fn, ft, comp );
}

int jtf_save_as (
	char	const * const *	const	args
	)
{
	int			comp;
	mtPixy::File::Type	ft;

	prep_save ( comp, ft );

	if ( args[1] )
	{
		mtKit::CharInt	const	chint_tab[] = {
				{ "bmp",	mtPixy::File::BMP },
				{ "gif",	mtPixy::File::GIF },
				{ "jpeg",	mtPixy::File::JPEG },
				{ "png",	mtPixy::File::PNG },
				{ NULL, 0 }
				};
		int		fti;

		if ( mtKit::cli_parse_charint ( args[1], chint_tab, fti ) )
		{
			return 1;
		}

		ft = (mtPixy::File::Type)fti;

		if ( ! args[2] )
		{
			if ( ft == mtPixy::File::JPEG )
			{
				comp = 85;
			}
		}
		else
		{
			int	cmin = 0, cmax = 9;

			if ( ft == mtPixy::File::JPEG )
			{
				cmax = 100;
			}

			if ( mtKit::cli_parse_int( args[0], comp, cmin, cmax ) )
			{
				return 1;
			}
		}
	}

	return backend.file().save_image ( args[0], ft, comp );
}

int jtf_save_undo (
	char	const * const *	const	args
	)
{
	return backend.file().export_undo_images ( args[0] );
}

int jtf_text (
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

	return backend.file().clipboard_render_text ( backend.clipboard,
		args[0], args[1], size, eff[0], eff[1], eff[2], eff[3] );
}

int jtf_undo (
	char	const * const *	const	ARG_UNUSED ( args )
	)
{
	return operation_update ( backend.file().undo () );
}

