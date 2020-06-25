/*
	Copyright (C) 2016-2020 Mark Tyler

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

#ifndef MTPIXY_H_
#define MTPIXY_H_

#include <stdlib.h>



#ifdef __cplusplus

#include <memory>


namespace mtPixy
{

class Brush;
class Color;
class Font;
class FontData;
class Image;
class LineOverlay;
class Overlay;
class PolySelOverlay;
class Palette;
class RecSelOverlay;

namespace File {}



inline int int_2_red	( int A ) { return (A >> 16) & 0xFF; }
inline int int_2_green	( int A ) { return (A >> 8) & 0xFF; }
inline int int_2_blue	( int A ) { return (A & 0xFF); }
inline int rgb_2_int	( int R, int G, int B )
				{ return ((R << 16) + (G << 8) + B); }

inline int rgb_2_brightness ( int const r, int const g, int const b )
	{ return (299 * r + 587 * g + 114 * b) / 1000; }



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



namespace File
{
	enum Type
	{
		TYPE_NONE	= -1,

		TYPE_MIN	= 0,
		TYPE_BMP	= 0,	// Indexed / RGB
		TYPE_PNG	= 1,	// Indexed / RGB
		TYPE_JPEG	= 2,	// RGB
		TYPE_GIF	= 3,	// Indexed
		TYPE_GPL	= 4,	// Palette only
		TYPE_PIXY	= 5,	// Indexed / RGB
		TYPE_BP24	= 6,	// RGB Colorspace

// Future additions will go here

		TYPE_MAX	= 6
	};


	Type detect_type ( char const * filename );
	char const * type_text ( Type t );
		// File type in text, or "" if not valid
}



class Color
{
public:
	Color ( unsigned char r, unsigned char g, unsigned char b );
	explicit Color ( int rgb );
	Color ();
	~Color ();

	void set ( int i );
	int get () const;

/// ----------------------------------------------------------------------------

	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
};



class Brush
{
public:
	enum	// Limits
	{
		SHAPE_PAD	= 8,		// On UI palette
		PATTERN_PAD	= 8,		// On UI palette

		SHAPE_SIZE	= 24,		// Pixels on file
		PATTERN_SIZE	= 8,		// Pixels on file

		SPACING_MIN	= 0,
		SPACING_MAX	= 100,

		FLOW_MIN	= 1,
		FLOW_MAX	= 1000
	};

	Brush ();
	~Brush ();

	int load_shapes ( char const * fn );
	int load_patterns ( char const * fn );

	void set_color_ab (
		unsigned char idx_a,
		unsigned char idx_b,
		mtPixy::Color const * col
		);
	void set_color_a ( unsigned char idx, mtPixy::Color const * col );
	void set_color_b ( unsigned char idx, mtPixy::Color const * col );
	int set_shape ( int num );
	int set_pattern ( int num );
	int set_shape ( int x, int y );		// X,Y on palette
	int set_pattern ( int x, int y );	// X,Y on palette
	void set_spacing ( int n );
	void set_space_mod ( int n );
	void set_flow ( int n );

	int rebuild_shapes_palette ( int zoom );
	int rebuild_patterns_palette ( int zoom );

	void rebuild_pattern_mask ();

	mtPixy::Image * build_color_swatch ( int zoom );
	mtPixy::Image * build_shape_swatch ( int zoom );
	mtPixy::Image * build_pattern_swatch ( int zoom );
	mtPixy::Image * build_preview_swatch ( int zoom );

	mtPixy::Image * get_shape_mask ();
	mtPixy::Image * get_pattern_idx ();
	mtPixy::Image * get_pattern_rgb ();

	mtPixy::Image * get_shapes_palette ();
	mtPixy::Image * get_patterns_palette ();

	mtPixy::Color get_color_a () const;
	unsigned char get_color_a_index () const;
	unsigned char get_color_b_index () const;

	int get_spacing () const;
	int get_space_mod () const;
	int get_flow () const;

	void render_cursor (
		int cx,
		int cy,
		unsigned char opacity,
		unsigned char * dest,
		int ox,
		int oy,
		int w,
		int h,
		int zs
		);

protected:
	void rebuild_shape_mask ();

/// ----------------------------------------------------------------------------

	mtPixy::Image	* m_shapes;		// Indexed 24x24 cells in grid
	mtPixy::Image	* m_patterns;		// Indexed 8x8 cells in grid

	mtPixy::Image	* m_shape_mask;		// 24x24 alpha mask (no canvas)
	mtPixy::Image	* m_pattern_idx;	// 8x8x256 canvas
	mtPixy::Image	* m_pattern_rgb;	// 8x8xRGB canvas

	mtPixy::Image	* m_shapes_palette;	// For UI - pick from all
	mtPixy::Image	* m_patterns_palette;	// For UI - pick from all

	int		m_shapes_palette_zoom;
	int		m_pattern_palette_zoom;

	mtPixy::Color	m_color_a;
	mtPixy::Color	m_color_b;

	unsigned char	m_index_a;
	unsigned char	m_index_b;

	int		m_shape_num;
	int		m_pattern_num;

	int		m_spacing;	// Pixel gap between brush impressions
	int		m_space_mod;	// Current spacing modulo

	int		m_flow;		// 0=No pixels FLOW_MAX=All pixels
};



class Palette
{
public:
	enum	// Limits
	{
		COLOR_TOTAL_MIN	= 2,
		COLOR_TOTAL_MAX	= 256,

		UNIFORM_MIN	= 2,
		UNIFORM_MAX	= 6
	};


	explicit Palette ( int paltype = 2 ); // 1=greyscale, 2..6=uniform
	~Palette ();


	int copy ( Palette const * src );	// this = src (with set_correct)
	int load ( char const * filename );
	int save ( char	const * filename ) const;

	int set_color_total ( int newtotal );
	int set_correct ();		// Corrects any errors in palette,
					// e.g. wrong size
	int set_uniform (
		int factor		// UNIFORM_MIN / MAX
		);
	int set_uniform_balanced (
		int factor		// UNIFORM_MIN / MAX
		);
	int set_grey ();		// Create greyscale gradient palette

	void effect_invert ();

	int create_gradient ( unsigned char a, unsigned char b );
	int append_color (
		unsigned char r,
		unsigned char g,
		unsigned char b
		);
		// -1 => Not added, else new colour index

	int get_color_total () const;
	Color * get_color ();
	Color const * get_color () const;
	int get_color_index (
		unsigned char r,
		unsigned char g,
		unsigned char b
		) const;
		// -1 => Not in palette

	void transform_color (
		int ga,		// Gamma	-100..100
		int br,		// Brightness	-255..255
		int co,		// Contrast	-100..100
		int sa,		// Saturation	-100..100
		int hu,		// Hue		-1530..1530
		int po		// Posterize	1..8
		);

protected:
	int		m_color_total;	// COLOR_TOTAL_MIN / MAX

	Color		m_color [ COLOR_TOTAL_MAX ];
};



class Image
{
public:
	enum	// Limits
	{
		WIDTH_MIN	= 1,
		WIDTH_MAX	= 32767,

		HEIGHT_MIN	= 1,
		HEIGHT_MAX	= 32767,

		FLAG_PALETTE_LOADED	= 1
	};

	enum Type
	{
		TYPE_ALPHA	= 0,	// Alpha only, no canvas
		TYPE_INDEXED	= 1,
		TYPE_RGB	= 2
	};

	enum ScaleType
	{
		SCALE_BLOCKY,		// Nearest neighbour
		SCALE_SMOOTH		// Area mapping (RGB only)
	};

	enum DitherType
	{
		DITHER_NONE,
		DITHER_BASIC,
		DITHER_FLOYD
	};

	enum PaletteSortType
	{
		SORT_HUE,
		SORT_SATURATION,
		SORT_VALUE,
		SORT_MIN,
		SORT_BRIGHTNESS,
		SORT_RED,
		SORT_GREEN,
		SORT_BLUE,
		SORT_FREQUENCY
	};

	Image (
		Type imtype,
		int w,
		int h,
		int * err = NULL	// Optional result flag 0=Success 1=Fail
		);
	~Image ();

	static Image * create ( Type imtype, int w, int h );

	static Image * from_data (
		Type imtype,
		int w,
		int h,
		unsigned char * canv,
		unsigned char * alp
		);

	int create_alpha ();		// Create new empty alpha, destroy old

	Type get_type () const;
	int get_canvas_bpp () const;
	int get_width () const;
	int get_height () const;
	unsigned char * get_canvas ();
	unsigned char const * get_canvas () const;
	unsigned char * get_alpha ();
	unsigned char const * get_alpha () const;
	Palette * get_palette ();
	Palette const * get_palette () const;
	int get_information (
		int &urp,		// Unique RGB pixels
		int &pnip,		// Pixels not in palette
		int * pf,		// Palette frequencies
					// Palette::COLOR_TOTAL_MAX item array
		int &pt			// Palette total
		);

	// copy_*: Geometry/Type must match,i must exist, else returns an error.
	int copy_canvas ( Image const * im );
	int copy_alpha ( Image const * im );

	int create_indexed_canvas ();
	int create_rgb_canvas ();

	// Move alpha to this image (same geometry), destroy im on success
	int move_alpha_destroy ( Image * im );

	void set_data (			// Change w/h/canvas/alpha
		int w,
		int h,
		unsigned char * canv,
		unsigned char * alp
		);

	int paste (			// Basic pixel paste (canvas & alpha)
		Image * src,
		int x,			// Paste src onto dest starting here
		int y
		);
	int paste_alpha_blend (		// Paste using src alpha channel to
		Image * src,		// blend canvas pixels. Alpha unchanged.
		int x,			// Paste src onto dest starting here
		int y
		);
	int paste_alpha_pattern (	// Paste using src alpha channel to
		Image * src,		// blend canvas pixels with brush
		Brush &bru,		// pattern. Alpha unchanged.
		int x,			// Paste src onto dest starting here
		int y
		);
	int paste_alpha_or (		// Paste using src alpha channel to
		Image * src,		// or (|) alpha pixels.
		int x,			// Paste src onto dest starting here
		int y
		);

	void blit_idx_alpha_blend (
		unsigned char * dest,
		int x,			// Start x on dest
		int y,			// Start y on dest
		int w,			// dest width
		int h			// dest height
		);
	void blit_idx (
		unsigned char * dest,
		int x,			// Start x on dest
		int y,			// Start y on dest
		int w,			// dest width
		int h			// dest height
		);
	void blit_rgb_alpha_blend (
		Color const * pal,
		unsigned char * dest,
		int x,			// Start x on dest
		int y,			// Start y on dest
		int w,			// dest width
		int h,			// dest height
		int zs = 1		// Zoom scale
		);
	void blit_rgb (
		Color const * pal,
		unsigned char * dest,
		int x,			// Start x on dest
		int y,			// Start y on dest
		int w,			// dest width
		int h,			// dest height
		int zs = 1		// Zoom scale
		);

/// PALETTE

	int palette_sort (
		unsigned char i_start,
		unsigned char i_end,
		PaletteSortType s_type,
		bool reverse
		);

	int palette_merge_duplicates ( int * tot ); // Must be INDEXED image
	int palette_remove_unused ( int * tot ); // Must be INDEXED image
	int palette_create_from_canvas ();	// Must be RGB image
	void palette_move_color ( unsigned char idx, unsigned char new_idx );
	void palette_set_default ( int pal_type, int pal_num );

	int quantize_pnn (		// Must be RGB image
		int coltot,		// Total colours to quantize to
		Palette * pal = NULL	// Destination palette,NULL= Use image
		);

/// PAINTING

	int paint_canvas_rectangle ( Brush &bru, int x, int y, int w, int h );
	int paint_rectangle ( Brush &bru, int x, int y, int w, int h );
	int paint_polygon (
		Brush &bru,
		PolySelOverlay const &ovl,
		int &x,
		int &y,
		int &w,
		int &h
		);
	int paint_flood_fill ( Brush &bru, int x, int y );
	int paint_brush (
		Brush &bru,
		int x1,
		int y1,
		int x2,
		int y2,
		int &dx,		// Dirty rectangle of updated canvas
		int &dy,
		int &dw,
		int &dh,
		bool skip = false	// true = skip first paint
		);

/// IMAGE TRANSFORMS

	Image * duplicate ();
	Image * resize ( int x, int y, int w, int h );	// Useful for cropping
	Image * resize_trim_by_alpha ( int &minx, int &miny );
	Image * scale ( int w, int h, ScaleType scaletype );
	Image * convert_to_rgb ();
	Image * convert_to_indexed ( DitherType dt );
	Image * effect_transform_color (
		int ga,		// Gamma	-100..100
		int br,		// Brightness	-255..255
		int co,		// Contrast	-100..100
		int sa,		// Saturation	-100..100
		int hu,		// Hue		-1530..1530
		int po		// Posterize	1..8
		);
	Image * effect_invert ();
	Image * effect_edge_detect ();
	Image * effect_sharpen ( int n );
	Image * effect_soften ( int n );
	Image * effect_emboss ();
	Image * effect_normalize ();
	Image * effect_bacteria ( int n );
	Image * flip_horizontally ();
	Image * flip_vertically ();
	Image * rotate_clockwise ();
	Image * rotate_anticlockwise ();

	int destroy_alpha ();
		// 0=Destroyed 1=Nothing to destroy

	int lasso ( int x, int y );

/// FILE I/O

	static Image * load ( char const * filename,
		File::Type * newtyp = NULL	// Optional: put file type here
		);
	static Image * load_bmp ( char const * filename );
	static Image * load_bp24 ( char const * filename );
	static Image * load_gif ( char const * filename );
	static Image * load_jpeg ( char const * filename );
	static Image * load_pixy ( char const * filename );
	static Image * load_png ( char const * filename );

	int save (
		char const * filename,
		File::Type filetype,
		int compression		// PNG/JPEG compression level
		) const;

	int save_bmp ( char const * filename ) const;
	int save_bp24 ( char const * filename,
		int compression		// 0..9 (0 = uncompressed)
		) const;
	int save_gif ( char const * filename ) const;
	int save_jpeg ( char const * filename,
		int compression		// 0..100 (100 = best quality)
		) const;
	int save_pixy ( char const * filename,
		int compression		// 0..9 (0 = uncompressed)
		) const;
	int save_png ( char const * filename,
		int compression		// 0..9 (0 = uncompressed)
		) const;

	void set_file_flag ( int n );	// n = Image::FLAG_*
	int get_file_flag () const;

protected:
	enum EffectType
	{
		EFFECT_EDGE_DETECT,
		EFFECT_SHARPEN,
		EFFECT_SOFTEN,
		EFFECT_EMBOSS
	};

	Image * effect_rgb_action ( EffectType et, int it = 1 );

	int create_canvas ();		// Create new empty canvas, destroy old

	void destroy_canvas ();		// Resets type to mtPixy::Image::ALPHA

	void flood_fill_internal (	// Args checked by caller
		Image * im, int x, int y ) const;

	void paint_flow ( Brush const &bru ) const;
	mtPixy::Image * flood_fill_prepare_alpha ( int x, int y );

/// ----------------------------------------------------------------------------

	Type		m_type;
	int		m_canvas_bpp;	// Bytes per pixel: 0, 1, 3
	int		m_file_flag;	// 1=Palette from file
	Palette		m_palette;

	unsigned char	* m_canvas;
	unsigned char	* m_alpha;

	int		m_width;
	int		m_height;
};



class Font
{
public:
	enum StyleUnderline
	{
		STYLE_UNDERLINE_NONE	= 0,
		STYLE_UNDERLINE_SINGLE	= 1,
		STYLE_UNDERLINE_DOUBLE	= 2,
		STYLE_UNDERLINE_WAVY	= 3
	};



	Font (				// Create new Pango font structure
		char const * name = "Sans",
		int size = 12
		);
	~Font ();

	Image * render_image (	// Create image from text
		char const * utf8,
		int max_width		// 0=No max width
		);

	int set_size ( int size );
	void set_style (
		int bold,
		int italics,
		StyleUnderline underline,
		int strikethrough
		);
	void set_row_pad (
		int row_pad		// Y start of glyph
		);

	int get_width () const;
	int get_height () const;

protected:
	void set_style ();

	Font ( const Font & );			// Disable copy constructor
	Font & operator = (const Font &);	// Disable = operator

/// ----------------------------------------------------------------------------

	FontData	* m_font_data;

	int		m_height;		// Glyph height in pixels
	int		m_width;		// Glyph width in pixels
	int		m_baseline;

	int		m_style_bold;
	int		m_style_italics;
	StyleUnderline	m_style_underline;
	int		m_style_strikethrough;
	int		m_style_row_pad;
};



class Overlay
{
public:
	Overlay ();

	void set_start ( int x, int y );
	void set_end ( int x, int y, int &dx, int &dy, int &dw, int &dh );

	int get_x1 () const;
	int get_y1 () const;
	void get_xy ( int &x1, int &y1, int &x2, int &y2 ) const;

protected:
	int		m_x1, m_y1;	// Image canvas coordinates
	int		m_x2, m_y2;
};



class LineOverlay : public Overlay
{
public:
	void render (
		Brush &bru,
		unsigned char * rgb,
		int x,
		int y,
		int w,
		int h,
		int zs
		) const;
};



class RecSelOverlay : public Overlay
{
public:
	int set ( int x, int y, int w, int h, mtPixy::Image const * im );

	void render (
		unsigned char * rgb,
		int x,
		int y,
		int w,
		int h,
		int zs,
		int col		// 0=Red 1=Blue
		) const;

	void move_selection ( int x, int y, int max_x, int max_y,
		int &dx, int &dy, int &dw, int &dh );
	void move_selection_end ( int x, int y, int max_x, int max_y,
		int &dx, int &dy, int &dw, int &dh );

	int set_paste ( Image * im, Image * pa, int px, int py );
	int set_paste ( Image * im, Image * pa );
	int move_paste ( int x, int y, Image const * im, Image const * pa,
		int &dx, int &dy, int &dw, int &dh );
		// 0 = Not moved

	void get_xywh ( int &x, int &y, int &w, int &h ) const;

	// Set new x2, y2 by moving the closest corner to x,y
	void set_corner( int x, int y, int &dx, int &dy, int &dw, int &dh );
};



class PolySelOverlay : public LineOverlay
{
public:
	enum
	{
		POINT_MAX = 100
	};

	PolySelOverlay ();

	Image * create_mask ( int &x, int &y, int &w, int &h ) const;

	void clear ();
	int add ();		// Push x1, y1 onto list, increment
	Image * copy ( Image * src, int &x, int &y, int &w, int &h ) const;

	void get_xywh ( int &x, int &y, int &w, int &h ) const;

///	------------------------------------------------------------------------

	int		m_point_total;
	int		m_x[ POINT_MAX ];
	int		m_y[ POINT_MAX ];
};



void image_print_geometry ( Image * im, char * buf, size_t buflen );

Image * text_render_preview (
	Image::Type type,
	char const * utf8,
	char const * font_name,
	int size,
	int bold,
	int italics,
	Font::StyleUnderline underline,
	int strikethrough
	);

Image * text_render_paste (
	Image::Type type,
	Brush &bru,
	char const * utf8,
	char const * font_name,
	int size,
	int bold,
	int italics,
	Font::StyleUnderline underline,
	int strikethrough
	);

void transform_color (
	unsigned char * buf,
	int buftot,	// Total pixels
	int gam,	// Gamma	-100..100
	int bri,	// Brightness	-255..255
	int con,	// Contrast	-100..100
	int sat,	// Saturation	-100..100
	int hue,	// Hue		-1530..1530
	int pos		// Posterize	1..8
	);



/*
Not every program using mtPixy wants a Cairo dependency.
If a program needs this function it must #include <cairo.h> before <mtpixy.h>
*/
#ifdef CAIRO_VERSION_MAJOR

Image * image_from_cairo ( cairo_surface_t * surface );

#endif



}		// namespace mtPixy



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
	enum
	{
		MIN_STEPS	= 1,
		MAX_STEPS	= 1000,
		MIN_BYTES	= 1048576,
		MAX_BYTES	= 10485760000
	};

	UndoStack ();
	~UndoStack ();

	int undo ( mtPixy::Image ** ppim );
	int redo ( mtPixy::Image ** ppim );

	void clear ();

	int add_next_step ( mtPixy::Image * pim );

	mtPixy::Image * get_current_image ();

	int64_t get_undo_bytes () const;
	int64_t get_redo_bytes () const;
	int64_t get_canvas_bytes () const;

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

	int		m_total_undo_steps;
	int		m_total_redo_steps;
};



class UndoStep
{
public:
	explicit UndoStep ( mtPixy::Image * pim );
	~UndoStep ();

	void insert_after ( UndoStep * us );
	int step_restore ( mtPixy::Image ** ppim );

	mtPixy::Image * get_image ();
	UndoStep * get_step_previous ();
	UndoStep * get_step_next ();
	int64_t get_canvas_bytes () const;
	void delete_steps_next ();

private:
	void set_canvas_bytes ();

/// ----------------------------------------------------------------------------

	UndoStep	* m_step_previous;
	UndoStep	* m_step_next;

	mtPixy::Image	* m_image;

	int64_t		m_canvas_bytes;	// Bytes used by m_image canvas
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
		) const;
	void protect (			// Args assumed to be valid
		mtPixy::Image * src,	// Old image
		mtPixy::Image * dest,	// Current updated image
		int x,			// Geomtry to check & change
		int y,
		int w,
		int h
		) const;

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

	std::string	create_filename ( int n ) const;

/// ----------------------------------------------------------------------------

	std::unique_ptr<mtPixy::Image> m_img;

	int		m_x, m_y;	// Position of original copy
	std::string	m_path;
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
	int export_colormap ( char const * fn, int comp ) const;

	// Create a new filename to set a new file extension
	static char * get_correct_filename (
		char		const *	filename,
		mtPixy::File::Type	filetype
		);
		// NULL = use "filename" in its current state
		// !NULL = allocated string with correct extension

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
	int effect_normalize ();
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
	static void render_zoom_grid (
		unsigned char * rgb,
		int x,
		int y,
		int w,
		int h,
		int zs,
		unsigned char gry
		);

	mtPixy::Image * get_image ();
	mtPixy::Palette * get_palette () const;
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

	int palette_set ( mtPixy::Palette const * pal );
	int palette_set_size ( int num );
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
	int selection_lasso ( Clipboard &clipboard ) const;
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



#endif		// __cplusplus



#endif		// MTPIXY_H_

