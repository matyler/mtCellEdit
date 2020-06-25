/*
	Copyright (C) 2019-2020 Mark Tyler

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

#include "app_hg.h"



int mtDW::Homoglyph::utf8_analyse (
	std::string	const	&input,
	std::string	const	&covert,
	std::string		&info
	) const
{
	info.clear ();

	FileOps fops ( this );

	if ( fops.load_input_utf8 ( input ) )
	{
		info += "Unable to load input string";
		return 1;
	}

	if ( fops.load_covert_utf8 ( covert ) )
	{
		info += "Unable to load covert string";
		return 1;
	}

	return fops.analyse_input ( info );
}

int mtDW::Homoglyph::utf8_decode (
	std::string	const	&input,
	std::string		&covert,
	std::string		&info,
	std::string		&output
	) const
{
	covert.clear ();
	info.clear ();
	output.clear ();

	FileOps fops ( this );

	if ( fops.load_input_utf8 ( input ) )
	{
		info = "Unable to load input text";
		return 1;
	}

	int const glyph_len = fops.get_utf8_len ();

	if ( glyph_len < 1 )
	{
		info = "Not UTF-8 input text";
		return 1;
	}

	if ( fops.open_output_mem () )
	{
		info = "Unable to open output mem 1";
		return 1;
	}

	if ( fops.cleanse_file ()
		|| fops.get_output_mem_utf8 ( output )
		)
	{
		info = "Unable to cleanse the input to create the output";
		return 1;
	}

	if ( fops.open_output_mem () )
	{
		info = "Unable to open output mem 2";
		return 1;
	}

	if ( fops.decode_hg_file () )
	{
		info = "Unable to decode input";
		return 1;
	}

	if ( fops.get_output_mem_utf8 ( covert ) )
	{
		info = "Covert binary is not UTF-8";
		return 1;
	}

	return 0;
}

int mtDW::Homoglyph::utf8_encode (
	std::string	const	&input,
	std::string	const	&covert,
	std::string		&info,
	std::string		&output,
	Well		* const	well
	) const
{
	info.clear ();
	output.clear ();

	FileOps fops ( this );

	if ( fops.load_input_utf8 ( input ) )
	{
		info = "Unable to load input text";
		return 1;
	}

	int const glyph_len = fops.get_utf8_len ();

	if ( glyph_len < 1 )
	{
		info = "Not UTF-8 input text";
		return 1;
	}

	if ( fops.load_covert_utf8 ( covert ) )
	{
		info = "Unable to load covert text";
		return 1;
	}

	if ( fops.open_output_mem () )
	{
		info = "Unable to open output mem";
		return 1;
	}

	if ( fops.encode_hg_file ( well ) )
	{
		info = "Unable to encode covert text";
		return 1;
	}

	if ( fops.get_output_mem_utf8 ( output ) )
	{
		info = "Unable to create output UTF-8";
		return 1;
	}

	return 0;
}

