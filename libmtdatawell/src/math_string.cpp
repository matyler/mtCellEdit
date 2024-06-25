/*
	Copyright (C) 2022-2024 Mark Tyler

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

#include "mtdatawell_math.h"



namespace {

static std::string snip_number (
	std::string	const	& numstr,
	size_t		const	maxlen
	)
{
	size_t const endlen = MIN(
				MAX( mtDW::Number::STRING_SNIP_MIN, maxlen)
				, mtDW::Number::STRING_SNIP_MAX
				);

	if ( numstr.size() <= (endlen * 2) )
	{
		return numstr;
	}

	std::string res ( numstr, 0, endlen );

	res += "~~~~";

	res.append ( numstr, numstr.size() - endlen, endlen );

	return res;
}

} // namespace



/// DOUBLE ---------------------------------------------------------------------



std::string mtDW::Double::to_string () const
{
	char txt[128];

	// Note: last 2 digits after decimal point contain errors so ignore them
	snprintf ( txt, sizeof(txt), "%.15g", m_num );

	return std::string ( txt );
}

std::string mtDW::Double::to_string_snip (
	size_t	const	maxlen
	) const
{
	return snip_number ( to_string (), maxlen );
}

int mtDW::Double::to_file ( FILE * const fp ) const
{
	std::string const txt = to_string ();

	if ( txt.size() != fwrite ( txt.c_str(), 1, txt.size(), fp ) )
	{
		return 1;
	}

	return 0;
}

int mtDW::Double::to_filename ( char const * const filename ) const
{
	if ( ! filename )
	{
		return 1;
	}

	mtKit::ByteFileWrite file;

	if ( file.open ( filename ) )
	{
		return 1;
	}

	return to_file ( file.get_fp() );
}



/// FLOAT ----------------------------------------------------------------------



std::string mtDW::Float::to_string () const
{
	size_t const digits = get_str_ndigits ();
	char prec[32];

	// Note: last 2 digits after decimal point contain errors so ignore them
	snprintf ( prec, sizeof(prec), "%%.%zuRg", digits > 15 ?
		digits - 2 :	// Error digits
		digits );

	FloatString	sbuf;
	std::string	st;

	if ( 0 <= mpfr_asprintf ( sbuf.pptr(), prec, m_num ) )
	{
		st = sbuf.ptr();
	}

	return st;
}

mpfr_prec_t mtDW::Float::get_str_precision (
	char	const * const	str,
	size_t			length
	)
{
	if ( ! str )
	{
		return DOUBLE_PRECISION;
	}

	if ( length == 0 )
	{
		length = strlen ( str );
	}

	if ( str[0] == '-' )
	{
		length--;
	}

	if ( length < 18 )
	{
		return DOUBLE_PRECISION;
	}

	static const double ln = ::log(10) / ::log(2);
	double const bits = 10.0 + ::ceil ( (double)(length) * ln );
	// NOTE: The extra bit is usually too much, but for some cases
	// it is required.

	return MAX ( DOUBLE_PRECISION, MIN( (mpfr_prec_t)bits, MPFR_PREC_MAX ));
}

std::string mtDW::Float::to_string_snip (
	size_t	const	maxlen
	) const
{
	return snip_number ( to_string (), maxlen );
}

int mtDW::Float::to_file ( FILE * const fp ) const
{
	std::string const txt = to_string ();

	if ( txt.size() != fwrite ( txt.c_str(), 1, txt.size(), fp ) )
	{
		return 1;
	}

/* NOTE: mpfr_out_str doesn't simplify the output so we don't use it
	if ( 0 == mpfr_out_str ( fp, 10, 0, m_num, m_rnd ) )
	{
		return 1;
	}
*/

	return 0;
}

int mtDW::Float::to_filename ( char const * const filename ) const
{
	if ( ! filename )
	{
		return 1;
	}

	mtKit::ByteFileWrite file;

	if ( file.open ( filename ) )
	{
		return 1;
	}

	return to_file ( file.get_fp() );
}

size_t mtDW::Float::get_str_ndigits () const
{
	// Simple mpfr_get_str_ndigits() replacement for MPFR 3.1
//	return mpfr_get_str_ndigits ( b, mpfr_get_prec (m_num) );

	double const bits = (double)mpfr_get_prec ( m_num );

	return (size_t)(1 + ::ceil ( bits * ::log(2.0) / ::log(10) ));
}



/// INTEGER --------------------------------------------------------------------



std::string mtDW::Integer::to_string ( int const base ) const
{
	std::string st;
	IntegerString pbuf ( mpz_get_str ( nullptr, base, m_num ) );

	if ( pbuf.ptr() )
	{
		st = pbuf.ptr();
	}

	return st;
}

std::string mtDW::Integer::to_string_snip (
	size_t	const	maxlen,
	int	const	base
	) const
{
	return snip_number ( to_string ( base ), maxlen );
}

int mtDW::Integer::to_file ( FILE * const fp, int const base ) const
{
	if ( 0 == mpz_out_str ( fp, base, m_num ) )
	{
		return 1;
	}

	return 0;
}

int mtDW::Integer::to_filename (
	char	const * const	filename,
	int		const	base
	) const
{
	if ( ! filename )
	{
		return 1;
	}

	mtKit::ByteFileWrite file;

	if ( file.open ( filename ) )
	{
		return 1;
	}

	return to_file ( file.get_fp(), base );
}



/// RATIONAL -------------------------------------------------------------------



#define DIVISOR_SEPARATOR	" / "
#define DIVISOR_SEPARATOR_LEN	3



std::string mtDW::Rational::to_string () const
{
	std::string st;

	IntegerString pbuf_n ( mpz_get_str ( nullptr, 10,
		mpq_numref (m_num) ) );
	IntegerString pbuf_d ( mpz_get_str ( nullptr, 10,
		mpq_denref (m_num) ) );

	if ( pbuf_n.ptr() && pbuf_d.ptr() )
	{
		st = pbuf_n.ptr();
		st += DIVISOR_SEPARATOR;
		st += pbuf_d.ptr();
	}

	return st;
}

std::string mtDW::Rational::to_string_snip (
	size_t	const	maxlen
	) const
{
	std::string st;

	IntegerString pbuf_n ( mpz_get_str ( nullptr, 10,
		mpq_numref (m_num) ) );
	IntegerString pbuf_d ( mpz_get_str ( nullptr, 10,
		mpq_denref (m_num) ) );

	if ( pbuf_n.ptr() && pbuf_d.ptr() )
	{
		st = snip_number ( pbuf_n.ptr(), maxlen );
		st += DIVISOR_SEPARATOR;
		st += snip_number ( pbuf_d.ptr(), maxlen );
	}

	return st;
}

int mtDW::Rational::to_file ( FILE * const fp ) const
{
	if (	0 == mpz_out_str ( fp, 10, mpq_numref (m_num) )
		|| 0 == fprintf ( fp, DIVISOR_SEPARATOR )
		|| 0 == mpz_out_str ( fp, 10, mpq_denref (m_num) )
		)
	{
		return 1;
	}

	return 0;
}

int mtDW::Rational::to_filename ( char const * const filename ) const
{
	if ( ! filename )
	{
		return 1;
	}

	mtKit::ByteFileWrite file;

	if ( file.open ( filename ) )
	{
		return 1;
	}

	return to_file ( file.get_fp() );
}

size_t mtDW::Rational::get_str_ndigits () const
{
	return (mpz_sizeinbase ( mpq_numref (m_num), 10 ) +
		mpz_sizeinbase ( mpq_denref (m_num), 10 ) +
		DIVISOR_SEPARATOR_LEN
		);
}

