/*
	Copyright (C) 2019-2022 Mark Tyler

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

#include "pixyutils.h"



Global::Global ()
{
	jump_table[ "cmp" ]	= [this](){ return pixy_cmp(); };
	jump_table[ "delta" ]	= [this](){ return pixy_delta(); };
	jump_table[ "fade" ]	= [this](){ return pixy_fade(); };
	jump_table[ "ls" ]	= [this](){ return pixy_ls(); };
	jump_table[ "new" ]	= [this](){ return pixy_new(); };
	jump_table[ "pica" ]	= [this](){ return pixy_pica(); };
	jump_table[ "resize" ]	= [this](){ return pixy_resize(); };
	jump_table[ "riba" ]	= [this](){ return pixy_riba(); };
	jump_table[ "rida" ]	= [this](){ return pixy_rida(); };
	jump_table[ "risa" ]	= [this](){ return pixy_risa(); };
	jump_table[ "scale" ]	= [this](){ return pixy_scale(); };
	jump_table[ "twit" ]	= [this](){ return pixy_twit(); };

	ft_table[ "bmp" ]	= PIXY_FILE_TYPE_BMP;
	ft_table[ "gif" ]	= PIXY_FILE_TYPE_GIF;
	ft_table[ "gpl" ]	= PIXY_FILE_TYPE_GPL;
	ft_table[ "jpeg" ]	= PIXY_FILE_TYPE_JPEG;
	ft_table[ "none" ]	= PIXY_FILE_TYPE_NONE;
	ft_table[ "png" ]	= PIXY_FILE_TYPE_PNG;

	im_type_table[ "indexed" ]	= PIXY_PIXMAP_BPP_INDEXED;
	im_type_table[ "rgb" ]		= PIXY_PIXMAP_BPP_RGB;

	err_table[ ERROR_LOAD_FILE ]	= "Unable to load file";
	err_table[ ERROR_LIBMTPIXY ]	= "Unexpected libmtpixy failure";
	err_table[ ERROR_BAD_PALETTE ]	= "Bad palette";
}

Global::~Global ()
{
}

int Global::init ()
{
	mtPixmap * const pixmap = pixy_pixmap_new_indexed ( 100, 100 );

	if ( ! pixmap )
	{
		i_error = 1;

		return 1;
	}

	set_pixmap ( pixmap );

	return 0;			// Success
}

void Global::set_function ( char const * name )
{
	if ( ! name )
	{
		m_function = nullptr;
	}
	else
	{
		if (	name[0] == 'p' &&
			name[1] == 'i' &&
			name[2] == 'x' &&
			name[3] == 'y'
			)
		{
			name += 4;
		}

		m_function = get_function ( name );
	}

	if ( m_function )
	{
		m_function_name = name;
	}
	else
	{
		m_function_name = "";
	}
}

int Global::file_func ( char const * const filename )
{
	if ( m_function )
	{
		s_arg = filename;

		int const res = m_function ();

		if ( res > 0 )
		{
			fprintf ( stderr, "%s error: %s. arg = '%s'\n",
				m_function_name.c_str(),
				get_error_message ( res ).c_str(),
				filename );

			i_error = 1;	// Terminate ASAP
		}
	}

	return i_error;		// Keep parsing if no errors encountered
}

int Global::print_version ()
{
	if ( m_function )
	{
		printf ( "pixy%s - Part of %s\n\n", m_function_name.c_str(),
			VERSION );
	}
	else
	{
		printf ( "%s\n\n", VERSION );
	}

	return 1;			// Stop parsing
}

int Global::print_help ()
{
	print_version ();

	if ( m_function )
	{
		printf ("For further information consult the man page "
			"pixy%s(1) or the mtPixy Handbook.\n"
			"\n",
			m_function_name.c_str() );
	}
	else
	{
		printf ("For further information consult the man page "
			"%s(1) or the mtPixy Handbook.\n"
			"\n",
			BIN_NAME );
	}

	return 1;			// Stop parsing
}

int Global::argcb_com ()
{
	set_function ( s_arg );

	if ( ! m_function )
	{
		// User error, so stop program here (loudly)
		fprintf ( stderr, "Error: No such command '%s'\n", s_arg );
		i_error = 1;

		return 1;
	}

	return 0;			// Continue parsing args
}

int Global::argcb_i ()
{
	if ( ut_load_file () )
	{
		fprintf ( stderr, "Error: Unable to load file '%s'\n", s_arg );

		i_error = 1;

		return 1;		// Failure, so stop parsing
	}

	return 0;			// Continue parsing args
}

int Global::argcb_o ()
{
	int		ftype, compress = 0;


	if ( i_ftype_out == PIXY_FILE_TYPE_NONE )
	{
		ftype = i_ftype_in;
	}
	else
	{
		ftype = i_ftype_out;
	}

	switch ( ftype )
	{
	case PIXY_FILE_TYPE_PNG:
		compress = i_comp_png;
		break;

	case PIXY_FILE_TYPE_JPEG:
		compress = i_comp_jpeg;
		break;
	}

	if ( pixy_pixmap_save ( m_pixmap.get(), s_arg, ftype, compress ) )
	{
		fprintf ( stderr, "Error: Unable to save file '%s'\n", s_arg );

		i_error = 1;

		return 1;		// Failure, so stop parsing
	}

	return 0;			// Continue parsing args
}

int Global::argcb_otype ()
{
	if ( strcmp ( s_arg, "list" ) == 0 )
	{
		printf ( "Valid file types:\n\n" );

		for ( auto && it : ft_table )
		{
			std::cout << it.first << "\n";
		}
	}
	else if ( get_filetype () )
	{
		// User error, so stop program here (loudly)
		fprintf ( stderr, "Error: No such file format '%s'\n", s_arg );
		i_error = 1;

		return 1;
	}

	return 0;			// Continue parsing args
}

int Global::argcb_imtype ()
{
	if ( strcmp ( s_arg, "list" ) == 0 )
	{
		printf ( "Valid image types:\n\n" );

		for ( auto && it : im_type_table )
		{
			std::cout << it.first << "\n";
		}
	}
	else if ( get_imtype () )
	{
		// User error, so stop program here (loudly)
		fprintf ( stderr, "Error: No such image format '%s'\n", s_arg );
		i_error = 1;

		return 1;
	}

	return 0;			// Continue parsing args
}

int Global::command_line ( int argc, char const * const argv[] )
{
	// Establish what command has been used to call this program
	s_arg = strrchr ( argv[0], MTKIT_DIR_SEP );

	if ( ! s_arg )
	{
		s_arg = argv[0];
	}
	else
	{
		s_arg ++;
	}

	set_function ( s_arg );

	mtKit::Arg args ( [this]( char const * const filename )
		{
			return file_func ( filename );
		} );

	args.add ( "-help",	[this]() { return print_help(); } );
	args.add ( "-version",	[this]() { return print_version(); } );
	args.add ( "com",	s_arg, [this]() { return argcb_com(); } );
	args.add ( "comp_png",	i_comp_png );
	args.add ( "comp_jpeg",	i_comp_jpeg );
	args.add ( "dir",	s_dir );
	args.add ( "fps",	i_fps );
	args.add ( "frame0",	i_frame0 );
	args.add ( "height",	i_height );
	args.add ( "i",		s_arg, [this]() { return argcb_i(); } );
	args.add ( "imtype",	s_arg, [this]() { return argcb_imtype(); } );
	args.add ( "o",		s_arg, [this]() { return argcb_o(); } );
	args.add ( "otype",	s_arg, [this]() { return argcb_otype(); } );
	args.add ( "palette",	i_palette );
	args.add ( "prefix",	s_prefix );
	args.add ( "scale_blocky", i_scale, 1 );
	args.add ( "seconds",	i_seconds );
	args.add ( "v",		i_verbose, 1 );
	args.add ( "width",	i_width );
	args.add ( "x",		i_x );
	args.add ( "y",		i_y );

	args.parse ( argc, argv );

	return i_error;
}

void Global::set_pixmap ( mtPixmap * const pixmap )
{
	m_pixmap.reset ( pixmap );
}

int Global::ut_load_file ()
{
	if ( ! s_arg )
	{
		i_error = 1;

		return 1;		// Fail
	}

	if ( s_arg[0] == 0 )
	{
		// User sent us "" which means action the current image

		return 0;
	}

	int new_type;
	mtPixmap * const new_image = pixy_pixmap_load ( s_arg, &new_type );

	if ( ! new_image )
	{
		return 1;		// Fail
	}

	set_pixmap ( new_image );
	i_ftype_in = new_type;

	return 0;			// Success
}

FunCB Global::get_function ( std::string const & txt ) const
{
	auto const it = jump_table.find ( txt );

	if ( it == jump_table.end() )
	{
		return nullptr;
	}

	return it->second;
}

std::string Global::get_error_message ( int const err ) const
{
	auto const it = err_table.find ( err );

	if ( it == err_table.end() )
	{
		return "Unknown";
	}

	return it->second;
}

int Global::get_filetype ()
{
	auto const it = ft_table.find ( s_arg );

	if ( it != ft_table.end() )
	{
		i_ftype_out = it->second;
		return 0;
	}

	return 1;			// Not found
}

int Global::get_imtype ()
{
	auto const it = im_type_table.find ( s_arg );

	if ( it != im_type_table.end() )
	{
		i_image_type = it->second;
		return 0;
	}

	return 1;			// Not found
}

