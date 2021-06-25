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

#ifdef U_TK_QT5
	#include <mtqex5.h>
#endif

#ifdef U_TK_QT6
	#include <mtqex6.h>
#endif



#include "be.h"



class Mainwindow;
class fileCombo;
class groupList;

class DialogWellInfo;
class DialogButtInfo;
class DialogButtAnalysis;

class ButtAddThread;

class DialogBinFile;
class DialogCardShuff;
class DialogCoinToss;
class DialogDecList;
class DialogDiceRolls;
class DialogHomoglyphs;
class DialogIntList;
class DialogNumShuff;
class DialogPasswords;
class DialogPins;
class DialogUnicodeFonts;



int report_lib_error (	QWidget * parent, int error );
	// Popup dialog reports the libmtDataWell problem
	// = error



class Mainwindow : public QMainWindow
{
	Q_OBJECT

public:
	explicit Mainwindow ( Backend &be );
	~Mainwindow ();

	void update_statusbar ();

/// ----------------------------------------------------------------------------

	Backend			& backend;
	mtKit::UserPrefs	& uprefs;
	MemPrefs	const	& mprefs;

private slots:

	void press_db_recent ( int i );

///	MENUS

	void press_database_open ();
	void press_database_preferences ();
	void press_database_quit ();

	void press_well_info ();
	void press_well_reset ();

	void press_butt_info ();
	void press_butt_analysis ();

	void press_soda_info ();
	void press_soda_create ();
	void press_soda_extract_file ();
	void press_soda_encrypt ();

	void press_tap_bottle_info ();
	void press_tap_create_bottle ();
	void press_tap_extract_file ();

	void press_apps_binfile ();
	void press_apps_cards ();
	void press_apps_coins ();
	void press_apps_declist ();
	void press_apps_dice ();
	void press_apps_homoglyphs ();
	void press_apps_intlist ();
	void press_apps_numshuff ();
	void press_apps_passwords ();
	void press_apps_pins ();
	void press_apps_unicodefonts ();

	void press_help_help ();
	void press_help_about_qt ();
	void press_help_about ();

private:
	void create_menu ();
	void create_main_ui ();

	void init_well_success ( int fail );

	void create_statusbar ();
	void update_recent_db_menu ();

	int validate_output_dir ();
		// 0=Valid & added to recent_dir & combo repopulated
		// 1=Invalid & user error dialog presented

	void database_load ( std::string const & path );

/// ----------------------------------------------------------------------------

	QLabel		* m_statusbar_db;
	QLabel		* m_statusbar_butt;
	QLabel		* m_statusbar_soda;

	QMenu		* m_well_menu;
	QMenu		* m_butt_menu;
	QMenu		* m_soda_menu;
	QMenu		* m_tap_menu;
	QMenu		* m_apps_menu;

	QAction		* act_db_recent [ PREFS_RECENT_DB_TOTAL ];
	QAction		* act_soda_encrypt;

	fileCombo	* m_combo;
	groupList	* m_input;
	groupList	* m_bottles;
};



class fileCombo : public QGroupBox
{
	Q_OBJECT

public:
	fileCombo (
		mtKit::UserPrefs &preferences,
		char const * title,
		QFileDialog::FileMode filemode
			// QFileDialog::ExistingFile =don't prompt for overwrite
			// QFileDialog::AnyFile = Save (prompt for overwrite)
			// QFileDialog::Directory
		);

	void repopulate ( mtKit::RecentFile const &recent_dir );
	QString get_directory ();

protected:
	void dragEnterEvent ( QDragEnterEvent * event );
	void dropEvent ( QDropEvent * event );

private slots:
	void press_select ();

private:
	mtKit::UserPrefs	& uprefs;

	QComboBox		* const	m_dir_combo;

	std::string		const	m_title;
	QFileDialog::FileMode	const	m_filemode;
};



class groupList : public QGroupBox
{
	Q_OBJECT

public:
	groupList (
		mtKit::UserPrefs &preferences,
		QString const &title,
		QWidget * parent = NULL
		);

	int add_filename ( char const * filename );
		// 0 = added, 1 = not added

	void update_title ();

	void resize_columns ();

	int get_filenames ( QList<std::string> &list );

protected:
	void dragEnterEvent ( QDragEnterEvent * event );
	void dropEvent ( QDropEvent * event );

private slots:

///	BUTTONS

	void press_add ();
	void press_remove ();

private:

/// ----------------------------------------------------------------------------

	mtKit::UserPrefs	& uprefs;
	QString			m_title;

	int			m_id;			// Next ID to use
	QTableWidget		* m_table;
	QMap<int,std::string>	m_file_map;
};



class DialogWellInfo : public QDialog
{
	Q_OBJECT

public:
	explicit DialogWellInfo ( Mainwindow &mw );

private slots:
	void press_seed_reset () const;
	void press_shifts_reset () const;
	void press_files_empty ();
	void press_files_add ();

private:
	void repopulate () const;

/// ----------------------------------------------------------------------------

	QLineEdit	* m_path;

	QLabel		* m_seed;
	QLabel		* m_shifts;
	QLabel		* m_files_done;
	QLabel		* m_files_todo;

	QPushButton	* m_files_empty;

	Mainwindow	&mainwindow;
};



class DialogButtInfo : public QDialog
{
	Q_OBJECT

public:
	explicit DialogButtInfo ( Mainwindow &mw );

private slots:
	void press_otp_change ( int i );
	void press_read_only ( int i );

	void press_otp_new ();
	void press_otp_import ();
	void press_bucket_add ();
	void press_otp_empty ();
	void press_otp_delete ();
	void press_edit_comment ();

	void press_list_row ( int, int, int, int );

private:
	void repopulate ();			// Everything
	void repopulate_partial () const;	// NOT button menu or butt list
	void repopulate_otp_list ();

	int get_otp_list_data (
		std::string * name,
		int * row,
		int * status
		);
		// 0 = Data in table
		// 1 = Empty table

/// ----------------------------------------------------------------------------

	QTableWidget	* m_otp_list;

	mtQEX::ButtonMenu * m_otp_name;
	QLabel		* m_otp_total;
	QLineEdit	* m_otp_comment;
	QCheckBox	* m_read_only;

	QLabel		* m_bucket_total;
	QLabel		* m_bucket_used;
	QLabel		* m_bucket_pos;

	QPushButton	* m_otp_delete;
	QPushButton	* m_edit_comment;
	QPushButton	* m_bucket_add;
	QPushButton	* m_bucket_empty;

	Mainwindow	&mainwindow;
};


class DialogButtAnalysis : public QDialog
{
	Q_OBJECT

public:
	explicit DialogButtAnalysis ( Mainwindow &mw );

private slots:
	void press_slider_8bit ( int i );
	void press_slider_16bit ( int i );
	void press_spin_bucket ( int i );
	void press_bucket_all ( int i );

private:
	void do_analysis ();
	void repopulate ();

/// ----------------------------------------------------------------------------

	QSpinBox		* m_bucket_analyse;
	QCheckBox		* m_bucket_all;
	QLabel			* m_bucket_size;

	QTableWidget		* m_bit_table;

	QLabel			* m_byte_min;
	QLabel			* m_byte_max;

	mtQEX::Image		* m_qi_8bit;
	mtQEX::Image		* m_qi_16bit;

	mtDW::OTPanalysis	analysis;
	Mainwindow		&mainwindow;
};



class DialogBinFile : public QDialog
{
	Q_OBJECT

public:
	explicit DialogBinFile ( Mainwindow &mw );

private slots:
	void press_save ();

private:
	QSpinBox	* m_size;

/// ----------------------------------------------------------------------------

	mtDW::Well	* const	m_well;
};



class DialogCardShuff : public QDialog
{
	Q_OBJECT

public:
	explicit DialogCardShuff ( Mainwindow &mw );

private slots:
	void press_apply ();

private:
	QTextEdit	* m_text;

/// ----------------------------------------------------------------------------

	mtDW::Well	* const	m_well;
};



class DialogCoinToss : public QDialog
{
	Q_OBJECT

public:
	explicit DialogCoinToss ( Mainwindow &mw );

private slots:
	void press_apply ();

private:
	QSpinBox	* m_total;
	QTextEdit	* m_text;

/// ----------------------------------------------------------------------------

	mtDW::Well	* const	m_well;
};



class DialogDecList : public QDialog
{
	Q_OBJECT

public:
	explicit DialogDecList ( Mainwindow &mw );

private slots:
	void press_apply ();

private:
	QSpinBox	* m_total;
	QDoubleSpinBox	* m_min;
	QDoubleSpinBox	* m_max;
	QTextEdit	* m_text;

/// ----------------------------------------------------------------------------

	mtDW::Well	* const	m_well;
};



class DialogDiceRolls : public QDialog
{
	Q_OBJECT

public:
	explicit DialogDiceRolls ( Mainwindow &mw );

private slots:
	void press_apply ();

private:
	QSpinBox	* m_total;
	QSpinBox	* m_faces;
	QTextEdit	* m_text;

/// ----------------------------------------------------------------------------

	mtDW::Well	* const	m_well;
};



class DialogHomoglyphs : public QDialog
{
	Q_OBJECT

public:
	explicit DialogHomoglyphs ( Mainwindow &mw );

private slots:
	void press_utf8_analyse ();
	void press_utf8_decode ();
	void press_utf8_encode ();
	void press_utf8_copy_covert ();
	void press_utf8_copy_output ();

	void press_enc_analyse ();
	void press_enc_encode ();

	void press_dec_clean ();
	void press_dec_decode ();

private:
	QTextEdit	* m_text_dec_info;
	QTextEdit	* m_text_enc_info;

	QTextEdit	* m_utf8_covert;
	QTextEdit	* m_utf8_info;
	QTextEdit	* m_utf8_input;
	QTextEdit	* m_utf8_output;

	fileCombo	* m_file_enc_overt;
	fileCombo	* m_file_enc_covert;
	fileCombo	* m_file_enc_output;

	fileCombo	* m_file_dec_input;
	fileCombo	* m_file_dec_output;

/// ----------------------------------------------------------------------------

	mtDW::Well	* const	m_well;
	mtDW::Homoglyph	&m_hg_index;
};



class DialogUnicodeFonts : public QDialog
{
	Q_OBJECT

public:
	explicit DialogUnicodeFonts ( Mainwindow &mw );

private slots:
	void press_utf8_decode ();
	void press_utf8_encode ();
	void press_utf8_copy_output ();

	void press_file_encode ();
	void press_file_decode ();

private:
	QTextEdit	* m_utf8_info;
	QTextEdit	* m_utf8_input;
	QTextEdit	* m_utf8_output;

	mtQEX::ButtonMenu * m_font_type;

	QTextEdit	* m_file_info;
	QTextEdit	* m_font_list;

	fileCombo	* m_file_input;
	fileCombo	* m_file_output;

/// ----------------------------------------------------------------------------

	mtDW::Utf8Font	&m_font_index;
};



class DialogIntList : public QDialog
{
	Q_OBJECT

public:
	explicit DialogIntList ( Mainwindow &mw );

private slots:
	void press_apply ();

private:
	QSpinBox	* m_total;
	QSpinBox	* m_min;
	QSpinBox	* m_range;
	QTextEdit	* m_text;

/// ----------------------------------------------------------------------------

	mtDW::Well	* const	m_well;
};



class DialogNumShuff : public QDialog
{
	Q_OBJECT

public:
	explicit DialogNumShuff ( Mainwindow &mw );

private slots:
	void press_apply ();

private:
	QSpinBox	* m_total;
	QTextEdit	* m_text;

/// ----------------------------------------------------------------------------

	mtDW::Well	* const	m_well;
};



class DialogPasswords : public QDialog
{
	Q_OBJECT

public:
	explicit DialogPasswords ( Mainwindow &mw );

private slots:
	void press_apply ();

private:
	QSpinBox	* m_total;
	QSpinBox	* m_char_tot;
	QCheckBox	* m_lowercase;
	QCheckBox	* m_uppercase;
	QCheckBox	* m_numbers;
	QCheckBox	* m_other;
	QLineEdit	* m_other_text;
	QTextEdit	* m_text;

/// ----------------------------------------------------------------------------

	mtDW::Well	* const	m_well;
};



class DialogPins : public QDialog
{
	Q_OBJECT

public:
	explicit DialogPins ( Mainwindow &mw );

private slots:
	void press_apply ();

private:
	QSpinBox	* m_total;
	QSpinBox	* m_digits;
	QTextEdit	* m_text;

/// ----------------------------------------------------------------------------

	mtDW::Well	* const	m_well;
};

