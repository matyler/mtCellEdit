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



#ifdef __cplusplus
	extern "C" {
#endif	//  __cplusplus


// C API

#include "mtkit.h"



#ifdef __cplusplus
	}
#endif	//  __cplusplus



class	qexArrowFilter;
class	qexButtonMenu;
class	qexImage;
class	qexImageArea;
class	qexPrefs;
class	qexPrefsWindow;
class	qexSaveFileDialog;



class qexPrefs
{
public:
	qexPrefs	();
	~qexPrefs	();

	int		load (	char const * filename,
					// If NULL use bin_name
				char const * bin_name
					// ~/.config/bin_name/prefs.txt
				);
	int		save ();

	mtPrefs		* getPrefsMem ();

	int		addTable ( mtPrefTable const * table );

	int		getInt ( char const * key );
	double		getDouble ( char const * key );
	char	const *	getString ( char const * key );

	void		set ( char const * key, int value );
	void		set ( char const * key, double value );
	void		set ( char const * key, char const * value );

private:
	mtPrefs		* prefsMem;
	char		* prefsFilename;
};

class qexPrefsWindow : public QDialog
{
	Q_OBJECT

public:
	qexPrefsWindow	( mtPrefs * prefsMem, QString title );
	~qexPrefsWindow	();

private slots:
	void		pressButtonFilter ();
	void		pressButtonReset ();
	void		pressButtonEdit ();
	void		pressButtonClose ();
	void		tableCellActivated ( int row, int column );
	void		tableCellChanged (
				int currentRow,
				int currentColumn,
				int previousRow,
				int previousColumn
				);

private:
	QLineEdit	* filterEdit,
			* infoEdit
			;
	QTableWidget	* tableWidget;
	QPushButton	* buttonReset,
			* buttonEdit;

	mtPrefs		* prefs;

	void		populateTable ();
};

/*
Allow arrow keys to move focus properly inside a grid layout of buttons.
Use:
	qexArrowFilter * arrowKeyFilter = new qexArrowFilter ( grid );
...
	button->installEventFilter ( arrowKeyFilter );
*/
class qexArrowFilter : public QObject
{
	Q_OBJECT

public:
	explicit qexArrowFilter	( QGridLayout * layout );

protected:
	bool		eventFilter ( QObject * obj, QEvent * event);

private:
	QGridLayout	* gridLayout;
};

class qexImage : public QScrollArea
{
	Q_OBJECT

public:
	qexImage ();
	~qexImage ();

	mtImage		* getImage ();
	int		getZoom ();
	void		setImage ( mtImage * im );
	int		setZoom ( int z );
	void		update ();

private:
	qexImageArea	* area;
	mtImage		* image;
	int		zoom;
};

class qexImageArea : public QWidget
{
	Q_OBJECT

public:
	explicit qexImageArea ( qexImage * parent );

private:
	void		paintEvent ( QPaintEvent * event );


	qexImage	* qi;
};

class qexButtonMenu : public QPushButton
{
	Q_OBJECT

public:
	qexButtonMenu ();
	~qexButtonMenu ();

	void		addItem ( QString t );
	void		clear ();
	int		count ();
	int		currentIndex ();
	int		findText ( QString t );
	void		setCurrentIndex ( int i );
	QString		text ();

signals:
	void		currentIndexChanged ( int i );

protected:
	void		keyPressEvent ( QKeyEvent * event );

public slots:
	void		popup ();

private slots:
	void		optionClicked ( int i );

private:
	QSignalMapper	* signalMapper;
	int		itemCurrent;
};

class qexSaveFileDialog : public QFileDialog
{
	Q_OBJECT

public:
	qexSaveFileDialog (
			QWidget		* parent = NULL,
			QString		title = QString (),
			QStringList	formatList = QStringList (),
			int		format = 0,
			char	const	* filename = NULL
			);

	int		getFormat ();

private:
	QComboBox	* comboFormat;
};



namespace mtQEX
{
	QString qstringFromC (
		char const * cstring,	// If not NUL terminated cstring, set
		int size = -1		// size to length in bytes.
		);

	int prefsInitPrefs (		// Initialize default prefs for
					// preferences window
		mtPrefs * prefs		// Call this before mtkit_prefs_load
		);

	int prefsWindowMirrorPrefs (
		mtPrefs * dest,
		mtPrefs * src
		);

	QString dialogTextLine (	// Single line text entry
		QWidget * parent,
		QString title,
		QString label,
		QString text,		// Initial text to populate dialog
		int maxLength = -1	// Maximum number of characters -1 = any
		);

	QString dialogText (		// Multiple line text entry
		QWidget * parent,
		QString title,
		QString label,
		QString text		// Initial text to populate dialog
		);
}






/*

You must set these up in the static lists somewhere if you don't use
prefsInitPrefs:
(if you don't you will forever live with defaults!)
NOTE: These do not appear in the editable list.

{ "prefs.col1",		MTKIT_PREF_TYPE_INT, "150" },
{ "prefs.col2",		MTKIT_PREF_TYPE_INT, "50" },
{ "prefs.col3",		MTKIT_PREF_TYPE_INT, "50" },
{ "prefs.col4",		MTKIT_PREF_TYPE_INT, "50" },

{ "prefs.window_x",	MTKIT_PREF_TYPE_INT, "259" },
{ "prefs.window_y",	MTKIT_PREF_TYPE_INT, "259" },
{ "prefs.window_w",	MTKIT_PREF_TYPE_INT, "693" },
{ "prefs.window_h",	MTKIT_PREF_TYPE_INT, "651" },

*/



#endif		// MTQEX5_H_

