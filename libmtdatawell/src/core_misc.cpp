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

#include "well.h"



std::string mtDW::prepare_path ( char const * const path )
{
	std::string real_path;

	if ( path )
	{
		real_path += path;

		mtkit_mkdir ( real_path.c_str() );
	}
	else
	{
		real_path += mtkit_file_home ();
		real_path += MTKIT_DIR_SEP;
		real_path += ".config";

		mtkit_mkdir ( real_path.c_str() );

		real_path += MTKIT_DIR_SEP;
		real_path += APP_NAME;

		mtkit_mkdir ( real_path.c_str() );
	}

	return mtKit::realpath ( real_path ) + MTKIT_DIR_SEP;
}

void mtDW::get_temp_filename (
	std::string		& filename,
	char	const * const	prefix
	)
{
	char buf[16];

	for ( int i = 1; i < 100; i++ )
	{
		snprintf ( buf, sizeof(buf), "_%02i", i );

		if ( prefix )
		{
			filename = prefix;
		}
		else
		{
			filename.clear ();
		}

		filename += "_x9Zwq";
		filename += buf;

		if ( ! mtkit_file_readable ( filename.c_str () ) )
		{
			break;
		}
	}
}

mtDW::FilenameSwap::FilenameSwap ( char const * const output )
	:
	m_res		( 0 ),
	m_prefix	( output )
{
	get_temp_filename ( m_tmp, output );

	// Reserve this file on the filesystem so further calls
	// don't use this same filename.
	mtkit_file_save ( m_tmp.c_str (), m_tmp.c_str (), 0, 0 );

	f1 = output;
	f2 = m_tmp.c_str ();
}

mtDW::FilenameSwap::~FilenameSwap ()
{
	if ( 0 == m_res )
	{
		// On success remove/rename as required

		if ( f1 == m_prefix )
		{
			rename ( f2, f1 );
		}
		else
		{
			remove ( f1 );
		}

		return;
	}

	// On failure remove both files
	remove ( f1 );
	remove ( f2 );
}

void mtDW::FilenameSwap::swap ()
{
	char const * const f0 = f1;
	f1 = f2;
	f2 = f0;
}

int mtDW::Database::open ( char const * const path )
{
	try
	{
		mtKit::unique_ptr<mtDW::Well> well ( new mtDW::Well ( path ) );
		mtKit::unique_ptr<mtDW::Butt> butt ( new mtDW::Butt ( path ) );
		mtKit::unique_ptr<mtDW::Soda> soda ( new mtDW::Soda ( path ) );
		mtKit::unique_ptr<mtDW::Tap> tap ( new mtDW::Tap () );

		m_tap.reset ( tap.release () );
		m_soda.reset ( soda.release () );
		m_butt.reset ( butt.release () );
		m_well.reset ( well.release () );

		m_path = mtDW::prepare_path ( path );
	}
	catch ( ... )
	{
		return 1;
	}

	return 0;
}

int mtDW::remove_dir ( std::string const &path )
{
	mtDW::OpenDir dir ( path );

	if ( ! dir.dp )
	{
		std::cerr << "Unable to opendir '" << path << "'\n";
		return 1;
	}

	struct dirent * ep;

	while ( ( ep = readdir ( dir.dp ) ) )
	{
		std::string const src = path + ep->d_name;
		struct stat buf;

		if ( lstat ( src.c_str (), &buf ) )
		{
			continue;
		}

		if (	S_ISDIR ( buf.st_mode )		||
			S_ISLNK ( buf.st_mode )		||
			! S_ISREG ( buf.st_mode )
			)
		{
			continue;
		}

		if ( remove ( src.c_str () ) )
		{
			std::cerr << "Unable to remove '" << src << "'\n";
			return 1;
		}
	}

	if ( remove ( path.c_str () ) )
	{
		std::cerr << "Unable to remove '" << path << "'\n";
		return 1;
	}

	return 0;
}

void mtDW::ByteBuf::load_whole ( std::string const &filename )
{
	int64_t const filesize = mtkit_file_size ( filename.c_str () );

	if ( filesize < 0 || this->allocate ( (size_t)filesize ) )
	{
		this->allocate ( 0 );

		return;
	}

	load_fill ( filename );
}

void mtDW::ByteBuf::load_fill ( std::string const &filename )
{
	mtKit::ByteFileRead file;

	file.open ( filename.c_str (), 0 );
	m_tot = file.read ( m_buf, m_size );
	m_pos = 0;
}

int mtDW::ByteBuf::save ( std::string const &filename ) const
{
	mtKit::ByteFileWrite file;

	if (	file.open ( filename.c_str () )		||
		file.write ( m_buf, m_size )
		)
	{
		return 1;
	}

	return 0;
}

