/*
	Copyright (C) 2022-2023 Mark Tyler

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

#ifdef U_TK_QT5
	#include <mtqex5.h>
#endif

#ifdef U_TK_QT6
	#include <mtqex6.h>
#endif



#include "static.h"
#include "be.h"



class MainWindow;
class DialogVars;



std::string choose_output_filename (
	QWidget		* parent,
	char	const	* title
	);

void report_file_error (
	QWidget			* parent,
	char		const	* message,
	std::string	const	& filename
	);



template < typename Tnum >
void save_a_number (
	QWidget		* const	parent,
	Tnum		const	& num,
	std::string	const	& filename
	)
{
	if ( num.to_filename ( filename.c_str() ) )
	{
		report_file_error ( parent, "Unable to save file:", filename );
	}
}



template <typename Tnum>
std::string print_digits_label (
	Tnum	const	& num,
	int	const	size
	)
{
	char		buf[128];

	snprintf ( buf, sizeof(buf), "(%i sf, showing %i digit",
		(int)num.get_str_ndigits(), size );

	std::string str = ( buf );

	if ( size == 1 )
	{
		str += ")";
	}
	else
	{
		str += "s)";
	}

	return str;
}



class MainWindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit MainWindow ( Cline & cline );
	~MainWindow ();

private slots:
	void press_menu_quit ();
	void press_menu_preferences ();

	void press_help_about_qt ();
	void press_help_about ();

	/*
	NOTE: Ideally I would like to create a template to handle these 3
	parts of the GUI, and have 3 calls to factory functions in Mainwindow().
	Sadly this isn't possible because subclassing Qt objects doesn't work
	for templates.  All forms of getting around this creates more complexity
	than is here for setting up 3 sets of GUI items.
	*/

	void press_evaluate_float ();
	void press_show_float_funcs ();
	void press_show_float_vars ();
	void press_copy_float_output ();
	void press_save_float_output ();

	void press_evaluate_int ();
	void press_show_int_funcs ();
	void press_show_int_vars ();
	void press_copy_int_output ();
	void press_save_int_output ();

	void press_evaluate_rational ();
	void press_show_rational_funcs ();
	void press_show_rational_vars ();
	void press_copy_rational_output ();
	void press_save_rational_output ();

private:
	void create_menu ();

	void update_float_digits_label ( std::string const & digits );
	void update_int_digits_label ( std::string const & digits );
	void update_rational_digits_label ( std::string const & digits );


/// ----------------------------------------------------------------------------

	mtKit::UserPrefs	& uprefs;
	MemPrefs	const	& mprefs;

	mtDW::MathState		m_math;

	mtDW::FloatParser	m_parser_float;
	mtDW::IntegerParser	m_parser_integer;
	mtDW::RationalParser	m_parser_rational;

	QGroupBox		* m_group_box_float	= nullptr;
	QPlainTextEdit		* m_text_float_input	= nullptr;
	QPlainTextEdit		* m_text_float_output	= nullptr;

	QGroupBox		* m_group_box_int	= nullptr;
	QPlainTextEdit		* m_text_int_input	= nullptr;
	QPlainTextEdit		* m_text_int_output	= nullptr;

	QGroupBox		* m_group_box_rational	= nullptr;
	QPlainTextEdit		* m_text_rational_input	= nullptr;
	QPlainTextEdit		* m_text_rational_output = nullptr;
};



class DialogVars : public QDialog
{
	Q_OBJECT

public:
	DialogVars (
		MainWindow * mw,
		MemPrefs const & mprefs,
		char const * title,
		mtDW::FloatParser const * parser
		);

	DialogVars (
		MainWindow * mw,
		MemPrefs const & mprefs,
		char const * title,
		mtDW::IntegerParser const * parser
		);

	DialogVars (
		MainWindow * mw,
		MemPrefs const & mprefs,
		char const * title,
		mtDW::RationalParser const * parser
		);

private slots:
	void press_var (
		int currentRow,
		int currentColumn,
		int previousRow,
		int previousColumn
		);
	void press_save_var ();

private:
	void build_gui ();

/// ----------------------------------------------------------------------------

	MemPrefs	const	& m_mprefs;

	mtDW::FloatParser	const * const	m_float_parser = nullptr;
	mtDW::IntegerParser	const * const	m_integer_parser = nullptr;
	mtDW::RationalParser	const * const	m_rational_parser = nullptr;

	QGroupBox		* m_group_box_var	= nullptr;
	QTableWidget		* m_table_vars		= nullptr;
	QPlainTextEdit		* m_text_var		= nullptr;
};

