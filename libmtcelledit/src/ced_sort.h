/*
	Copyright (C) 2009-2021 Mark Tyler

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

#include "ced.h"



typedef int (* strCMPfunc) (
	char	const *	s1,
	char	const *	s2
	);
	// -1 = s1 < s2
	//  0 = s1 == s2
	//  1 = s1 > s2



class Sort_State
{
public:
	Sort_State (
		CedSheet	* const	sheet,
		int		const	mode,
		int	const * const	mode_list
		)
		:
		m_sheet		( sheet ),
		m_mode_list	( mode_list ),
		m_order		( mode & CED_SORT_MODE_DESCENDING ? -1 : 1 ),
		m_cmp_func ( mode & CED_SORT_MODE_CASE ? strcmp : strcasecmp )
	{
	}

	inline CedSheet * sheet () const	{ return m_sheet; }
	inline int cmp_cstring ( char const * s1, char const * s2 ) const
	{
		return m_cmp_func ( s1, s2 );
	}

	int cmp_cells ( CedCell const * c1, CedCell const * c2 ) const;

	void set_mode_from_list ( int i );

private:
	CedSheet	* const	m_sheet;
	int	const * const	m_mode_list;
	int			m_order;	// -1 = descending 1 = ascending
	strCMPfunc		m_cmp_func;	// strcmp or strcasecmp
};

