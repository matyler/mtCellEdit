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

@@HEADER_GUARD@@



// Disable noisy warnings for other peoples code
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wsign-conversion"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wfloat-conversion"
#pragma GCC diagnostic ignored "-Wdouble-promotion"

@@HEADER_INCLUDE@@

// Re-enable warnings for my code
#pragma GCC diagnostic pop



#include <mtkit.h>
#include <mtpixy.h>



namespace mtQEX
{

class ArrowFilter;
class BusyDialog;
class ButtonMenu;
class DialogAbout;
class Image;
class ImageArea;
class SaveFileDialog;
class Thread;



typedef std::function<void()> ThreadFunc;



/*
Allow arrow keys to move focus properly inside a grid layout of buttons.
Use:
	mtQEX::ArrowFilter * arrowKeyFilter = new mtQEX::ArrowFilter ( grid );
...
	button->installEventFilter ( arrowKeyFilter );
*/
class ArrowFilter : public QObject
{
	Q_OBJECT

public:
	explicit ArrowFilter ( QGridLayout * layout );

protected:
	bool eventFilter ( QObject * obj, QEvent * ev );

private:
	QGridLayout * gridLayout;
};

class Image : public QScrollArea
{
	Q_OBJECT

public:
	Image ();
	~Image ();

	int getZoom ();
	int setZoom ( int z );
	void update ();

	void setPixmap ( mtPixmap * pixmap );
	mtPixmap * getPixmap ();

private:
	ImageArea	* const m_area;
	mtPixy::Pixmap	m_pixmap;
	int		m_zoom;
};

class ImageArea : public QWidget
{
	Q_OBJECT

public:
	explicit ImageArea ( Image * par );

private:
	void		paintEvent ( QPaintEvent * ev )		override;

/// ----------------------------------------------------------------------------

	Image	* const	qi;
};

class ButtonMenu : public QPushButton
{
	Q_OBJECT

public:
	ButtonMenu ();
	~ButtonMenu ();

	void addItem ( QString t );
	void clear ();
	int count ();
	int currentIndex ();
	int findText ( QString t );
	void setCurrentIndex ( int i );
	QString text ();

signals:
	void currentIndexChanged ( int i );

protected:
	void keyPressEvent ( QKeyEvent * ev );

public slots:
	void popup ();

private slots:
	void optionClicked ( int i );

private:
	int		itemCurrent;
};

class SaveFileDialog : public QFileDialog
{
	Q_OBJECT

public:
	SaveFileDialog (
		QWidget * par = NULL,
		QString title = QString (),
		QStringList formatList = QStringList (),
		int format = 0,
		char const * filename = NULL
		);

	int getFormat ();

private:
	QComboBox	* comboFormat;
};



class Thread : public QThread
{
public:
	explicit Thread ( ThreadFunc func ) : m_func (func) {}
	void run () override { if ( m_func ) m_func (); }
private:
	ThreadFunc	m_func;
};



class BusyDialog : public QDialog
{
	Q_OBJECT

public:
	explicit BusyDialog (
		QWidget		* parent = nullptr,
		char	const	* message = nullptr,
		ThreadFunc	func = nullptr
		);
	~BusyDialog ();

	void show_abort () const;
	inline bool aborted () const { return m_busy.aborted (); }
	mtKit::Busy * get_busy () { return &m_busy; }

	void wait_for_thread ( QThread * thread = nullptr );

public slots:
	void accept ();
	void reject ();

protected:
	void closeEvent ( QCloseEvent * ev );

private slots:
	void press_abort ();

private:
	mtKit::Busy		m_busy;
	QProgressBar		* m_progress	= nullptr;
	QDialogButtonBox	* m_button_box	= nullptr;
	bool			m_default	= true;
	ThreadFunc	const	m_thread_func	= nullptr;
};



class DialogAbout : public QDialog
{
	Q_OBJECT

public:
	DialogAbout ( QWidget * parent, char const * title );

	void add_info ( char const * title, char const * message );

private:
	QTabWidget * m_tab_widget = nullptr;
};



/// Utility functions


void prefs_window (
	QWidget * parent,
	mtKit::UserPrefs & prefs,
	QString const & title
	);

QString qstringFromC (
	char const * cstring,	// If not NUL terminated cstring, set
	int size = -1		// size to length in bytes.
	);

int dialogTextLine (		// Single line text entry
	QWidget * par,
	QString title,
	QString label,
	QString text,		// Initial text to populate dialog
	int maxLength,		// Maximum number of characters -1 = any
	QString &result		// Put result here
	);
	// 0 = OK Pressed
	// 1 = Ignore

int dialogText (		// Multiple line text entry
	QWidget * par,
	QString title,
	QString label,
	QString text,		// Initial text to populate dialog
	QString &result		// Put result here
	);
	// 0 = OK Pressed
	// 1 = Ignore

// Used on app shutdown to store dock/toolbar positions.
int qt_set_state ( mtKit::UserPrefs & prefs, char const * key, QByteArray &qba);

// Used on app startup to restore dock/toolbar positions.
int qt_get_state ( mtKit::UserPrefs & prefs, char const * key, QByteArray &qba);

QPixmap * qpixmap_from_pixypixmap ( mtPixmap const * pm );
mtPixmap * pixypixmap_from_qpixmap ( QPixmap const * pm );

// If this file exists, get the user to confirm the save operation
int message_file_overwrite ( QWidget * parent, QString const &filename );
	// 0 = Caller can save the file
	// 1 = Caller must NOT save the file

// Allow user to select multiple directories
void set_multi_directories ( QFileDialog &dialog );

// Get filename from the dialog
QString get_filename ( QFileDialog &dialog );

QAction * menu_init (
	QObject		* parent,
	char	const	* txt,
	char	const	* shcut,
	char	const	* icon
	);

#define QEX_MENU( A, B, C, D ) \
	act_ ## A = mtQEX::menu_init ( this, B, C, D ); \
	connect ( act_ ## A, SIGNAL ( triggered () ), this, \
		SLOT ( press_ ## A () ) );

void process_qt_pending ();

void set_minimum_width (
	QLineEdit	* const	edit,
	int		const	length
	);



}


#endif		// MTQEX*_H_

