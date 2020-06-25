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

#include "qt4.h"



void Mainwindow::press_butt_info ()
{
	DialogButtInfo ( *this );
}



ButtAddThread::ButtAddThread (
	mtQEX::BusyDialog	&busy,
	Mainwindow		&mw,
	int		const	tot
	)
	:
	m_busy		( busy.get_busy () ),
	m_well		( mw.backend.db.get_well () ),
	m_butt		( mw.backend.db.get_butt () ),
	m_total		( tot ),
	m_error		( 0 )
{
}

void ButtAddThread::run ()
{
	m_busy->set_minmax ( 0, m_total );

	for ( int i = 0; i < m_total; i++ )
	{
		if ( m_busy->aborted () )
		{
			break;
		}

		m_busy->set_value ( i );

		m_error = m_butt->add_buckets ( m_well, 1 );
	}
}



#define LABEL_COL	1
#define INFO_COL	2
#define BUTTON_COL	3



DialogButtInfo::DialogButtInfo (
	Mainwindow	&mw
	)
	:
	QDialog		( &mw ),
	m_otp_list	(),
	m_otp_name	(),
	m_otp_total	(),
	m_otp_comment	(),
	m_read_only	(),
	m_bucket_total	(),
	m_bucket_used	(),
	m_bucket_pos	(),
	m_otp_delete	(),
	m_edit_comment	(),
	m_bucket_add	(),
	m_bucket_empty	(),
	mainwindow	( mw )
{
	QGroupBox	* groupBox;
	QVBoxLayout	* gvb;
	QHBoxLayout	* hbox;
	QGridLayout	* grid;
	QPushButton	* button;
	QWidget		* w;
	QWidget		* tab;
	QVBoxLayout	* vbox;

	setWindowTitle ( "Butt Information" );
	setModal ( true );

	QVBoxLayout * vbox_main = new QVBoxLayout;
	setLayout ( vbox_main );

	QTabWidget * tabWidget = new QTabWidget;
	vbox_main->addWidget ( tabWidget );

	tab = new QWidget;
	tabWidget->addTab ( tab, "Active" );

	vbox = new QVBoxLayout;
	tab->setLayout ( vbox );

/// ----------------------------------------------------------------------------

	groupBox = new QGroupBox ( "OTP" );
	vbox->addWidget ( groupBox );
	groupBox->setSizePolicy ( QSizePolicy::Preferred,
		QSizePolicy::Fixed );

	gvb = new QVBoxLayout;

	groupBox->setLayout ( gvb );

	w = new QWidget;
	gvb->addWidget ( w );

	grid = new QGridLayout;
	w->setLayout ( grid );
	grid->setColumnStretch ( INFO_COL, 1 ); // Stretch column INFO_COL

	grid->addWidget ( new QLabel ( "Name" ), 0, LABEL_COL );
	grid->addWidget ( new QLabel ( "Comment" ), 1, LABEL_COL );
	grid->addWidget ( new QLabel ( "Status" ), 2, LABEL_COL );

	m_otp_name = new mtQEX::ButtonMenu;
	grid->addWidget ( m_otp_name, 0, INFO_COL );
	connect ( m_otp_name, SIGNAL ( currentIndexChanged (int) ), this,
		SLOT ( press_otp_change (int) ) );

	m_otp_comment = new QLineEdit;
	m_otp_comment->setReadOnly ( true );
	grid->addWidget ( m_otp_comment, 1, INFO_COL );
	mtQEX::set_minimum_width ( m_otp_comment, 25 );

	m_read_only = new QCheckBox ( "Read Only" );
	grid->addWidget ( m_read_only, 2, INFO_COL );
	connect ( m_read_only, SIGNAL ( stateChanged (int) ), this,
		SLOT ( press_read_only (int) ) );

	w = new QWidget;
	gvb->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	button = new QPushButton ( QIcon::fromTheme("stock-edit"),
		"Edit Comment" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	m_edit_comment = button;
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_edit_comment () ) );

	// Squashes buttons together
	hbox->addWidget ( new QWidget );

/// ----------------------------------------------------------------------------

	groupBox = new QGroupBox ( "Buckets" );
	vbox->addWidget ( groupBox );
	groupBox->setSizePolicy ( QSizePolicy::Preferred,
		QSizePolicy::Fixed );

	gvb = new QVBoxLayout;

	groupBox->setLayout ( gvb );

	w = new QWidget;
	gvb->addWidget ( w );

	grid = new QGridLayout;
	w->setLayout ( grid );
	grid->setColumnStretch ( INFO_COL, 1 ); // Stretch column INFO_COL

	grid->addWidget ( new QLabel ( "Total" ), 2, LABEL_COL );
	grid->addWidget ( new QLabel ( "Used" ), 3, LABEL_COL );
	grid->addWidget ( new QLabel ( "Position" ), 4, LABEL_COL );

	m_bucket_total = new QLabel;
	grid->addWidget ( m_bucket_total, 2, INFO_COL );

	m_bucket_used = new QLabel;
	grid->addWidget ( m_bucket_used, 3, INFO_COL );

	m_bucket_pos = new QLabel;
	grid->addWidget ( m_bucket_pos, 4, INFO_COL );

	w = new QWidget;
	gvb->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	button = new QPushButton ( QIcon::fromTheme("list-add"), "Add" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	m_bucket_add = button;
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_bucket_add () ) );

	button = new QPushButton ( QIcon::fromTheme("edit-clear"), "Empty" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	m_bucket_empty = button;
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_otp_empty () ) );

	// Squashes buttons together
	hbox->addWidget ( new QWidget );

	// Squashes group boxes together
	vbox->addWidget ( new QWidget );

/// ----------------------------------------------------------------------------

	tab = new QWidget;
	tabWidget->addTab ( tab, "Overview" );

	vbox = new QVBoxLayout;
	tab->setLayout ( vbox );

	groupBox = new QGroupBox ( "OTP's" );
	vbox->addWidget ( groupBox );

	groupBox->setSizePolicy ( QSizePolicy::Preferred,
		QSizePolicy::Expanding );

	gvb = new QVBoxLayout;

	groupBox->setLayout ( gvb );

	w = new QWidget;
	gvb->addWidget ( w );

	grid = new QGridLayout;
	w->setLayout ( grid );
	grid->setColumnStretch ( INFO_COL, 1 ); // Stretch column INFO_COL

	grid->addWidget ( new QLabel ( "Total" ), 0, LABEL_COL );

	m_otp_total = new QLabel;
	grid->addWidget ( m_otp_total, 0, INFO_COL );

	m_otp_list = new QTableWidget;
	gvb->addWidget ( m_otp_list );

	m_otp_list->setSelectionMode ( QAbstractItemView::SingleSelection );
	m_otp_list->setSelectionBehavior ( QAbstractItemView::SelectRows );
	m_otp_list->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	m_otp_list->setColumnCount ( 4 );
	m_otp_list->setShowGrid ( false );
	m_otp_list->verticalHeader ()->QEX_RESIZEMODE ( QHeaderView::Fixed );
	m_otp_list->horizontalHeader ()->resizeSections (
		QHeaderView::ResizeToContents );
	m_otp_list->horizontalHeader ()->setStretchLastSection ( true );
	connect ( m_otp_list, SIGNAL( currentCellChanged ( int, int, int,int)),
		this, SLOT ( press_list_row ( int, int, int, int ) ) );

	QStringList columnLabels;
	columnLabels << "Name" << "Read Write" << "Buckets" << "Comment";
	m_otp_list->setHorizontalHeaderLabels ( columnLabels );

	w = new QWidget;
	gvb->addWidget ( w );

	hbox = new QHBoxLayout;
	w->setLayout ( hbox );

	button = new QPushButton ( QIcon::fromTheme("document-new"), "New" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_otp_new () ) );

	button = new QPushButton ( QIcon::fromTheme("document-open"), "Import");
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_otp_import () ) );

	button = new QPushButton ( QIcon::fromTheme("edit-delete"), "Delete" );
	button->setSizePolicy ( QSizePolicy::Fixed, QSizePolicy::Preferred );
	m_otp_delete = button;
	hbox->addWidget ( button );
	connect ( button, SIGNAL ( clicked () ), this,
		SLOT ( press_otp_delete () ) );

	// Squashes buttons together
	hbox->addWidget ( new QWidget );

/// ----------------------------------------------------------------------------

	QDialogButtonBox * button_box = new QDialogButtonBox (
		QDialogButtonBox::Close );


	vbox_main->addWidget ( button_box );

	connect ( button_box, SIGNAL ( rejected () ), this, SLOT ( reject () ));

	button_box->setFocus ();

	// Needed to realize everything before calculations below
	show ();
	mtQEX::process_qt_pending ();

	// Enlarge the window a little bit
	int const pix = width () / 3;
	resize ( width () + 2*pix, -1 );

	// Centralize the dialog to the main window
	int const cx = mainwindow.x () + mainwindow.width () / 2;
	int const cy = mainwindow.y () + mainwindow.height () / 2;
	move ( cx - width () / 2, cy - height () / 2 );

	repopulate ();
	repopulate_otp_list ();

	exec ();
}

void DialogButtInfo::press_edit_comment ()
{
	mtDW::Butt const * const butt = mainwindow.backend.db.get_butt ();

	QString qname = butt->get_comment ();

	if ( mtQEX::dialogTextLine ( this, "OTP Comment", "Comment:", qname,
		-1, qname )
		)
	{
		return;
	}

	report_lib_error ( this, butt->set_comment ( qname.toUtf8 ().data() ) );

	repopulate_partial ();
	repopulate_otp_list ();
}

void DialogButtInfo::press_otp_change ( int const ARG_UNUSED ( i ) )
{
	QString text = m_otp_name->text ();

	report_lib_error ( this, mainwindow.backend.db.get_butt ()->set_otp (
		text.toUtf8 (). data () ) );

	mainwindow.update_statusbar ();

	repopulate_partial ();
	press_list_row ( 0, 0, 0, 0 );
}

void DialogButtInfo::press_read_only ( int const ARG_UNUSED ( i ) )
{
	mtDW::Butt * const butt = mainwindow.backend.db.get_butt ();

	if ( m_read_only->checkState () )
	{
		butt->set_read_only ();
	}
	else
	{
		butt->set_read_write ();
	}

	repopulate_partial ();
	repopulate_otp_list ();
}

void DialogButtInfo::press_otp_new ()
{
	QString qname;
	std::string name;
	mtDW::Well const * const well = mainwindow.backend.db.get_well ();
	mtDW::Butt const * const butt = mainwindow.backend.db.get_butt ();

	do
	{
		butt->get_new_name ( well, name );

		qname = name.c_str ();

		if ( mtQEX::dialogTextLine ( this, "New OTP", "Name:", qname,
			mtDW::OTP_NAME_LEN_MAX, qname )
			)
		{
			return;
		}

		if ( 0 == mtDW::Butt::validate_otp_name (qname.toUtf8().data()))
		{
			break;
		}

		char buf[64];
		snprintf ( buf, sizeof(buf), "%i - %i", mtDW::OTP_NAME_LEN_MIN,
			mtDW::OTP_NAME_LEN_MAX );

		std::string message ( "Invalid OTP name.\n" );
		message += "Valid characters: 0-9 a-z A-Z _ . -\n";
		message += "Valid length: ";
		message += buf;

		QMessageBox::critical ( this, "Error", message.c_str () );

	} while ( 1 );

	report_lib_error ( this, butt->add_otp ( qname.toUtf8 ().data () ) );

	repopulate ();
	repopulate_otp_list ();
}

void DialogButtInfo::press_bucket_add ()
{
	bool ok;
	int const total = QInputDialog::getInt ( this, "Add Buckets", "Total",
		5, 1, 1000, 1, &ok );

	if ( ! ok )
	{
		return;
	}

	mtQEX::BusyDialog busy ( this );
	ButtAddThread work ( busy, mainwindow, total );

	work.start ();

	busy.show_abort ();
	busy.wait_for_thread ( work );

	report_lib_error ( this, work.error () );

	repopulate_partial ();
	repopulate_otp_list ();
}

void DialogButtInfo::press_otp_empty ()
{
	int const res = QMessageBox::question ( this, "Question",
		"Do you really want to empty the OTP buckets?",
		QMessageBox::Yes | QMessageBox::No,
		QMessageBox::No );

	if ( res == QMessageBox::No )
	{
		return;
	}

	report_lib_error ( this, mainwindow.backend.db.get_butt()->
		empty_buckets () );

	repopulate_partial ();
	repopulate_otp_list ();
}

void DialogButtInfo::press_otp_delete ()
{
	std::string name;

	get_otp_list_data ( &name, NULL, NULL );

	QString message = "Do you really want to delete OTP name:\n";
	message += name.c_str ();

	int const res = QMessageBox::question ( this, "Question",
		message, QMessageBox::Yes | QMessageBox::No, QMessageBox::No );

	if ( res == QMessageBox::No )
	{
		return;
	}

	report_lib_error ( this, mainwindow.backend.db.get_butt()->delete_otp (
		name ) );

	repopulate ();
	repopulate_otp_list ();
}

void DialogButtInfo::repopulate ()
{
	m_otp_name->clear ();
	m_otp_name->blockSignals ( true );

	mtDW::Butt * const butt = mainwindow.backend.db.get_butt ();

	std::vector<mtDW::OTPinfo> list;
	butt->get_otp_list ( list );

	for ( size_t i = 0; i < list.size (); i++ )
	{
		m_otp_name->addItem ( mtQEX::qstringFromC (
			list[i].m_name.c_str () ) );
	}

	int const i = m_otp_name->findText ( butt->get_otp_name ().c_str () );

	m_otp_name->setCurrentIndex ( i );
	m_otp_name->blockSignals ( false );

	press_otp_change ( i );

	repopulate_partial ();
}

void DialogButtInfo::repopulate_partial () const
{
	mtDW::Butt * const butt = mainwindow.backend.db.get_butt ();
	int const otp_tot = m_otp_name->count ();
	int const bucket_tot = butt->get_bucket_total ();

	m_otp_total->setText ( QString::number ( otp_tot ) );
	m_otp_comment->setText ( butt->get_comment () );

	m_bucket_total->setText ( QString::number ( bucket_tot ));
	m_bucket_used->setText ( QString::number ( butt->get_bucket_used() ));
	m_bucket_pos->setText ( QString::number ( butt->get_bucket_position()));

	if ( butt->is_read_only () )
	{
		m_bucket_add->setEnabled ( false );
		m_bucket_empty->setEnabled ( false );
		m_edit_comment->setEnabled ( false );

		m_read_only->setCheckState ( Qt::Checked );
	}
	else
	{
		m_bucket_add->setEnabled ( true );
		m_edit_comment->setEnabled ( true );

		if ( bucket_tot > 0 )
		{
			m_bucket_empty->setEnabled ( true );
		}
		else
		{
			m_bucket_empty->setEnabled ( false );
		}

		m_read_only->setCheckState ( Qt::Unchecked );
	}
}



class ButtImportThread : public QThread
{
public:
	ButtImportThread ( Mainwindow &mw, std::string const & path )
		:
		m_butt		( mw.backend.db.get_butt () ),
		m_path		( path ),
		m_error		( 0 )
	{
	}

	void run ()
	{
		m_error = m_butt->import_otp ( m_path );
	}

	inline int error () { return m_error; }

private:
	mtDW::Butt	* const	m_butt;
	std::string	const	m_path;
	int			m_error;
};



void DialogButtInfo::press_otp_import ()
{
	QString const filename = QFileDialog::getExistingDirectory ( this,
		"Select Butt to Import", NULL,QFileDialog::DontUseNativeDialog);

	if ( filename.isEmpty () )
	{
		return;
	}

	mtQEX::BusyDialog busy ( this );
	ButtImportThread work ( mainwindow, filename.toUtf8 ().data () );

	work.start ();

	busy.wait_for_thread ( work );

	report_lib_error ( this, work.error () );

	repopulate ();
	repopulate_otp_list ();
}

void DialogButtInfo::repopulate_otp_list ()
{
	int		selected_row = 0;
	std::string	selected_name;

	get_otp_list_data ( &selected_name, &selected_row, NULL );

	setEnabled ( false );

	mtDW::Butt * const butt = mainwindow.backend.db.get_butt ();

	std::vector<mtDW::OTPinfo> list;
	butt->get_otp_list ( list );

	m_otp_list->setRowCount ( (int)list.size () );

	selected_row = MIN ( selected_row, ((int)list.size ()) - 1 );

	for ( size_t row = 0; row < list.size (); row++ )
	{
		QTableWidgetItem * twItem;
		int const status = list[row].m_status;

		twItem = new QTableWidgetItem;
		twItem->setText ( list[row].m_name.c_str () );
		m_otp_list->setItem ( (int)row, 0, twItem );
		twItem->setData ( Qt::UserRole, QVariant::fromValue ( status ));

		twItem = new QTableWidgetItem;
		twItem->setText ( (status & 1) ? "R" : "RW" );
		m_otp_list->setItem ( (int)row, 1, twItem );

		twItem = new QTableWidgetItem;
		twItem->setText ( QString::number ( list[row].m_buckets ) );
		m_otp_list->setItem ( (int)row, 2, twItem );

		twItem = new QTableWidgetItem;
		twItem->setText ( list[row].m_comment.c_str () );
		m_otp_list->setItem ( (int)row, 3, twItem );

		if ( selected_name == list[row].m_name.c_str () )
		{
			selected_row = (int)row;
		}
	}

	// This hack ensures that all columns are right width (not just visible)
	m_otp_list->setVisible ( false );
	m_otp_list->resizeColumnsToContents ();
	m_otp_list->setVisible ( true );

	m_otp_list->setCurrentCell ( selected_row, 1 );

	setEnabled ( true );
}

int DialogButtInfo::get_otp_list_data (
	std::string	* const	name,
	int		* const	row,
	int		* const	status
	)
{
	if ( m_otp_list->rowCount () < 1 )
	{
		return 1;
	}

	int sel_row = m_otp_list->currentRow ();

	if ( name )
	{
		name->clear ();

		QTableWidgetItem * twItem = m_otp_list->item ( sel_row, 0 );
		if ( twItem )
		{
			*name = twItem->text ().toUtf8 ().data ();
		}
	}

	if ( row )
	{
		*row = sel_row;
	}

	if ( status )
	{
		QTableWidgetItem * twItem = m_otp_list->item ( sel_row, 0 );
		if ( twItem )
		{
			*status = twItem->data( Qt::UserRole ).value<int>();
		}
	}

	return 0;
}

void DialogButtInfo::press_list_row (
	int	const	row,
	int	const	ARG_UNUSED ( column ),
	int	const	ARG_UNUSED ( old_row ),
	int	const	ARG_UNUSED ( old_column )
	)
{
	if ( row < 0 )
	{
		return;
	}

	std::string name;
	int status = 0;

	if ( get_otp_list_data ( &name, NULL, &status ) )
	{
		return;
	}

	mtDW::Butt * const butt = mainwindow.backend.db.get_butt ();

	if ( (status & 1) || name == butt->get_otp_name () )
	{
		m_otp_delete->setEnabled ( false );
	}
	else
	{
		m_otp_delete->setEnabled ( true );
	}
}

