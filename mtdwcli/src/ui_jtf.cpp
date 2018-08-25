/*
	Copyright (C) 2018 Mark Tyler

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

#include "ui.h"



int jtf_about (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	printf ( "%s\n"
		"Copyright (C) " MT_COPYRIGHT_YEARS " Mark Tyler\n"
		"Type 'help' for command hints.  Read the manual for more "
		"specific info.\n\n", VERSION );

	return 0;
}

int jtf_help (
	char	const * const * const	args
	)
{
	return backend.get_help ( args );
}

int jtf_quit (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	backend.exit.abort ();

	return 0;
}

int jtf_butt_add_buckets (
	char	const * const * const	args
	)
{
	int tot = 0;

	if ( mtKit::cli_parse_int ( args[0], tot, 1, 1024 ) )
	{
		return 1;
	}

	return backend.get_butt ()->add_buckets ( backend.get_well (), tot );
}

int jtf_butt_add_name (
	char	const * const * const	args
	)
{
	return backend.get_butt ()->add_name ( args[0] );
}

int jtf_butt_info (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	mtDW::Butt * const butt = backend.get_butt ();

	std::cout << "\n"
		<< "Name               : " << butt->get_name () << "\n"
		<< "Buckets (total)    : " << butt->get_bucket_total () << "\n"
		<< "Buckets (used)     : " << butt->get_bucket_used () << "\n"
		<< "Bucket (position)  : " << butt->get_bucket_position() <<"\n"
		<< "\n";

	return 0;
}

static int butt_name_scan (
	mtTreeNode	* const node,
	void		* const user_data
	)
{
	int * const tot = (int *)user_data;

	tot[0]++;

	std::cout << tot[0] << "\t" << (char const *)node->key << "\n";

	return 0;	// Continue
}

int jtf_butt_list (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	mtDW::Butt * const butt = backend.get_butt ();
	int tot = 0;

	std::cout << "\n";

	mtTree * tree = butt->get_name_list ();
	mtkit_tree_scan ( tree, butt_name_scan, &tot, 0 );

	std::cout << "\n";

	mtkit_tree_destroy ( tree );
	tree = NULL;

	return 0;
}

int jtf_butt_set_name (
	char	const * const * const	args
	)
{
	return backend.get_butt ()->set_name ( args[0] );
}

int jtf_soda_decode (
	char	const * const * const	args
	)
{
	mtDW::Butt * const butt = backend.get_butt ();

	return mtDW::Soda::decode ( butt, args[0], args[1] );
}

int jtf_soda_encode (
	char	const * const * const	args
	)
{
	mtDW::Butt * const butt = backend.get_butt ();

	return backend.get_soda ()->encode ( butt, args[0], args[1] );
}

int jtf_soda_file_info (
	char	const * const * const	args
	)
{
	mtDW::SodaFile soda;

	if ( soda.open ( args[0] ) )
	{
		return 1;
	}

	std::cout << "\n"
		<< "Filename           : " << args[0]		<< "\n"
		<< "Soda size          : " << soda.m_filesize	<< "\n"
		<< "Mode               : " << soda.m_mode_raw	<< "\n"
		;

	if ( ! soda.m_mode_raw )
	{
		std::cout
		<< "Butt               : " << soda.m_butt_name	<< "\n"
		<< "Bucket             : " << soda.m_bucket	<< "\n"
		<< "Bucket Position    : " << soda.m_bucket_pos	<< "\n"
		;
	}

	puts ("");

	return 0;
}

int jtf_soda_info (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	mtDW::Soda * soda = backend.get_soda ();

	std::cout << "\n"
		<< "Mode               : " << soda->get_mode () << "\n"
		<< "\n";

	return 0;
}

int jtf_soda_multi_decode (
	char	const * const * const	args
	)
{
	mtDW::Butt * const butt = backend.get_butt ();

	return mtDW::Soda::multi_decode ( butt, args[0], args[1] );
}

int jtf_soda_multi_encode (
	char	const * const * const	args
	)
{
	mtDW::Butt * const butt = backend.get_butt ();

	return backend.get_soda ()->multi_encode ( butt, args[0], args[1],
		args + 2 );
}

int jtf_soda_set_mode (
	char	const * const * const	args
	)
{
	int mode = 0;

	if ( mtKit::cli_parse_int ( args[0], mode, 1, -1 ) )
	{
		return 1;
	}

	backend.get_soda ()->set_mode ( mode );

	return 0;
}

int jtf_tap_file_info (
	char	const * const * const	args
	)
{
	std::cout << "\n"
		<< "Filename           : " << args[0] << "\n"
		;

	mtDW::TapFile	tap;
	char	const *	type;
	int	const	res = tap.open_soda ( args[0] );

	switch ( res )
	{
	case mtDW::TapFile::TYPE_RGB:
		type = "Empty RGB Image";
		break;

	case mtDW::TapFile::TYPE_SND:
		type = "Empty Audio";
		break;

	case mtDW::TapFile::TYPE_RGB_1:
		type = "Soda RGB Image (3bpp)";
		break;

	case mtDW::TapFile::TYPE_SND_1:
		type = "Soda Audio";
		break;

	// Future additions go here

	case mtDW::TapFile::TYPE_INVALID:
	default:
		type = "Invalid";
		break;
	}

	std::cout
		<< "Bottle type        : " << type << "\n"
		;

	switch ( res )
	{
	case mtDW::TapFile::TYPE_RGB:
	case mtDW::TapFile::TYPE_RGB_1:
	case mtDW::TapFile::TYPE_SND:
	case mtDW::TapFile::TYPE_SND_1:
		{
			std::cout
			<< "Bottle capacity    : " << tap.get_capacity ()
			<< "\n"
			;

			break;
		}

	// Future additions go here

	}

	puts ("");

	return 0;
}

int jtf_tap_decode (
	char	const * const * const	args
	)
{
	mtDW::Butt * const butt = backend.get_butt ();

	return mtDW::Tap::decode ( butt, args[0], args[1] );
}

int jtf_tap_multi_decode (
	char	const * const * const	args
	)
{
	mtDW::Butt * const butt = backend.get_butt ();

	return mtDW::Tap::multi_decode ( butt, args[0], args[1] );
}

int jtf_tap_encode (
	char	const * const * const	args
	)
{
	mtDW::Butt * const butt = backend.get_butt ();
	mtDW::Tap const * const tap = backend.get_tap ();

	return tap->encode ( butt, args[0], args[1], args[2] );
}

int jtf_well_empty (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	backend.get_well ()->empty ();

	return 0;
}

int jtf_well_info (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	mtDW::Well * well = backend.get_well ();

	std::cout << "\n"
		<< "Seed               : " << well->get_seed () << "\n"
		;

	int shifts[8] = { 0 };
	well->get_shifts ( shifts );

	printf ( "Shifts             : %i, %i, %i, %i, %i, %i, %i, %i\n",
		shifts[0], shifts[1], shifts[2], shifts[3],
		shifts[4], shifts[5], shifts[6], shifts[7] );

	std::cout
	<< "Path               : " << well->get_path () << "\n"
	<< "Files done         : " << well->get_files_done () << "\n"
	<< "Files to do        : " << well->get_files_todo () << "\n"
	;

	puts ( "" );

	return 0;
}

int jtf_well_add_path (
	char	const * const * const	args
	)
{
	return backend.get_well ()->add_path ( args[0] );
}

int jtf_well_save_file (
	char	const * const * const	args
	)
{
	int bytes = 0;

	if ( mtKit::cli_parse_int ( args[0], bytes, 1, INT_MAX ) )
	{
		return 1;
	}

	return backend.get_well ()->save_file ( bytes, args[1] );
}

int jtf_well_seed (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	backend.get_well ()->set_seed_by_time ();

	return 0;
}

int jtf_well_seed_int (
	char	const * const * const	args
	)
{
	int seed = 0;

	if ( mtKit::cli_parse_int ( args[0], seed, 1, 0 ) )
	{
		return 1;
	}

	backend.get_well ()->set_seed ( (uint64_t)seed );

	return 0;
}

