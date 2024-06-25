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
void populate_vars (
	Tparser			& parser,
	QTableWidget	* const	table
	)
{
	auto	const	& vars = parser.variables ();
	int		row = 0;

	for ( auto && it : vars )
	{
		QTableWidgetItem * const twItem = new QTableWidgetItem;

		table->setRowCount ( row + 1 );

		twItem->setText ( it.first.c_str() );
		table->setItem ( row++, 0, twItem );
	}

	// This hack ensures that all columns are right width (not just visible)
	table->setVisible ( false );
	table->resizeColumnsToContents ();
	table->setVisible ( true );

	table->setCurrentCell ( 0, 0 );
}

template < typename Tparser, typename Tnum >
int select_vars (
	Tparser			& parser,
	QTableWidget	* const	table,
	QPlainTextEdit	* const	text_edit,
	int		const	row,
	int		const	old_row,
	int		const	snip_size,
	Tnum		const	*& num,
	int			& num_digits
	)
{
	if ( row == old_row )
	{
		return 1;
	}

	QTableWidgetItem * twItem = table->item ( row, 0 );
	if ( ! twItem )
	{
		return 1;
	}

	std::string const var = twItem->text().toStdString();

	auto	const	& map = parser.variables();
	auto	const	it = map.find ( var );

	if ( it == map.end() )
	{
		text_edit->setPlainText ( "" );
		return 1;
	}

	num = &it->second;
	std::string const digits = num->to_string_snip ((size_t)snip_size);
	num_digits = (int)digits.size();

	text_edit->setPlainText ( digits.c_str() );

	return 0;
}

template < typename Tparser, typename Tnum >
int get_variable_and_filename (
	QWidget		* const	parent,
	Tparser		const	& parser,
	Tnum		const	*& var,
	QTableWidget	* const	table,
	std::string		& filename,
	char	const	* const	message
	)
{
	int const row = table->currentRow();

	if ( row < 0 )
	{
		// No current row
		return 1;
	}

	QTableWidgetItem * twItem = table->item ( row, 0 );
	if ( ! twItem )
	{
		// No row of this index
		return 1;
	}

	std::string const var_name = twItem->text().toStdString();

	auto	const	& map = parser.variables();
	auto	const	it = map.find ( var_name );

	if ( it == map.end() )
	{
		// Variable not found
		return 1;
	}

	var = &it->second;

	filename = choose_output_filename ( parent, message );

	return 0;
}



/// ----------------------------------------------------------------------------



void MainWindow::press_show_float_vars ()
{
	DialogVars dialog ( this, mprefs, "Float Variables", &m_parser_float );
}

void MainWindow::press_show_double_vars ()
{
	DialogVars dialog ( this, mprefs, "Double Variables", &m_parser_double );
}

void MainWindow::press_show_int_vars ()
{
	DialogVars dialog ( this, mprefs, "Integer Variables",
		&m_parser_integer );
}

void MainWindow::press_show_rational_vars ()
{
	DialogVars dialog ( this, mprefs, "Rational Variables",
		&m_parser_rational );
}



/// ----------------------------------------------------------------------------



DialogVars::DialogVars (
	MainWindow			* const	mw,
	MemPrefs			const	& mprefs,
	char			const * const	title,
	mtDW::FloatParser	const * const	parser
	)
	:
	QDialog			( mw ),
	m_mprefs		( mprefs ),
	m_float_parser		( parser )
{
	setWindowTitle ( title );

	build_gui ();
}

DialogVars::DialogVars (
	MainWindow			* const	mw,
	MemPrefs			const	& mprefs,
	char			const * const	title,
	mtDW::IntegerParser	const * const	parser
	)
	:
	QDialog			( mw ),
	m_mprefs		( mprefs ),
	m_integer_parser	( parser )
{
	setWindowTitle ( title );

	build_gui ();
}

DialogVars::DialogVars (
	MainWindow			* const	mw,
	MemPrefs			const	& mprefs,
	char			const * const	title,
	mtDW::RationalParser	const * const	parser
	)
	:
	QDialog			( mw ),
	m_mprefs		( mprefs ),
	m_rational_parser	( parser )
{
	setWindowTitle ( title );

	build_gui ();
}

void DialogVars::build_gui ()
{
	setModal ( true );

	QVBoxLayout * vbox = new QVBoxLayout;
	setLayout ( vbox );

	QWidget		* w;
	QVBoxLayout	* sgvb;
	QGroupBox	* groupBox;
	QGridLayout	* grid;
	QTableWidget	* table;
	QStringList	var_column_labels;

	var_column_labels << "Variable";

	groupBox = new QGroupBox ( "Variables" );
	vbox->addWidget ( groupBox );
	m_group_box_var = groupBox;

	sgvb = new QVBoxLayout;
	groupBox->setLayout ( sgvb );

	w = new QWidget;
	sgvb->addWidget ( w );

	grid = new QGridLayout;
	w->setLayout ( grid );

	table = new QTableWidget;
	grid->addWidget ( table, 0, 0 );
	m_table_vars = table;

	table->setSelectionMode ( QAbstractItemView::SingleSelection );
	table->setSelectionBehavior ( QAbstractItemView::SelectRows );
	table->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	table->setColumnCount ( 1 );
	table->setShowGrid ( false );
	table->verticalHeader ()->setSectionResizeMode( QHeaderView::Fixed );
	table->verticalHeader ()->setSectionsClickable ( false );

	table->horizontalHeader ()->resizeSections (
		QHeaderView::ResizeToContents );
	table->horizontalHeader ()->setStretchLastSection ( true );
	table->horizontalHeader ()->setSectionsClickable ( false );

	connect ( table, SIGNAL( currentCellChanged ( int, int, int, int )),
		this, SLOT ( press_var ( int, int, int, int ) ) );

	table->setHorizontalHeaderLabels ( var_column_labels );
	table->horizontalHeader ()->resizeSections (
		QHeaderView::ResizeToContents );

	m_text_var = new QPlainTextEdit;
	grid->addWidget ( m_text_var, 0, 1 );

/// ----------------------------------------------------------------------------

	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Close | QDialogButtonBox::Save );

	QPushButton * const button_var_save = button_box->button (
		QDialogButtonBox::Save );
	vbox->addWidget ( button_box );

	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));
	connect ( button_var_save, SIGNAL ( clicked () ), this,
		SLOT( press_save_var() ) );

	if ( m_float_parser )
	{
		populate_vars ( *m_float_parser, m_table_vars );
	}
	else if ( m_integer_parser )
	{
		populate_vars ( *m_integer_parser, m_table_vars );
	}
	else if ( m_rational_parser )
	{
		populate_vars ( *m_rational_parser, m_table_vars );
	}

	if ( m_table_vars->rowCount() > 0 )
	{
		button_var_save->setEnabled ( true );
	}
	else
	{
		button_var_save->setEnabled ( false );
	}

	resize ( 800, 600 );

	exec ();
}

void DialogVars::press_save_var ()
{
	std::string		filename;

	if ( m_float_parser )
	{
		mtDW::Float	const	* var;

		if ( 0 == get_variable_and_filename ( this, *m_float_parser,
			var, m_table_vars, filename, "Save Float number" )
			)
		{
			save_a_number ( this, *var, filename );
		}
	}
	else if ( m_integer_parser )
	{
		mtDW::Integer	const	* var;

		if ( 0 == get_variable_and_filename ( this, *m_integer_parser,
			var, m_table_vars, filename, "Save Integer number" )
			)
		{
			save_a_number ( this, *var, filename );
		}
	}
	else if ( m_rational_parser )
	{
		mtDW::Rational	const	* var;

		if ( 0 == get_variable_and_filename ( this, *m_rational_parser,
			var, m_table_vars, filename, "Save Rational number" )
			)
		{
			save_a_number ( this, *var, filename );
		}
	}
}

void DialogVars::press_var (
	int	const	currentRow,
	int	const	ARG_UNUSED ( currentColumn ),
	int	const	previousRow,
	int	const	ARG_UNUSED ( previousColumn )
	)
{
	std::string	str;
	int		digits;

	if ( m_float_parser )
	{
		mtDW::Float const * num;

		if ( 0 == select_vars ( *m_float_parser, m_table_vars,
			m_text_var, currentRow, previousRow,
			m_mprefs.calc_snip_size, num, digits )
			)
		{
			str += "Float Variables ";
			str += print_digits_label ( *num, digits );
		}
	}
	else if ( m_integer_parser )
	{
		mtDW::Integer const * num;

		if ( 0 == select_vars ( *m_integer_parser, m_table_vars,
			m_text_var, currentRow, previousRow,
			m_mprefs.calc_snip_size, num, digits )
			)
		{
			str += "Integer Variables ";
			str += print_digits_label ( *num, digits );
		}
	}
	else if ( m_rational_parser )
	{
		mtDW::Rational const * num;

		if ( 0 == select_vars ( *m_rational_parser, m_table_vars,
			m_text_var, currentRow, previousRow,
			m_mprefs.calc_snip_size, num, digits )
			)
		{
			str += "Rational Variables ";
			str += print_digits_label ( *num, digits );
		}
	}

	m_group_box_var->setTitle ( str.c_str() );
}

