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

#ifndef MTDATAWELL_MATH_H_
#define MTDATAWELL_MATH_H_


/*
	PURPOSE

This is a very thin layer around mpfr/gmp to make it useful for C++ apps.

*/


#include <stdio.h>	// Needed to get MPFR (FILE *) funcs to work
#include <math.h>
#include <mpfr.h>

#include <mtdatawell.h>



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifdef __cplusplus
extern "C" {
#endif

// C API



#ifdef __cplusplus
}

// C++ API



namespace mtDW
{

class Double;
class DoubleLexer;
class DoubleParser;
class Float;
class FloatLexer;
class FloatParser;
class FloatString;
class Integer;
class IntegerLexer;
class IntegerMemory;
class IntegerParser;
class IntegerString;
class Lexer;
class MathState;
class Rational;
class RationalLexer;
class RationalParser;

template< typename Tnum, typename Tlexer >
class Parser;



namespace Number
{

enum
{
	STRING_SNIP_MIN		= 100,
	STRING_SNIP_MAX		= 100000,

	// Text input limit for parsing.
	EVAL_TEXT_MAX_SIZE	= 1000000,	// 1MB

	// For rational parsing via scientific notation.
	RATIONAL_EXP_MAX	= 1000000000
};


}	// namespace Number



class MathState
{
public:
	~MathState ()
	{
		mpfr_free_cache();
	}
};



class IntegerString	// Wrapper for mpz_get_str(), see Integer::to_string()
{
public:
	explicit IntegerString ( char * const p )
		:
		m_ptr	( p )
	{
		// We get this func now as the calling app may change this later
		// before the resource is free'd
		mp_get_memory_functions ( nullptr, nullptr, &m_free_func );
	}

	~IntegerString ()
	{
		if ( m_ptr )
		{
			m_free_func ( m_ptr, 1 + strlen(m_ptr) );
			m_ptr = nullptr;
		}
	}

	char const * ptr() const { return m_ptr; }

private:
	char	* m_ptr;
	void	(* m_free_func)(void *, size_t);

	MTKIT_RULE_OF_FIVE( IntegerString )
};



class FloatString	// Wrapper for mpfr_asprintf(), see Float::to_string()
{
public:
	explicit FloatString ()
	{
	}

	~FloatString ()
	{
		if ( m_ptr )
		{
			mpfr_free_str ( m_ptr );
			m_ptr = nullptr;
		}
	}

	char * ptr()	{ return m_ptr; }
	char ** pptr()	{ return &m_ptr; }

private:
	char	* m_ptr = nullptr;

	MTKIT_RULE_OF_FIVE( FloatString )
};



class Integer
{
private:
	void init_str ( char const * const text, int const base )
	{
		if ( text )
		{
			if ( mpz_init_set_str ( m_num, text, base ) )
			{
				std::cerr << "Invalid Integer: '" << text
					<< "'\n";
				throw 123;
			}
		}
		else
		{
			mpz_init ( m_num );
		}
	}
public:
	Integer ()
	{
		mpz_init ( m_num );
	}

	Integer ( int const num )
	{
		mpz_init_set_si ( m_num, num );
	}

	Integer ( signed long int const num )
	{
		mpz_init_set_si ( m_num, num );
	}

	Integer ( Integer const & i )
	{
		mpz_init_set ( m_num, i.m_num );
	}

	Integer ( char const * const text, int const base = 10 )
	{
		init_str ( text, base );
	}

	Integer ( std::string const & text, int const base = 10 )
	{
		init_str ( text.c_str(), base );
	}

	~Integer()
	{
		mpz_clear ( m_num );
	}

	// Move constructor definition needed for creating C arrays
	Integer ( Integer && other )
	{
		mpz_init ( m_num );
		mpz_swap ( m_num, other.m_num );
	}

	// Copy assignment
	Integer & operator = ( Integer const & other )
	{
		if ( this != &other )
		{
			mpz_set ( m_num, other.m_num );
		}

		return *this;
	}

	// Move assignment
	Integer & operator = ( Integer && other )
	{
		if ( this != &other )
		{
			mpz_swap ( m_num, other.m_num );
		}

		return *this;
	}

	// After initialization, these are the most efficient ways to set values

	void set_number ( int const num )
	{
		mpz_set_si ( m_num, num );
	}

	void set_number ( signed long int const num )
	{
		mpz_set_si ( m_num, num );
	}

	void set_number ( Integer const & num )
	{
		mpz_set ( m_num, num.m_num );
	}

	int set_number ( char const * const text, int const base = 10 )
	{
		return text ? mpz_set_str ( m_num, text, base ) : 1;
	}

	int set_number ( std::string const & text, int const base = 10 )
	{
		return mpz_set_str ( m_num, text.c_str(), base );
	}

///	Logical comparisons ----------------------------------------------------

	int cmp ( Integer const & rhs ) const
	{
		return mpz_cmp ( m_num, rhs.m_num );
	}

	bool operator <  ( Integer const & rhs ) const { return cmp(rhs) <  0; }
	bool operator >  ( Integer const & rhs ) const { return cmp(rhs) >  0; }
	bool operator <= ( Integer const & rhs ) const { return cmp(rhs) <= 0; }
	bool operator >= ( Integer const & rhs ) const { return cmp(rhs) >= 0; }
	bool operator == ( Integer const & rhs ) const { return cmp(rhs) == 0; }
	bool operator != ( Integer const & rhs ) const { return cmp(rhs) != 0; }

	void set_bound ( Integer const & min, Integer const & max )
	{
		if ( *this < min )
		{
			this->set_number ( min );
			return;
		}

		if ( *this > max )
		{
			this->set_number ( max );
			return;
		}
	}

///	Arithmetic -------------------------------------------------------------

	Integer & operator += ( Integer const & rhs )
	{
		mpz_add ( m_num, m_num, rhs.m_num );
		return *this;
	}

	Integer & operator += ( unsigned long int const & rhs )
	{
		mpz_add_ui ( m_num, m_num, rhs );
		return *this;
	}

	Integer & operator -= ( Integer const & rhs )
	{
		mpz_sub ( m_num, m_num, rhs.m_num );
		return *this;
	}

	Integer & operator -= ( unsigned long int const & rhs )
	{
		mpz_sub_ui ( m_num, m_num, rhs );
		return *this;
	}

	Integer & operator *= ( Integer const & rhs )
	{
		mpz_mul ( m_num, m_num, rhs.m_num );
		return *this;
	}

	Integer & operator *= ( unsigned long int const & rhs )
	{
		mpz_mul_ui ( m_num, m_num, rhs );
		return *this;
	}

	Integer & operator *= ( long int const & rhs )
	{
		mpz_mul_si ( m_num, m_num, rhs );
		return *this;
	}

	Integer & operator /= ( Integer const & rhs )
	{
		if ( mpz_cmp_ui ( rhs.m_num, 0 ) == 0 )
		{
			throw 123;
		}

		mpz_tdiv_q ( m_num, m_num, rhs.m_num );
		return *this;
	}

	Integer & operator /= ( unsigned long int const & rhs )
	{
		if ( rhs == 0 )
		{
			throw 123;
		}

		mpz_tdiv_q_ui ( m_num, m_num, rhs );
		return *this;
	}

	// NOTE: These friend operators are somewhat inefficient, so not worth
	// using inside inner loops, but useful for (a+b).to_string().

	friend Integer operator + ( Integer lhs, Integer const & rhs )
	{
		lhs += rhs;
		return lhs;
	}

	friend Integer operator - ( Integer lhs, Integer const & rhs )
	{
		lhs -= rhs;
		return lhs;
	}

	friend Integer operator * ( Integer lhs, Integer const & rhs )
	{
		lhs *= rhs;
		return lhs;
	}

	friend Integer operator / ( Integer lhs, Integer const & rhs )
	{
		lhs /= rhs;
		return lhs;
	}

///	Convenience functions --------------------------------------------------

	void abs ()
	{
		mpz_abs ( m_num, m_num );
	}

	void bit_and ( Integer const & a, Integer const & b )
	{
		mpz_and ( m_num, a.m_num, b.m_num );
	}

	void bit_not ()
	{
		mpz_com ( m_num, m_num );
	}

	void bit_or ( Integer const & a, Integer const & b )
	{
		mpz_ior ( m_num, a.m_num, b.m_num );
	}

	void bit_xor ( Integer const & a, Integer const & b )
	{
		mpz_xor ( m_num, a.m_num, b.m_num );
	}

	void factorial ( Integer const & a )
	{
		mpz_fac_ui ( m_num, a.get_number_uli () );
	}

	// Greatest Common Divisor
	void gcd ( Integer const & a, Integer const & b )
	{
		mpz_gcd ( m_num, a.m_num, b.m_num );
	}

	// Lowest Common Multiple
	void lcm ( Integer const & a, Integer const & b )
	{
		mpz_lcm ( m_num, a.m_num, b.m_num );
	}

	void max ( Integer const & a, Integer const & b )
	{
		if ( a < b )
		{
			this->set_number ( b );
		}
		else
		{
			this->set_number ( a );
		}
	}

	void min ( Integer const & a, Integer const & b )
	{
		if ( a > b )
		{
			this->set_number ( b );
		}
		else
		{
			this->set_number ( a );
		}
	}

	void mod ( Integer const & a, Integer const & b )
	{
		mpz_mod ( m_num, a.m_num, b.m_num );
	}

	void pow ( Integer const & a, Integer const & b )
	{
		if ( ! mpz_fits_ulong_p ( b.m_num ) )
		{
			throw 123;
		}

		mpz_pow_ui ( m_num, a.m_num, mpz_get_ui ( b.m_num ) );
	}

	void negate ()
	{
		mpz_neg ( m_num, m_num );
	}

	int sign () const
	{
		return mpz_sgn ( m_num );
	}
	// -1 : num < 0
	//  0 : num == 0
	// +1 : num > 0

	void swap ( Integer & rhs )
	{
		mpz_swap ( m_num, rhs.m_num );
	}

	size_t get_str_ndigits ( int const base = 10 ) const
	{
		return mpz_sizeinbase ( m_num, base );
	}

///	I/O funcs --------------------------------------------------------------

	std::string to_string ( int const base = 10 ) const;
	std::string to_string_snip ( size_t maxlen, int base = 10 ) const;
	int to_file ( FILE * const fp, int const base = 10 ) const;
	int to_filename ( char const * filename, int const base = 10 ) const;

	int to_stdout ( int base = 10 ) const
	{
		return to_file ( stdout, base );
	}

///	Accessor ---------------------------------------------------------------

	mpz_t & get_num ()		{ return m_num; }
	mpz_t const & get_num () const	{ return m_num; }

///	Conversions ------------------------------------------------------------

	int get_number_si_fits () const
	{
		return mpz_fits_sint_p ( m_num );
	}

	signed long int get_number_si () const
	{
		if ( get_number_si_fits () )
		{
			return (int)mpz_get_si ( m_num );
		}

		throw 123;
	}

	int get_number_sli_fits () const
	{
		return mpz_fits_slong_p ( m_num );
	}

	signed long int get_number_sli () const
	{
		if ( get_number_sli_fits () )
		{
			return mpz_get_si ( m_num );
		}

		throw 123;
	}

	int get_number_uli_fits () const
	{
		return mpz_fits_ulong_p ( m_num );
	}

	unsigned long int get_number_uli () const
	{
		if ( get_number_uli_fits () )
		{
			return mpz_get_ui ( m_num );
		}

		throw 123;
	}

private:
	mpz_t		m_num;
};



class Rational
{
private:
	void init_str ( char const * const text )
	{
		mpq_init ( m_num );

		if ( set_number ( text ) )
		{
			std::cerr << "Invalid Rational: '" << text << "'\n";
			mpq_clear ( m_num );
			throw 123;
		}
	}

public:
	Rational ()
	{
		mpq_init ( m_num );
	}

	Rational ( int const num )
	{
		mpq_init ( m_num );

		set_number ( num );
	}

	Rational ( signed long int const num )
	{
		mpq_init ( m_num );

		set_number ( num );
	}

	Rational ( Rational const & i )
	{
		mpq_init ( m_num );

		set_number ( i );
	}

	Rational ( Integer const & i )
	{
		mpq_init ( m_num );

		set_number ( i );
	}

	Rational ( char const * const text )
	{
		init_str ( text );
	}

	Rational ( std::string const & text )
	{
		init_str ( text.c_str() );
	}

	~Rational()
	{
		mpq_clear ( m_num );
	}

	// Move constructor definition needed for creating C arrays
	Rational ( Rational && other )
	{
		mpq_init ( m_num );
		mpq_swap ( m_num, other.m_num );
	}

	// Copy assignment
	Rational & operator = ( Rational const & other )
	{
		if ( this != &other )
		{
			mpq_set ( m_num, other.m_num );
		}

		return *this;
	}

	// Move assignment
	Rational & operator = ( Rational && other )
	{
		if ( this != &other )
		{
			mpq_swap ( m_num, other.m_num );
		}

		return *this;
	}

	// After initialization, these are the most efficient ways to set values

	void set_number ( int const num )
	{
		mpq_set_si ( m_num, num, 1 );
	}

	void set_number ( signed long int const num )
	{
		mpq_set_si ( m_num, num, 1 );
	}

	int set_number (
		signed long int const num,
		unsigned long int const den
		)
	{
		if ( 0 == den )
		{
			return 1;
		}

		mpq_set_si ( m_num, num, den );

		mpq_canonicalize ( m_num );

		return 0;
	}

	void set_number ( Rational const & num )
	{
		mpq_set ( m_num, num.m_num );
	}

	void set_number ( Integer const & num )
	{
		mpq_set_z ( m_num, num.get_num() );
	}

	int set_number (
		Integer const & numerator,
		Integer const & denominator
		)
	{
		if ( 0 == denominator.sign() )
		{
			return 1;
		}

		mpz_set ( mpq_numref( m_num ), numerator.get_num() );
		mpz_set ( mpq_denref( m_num ), denominator.get_num() );

		mpq_canonicalize ( m_num );

		return 0;
	}

	int set_number ( char const * const text );

	int set_number ( std::string const & text )
	{
		return set_number ( text.c_str() );
	}

///	Logical comparisons ----------------------------------------------------

	int cmp ( Rational const & rhs ) const
	{
		return mpq_cmp ( m_num, rhs.m_num );
	}

	bool operator <  ( Rational const & rhs ) const { return cmp(rhs) <  0;}
	bool operator >  ( Rational const & rhs ) const { return cmp(rhs) >  0;}
	bool operator <= ( Rational const & rhs ) const { return cmp(rhs) <= 0;}
	bool operator >= ( Rational const & rhs ) const { return cmp(rhs) >= 0;}
	bool operator == ( Rational const & rhs ) const { return cmp(rhs) == 0;}
	bool operator != ( Rational const & rhs ) const { return cmp(rhs) != 0;}

	void set_bound ( Rational const & min, Rational const & max )
	{
		if ( *this < min )
		{
			this->set_number ( min );
			return;
		}

		if ( *this > max )
		{
			this->set_number ( max );
			return;
		}
	}

///	Arithmetic -------------------------------------------------------------

	Rational & operator += ( Rational const & rhs )
	{
		mpq_add ( m_num, m_num, rhs.m_num );
		return *this;
	}

	Rational & operator -= ( Rational const & rhs )
	{
		mpq_sub ( m_num, m_num, rhs.m_num );
		return *this;
	}

	Rational & operator *= ( Rational const & rhs )
	{
		mpq_mul ( m_num, m_num, rhs.m_num );
		return *this;
	}

	Rational & operator /= ( Rational const & rhs )
	{
		if ( mpq_cmp_ui ( rhs.m_num, 0, 1 ) == 0 )
		{
			throw 123;
		}

		mpq_div ( m_num, m_num, rhs.m_num );
		return *this;
	}

	// NOTE: These friend operators are somewhat inefficient, so not worth
	// using inside inner loops, but useful for (a+b).to_string().

	friend Rational operator + ( Rational lhs, Rational const & rhs )
	{
		lhs += rhs;
		return lhs;
	}

	friend Rational operator - ( Rational lhs, Rational const & rhs )
	{
		lhs -= rhs;
		return lhs;
	}

	friend Rational operator * ( Rational lhs, Rational const & rhs )
	{
		lhs *= rhs;
		return lhs;
	}

	friend Rational operator / ( Rational lhs, Rational const & rhs )
	{
		lhs /= rhs;
		return lhs;
	}

///	Convenience functions --------------------------------------------------

	void abs ()
	{
		mpq_abs ( m_num, m_num );
	}

	void denom ()
	{
		mpz_set_ui ( mpq_numref (m_num), 1 );

		mpq_inv ( m_num, m_num );
	}

	void inv ()
	{
		if ( mpq_cmp_ui ( m_num, 0, 1 ) == 0 )
		{
			throw 123;
		}

		mpq_inv ( m_num, m_num );
	}

	void max ( Rational const & a, Rational const & b )
	{
		if ( a < b )
		{
			this->set_number ( b );
		}
		else
		{
			this->set_number ( a );
		}
	}

	void min ( Rational const & a, Rational const & b )
	{
		if ( a > b )
		{
			this->set_number ( b );
		}
		else
		{
			this->set_number ( a );
		}
	}

	void numer ()
	{
		mpz_set_ui ( mpq_denref (m_num), 1 );
	}

	void pow ( Rational const & a, Rational const & b )
	{
		if (	mpz_cmp_ui ( mpq_denref ( b.m_num ), 1 ) != 0
			|| ! mpz_fits_ulong_p ( mpq_numref ( b.m_num ) )
			)
		{
			throw 123;
		}

		unsigned long int const p = mpz_get_ui ( mpq_numref(b.m_num) );

		mpz_pow_ui( mpq_numref (m_num), mpq_numref (a.m_num), p );
		mpz_pow_ui( mpq_denref (m_num), mpq_denref (a.m_num), p );
	}

	void negate ()
	{
		mpq_neg ( m_num, m_num );
	}

	int sign () const
	{
		return mpq_sgn ( m_num );
	}
	// -1 : num < 0
	//  0 : num == 0
	// +1 : num > 0

	void swap ( Rational & rhs )
	{
		mpq_swap ( m_num, rhs.m_num );
	}

	size_t get_str_ndigits () const;

///	I/O funcs --------------------------------------------------------------

	std::string to_string () const;
	std::string to_string_snip ( size_t maxlen ) const;
	int to_file ( FILE * const fp ) const;
	int to_filename ( char const * filename ) const;

	int to_stdout () const
	{
		return to_file ( stdout );
	}

///	Accessor ---------------------------------------------------------------

	mpq_t & get_num ()		{ return m_num; }
	mpq_t const & get_num () const	{ return m_num; }

///	Conversions ------------------------------------------------------------

	double get_number_double () const
	{
		return mpq_get_d ( m_num );
	}

private:
	void parse_rational_number ( char const * const text );

/// ----------------------------------------------------------------------------

	mpq_t		m_num;
};



class Float
{
private:
	void init_str ( char const * const text )
	{
		if ( text )
		{
			if ( mpfr_init_set_str ( m_num, text, 10, m_rnd ) )
			{
				std::cerr << "Invalid Float: '" << text <<"'\n";
				throw 123;
			}
		}
		else
		{
			mpfr_init ( m_num );
		}
	}
public:
	Float ()
	{
		if ( mpfr_init_set_si ( m_num, 0, m_rnd ) )
		{
			throw 123;
		}
	}

	Float ( int const num )
	{
		if ( mpfr_init_set_si ( m_num, num, m_rnd ) )
		{
			throw 123;
		}
	}

	Float ( double const num )
	{
		if ( mpfr_init_set_d ( m_num, num, m_rnd ) )
		{
			throw 123;
		}
	}

	Float ( Integer const & num )
	{
		if ( mpfr_init_set_z ( m_num, num.get_num(), m_rnd ) )
		{
			throw 123;
		}
	}

	Float ( Float const & num )
	{
		if ( mpfr_init_set ( m_num, num.get_num(), m_rnd ) )
		{
			throw 123;
		}
	}

	Float ( char const * const text )
	{
		init_str ( text );
	}

	Float ( std::string const & text )
	{
		init_str ( text.c_str() );
	}

	~Float ()
	{
		mpfr_clear ( m_num );
	}

	// Move constructor definition needed for creating C arrays
	Float ( Float && other )
	{
		mpfr_init2 ( m_num, MPFR_PREC_MIN );
		mpfr_swap ( m_num, other.m_num );
	}

	// Copy assignment
	Float & operator = ( Float const & other )
	{
		if ( this != &other )
		{
			mpfr_set ( m_num, other.m_num, m_rnd );
		}

		return *this;
	}

	// Move assignment
	Float & operator = ( Float && other )
	{
		if ( this != &other )
		{
			mpfr_swap ( m_num, other.m_num );
		}

		return *this;
	}

	// After initialization, these are the most efficient ways to set values

	int set_number ( int const num )
	{
		set_precision ();
		return mpfr_set_si ( m_num, num, m_rnd );
	}

	int set_number ( double const num  )
	{
		set_precision ();
		return mpfr_set_d ( m_num, num, m_rnd );
	}

	int set_number ( Float const & num )
	{
		return mpfr_set ( m_num, num.m_num, m_rnd );
	}

	int set_number ( Rational const & num )
	{
		set_precision ();
		return mpfr_set_q ( m_num, num.get_num(), m_rnd );
	}

	int set_number ( char const * const text )
	{
		set_precision ();
		return text ? mpfr_set_str ( m_num, text, 10, m_rnd ) : 1;
	}

	int set_number ( std::string const & text )
	{
		return set_number ( text.c_str() );
	}

///	Logical comparisons ----------------------------------------------------

	int cmp ( Float const & rhs ) const
	{
		return mpfr_cmp ( m_num, rhs.m_num );
	}

	bool operator <  ( Float const & rhs ) const
	{
		return mpfr_less_p ( m_num, rhs.m_num ) != 0;
	}

	bool operator >  ( Float const & rhs ) const
	{
		return mpfr_greater_p ( m_num, rhs.m_num ) != 0;
	}

	bool operator <= ( Float const & rhs ) const
	{
		return mpfr_lessequal_p ( m_num, rhs.m_num ) != 0;
	}

	bool operator >= ( Float const & rhs ) const
	{
		return mpfr_greaterequal_p ( m_num, rhs.m_num ) != 0;
	}

	bool operator == ( Float const & rhs ) const
	{
		return mpfr_equal_p ( m_num, rhs.m_num ) != 0;
	}

	bool operator != ( Float const & rhs ) const
	{
		return mpfr_equal_p ( m_num, rhs.m_num ) == 0;
	}

	void set_bound ( Float const & min, Float const & max )
	{
		if ( *this < min )
		{
			this->set_number ( min );
			return;
		}

		if ( *this > max )
		{
			this->set_number ( max );
			return;
		}
	}

///	Arithmetic -------------------------------------------------------------

	Float & operator += ( Float const & rhs )
	{
		mpfr_add ( m_num, m_num, rhs.m_num, m_rnd );
		return *this;
	}

	Float & operator -= ( Float const & rhs )
	{
		mpfr_sub ( m_num, m_num, rhs.m_num, m_rnd );
		return *this;
	}

	Float & operator *= ( Float const & rhs )
	{
		mpfr_mul ( m_num, m_num, rhs.m_num, m_rnd );
		return *this;
	}

	Float & operator /= ( Float const & rhs )
	{
		mpfr_div ( m_num, m_num, rhs.m_num, m_rnd );
		return *this;
	}

	// NOTE: These friend operators are somewhat inefficient, so not worth
	// using inside inner loops, but useful for (a+b).to_string().

	friend Float operator + ( Float lhs, Float const & rhs )
	{
		lhs += rhs;
		return lhs;
	}

	friend Float operator - ( Float lhs, Float const & rhs )
	{
		lhs -= rhs;
		return lhs;
	}

	friend Float operator * ( Float lhs, Float const & rhs )
	{
		lhs *= rhs;
		return lhs;
	}

	friend Float operator / ( Float lhs, Float const & rhs )
	{
		lhs /= rhs;
		return lhs;
	}

///	Convenience functions --------------------------------------------------

	void abs ()
	{
		mpfr_abs ( m_num, m_num, m_rnd );
	}

	void acos ( Float const & a )
	{
		mpfr_acos ( m_num, a.m_num, m_rnd );
	}

	void asin ( Float const & a )
	{
		mpfr_asin ( m_num, a.m_num, m_rnd );
	}

	void atan ( Float const & a )
	{
		mpfr_atan ( m_num, a.m_num, m_rnd );
	}

	void atan2 ( Float const & y, Float const & x )
	{
		mpfr_atan2 ( m_num, y.m_num, x.m_num, m_rnd );
	}

	void ceil ( Float const & a )
	{
		mpfr_ceil ( m_num, a.m_num );
	}

	void cos ( Float const & a )
	{
		mpfr_cos ( m_num, a.m_num, m_rnd );
	}

	void cot ( Float const & a )
	{
		mpfr_cot ( m_num, a.m_num, m_rnd );
	}

	void csc ( Float const & a )
	{
		mpfr_csc ( m_num, a.m_num, m_rnd );
	}

	void exp ( Float const & a )
	{
		mpfr_exp ( m_num, a.m_num, m_rnd );
	}

	void floor ( Float const & a )
	{
		mpfr_floor ( m_num, a.m_num );
	}

	void frac ( Float const & a )
	{
		mpfr_frac ( m_num, a.m_num, m_rnd );
	}

	bool is_inf () const
	{
		return mpfr_inf_p ( m_num );
	}

	bool is_nan () const
	{
		return mpfr_nan_p ( m_num );
	}

	bool is_number () const
	{
		return mpfr_number_p ( m_num );
	}

	void log ( Float const & a )
	{
		mpfr_log ( m_num, a.m_num, m_rnd );
	}

	void max ( Float const & a, Float const & b )
	{
		mpfr_max ( m_num, a.m_num, b.m_num, m_rnd );
	}

	void min ( Float const & a, Float const & b )
	{
		mpfr_min ( m_num, a.m_num, b.m_num, m_rnd );
	}

	void mod ( Float const & a, Float const & b )
	{
		mpfr_fmod ( m_num, a.m_num, b.m_num, m_rnd );
	}

	void negate ()
	{
		mpfr_neg ( m_num, m_num, m_rnd );
	}

	void pi ()
	{
		mpfr_const_pi ( m_num, m_rnd );
	}

	void pow ( Float const & a, Float const & b )
	{
		mpfr_pow ( m_num, a.m_num, b.m_num, m_rnd );
	}

	void round ( Float const & a )
	{
		mpfr_round ( m_num, a.m_num );
	}

	int sign () const
	{
		return mpfr_sgn ( m_num );
	}
	// -1 : num < 0
	//  0 : num == 0	NaN is also 0, and sets erange flag
	// +1 : num > 0

	void sin ( Float const & a )
	{
		mpfr_sin ( m_num, a.m_num, m_rnd );
	}

	void tan ( Float const & a )
	{
		mpfr_tan ( m_num, a.m_num, m_rnd );
	}

	void sec ( Float const & a )
	{
		mpfr_sec ( m_num, a.m_num, m_rnd );
	}

	void swap ( Float & rhs )
	{
		mpfr_swap ( m_num, rhs.m_num );
	}

	void trunc ( Float const & a )
	{
		mpfr_trunc ( m_num, a.m_num );
	}

	size_t get_str_ndigits () const;

///	I/O funcs --------------------------------------------------------------

	std::string to_string () const;
	std::string to_string_snip ( size_t maxlen ) const;
	int to_file ( FILE * const fp ) const;
	int to_filename ( char const * filename ) const;

	int to_stdout () const
	{
		return to_file ( stdout );
	}

	// Get precision bits required to hold this decimal string
	mpfr_prec_t get_str_precision(char const * const str, size_t length =0);
		// length = 0 means use strlen(str) to get length
		// = 53 or more

	static constexpr mpfr_prec_t	DOUBLE_PRECISION = 53;	// Bits

///	Accessor ---------------------------------------------------------------

	mpfr_t & get_num ()			{ return m_num; }
	mpfr_t const & get_num () const		{ return m_num; }

	mpfr_prec_t get_precision () const	{ return mpfr_get_prec(m_num); }

	void set_precision ( mpfr_prec_t const prec = mpfr_get_default_prec() )
	{
		mpfr_set_prec ( m_num, prec );
	}

	static mpfr_rnd_t get_rnd ()		{ return m_rnd; }

///	Conversions ------------------------------------------------------------

	double get_number_double () const
	{
		return mpfr_get_d ( m_num, m_rnd );
	}

private:
	mpfr_t			m_num;
	static mpfr_rnd_t const	m_rnd = MPFR_RNDN;
};



class Double
{
private:
	void init_str ( char const * const text )
	{
		if ( set_number ( text ) )
		{
			std::cerr << "Invalid Double: '" << text <<"'\n";
			throw 123;
		}
	}
public:
	Double ()
	{
		m_num = 0.0;
	}

	Double ( int const num )
	{
		m_num = num;
	}

	Double ( double const num )
	{
		m_num = num;
	}

	Double ( Double const & num )
	{
		m_num = num.m_num;
	}

	Double ( char const * const text )
	{
		init_str ( text );
	}

	Double ( std::string const & text )
	{
		init_str ( text.c_str() );
	}

/*	No destructor required

	~Double ()
	{
	}
*/

	// Move constructor definition needed for creating C arrays
	Double ( Double && other )
	{
		m_num = other.m_num;
	}

	// Copy assignment
	Double & operator = ( Double const & other )
	{
		if ( this != &other )
		{
			m_num = other.m_num;
		}

		return *this;
	}

	// Move assignment
	Double & operator = ( Double && other )
	{
		if ( this != &other )
		{
			m_num = other.m_num;
		}

		return *this;
	}

	// After initialization, these are the most efficient ways to set values

	void set_number ( int const num )
	{
		m_num = num;
	}

	void set_number ( double const num )
	{
		m_num = num;
	}

	void set_number ( Double const & num )
	{
		m_num = num.m_num;
	}

	int set_number ( char const * const text )
	{
		return mtkit_strtod ( text, & m_num, nullptr, 0 );
	}

	int set_number ( std::string const & text )
	{
		return set_number ( text.c_str() );
	}

///	Logical comparisons ----------------------------------------------------

	int cmp ( Double const & rhs ) const
	{
		if ( m_num > rhs.m_num )
		{
			return 1;
		}

		if ( m_num < rhs.m_num )
		{
			return -1;
		}

		return 0;
	}

	bool operator <  ( Double const & rhs ) const
	{
		return m_num < rhs.m_num ? true : false;
	}

	bool operator >  ( Double const & rhs ) const
	{
		return m_num > rhs.m_num ? true : false;
	}

	bool operator <= ( Double const & rhs ) const
	{
		return m_num <= rhs.m_num ? true : false;
	}

	bool operator >= ( Double const & rhs ) const
	{
		return m_num >= rhs.m_num ? true : false;
	}

	bool operator == ( Double const & rhs ) const
	{
		return m_num == rhs.m_num ? true : false;
	}

	bool operator != ( Double const & rhs ) const
	{
		return m_num != rhs.m_num ? true : false;
	}

	void set_bound ( Double const & min, Double const & max )
	{
		if ( m_num < min.m_num )
		{
			m_num = min.m_num;
			return;
		}

		if ( m_num > max.m_num )
		{
			m_num = max.m_num;
			return;
		}
	}

///	Arithmetic -------------------------------------------------------------

	Double & operator += ( Double const & rhs )
	{
		m_num = m_num + rhs.m_num;
		return *this;
	}

	Double & operator -= ( Double const & rhs )
	{
		m_num = m_num - rhs.m_num;
		return *this;
	}

	Double & operator *= ( Double const & rhs )
	{
		m_num = m_num * rhs.m_num;
		return *this;
	}

	Double & operator /= ( Double const & rhs )
	{
		m_num = m_num / rhs.m_num;
		return *this;
	}

	// NOTE: These friend operators are somewhat inefficient, so not worth
	// using inside inner loops, but useful for (a+b).to_string().

	friend Double operator + ( Double lhs, Double const & rhs )
	{
		lhs += rhs;
		return lhs;
	}

	friend Double operator - ( Double lhs, Double const & rhs )
	{
		lhs -= rhs;
		return lhs;
	}

	friend Double operator * ( Double lhs, Double const & rhs )
	{
		lhs *= rhs;
		return lhs;
	}

	friend Double operator / ( Double lhs, Double const & rhs )
	{
		lhs /= rhs;
		return lhs;
	}

///	Convenience functions --------------------------------------------------

	void abs ()
	{
		m_num = ::abs ( m_num );
	}

	void acos ( Double const & a )
	{
		m_num = ::acos ( a.m_num );
	}

	void asin ( Double const & a )
	{
		m_num = ::asin ( a.m_num );
	}

	void atan ( Double const & a )
	{
		m_num = ::atan ( a.m_num );
	}

	void atan2 ( Double const & y, Double const & x )
	{
		m_num = ::atan2 ( y.m_num, x.m_num );
	}

	void ceil ( Double const & a )
	{
		m_num = ::ceil ( a.m_num );
	}

	void cos ( Double const & a )
	{
		m_num = ::cos ( a.m_num );
	}

	void cot ( Double const & a )
	{
		m_num = 1 / ::tan ( a.m_num );
	}

	void csc ( Double const & a )
	{
		m_num = 1 / ::sin ( a.m_num );
	}

	void exp ( Double const & a )
	{
		m_num = ::exp ( a.m_num );
	}

	void floor ( Double const & a )
	{
		m_num = ::floor ( a.m_num );
	}

	void frac ( Double const & a )
	{
		double tmp;
		m_num = ::modf ( a.m_num, &tmp );
	}

	bool is_inf () const
	{
		return isinf ( m_num ) ? true : false;
	}

	bool is_nan () const
	{
		return isnan ( m_num ) ? true : false;
	}

	bool is_number () const
	{
		return isnormal ( m_num ) ? true : false;
	}

	void log ( Double const & a )
	{
		m_num = ::log ( a.m_num );
	}

	void max ( Double const & a, Double const & b )
	{
		m_num = a.m_num > b.m_num ? a.m_num : b.m_num;
	}

	void min ( Double const & a, Double const & b )
	{
		m_num = a.m_num < b.m_num ? a.m_num : b.m_num;
	}

	void mod ( Double const & a, Double const & b )
	{
		m_num = ::fmod ( a.m_num, b.m_num );
	}

	void negate ()
	{
		m_num = -m_num;
	}

	void pi ()
	{
		m_num = M_PI;
	}

	void pow ( Double const & a, Double const & b )
	{
		m_num = ::pow ( a.m_num, b.m_num );
	}

	void round ( Double const & a )
	{
		m_num = ::round ( a.m_num );
	}

	int sign () const
	{
		if ( m_num < 0 )
		{
			return -1;
		}

		if ( m_num > 0 )
		{
			return 1;
		}

		return 0;
	}
	// -1 : num < 0
	//  0 : num == 0
	// +1 : num > 0

	void sin ( Double const & a )
	{
		m_num = ::sin ( a.m_num );
	}

	void tan ( Double const & a )
	{
		m_num = ::tan ( a.m_num );
	}

	void sec ( Double const & a )
	{
		m_num = 1.0 / ::cos ( a.m_num );
	}

	void swap ( Double & rhs )
	{
		double const tmp = m_num;

		m_num = rhs.m_num;
		rhs.m_num = tmp;
	}

	void trunc ( Double const & a )
	{
		m_num = ::trunc ( a.m_num );
	}

///	I/O funcs --------------------------------------------------------------

	std::string to_string () const;
	std::string to_string_snip ( size_t maxlen ) const;
	int to_file ( FILE * const fp ) const;
	int to_filename ( char const * filename ) const;

	int to_stdout () const
	{
		return to_file ( stdout );
	}

///	Accessor ---------------------------------------------------------------

	double get_num () const			{ return m_num; }

private:
	double			m_num;
};



class Lexer
{
public:
	enum
	{
		// Tokens/Errors/etc from the Lexer
		// Whitespace between tokens is skipped and not reported

		TK_END			= 0,	// End of input

		TK_NUM,			// 1, -1.1, -1.1e+12, -1.1e-12, etc

		TK_STRING,		// Function or variable name

		TK_OP_ADD,		// +
		TK_OP_SUB,		// -
		TK_OP_MULT,		// *
		TK_OP_DIV,		// /
		TK_OP_POWER,		// ^

		TK_CMP_LT,		// <	=> 1,0
		TK_CMP_LTE,		// <=	=> 1,0
		TK_CMP_GT,		// >	=> 1,0
		TK_CMP_GTE,		// >=	=> 1,0
		TK_CMP_EQ,		// ==	=> 1,0
		TK_CMP_NEQ,		// !=	=> 1,0
		TK_CMP_LEG,		// <=>	=> -1,0,1

		TK_PAREN_OPEN,		// (
		TK_PAREN_CLOSE,		// )

		TK_ASSIGN,		// =	"variable = expression"
		TK_ASSIGN_ADD,		// +=
		TK_ASSIGN_SUB,		// -=
		TK_ASSIGN_MULT,		// *=
		TK_ASSIGN_DIV,		// /=
		TK_ASSIGN_POWER,	// ^=

		TK_ARG_SEP,		// ,	Function argument separator

		TK_EXP_SEP,		// ;	Expression separator

		// Token errors
		TK_BAD_SYMBOL,
		TK_BAD_NUMBER,

		TK_MAX
	};


	virtual ~Lexer () {}		// V destructor as we use V funcs below

	void init ( char const * const input )
	{
		m_input = m_token = m_symbol = input;
	}

	int scan_token ();
		// TK_*

	std::string const & string () const { return m_string; }

	int input_pos () const	{ return (int)(m_symbol - m_input); }

	void rewind_token ()	{ m_symbol = m_token; }

	char const * token_position () const { return m_token; }
	void set_token_position ( char const * const pos )
		{ m_symbol = m_token = pos; }

	static int is_whitespace ( char ch )
	{
		switch ( ch )
		{
		case ' ':
		case '\t':
			return 1;
		}

		return 0;
	}

	static int is_alpha ( char ch )
	{
		if (	(ch >= 'a' && ch <= 'z')	||
			(ch >= 'A' && ch <= 'Z')
			)
		{
			return 1;
		}

		return 0;
	}

	static int is_numeric ( char ch )
	{
		if ( (ch >= '0' && ch <= '9') )
		{
			return 1;
		}

		return 0;
	}

protected:
	// scan_token() has found a digit/point, so scan the remaining digits
	// and tokenize.
	virtual int scan_number () = 0;
	virtual int scan_number_decimal ();

	int scan_number_scientific ();	// 1, 1.2, 1e5, 1.2e5, 1.2e-5, etc
	int scan_number_exponent ();
	int scan_number_exponent_number ();
	int scan_trailing_digits ();	// Scan trailing digits and tokenize

	void scan_digits ();

	int scan_string ();		// variable/function name

	char scan_symbol ()	{ return *m_symbol++; }
	void rewind_symbol ()	{ m_symbol--; }

	void assign_string ()	// Tokenize the validated string
	{
		// Caller ensures (m_symbol > m_token)
		m_string.assign ( m_token, (size_t)(m_symbol - m_token) );
	}

private:
	char	const * m_input		= nullptr;
	char	const * m_token		= nullptr;
	char	const * m_symbol	= nullptr;

	std::string	m_string;	// Most recent number or string
};



class DoubleLexer : public Lexer
{
private:
	int scan_number ()			override;
};



class FloatLexer : public Lexer
{
private:
	int scan_number ()			override;
};



class IntegerLexer : public Lexer
{
private:
	int scan_number ()			override;
	int scan_number_decimal ()		override
	{
		return TK_BAD_SYMBOL;
	}
};



class RationalLexer : public Lexer
{
private:
	int scan_number ()			override;
};



template< typename Tnum, typename Tlexer > class Parser
{
public:
	virtual ~Parser () {}

	int evaluate ( char const * const text )
	{
		if ( ! text )
		{
			return ERROR_EVAL_TEXT_NULL;
		}

		if ( strlen ( text ) > mtDW::Number::EVAL_TEXT_MAX_SIZE )
		{
			return ERROR_EVAL_TEXT_TOO_LARGE;
		}

		return evaluate_internal ( text );
	}

	int error_pos () const
	{
		return m_lexer.input_pos();
	}

	std::map<std::string, Tnum> & variables ()
	{
		return m_variables;
	}

	std::map<std::string, Tnum> const & variables () const
	{
		return m_variables;
	}

	Tnum const * get_variable (
		std::string const & name
		) const
		// nullptr = doesn't exist, else the variable
	{
		auto const it = m_variables.find ( name );

		if ( it != m_variables.end() )
		{
			return &it->second;	// mtDW::Float/Integer/etc
		}

		return nullptr;			// Not found
	}

	Tnum const & result() const
	{
		return m_result;
	}

	virtual int get_function_data (
		int		index, // 0+=return func data, <0=return argtot
		char	const	** name,
		std::string	& help	//  e.g. "( a1, a2, a3 ) = Returns ..."
		) const = 0;

protected:
	std::map<std::string, Tnum>
				m_variables;
	Tlexer			m_lexer;
	Tnum			m_result;

private:
	virtual int evaluate_internal ( char const * text ) = 0;
		// 0=success, else ERROR_*
};



class DoubleParser : public Parser<Double, DoubleLexer>
{
public:
	int get_function_data (
		int		index,
		char	const	** name,
		std::string	& help
		) const					override;

private:
	int evaluate_internal ( char const * text )	override;
};



class FloatParser : public Parser<Float, FloatLexer>
{
public:
	int get_function_data (
		int		index,
		char	const	** name,
		std::string	& help
		) const					override;

private:
	int evaluate_internal ( char const * text )	override;
};



class IntegerParser : public Parser<Integer, IntegerLexer>
{
public:
	int get_function_data (
		int		index,
		char	const	** name,
		std::string	& help
		) const					override;

private:
	int evaluate_internal ( char const * text )	override;
};



class RationalParser : public Parser<Rational, RationalLexer>
{
public:
	int get_function_data (
		int		index,
		char	const	** name,
		std::string	& help
		) const					override;

private:
	int evaluate_internal ( char const * text )	override;
};



class IntegerMemory
{
/*
	A class for temporarily holding integer binary data for import/export.
*/
public:
	enum
	{
		HEADER_SIZE	= 10
	};

	IntegerMemory () {}
	~IntegerMemory () { clear(); }

	int import_memory ( unsigned char const * mem, size_t mem_size,
		int num_sign );
	int import_memory_with_header ( unsigned char const * mem,
		size_t mem_size );
	int import_number ( Integer const & num );
	int import_file ( ::FILE * fp );
	int import_file ( char const * filename );

	static int import_memory_export_number (
		unsigned char const * mem,
		size_t mem_size,
		int num_sign,
		Integer & num
		);
	int export_number ( Integer & num ) const;
	int export_file ( ::FILE * fp ) const;
	int export_file ( char const * filename ) const;

	void get_data ( int & extra, size_t & size, unsigned char const *& mem
		) const
	{
		extra = m_extra;
		size = m_size;
		mem = m_mem;
	}

private:
	void clear ()
	{
		m_extra = 0;
		m_size = 0;
		free ( m_mem );
		m_mem = nullptr;
	}

	int allocate_buffer (
		size_t bufsize,		// Without header
		int num_sign
		);

/// ----------------------------------------------------------------------------

	int		m_extra	= 0; // 0=+ 1=-
	size_t		m_size	= 0; // Size of mem buffer (including header)
	unsigned char	* m_mem	= nullptr;

/*	Memory format:
	mem[0] = header size in bytes
	mem[1] = extra (bit 1 = sign)
	mem[2-9] = size (little endian)
	After header, (m_size - HEADER_SIZE) bytes appear in little endian form.
*/

	MTKIT_RULE_OF_FIVE( IntegerMemory )
};



}	// namespace mtDW



#endif		// C++ API



#endif		// MTDATAWELL_MATH_H_

