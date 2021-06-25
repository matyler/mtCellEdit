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

#include "core.h"



mtDW::GlyphNode::GlyphNode ( std::string const &nodes )
	:
	m_root		( nodes.c_str() [0] ),
	m_bit_total	( 0 )
{
	unsigned char const * const src = (unsigned char const *)nodes.c_str ();
	size_t const gtot = (size_t)mtkit_utf8_len ( src, 0 );

	for ( size_t g = 0, i = 0; g < gtot; g++ )
	{
		int const len = mtkit_utf8_offset ( src + i, 1 );
		if ( len < 1 )
		{
			// Should never happen
			std::cerr << "Illegal UTF-8 glyph.\n";
			break;
		}

		if ( add ( std::string ( (char const *)(src + i), (size_t)len)))
		{
			break;
		}

		i += (size_t)len;
	}

	recalc_bit_total ();

#if 1==0
	std::cout << "GlyphNode: '" << nodes << "'"
		<< " len=" << gtot
		<< " tot=" << m_nodes.size ()
		<< " bits=" << m_bit_total << "\n";
#endif
}


int mtDW::GlyphNode::get_index (
	std::string	const	& node,
	int		* const	bit_total,
	int		* const	node_count
	) const
{
	std::map<std::string, int>::const_iterator const it =
		m_nodes.find ( node );

	if ( m_nodes.end () == it )
	{
//		std::cerr << "GlyphNode::get_index node not in map - '"
//			<< node << "'\n";

		return -1;
	}

	if ( bit_total )
	{
		*bit_total = m_bit_total;
	}

	if ( node_count )
	{
		*node_count = (int)m_index.size ();
	}

	return it->second;
}

int mtDW::GlyphNode::get_node (
	int	const	index,
	std::string	&node
	) const
{
	std::map<int, std::string>::const_iterator const it =
		m_index.find ( index );

	if ( m_index.end () == it )
	{
//		std::cerr << "GlyphNode::get_node index not in map - '"
//			<< index << "'\n";

		return 1;
	}

	node = it->second;

	return 0;
}

int mtDW::GlyphNode::add ( std::string const &node )
{
	size_t const size = m_nodes.size ();

/*
	if ( size >= NODE_MAX )
	{
		std::cerr << "GlyphNode::add NODE_MAX reached."
			<< node << "\n";

		return 1;
	}
*/

	try
	{
		m_nodes.insert ( std::pair<std::string, int>(node, (int)size) );
		m_index.insert ( std::pair<int, std::string>((int)size, node) );
	}
	catch ( ... )
	{
		std::cerr << "GlyphNode::add unable to add node."
			<< node << "\n";

		return 1;
	}

	return 0;
}

void mtDW::GlyphNode::recalc_bit_total ()
{
	size_t const gtot = m_nodes.size ();

	m_bit_total =	(gtot < 2)	? 0 :
			(gtot < 4)	? 1 :
			(gtot < 8)	? 2 :
			(gtot < 16)	? 3 :
			(gtot < 32)	? 4 :
			(gtot < 64)	? 5 : 6;
}

