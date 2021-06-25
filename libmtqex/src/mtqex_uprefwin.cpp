/*
	Copyright (C) 2013-2020 Mark Tyler

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

#include "private.h"



enum
{
	COLUMN_KEY		= 0,
	COLUMN_STATUS		= 1,
	COLUMN_TYPE		= 2,
	COLUMN_VALUE		= 3,

	COLUMN_TOTAL		= 4
};



UPrefsWindow::UPrefsWindow (
	QWidget		* const	parent,
	mtKit::UserPrefs	& prefs,
	QString		const	title
	)
	:
	QDialog		( parent ),
	m_prefs		( prefs )
{
	setWindowModality ( Qt::ApplicationModal );

	setWindowTitle ( title );

	setGeometry ( m_prefs.get_int ("prefs.window_x"),
		m_prefs.get_int ("prefs.window_y"),
		m_prefs.get_int ("prefs.window_w"),
		m_prefs.get_int ("prefs.window_h")
		);

	QVBoxLayout	* vlayout;
	QHBoxLayout	* hRow;
	QPushButton	* button;

	vlayout = new QVBoxLayout;
	setLayout ( vlayout );

	hRow = new QHBoxLayout ();
	vlayout->addLayout ( hRow );

	button = new QPushButton ( "&Filter" );
	button->setAutoDefault ( false );
	connect( button, &QPushButton::clicked, [this]() { populateTable (); });
	hRow->addWidget ( button );

	m_filter_edit = new QLineEdit ();
	hRow->addWidget ( m_filter_edit );
	connect ( m_filter_edit, &QLineEdit::returnPressed,
		[this]() { populateTable (); } );

	m_table_widget = new QTableWidget;
	vlayout->addWidget ( m_table_widget );

	m_table_widget->setSelectionMode ( QAbstractItemView::SingleSelection );
	m_table_widget->setSelectionBehavior ( QAbstractItemView::SelectRows );
	m_table_widget->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	m_table_widget->setColumnCount ( 4 );
	m_table_widget->setShowGrid ( false );

	m_table_widget->verticalHeader ()->hide ();

	m_table_widget->horizontalHeader ()->setSectionsClickable ( false );
	m_table_widget->horizontalHeader ()->setStretchLastSection ( true );

	QStringList	columnLabels;

	columnLabels << "Key" << "Status" << "Type" << "Value";
	m_table_widget->setHorizontalHeaderLabels ( columnLabels );

	connect ( m_table_widget, &QTableWidget::cellActivated,
		[this]( int, int ) { pressButtonEdit (); } );

	connect ( m_table_widget, &QTableWidget::currentCellChanged,
		[this] ( int, int, int, int )
		{
			int const row = m_table_widget->currentRow ();
			std::string const key ( get_key( row ) );
			std::string const desc ( m_prefs.get_description (
				key.c_str() ) );

			m_info_edit->setText ( desc.c_str () );
			m_info_edit->setToolTip ( desc.c_str () );
		}
		);

	hRow = new QHBoxLayout ();
	vlayout->addLayout ( hRow );

	m_info_edit = new QLineEdit ();
	hRow->addWidget ( m_info_edit );
	m_info_edit->setReadOnly ( true );

	button = new QPushButton ( QIcon::fromTheme ( "edit-clear" ),
		"&Reset" );
	button->setAutoDefault ( false );
	connect ( button, &QPushButton::clicked, [this]()
		{
			int const row = m_table_widget->currentRow ();
			std::string const key ( get_key( row ) );

			m_prefs.set_default_value ( key.c_str() );

			update_table_status_value ( row );
		}
		);

	hRow->addWidget ( button );
	m_button_reset = button;

	button = new QPushButton ( QIcon::fromTheme ( "document-properties" ),
		"&Edit" );
	button->setAutoDefault ( false );
	connect( button, &QPushButton::clicked, [this](){ pressButtonEdit(); });
	hRow->addWidget ( button );
	m_button_edit = button;

	button = new QPushButton( QIcon::fromTheme ("window-close"), "&Close" );
	button->setAutoDefault ( false );
	connect ( button, &QPushButton::clicked, [this](){ close (); } );
	hRow->addWidget ( button );

	m_filter_edit->setFocus ();
	show ();
	populateTable ();

	// Set column widths
	for ( int i = 0; i < COLUMN_TOTAL - 1; i++ )
	{
		char txt[ 256 ];
		snprintf ( txt, sizeof ( txt ), "prefs.col%i", i + 1 );

		int const cw = m_prefs.get_int ( txt );

		if ( cw > 0 )
		{
			m_table_widget->horizontalHeader ()->
				resizeSection ( i, cw );
		}
		else
		{
			m_table_widget->resizeColumnToContents ( i );
		}
	}

	exec ();
}

UPrefsWindow::~UPrefsWindow ()
{
	m_prefs.set ( "prefs.window_x", geometry().x () );
	m_prefs.set ( "prefs.window_y", geometry().y () );
	m_prefs.set ( "prefs.window_w", geometry().width () );
	m_prefs.set ( "prefs.window_h", geometry().height () );

	// Set column widths
	for ( int i = 0; i < COLUMN_TOTAL - 1; i++ )
	{
		char txt[ 256 ];
		snprintf ( txt, sizeof ( txt ), "prefs.col%i", i + 1 );

		int const cw = m_table_widget->horizontalHeader()->
			sectionSize(i);

		m_prefs.set ( txt, cw );
	}
}

std::string UPrefsWindow::get_key (
	int	const	row,
	int	* const	type
	) const
{
	QTableWidgetItem * const twItem = m_table_widget->item(row, COLUMN_KEY);

	if ( ! twItem )
	{
		if ( type )
		{
			*type = mtKit::PrefType::ERROR;
		}

		return "";
	}

	if ( type )
	{
		*type = twItem->data( Qt::UserRole ).value<int>();
	}

	return twItem->text ().toUtf8 ().data ();
}

void UPrefsWindow::update_table_status_value ( int const row )
{
	std::string const key ( get_key (row) );

	if ( key.size() < 1 )
	{
		return;
	}

	QTableWidgetItem * twItem;
	std::string	txt;

	if ( m_prefs.is_default ( key.c_str() ) )
	{
		txt = "default";
	}
	else
	{
		txt = "user set";
	}

	twItem = new QTableWidgetItem;
	twItem->setText ( mtQEX::qstringFromC ( txt.c_str() ) );
	m_table_widget->setItem ( row, COLUMN_STATUS, twItem );

	txt = m_prefs.get_ui_string ( key.c_str() );

	// Remove any newlines
	for ( size_t i = 0; i < txt.size (); i++ )
	{
		if ( '\n' == txt[i] )
		{
			txt[i] = ' ';
		}
	}

	twItem = new QTableWidgetItem;
	twItem->setText ( mtQEX::qstringFromC ( txt.c_str() ) );
	m_table_widget->setItem ( row, COLUMN_VALUE, twItem );
}

static int winget_int (
	UPrefsWindow	* const win,
	mtKit::UserPrefs	& prefs,
	std::string	const	& key
	)
{
	bool		ok = false;
	int	const	val = prefs.get_int ( key.c_str() );

	int		min = 0, max = 0;
	prefs.get_int_range ( key.c_str(), min, max );

	if ( min >= max )
	{
		min = -1000000;
		max = 1000000;
	}

	int const num = QInputDialog::getInt ( win, "Edit Integer",
		key.c_str(), val, min, max, 1, &ok );

	if ( ok )
	{
		prefs.set ( key.c_str(), num );

		return 1;		// Changed
	}

	return 0;			// No change
}

static int winget_bool (
	UPrefsWindow	* const win,
	mtKit::UserPrefs	& prefs,
	std::string	const	& key
	)
{
	int const val = prefs.get_int ( key.c_str() );

	QStringList	items;
	items << "False" << "True";

	bool		ok = false;
	QString const res = QInputDialog::getItem ( win, "Edit Boolean",
		key.c_str(), items, val, false, &ok );

	if ( ok && ! res.isEmpty () )
	{
		int const num = items.indexOf ( res, 0 );
		prefs.set ( key.c_str(), num );

		return 1;		// Changed
	}

	return 0;			// No change
}

static int winget_rgb (
	UPrefsWindow	* const win,
	mtKit::UserPrefs	& prefs,
	std::string	const	& key
	)
{
	int colRGB = prefs.get_int ( key.c_str() );
	QColor const color = QColorDialog::getColor ( QColor (
			pixy_int_2_red ( colRGB ),
			pixy_int_2_green ( colRGB ),
			pixy_int_2_blue ( colRGB )  ),
		win, key.c_str(), QColorDialog::DontUseNativeDialog );

	if ( color.isValid () )
	{
		colRGB = pixy_rgb_2_int ( color.red (), color.green (),
			color.blue () );

		prefs.set ( key.c_str(), colRGB );

		return 1;		// Changed
	}

	return 0;			// No change
}

static int winget_option (
	UPrefsWindow	* const win,
	mtKit::UserPrefs	& prefs,
	std::string	const	& key
	)
{
	int const val = prefs.get_int ( key.c_str() );

	QStringList	items;
	prefs.scan_options ( key.c_str(), [&items]( std::string const & txt )
		{
			items << txt.c_str();
		});

	bool		ok = false;
	QString	const	res = QInputDialog::getItem ( win, "Edit Option",
				key.c_str(), items, val, false, &ok );

	if ( ok && ! res.isEmpty () )
	{
		int const num = items.indexOf ( res, 0 );
		prefs.set ( key.c_str(), num );

		return 1;		// Changed
	}


	return 0;			// No change
}

static int winget_double (
	UPrefsWindow	* const win,
	mtKit::UserPrefs	& prefs,
	std::string	const	& key
	)
{
	double		min = 0, max = 0;
	prefs.get_double_range ( key.c_str(), min, max );

	if ( min >= max )
	{
		min = -1000000;
		max = 1000000;
	}

	bool		ok = false;
	double	const	val = prefs.get_double ( key.c_str() );
	double	const	num = QInputDialog::getDouble ( win, "Edit Decimal",
		key.c_str(), val, min, max, 5, &ok );

	if ( ok )
	{
		prefs.set ( key.c_str(), num );

		return 1;		// Changed
	}

	return 0;			// No change
}

static int winget_string (
	UPrefsWindow	* const win,
	mtKit::UserPrefs	& prefs,
	std::string	const	& key
	)
{
	size_t const max = prefs.get_string_max ( key.c_str() );

	QString text;

	if ( 0 == mtQEX::dialogTextLine ( win, "Edit Text", key.c_str(),
		prefs.get_string ( key ).c_str(), (int)max, text ) )
	{
		prefs.set ( key.c_str(), text.toUtf8 ().data () );

		return 1;		// Changed
	}

	return 0;			// No change
}

static int winget_string_multi (
	UPrefsWindow	* const win,
	mtKit::UserPrefs	& prefs,
	std::string	const	& key
	)
{
	QString text;

	if ( 0 == mtQEX::dialogText ( win, "Edit Text", key.c_str(),
		prefs.get_string ( key ).c_str(), text ) )
	{
		prefs.set ( key.c_str(), text.toUtf8 ().data () );

		return 1;		// Changed
	}

	return 0;			// No change
}

static int winget_directory (
	UPrefsWindow	* const win,
	mtKit::UserPrefs	& prefs,
	std::string	const	& key
	)
{
	QString const filename = QFileDialog::getExistingDirectory ( win,
		key.c_str(), prefs.get_string ( key ).c_str(),
		QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly );

	if ( ! filename.isEmpty () )
	{
		prefs.set ( key.c_str(), filename.toUtf8 ().data () );

		return 1;		// Changed
	}

	return 0;			// No change
}

static int winget_file (
	UPrefsWindow	* const win,
	mtKit::UserPrefs	& prefs,
	std::string	const	& key
	)
{
	QString const filename = QFileDialog::getOpenFileName( win, key.c_str(),
		prefs.get_string ( key ).c_str(), NULL, NULL,
		QFileDialog::DontUseNativeDialog );

	if ( ! filename.isEmpty () )
	{
		prefs.set ( key.c_str(), filename.toUtf8 ().data () );

		return 1;		// Changed
	}

	return 0;			// No change
}

void UPrefsWindow::pressButtonEdit ()
{
	int	const	row = m_table_widget->currentRow ();
	int		change = 0;
	int		type = mtKit::PrefType::ERROR;

	std::string const key ( get_key (row, &type) );

	switch ( type )
	{
	case mtKit::PrefType::INT:
		change = winget_int ( this, m_prefs, key );
		break;

	case mtKit::PrefType::BOOL:
		change = winget_bool ( this, m_prefs, key );
		break;

	case mtKit::PrefType::RGB:
		change = winget_rgb ( this, m_prefs, key );
		break;

	case mtKit::PrefType::OPTION:
		change = winget_option ( this, m_prefs, key );
		break;

	case mtKit::PrefType::DOUBLE:
		change = winget_double ( this, m_prefs, key );
		break;

	case mtKit::PrefType::STRING:
		change = winget_string ( this, m_prefs, key );
		break;

	case mtKit::PrefType::STRING_MULTI:
		change = winget_string_multi ( this, m_prefs, key );
		break;

	case mtKit::PrefType::FILENAME:
		change = winget_file ( this, m_prefs, key );
		break;

	case mtKit::PrefType::DIRECTORY:
		change = winget_directory ( this, m_prefs, key );
		break;

	default:
		break;
	}

	if ( change )
	{
		update_table_status_value ( row );
	}
}

void UPrefsWindow::populateTable ()
{
	m_table_widget->clearContents ();
	m_table_widget->setRowCount ( 0 );

	std::string const filter ( m_filter_edit->text ().toUtf8 ().data () );

	m_prefs.scan_prefs ( [this, &filter](
			mtKit::PrefType	const	type,
			std::string	const & key,
			std::string	const & type_name,
			std::string	const & ARG_UNUSED(var_value),
			bool		const	ARG_UNUSED(var_default)
		)
		{
			if ( filter.size() > 0 && ! mtkit_strcasestr (
				key.c_str (), filter.c_str () )
				)
			{
				return;
			}

			std::string const description ( m_prefs.get_description(
				key.c_str () ) );

			int const row = m_table_widget->rowCount ();

			m_table_widget->setRowCount ( row + 1 );

			QTableWidgetItem * twItem = new QTableWidgetItem;
			twItem->setText ( mtQEX::qstringFromC (key.c_str()) );
			twItem->setData ( Qt::UserRole,
				QVariant::fromValue ( (int)type ) );
			twItem->setToolTip ( description.c_str() );
			m_table_widget->setItem ( row, COLUMN_KEY, twItem );

			twItem = new QTableWidgetItem;
			twItem->setText(mtQEX::qstringFromC(type_name.c_str()));
			m_table_widget->setItem ( row, COLUMN_TYPE, twItem );

			update_table_status_value ( row );
		});

	m_table_widget->setCurrentCell ( 0, 0 );

	if ( m_table_widget->rowCount () < 1 )
	{
		m_button_reset->setEnabled ( false );
		m_button_edit->setEnabled ( false );
	}
	else
	{
		m_button_reset->setEnabled ( true );
		m_button_edit->setEnabled ( true );
	}
}

void mtQEX::prefs_window (
	QWidget	* const	parent,
	mtKit::UserPrefs & prefs,
	QString	const	& title
	)
{
	UPrefsWindow ( parent, prefs, title );
}

