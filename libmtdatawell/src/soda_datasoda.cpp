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

#include "soda.h"



mtDW::Soda::Soda ( char const * const path )
	:
	m_op		( new Soda::Op ( path ) )
{
}

mtDW::Soda::~Soda ()
{
}

int mtDW::Soda::decode (
	Butt		* const	butt,
	char	const * const	input,
	char	const * const	output
	)
{
	return mtDW::Soda::Op::decode ( butt, input, output );
}

int mtDW::Soda::multi_decode (
	Butt		* const	butt,
	char	const * const	input,
	char	const * const	output
	)
{
	return mtDW::Soda::Op::multi_decode ( butt, input, output );
}

int mtDW::Soda::encode (
	Butt		* const	butt,
	char	const * const	input,
	char	const * const	output
	) const
{
	return m_op->encode ( butt, input, output );
}

int mtDW::Soda::multi_encode (
	Butt			* const	butt,
	char		const * const	input,
	char		const * const	output,
	char	const * const * const	otp_names
	) const
{
	return m_op->multi_encode ( butt, input, output, otp_names );
}

void mtDW::Soda::set_mode ( int m ) const
{
	return m_op->set_mode ( m );
}

int mtDW::Soda::get_mode () const
{
	return m_op->get_mode ();
}

mtDW::SodaTransaction::SodaTransaction ( Soda & soda )
	:
	m_op	( new SodaTransaction::Op ( soda.m_op->m_db ) )
{
}

mtDW::SodaTransaction::~SodaTransaction ()
{
}

