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

#include "butt.h"



mtDW::Butt::Butt (
	mtKit::Random	&	random,
	char	const * const	path
	)
	:
	op		( new ButtOp ( random, path ) )
{
}

mtDW::Butt::~Butt ()
{
	delete op;
}

int mtDW::Butt::add_buckets (
	Well	* const	well,
	int	const	tot
	) const
{
	return op->add_buckets ( well, tot );
}

int mtDW::Butt::get_bucket_total () const
{
	return op->get_write_next ();
}

int mtDW::Butt::get_bucket_used () const
{
	return op->get_otp_bucket ();
}

int mtDW::Butt::get_bucket_position () const
{
	return op->get_otp_position ();
}

std::string const & mtDW::Butt::get_path () const
{
	return op->get_path ();
}

std::string const & mtDW::Butt::get_name () const
{
	return op->get_name ();
}

int mtDW::Butt::add_name ( std::string const & name ) const
{
	return op->add_name ( name );
}

int mtDW::Butt::set_name ( std::string const & name ) const
{
	return op->set_name ( name );
}

static int name_cmp (
	void	const * const	k1,
	void	const * const	k2
	)
{
	return strcmp ( (char const *)k1, (char const *)k2 );
}

static void name_del (
	mtTreeNode	* const	node
	)
{
	free ( node->key );
	node->key = NULL;
}

mtTree * mtDW::Butt::get_name_list () const
{
	std::string const & root = op->get_butt_root ();
	mtDW::OpenDir dir ( root );

	if ( ! dir.dp )
	{
		return NULL;
	}

	mtTree * const tree = mtkit_tree_new ( name_cmp, name_del );
	if ( ! tree )
	{
		return NULL;
	}

	struct dirent * ep;

	while ( (ep = readdir(dir.dp)) )
	{
		if (	! strcmp ( ep->d_name, "." ) ||
			! strcmp ( ep->d_name, ".." )
			)
		{
			// Quietly ignore "." and ".." directories
			continue;
		}

		std::string const tmp = root + ep->d_name;
		struct stat buf;

		if ( lstat ( tmp.c_str (), &buf ) )	// Get file details
		{
			// Unable to access
			continue;
		}

		if ( S_ISDIR ( buf.st_mode ) )
		{
			char * st = strdup ( ep->d_name );
			if ( ! st )
			{
				continue;
			}

			if ( 0 == mtkit_tree_node_add ( tree, st, NULL ) )
			{
				free ( st );
				st = NULL;
			}
		}
	}

	return tree;
}

int mtDW::Butt::validate_butt_name ( std::string const & name )
{
	if ( name.size () < 1 )
	{
		std::cerr << "Name '" << name << "' has too few characters.\n";
		return 1;
	}

	if ( name.size () > 16 )
	{
		std::cerr << "Name '" << name << "' has too many characters.\n";
		return 1;
	}

	char const * const str = name.c_str ();

	for ( size_t i = 0; i < name.size (); i++ )
	{
		if (	! (str[i] >= '0' && str[i] <= '9')
			&& ! (str[i] >= 'a' && str[i] <= 'z')
			&& ! (str[i] >= 'A' && str[i] <= 'Z')
			&& str[i] != '_'
			&& str[i] != '.'
			)
		{
			std::cerr << "Name '" << name
				<< "' contains an illegal character.\n";
			return 1;
		}
	}

	return 0;
}

int mtDW::Butt::otp_get_int ( int & res ) const
{
	return op->otp_get_int ( res );
}

int mtDW::Butt::otp_get_int ( int modulo, int & res ) const
{
	return op->otp_get_int ( modulo, res );
}

int mtDW::Butt::otp_get_data ( uint8_t * buf, size_t buflen ) const
{
	return op->otp_get_data ( buf, buflen );
}

int mtDW::Butt::read_set_butt (
	std::string	const & name,
	int		const	bucket,
	int		const	pos
	) const
{
	return op->read_set_butt ( name, bucket, pos );
}

int mtDW::Butt::read_get_data (
	uint8_t		* const	buf,
	size_t		const	buflen
	) const
{
	return op->read_get_data ( buf, buflen );
}

