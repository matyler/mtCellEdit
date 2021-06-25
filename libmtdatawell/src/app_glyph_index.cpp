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



void mtDW::GlyphIndex::add_root_nodes ( std::string const &nodes )
{
	unsigned char const * const src = (unsigned char const *)nodes.c_str ();

	if ( 1 != mtkit_utf8_string_legal ( src, 0 ) )
	{
		std::cerr << "GlyphIndex::add_root_nodes not UTF-8 - '"
			<< nodes << "'\n";
		throw 123;
	}

	int const len = mtkit_utf8_len ( src, 0 );

/*
	if (	len != 2	&&
		len != 4	&&
		len != 8	&&
		len != 16	&&
		len != 32	&&
		len != 64
		)
	{
		std::cerr << "GlyphIndex::add bad UTF-8 glyph length - "
			<< len << " '" << nodes << "'\n";
		throw 123;
	}
*/

	char const root = nodes[0];

	if ( root <= 32 || root >= 127 )
	{
		std::cerr << "GlyphIndex::add_root_nodes bad root - '"
			<< nodes << "'\n";
		throw 123;
	}

	if ( m_root.end () != m_root.find ( root ) )
	{
		std::cerr << "GlyphIndex::add_root_nodes "
			<< "root already exists - '"
			<< nodes << "'\n";
		throw 123;
	}

	std::pair<std::map<char, GlyphNode>::const_iterator,bool> const it =
		m_root.insert ( std::pair<char, GlyphNode>( root,
			GlyphNode ( nodes ) ) );

/*
	if ( it.second == false )
	{
		// Already exists
	}
*/

	GlyphNode const * const new_node = &it.first->second;

	for ( int i = 0, j = 0; j < len; j++ )
	{
		int const glyph_len = mtkit_utf8_offset ( src + i, 1 );

		if ( glyph_len < 1 )
		{
			std::cerr << "GlyphIndex::add_root_nodes "
				<< "bad glyph at byte="
				<< i << " glyph=" << j << " '" << nodes << "\n";
			throw 123;
		}

		std::string st ( (char const *)(src + i), (size_t)glyph_len );

		if ( m_nodes.end () != m_nodes.find ( st ) )
		{
			std::cerr << "GlyphIndex::add_root_nodes "
				<< "node already exists - '"
				<< st << "' "
				<< nodes << "'\n";
			throw 123;
		}

		// Add reference to the new Homoglyph to the m_nodes tree
		m_nodes.insert ( std::pair<std::string, GlyphNode const *>(
			st, new_node ) );

		i += glyph_len;
	}
}

int mtDW::GlyphIndex::get_root_bits (
	std::string	const	&node,
	char		* const	root,
	int		* const	bit_tot,
	int		* const	node_count
	) const
{
	if ( node.length () < 1 )
	{
		return 1;
	}

	std::map<std::string, GlyphNode const *>::const_iterator const it =
		m_nodes.find ( node );

	if ( m_nodes.end () == it )
	{
//		std::cerr << "GlyphIndex::get_root node not in map - '"
//			<< node << "'\n";

		return 1;
	}

	if ( root )
	{
		*root = it->second->get_root ();
	}

	if ( bit_tot )
	{
		it->second->get_index ( node, bit_tot, node_count );
	}

	return 0;
}

int mtDW::GlyphIndex::get_index (
	std::string	const	&node,
	int		* const	bit_total,
	int		* const	node_count
	) const
{
	if ( node.length () < 1 )
	{
		return 1;
	}

	std::map<std::string, GlyphNode const *>::const_iterator const it =
		m_nodes.find ( node );

	if ( m_nodes.end () == it )
	{
//		std::cerr << "GlyphIndex::get_index node not in map - '"
//			<< node << "'\n";

		return 1;
	}

	return it->second->get_index ( node, bit_total, node_count );
}

int mtDW::GlyphIndex::get_node (
	char	const	root,
	int	const	index,
	std::string	&node
	) const
{
	if ( root < 32 || index < 0 )
	{
		return 1;
	}

	std::map<char, GlyphNode>::const_iterator const it =
		m_root.find ( root );

	if ( m_root.end () == it )
	{
//		std::cerr << "GlyphIndex::get_node root not in map - '"
//			<< root << "'\n";

		return 1;
	}

	return it->second.get_node ( index, node );
}

int mtDW::GlyphIndex::file_clean (
	char	const * const	input,
	char	const * const	output,
	std::string		&info
	) const
{
	info.clear ();

	if ( ! input || ! output )
	{
		info = "Bad argument";
		return 1;
	}

	FileOps fops ( this );

	if ( fops.load_input_file ( input ) )
	{
		info = "Unable to load file";
		return 1;
	}

	int const glyph_len = fops.get_utf8_len ();

	if ( glyph_len < 1 )
	{
		info += "Not UTF-8 input file";
		return 1;
	}

	if ( fops.open_output_file ( output ) )
	{
		info = "Unable to open output file";
		return 1;
	}

	if ( fops.cleanse_file () )
	{
		info = "Unable to cleanse file";
		return 1;
	}

	return 0;
}

int mtDW::GlyphIndex::utf8_clean (
	std::string	const	&input,
	std::string		&info,
	std::string		&output
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

	if ( fops.open_output_mem () )
	{
		info = "Unable to open output mem";
		return 1;
	}

	if ( fops.cleanse_file ()
		|| fops.get_output_mem_utf8 ( output )
		)
	{
		info = "Unable to cleanse the input to create the output";
		return 1;
	}

	return 0;
}

