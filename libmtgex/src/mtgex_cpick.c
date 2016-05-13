/*
	Copyright (C) 2008-2016 Mark Tyler
	Copyright (C) 2008 Mark Tyler and Dmitry Groshev

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



// Focusable pixmap widget

#define FPIXMAP_KEY "mtPaint.fpixmap"

typedef struct {
	int		xp,
			yp,
			width,
			height,
			xc,
			yc,
			focused_cursor;
	GdkRectangle	pm,
			cr;
	GdkPixmap	* pixmap,
			* cursor;
	GdkBitmap	* cmask;
} fpixmap_data;



static guint		fpixmap_key;



static fpixmap_data * wj_fpixmap_data (
	GtkWidget	* const	widget
	)
{
	return gtk_object_get_data_by_id ( GTK_OBJECT ( widget ), fpixmap_key );
}

static void wj_fpixmap_paint (
	GtkWidget	* const	widget,
	GdkRectangle	* const	area
	)
{
	GdkRectangle	pdest,
			cdest;
	fpixmap_data	* d;
	int		draw_pmap;


	if ( ! GTK_WIDGET_DRAWABLE ( widget ) )
	{
		return;
	}

	if ( ! (d = wj_fpixmap_data ( widget ) ) )
	{
		return;
	}

	draw_pmap = d->pixmap && gdk_rectangle_intersect ( &d->pm, area,
		&pdest );

	/* Frame */
	if ( d->pixmap )
	{
		gdk_draw_rectangle ( widget->window, widget->style->black_gc,
			FALSE, d->pm.x - 1, d->pm.y - 1, d->pm.width + 1,
			d->pm.height + 1 );
	}

	while ( draw_pmap )
	{
		/* Contents pixmap */
		gdk_draw_pixmap ( widget->window, widget->style->black_gc,
			d->pixmap, pdest.x - d->pm.x, pdest.y - d->pm.y,
			pdest.x, pdest.y, pdest.width, pdest.height );

		/* Cursor pixmap */
		if (	d->focused_cursor &&
			! GTK_WIDGET_HAS_FOCUS ( widget )
			)
		{
			break;
		}

		if (	! d->cursor ||
			! gdk_rectangle_intersect ( &d->cr, &pdest, &cdest )
			)
		{
			break;
		}

		if ( d->cmask )
		{
			gdk_gc_set_clip_mask ( widget->style->black_gc,
				d->cmask );
			gdk_gc_set_clip_origin ( widget->style->black_gc,
				d->cr.x, d->cr.y );
		}

		gdk_draw_pixmap ( widget->window, widget->style->black_gc,
			d->cursor, cdest.x - d->cr.x, cdest.y - d->cr.y,
			cdest.x, cdest.y, cdest.width, cdest.height );

		if ( d->cmask )
		{
			gdk_gc_set_clip_mask ( widget->style->black_gc, NULL );
			gdk_gc_set_clip_origin ( widget->style->black_gc, 0, 0
				);
		}

		break;
	}

	/* Focus rectangle */
	if ( GTK_WIDGET_HAS_FOCUS ( widget ) )
	{
		gtk_paint_focus ( widget->style, widget->window,
			GTK_WIDGET_STATE ( widget ), area, widget, NULL, 0, 0,
			widget->allocation.width - 1,
			widget->allocation.height - 1 );
	}
}

static gboolean wj_fpixmap_expose (
	GtkWidget	* const	widget,
	GdkEventExpose	* const	event
	)
{
	wj_fpixmap_paint ( widget, &event->area );

	return FALSE;
}

static void wj_fpixmap_size_req (
	GtkWidget	* const	widget,
	GtkRequisition	* const	requisition,
	gpointer	const	user_data
	)
{
	fpixmap_data	* const	d = user_data;
	gint		xp,
			yp;


	gtk_widget_style_get ( GTK_WIDGET ( widget ),
		"focus-line-width", &xp, "focus-padding", &yp, NULL );
	yp = (xp + yp) * 2 + 2;
	xp = widget->style->xthickness * 2 + yp;
	yp = widget->style->ythickness * 2 + yp;

	requisition->width = d->width + (d->xp = xp);
	requisition->height = d->height + (d->yp = yp);
}

static void wj_fpixmap_size_alloc (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GtkAllocation	* const	allocation,
	gpointer	const	user_data
	)
{
	fpixmap_data	* const	d = user_data;
	GdkRectangle	opm = d->pm;
	int		w,
			h,
			x = d->xp,
			y = d->yp;


	w = allocation->width - d->xp;
	h = allocation->height - d->yp;

	if ( w > d->width )
	{
		x = allocation->width - d->width , w = d->width;
	}

	if ( y > d->height )
	{
		y = allocation->height - d->height , h = d->height;
	}

	x /= 2; y /= 2;
	w = w < 0 ? 0 : w;
	h = h < 0 ? 0 : h;
	d->pm.x = x;
	d->pm.y = y;
	d->pm.width = w;
	d->pm.height = h;
	d->cr.x += x - opm.x;
	d->cr.y += y - opm.y;
}

static void wj_fpixmap_destroy (
	GtkObject	* const	ARG_UNUSED ( object ),
	gpointer	const	user_data
	)
{
	fpixmap_data	* const	d = user_data;


	if ( d->pixmap )
	{
		gdk_pixmap_unref ( d->pixmap );
	}

	if ( d->cursor )
	{
		gdk_pixmap_unref ( d->cursor );
	}

	if ( d->cmask )
	{
		gdk_bitmap_unref ( d->cmask );
	}

	free ( d );
}

static GtkWidget * mtgex_wj_fpixmap (
	int		const	width,
	int		const	height
	)
{
	GtkWidget	* w;
	fpixmap_data	* d;


	if ( ! fpixmap_key )
	{
		fpixmap_key = g_quark_from_static_string ( FPIXMAP_KEY );
	}

	d = calloc ( 1, sizeof ( fpixmap_data ) );

	if ( ! d )
	{
		return NULL;
	}

	d->width = width;
	d->height = height;
	w = gtk_drawing_area_new ();
	GTK_WIDGET_SET_FLAGS ( w, GTK_CAN_FOCUS );
	gtk_widget_set_events ( w, GDK_ALL_EVENTS_MASK );
	gtk_widget_show ( w );
	gtk_object_set_data_by_id ( GTK_OBJECT ( w ), fpixmap_key, d );

	gtk_signal_connect ( GTK_OBJECT ( w ), "destroy",
		GTK_SIGNAL_FUNC ( wj_fpixmap_destroy ), d );

	gtk_signal_connect ( GTK_OBJECT ( w ), "expose_event",
		GTK_SIGNAL_FUNC ( wj_fpixmap_expose ), NULL );

	gtk_signal_connect_after ( GTK_OBJECT ( w ), "size_request",
		GTK_SIGNAL_FUNC ( wj_fpixmap_size_req ), d );

	gtk_signal_connect ( GTK_OBJECT ( w ), "size_allocate",
		GTK_SIGNAL_FUNC ( wj_fpixmap_size_alloc ), d );

	return w;
}

// Must be called after realize to init, and afterwards to access pixmap
static GdkPixmap * mtgex_wj_fpixmap_pixmap (
	GtkWidget	* const	widget
	)
{
	fpixmap_data	* d;


	d = wj_fpixmap_data ( widget );

	if ( ! d )
	{
		return NULL;
	}

	if (	! d->pixmap &&
		GTK_WIDGET_REALIZED ( widget )
		)
	{
		d->pixmap = gdk_pixmap_new ( widget->window, d->width,
			d->height, -1 );
	}

	return d->pixmap;
}

static void mtgex_wj_fpixmap_draw_rgb (
	GtkWidget	* const	widget,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	unsigned char	* const	rgb,
	int		const	step
	)
{
	fpixmap_data	* d;


	d = wj_fpixmap_data ( widget );

	if ( ! d )
	{
		return;
	}

	if ( ! d->pixmap )
	{
		return;
	}

	gdk_draw_rgb_image ( d->pixmap, widget->style->black_gc,
		x, y, w, h, GDK_RGB_DITHER_NONE, rgb, step );
	gtk_widget_queue_draw_area ( widget, x + d->pm.x, y + d->pm.y, w, h );
}

static void mtgex_wj_fpixmap_fill_rgb (
	GtkWidget	* const	widget,
	int		const	x,
	int		const	y,
	int		const	w,
	int		const	h,
	int		const	rgb
	)
{
	GdkGCValues	sv;
	fpixmap_data	* d;


	d = wj_fpixmap_data ( widget );

	if ( ! d )
	{
		return;
	}

	if ( ! d->pixmap )
	{
		return;
	}

	gdk_gc_get_values ( widget->style->black_gc, &sv );
	gdk_rgb_gc_set_foreground ( widget->style->black_gc, (guint32)rgb );
	gdk_draw_rectangle ( d->pixmap, widget->style->black_gc, TRUE, x, y, w,
		h );
	gdk_gc_set_foreground ( widget->style->black_gc, &sv.foreground );
	gtk_widget_queue_draw_area ( widget, x + d->pm.x, y + d->pm.y, w, h );
}

static void mtgex_wj_fpixmap_move_cursor (
	GtkWidget	* const	widget,
	int		const	x,
	int		const	y
	)
{
	GdkRectangle	ocr,
			tcr1,
			tcr2,
			* rcr = NULL;
	fpixmap_data	* d;


	d = wj_fpixmap_data ( widget );

	if ( ! d )
	{
		return;
	}

	if ( ( x == d->xc ) && ( y == d->yc ) )
	{
		return;
	}

	ocr = d->cr;
	d->cr.x += x - d->xc;
	d->cr.y += y - d->yc;
	d->xc = x; d->yc = y;

	if ( ! d->pixmap || ! d->cursor )
	{
		return;
	}

	if ( d->focused_cursor && ! GTK_WIDGET_HAS_FOCUS ( widget ) )
	{
		return;
	}

	/* Anything visible? */
	if ( ! GTK_WIDGET_VISIBLE ( widget ) )
	{
		return;
	}

	if ( gdk_rectangle_intersect ( &ocr, &d->pm, &tcr1 ) )
	{
		rcr = &tcr1;
	}

	if ( gdk_rectangle_intersect ( &d->cr, &d->pm, &tcr2 ) )
	{
		if ( rcr )
		{
			gdk_rectangle_union ( &tcr1, &tcr2, rcr = &ocr );
		}
		else
		{
			rcr = &tcr2;
		}
	}

	if ( ! rcr )
	{
		return;			// Both positions invisible
	}

	gtk_widget_queue_draw_area ( widget, rcr->x, rcr->y, rcr->width,
		rcr->height );
}

static int mtgex_wj_fpixmap_set_cursor ( // Must be called after realize
	GtkWidget	* const	widget,
	char		* const	image,
	char		* const	mask,
	int		const	width,
	int		const	height,
	int		const	hot_x,
	int		const	hot_y,
	int		const	focused
	)
{
	fpixmap_data	* d;


	if ( ! GTK_WIDGET_REALIZED ( widget ) )
	{
		return FALSE;
	}

	d = wj_fpixmap_data ( widget );

	if ( ! d )
	{
		return FALSE;
	}

	if ( d->cursor )
	{
		gdk_pixmap_unref ( d->cursor );
	}

	if ( d->cmask )
	{
		gdk_bitmap_unref ( d->cmask );
	}

	d->cursor = NULL; d->cmask = NULL;
	d->focused_cursor = focused;

	while ( image )
	{
		d->cursor = gdk_pixmap_create_from_data ( widget->window,
			image, width, height, -1,
			&widget->style->white, &widget->style->black );
		d->cr.x = d->pm.x + d->xc - hot_x;
		d->cr.y = d->pm.y + d->yc - hot_y;
		d->cr.width = width;
		d->cr.height = height;

		if ( ! mask )
		{
			break;
		}

		d->cmask = gdk_bitmap_create_from_data ( widget->window,
			mask, width, height );

		break;
	}

	if ( ! d->pixmap )
	{
		return TRUE;
	}

	/* Optimizing redraw in a rare operation is useless */
	gtk_widget_queue_draw ( widget );

	return TRUE;
}

static int mtgex_wj_fpixmap_xy (
	GtkWidget	* const	widget,
	int			x,
	int			y,
	int		* const	xr,
	int		* const	yr
	)
{
	fpixmap_data	* d;


	d = wj_fpixmap_data ( widget );

	if ( ! d )
	{
		return FALSE;
	}

	if ( ! d->pixmap )
	{
		return FALSE;
	}

	x -= d->pm.x;
	y -= d->pm.y;
	xr[0] = x;
	yr[0] = y;

	if (	x < 0			||
		x >= d->pm.width	||
		y < 0			||
		y >= d->pm.height
		)
	{
		return FALSE;
	}

	return TRUE;
}

// Drawable to RGB
static unsigned char * mtgex_wj_get_rgb_image (
	GdkWindow	* const	window,
	GdkPixmap	* const	pixmap,
	unsigned char	*	buf,
	int		const	x,
	int		const	y,
	int		const	width,
	int		const	height
	)
{
	GdkPixbuf	* pix,
			* res;
	unsigned char	* wbuf = NULL;


	if ( ! buf )
	{
		buf = wbuf = malloc ( (size_t)(width * height * 3) );
	}

	if ( ! buf )
	{
		return NULL;
	}

	pix = gdk_pixbuf_new_from_data ( buf, GDK_COLORSPACE_RGB,
		FALSE, 8, width, height, width * 3, NULL, NULL );

	if ( pix )
	{
		res = gdk_pixbuf_get_from_drawable ( pix,
			pixmap ? pixmap : window, NULL,
			x, y, 0, 0, width, height );
		g_object_unref ( pix );

		if ( res )
		{
			return buf;
		}
	}

	free ( wbuf );

	return NULL;
}



#include "graphics/xbm_picker.xbm"
#include "graphics/xbm_picker_mask.xbm"
#include "graphics/xbm_ring4.xbm"
#include "graphics/xbm_ring4_mask.xbm"



#define CPICKER(obj)		GTK_CHECK_CAST (obj, cpicker_get_type (), cpicker)
#define CPICKER_CLASS(klass)	GTK_CHECK_CLASS_CAST (klass, cpicker_get_type (), cpickerClass)
#define IS_CPICKER(obj)		GTK_CHECK_TYPE (obj, cpicker_get_type ())


#define CPICK_KEY "mtPaint.cpicker"

#define CPICK_PAL_STRIPS_MIN	1
#define CPICK_PAL_STRIPS_DEFAULT 2
#define CPICK_PAL_STRIPS_MAX	8	// Max vertical strips the user can have
#define CPICK_PAL_STRIP_ITEMS	8	// Colours on each vertical strip

#define CPICK_SIZE_MIN		64	// Minimum size of mixer/palette areas
#define CPICK_SIZE_DEFAULT	128
#define CPICK_SIZE_MAX		1024	// Maximum size of mixer/palette areas

#define CPICK_INPUT_RED		0
#define CPICK_INPUT_GREEN	1
#define CPICK_INPUT_BLUE	2
#define CPICK_INPUT_HUE		3
#define CPICK_INPUT_SATURATION	4
#define CPICK_INPUT_VALUE	5
#define CPICK_INPUT_HEX		6
#define CPICK_INPUT_OPACITY	7
#define CPICK_INPUT_TOT		8	// Manual inputs on the right hand side

#define CPICK_AREA_PRECUR	0	// Current / Previous colour swatch
#define CPICK_AREA_PICKER	1	// Picker drawing area - Main
#define CPICK_AREA_HUE		2	// Picker drawing area - Hue slider
#define CPICK_AREA_PALETTE	3	// Palette
#define CPICK_AREA_OPACITY	4	// Opacity
#define CPICK_AREA_TOT		5

#define CPICK_AREA_CURRENT	1
#define CPICK_AREA_PREVIOUS	2

#define GREY_W			153
#define GREY_B			102
#define CPICK_PREFIX_BUF_SIZE	128



enum
{
	COLOR_CHANGED,
	LAST_SIGNAL
};



typedef struct
{
	GtkVBox		vbox;		// Parent class

	GtkWidget	* hbox,		// Main hbox
			* inputs[CPICK_INPUT_TOT],
					// Spin buttons / Hex input
			* opacity_label,
			* areas[CPICK_AREA_TOT]
					// wj_fpixmap's
			;

	int		size,		// Vertical/horizontal size of main
					// mixer
			pal_strips,	// Number of palette strips
			input_vals[CPICK_INPUT_TOT],	// Current input values
			rgb_previous[4], // Previous colour/opacity
			area_size[CPICK_AREA_TOT][2],
					// Width / height of each wj_fpixmap
			lock;		// To block input handlers
	int		drag_x,
			drag_y,
			may_drag;	// For drag & drop
	unsigned char	drag_rgba[4];	// The color being dragged out
} cpicker;

typedef struct
{
	GtkVBoxClass	parent_class;
	void		(* color_changed)(cpicker * cp);
} cpickerClass;



static const GtkTargetEntry cpick_target = { "application/x-color", 0, 0 };
static GtkTargetList	* cpick_tlist;
static void cpicker_class_init	(cpickerClass	* klass);
static void cpicker_init	(cpicker	* cp);
static guint cpicker_signals[LAST_SIGNAL];
static GtkType		cpicker_type;



static unsigned char const greyz[2] = {GREY_W, GREY_B}; // For opacity squares
static unsigned char	mem_pal_def[
			CPICK_PAL_STRIP_ITEMS * CPICK_PAL_STRIPS_DEFAULT]
			[3] = {
				{0,0,0}, {255,0,0},
				{255,255,0}, {0,255,0},
				{0,0,255}, {255,0,255},
				{0,255,255}, {255,255,255},
				{219,219,219}, {182,182,182},
				{146,146,146}, {109,109,109},
				{73,73,73}, {36,36,36},
				{219,146,0}, {109,182,73}
			};
static GtkWidget	* cpick_main_window = NULL;
static mtPrefs		* cpick_prefs = NULL;
static char		cpick_prefix_buf[CPICK_PREFIX_BUF_SIZE] = {0};



static GtkType cpicker_get_type ( void )
{
	if ( ! cpicker_type )
	{
		static const GtkTypeInfo cpicker_info = {
			"cpicker", sizeof ( cpicker ), sizeof ( cpickerClass ),
			(GtkClassInitFunc)cpicker_class_init,
			(GtkObjectInitFunc)cpicker_init,
			NULL, NULL, NULL };


		cpicker_type = gtk_type_unique ( GTK_TYPE_HBOX, &cpicker_info );
	}

	return cpicker_type;
}

static void cpicker_class_init (
	cpickerClass	* const	class
	)
{
	GTK_WIDGET_CLASS ( class )->show_all = gtk_widget_show;

	cpicker_signals[COLOR_CHANGED] = gtk_signal_new ( "color_changed",
		GTK_RUN_FIRST, cpicker_type,
		GTK_SIGNAL_OFFSET ( cpickerClass, color_changed ),
		gtk_signal_default_marshaller, GTK_TYPE_NONE, 0 );

	class->color_changed = NULL;

	/* For drag & drop */
	cpick_tlist = gtk_target_list_new ( &cpick_target, 1 );
}



/* Valid for x = 0..5, which is enough here */
#define MOD3( x )	( ( ( (x) * 5 + 1 ) >> 2 ) & 3 )



/* Nonclassical HSV: H is 0..6, S is 0..1, V is 0..255 */
static void rgb2hsv (
	unsigned char	* const	rgb,
	double		* const	hsv
	)
{
	int		c0,
			c1,
			c2;


	if ( ! ( ( rgb[0] ^ rgb[1] ) | ( rgb[0] ^ rgb[2] ) ) )
	{
		hsv[0] = hsv[1] = 0.0;
		hsv[2] = rgb[0];

		return;
	}

	c2 = rgb[2] < rgb[0] ? 1 : 0;

	if ( rgb[c2] >= rgb[c2 + 1] )
	{
		c2++;
	}

	c0 = MOD3 ( c2 + 1 );
	c1 = ( c2 + c0 ) ^ 3;
	hsv[2] = rgb[c0] > rgb[c1] ? rgb[c0] : rgb[c1];
	hsv[1] = hsv[2] - rgb[c2];
	hsv[0] = c0 * 2 + 1 + ( rgb[c1] - rgb[c0] ) / hsv[1];
	hsv[1] /= hsv[2];
}

static void hsv2rgb (
	unsigned char	* const	rgb,
	double		* const	hsv
	)
{
	double		h0,
			h1,
			h2;
	int		i;


	h2 = hsv[2] * 2;
	h1 = h2 * ( 1.0 - hsv[1] );
	i = (int)hsv[0];
	h0 = ( hsv[0] - i ) * ( h2 - h1 );

	if ( i & 1 )
	{
		h2 -= h0;
		h0 += h2;
	}
	else
	{
		h0 += h1;
	}

	i >>= 1;
	rgb[i] = (unsigned char) ( ( (int)h2 + 1 ) >> 1 );
	rgb[ MOD3 ( i + 1 ) ] = (unsigned char)( ( (int)h0 + 1 ) >> 1 );
	rgb[ MOD3 ( i + 2 ) ] = (unsigned char)( ( (int)h1 + 1 ) >> 1 );
}

static void cpick_area_picker_create (
	cpicker		* const	win
	)
{
	unsigned char	* rgb,
			* dest,
			* bw,
			full[3];
	int		i,
			j,
			k,
			x,
			y,
			w,
			h,
			w1,
			h1,
			w3,
			col;
	double		hsv[3];

	w = win->area_size[CPICK_AREA_PICKER][0];
	h = win->area_size[CPICK_AREA_PICKER][1];
	rgb = malloc ( (size_t)(w * h * 3) );

	if ( ! rgb )
	{
		return;
	}

	w1 = w - 1;
	h1 = h - 1;
	w3 = w * 3;

	// Colour in top right corner

	hsv[0] = (double)win->input_vals[CPICK_INPUT_HUE] / 255.0;
	hsv[1] = 1;
	hsv[2] = 255;

	hsv2rgb ( full, hsv );

	/* Bottom row is black->white */
	dest = bw = rgb + h1 * w3;
	for ( i = 0; i < w; i++ , dest += 3 )
	{
		dest[0] = dest[1] = dest[2] = (unsigned char)( (255 * i) / w1 );
	}

	/* And now use it as multiplier for all other rows */
	for ( y = 0; y < h1; y++ )
	{
		dest = rgb + y * w3;
		// Colour on right side, i.e. corner->white
		k = (255 * (h1 - y)) / h1;

		for ( i = 0; i < 3; i++ )
		{
			col = (255 * 255) + k * (full[i] - 255);
			col = (col + (col >> 8) + 1) >> 8;
			for ( x = i; x < w3; x += 3 )
			{
				j = col * bw[x];
				dest[x] = (unsigned char)
					( (j + (j >> 8) + 1) >> 8 );
			}
		}
	}

	mtgex_wj_fpixmap_draw_rgb ( win->areas[CPICK_AREA_PICKER], 0, 0, w, h,
		rgb, w3);

	free (rgb);
}

static void cpick_precur_paint (
	cpicker		* const	win,
	int		* const	col,
	int		const	opacity,
	unsigned char	* const	rgb,
	int		const	dx,
	int		const	w,
	int		const	h
	)
{
	int		i,
			j,
			k,
			x,
			y;
	unsigned char	cols[6],
			* dest = rgb;


	for ( i = 0; i < 6; i++ )
	{
		k = greyz[i & 1];
		j = 255 * k + opacity * ( col[i >> 1] - k );
		cols[i] = (unsigned char)( ( j + (j >> 8) + 1 ) >> 8 );
	}

	for ( y = 0; y < h; y++ )
	{
		j = (y >> 3) & 1;

		for ( x = 0; x < w; x++ )
		{
			k = ( ( (x + dx) >> 3 ) & 1 ) ^ j;

			*dest++ = cols[k + 0];
			*dest++ = cols[k + 2];
			*dest++ = cols[k + 4];
		}
	}

	mtgex_wj_fpixmap_draw_rgb ( win->areas[CPICK_AREA_PRECUR], dx, 0, w, h,
		rgb, w * 3 );
}

static void cpick_area_precur_create (
	cpicker		* const	win,
	int		const	flag
	)
{
	unsigned char	* rgb;
	int		w,
			h,
			w2,
			w2p;


	w = win->area_size[CPICK_AREA_PRECUR][0];
	h = win->area_size[CPICK_AREA_PRECUR][1];
	w2 = w >> 1;
	w2p = w - w2;

	rgb = malloc ( (size_t)(w2p * h * 3) );
	if ( ! rgb )
	{
		return;
	}

	if ( flag & CPICK_AREA_CURRENT )
	{
		cpick_precur_paint ( win, win->input_vals + CPICK_INPUT_RED,
			win->input_vals[CPICK_INPUT_OPACITY], rgb, w2, w2p, h );
	}

	if ( flag & CPICK_AREA_PREVIOUS )
	{
		cpick_precur_paint ( win, win->rgb_previous,
			win->rgb_previous[3], rgb, 0, w2, h );
	}

	free ( rgb );
}



// Forward references
static void cpick_area_update_cursors (
	cpicker		* cp
	);

static void cpick_refresh_inputs_areas (
	cpicker		* cp
	);

static void cpick_get_rgb (
	cpicker		* cp
	);



static void cpick_populate_inputs (
	cpicker		* const	win
	)
{
	int		i;
	char		txt[32];


	win->lock = TRUE;
	for ( i = 0; i < CPICK_INPUT_TOT; i++ )
	{
		if ( i != CPICK_INPUT_HEX )
		{
			gtk_spin_button_set_value (
				GTK_SPIN_BUTTON ( win->inputs[i] ),
				win->input_vals[i] );
		}
	}

	snprintf ( txt, sizeof ( txt ), "#%06X", MTKIT_RGB_2_INT (
		win->input_vals[CPICK_INPUT_RED],
		win->input_vals[CPICK_INPUT_GREEN],
		win->input_vals[CPICK_INPUT_BLUE] )
		);

	gtk_entry_set_text ( GTK_ENTRY ( win->inputs[CPICK_INPUT_HEX] ), txt );
	win->lock = FALSE;
}

static void cpick_rgba_at (
	cpicker		* const	cp,
	GtkWidget	* const	widget,
	int			x,
	int			y,
	unsigned char	* const	get,
	unsigned char	* const	set
	)
{
	if ( widget == cp->areas[CPICK_AREA_PALETTE] )
	{
		char		txt[128];
		int		col,
				ppc,
				ini_col;


		ppc = cp->area_size[CPICK_AREA_PALETTE][1] /
			CPICK_PAL_STRIP_ITEMS;
		x /= ppc; y /= ppc;
		col = y + CPICK_PAL_STRIP_ITEMS * x;

		snprintf ( txt, 128, "%scpick_pal_%i", cpick_prefix_buf,
			col + 1 );

		if ( get )
		{
			// Use default if not in app's pref table
			ini_col = MTKIT_RGB_2_INT ( mem_pal_def[col][0],
				mem_pal_def[col][1], mem_pal_def[col][2] );
			mtkit_prefs_get_int ( cpick_prefs, txt, &ini_col );

			get[0] = (unsigned char)MTKIT_INT_2_R ( ini_col );
			get[1] = (unsigned char)MTKIT_INT_2_G ( ini_col );
			get[2] = (unsigned char)MTKIT_INT_2_B ( ini_col );
			get[3] = (unsigned char)
				cp->input_vals[CPICK_INPUT_OPACITY];
		}
		if ( set )
		{
#define MEM_2_INT( M, I) (((M)[(I)] << 16) + ((M)[(I) + 1] << 8) + (M)[(I) + 2])

			ini_col = MEM_2_INT ( set, 0 );
			mtkit_prefs_set_int ( cpick_prefs, txt, ini_col );
			mtgex_wj_fpixmap_fill_rgb ( widget, x * ppc, y * ppc,
				ppc, ppc, ini_col );
		}
	}
	else if ( widget == cp->areas[CPICK_AREA_PRECUR] )
	{
		int		* irgb,
				* ialpha;
		int		pc = x >= cp->area_size[CPICK_AREA_PRECUR][0]
					>> 1;


		if ( pc || set )	// Current
		{
			irgb = cp->input_vals + CPICK_INPUT_RED;
			ialpha = cp->input_vals + CPICK_INPUT_OPACITY;
		}
		else			// Previous
		{
			irgb = cp->rgb_previous;
			ialpha = cp->rgb_previous + 3;
		}

		if ( get )
		{
			get[0] = (unsigned char)irgb[0];
			get[1] = (unsigned char)irgb[1];
			get[2] = (unsigned char)irgb[2];
			get[3] = (unsigned char)ialpha[0];
		}

		if ( set )
		{
			irgb[0] = set[0]; irgb[1] = set[1]; irgb[2] = set[2];
			ialpha[0] = set[3];

			if (pc)		// Current color changed - announce it
			{
				mtgex_cpick_set_color ( GTK_WIDGET ( cp ),
					MEM_2_INT ( set, 0 ), set[3] );

				gtk_signal_emit ( GTK_OBJECT ( cp ),
					cpicker_signals[COLOR_CHANGED] );
			}
			else
			{
				cpick_area_precur_create ( cp,
					CPICK_AREA_PREVIOUS );
			}
		}
	}
}

static void cpick_area_mouse (
	GtkWidget	* const	widget,
	cpicker		* const	cp,
	int		const	x,
	int		const	y,
	int		const	ARG_UNUSED ( button )
	)
{
	int		idx,
			rx,
			ry,
			aw,
			ah,
			ah1;


	for ( idx = 0; idx < CPICK_AREA_TOT; idx ++ )
	{
		if ( cp->areas[idx] == widget )
		{
			break;
		}
	}

	if ( idx >= CPICK_AREA_TOT )
	{
		return;
	}

	aw = cp->area_size[idx][0];
	ah = cp->area_size[idx][1];
	ah1 = ah - 1;

	rx = x < 0 ? 0 : x >= aw ? aw - 1 : x;
	ry = y < 0 ? 0 : y > ah1 ? ah1 : y;

	if ( idx == CPICK_AREA_OPACITY )
	{
		mtgex_wj_fpixmap_move_cursor ( widget, aw / 2, ry );

		cp->input_vals[CPICK_INPUT_OPACITY] = 255 - (ry * 255) / ah1;
	}
	else if ( idx == CPICK_AREA_HUE )
	{
		mtgex_wj_fpixmap_move_cursor ( widget, aw / 2, ry );

		cp->input_vals[CPICK_INPUT_HUE] = 1529 - (ry * 1529) / ah1;

		cpick_area_picker_create ( cp );
		cpick_get_rgb ( cp );
	}
	else if ( idx == CPICK_AREA_PICKER )
	{
		mtgex_wj_fpixmap_move_cursor ( widget, rx, ry );

		cp->input_vals[CPICK_INPUT_VALUE] = (rx * 255) / (aw - 1);
		cp->input_vals[CPICK_INPUT_SATURATION] = 255 - (ry * 255) / ah1;
		cpick_get_rgb ( cp );
	}
	else if ( idx == CPICK_AREA_PALETTE )
	{
		unsigned char	rgba[4] = {0,0,0,0};
		int		ini_col;


		cpick_rgba_at ( cp, widget, rx, ry, rgba, NULL );
		ini_col = MEM_2_INT ( rgba, 0 );

		// Only update if colour is different

		if ( ini_col == MTKIT_RGB_2_INT (
			cp->input_vals[CPICK_INPUT_RED],
			cp->input_vals[CPICK_INPUT_GREEN],
			cp->input_vals[CPICK_INPUT_BLUE]) )
		{
			return;
		}

		mtgex_cpick_set_color ( GTK_WIDGET ( cp ), ini_col,
			cp->input_vals[CPICK_INPUT_OPACITY] );
	}
	else
	{
		return;
	}

	if ( idx != CPICK_AREA_PALETTE )
	{
		// cpick_set_color () does that and more

		cpick_populate_inputs ( cp );
		cpick_area_precur_create ( cp, CPICK_AREA_CURRENT );
	}

	gtk_signal_emit ( GTK_OBJECT ( cp ), cpicker_signals[COLOR_CHANGED] );
}

static void cpick_drag_get (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	GdkDragContext	* const	ARG_UNUSED ( drag_context ),
	GtkSelectionData * const data,
	guint		const	ARG_UNUSED ( info ),
	guint		const	ARG_UNUSED ( time ),
	gpointer	const	user_data
	)
{
	cpicker		* const	cp = user_data;
	guint16		vals[4];


	/* Source RGBA values prepared already - just export them */
	vals[0] = (guint16)(cp->drag_rgba[0] * 257);
	vals[1] = (guint16)(cp->drag_rgba[1] * 257);
	vals[2] = (guint16)(cp->drag_rgba[2] * 257);
	vals[3] = (guint16)(cp->drag_rgba[3] * 257);

	gtk_selection_data_set ( data,
		gdk_atom_intern ( "application/x-color", FALSE ),
		16, (guchar *)vals, 8 );
}

static void cpick_drag_set (
	GtkWidget	* const	widget,
	GdkDragContext	* const	ARG_UNUSED ( drag_context ),
	gint		const	x,
	gint		const	y,
	GtkSelectionData * const data,
	guint		const	ARG_UNUSED ( info ),
	guint		const	ARG_UNUSED ( time ),
	gpointer	const	user_data
	)
{
	cpicker		* const	cp = user_data;
	unsigned char	rgba[4];
	int		i,
			idx,
			rx = 0,
			ry = 0,
			aw,
			ah;


	idx = widget == cp->areas[CPICK_AREA_PRECUR] ? CPICK_AREA_PRECUR :
		CPICK_AREA_PALETTE;
	aw = cp->area_size[idx][0];
	ah = cp->area_size[idx][1];

	mtgex_wj_fpixmap_xy ( widget, x, y, &rx, &ry );
	rx = rx < 0 ? 0 : rx >= aw ? aw - 1 : rx;
	ry = ry < 0 ? 0 : ry >= ah ? ah - 1 : ry;

/*
	Selection data format isn't checked because it's how GTK+2
	does it, reportedly to ignore a bug in (some versions of) KDE - WJ
*/

	if ( data->length != 8 )
	{
		return;
	}

	for ( i = 0; i < 4; i++ )
	{
		rgba[i] = (unsigned char)( ( ( (guint16 *)data->data )[i] + 128
				) / 257 );
	}

	cpick_rgba_at ( cp, widget, rx, ry, NULL, rgba );
}



#define RGB_DND_W 48
#define RGB_DND_H 32



static void set_drag_icon (
	GdkDragContext	* const	context,
	GtkWidget	* const	src,
	unsigned char	* const	rgba
	)
{
	GdkGCValues	sv;
	GdkPixmap	* swatch;


	if ( ! context )
	{
		return;
	}

	swatch = gdk_pixmap_new ( src->window, RGB_DND_W, RGB_DND_H, -1 );
	gdk_gc_get_values ( src->style->black_gc, &sv );
	gdk_rgb_gc_set_foreground ( src->style->black_gc,
		(guint32)MEM_2_INT ( rgba, 0 ) );
	gdk_draw_rectangle ( swatch, src->style->black_gc, TRUE, 0, 0,
		RGB_DND_W, RGB_DND_H );
	gdk_gc_set_foreground ( src->style->black_gc, &sv.foreground );
	gtk_drag_set_icon_pixmap ( context, gtk_widget_get_colormap ( src ),
		swatch, NULL, -2, -2 );
	gdk_pixmap_unref ( swatch );
}

static gboolean cpick_area_event (
	GtkWidget	* const	widget,
	GdkEvent	* const	event,
	cpicker		* const	cp
	)
{
	int		x,
			y,
			rx = 0,
			ry = 0,
			button = 0;
	GdkModifierType	state;
	int		can_drag = (widget == cp->areas[CPICK_AREA_PRECUR]) ||
				(widget == cp->areas[CPICK_AREA_PALETTE]);


	if ( event->type == GDK_BUTTON_PRESS )
	{
		x = (int)event->button.x;
		y = (int)event->button.y;
		button = (int)event->button.button;

		// Clicks outside pixmap are ignored
		can_drag &= mtgex_wj_fpixmap_xy ( widget, x, y, &rx, &ry );

		if (	can_drag &&
			(button == 1)
			)		// Only left button inits drag
		{
			cp->drag_x = rx;
			cp->drag_y = ry;
			cp->may_drag = TRUE;
		}

		gtk_widget_grab_focus ( widget );
	}
	else if (event->type == GDK_BUTTON_RELEASE)
	{
		if ( event->button.button == 1 )
		{
			cp->may_drag = FALSE;
		}
	}
	else if ( event->type == GDK_MOTION_NOTIFY )
	{
		if (event->motion.is_hint)
		{
			gdk_window_get_pointer ( event->motion.window, &x, &y,
				&state );
		}
		else
		{
			x = (int)event->motion.x;
			y = (int)event->motion.y;
			state = event->motion.state;
		}

		mtgex_wj_fpixmap_xy ( widget, x, y, &rx, &ry );

		/* May init drag */
		if ( state & GDK_BUTTON1_MASK )
		{
		/*
			No dragging where not allowed, or without
			clicking on the widget first
		*/

			if (	can_drag	&&
				cp->may_drag	&&
				gtk_drag_check_threshold ( widget,
					cp->drag_x, cp->drag_y, rx, ry )
				)	/* Initiate drag */
			{
				GdkDragContext		* context;


				cp->may_drag = FALSE;

		//	Start drag at current position, not at saved one

				cpick_rgba_at ( cp, widget, rx, ry,
					cp->drag_rgba, NULL );
				context = gtk_drag_begin ( widget, cpick_tlist,
					GDK_ACTION_COPY | GDK_ACTION_MOVE,
					1, event );

				set_drag_icon ( context, widget, cp->drag_rgba
					);

				return TRUE;
			}
		}
		else
		{
			// Release events can be lost
			cp->may_drag = FALSE;
		}

		if ( (state & (GDK_BUTTON1_MASK | GDK_BUTTON3_MASK) ) ==
			(GDK_BUTTON1_MASK | GDK_BUTTON3_MASK) )
		{
			button = 13;
		}
		else if ( state & GDK_BUTTON1_MASK )
		{
			button = 1;
		}
		else if ( state & GDK_BUTTON3_MASK )
		{
			button = 3;
		}
		else if ( state & GDK_BUTTON2_MASK )
		{
			button = 2;
		}
	}

	if ( button )
	{
		cpick_area_mouse ( widget, cp, rx, ry, button );
	}

	return TRUE;
}

static void cpick_realize_area (
	GtkWidget	* const	widget,
	cpicker		* const	cp
	)
{
	static unsigned char const hue[7][3] = {
		{255, 0, 0}, {255, 0, 255}, {0, 0, 255},
		{0, 255, 255}, {0, 255, 0}, {255, 255, 0},
		{255, 0, 0}
			};
	unsigned char	* dest,
			* rgb = NULL;
	char		txt[128];
	int		i,
			k,
			kk,
			w,
			h,
			w3,
			sz,
			x,
			y,
			dd,
			d1,
			hy,
			oy,
			idx;


	if ( ! mtgex_wj_fpixmap_pixmap ( widget ) )
	{
		return;
	}

	if ( ! IS_CPICKER ( cp ) )
	{
		return;
	}

	for ( idx = 0; idx < CPICK_AREA_TOT; idx ++ )
	{
		if ( cp->areas[idx] == widget )
		{
			break;
		}
	}

	if ( idx >= CPICK_AREA_TOT )
	{
		return;
	}

	w = cp->area_size[idx][0];
	h = cp->area_size[idx][1];
	w3 = w * 3; sz = w3 * h;

	switch (idx)
	{
	case CPICK_AREA_PRECUR:
	case CPICK_AREA_PICKER:
		mtgex_wj_fpixmap_fill_rgb ( widget, 0, 0, w, h, 0 );
		break;

	case CPICK_AREA_HUE:
		if ( ! ( dest = rgb = malloc ( (size_t)sz ) ) )
		{
			break;
		}

		for ( hy = y = k = 0; k < 6; k++ )
		{
			oy = hy;
			hy = ( (k + 1) * h ) / 6;
			dd = hy - oy;

			for ( ; y < hy; y++ )
			{
				d1 = y - oy;

				for ( i = 0; i < 3; i++ )
				{
					*dest++ = (unsigned char)(hue[k][i] +
						( ( hue[k + 1][i] - hue[k][i])
						* d1 ) / dd);
				}

				for ( ; i < w3; i++ , dest ++ )
				{
					dest[0] = dest[-3];
				}
			}
		}
		break;

	case CPICK_AREA_PALETTE:
		dd = cp->size / CPICK_PAL_STRIP_ITEMS;

		for ( kk = 0; kk < cp->pal_strips; kk ++ )
		{
			for ( k = 0; k < CPICK_PAL_STRIP_ITEMS; k++ )
			{
				i = kk * CPICK_PAL_STRIP_ITEMS + k;
				snprintf ( txt, 128, "%scpick_pal_%i",
					cpick_prefix_buf, i + 1 );
				i = i < 256 ? MTKIT_RGB_2_INT (
					mem_pal_def[i][0],
					mem_pal_def[i][1],
					mem_pal_def[i][2] ) : 0;

				mtkit_prefs_get_int ( cpick_prefs, txt, &i );
				mtgex_wj_fpixmap_fill_rgb ( widget, dd * kk,
					dd * k, dd, dd, i);
			}
		}
		break;

	case CPICK_AREA_OPACITY:
		if ( ! ( dest = rgb = malloc ( (size_t)sz ) ) )
		{
			break;
		}

		for ( y = h - 1; y >= 0; y-- )
		{
			k = 255 - (255 * y) / h;
			kk = (y >> 3) & 1;

			for ( x = 0; x < w; x++ , dest += 3 )
			{
				i = k * greyz[ ( (x >> 3) & 1 ) ^ kk];
				dest[0] = dest[1] = dest[2] =
					(unsigned char)((i + (i >> 8) + 1) >> 8);
			}
		}
		break;
	}

	if ( rgb )
	{
		mtgex_wj_fpixmap_draw_rgb ( widget, 0, 0, w, h, rgb, w * 3 );
	}

	free ( rgb );

	if (	idx == CPICK_AREA_HUE		||
		idx == CPICK_AREA_PICKER	||
		idx == CPICK_AREA_OPACITY
		)
	{
		mtgex_wj_fpixmap_set_cursor ( widget, (char *)xbm_ring4_bits,
			(char *)xbm_ring4_mask_bits,
			xbm_ring4_width, xbm_ring4_height,
			xbm_ring4_x_hot, xbm_ring4_y_hot, FALSE );

		mtgex_wj_fpixmap_move_cursor ( widget, w / 2, h / 2 );
	}
}

static void cpick_get_rgb (		// Calculate RGB values from HSV
	cpicker		* const	cp
	)
{
	unsigned char	rgb[3];
	double		hsv[3] = {
		(double)cp->input_vals[CPICK_INPUT_HUE] / 255.0,
		(double)cp->input_vals[CPICK_INPUT_SATURATION] / 255.0,
		(double)cp->input_vals[CPICK_INPUT_VALUE]
			};


	hsv2rgb ( rgb, hsv );

	cp->input_vals[CPICK_INPUT_RED]   = rgb[0];
	cp->input_vals[CPICK_INPUT_GREEN] = rgb[1];
	cp->input_vals[CPICK_INPUT_BLUE]  = rgb[2];
}

static void cpick_get_hsv (		// Calculate HSV values from RGB
	cpicker		* const	cp
	)
{
	unsigned char	rgb[3] = {
		(unsigned char)cp->input_vals[CPICK_INPUT_RED],
		(unsigned char)cp->input_vals[CPICK_INPUT_GREEN],
		(unsigned char)cp->input_vals[CPICK_INPUT_BLUE] };
	double		hsv[3];


	rgb2hsv ( rgb, hsv );

	cp->input_vals[CPICK_INPUT_HUE]        = (int)(255 * hsv[0]);
	cp->input_vals[CPICK_INPUT_SATURATION] = (int)(255 * hsv[1]);
	cp->input_vals[CPICK_INPUT_VALUE]      = (int)(hsv[2]);
}

static void cpick_update (
	cpicker		* const	cp,
	int		const	what
	)
{
	int		new_rgb = FALSE,
			new_h = FALSE,
			new_sv = FALSE;


	switch ( what )
	{
	case CPICK_INPUT_RED:
	case CPICK_INPUT_GREEN:
	case CPICK_INPUT_BLUE:
	case CPICK_INPUT_HEX:
		new_rgb = TRUE;
		break;

	case CPICK_INPUT_HUE:
		new_h = TRUE;
		break;

	case CPICK_INPUT_SATURATION:
	case CPICK_INPUT_VALUE:
		new_sv = TRUE;
		break;

	case CPICK_INPUT_OPACITY:
		break;

	default:
		return;
	}

	if (	new_h	||
		new_sv	||
		new_rgb
		)
	{
		if ( new_rgb )
		{
			cpick_get_hsv ( cp );
		}
		else
		{
			cpick_get_rgb ( cp );
		}

		cpick_populate_inputs ( cp );	// Update all inputs in dialog

		// New RGB or Hue so recalc picker
		if ( ! new_sv )
		{
			cpick_area_picker_create ( cp );
		}
	}

	cpick_area_update_cursors ( cp );

	// Update current colour
	cpick_area_precur_create ( cp, CPICK_AREA_CURRENT );

	gtk_signal_emit ( GTK_OBJECT ( cp ), cpicker_signals[COLOR_CHANGED] );
}

static gboolean cpick_hex_change (
	GtkWidget	* const	widget,
	GdkEventFocus	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	cpicker		* const	cp = gtk_object_get_data ( GTK_OBJECT ( widget),
				CPICK_KEY );
	GdkColor	col;
	int		r,
			g,
			b;


	if ( ! cp || cp->lock )
	{
		return FALSE;
	}

	if ( ! gdk_color_parse ( gtk_entry_get_text (
		GTK_ENTRY ( cp->inputs[CPICK_INPUT_HEX] ) ), &col ) )
	{
		return FALSE;
	}

	r = ( (int)col.red + 128 ) / 257;
	g = ( (int)col.green + 128 ) / 257;
	b = ( (int)col.blue + 128 ) / 257;

	if ( ! ( (r ^ cp->input_vals[CPICK_INPUT_RED]) |
		(g ^ cp->input_vals[CPICK_INPUT_GREEN]) |
		(b ^ cp->input_vals[CPICK_INPUT_BLUE] ) )
		)
	{
		return FALSE;
	}

	cp->input_vals[CPICK_INPUT_RED] = r;
	cp->input_vals[CPICK_INPUT_GREEN] = g;
	cp->input_vals[CPICK_INPUT_BLUE] = b;
	cpick_update ( cp, CPICK_INPUT_HEX );

	return FALSE;
}

static void cpick_spin_change (
	GtkAdjustment	* const	adjustment,
	gpointer	const	user_data
	)
{
	cpicker		* cp;
	int		i,
			input = (int)(intptr_t)user_data;


	cp = gtk_object_get_data ( GTK_OBJECT ( adjustment ),
		CPICK_KEY);

	if ( ! cp || cp->lock )
	{
		return;
	}

	i = gtk_spin_button_get_value_as_int (
		GTK_SPIN_BUTTON ( cp->inputs[input] ) );

	if ( cp->input_vals[input] == i )
	{
		return;
	}

	cp->input_vals[input] = i;
	cpick_update ( cp, input );
}

static void dropper_terminate (
	GtkWidget	* const	ARG_UNUSED ( widget ),
	gpointer	const	user_data
	)
{
	cpicker		* const	cp	= user_data;
	GtkWidget	* const	widget	= cp->hbox;
	guint32		const	time	= gtk_get_current_event_time ();
	GdkDisplay	* const	display	= gtk_widget_get_display ( widget );


	gdk_display_keyboard_ungrab ( display, time );
	gdk_display_pointer_ungrab ( display, time );

	gtk_signal_disconnect_by_data ( GTK_OBJECT ( widget ), user_data );
	gtk_grab_remove ( widget );
}

static void dropper_grab_colour (
	GtkWidget	* const	widget,
	gint		const	x,
	gint		const	y,
	gpointer	const	user_data
	)
{
	cpicker		* const	cp = user_data;
	unsigned char	rgb[3] = {0,0,0};


	if ( ! mtgex_wj_get_rgb_image ( gdk_get_default_root_window (), NULL,
		rgb, x, y, 1, 1 ) )
	{
		return;
	}

	/* Ungrab before sending signal - better safe than sorry */
	dropper_terminate ( widget, user_data );

	mtgex_cpick_set_color ( GTK_WIDGET ( cp ), MEM_2_INT ( rgb, 0 ),
		cp->input_vals[CPICK_INPUT_OPACITY] );
	gtk_signal_emit ( GTK_OBJECT ( cp ), cpicker_signals[COLOR_CHANGED] );
}

static gboolean dropper_key_press (
	GtkWidget	* const	widget,
	GdkEventKey	* const	event,
	gpointer	const	user_data
	)
{
	int		x,
			y;


	if ( event->keyval == GDK_Escape )
	{
		dropper_terminate ( widget, user_data );
	}
	else if ( event->keyval == GDK_Return	||
		event->keyval == GDK_KP_Enter	||
		event->keyval == GDK_space	||
		event->keyval == GDK_KP_Space
		)
	{
		gdk_display_get_pointer ( gtk_widget_get_display ( widget ),
			NULL, &x, &y, NULL );

		dropper_grab_colour ( widget, x, y, user_data );
	}

	return TRUE;
}

static gboolean dropper_mouse_press (
	GtkWidget	* const	widget,
	GdkEventButton	* const	event,
	gpointer	const	user_data
	)
{
	if ( event->type != GDK_BUTTON_RELEASE )
	{
		return FALSE;
	}

	dropper_grab_colour ( widget, (gint)event->x_root, (gint)event->y_root,
		user_data );

	return TRUE;
}

static GdkCursor * mtgex_make_cursor (	// Create pixmap cursor
	char	const	* const	icon,
	char	const	* const	mask,
	int		const	w,
	int		const	h,
	int		const	tip_x,
	int		const	tip_y
	)
{
	static GdkColor	cfg = { (guint)-1, (guint8)-1, (guint8)-1, (guint8)-1 },
			cbg = { 0, 0, 0, 0 };
	GdkPixmap	* icn,
			* msk;
	GdkCursor	* cursor;


	icn = gdk_bitmap_create_from_data ( NULL, icon, w, h );
	msk = gdk_bitmap_create_from_data ( NULL, mask, w, h );
	cursor = gdk_cursor_new_from_pixmap ( icn, msk, &cfg, &cbg, tip_x,
		tip_y );
	gdk_pixmap_unref ( icn );
	gdk_pixmap_unref ( msk );

	return cursor;
}

static void cpick_eyedropper (
	GtkButton	* const	ARG_UNUSED ( button ),
	gpointer	const	user_data
	)
{
	cpicker		* const	cp = user_data;
	GtkWidget	* widget = cp->hbox;
	GdkCursor	* cursor;
	GdkGrabStatus	grab_status;


	if ( gdk_keyboard_grab ( widget->window, FALSE,
		gtk_get_current_event_time () ) != GDK_GRAB_SUCCESS )
	{
		return;
	}

	cursor = mtgex_make_cursor ( (char *)xbm_picker_bits,
		(char *)xbm_picker_mask_bits, xbm_picker_width,
		xbm_picker_height, xbm_picker_x_hot, xbm_picker_y_hot);

	grab_status = gdk_pointer_grab (widget->window, FALSE,
		GDK_BUTTON_RELEASE_MASK | GDK_BUTTON_PRESS_MASK |
		GDK_POINTER_MOTION_MASK,
		NULL, cursor, gtk_get_current_event_time () );

	gdk_cursor_destroy ( cursor );

	if ( grab_status != GDK_GRAB_SUCCESS )
	{
		gdk_display_keyboard_ungrab ( gtk_widget_get_display ( widget ),
			GDK_CURRENT_TIME);

		return;
	}

	gtk_grab_add ( widget );

	gtk_signal_connect ( GTK_OBJECT ( widget ), "button_release_event",
		GTK_SIGNAL_FUNC ( dropper_mouse_press ), cp );
	gtk_signal_connect ( GTK_OBJECT ( widget ), "key_press_event",
		GTK_SIGNAL_FUNC ( dropper_key_press ), cp);
}

static int mtgex_arrow_key (		// Interpreting arrow keys
	GdkEventKey	* const	event,
	int		* const	dx,
	int		* const	dy,
	int			mult
	)
{
	if ( ( event->state & (GDK_SHIFT_MASK | GDK_CONTROL_MASK) ) !=
		GDK_SHIFT_MASK )
	{
		mult = 1;
	}

	dx[0] = dy[0] = 0;

	switch ( event->keyval )
	{
		case GDK_KP_Left:
		case GDK_Left:
			dx[0] = -mult;
			break;

		case GDK_KP_Right:
		case GDK_Right:
			dx[0] = mult;
			break;

		case GDK_KP_Up:
		case GDK_Up:
			dy[0] = -mult;
			break;

		case GDK_KP_Down:
		case GDK_Down:
			dy[0] = mult;
			break;
	}

	return ( dx[0] || dy[0] );
}

static gboolean cpick_area_key (
	GtkWidget	* const	widget,
	GdkEventKey	* const	event,
	cpicker		* const	cp
	)
{
	int		dx,
			dy;


	if ( ! mtgex_arrow_key ( event, &dx, &dy, 16 ) )
	{
		return FALSE;
	}

	if ( widget == cp->areas[CPICK_AREA_PICKER] )
	{
		int	new_sat = cp->input_vals[CPICK_INPUT_SATURATION] - dy,
			new_val = cp->input_vals[CPICK_INPUT_VALUE] + dx;


		new_sat = new_sat < 0 ? 0 : new_sat > 255 ? 255 : new_sat;
		new_val = new_val < 0 ? 0 : new_val > 255 ? 255 : new_val;

		if (	new_sat != cp->input_vals[CPICK_INPUT_SATURATION] ||
			new_val != cp->input_vals[CPICK_INPUT_VALUE]
			)
		{
			cp->input_vals[CPICK_INPUT_SATURATION] = new_sat;
			cp->input_vals[CPICK_INPUT_VALUE] = new_val;
			cpick_get_rgb ( cp );		// Update RGB values
			cpick_area_update_cursors ( cp ); // Update cursors
			cpick_refresh_inputs_areas ( cp ); // Update inputs

			gtk_signal_emit ( GTK_OBJECT ( cp ),
				cpicker_signals[COLOR_CHANGED] );
		}
	}
	else if ( ! dy )
	{
		// X isn't used anywhere else
	}
	else if ( widget == cp->areas[CPICK_AREA_OPACITY] )
	{
		int		new_opac;


		new_opac = cp->input_vals[CPICK_INPUT_OPACITY] - dy;
		new_opac = new_opac < 0 ? 0 : new_opac > 255 ? 255 : new_opac;

		if ( new_opac != cp->input_vals[CPICK_INPUT_OPACITY] )
		{
			cp->input_vals[CPICK_INPUT_OPACITY] = new_opac;
			cpick_area_update_cursors ( cp );
			cpick_populate_inputs ( cp );

			// Update all inputs in dialog

			cpick_area_precur_create ( cp, CPICK_AREA_CURRENT );
			gtk_signal_emit ( GTK_OBJECT ( cp ),
				cpicker_signals[COLOR_CHANGED] );
		}
	}
	else if (widget == cp->areas[CPICK_AREA_HUE])
	{
		int		new_hue;


		new_hue = cp->input_vals[CPICK_INPUT_HUE] - 8 * dy;
		new_hue = new_hue < 0 ? 0 : new_hue > 1529 ? 1529 : new_hue;

		if ( new_hue != cp->input_vals[CPICK_INPUT_HUE] )
		{
			cp->input_vals[CPICK_INPUT_HUE] = new_hue; // Change hue
			cpick_get_rgb ( cp );		// Update RGB values
			cpick_area_update_cursors ( cp ); // Update cursors
			cpick_area_picker_create ( cp ); // Repaint picker
			cpick_refresh_inputs_areas ( cp ); // Update inputs
			gtk_signal_emit ( GTK_OBJECT ( cp ),
				cpicker_signals[COLOR_CHANGED] );
		}
	}

	return TRUE;
}

static GtkWidget * mtgex_add_a_table (
	int		const	rows,
	int		const	columns,
	int		const	bord,
	GtkWidget	* const	box
	)
{
	GtkWidget	* table;


	table = mtgex_pack ( box, gtk_table_new ( (guint)rows, (guint)columns,
		FALSE ) );

	gtk_widget_show ( table );
	gtk_container_set_border_width ( GTK_CONTAINER ( table ), (guint)bord );

	return table;
}

static void cpicker_init (
	cpicker		* const	cp
	)
{
	static unsigned char const pos[CPICK_AREA_TOT][2] = {
			{1,1}, {0,1}, {0,2}, {0,0}, {0,3}
			};
	static short const input_vals[CPICK_INPUT_TOT][3] = {
			{0,0,255}, {0,0,255}, {0,0,255}, {0,0,1529},
			{255,0,255}, {255,0,255}, {-1,-1,-1}, {128,0,255}
			};
	char	const	* in_txt[CPICK_INPUT_TOT] = {
				_("Red"),
				_("Green"),
				_("Blue"),
				_("Hue"),
				_("Saturation"),
				_("Value"),
				_("Hex"),
				_("Opacity")
				};
	GtkWidget	* widget,
			* hbox,
			* button,
			* table,
			* label,
			* iconw;
	GtkObject	* obj;
	GdkPixmap	* icon,
			* mask;
	int		i;


	cp->size = CPICK_SIZE_DEFAULT;
	cp->pal_strips = CPICK_PAL_STRIPS_DEFAULT;

	cp->rgb_previous[3] = 255;
	cp->input_vals[CPICK_INPUT_OPACITY] = 255;

	cp->area_size[CPICK_AREA_PRECUR][0] = cp->size;
	cp->area_size[CPICK_AREA_PRECUR][1] = 3 * cp->size / 16;
	cp->area_size[CPICK_AREA_PICKER][0] = cp->size;
	cp->area_size[CPICK_AREA_PICKER][1] = cp->size;
	cp->area_size[CPICK_AREA_HUE][0] = 3 * cp->size / 16;
	cp->area_size[CPICK_AREA_HUE][1] = cp->size;
	cp->area_size[CPICK_AREA_PALETTE][0] = cp->pal_strips * cp->size /
		CPICK_PAL_STRIP_ITEMS;
	cp->area_size[CPICK_AREA_PALETTE][1] = cp->size;
	cp->area_size[CPICK_AREA_OPACITY][0] = 3 * cp->size / 16;
	cp->area_size[CPICK_AREA_OPACITY][1] = cp->size;

	hbox = gtk_hbox_new ( FALSE, 2 );
	gtk_widget_show ( hbox );

	cp->hbox = hbox;
	gtk_container_add ( GTK_CONTAINER ( cp ), hbox );

	// --- Palette/Mixer table

	table = mtgex_add_a_table ( 2, 4, 0, hbox );

	for ( i = 0; i < CPICK_AREA_TOT; i++ )
	{
		widget = cp->areas[i] = mtgex_wj_fpixmap ( cp->area_size[i][0],
			cp->area_size[i][1] );
		gtk_table_attach ( GTK_TABLE ( table ), widget,
			(guint)pos[i][1], (guint)(pos[i][1] + 1),
			(guint)pos[i][0], (guint)(pos[i][0] + 1),
			(GtkAttachOptions)0, (GtkAttachOptions)0, 0, 0);
		gtk_signal_connect ( GTK_OBJECT ( widget ), "realize",
			GTK_SIGNAL_FUNC ( cpick_realize_area ), cp );
		gtk_signal_connect ( GTK_OBJECT ( widget ),
			"button_press_event",
			GTK_SIGNAL_FUNC ( cpick_area_event ), cp );
		gtk_signal_connect ( GTK_OBJECT ( widget ),
			"button_release_event",
			GTK_SIGNAL_FUNC ( cpick_area_event ), cp );
		gtk_signal_connect ( GTK_OBJECT ( widget ),
			"motion_notify_event",
			GTK_SIGNAL_FUNC ( cpick_area_event ), cp );

		if (	i == CPICK_AREA_PRECUR	||
			i == CPICK_AREA_PALETTE
			)
		{
/* !!! Maybe handle "drag_motion" & "drag_drop" instead of GTK_DEST_DEFAULT_*,
 * !!! to prevent drops on borders outside of pixmap? */

			gtk_drag_dest_set ( widget,
				GTK_DEST_DEFAULT_HIGHLIGHT |
				GTK_DEST_DEFAULT_MOTION |
				GTK_DEST_DEFAULT_DROP,
				&cpick_target, 1, GDK_ACTION_COPY );
			gtk_signal_connect ( GTK_OBJECT ( widget ),
				"drag_data_get",
				GTK_SIGNAL_FUNC ( cpick_drag_get ), cp );
			gtk_signal_connect ( GTK_OBJECT ( widget ),
				"drag_data_received",
				GTK_SIGNAL_FUNC ( cpick_drag_set ), cp );
		}

		if (	i == CPICK_AREA_PICKER	||
			i == CPICK_AREA_HUE	||
			i == CPICK_AREA_OPACITY
			)
		{
			gtk_signal_connect ( GTK_OBJECT ( widget ),
				"key_press_event",
				GTK_SIGNAL_FUNC ( cpick_area_key ), cp );
		}
	}

	button = gtk_button_new ();

	icon = gdk_pixmap_create_from_data ( cpick_main_window->window,
		(char *)xbm_picker_bits, xbm_picker_width, xbm_picker_height,
		-1, &cpick_main_window->style->white,
		&cpick_main_window->style->black );

	mask = gdk_bitmap_create_from_data ( cpick_main_window->window,
		(char *)xbm_picker_mask_bits, xbm_picker_width,
		xbm_picker_height );

	iconw = gtk_pixmap_new ( icon, mask );
	gtk_widget_show ( iconw );
	gdk_pixmap_unref ( icon );
	gdk_pixmap_unref ( mask );
	gtk_container_add ( GTK_CONTAINER ( button ), iconw );

	gtk_table_attach ( GTK_TABLE ( table ), button, 2, 3, 1, 2,
		(GtkAttachOptions) GTK_FILL, (GtkAttachOptions) GTK_FILL,
		2, 2 );
	gtk_widget_show ( button );
	gtk_signal_connect ( GTK_OBJECT ( button ), "clicked",
		GTK_SIGNAL_FUNC ( cpick_eyedropper ), cp );

	// --- Table for inputs on right hand side

	table = mtgex_add_a_table ( 8, 2, 0, hbox );

	for ( i = 0; i < CPICK_INPUT_TOT; i++ )
	{
		label = mtgex_add_to_table ( in_txt[i], table, i, 0, 2, 2);

		if ( i == CPICK_INPUT_OPACITY )
		{
			cp->opacity_label = label;
		}

		gtk_misc_set_alignment ( GTK_MISC ( label ), 1.0, 0.5 );

		if ( i == CPICK_INPUT_HEX )
		{
			cp->inputs[i] = gtk_entry_new ();

			gtk_entry_set_width_chars (
				GTK_ENTRY ( cp->inputs[i] ), 9 );
			obj = GTK_OBJECT ( cp->inputs[i] );

			gtk_signal_connect ( obj, "focus_out_event",
				GTK_SIGNAL_FUNC ( cpick_hex_change ),
				(gpointer)(intptr_t) i );
		}
		else
		{
			cp->inputs[i] = mtgex_add_a_spin ( input_vals[i][0],
				input_vals[i][1], input_vals[i][2] );

			obj = GTK_OBJECT ( GTK_SPIN_BUTTON ( cp->inputs[i]
				)->adjustment );

			gtk_signal_connect ( obj, "value_changed",
				GTK_SIGNAL_FUNC ( cpick_spin_change ),
				(gpointer)(intptr_t) i );
		}

		gtk_object_set_data ( obj, CPICK_KEY, cp );
		gtk_widget_show ( cp->inputs[i] );
		gtk_table_attach ( GTK_TABLE ( table ), cp->inputs[i],
				1, 2, (guint)i, (guint)(i + 1),
				(GtkAttachOptions) GTK_FILL,
				(GtkAttachOptions) 0, 0, 0);
	}

	gtk_widget_grab_focus ( cp->inputs[0] );
}

GtkWidget * mtgex_cpick_new (
	GtkWidget	* const	main_window,
	mtPrefs		* const	prefs,
	char	const	* const	prefs_prefix
	)
{
	cpick_main_window = main_window;
	cpick_prefs = prefs;

	cpick_prefix_buf[0] = 0;

	// prefs_prefix check by function
	mtkit_strnncpy ( cpick_prefix_buf, prefs_prefix,CPICK_PREFIX_BUF_SIZE );

	return gtk_widget_new ( cpicker_get_type (), NULL );
}

/* These formulas perfectly reverse ones in cpick_area_mouse () when possible;
 * however, for sizes > 255 it's impossible in principle - WJ */
static void cpick_area_update_cursors (
	cpicker		* const	cp
	)
{
	int		x, y, l;


	l = cp->area_size[CPICK_AREA_PICKER][0] - 1;
	x = (cp->input_vals[CPICK_INPUT_VALUE] * l + l - 1) / 255;
	l = cp->area_size[CPICK_AREA_PICKER][1] - 1;
	y = ( ( 255 - cp->input_vals[CPICK_INPUT_SATURATION] ) * l + l - 1 ) /
		255;
	mtgex_wj_fpixmap_move_cursor ( cp->areas[CPICK_AREA_PICKER], x, y );

	x = cp->area_size[CPICK_AREA_HUE][0] / 2;
	l = cp->area_size[CPICK_AREA_HUE][1] - 1;
	y = ( ( 1529 - cp->input_vals[CPICK_INPUT_HUE] ) * l + l - 1 ) / 1529;
	mtgex_wj_fpixmap_move_cursor ( cp->areas[CPICK_AREA_HUE], x, y );

	x = cp->area_size[CPICK_AREA_OPACITY][0] / 2;
	l = cp->area_size[CPICK_AREA_OPACITY][1] - 1;
	y = ( ( 255 - cp->input_vals[CPICK_INPUT_OPACITY] ) * l + l - 1 ) / 255;
	mtgex_wj_fpixmap_move_cursor ( cp->areas[CPICK_AREA_OPACITY], x, y );
}

/* Update whole dialog according to values */
static void cpick_refresh_inputs_areas (
	cpicker		* const	cp
	)
{
	cpick_populate_inputs ( cp );		// Update all inputs in dialog

	// Update current colour
	cpick_area_precur_create ( cp, CPICK_AREA_CURRENT );

	cpick_area_picker_create ( cp );	// Update picker colours
	cpick_area_update_cursors ( cp );	// Update area cursors
}

int mtgex_cpick_get_color (
	GtkWidget	* const	w,
	int		* const	opacity
	)
{
	int		i;
	cpicker		* cp = CPICKER ( w );


	if ( ! IS_CPICKER ( cp ) )
	{
		return 0;
	}

	for ( i = 0; i < CPICK_INPUT_HEX; i++ )
	{
		gtk_spin_button_update ( GTK_SPIN_BUTTON ( cp->inputs[i] ) );
	}

	if ( opacity )
	{
		opacity[0] = cp->input_vals[CPICK_INPUT_OPACITY];
	}

	return MTKIT_RGB_2_INT ( cp->input_vals[CPICK_INPUT_RED],
		cp->input_vals[CPICK_INPUT_GREEN],
		cp->input_vals[CPICK_INPUT_BLUE] );
}

void mtgex_cpick_set_color (
	GtkWidget	* const	w,
	int		const	rgb,
	int		const	opacity
	)
{
	cpicker		* const	cp = CPICKER ( w );


	if ( ! IS_CPICKER ( cp ) )
	{
		return;
	}

	cp->input_vals[CPICK_INPUT_RED] = MTKIT_INT_2_R ( rgb );
	cp->input_vals[CPICK_INPUT_GREEN] = MTKIT_INT_2_G ( rgb );
	cp->input_vals[CPICK_INPUT_BLUE] = MTKIT_INT_2_B ( rgb );
	cp->input_vals[CPICK_INPUT_OPACITY] = opacity & 0xFF;

	cpick_get_hsv ( cp );

	cpick_refresh_inputs_areas ( cp );	// Update everything
}

void mtgex_cpick_set_color_previous (
	GtkWidget	* const	w,
	int		const	rgb,
	int		const	opacity
	)
{
	cpicker		* const	cp = CPICKER ( w );


	if ( ! IS_CPICKER ( cp ) )
	{
		return;
	}

	cp->rgb_previous[0] = MTKIT_INT_2_R ( rgb );
	cp->rgb_previous[1] = MTKIT_INT_2_G ( rgb );
	cp->rgb_previous[2] = MTKIT_INT_2_B ( rgb );
	cp->rgb_previous[3] = opacity & 0xFF;

	// Update previous colour
	cpick_area_precur_create ( cp, CPICK_AREA_PREVIOUS );
}

void mtgex_cpick_set_opacity_visibility (
	GtkWidget	* const	w,
	int		const	visible
	)
{
	void		(* showhide)(GtkWidget * w);
	cpicker		* const	cp = CPICKER ( w );


	if ( ! IS_CPICKER ( cp ) )
	{
		return;
	}

	showhide = visible ? gtk_widget_show : gtk_widget_hide;

	showhide ( cp->areas[CPICK_AREA_OPACITY] );
	showhide ( cp->inputs[CPICK_INPUT_OPACITY] );
	showhide ( cp->opacity_label );
}

