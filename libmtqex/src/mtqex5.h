/*
	Copyright (C) 2013-2016 Mark Tyler

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

#ifndef MTQEX5_H_
#define MTQEX5_H_



#include <QtWidgets>

// tableWidget->horizontalHeader ()->setSectionsClickable
#define QEX_SETCLICKABLE	setSectionsClickable

// tableWidget->horizontalHeader ()->setSectionResizeMode
#define QEX_RESIZEMODE		setSectionResizeMode



#include <mtkit.h>
#include <mtpixy.h>



namespace mtQEX
{

class	ArrowFilter;
class	ButtonMenu;
class	Image;
class	ImageArea;
class	PrefsWindow;
class	SaveFileDialog;



class PrefsWindow : public QDialog
{
	Q_OBJECT

public:
	PrefsWindow ( mtPrefs * prefsMem, QString title );
	~PrefsWindow ();

private slots:
	void pressButtonFilter ();
	void pressButtonReset ();
	void pressButtonEdit ();
	void pressButtonClose ();
	void tableCellActivated ( int row, int column );
	void tableCellChanged (
		int currentRow,
		int currentColumn,
		int previousRow,
		int previousColumn
		);

private:
	QLineEdit	* filterEdit;
	QLineEdit	* infoEdit;
	QTableWidget	* tableWidget;
	QPushButton	* buttonReset;
	QPushButton	* buttonEdit;

	mtPrefs		* prefs;

	void		populateTable ();
};

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

	mtPixy::Image * getImage ();
	int getZoom ();
	void setImage ( mtPixy::Image * im );
	int setZoom ( int z );
	void update ();

private:
	ImageArea	* area;
	mtPixy::Image	* image;
	int		zoom;
};

class ImageArea : public QWidget
{
	Q_OBJECT

public:
	explicit ImageArea ( Image * par );

private:
	void		paintEvent ( QPaintEvent * ev );

	Image		* qi;
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
	QSignalMapper	* signalMapper;
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



QString qstringFromC (
	char const * cstring,	// If not NUL terminated cstring, set
	int size = -1		// size to length in bytes.
	);

QString dialogTextLine (	// Single line text entry
	QWidget * par,
	QString title,
	QString label,
	QString text,		// Initial text to populate dialog
	int maxLength = -1	// Maximum number of characters -1 = any
	);

QString dialogText (		// Multiple line text entry
	QWidget * par,
	QString title,
	QString label,
	QString text		// Initial text to populate dialog
	);

// Used on app shutdown to store dock/toolbar positions.
int qt_set_state ( mtKit::Prefs * pr, char const * key, QByteArray * qb );

// Used on app startup to restore dock/toolbar positions.
int qt_get_state ( mtKit::Prefs * pr, char const * key, QByteArray * qb );

QPixmap * qpixmap_from_pixyimage ( mtPixy::Image * i );
mtPixy::Image * pixyimage_from_qpixmap ( QPixmap * pm );



}



/*

You must set these up in the static lists somewhere if you don't use
mtKit::Prefs::initWindowPrefs or mtKit::prefsInitWindowPrefs:
(if you don't you will forever live with defaults!)
NOTE: These do not appear in the editable list.

{ "prefs.col1",		MTKIT_PREF_TYPE_INT, "0" },
{ "prefs.col2",		MTKIT_PREF_TYPE_INT, "0" },
{ "prefs.col3",		MTKIT_PREF_TYPE_INT, "0" },
{ "prefs.col4",		MTKIT_PREF_TYPE_INT, "0" },

{ "prefs.window_x",	MTKIT_PREF_TYPE_INT, "50" },
{ "prefs.window_y",	MTKIT_PREF_TYPE_INT, "50" },
{ "prefs.window_w",	MTKIT_PREF_TYPE_INT, "800" },
{ "prefs.window_h",	MTKIT_PREF_TYPE_INT, "600" },

*/



#endif		// MTQEX5_H_

