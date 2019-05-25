/*
	Copyright (C) 2018-2019 Mark Tyler

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

#include <string.h>

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

int jtf_app_cardshuff (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	std::string	txt;

	backend.db.get_well ()->app_card_shuffle ( txt );

	std::cout << txt << "\n";

	return 0;
}

int jtf_app_cointoss (
	char	const * const * const	args
	)
{
	std::string	txt;
	int	const	totmin = mtDW::Well::COIN_TOTAL_MIN;
	int	const	totmax = mtDW::Well::COIN_TOTAL_MAX;
	int		tot = 0;

	if ( mtKit::cli_parse_int ( args[0], tot, totmin, totmax ) )
	{
		return 1;
	}

	backend.db.get_well ()->app_coin_toss ( txt, tot );

	std::cout << txt << "\n";

	return 0;
}

int jtf_app_declist (
	char	const * const * const	args
	)
{
	std::string	txt;
	int	const	totmin = mtDW::Well::DECLIST_TOTAL_MIN;
	int	const	totmax = mtDW::Well::DECLIST_TOTAL_MAX;
	int		tot = 0;
	double	const	minlo = mtDW::Well::DECLIST_MIN_LO;
	double	const	minhi = mtDW::Well::DECLIST_MIN_HI;
	double		min = 0.0;
	double	const	maxlo = mtDW::Well::DECLIST_MAX_LO;
	double	const	maxhi = mtDW::Well::DECLIST_MAX_HI;
	double		max = 0.0;

	if (	mtKit::cli_parse_int ( args[0], tot, totmin, totmax )	||
		mtKit::cli_parse_double ( args[1], min, minlo, minhi )	||
		mtKit::cli_parse_double ( args[2], max, maxlo, maxhi )
		)
	{
		return 1;
	}

	backend.db.get_well ()->app_declist ( txt, tot, min, max );

	std::cout << txt << "\n";

	return 0;
}

int jtf_app_diceroll (
	char	const * const * const	args
	)
{
	std::string	txt;
	int	const	totmin = mtDW::Well::DICE_TOTAL_MIN;
	int	const	totmax = mtDW::Well::DICE_TOTAL_MAX;
	int		tot = 0;
	int	const	facemin = mtDW::Well::DICE_FACES_MIN;
	int	const	facemax = mtDW::Well::DICE_FACES_MAX;
	int		faces = 0;

	if (	mtKit::cli_parse_int ( args[0], tot, totmin, totmax )	||
		mtKit::cli_parse_int ( args[1], faces, facemin, facemax )
		)
	{
		return 1;
	}

	backend.db.get_well ()->app_dice_rolls ( txt, tot, faces );

	std::cout << txt << "\n";

	return 0;
}

int jtf_app_intlist (
	char	const * const * const	args
	)
{
	std::string	txt;
	int	const	totmin = mtDW::Well::INTLIST_TOTAL_MIN;
	int	const	totmax = mtDW::Well::INTLIST_TOTAL_MAX;
	int		tot = 0;
	int	const	minlo = mtDW::Well::INTLIST_MIN_LO;
	int	const	minhi = mtDW::Well::INTLIST_MIN_HI;
	int		min = 0;
	int	const	rangelo = mtDW::Well::INTLIST_RANGE_MIN;
	int	const	rangehi = mtDW::Well::INTLIST_RANGE_MAX;
	int		range = 0;

	if (	mtKit::cli_parse_int ( args[0], tot, totmin, totmax )	||
		mtKit::cli_parse_int ( args[1], min, minlo, minhi )	||
		mtKit::cli_parse_int ( args[2], range, rangelo, rangehi )
		)
	{
		return 1;
	}

	backend.db.get_well ()->app_intlist ( txt, tot, min, range );

	std::cout << txt << "\n";

	return 0;
}

int jtf_app_numshuff (
	char	const * const * const	args
	)
{
	std::string	txt;
	int	const	totmin = mtDW::Well::NUMSHUFF_TOTAL_MIN;
	int	const	totmax = mtDW::Well::NUMSHUFF_TOTAL_MAX;
	int		tot = 0;

	if ( mtKit::cli_parse_int ( args[0], tot, totmin, totmax ) )
	{
		return 1;
	}

	backend.db.get_well ()->app_number_shuffle ( txt, tot );

	std::cout << txt << "\n";

	return 0;
}

int jtf_app_password (
	char	const * const * const	args
	)
{
	std::string	txt;
	int	const	totmin = mtDW::Well::PASSWORD_TOTAL_MIN;
	int	const	totmax = mtDW::Well::PASSWORD_TOTAL_MAX;
	int		tot = 0;
	int	const	chmin = mtDW::Well::PASSWORD_CHAR_MIN;
	int	const	chmax = mtDW::Well::PASSWORD_CHAR_MAX;
	int		chtot = mtDW::Well::PASSWORD_CHAR_DEFAULT;
	bool		lower = false;
	bool		upper = false;
	bool		num = false;
	bool		other = false;
	char	const *	other_txt = mtDW::Well::PASSWORD_OTHER_DEFAULT;

	for ( int i = 2; args[i]; i++ )
	{
		if ( 0 == strcmp ( args[i], "lower" ) )
		{
			lower = true;
		}
		else if ( 0 == strcmp ( args[i], "upper" ) )
		{
			upper = true;
		}
		else if ( 0 == strcmp ( args[i], "num" ) )
		{
			num = true;
		}
		else if ( 0 == strcmp ( args[i], "other" ) )
		{
			other = true;
		}
		else
		{
			other_txt = args[i];
		}
	}

	if (	mtKit::cli_parse_int ( args[0], tot, totmin, totmax )	||
		mtKit::cli_parse_int ( args[1], chtot, chmin, chmax )
		)
	{
		return 1;
	}

	if ( ! other )
	{
		other_txt = "";
	}

	backend.db.get_well ()->app_passwords ( mtDW::AppPassword ( lower,
		upper, num, other_txt ), chtot, txt, tot );

	std::cout << txt << "\n";

	return 0;
}

int jtf_app_pins (
	char	const * const * const	args
	)
{
	std::string	txt;
	int	const	totmin = mtDW::Well::PIN_TOTAL_MIN;
	int	const	totmax = mtDW::Well::PIN_TOTAL_MAX;
	int		tot = 0;
	int	const	digmin = mtDW::Well::PIN_DIGITS_MIN;
	int	const	digmax = mtDW::Well::PIN_DIGITS_MAX;
	int		digits = 0;

	if (	mtKit::cli_parse_int ( args[0], tot, totmin, totmax )	||
		mtKit::cli_parse_int ( args[1], digits, digmin, digmax )
		)
	{
		return 1;
	}

	backend.db.get_well ()->app_pins ( txt, tot, digits );

	std::cout << txt << "\n";

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

	return backend.db.get_butt ()->add_buckets ( backend.db.get_well (),
		tot );
}

int jtf_butt_add_otp (
	char	const * const * const	args
	)
{
	return backend.db.get_butt ()->add_otp ( args[0] );
}

int jtf_butt_add_random_otp (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	mtDW::Well * const well = backend.db.get_well ();
	mtDW::Butt * const butt = backend.db.get_butt ();
	std::string name;

	butt->get_new_name ( well, name );

	std::cout << name << "\n";

	return butt->add_otp ( name.c_str () );
}

int jtf_butt_delete_otp (
	char	const * const * const	args
	)
{
	return backend.db.get_butt ()->delete_otp ( args[0] );
}

int jtf_butt_empty (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	return backend.db.get_butt ()->empty_buckets ();
}

int jtf_butt_import_otp (
	char	const * const * const	args
	)
{
	return backend.db.get_butt ()->import_otp ( args[0] );
}

int jtf_butt_info (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	mtDW::Butt * const butt = backend.db.get_butt ();
	std::string const status = butt->is_read_only () ?
		"Read Only" : "Read Write";

	std::vector<mtDW::OTPinfo> list;
	butt->get_otp_list ( list );

	std::cout << "\n"
		<< "Total OTP names    : " << list.size () << "\n"
		<< "\n"
		<< "Active OTP name    : " << butt->get_otp_name () << "\n"
		<< "Comment            : " << butt->get_comment () << "\n"
		<< "Status             : " << status << "\n"
		<< "\n"
		<< "Buckets (total)    : " << butt->get_bucket_total () << "\n"
		<< "Buckets (used)     : " << butt->get_bucket_used () << "\n"
		<< "Bucket (position)  : " << butt->get_bucket_position() <<"\n"
		<< "\n";

	return 0;
}

int jtf_butt_list (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	mtDW::Butt * const butt = backend.db.get_butt ();

	std::cout << "\n";

	std::vector<mtDW::OTPinfo> list;
	butt->get_otp_list ( list );

	std::cout
	<< "    #  Name          Read Write Buckets  Comment\n"
	<< "-------------------------------------------------------------------"
	<< "\n";

	for ( size_t i = 0; i < list.size (); i++ )
	{
		printf ( "%5zu  %-20s  %s  %6i  %s\n", i+1,
			list[i].m_name.c_str (),
			(list[i].m_status & 1) ? "R " : "RW",
			list[i].m_buckets,
			list[i].m_comment.c_str () );
	}

	std::cout << "\n";

	return 0;
}

int jtf_butt_set_comment (
	char	const * const * const	args
	)
{
	return backend.db.get_butt ()->set_comment ( args[0] );
}

int jtf_butt_set_otp (
	char	const * const * const	args
	)
{
	return backend.db.get_butt ()->set_otp ( args[0] );
}

int jtf_butt_set_read_only (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	backend.db.get_butt ()->set_read_only ();

	return 0;
}

int jtf_butt_set_read_write (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	backend.db.get_butt ()->set_read_write ();

	return 0;
}

int jtf_db (
	char	const * const * const	args
	)
{
	return backend.db.open ( args[0] );
}

int jtf_info (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	jtf_well_info ( NULL );

	puts ( "--------" );

	jtf_butt_info ( NULL );

	puts ( "--------" );

	jtf_soda_info ( NULL );

	jtf_butt_list ( NULL );

	return 0;
}

int jtf_soda_decode (
	char	const * const * const	args
	)
{
	mtDW::Butt * const butt = backend.db.get_butt ();

	return mtDW::Soda::decode ( butt, args[0], args[1] );
}

int jtf_soda_encode (
	char	const * const * const	args
	)
{
	mtDW::Butt * const butt = backend.db.get_butt ();

	return backend.db.get_soda ()->encode ( butt, args[0], args[1] );
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
		<< "OTP                : " << soda.m_otp_name	<< "\n"
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
	mtDW::Soda * soda = backend.db.get_soda ();

	std::cout << "\n"
		<< "Soda Mode          : " << soda->get_mode () << "\n"
		<< "\n";

	return 0;
}

int jtf_soda_multi_decode (
	char	const * const * const	args
	)
{
	mtDW::Butt * const butt = backend.db.get_butt ();

	return mtDW::Soda::multi_decode ( butt, args[0], args[1] );
}

int jtf_soda_multi_encode (
	char	const * const * const	args
	)
{
	mtDW::Butt * const butt = backend.db.get_butt ();

	return backend.db.get_soda ()->multi_encode ( butt, args[0], args[1],
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

	backend.db.get_soda ()->set_mode ( mode );

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
	int		res;

	if ( tap.open_soda ( args[0], res ) )
	{
		return 1;
	}

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
	mtDW::Butt * const butt = backend.db.get_butt ();

	return mtDW::Tap::decode ( butt, args[0], args[1] );
}

int jtf_tap_multi_decode (
	char	const * const * const	args
	)
{
	mtDW::Butt * const butt = backend.db.get_butt ();

	return mtDW::Tap::multi_decode ( butt, args[0], args[1] );
}

int jtf_tap_encode (
	char	const * const * const	args
	)
{
	mtDW::Well * const well = backend.db.get_well ();
	mtDW::Butt * const butt = backend.db.get_butt ();
	mtDW::Soda * const soda = backend.db.get_soda ();
	mtDW::Tap const * const tap = backend.db.get_tap ();

	return tap->encode ( well, butt, soda, args[0], args[1], args[2] );
}

int jtf_well_empty (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	backend.db.get_well ()->empty ();

	return 0;
}

int jtf_well_info (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	mtDW::Well * well = backend.db.get_well ();

	std::cout << "\n"
	<< "DB Path            : " << backend.db.get_path () << "\n"
	<< "\n"
	<< "Seed               : " << well->get_seed () << "\n"
	;

	int shifts[8] = { 0 };
	well->get_shifts ( shifts );

	printf ( "Shifts             : %i, %i, %i, %i, %i, %i, %i, %i\n",
		shifts[0], shifts[1], shifts[2], shifts[3],
		shifts[4], shifts[5], shifts[6], shifts[7] );

	std::cout
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
	backend.db.get_well ()->add_path ( args[0] );

	return 0;
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

	return backend.db.get_well ()->save_file ( bytes, args[1] );
}

int jtf_well_seed (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	backend.db.get_well ()->set_seed_by_time ();

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

	backend.db.get_well ()->set_seed ( (uint64_t)seed );

	return 0;
}

int jtf_well_reset_shifts (
	char	const * const * const	ARG_UNUSED ( args )
	)
{
	backend.db.get_well ()->set_shifts ();

	return 0;
}

