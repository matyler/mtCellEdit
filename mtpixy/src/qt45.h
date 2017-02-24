/*
	Copyright (C) 2016-2017 Mark Tyler

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

#include "be.h"

#ifdef U_TK_QT4
	#include <mtqex4.h>
#endif

#ifdef U_TK_QT5
	#include <mtqex5.h>
#endif


class Mainwindow;

class keyPressEater;

class Cursor;
class CanvasView;
class LabelClick;
class PaletteHolder;
class PaletteView;
class PanView;

class DialogBrushSettings;
class DialogClickImage;
class DialogGetInt;
class DialogImageIndexed;
class DialogImageInfo;
class DialogImageNew;
class DialogImageResize;
class DialogImageScale;
class DialogPaletteSort;
class DialogPan;
class DialogPasteText;
class DialogTransColor;



void get_scroll_position_h ( QScrollArea * const sa, double * zxp );
void get_scroll_position_v ( QScrollArea * const sa, double * zyp );
void set_scroll_position_h ( QScrollArea * const sa, double zxp );
void set_scroll_position_v ( QScrollArea * const sa, double zyp );



class Cursor
{
public:
	Cursor ( Mainwindow &mw );

	void set_xy ( int xx, int yy );
	inline int x () const { return m_x; };
	inline int y () const { return m_y; };
	inline int is_on_screen () const { return m_on_screen; };
	void redraw () const;

private:
	Mainwindow	&mainwindow;
	int		m_x;
	int		m_y;
	bool		m_on_screen;
};



class Mainwindow : public QMainWindow
{
	Q_OBJECT

public:
	enum Limits
	{
		RECENT_MENU_TOTAL	= 20,
		CLIPBOARD_MENU_TOTAL	= 12
	};

	Mainwindow ( QApplication &app, Backend &be );
	~Mainwindow ();

	void create_icons ();

	enum
	{
		UPDATE_NONE		= 0,

		UPDATE_CANVAS		= 1,
		UPDATE_PALETTE		= 2,
		UPDATE_MENUS		= 4,
		UPDATE_TOOLBAR		= 8,	// 4 Brush buttons
		UPDATE_BRUSH		= 16,	// BEnd Brush colours recalc'd
		UPDATE_TITLEBAR		= 32,
		UPDATE_RECENT_FILES	= 64,	// NOT in ALL below

		UPDATE_STATUS_UNDO	= 256,
		UPDATE_STATUS_GEOMETRY	= 512,
		UPDATE_STATUS_SELECTION	= 1024,

		UPDATE_STATUS		= UPDATE_STATUS_UNDO
						| UPDATE_STATUS_GEOMETRY
						| UPDATE_STATUS_SELECTION
						,

		UPDATE_ALL_IMAGE	= UPDATE_CANVAS
						| UPDATE_TITLEBAR
						| UPDATE_MENUS
						| UPDATE_STATUS
						,

		UPDATE_ALL		= UPDATE_PALETTE
						| UPDATE_ALL_IMAGE
						| UPDATE_BRUSH
						| UPDATE_TOOLBAR
	};

	void update_ui_scale ();
	void update_ui_scale_palette ();
	void update_undo_mb_max ();
	void update_undo_steps_max ();
	void update_canvas ( int xx, int yy, int w, int h );
	void update_canvas_easel_rgb ();

	void set_statusbar_geometry ( QString txt );
	void set_statusbar_cursor ( QString txt );
	void set_statusbar_selection ( QString txt );
	void set_statusbar_undo ( QString txt );

	void set_canvas_rgb_transform (
		int g, int b, int c, int s, int h, int p );
	void unset_canvas_rgb_transform ();

	// updt => UPDATE_*
	int operation_update ( int res, char const * txt, int updt );
	void update_ui ( int updt );

	void tool_action_key ();

	void tool_action_paint_start ( int cx, int cy );
	void tool_action_paint_to ( int cx, int cy );
	void tool_action_paint_finish ();

	void tool_action_line_start ( int cx, int cy );
	void tool_action_line_to ( int cx, int cy );
	void tool_action_line_finish ();

	void tool_action_recsel_start ( int cx, int cy );
	void tool_action_recsel_to ( int cx, int cy );
	void tool_action_recsel_corner ( int cx, int cy );
	void tool_action_recsel_finish ();
	void tool_action_recsel_clear ();
	void tool_action_recsel_move (
		int xdir,
		int ydir,
		int key_shift,
		int key_ctrl,
		int delta
		);

	void tool_action_polysel_start ( int cx, int cy );
	void tool_action_polysel_to ( int cx, int cy );
	void tool_action_polysel_finish ();
	void tool_action_polysel_clear ();

	void tool_action_paste_drag_start ( int cx, int cy );
	void tool_action_paste_drag_to ( int cx, int cy );
	void tool_action_paste_move (
		int xdir,
		int ydir,
		int key_shift,
		int delta
		);
	void tool_action_paste_commit ();
	void tool_action_paste_set_undo ();
	void tool_action_paste_finish ();

	void tool_action_flood_fill ( int cx, int cy );

	void color_ab_delta ( int d, char c = 'a' );
	void toggle_view_mode ();
	bool handle_zoom_keys ( QKeyEvent * ev );

	void set_tool_mode (
		mtPixyUI::File::ToolMode m,
		int updt = UPDATE_ALL
		);
	void finish_tool_mode ();
	int get_last_zoom_scale ();
	void set_last_zoom_scale ( int zs );

	int render_text_paste ();

/// ----------------------------------------------------------------------------

	mtPixy::LineOverlay	line_overlay;

	Backend			&backend;
	mtKit::Prefs		&prefs;

	Cursor			m_cursor;

public slots:
	void press_file_quit ();

	void press_edit_paste_centre ();

	void press_options_pan_window ();
	void press_options_zoom_main_in ();
	void press_options_zoom_main_out ();
	void press_options_zoom_main_3 ();
	void press_options_zoom_main_100 ();
	void press_options_zoom_main_3200 ();
	void press_options_zoom_split_in ();
	void press_options_zoom_split_out ();
	void press_options_zoom_split_3 ();
	void press_options_zoom_split_100 ();
	void press_options_zoom_split_3200 ();

protected:
	void closeEvent ( QCloseEvent * ev );
	void changeEvent ( QEvent * ev );

private slots:
	void main_view_move_h ( int v );
	void main_view_change_h ( int a, int b );
	void main_view_move_v ( int v );
	void main_view_change_v ( int a, int b );

///	MENUS

	void press_file_new ();
	void press_file_open ();
	void press_file_save ();
	void press_file_save_as ();
	void press_file_export_undo ();
	void press_file_recent ( int i );

	void press_edit_undo ();
	void press_edit_redo ();
	void press_edit_cut ();
	void press_edit_copy ();
	void press_edit_paste ();
	void press_edit_paste_text ();
	void press_edit_load_clipboard ( int num );
	void press_edit_save_clipboard ( int num );

	void press_image_to_rgb ();
	void press_image_to_indexed ();
	void press_image_delete_alpha ();
	void press_image_scale ();
	void press_image_resize ();
	void press_image_crop ();
	void press_image_flip_horizontally ();
	void press_image_flip_vertically ();
	void press_image_rotate_clockwise ();
	void press_image_rotate_anticlockwise ();
	void press_image_information ();

	void press_palette_load ();
	void press_palette_save_as ();
	void press_palette_load_default ();
	void press_palette_mask_all ();
	void press_palette_mask_none ();
	void press_palette_swap_ab ();
	void press_palette_create_gradient ();
	void press_palette_size ();
	void press_palette_merge_duplicates ();
	void press_palette_remove_unused ();
	void press_palette_create_from_canvas ();
	void press_palette_quantize_pnn ();
	void press_palette_sort ();

	void press_selection_all ();
	void press_selection_none ();
	void press_selection_lasso ();
	void press_selection_fill ();
	void press_selection_outline ();
	void press_selection_flip_h ();
	void press_selection_flip_v ();
	void press_selection_rotate_c ();
	void press_selection_rotate_a ();

	void press_effects_transform_color ();
	void press_effects_invert ();
	void press_effects_edge_detect ();
	void press_effects_sharpen ();
	void press_effects_soften ();
	void press_effects_emboss ();
	void press_effects_bacteria ();

	void press_options_full_screen ();
	void press_options_preferences ();
	void press_options_statusbar ();
	void press_options_split_canvas ();
	void press_options_split_switch ();
	void press_options_split_focus ();
	void press_options_zoom_grid ();

	void press_help_help ();
	void press_help_about ();
	void press_help_about_qt ();

///	TOOLBAR

	void press_paint ();
	void press_line ();
	void press_flood_fill ();
	void press_select_rectangle ();
	void press_select_polygon ();

	void press_brush_color ();
	void press_brush_shape ();
	void press_brush_pattern ();
	void press_brush_settings ();

private:
	void create_menu ();
	void create_toolbar ();
	void create_dock ();
	void create_statusbar ();
	void create_prefs_callbacks ();
	void create_easel ();

	void split_hide ();
	void split_show ();
	void split_switch ();
	bool is_split_visible ();

	void main_view_moved ();

	void menu_init (
		QAction ** act,
		char const * txt,
		char const * shcut,
		char const * icn
		);

	mtPixy::Image * get_screenshot ();

	int project_load ( char const * fn );
	int project_save ( char const * fn, mtPixy::File::Type ft );
	int project_new ( mtPixy::Image * im = NULL );
	int copy_selection ();
	int ok_to_lose_changes ();	// 0=No 1=Yes
	void prepare_clipboard_paste ( int centre );

/// UPDATES

	void update_titlebar ();
	void update_recent_files ();

	void update_statusbar_undo ();
	void update_statusbar_geometry ();
	void update_statusbar_selection ();

	void update_brush_colors ();
	void update_toolbars ();
	void update_menus ();
	void reconfigure_views ();
	void rebuild_palette ();

/// ----------------------------------------------------------------------------

	QDockWidget	* m_palette_dock;
	QToolBar	* m_toolbar;
	PaletteHolder	* m_palette_holder;

	QSplitter	* m_easel_split;
	QScrollArea	* m_scroll_main;
	CanvasView	* m_canvas_main;
	QScrollArea	* m_scroll_split;
	CanvasView	* m_canvas_split;

	QAction		* act_file_export_undo;
	QAction		* act_file_recent_separator;
	QAction		* act_file_recent [ RECENT_MENU_TOTAL ];

	QAction		* act_edit_undo;
	QAction		* act_edit_redo;
	QAction		* act_edit_cut;
	QAction		* act_edit_copy;
	QAction		* act_edit_paste;
	QAction		* act_edit_paste_centre;
	QAction		* act_edit_save_clipboard[ CLIPBOARD_MENU_TOTAL ];

	QAction		* act_image_delete_alpha;
	QAction		* act_image_to_rgb;
	QAction		* act_image_to_indexed;
	QAction		* act_image_crop;

	QAction		* act_palette_merge_duplicates;
	QAction		* act_palette_remove_unused;
	QAction		* act_palette_create_from_canvas;
	QAction		* act_palette_quantize_pnn;

	QAction		* act_selection_all;
	QAction		* act_selection_none;
	QAction		* act_selection_lasso;
	QAction		* act_selection_fill;
	QAction		* act_selection_outline;
	QAction		* act_selection_flip_h;
	QAction		* act_selection_flip_v;
	QAction		* act_selection_rotate_c;
	QAction		* act_selection_rotate_a;

	QAction		* act_effects_edge_detect;
	QAction		* act_effects_sharpen;
	QAction		* act_effects_soften;
	QAction		* act_effects_emboss;
	QAction		* act_effects_bacteria;

	QAction		* act_options_statusbar;
	QAction		* act_options_split_canvas;
	QAction		* act_options_split_switch;
	QAction		* act_options_split_focus;
	QMenu		* options_menu_zoom_split;
	QAction		* act_options_zoom_split_in;
	QAction		* act_options_zoom_split_out;
	QAction		* act_options_zoom_split_3;
	QAction		* act_options_zoom_split_100;
	QAction		* act_options_zoom_split_3200;
	QAction		* act_options_zoom_grid;

	QAction		* tb_paint;
	QAction		* tb_line;
	QAction		* tb_flood_fill;
	QAction		* tb_select_rectangle;
	QAction		* tb_select_polygon;

	QAction		* tb_brush_color;
	QAction		* tb_brush_shape;
	QAction		* tb_brush_pattern;
	QAction		* tb_brush_settings;

	QLabel		* m_statusbar_geometry;
	QLabel		* m_statusbar_cursor;
	QLabel		* m_statusbar_selection;
	QLabel		* m_statusbar_undo;

	keyPressEater	* m_key_eater;

	enum
	{
		VIEW_MODE		= 1,
		VIEW_MODE_STATUSBAR	= 2,
		VIEW_MODE_PALETTE	= 4,
		VIEW_MODE_TOOLBAR	= 8
	};

	int		m_paste_drag_x;
	int		m_paste_drag_y;
	int		m_paste_committed;
	int		m_last_zoom_scale;
	int		m_view_mode_flags;
};



class PaletteHolder : public QScrollArea
{
	Q_OBJECT

public:
	PaletteHolder ( Mainwindow &mw );
	~PaletteHolder ();

	void rebuild ();	// Redraw all swatches + update view
	void update_canvas_easel_rgb ();

private:
	void resizeEvent ( QResizeEvent * e );

/// ----------------------------------------------------------------------------

	PaletteView	* m_palette_view;
	Mainwindow	&mainwindow;
};



class PaletteView : public QWidget
{
	Q_OBJECT

public:
	PaletteView ( Mainwindow &mw );
	~PaletteView ();

	void set_swatch_size ( int unit, int coltot, int rowtot );
	void rebuild ();	// Redraw all swatches + update view

private:
	void paintEvent ( QPaintEvent * ev );
	void mousePressEvent ( QMouseEvent * ev );
	void mouseReleaseEvent ( QMouseEvent * ev );
	void mouseMoveEvent ( QMouseEvent * ev );
	void mouseEventRouter ( QMouseEvent * ev, int caller );
	void leaveEvent ( QEvent * ev );

/// ----------------------------------------------------------------------------

	int		m_swatch_size;	// Width/height in pixels
	int		m_swatch_rows;
	int		m_swatch_cols;

	int		m_drag_index;
	int		m_drag_change;

	QPixmap		* m_pixmap;

	Mainwindow	&mainwindow;
};



class CanvasView : public QWidget
{
	Q_OBJECT

public:
	enum Limits
	{
		ZOOM_MIN	= -19,
		ZOOM_DEFAULT	= 0,		// 100%
		ZOOM_MAX	= 19
	};

	CanvasView ( QScrollArea * scr, Mainwindow &mw );
	~CanvasView ();

	void set_zoom ( int z );
	int get_zoom_percent () const;
	int get_zoom_scale () const;	// <0 => Divide >0 => Multiply
	double get_centre_canvas_x () const;
	double get_centre_canvas_y () const;
	void set_zoom_grid ( int z );
	void zoom_in ();
	void zoom_out ();

	void reconfigure ();
	void update_canvas ( int xx, int yy, int w, int h );
	void update_canvas_easel_rgb ();

	void set_canvas_rgb_transform (
				int g, int b, int c, int s, int h, int p );
	void unset_canvas_rgb_transform ();

private:
	void paintEvent ( QPaintEvent * ev );
	void mousePressEvent ( QMouseEvent * ev );
	void mouseReleaseEvent ( QMouseEvent * ev );
	void mouseMoveEvent ( QMouseEvent * ev );
	void mouseEventRouter ( QMouseEvent * ev, int caller );
	void leaveEvent ( QEvent * ev );

/// ----------------------------------------------------------------------------

	int		m_zoom;		// -99=1% -1=50% 0=100% 99=10000%
	int		m_zoom_cx;	// Centre for zoom -1 => nothing.
	int		m_zoom_cy;	// Is set by middle mouse button.
	int		m_zoom_grid;

	int		m_tran_rgb;
	int		m_tran_gamma;
	int		m_tran_brightness;
	int		m_tran_contrast;
	int		m_tran_saturation;
	int		m_tran_hue;
	int		m_tran_posterize;

	QScrollArea	* const	m_scroll_area;
	Mainwindow	&mainwindow;
};



class PanView : public QLabel
{
	Q_OBJECT

public:
	PanView ( QDialog *, Mainwindow &, QScrollArea *, int, mtPixy::Image *);

private:
	void mousePressEvent ( QMouseEvent * ev );
	void mouseMoveEvent ( QMouseEvent * ev );

/// ----------------------------------------------------------------------------

	QDialog		* m_dialog;
	int		m_iw, m_ih, m_pw, m_ph;

	QScrollArea	* const m_scroll_area;
	Mainwindow	&mainwindow;
};



class DialogPan : public QDialog
{
	Q_OBJECT

public:
	DialogPan ( Mainwindow &, int, mtPixy::Image *, QScrollArea * );

protected:
	void keyPressEvent ( QKeyEvent * ev );

private:
	QScrollArea	* const m_scroll_area;
	Mainwindow	&mainwindow;
};



class LabelClick : public QLabel
{
	Q_OBJECT

public:
	LabelClick ( QDialog * dia, int &px, int &py );

private:
	void mouseReleaseEvent ( QMouseEvent * ev );

/// ----------------------------------------------------------------------------

	QDialog		* m_dialog;
	int		&m_x, &m_y;
};



class DialogClickImage : public QDialog
{
	Q_OBJECT

public:
	DialogClickImage (
		mtPixy::Image * im,
		int &px,
		int &py
		);

private:
	bool event ( QEvent * ev );
};



class DialogBrushSettings : public QDialog
{
	Q_OBJECT

public:
	DialogBrushSettings ( int &s, int &f );

private slots:
	void press_ok ();

private:
	int		&m_spacing;
	int		&m_flow;

	QSpinBox	* m_sbox_spacing;
	QSpinBox	* m_sbox_flow;
};



class DialogGetInt : public QDialog
{
	Q_OBJECT

public:
	DialogGetInt (
		int min,
		int max,
		int val,
		char const * title,
		char const * subtitle,
		int apply		// 0=OK 1=Apply
		);

	int get_int ();

private:
	QSpinBox	* sbox_int;
};



class DialogImageIndexed : public QDialog
{
	Q_OBJECT

public:
	DialogImageIndexed ( Mainwindow &mw );

private slots:
	void press_ok ();

private:
	QRadioButton	* m_rbut_none;
	QRadioButton	* m_rbut_basic;
	QRadioButton	* m_rbut_floyd;

	Mainwindow	&mainwindow;
};



class DialogImageInfo : public QDialog
{
	Q_OBJECT

public:
	DialogImageInfo ( Backend &be );
};



class DialogImageNew : public QDialog
{
	Q_OBJECT

public:
	DialogImageNew ( int w, int h, int t, bool clip );

	int get_width ();
	int get_height ();
	int get_type ();

private:
	int		m_result;
	QSpinBox	* m_sbox_width;
	QSpinBox	* m_sbox_height;
	QRadioButton	* m_rbut_rgb;
	QRadioButton	* m_rbut_indexed;
	QRadioButton	* m_rbut_clipboard;
};



class DialogImageResize : public QDialog
{
	Q_OBJECT

public:
	DialogImageResize ( Mainwindow &mw );

private slots:
	void changed_sbox_width ( int i );
	void changed_sbox_height ( int i );
	void press_centre ();
	void press_ok ();

private:
	QSpinBox	* m_sbox_width;
	QSpinBox	* m_sbox_height;
	QSpinBox	* m_sbox_off_x;
	QSpinBox	* m_sbox_off_y;
	QCheckBox	* m_cb_aspect_ratio;

	int	const	m_start_w;
	int	const	m_start_h;
	double	const	m_wh_scale;

	Mainwindow	&mainwindow;
};



class DialogImageScale : public QDialog
{
	Q_OBJECT

public:
	DialogImageScale ( Mainwindow &mw );

private slots:
	void changed_sbox_width ( int i );
	void changed_sbox_height ( int i );
	void press_ok ();

private:
	QSpinBox	* m_sbox_width;
	QSpinBox	* m_sbox_height;
	QCheckBox	* m_cb_smooth;
	QCheckBox	* m_cb_aspect_ratio;

	int	const	m_start_w;
	int	const	m_start_h;
	double	const	m_wh_scale;

	Mainwindow	&mainwindow;
};



class DialogPasteText : public QDialog
{
	Q_OBJECT

public:
	DialogPasteText ( Mainwindow &mw );

private slots:
	void change_entry_text ( QString const &txt );
	void press_select_font ();
	void press_ok ();

private:
	void update_preview ();
	void update_font_table ();

///	------------------------------------------------------------------------

	QLineEdit	* m_line_edit;
	mtQEX::Image	* m_qex_image;
	QLabel		* m_label_font_name;
	QLabel		* m_label_font_size;
	QLabel		* m_label_font_effects;

	Mainwindow	&mainwindow;
};



class DialogPaletteSort : public QDialog
{
	Q_OBJECT

public:
	DialogPaletteSort ( int coltot );

	int get_start ();
	int get_end ();
	mtPixy::Image::PaletteSortType get_sort_type ();
	bool get_reverse ();

private:
	QSpinBox	* m_sbox_start;
	QSpinBox	* m_sbox_end;

	QRadioButton	* m_rbut_hue;
	QRadioButton	* m_rbut_saturation;
	QRadioButton	* m_rbut_value;
	QRadioButton	* m_rbut_min;
	QRadioButton	* m_rbut_brightness;
	QRadioButton	* m_rbut_red;
	QRadioButton	* m_rbut_green;
	QRadioButton	* m_rbut_blue;
	QRadioButton	* m_rbut_frequency;

	QCheckBox	* m_cb_reverse;
};



class DialogTransColor : public QDialog
{
	Q_OBJECT

public:
	DialogTransColor (
		Mainwindow &mw,
		mtPixy::Image * im
		);

private slots:
	void slider_reset ();
	void slider_changed ( int i );
	void press_finished ( int r );

private:
	QSlider		* m_sl_gamma;
	QSlider		* m_sl_brightness;
	QSlider		* m_sl_contrast;
	QSlider		* m_sl_saturation;
	QSlider		* m_sl_hue;
	QSlider		* m_sl_posterize;

	QLabel		* m_label_gamma;
	QLabel		* m_label_brightness;
	QLabel		* m_label_contrast;
	QLabel		* m_label_saturation;
	QLabel		* m_label_hue;
	QLabel		* m_label_posterize;

	mtPixy::Palette	m_opal;
	mtPixy::Palette * const	m_palette_live;
	int		const	m_rgb;

	Mainwindow	&mainwindow;
};


class keyPressEater : public QObject
{
	Q_OBJECT

public:
	keyPressEater ( Mainwindow &mw );

protected:
	bool eventFilter ( QObject * obj, QEvent * ev );

private:
	bool key_filter ( QKeyEvent * ev );

/// ----------------------------------------------------------------------------

	Mainwindow	&mainwindow;
};
