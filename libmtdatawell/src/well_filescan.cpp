/*
	Copyright (C) 2018-2020 Mark Tyler

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



#define FILESIZE_MIN 512



void mtDW::FileScan::path_recurse ( std::string const & path )
{
	mtDW::OpenDir dir ( path );

	if ( ! dir.dp )
	{
		std::cerr << "Unable to access '" << path << "'\n";

		return;
	}

	struct dirent * ep;

	while ( ( ep = readdir (dir.dp) ) )
	{
		if (	! strcmp ( ep->d_name, "." ) ||
			! strcmp ( ep->d_name, ".." )
			)
		{
			// Quietly ignore "." and ".." directories
			continue;
		}

		std::string const tmp = path + MTKIT_DIR_SEP + ep->d_name;
		struct stat buf;

		if ( lstat ( tmp.c_str (), &buf ) )	// Get file details
		{
			std::cerr << "Unable to access '" << tmp << "'\n";
			continue;
		}

		if ( S_ISDIR ( buf.st_mode ) )
		{
			path_recurse ( tmp );
		}
		else
		{
			if ( buf.st_size < FILESIZE_MIN )
			{
				continue;
			}

			if (	! S_ISLNK ( buf.st_mode ) &&
				S_ISREG ( buf.st_mode )
				)
			{
				// This is a normal file (no symlink)

				m_rec.set_blob ( tmp.c_str (), tmp.size () );
				m_rec.insert_record ();
			}
		}
	}
}

mtDW::FileScan::FileScan (
	mtDW::FileDB		& db,
	std::string	const	& path
	)
	:
	m_file_db	( db ),
	m_rec		( db.m_db, DB_TABLE_FILES )
{
	mtKit::SqliteTransaction trans ( db.m_db );

	m_rec.add_field ( DB_FIELD_FILENAME );
	m_rec.end_field ();

	path_recurse ( mtKit::realpath ( path ) );
}

