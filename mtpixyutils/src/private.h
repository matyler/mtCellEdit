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

#include <stdlib.h>
#include <string.h>

#include <mtkit.h>
#include <mtpixy.h>



class Global;
class ImagePair;



typedef std::function<int()>	FunCB;



class ImagePair
{
public:
	int open_file (
		char const * filename,
		int & i_ftype_in,
		mtPixmap ** img
		);
		// -1 = Loaded 1st image
		// 0 = Loaded 2nd image + duplicated A into img
		// 1 = Error

	inline mtPixmap * get_pixmap_a () const { return m_pixmap_a.get (); }
	inline mtPixmap * get_pixmap_b () const	{ return m_pixmap_b.get (); }

private:
	bool both_loaded () const;	// Both images exist
	bool both_match () const;
		// Both images exist, have canvas, same type/geometry

	int prepare_output ( mtPixmap ** dup ) const;

/// ----------------------------------------------------------------------------

	mtPixy::Pixmap	m_pixmap_a;
	mtPixy::Pixmap	m_pixmap_b;
};



class Global
{
public:
	Global ();
	~Global ();

	int init ();
	int command_line ( int argc, char const * const argv[] );

private:
	enum
	{
		ERROR_LOAD_FILE		= 1,
		ERROR_LIBMTPIXY		= 2,
		ERROR_BAD_PALETTE	= 3
	};


	void set_pixmap ( mtPixmap * pixmap );

	int ut_load_file ();	// Loads image file (name in
				// global.s_arg).
		// 0 = success. Failure is not sent to stderr, but i_error is
		// set.

/*	Command functions

	Return 0 = success.
	Return > 0 = Generic error to be reported by caller (Global::file_func)
*/
	int pixy_cmp ();
	int pixy_delta ();
	int pixy_fade ();
	int pixy_ls ();
	int pixy_new ();
	int pixy_pica ();
	int pixy_resize ();
	int pixy_riba ();
	int pixy_rida ();
	int pixy_risa ();
	int pixy_scale ();
	int pixy_twit ();

	void set_function ( char const * name );

	int print_help ();
	int print_version ();

	int argcb_com ();
	int argcb_i ();
	int argcb_o ();
	int argcb_otype ();
	int argcb_imtype ();

	int file_func ( char const * filename );

	FunCB get_function ( std::string const & txt ) const;
	std::string get_error_message ( int err ) const;
	int get_filetype ();
	int get_imtype ();

/// ----------------------------------------------------------------------------

	int	i_comp_png	= 6;
	int	i_comp_jpeg	= 85;
	int	i_error		= 0;	// 0 = success 1 = error
	int	i_fps		= 60;
	int	i_frame0	= 0;
	int	i_ftype_in	= PIXY_FILE_TYPE_BMP;
	int	i_ftype_out	= PIXY_FILE_TYPE_NONE;
				// If PIXY_FILE_TYPE_NONE use i_ftype_in
	int	i_height	= 100;
	int	i_image_type	= PIXY_PIXMAP_BPP_INDEXED;
	int	i_palette	= 0;	// 0=default 1=grey 2-6=uniform
	int	i_scale		= 0;	// 0=default 1=blocky (RGB)
	int	i_seconds	= 1;
	int	i_tmp		= 0;
	int	i_verbose	= 0;	// 1 = verbose 0 = normal
	int	i_width		= 100;
	int	i_x		= 0;
	int	i_y		= 0;

	char	const	* s_dir	= "";
	char	const	* s_prefix = "";

	char	const	* s_arg	= nullptr; // Current command line argument

	mtPixy::Pixmap	m_pixmap;	// Current pixmap

	ImagePair	m_image_pair;	// 2 working images

	FunCB		m_function = nullptr;
	std::string	m_function_name;

	std::map< std::string, FunCB > jump_table;
	std::map< std::string, int > ft_table;
	std::map< std::string, int > im_type_table;
	std::map< int, std::string > err_table;

	MTKIT_RULE_OF_FIVE( Global )
};



int * create_cube_analysis (
	int			i,	// 8=full cube 1=corners of cube
	unsigned char	const *	rgb,
	int			w,
	int			h
	);

