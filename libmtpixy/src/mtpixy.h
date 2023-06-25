/*
	Copyright (C) 2016-2023 Mark Tyler

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

#include <mtkit.h>



#ifdef __cplusplus
extern "C" {
#endif



typedef struct mtColor		mtColor;
typedef struct mtPalette	mtPalette;
typedef struct mtPixmap		mtPixmap;



enum
{
	PIXY_FILE_TYPE_NONE	= -1,
	PIXY_FILE_TYPE_MIN	= 0,	// -------------------------------------
	PIXY_FILE_TYPE_BMP	= 0,	// Indexed / RGB
	PIXY_FILE_TYPE_PNG	= 1,	// Indexed / RGB
	PIXY_FILE_TYPE_JPEG	= 2,	// RGB
	PIXY_FILE_TYPE_GIF	= 3,	// Indexed
	PIXY_FILE_TYPE_GPL	= 4,	// Palette only
	PIXY_FILE_TYPE_SVG	= 5,	// Load only
// Future additions will go here
	PIXY_FILE_TYPE_MAX	= 5,	// -------------------------------------

	PIXY_PIXMAP_BPP_ALPHA_ONLY	= 0,
	PIXY_PIXMAP_BPP_INDEXED		= 1,
	PIXY_PIXMAP_BPP_RGB		= 3,

	PIXY_PIXMAP_WIDTH_MIN		= 1,
	PIXY_PIXMAP_WIDTH_MAX		= 32767,
	PIXY_PIXMAP_HEIGHT_MIN		= 1,
	PIXY_PIXMAP_HEIGHT_MAX		= 32767,

	PIXY_PALETTE_COLOR_TOTAL_MIN	= 2,
	PIXY_PALETTE_COLOR_TOTAL_MAX	= 256,

	PIXY_PALETTE_UNIFORM_MIN	= 2,
	PIXY_PALETTE_UNIFORM_MAX	= 6,

	PIXY_PALETTE_SORT_HUE		= 0,
	PIXY_PALETTE_SORT_SATURATION	= 1,
	PIXY_PALETTE_SORT_VALUE		= 2,
	PIXY_PALETTE_SORT_MIN		= 3,
	PIXY_PALETTE_SORT_BRIGHTNESS	= 4,
	PIXY_PALETTE_SORT_RED		= 5,
	PIXY_PALETTE_SORT_GREEN		= 6,
	PIXY_PALETTE_SORT_BLUE		= 7,
	PIXY_PALETTE_SORT_FREQUENCY	= 8,

	PIXY_EFFECT_CRT_SCALE_MIN	= 2,
	PIXY_EFFECT_CRT_SCALE_MAX	= 32,

	PIXY_SCALE_BLOCKY		= 0,	// Nearest neighbour
	PIXY_SCALE_SMOOTH		= 1,	// Area mapping (RGB only)

	PIXY_DITHER_NONE		= 0,
	PIXY_DITHER_BASIC		= 1,
	PIXY_DITHER_AVERAGE		= 2,
	PIXY_DITHER_FLOYD		= 3,

	PIXY_BRUSH_FLOW_MIN		= 1,
	PIXY_BRUSH_FLOW_MAX		= 1000
};



struct mtColor
{
	unsigned char	red;
	unsigned char	green;
	unsigned char	blue;
};



struct mtPalette
{
	int		size;
	mtColor		color[ PIXY_PALETTE_COLOR_TOTAL_MAX ];
};



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



///	mtColor		--------------------------------------------------------

#define pixy_int_2_red( A )		( ((A) >> 16) & 0xFF )
#define pixy_int_2_green( A )		( ((A) >> 8) & 0xFF )
#define pixy_int_2_blue( A )		( (A) & 0xFF )
#define pixy_rgb_2_int( R, G, B )	( ((R) << 16) + ((G) << 8) + (B) )
#define pixy_rgb_2_brightness(R,G,B)	( (299*(R) + 587*(G) + 114*(B)) / 1000 )



void pixy_transform_color (
	unsigned char *	buf,
	int		buftot,
	int		gam,
	int		bri,
	int		con,
	int		sat,
	int		hue,
	int		pos
	);

void pixy_color_set_rgb_int (
	mtColor		* color,	// Must be valid
	int		rgb
	);

int pixy_color_get_rgb (
	mtColor	const	* color		// Must be valid
	);

///	mtPalette	--------------------------------------------------------

void pixy_palette_init (		// Default 8 color palette
	mtPalette	* palette	// Must be valid
	);

int pixy_palette_load (
	mtPalette	* palette,	// Must be valid
	char	const	* filename
	);

int pixy_palette_save (
	mtPalette const	* palette,	// Must be valid
	char	const	* filename
	);

int pixy_palette_copy (
	mtPalette	* dest,		// Must be valid
	mtPalette const	* src		// Must be valid
	);

int pixy_palette_set_size (
	mtPalette	* palette,	// Must be valid
	int		newtotal
	);

int pixy_palette_set_correct (		// Corrects any errors in palette,
					// e.g. wrong size
	mtPalette	* palette	// Must be valid
	);

int pixy_palette_set_uniform (
	mtPalette	* palette,	// Must be valid
	int		factor		// UNIFORM_MIN / MAX
	);

int pixy_palette_set_uniform_balanced (
	mtPalette	* palette,	// Must be valid
	int		factor		// UNIFORM_MIN / MAX
	);

int pixy_palette_set_grey (
	mtPalette	* palette	// Must be valid
	);

void pixy_palette_effect_invert (
	mtPalette	* palette	// Must be valid
	);

int pixy_palette_create_gradient (
	mtPalette	* palette,	// Must be valid
	unsigned char	a,
	unsigned char	b
	);

int pixy_palette_append_color (
	mtPalette	* palette,	// Must be valid
	unsigned char	r,
	unsigned char	g,
	unsigned char	b
	);
	// -1 => Not added, else new colour index

int pixy_palette_get_color_index (
	mtPalette const * palette,	// Must be valid
	unsigned char	r,
	unsigned char	g,
	unsigned char	b
	);
	// -1 => Not in palette

void pixy_palette_transform_color (
	mtPalette	* palette,	// Must be valid
	int		ga,		// Gamma	-100..100
	int		br,		// Brightness	-255..255
	int		co,		// Contrast	-100..100
	int		sa,		// Saturation	-100..100
	int		hu,		// Hue		-1530..1530
	int		po		// Posterize	1..8
	);

///	mtPixmap	--------------------------------------------------------

mtPixmap * pixy_pixmap_new (
	int	type,		// 0=Alpha only 1=Indexed 3=RGB
	int	width,
	int	height
	);

mtPixmap * pixy_pixmap_new_rgb (
	int	width,
	int	height
	);

mtPixmap * pixy_pixmap_new_indexed (
	int	width,
	int	height
	);

mtPixmap * pixy_pixmap_new_alpha (
	int	width,
	int	height
	);

mtPixmap * pixy_pixmap_duplicate (
	mtPixmap const	* pixmap
	);

void pixy_pixmap_destroy (
	mtPixmap	** pixmap
	);

/*
Not every program using mtPixy wants a Cairo dependency.
If a program needs this function it must #include <cairo.h> before <mtpixy.h>
*/
#ifdef CAIRO_VERSION_MAJOR

mtPixmap * pixy_pixmap_from_cairo ( cairo_surface_t * surface );

#endif


int pixy_pixmap_create_rgb_canvas (
	mtPixmap	* pixmap
	);

int pixy_pixmap_create_indexed_canvas (
	mtPixmap	* pixmap
	);

int pixy_pixmap_create_alpha (
	mtPixmap	* pixmap
	);

mtPixmap * pixy_pixmap_load (
	char	const	* filename,
	int		* file_type	// MTPIXMAP_FILETYPE_*
	);

int pixy_pixmap_save (
	mtPixmap const	* pixmap,
	char	const	* filename,
	int		file_type,	// MTPIXMAP_FILETYPE_*
	int		compression
	);

int pixy_pixmap_get_width (
	mtPixmap const	* pixmap
	);

int pixy_pixmap_get_height (
	mtPixmap const	* pixmap
	);

int pixy_pixmap_get_bytes_per_pixel (
	mtPixmap const	* pixmap
	);
	// 0=Empty 1=Indexed 3=RGB

mtPalette * pixy_pixmap_get_palette (
	mtPixmap	* pixmap
	);

mtPalette const * pixy_pixmap_get_palette_const (
	mtPixmap const * pixmap
	);

int pixy_pixmap_get_palette_size (
	mtPixmap const * pixmap
	);

unsigned char * pixy_pixmap_get_canvas (
	mtPixmap const	* pixmap
	);

unsigned char * pixy_pixmap_get_alpha (
	mtPixmap const	* pixmap
	);

int pixy_pixmap_destroy_alpha ( mtPixmap * pixmap );
	// 0=Destroyed 1=Nothing to destroy

/*
void pixy_pixmap_set_data (		// Change w/h/canvas/alpha
	mtPixmap	* pixmap,
	int		w,
	int		h,
	unsigned char	* canvas,
	unsigned char	* alpha
	);
*/

void pixy_pixmap_print_geometry (
	mtPixmap const	* pixmap,
	char		* buf,
	size_t		buflen
	);

int pixy_pixmap_get_information (
	mtPixmap const	* pixmap,
	int		* urp,		// Unique RGB pixels
	int		* pnip,		// Pixels not in palette
	int		* pf,		// Palette frequencies
					// Palette::COLOR_TOTAL_MAX item array
	int		* pt		// Palette total
	);


/// BLITTING -------------------------------------------------------------------

void pixy_pixmap_blit_idx_alpha_blend (
	mtPixmap const	* pixmap,
	unsigned char	* dest,
	int		x,		// Start x on dest
	int		y,		// Start y on dest
	int		w,		// dest width
	int		h		// dest height
	);

void pixy_pixmap_blit_idx (
	mtPixmap const	* pixmap,
	unsigned char	* dest,
	int		x,		// Start x on dest
	int		y,		// Start y on dest
	int		w,		// dest width
	int		h		// dest height
	);

void pixy_pixmap_blit_rgb_alpha_blend (
	mtPixmap const	* pixmap,
	mtPalette const	* palette,	// See note below
	unsigned char	* dest,
	int		x,		// Start x on dest
	int		y,		// Start y on dest
	int		w,		// dest width
	int		h,		// dest height
	int		zs		// Zoom scale
	);

void pixy_pixmap_blit_rgb (
	mtPixmap const	* pixmap,
	mtPalette const	* palette,	// See note below
	unsigned char	* dest,
	int		x,		// Start x on dest
	int		y,		// Start y on dest
	int		w,		// dest width
	int		h,		// dest height
	int		zs		// Zoom scale
	);

/*
NOTE: We need a palette reference as the source pixmap may not have the correct
palette.  For example if the user copied an indexed image and wants to paste
this, we must use the current canvas palette and not the old clipboard palette.
*/

/// PALETTE --------------------------------------------------------------------

int pixy_pixmap_palette_sort (
	mtPixmap 	* pixmap,
	unsigned char	i_start,
	unsigned char	i_end,
	int		s_type,
	int		reverse
	);

void pixy_pixmap_palette_move_color (
	mtPixmap 	* pixmap,
	unsigned char	idx,
	unsigned char	new_idx
	);

void pixy_pixmap_palette_set_default (
	mtPixmap 	* pixmap,
	int		pal_type,
	int		pal_num,
	char const	* pal_filename
	);


/// IMAGE TRANSFORMS

mtPixmap * pixy_pixmap_resize (		 // Cropping
	mtPixmap const	* pixmap,
	int		x,
	int		y,
	int		w,
	int		h
	);

mtPixmap * pixy_pixmap_scale (
	mtPixmap const	* pixmap,
	int		w,
	int		h,
	int		scaletype
	);

mtPixmap * pixy_pixmap_convert_to_rgb (
	mtPixmap const	* pixmap
	);

mtPixmap * pixy_pixmap_convert_to_indexed (
	mtPixmap const	* pixmap,
	int		dt		// PIXY_DITHER_**
	);


///	File I/O	--------------------------------------------------------

int pixy_file_detect_type (
	char const * filename
	);
	// PIXY_FILE_*

char const * pixy_file_type_text (
	int type			// PIXY_FILE_*
	);
	// File type in text, or "" if not valid

mtPixmap * pixy_pixmap_load_bmp (
	char	const * filename
	);

mtPixmap * pixy_pixmap_load_png (
	char	const * filename
	);

mtPixmap * pixy_pixmap_load_jpeg (
	char	const * filename
	);

mtPixmap * pixy_pixmap_load_gif (
	char	const * filename
	);

int pixy_pixmap_save_bmp (
	mtPixmap const	* pixmap,
	char	const	* filename
	);

int pixy_pixmap_save_png (
	mtPixmap const	* pixmap,
	char	const	* filename,
	int		compression
	);

int pixy_pixmap_save_jpeg (
	mtPixmap const	* pixmap,
	char	const	* filename,
	int		compression
	);

int pixy_pixmap_save_gif (
	mtPixmap const	* pixmap,
	char	const	* filename
	);



/// C++ API --------------------------------------------------------------------

#ifdef __cplusplus
}

#include <memory>


namespace mtPixy
{

class Brush;
class Font;
class FontData;
class LineOverlay;
class Overlay;
class Pixmap;
class PolySelOverlay;
class RecSelOverlay;
class SVG;



/// PAINTING -------------------------------------------------------------------



class Pixmap
{
public:
	explicit Pixmap ( mtPixmap * ptr = nullptr ) : m_ptr ( ptr ) {}
	~Pixmap () { reset ( nullptr ); }

	inline mtPixmap * get () const { return m_ptr; }

	inline void reset ( mtPixmap * ptr )
	{
		if ( ptr != m_ptr )
		{
			pixy_pixmap_destroy ( &m_ptr );
			m_ptr = ptr;
		}
	}

	inline mtPixmap * release ()
	{
		mtPixmap * res = m_ptr;
		m_ptr = nullptr;
		return res;
	}

private:
	mtPixmap	* m_ptr;

	MTKIT_RULE_OF_FIVE( Pixmap )
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

		FLOW_MIN	= PIXY_BRUSH_FLOW_MIN,
		FLOW_MAX	= PIXY_BRUSH_FLOW_MAX
	};

	Brush ();
	~Brush ();

	int load_shapes ( char const * fn );
	int load_patterns ( char const * fn );

	void set_color_ab (
		unsigned char idx_a,
		unsigned char idx_b,
		mtColor const * col
		);
	void set_color_a ( unsigned char idx, mtColor const * col );
	void set_color_b ( unsigned char idx, mtColor const * col );
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

	mtPixmap * build_color_swatch ( int zoom );
	mtPixmap * build_shape_swatch ( int zoom );
	mtPixmap * build_pattern_swatch ( int zoom );
	mtPixmap * build_preview_swatch ( int zoom );

	mtPixmap * get_shape_mask ();
	mtPixmap * get_pattern_idx ();
	mtPixmap * get_pattern_rgb ();

	mtPixmap * get_shapes_palette ();
	mtPixmap * get_patterns_palette ();

	mtColor get_color_a () const;
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

/// PAINTING

	int paint_canvas_rectangle (
		mtPixmap	* pixmap,
		int		x,
		int		y,
		int		w,
		int		h
		);

	int paint_rectangle (
		mtPixmap	* pixmap,
		int		x,
		int		y,
		int		w,
		int		h
		);

	int paint_polygon (
		mtPixmap * pixmap,
		PolySelOverlay const &ovl,
		int &x,
		int &y,
		int &w,
		int &h
		);

	int paint_brush (
		mtPixmap * pixmap,
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

private:
	void rebuild_shape_mask ();

/// ----------------------------------------------------------------------------

	mtPixy::Pixmap	m_shapes;		// Indexed 24x24 cells in grid
	mtPixy::Pixmap	m_patterns;		// Indexed 8x8 cells in grid

	mtPixy::Pixmap	const m_shape_mask;	// 24x24 alpha mask (no canvas)
	mtPixy::Pixmap	const m_pattern_idx;	// 8x8x256 canvas
	mtPixy::Pixmap	const m_pattern_rgb;	// 8x8xRGB canvas

	mtPixy::Pixmap	m_shapes_palette;	// For UI - pick from all
	mtPixy::Pixmap	m_patterns_palette;	// For UI - pick from all

	int		m_shapes_palette_zoom	= 1;
	int		m_pattern_palette_zoom	= 1;

	mtColor		m_color_a		= {0,0,0};
	mtColor		m_color_b		= {0,0,0};

	unsigned char	m_index_a		= 0;
	unsigned char	m_index_b		= 0;

	int		m_shape_num		= 0;
	int		m_pattern_num		= 0;

	int		m_spacing = 1;	// Pixel gap between brush impressions
	int		m_space_mod = 0;	// Current spacing modulo

	int		m_flow = FLOW_MAX; // 0=No pixels FLOW_MAX=All pixels

	MTKIT_RULE_OF_FIVE( Brush )
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



	explicit Font (			// Create new Pango font structure
		char const * name = "Sans",
		int size = 12
		);
	~Font ();

	mtPixmap * render_pixmap (	// Create pixmap from text
		char const * utf8,
		int max_width		// 0=No max width
		);

	int set_font ( char const * name, int size );
	void set_style (
		int bold,
		int italics,
		StyleUnderline underline,
		int strikethrough
		);
	inline void set_row_pad ( int const row_pad ) // Y start of glyph
	{
		m_row_pad = row_pad;
	}

	inline std::string const & get_name () const	{ return m_name; }
	inline int get_size () const	{ return m_size; }
	inline int get_height () const	{ return m_height; }
	inline int get_width () const	{ return m_width; }
	inline int get_baseline () const { return m_baseline; }

private:
	void set_style ();

/// ----------------------------------------------------------------------------

	std::unique_ptr<FontData> m_font_data;

	std::string	m_name;
	int		m_size		= 0;

	int		m_height	= 0;	// Glyph height in pixels
	int		m_width		= 0;	// Glyph width in pixels
	int		m_baseline	= 0;
	int		m_row_pad	= 0;

	int		m_style_bold	= 0;
	int		m_style_italics = 0;
	StyleUnderline	m_style_underline = STYLE_UNDERLINE_NONE;
	int		m_style_strikethrough = 0;

	MTKIT_RULE_OF_FIVE( Font )
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
	int set ( int x, int y, int w, int h, mtPixmap const * im );

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

	int set_paste (
		mtPixmap const * image,
		mtPixmap const * paste,
		int px,
		int py
		);

	int set_paste ( mtPixmap const * image, mtPixmap const * paste );

	int move_paste (
		int x,
		int y,
		mtPixmap const * image,
		mtPixmap const * paste,
		int &dx,
		int &dy,
		int &dw,
		int &dh
		);
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

	mtPixmap * create_mask ( int &x, int &y, int &w, int &h ) const;

	void clear ();
	int add ();		// Push x1, y1 onto list, increment
	mtPixmap * copy (
		mtPixmap const * src,
		int &x,
		int &y,
		int &w,
		int &h
		) const;

	void get_xywh ( int &x, int &y, int &w, int &h ) const;

///	------------------------------------------------------------------------

	int		m_point_total;
	int		m_x[ POINT_MAX ];
	int		m_y[ POINT_MAX ];
};



class SVG
{
public:
	SVG ();
	~SVG ();

	int load ( char const * filename );
	unsigned char * render ( int width, int height );
		// Returns view of ARGB 32 bit memory slab (w x h).
	void render_free ();

	static int load_pixmap (
		mtPixy::Pixmap	& pixmap,
		char	const *	filename,
		int		width,
		int		height
		);
	int create_pixmap ( Pixmap & pixmap );

	class Op;
	Op const * get_op () const { return m_op.get(); }

private:
	std::unique_ptr<Op> m_op;

	MTKIT_RULE_OF_FIVE( SVG )
};



mtPixmap * text_render_preview_pixmap (
	int bpp,
	char const * utf8,
	char const * font_name,
	int size,
	int bold,
	int italics,
	Font::StyleUnderline underline,
	int strikethrough
	);

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

	UndoStack () {}
	~UndoStack () { clear(); }

	int undo ( mtPixy::Pixmap & ppim );
	int redo ( mtPixy::Pixmap & ppim );

	void clear ();

	int add_next_step ( mtPixmap const * pim );

	mtPixmap * get_pixmap ();

	int64_t get_undo_bytes () const;
	int64_t get_redo_bytes () const;
	int64_t get_canvas_bytes () const;

	int get_undo_steps () const;
	int get_redo_steps () const;

	UndoStep * get_step_current () const;

	void set_max_bytes ( int64_t n );
	void set_max_steps ( int n );

private:
	UndoStep	* m_step_first		= nullptr;
	UndoStep	* m_step_current	= nullptr;

	int64_t		m_max_bytes		= 1000000000;
	int		m_max_steps		= 100;

	int		m_total_undo_steps	= 0;
	int		m_total_redo_steps	= 0;
};



class PaletteMask
{
public:
	PaletteMask ();

	void clear ();
	bool is_masked (
		mtPixmap * img,
		int x,
		int y
		) const;
	void protect (
		mtPixmap const * src,	// Old image
		mtPixmap * dest,	// Current updated image
		int x,			// Geomtry to check & change
		int y,
		int w,
		int h
		) const;

/// ----------------------------------------------------------------------------

	char		color[ PIXY_PALETTE_COLOR_TOTAL_MAX ];
			// 1=Protected

private:
	MTKIT_RULE_OF_FIVE( PaletteMask )
};



class Clipboard
{
public:
	Clipboard ();
	~Clipboard ();

	int load ( int n );
	int save ( int n ) const;
	int set_pixmap ( mtPixmap * im, int x, int y, bool txt = false );

	inline mtPixmap * get_pixmap () const
	{
		return m_pixmap.get();
	}

	void get_xy ( int &x, int &y ) const;

	void render (
		mtPalette const * pal,
		mtPixy::RecSelOverlay const &ovl,
		unsigned char * rgb,
		int x,
		int y,
		int w,
		int h,
		int zs
		) const;

	int paste (			// Pastes clipboard to image
		File const &file,
		int x,
		int y
		) const;
	int paste (			// Pastes clipboard to image
		File const &file,
		int x,
		int y,
		int &dirty_x,
		int &dirty_y,
		int &dirty_w,
		int &dirty_h
		) const;

	int flip_vertical ();
	int flip_horizontal ();
	int rotate_clockwise ();
	int rotate_anticlockwise ();

	int lasso ( int x, int y );

	bool is_text_paste () const;

private:

	std::string	create_filename ( int n ) const;

/// ----------------------------------------------------------------------------

	mtPixy::Pixmap	m_pixmap;

	int		m_x, m_y;	// Position of original copy
	std::string	m_path;
	bool		m_text_paste;
};



class File
{
public:
	File () {}
	~File () {}

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

///	GENERAL

	int new_image (
		int bpp,
		int w,
		int h,
		int pal_type,		// 0..2
		int pal_num,		// 2..6
		std::string const & pal_filename
		);
	int load_image ( char const * fn, int pal_type, int pal_num,
		std::string const & pal_filename );
	int save_image ( char const * fn, int ft, int comp );

	void set_pixmap ( mtPixmap * im ); // This File takes ownership of 'im'

	int export_undo_images ( char const * fn ) const;

	// Create a new filename to set a new file extension
	static char * get_correct_filename (
		char	const *	filename,
		int	filetype		// PIXY_FILE_*
		);
		// NULL = use "filename" in its current state
		// !NULL = allocated string with correct extension

	int resize ( int x, int y, int w, int h );
	int crop ();
	int scale ( int w, int h, int scaletype );	// PIXY_SCALE_*
	int convert_to_rgb ();
	int convert_to_indexed ( int dt );	// PIXY_DITHER_*
	int effect_transform_color (
		int ga,		// Gamma	-100..100
		int br,		// Brightness	-255..255
		int co,		// Contrast	-100..100
		int sa,		// Saturation	-100..100
		int hu,		// Hue		-1530..1530
		int po		// Posterize	1..8
		);
	int effect_invert ();
	int effect_crt ( int scale );
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

	mtPixmap * render_canvas (
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

	mtPalette * get_palette () const;

	inline char const * get_filename () const { return m_filename.c_str(); }
	inline int get_modified () const	{ return m_modified; }
	inline int get_filetype () const	{ return m_filetype; }

	int get_pixel_info (
		int canvas_x,
		int canvas_y,
		unsigned char &pixel_red,
		unsigned char &pixel_green,
		unsigned char &pixel_blue,
		int &pixel_index
		) const;

///	PALETTE

	int palette_set ( mtPalette const * pal );
	int palette_set_size ( int num );
	int palette_load ( char const * fn );
	int palette_save ( char const * fn ) const;
	int palette_load_default (
		int pal_type,		// 0=Uniform 1=Balanced 2=file
		int pal_num,
		std::string const & pal_filename
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
		int s_type,		// PIXY_PALETTE_SORT_*
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
	int selection_copy ( Clipboard &clipboard ) const;
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

	inline mtPixmap * get_pixmap () const
	{
		return m_pixmap.get();
	}

/// ----------------------------------------------------------------------------

	mtPixy::Brush		brush;
	PaletteMask		palette_mask;

	mtPixy::RecSelOverlay	rectangle_overlay;
	mtPixy::PolySelOverlay	polygon_overlay;

private:
	void project_new_chores ( mtPixmap * ni );
	int image_new_chores ( mtPixmap * i );
	int palette_new_chores ( int num );

	int rectangle_fill ();
	int rectangle_outline ();
	int polygon_fill ();
	int polygon_outline ();

/// ----------------------------------------------------------------------------

	std::string	m_filename;

	mtPixy::Pixmap	m_pixmap;

	UndoStack	m_undo_stack;

	int		m_brush_x	= 0;
	int		m_brush_y	= 0;
	int		m_modified	= 0;
	int		m_filetype	= PIXY_FILE_TYPE_NONE;

	ToolMode	m_tool_mode	= TOOL_MODE_PAINT;

	MTKIT_RULE_OF_FIVE( File )
};



}		// namespace mtPixyUI



#endif		// __cplusplus



#endif		// MTPIXY_H_

