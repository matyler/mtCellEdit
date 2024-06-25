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



int mtDW::Lexer::scan_token ()
{
	if ( ! m_input )
	{
		return TK_END;
	}

restart:
	while ( is_whitespace ( *m_symbol ) )
	{
		m_symbol++;
	}

	// No whitespace, so this symbol starts a token
	m_token = m_symbol;

	if ( 0 == *m_symbol )
	{
		return TK_END;
	}

	char const ch = scan_symbol ();

	switch ( ch )
	{
	case '+':
		switch ( scan_symbol () )
		{
		case '=':
			return TK_ASSIGN_ADD;
		default:
			rewind_symbol ();
			return TK_OP_ADD;
		}

	case '-':
		switch ( scan_symbol () )
		{
		case '=':
			return TK_ASSIGN_SUB;
		default:
			rewind_symbol ();
			return TK_OP_SUB;
		}

	case '*':
		switch ( scan_symbol () )
		{
		case '=':
			return TK_ASSIGN_MULT;
		default:
			rewind_symbol ();
			return TK_OP_MULT;
		}

	case '/':
		switch ( scan_symbol () )
		{
		case '=':
			return TK_ASSIGN_DIV;
		case '*':
			// Comment
			while (1)
			{
				switch ( scan_symbol () )
				{
				case 0:
					rewind_symbol ();
					return TK_END;
				case '*':
					switch ( scan_symbol () )
					{
					case 0:
						rewind_symbol ();
						return TK_END;
					case '/':
						goto restart;
					}
				}
			}
		default:
			rewind_symbol ();
			return TK_OP_DIV;
		}

	case '^':
		switch ( scan_symbol () )
		{
		case '=':
			return TK_ASSIGN_POWER;
		default:
			rewind_symbol ();
			return TK_OP_POWER;
		}

	case '(':
		return TK_PAREN_OPEN;

	case ')':
		return TK_PAREN_CLOSE;

	case '=':
		switch ( scan_symbol () )
		{
		case '=':
			return TK_CMP_EQ;
		default:
			rewind_symbol ();
			return TK_ASSIGN;
		}

	case '<':
		switch ( scan_symbol () )
		{
		case '=':
			switch ( scan_symbol () )
			{
			case '>':
				return TK_CMP_LEG;
			default:
				rewind_symbol ();
				return TK_CMP_LTE;
			}

		default:
			rewind_symbol ();
			return TK_CMP_LT;
		}

	case '>':
		switch ( scan_symbol () )
		{
		case '=':
			return TK_CMP_GTE;
		default:
			rewind_symbol ();
			return TK_CMP_GT;
		}

	case '!':
		switch ( scan_symbol () )
		{
		case '=':
			return TK_CMP_NEQ;
		default:
			rewind_symbol ();
			return TK_BAD_SYMBOL;
		}

	case ',':
		return TK_ARG_SEP;

	case ';':
	case '\f':
	case '\n':
	case '\r':
		return TK_EXP_SEP;

	case '.':
		return scan_number_decimal ();
	}

	if ( is_numeric ( ch ) )
	{
		return scan_number ();
	}

	if ( is_alpha ( ch ) )
	{
		return scan_string ();
	}

	// Symbol not recognised
	rewind_symbol ();

	return TK_BAD_SYMBOL;
}

int mtDW::Lexer::scan_number_scientific ()
{
	// Scan the remaining digits (to the left of the decimal point)
	scan_digits ();

	switch ( scan_symbol () )
	{
	case '.':
		return scan_number_decimal ();

	case 'e':
	case 'E':
		return scan_number_exponent ();
	}

	// Symbol not recognised, so we have passed the end of the number
	rewind_symbol ();

	assign_string ();

	return TK_NUM;
}

int mtDW::Lexer::scan_number_decimal ()
{
	// Parse the remaining digits (to the right of the decimal point)
	scan_digits ();

	switch ( scan_symbol () )
	{
	case 'e':
	case 'E':
		return scan_number_exponent ();
	}

	// Symbol not recognised, so we have passed the end of the number
	rewind_symbol ();

	assign_string ();

	return TK_NUM;
}

int mtDW::Lexer::scan_number_exponent ()
{
	char const ch = scan_symbol ();

	if ( '+' == ch || '-' == ch )
	{
		if ( is_numeric ( scan_symbol() ) )
		{
			return scan_number_exponent_number ();
		}

		// 123e+ | 123.e- | 123.456e+ => bad number
	}
	else
	{
		if ( is_numeric ( ch ) )
		{
			return scan_number_exponent_number ();
		}

		// 123e | 123.e | 123.456e => bad number
	}

	// Symbol not recognised, so this isn't a well formed number
	rewind_token ();

	return TK_BAD_NUMBER;
}

int mtDW::Lexer::scan_number_exponent_number ()
{
	return scan_trailing_digits ();
}

int mtDW::Lexer::scan_trailing_digits ()
{
	// Scan the remaining digits
	scan_digits ();

	assign_string ();

	return TK_NUM;
}

void mtDW::Lexer::scan_digits ()
{
	while ( is_numeric ( *m_symbol ) )
	{
		m_symbol++;
	}
}

int mtDW::Lexer::scan_string ()
{
	while (	is_alpha ( *m_symbol )		||
		is_numeric ( *m_symbol )	||
		*m_symbol == '_'
		)
	{
		m_symbol++;
	}

	assign_string ();

	return TK_STRING;
}



/// DOUBLE ---------------------------------------------------------------------



int mtDW::DoubleLexer::scan_number ()
{
	return scan_number_scientific ();
}



/// FLOAT ----------------------------------------------------------------------



int mtDW::FloatLexer::scan_number ()
{
	return scan_number_scientific ();
}



/// INTEGER --------------------------------------------------------------------



int mtDW::IntegerLexer::scan_number ()
{
	return scan_trailing_digits ();
}



/// RATIONAL -------------------------------------------------------------------



int mtDW::RationalLexer::scan_number ()
{
	return scan_number_scientific ();
}
