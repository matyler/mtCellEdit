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

#include "qt5.h"



template < typename Tparser >
void evaluate (
	std::string		& result,
	MainWindow	* const	window,
	QPlainTextEdit	* const	input,
	Tparser			& parser,
	QPlainTextEdit	* const	output,
	mpfr_prec_t	const	num_bits,
	int		const	snip_size
	)
{
	mtQEX::BusyDialog busy ( window, nullptr,
	[&parser, &result, input, num_bits]()
	{
		std::string	txt;
		QString		whole = input->toPlainText();
		QStringList	lines = whole.split ( '\n' );

		parser.variables().clear();

		if ( num_bits )
		{
			mpfr_set_default_prec ( num_bits );
		}

		int l = 0;
		int e = 0;

		for ( ; l < lines.size(); l++ )
		{
			txt = lines[l].toStdString();

			if (	txt.size() < 1
				|| txt[0] == '#'
				)
			{
				continue;
			}

			e = parser.evaluate ( txt.c_str() );
			if ( e )
			{
				break;
			}
		}

		if ( e )
		{
			char buf[128];
			result = "ERROR!\n";

			snprintf ( buf, sizeof(buf), "Line %i:\n", l+1 );

			result += buf;
			result += txt;
			result += '\n';

			size_t const pos = (size_t)parser.error_pos ();

			result += txt.substr ( 0, pos );
			result += "^\n";
			result += mtDW::get_error_text ( e );
		}

		if ( num_bits )
		{
			mpfr_free_cache();
		}
	});

	busy.wait_for_thread ();

	if ( result.size() == 0 )
	{
		// String is empty, so no error occurred. Get the number.
		result = parser.result().to_string_snip ( (size_t)snip_size );
	}

	output->setPlainText ( result.c_str() );
}



/// ----------------------------------------------------------------------------



std::string choose_output_filename (
	QWidget		* const	parent,
	char	const * const	title
	)
{
	mtQEX::SaveFileDialog dialog ( parent, title );

	// Loop until successful save or user cancel
	while ( dialog.exec () )
	{
		QString filename = mtQEX::get_filename ( dialog );

		if ( filename.isEmpty () )
		{
			continue;
		}

		return filename.toStdString();
	}

	return "";
}

void report_file_error (
	QWidget		* const	parent,
	char	const * const	message,
	std::string	const	& filename
	)
{
	std::string txt ( message );

	txt += '\n';
	txt += filename;

	QMessageBox::critical ( parent, "Error", mtQEX::qstringFromC (
		txt.c_str() ) );
}



/// ----------------------------------------------------------------------------



void MainWindow::press_evaluate_float ()
{
	std::string result;

	::evaluate ( result, this, m_text_float_input, m_parser_float,
		m_text_float_output, 1 << mprefs.calc_float_bits,
		mprefs.calc_snip_size );

	update_float_digits_label ( result );
}

void MainWindow::press_copy_float_output ()
{
	QApplication::clipboard()->setText ( m_text_float_output->toPlainText());
}

void MainWindow::update_float_digits_label ( std::string const & digits )
{
	std::string	str ( "Output " );

	str += print_digits_label ( m_parser_float.result(),
		(int)digits.size() );

	m_group_box_float->setTitle ( str.c_str() );
}

void MainWindow::press_save_float_output ()
{
	std::string const filename = choose_output_filename ( this,
		"Save Float number" );

	save_a_number ( this, m_parser_float.result(), filename );
}



/// ----------------------------------------------------------------------------



void MainWindow::press_evaluate_double ()
{
	std::string result;

	::evaluate ( result, this, m_text_double_input, m_parser_double,
		m_text_double_output, 0, mprefs.calc_snip_size );

	update_double_digits_label ( result );
}

void MainWindow::press_copy_double_output ()
{
	QApplication::clipboard()->setText ( m_text_double_output->toPlainText());
}

void MainWindow::update_double_digits_label ( std::string const & digits )
{
	std::string	str ( "Output " );

	str += print_digits_label ( m_parser_double.result(),
		(int)digits.size() );

	m_group_box_double->setTitle ( str.c_str() );
}

void MainWindow::press_save_double_output ()
{
	std::string const filename = choose_output_filename ( this,
		"Save Float number" );

	save_a_number ( this, m_parser_double.result(), filename );
}



/// ----------------------------------------------------------------------------



void MainWindow::press_evaluate_int ()
{
	std::string result;

	::evaluate ( result, this, m_text_int_input, m_parser_integer,
		m_text_int_output, 0, mprefs.calc_snip_size );

	update_int_digits_label ( result );
}

void MainWindow::press_copy_int_output ()
{
	QApplication::clipboard()->setText ( m_text_int_output->toPlainText());
}

void MainWindow::update_int_digits_label ( std::string const & digits )
{
	std::string	str ( "Output " );

	str += print_digits_label ( m_parser_integer.result(),
		(int)digits.size() );

	m_group_box_int->setTitle ( str.c_str() );
}

void MainWindow::press_save_int_output ()
{
	std::string const filename = choose_output_filename ( this,
		"Save Integer number" );

	save_a_number ( this, m_parser_integer.result(), filename );
}



/// ----------------------------------------------------------------------------



void MainWindow::press_evaluate_rational ()
{
	std::string result;

	::evaluate ( result, this, m_text_rational_input, m_parser_rational,
		m_text_rational_output, 0, mprefs.calc_snip_size );

	update_rational_digits_label ( result );
}

void MainWindow::press_copy_rational_output ()
{
	QApplication::clipboard()->setText ( m_text_rational_output->
		toPlainText() );
}

void MainWindow::update_rational_digits_label ( std::string const & digits )
{
	std::string	str ( "Output " );

	str += print_digits_label ( m_parser_rational.result(),
		(int)digits.size() );

	m_group_box_rational->setTitle ( str.c_str() );
}

void MainWindow::press_save_rational_output ()
{
	std::string const filename = choose_output_filename ( this,
		"Save Rational number" );

	save_a_number ( this, m_parser_rational.result(), filename );
}

