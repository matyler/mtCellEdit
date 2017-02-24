/*
	Copyright (C) 2013-2017 Mark Tyler

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



mtQEX::PrefsWindow::PrefsWindow (
	mtPrefs		* const	prefsMem,
	QString		const	title
	)
	:
	prefs		( prefsMem )
{
	int		wx = 50,
			wy = 50,
			ww = 500,
			wh = 500,
			i,
			cw
			;
	char		txt[ 256 ];
	QVBoxLayout	* vlayout;
	QHBoxLayout	* hRow;
	QPushButton	* button;
	QStringList	columnLabels;


	setWindowModality ( Qt::ApplicationModal );

	setWindowTitle ( title );

	mtkit_prefs_get_int ( prefs, "prefs.window_x", &wx );
	mtkit_prefs_get_int ( prefs, "prefs.window_y", &wy );
	mtkit_prefs_get_int ( prefs, "prefs.window_w", &ww );
	mtkit_prefs_get_int ( prefs, "prefs.window_h", &wh );

	setGeometry ( wx, wy, ww, wh );

	vlayout = new QVBoxLayout;
	setLayout ( vlayout );

	hRow = new QHBoxLayout ();
	vlayout->addLayout ( hRow );

	button = new QPushButton ( "&Filter" );
	button->setAutoDefault ( false );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( pressButtonFilter () ) );
	hRow->addWidget ( button );

	filterEdit = new QLineEdit ();
	hRow->addWidget ( filterEdit );
	connect ( filterEdit, SIGNAL ( returnPressed () ), this,
		SLOT ( pressButtonFilter () ) );

	tableWidget = new QTableWidget;
	vlayout->addWidget ( tableWidget );

	tableWidget->setSelectionMode ( QAbstractItemView::SingleSelection );
	tableWidget->setSelectionBehavior ( QAbstractItemView::SelectRows );
	tableWidget->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	tableWidget->setColumnCount ( 4 );
	tableWidget->setShowGrid ( false );

	tableWidget->verticalHeader ()->hide ();
	tableWidget->verticalHeader ()->setDefaultSectionSize (
		tableWidget->verticalHeader ()->fontMetrics ().height () + 4 );

	tableWidget->horizontalHeader ()->QEX_SETCLICKABLE ( false );
	tableWidget->horizontalHeader ()->setStretchLastSection ( true );

	columnLabels
		<< "Key"
		<< "Status"
		<< "Type"
		<< "Value"
		;
	tableWidget->setHorizontalHeaderLabels ( columnLabels );

	connect ( tableWidget, SIGNAL ( cellActivated ( int, int ) ),
		this, SLOT ( tableCellActivated ( int, int ) ) );

	connect ( tableWidget, SIGNAL( currentCellChanged (int, int, int, int)),
		this, SLOT ( tableCellChanged ( int, int, int, int ) ) );

	hRow = new QHBoxLayout ();
	vlayout->addLayout ( hRow );

	infoEdit = new QLineEdit ();
	hRow->addWidget ( infoEdit );
	infoEdit->setReadOnly ( true );

	button = new QPushButton ( QIcon::fromTheme ( "edit-clear" ),
		"&Reset" );
	button->setAutoDefault ( false );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( pressButtonReset () ) );
	hRow->addWidget ( button );
	buttonReset = button;

	button = new QPushButton ( QIcon::fromTheme ( "document-properties" ),
		"&Edit" );
	button->setAutoDefault ( false );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( pressButtonEdit () ) );
	hRow->addWidget ( button );
	buttonEdit = button;

	button = new QPushButton ( QIcon::fromTheme ( "window-close" ),
		"&Close" );
	button->setAutoDefault ( false );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( pressButtonClose () ) );
	hRow->addWidget ( button );

	filterEdit->setFocus ();
	show ();
	populateTable ();

	// Set column widths
	for ( i = 0; i < COLUMN_TOTAL - 1; i++ )
	{
		snprintf ( txt, sizeof ( txt ), "prefs.col%i", i + 1 );

		if ( mtkit_prefs_get_int ( prefs, txt, &cw ) )
		{
			continue;
		}

		if ( cw > 0 )
		{
			tableWidget->horizontalHeader ()->
				resizeSection ( i, cw );
		}
		else
		{
			tableWidget->resizeColumnToContents ( i );
		}
	}

	exec ();
}

mtQEX::PrefsWindow::~PrefsWindow ()
{
	int		i,
			cw;
	char		txt[ 256 ];


	mtkit_prefs_set_int ( prefs, "prefs.window_x", geometry().x () );
	mtkit_prefs_set_int ( prefs, "prefs.window_y", geometry().y () );
	mtkit_prefs_set_int ( prefs, "prefs.window_w", geometry().width () );
	mtkit_prefs_set_int ( prefs, "prefs.window_h", geometry().height () );

	// Set column widths
	for ( i = 0; i < COLUMN_TOTAL - 1; i++ )
	{
		snprintf ( txt, sizeof ( txt ), "prefs.col%i", i + 1 );

		cw = tableWidget->horizontalHeader ()-> sectionSize ( i );
		mtkit_prefs_set_int ( prefs, txt, cw );
	}
}

void mtQEX::PrefsWindow::pressButtonFilter ()
{
	populateTable ();
}

static mtPrefValue * get_piv (
	QTableWidget	* const	tableWidget,
	int		const	row
	)
{
	QTableWidgetItem * twItem;


	twItem = tableWidget->item ( row, COLUMN_KEY );

	if ( ! twItem )
	{
		return NULL;
	}

	return (mtPrefValue *)twItem->data( Qt::UserRole ).value<void *> ();
}

static void update_table_status_value (
	QTableWidget	* const	tableWidget,
	mtPrefValue	* const	piv,
	int		const	row
	)
{
	char	const	* st;
	char		buf[ 256 ] =  { 0 },
			* bp;
	QTableWidgetItem * twItem;


	if ( piv->def )
	{
		if ( strcmp ( piv->value, piv->def ) == 0 )
		{
			st = "default";
		}
		else
		{
			st = "user set";
		}
	}
	else		// Default is NULL
	{
		if ( piv->value[0] == 0 )
		{
			st = "default";
		}
		else
		{
			st = "user set";
		}
	}

	twItem = new QTableWidgetItem;
	twItem->setText ( mtQEX::qstringFromC ( st ) );
	tableWidget->setItem ( row, COLUMN_STATUS, twItem );

	mtkit_prefs_get_str_val ( piv, piv->value, buf, sizeof ( buf ) );

	// Remove any newlines
	while ( ( bp = strchr ( buf, '\n' ) ) )
	{
		bp[0] = ' ';
	}

	twItem = new QTableWidgetItem;
	twItem->setText ( mtQEX::qstringFromC ( buf ) );
	tableWidget->setItem ( row, COLUMN_VALUE, twItem );
}

void mtQEX::PrefsWindow::pressButtonReset ()
{
	int		row;
	mtPrefValue	* piv;


	row = tableWidget->currentRow ();
	piv = get_piv ( tableWidget, row );

	if ( ! piv )
	{
		return;
	}

	mtkit_prefs_set_default ( prefs, piv->key );
	update_table_status_value ( tableWidget, piv, row );
}

static int winget_int (
	mtQEX::PrefsWindow * const win,
	mtPrefs		* const	prefs,
	mtPrefValue	* const	piv
	)
{
	bool		ok;
	int		num, val = 0;
	double		min = -1000000, max = 1000000;


	if ( piv->opt )
	{
		mtkit_strtok_num ( piv->opt, "\t", 0, &min );
		mtkit_strtok_num ( piv->opt, "\t", 1, &max );
	}

	mtkit_prefs_get_int ( prefs, piv->key, &val );

	num = QInputDialog::getInt ( win, "Edit Integer",
		piv->key, val, (int)min, (int)max, 1, &ok );

	if ( ok )
	{
		mtkit_prefs_set_int ( prefs, piv->key, num );

		return 1;		// Changed
	}

	return 0;			// No change
}

static int winget_bool (
	mtQEX::PrefsWindow * const win,
	mtPrefs		* const	prefs,
	mtPrefValue	* const	piv
	)
{
	bool		ok;
	int		val = 0;
	QString		res;
	QStringList	items;


	mtkit_prefs_get_int ( prefs, piv->key, &val );

	items << "False" << "True";

	res = QInputDialog::getItem ( win, "Edit Boolean",
		piv->key, items, val, false, &ok );

	if ( ok && ! res.isEmpty () )
	{
		int		num;


		num = items.indexOf ( res, 0 );
		mtkit_prefs_set_int ( prefs, piv->key, num );

		return 1;		// Changed
	}

	return 0;			// No change
}

static int winget_option (
	mtQEX::PrefsWindow * const win,
	mtPrefs		* const	prefs,
	mtPrefValue	* const	piv
	)
{
	bool		ok;
	int		val = 0, i;
	char		* txt;
	QString		res;
	QStringList	items;


	if ( ! piv->opt )
	{
		return 0;		// No change
	}

	mtkit_prefs_get_int ( prefs, piv->key, &val );

	for ( i = 0; ; i++ )
	{
		txt = mtkit_strtok ( piv->opt, "\t", i );
		if ( ! txt )
		{
			break;		// End of list
		}

		items << txt;

		free ( txt );
	}

	res = QInputDialog::getItem ( win, "Edit Option", piv->key, items,
		val, false, &ok );

	if ( ok && ! res.isEmpty () )
	{
		int		num;


		num = items.indexOf ( res, 0 );
		mtkit_prefs_set_int ( prefs, piv->key, num );

		return 1;		// Changed
	}


	return 0;			// No change
}

static int winget_double (
	mtQEX::PrefsWindow * const win,
	mtPrefs		* const	prefs,
	mtPrefValue	* const	piv
	)
{
	bool		ok;
	double		min = -1000000, max = 1000000, val = 0, dp = 3, num;


	if ( piv->opt )
	{
		double		a = 0, b = 0, c = 0;


		mtkit_strtok_num ( piv->opt, "\t", 0, &a );
		mtkit_strtok_num ( piv->opt, "\t", 1, &b );
		mtkit_strtok_num ( piv->opt, "\t", 2, &c );

		if ( b > a )
		{
			// User is setting min/max in first 2 args
			min = a;
			max = b;

			if ( c > 0 && c < 15 )
			{
				// User is setting decimal places in 3rd arg
				dp = c;
			}
		}
		else
		{
			if ( a > 0 && a < 15 )
			{
				// User is setting decimal places in 1st arg
				dp = a;
			}
		}
	}

	mtkit_prefs_get_double ( prefs, piv->key, &val );

	num = QInputDialog::getDouble ( win, "Edit Decimal", piv->key, val,
		min, max, (int)dp, &ok );

	if ( ok )
	{
		mtkit_prefs_set_double ( prefs, piv->key, num );

		return 1;		// Changed
	}

	return 0;			// No change
}

static int winget_string (
	mtQEX::PrefsWindow * const win,
	mtPrefs		* const	prefs,
	mtPrefValue	* const	piv
	)
{
	double		maxLen = -1;


	if ( piv->opt )
	{
		mtkit_strtok_num ( piv->opt, "\t", 0, &maxLen );
	}

	QString text = mtQEX::dialogTextLine ( win, "Edit Text", piv->key,
		piv->value, (int)maxLen );

	if ( ! text.isEmpty () )
	{
		mtkit_prefs_set_str ( prefs, piv->key, text.toUtf8 ().data () );

		return 1;		// Changed
	}

	return 0;			// No change
}

static int winget_string_multi (
	mtQEX::PrefsWindow * const win,
	mtPrefs		* const	prefs,
	mtPrefValue	* const	piv
	)
{
	QString text = mtQEX::dialogText ( win, "Edit Text", piv->key,
		piv->value );


	if ( ! text.isEmpty () )
	{
		mtkit_prefs_set_str ( prefs, piv->key, text.toUtf8 ().data () );

		return 1;		// Changed
	}

	return 0;			// No change
}

static int winget_rgb (
	mtQEX::PrefsWindow * const win,
	mtPrefs		* const	prefs,
	mtPrefValue	* const	piv
	)
{
	int		colRGB = 0;
	QColor		color;


	mtkit_prefs_get_int ( prefs, piv->key, &colRGB );

	color = QColorDialog::getColor ( QColor (
			mtPixy::int_2_red ( colRGB ),
			mtPixy::int_2_green ( colRGB ),
			mtPixy::int_2_blue ( colRGB )  ),
		win, piv->key, QColorDialog::DontUseNativeDialog );

	if ( color.isValid () )
	{
		colRGB = mtPixy::rgb_2_int ( color.red (), color.green (),
			color.blue () );

		mtkit_prefs_set_int ( prefs, piv->key, colRGB );

		return 1;		// Changed
	}

	return 0;			// No change
}

static int winget_directory (
	mtQEX::PrefsWindow * const win,
	mtPrefs		* const	prefs,
	mtPrefValue	* const	piv
	)
{
	QString filename = QFileDialog::getExistingDirectory ( win,
		piv->key, piv->value,
		QFileDialog::DontUseNativeDialog | QFileDialog::ShowDirsOnly );


	if ( ! filename.isEmpty () )
	{
		mtkit_prefs_set_str ( prefs, piv->key,
			filename.toUtf8 ().data () );

		return 1;		// Changed
	}

	return 0;			// No change
}

static int winget_file (
	mtQEX::PrefsWindow * const win,
	mtPrefs		* const	prefs,
	mtPrefValue	* const	piv
	)
{
	QString filename = QFileDialog::getOpenFileName ( win, piv->key,
		piv->value, NULL, NULL, QFileDialog::DontUseNativeDialog );


	if ( ! filename.isEmpty () )
	{
		mtkit_prefs_set_str ( prefs, piv->key,
			filename.toUtf8 ().data () );

		return 1;		// Changed
	}

	return 0;			// No change
}

void mtQEX::PrefsWindow::pressButtonEdit ()
{
	int		row, change = 0;
	mtPrefValue	* piv;


	row = tableWidget->currentRow ();
	piv = get_piv ( tableWidget, row );

	if ( ! piv )
	{
		return;
	}

	switch ( piv->type )
	{
	case MTKIT_PREF_TYPE_INT:
		change = winget_int ( this, prefs, piv );
		break;

	case MTKIT_PREF_TYPE_BOOL:
		change = winget_bool ( this, prefs, piv );
		break;

	case MTKIT_PREF_TYPE_OPTION:
		change = winget_option ( this, prefs, piv );
		break;

	case MTKIT_PREF_TYPE_DOUBLE:
		change = winget_double ( this, prefs, piv );
		break;

	case MTKIT_PREF_TYPE_STR:
		change = winget_string ( this, prefs, piv );
		break;

	case MTKIT_PREF_TYPE_STR_MULTI:
		change = winget_string_multi ( this, prefs, piv );
		break;

	case MTKIT_PREF_TYPE_RGB:
		change = winget_rgb ( this, prefs, piv );
		break;

	case MTKIT_PREF_TYPE_DIR:
		change = winget_directory ( this, prefs, piv );
		break;

	case MTKIT_PREF_TYPE_FILE:
		change = winget_file ( this, prefs, piv );
		break;

	default:
		break;
	}

	if ( change )
	{
		update_table_status_value ( tableWidget, piv, row );
	}
}

void mtQEX::PrefsWindow::pressButtonClose ()
{
	close ();
}

static void populate_recurse (
	QTableWidget	* const	tableWidget,
	mtTreeNode	* const	node,
	char	const	* const	filter
	)
{
	mtPrefValue	* piv;


	if ( ! node )
	{
		return;
	}

	populate_recurse ( tableWidget, node->left, filter );
	piv = (mtPrefValue *)node->data;

	if (	piv &&
		strncmp ( piv->key, "prefs.", 6 ) &&
		( ! filter || mtkit_strcasestr ( piv->key, filter ) )
		)
	{
		int		row;
		QTableWidgetItem * twItem;
		QString		qs = mtQEX::qstringFromC( piv->description );


		row = tableWidget->rowCount ();
		tableWidget->setRowCount ( row + 1 );

		twItem = new QTableWidgetItem;
		twItem->setText ( mtQEX::qstringFromC (piv->key ));
		tableWidget->setItem ( row, COLUMN_KEY, twItem );
		twItem->setData ( Qt::UserRole,
			QVariant::fromValue ( (void *)piv ) );
		twItem->setToolTip ( qs );

		twItem = new QTableWidgetItem;
		twItem->setText ( mtQEX::qstringFromC (
			mtkit_prefs_type_text ( piv->type ) ) );
		tableWidget->setItem ( row, COLUMN_TYPE, twItem );

		update_table_status_value ( tableWidget, piv, row );
	}

	populate_recurse ( tableWidget, node->right, filter );
}

void mtQEX::PrefsWindow::populateTable ()
{
	mtTree		* tree;


	tree = mtkit_prefs_get_tree ( prefs );

	if ( tree )
	{
		tableWidget->clearContents ();
		tableWidget->setRowCount ( 0 );

		populate_recurse ( tableWidget, tree->root,
			filterEdit->text ().toUtf8 ().data () );

		tableWidget->setCurrentCell ( 0, 0 );
	}

	if ( tableWidget->rowCount () < 1 )
	{
		buttonReset->setEnabled ( false );
		buttonEdit->setEnabled ( false );
	}
	else
	{
		buttonReset->setEnabled ( true );
		buttonEdit->setEnabled ( true );
	}
}

void mtQEX::PrefsWindow::tableCellActivated (
	int	const	ARG_UNUSED ( row ),
	int	const	ARG_UNUSED ( column )
	)
{
	pressButtonEdit ();
}

void mtQEX::PrefsWindow::tableCellChanged (
	int	const	currentRow,
	int	const	ARG_UNUSED ( currentColumn ),
	int	const	ARG_UNUSED ( previousRow ),
	int	const	ARG_UNUSED ( previousColumn )
	)
{
	mtPrefValue	* piv;


	piv = get_piv ( tableWidget, currentRow );

	if ( piv )
	{
		QString qs = mtQEX::qstringFromC( piv->description );


		infoEdit->setText ( qs );
		infoEdit->setToolTip ( qs );
	}
}

