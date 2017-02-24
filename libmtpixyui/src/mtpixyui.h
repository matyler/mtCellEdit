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

#ifndef MTPIXYUI_H_
#define MTPIXYUI_H_

#include <mtkit.h>
#include <mtpixy.h>



namespace mtPixyUI
{

class Clipboard;
class File;
class PaletteMask;
class UndoStack;
class UndoStep;



class UndoStack
{
public:
	UndoStack ();
	~UndoStack ();

	int undo ( mtPixy::Image ** ppim );
	int redo ( mtPixy::Image ** ppim );

	void clear ();

	int add_next_step ( mtPixy::Image * pim );

	mtPixy::Image * get_current_image ();

	int64_t get_undo_bytes () const;
	int get_undo_steps () const;
	int get_redo_steps () const;

	UndoStep * get_step_current ();

	void set_max_bytes ( int64_t n );
	void set_max_steps ( int n );

private:
	void add_step ( UndoStep * step );

/// ----------------------------------------------------------------------------

	UndoStep	* m_step_first;
	UndoStep	* m_step_current;

	int64_t		m_max_bytes;
	int		m_max_steps;

	int64_t		m_total_bytes;	// In previous items
	int		m_total_undo_steps;
	int		m_total_redo_steps;
};



class UndoStep
{
public:
	UndoStep ( mtPixy::Image * pim );
	~UndoStep ();

	void insert_after ( UndoStep * us );
	int step_restore ( mtPixy::Image ** ppim );

	mtPixy::Image * get_image ();
	UndoStep * get_step_previous ();
	UndoStep * get_step_next ();
	int64_t get_byte_size () const;
	void delete_steps_next ();

private:
	void set_byte_size ();

/// ----------------------------------------------------------------------------

	UndoStep	* m_step_previous;
	UndoStep	* m_step_next;

	mtPixy::Image	* m_image;

	int64_t		m_byte_size;
};



class PaletteMask
{
public:
	PaletteMask ();

	void clear ();
	bool is_masked (		// Args assumed to be valid
		mtPixy::Image * img,
		int x,
		int y
		);
	void protect (			// Args assumed to be valid
		mtPixy::Image * src,	// Old image
		mtPixy::Image * dest,	// Current updated image
		int x,			// Geomtry to check & change
		int y,
		int w,
		int h
		);

/// ----------------------------------------------------------------------------

	char		color[ mtPixy::Palette::COLOR_TOTAL_MAX ];
			// 1=Protected
};



class Clipboard
{
public:
	Clipboard ();
	~Clipboard ();

	int load ( int n );
	int save ( int n );
	int set_image ( mtPixy::Image * im, int x, int y, bool txt = false );

	mtPixy::Image * get_image ();
	void get_xy ( int &x, int &y ) const;

	void render (
		mtPixy::Color const * pal,
		mtPixy::RecSelOverlay const &ovl,
		unsigned char * rgb,
		int x,
		int y,
		int w,
		int h,
		int zs
		);

	int paste (			// Pastes clipboard to image
		File &file,
		int x,
		int y
		);
	int paste (			// Pastes clipboard to image
		File &file,
		int x,
		int y,
		int &dirty_x,
		int &dirty_y,
		int &dirty_w,
		int &dirty_h
		);

	int flip_vertical ();
	int flip_horizontal ();
	int rotate_clockwise ();
	int rotate_anticlockwise ();

	int lasso ( int x, int y );

	bool is_text_paste () const;

private:
	mtPixy::Image	* m_image;
	int		m_x, m_y;	// Position of original copy
	char		m_filename [ 2048 ];
	bool		m_text_paste;
};



class File
{
public:
	enum ToolMode
	{
		TOOL_MODE_PAINT,
		TOOL_MODE_PAINTING,
		TOOL_MODE_LINE,
		TOOL_MODE_LINING,
		TOOL_MODE_SELECT_RECTANGLE,
		TOOL_MODE_SELECTING_RECTANGLE,
		TOOL_MODE_SELECTED_RECTANGLE,
		TOOL_MODE_SELECT_POLYGON,
		TOOL_MODE_SELECTING_POLYGON,
		TOOL_MODE_SELECTED_POLYGON,
		TOOL_MODE_PASTE,
		TOOL_MODE_PASTING,
		TOOL_MODE_FLOODFILL
	};

	File ();
	~File ();

///	GENERAL

	int new_image (
		mtPixy::Image::Type imtype,
		int w,
		int h,
		int pal_type,		// 0..1
		int pal_num		// 2..6
		);
	int load_image ( char const * fn, int pal_type, int pal_num );
	int save_image ( char const * fn, mtPixy::File::Type ft, int comp );
	void set_image ( mtPixy::Image * im );

	int export_undo_images ( char const * fn );

	int resize ( int x, int y, int w, int h );
	int crop ();
	int scale ( int w, int h, mtPixy::Image::ScaleType scaletype );
	int convert_to_rgb ();
	int convert_to_indexed ( mtPixy::Image::DitherType dt );
	int effect_transform_color (
		int ga,		// Gamma	-100..100
		int br,		// Brightness	-255..255
		int co,		// Contrast	-100..100
		int sa,		// Saturation	-100..100
		int hu,		// Hue		-1530..1530
		int po		// Posterize	1..8
		);
	int effect_invert ();
	int effect_edge_detect ();
	int effect_sharpen ( int n );
	int effect_soften ( int n );
	int effect_emboss ();
	int effect_bacteria ( int n );
	int flip_horizontally ();
	int flip_vertically ();
	int rotate_clockwise ();
	int rotate_anticlockwise ();
	int destroy_alpha ();

	mtPixy::Image * render_canvas (
		int x,
		int y,
		int w,
		int h,
		int zs
		);
	void render_zoom_grid (
		unsigned char * rgb,
		int x,
		int y,
		int w,
		int h,
		int zs,
		unsigned char gry
		);

	mtPixy::Image * get_image ();
	char const * get_filename () const;
	int get_modified () const;
	mtPixy::File::Type get_filetype () const;

	int get_pixel_info (
		int canvas_x,
		int canvas_y,
		unsigned char &pixel_red,
		unsigned char &pixel_green,
		unsigned char &pixel_blue,
		int &pixel_index
		);

///	PALETTE

	int palette_set_size ( int n );
	int palette_load ( char const * fn );
	int palette_save ( char const * fn );
	int palette_load_default (
		int pal_type,		// 0=Uniform 1=Balanced
		int pal_num
		);
	int palette_load_color (
		unsigned char idx,
		unsigned char r,
		unsigned char g,
		unsigned char b
		);
	int palette_append (
		unsigned char r,
		unsigned char g,
		unsigned char b
		);
		// Returns new index or -1 on error
	int palette_sort (
		unsigned char i_start,
		unsigned char i_end,
		mtPixy::Image::PaletteSortType s_type,
		bool reverse
		);
	int palette_merge_duplicates ( int * tot );
	int palette_remove_unused ( int * tot );
	int palette_create_gradient ();
	int palette_create_from_canvas ();
	int palette_quantize_pnn ();
	int palette_changed ();
	int palette_swap_ab ();
	int palette_mask_all ();
	int palette_unmask_all ();

	int update_brush_colors ();

///	UNDO

	int undo ();
	int redo ();

	int get_undo_steps () const;
	double get_undo_mb () const;
	int get_redo_steps () const;
	void set_undo_mb_max ( int num );
	void set_undo_steps_max ( int num );

	int commit_undo_step ();

///	TOOLS

	void set_tool_mode ( ToolMode m );
	ToolMode get_tool_mode () const;
	int reset_tool_mode ();
		// 1 = Mode changed

	int paint_brush_start (
		int x,
		int y,
		int &dirty_x,
		int &dirty_y,
		int &dirty_w,
		int &dirty_h
		);
	int paint_brush_to (
		int x,
		int y,
		int &dirty_x,
		int &dirty_y,
		int &dirty_w,
		int &dirty_h
		);
	int paint_brush_to (
		int x,
		int y
		);
	int paint_brush_finish ();
	int paint_line ( int x1, int y1, int x2, int y2 );
	int flood_fill ( int x, int y );

	int select_all ();
	int selection_copy ( Clipboard &clipboard );
	int selection_lasso ( Clipboard &clipboard );
	int selection_fill ();
	int selection_outline ();

	int clipboard_rotate_clockwise ( Clipboard &clipboard );
	int clipboard_rotate_anticlockwise ( Clipboard &clipboard );
	int clipboard_render_text (
		Clipboard &clipboard,
		char const * txt,
		char const * font_name,
		int size,
		int eff_bold,
		int eff_italic,
		int eff_underline,
		int eff_strikethrough
		);


/// ----------------------------------------------------------------------------

	mtPixy::Brush		brush;
	PaletteMask		palette_mask;

	mtPixy::RecSelOverlay	rectangle_overlay;
	mtPixy::PolySelOverlay	polygon_overlay;

private:
	void project_new_chores ( mtPixy::Image * ni );
	int image_new_chores ( mtPixy::Image * i );
	int palette_new_chores ( int num );

	int rectangle_fill ();
	int rectangle_outline ();
	int polygon_fill ();
	int polygon_outline ();

/// ----------------------------------------------------------------------------

	char		* m_filename;
	mtPixy::Image	* m_image;

	UndoStack	m_undo_stack;

	int		m_brush_x, m_brush_y;
	int		m_modified;
	mtPixy::File::Type m_filetype;

	ToolMode	m_tool_mode;
};



}		// namespace mtPixyUI



#endif		// MTPIXYUI_H_

