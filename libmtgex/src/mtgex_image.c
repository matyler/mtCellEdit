/*
	Copyright (C) 2009-2016 Mark Tyler

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



#define GEXIMAGE_DATA_KEY "mtGEX.image_data"



static void realize_trick (
	GtkWidget	* const	widget,
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	gdk_window_set_back_pixmap ( widget->window, NULL, FALSE );
}

static void viewport_style (
	GtkWidget	* const	widget
	)
{
	static GtkStyle	* defstyle;
	GdkColor	* col;


	if ( ! defstyle )
	{
		defstyle = gtk_style_new ();
		col = &defstyle->bg[GTK_STATE_NORMAL];
		col->red = col->green = col->blue = 0xD6D6;
		defstyle->xthickness = defstyle->ythickness = 1;
	}

	gtk_widget_set_style ( widget, defstyle );
}

void mtgex_fix_darea (
	GtkWidget	* const	darea
	)
{
	gtk_signal_connect_after ( GTK_OBJECT ( darea ), "realize",
		GTK_SIGNAL_FUNC ( realize_trick ), NULL );
	gtk_widget_set_double_buffered ( darea, FALSE );
}

static void mtgex_fix_vport (		// Stop theme engines from messing up
					// viewport's frame
	GtkWidget	* const	vport
	)
{
	/* Stop theme engines from messing up viewport's frame */

	viewport_style ( vport );
	gtk_signal_connect_after ( GTK_OBJECT ( vport ), "realize",
		GTK_SIGNAL_FUNC ( realize_trick ), NULL );
	gtk_widget_set_double_buffered ( vport, FALSE );

	mtgex_fix_darea ( GTK_BIN ( vport )->child );
}

static gboolean configure_gimage_canvas (
	GtkWidget	* const	widget,
	GdkEventConfigure * const	ARG_UNUSED ( event ),
	gpointer	const	user_data
	)
{
	int		ww = widget->allocation.width,
			wh = widget->allocation.height,
			new_margin_x,
			new_margin_y,
			cw,
			ch,
			iw = 0,
			ih = 0
			;
	gexImage	* const	gimage = user_data;


	if ( ! gimage )
	{
		return TRUE;
	}

	if ( gimage->image )
	{
		iw = mtkit_image_get_width ( gimage->image );
		ih = mtkit_image_get_height ( gimage->image );
	}

	cw = gimage->zoom * iw;
	ch = gimage->zoom * ih;

	if ( ww > cw )
	{
		new_margin_x = ( ww - cw ) / 2;
	}
	else
	{
		new_margin_x = 0;
	}

	if ( wh > ch )
	{
		new_margin_y = ( wh - ch ) / 2;
	}
	else
	{
		new_margin_y = 0;
	}

	if (	new_margin_x != gimage->margin_x ||
		new_margin_y != gimage->margin_y
		)
	{
		gtk_widget_queue_draw ( gimage->drawing_area );

		gimage->margin_x = new_margin_x;
		gimage->margin_y = new_margin_y;
	}

	return TRUE;
}

static gboolean expose_gimage_canvas (
	GtkWidget	* const	widget,
	GdkEventExpose	* const	event,
	gpointer	const	user_data
	)
{
	unsigned char	* rgb,
			* src;
	int		px,
			py,
			pw,
			ph;
	gexImage	* const	gimage = user_data;


	if ( ! widget || ! event || ! gimage )
	{
		return TRUE;
	}

	px = event->area.x;
	py = event->area.y;
	pw = event->area.width;
	ph = event->area.height;

	if ( pw < 1 || ph < 1 )
	{
		return TRUE;
	}

	rgb = malloc ( (size_t)(3 * pw * ph) );

	if ( ! rgb )
	{
		return TRUE;
	}

	memset ( rgb, gimage->background, (size_t)(3 * pw * ph) );

	src = mtkit_image_get_rgb ( gimage->image );

	if ( src )
	{
		unsigned char	* pix,
				* dest;
		int		pw2,
				ph2,
				nix = 0,
				niy = 0,
				zoom = gimage->zoom,
				iw,
				ih,
				px2,
				py2,
				x,
				y
				;


		px2 = px - gimage->margin_x;
		py2 = py - gimage->margin_y;

		iw = mtkit_image_get_width ( gimage->image );
		ih = mtkit_image_get_height ( gimage->image );

		pw2 = pw;
		ph2 = ph;

		if ( px2 < 0 )
		{
			nix = -px2;
		}

		if ( py2 < 0 )
		{
			niy = -py2;
		}

		if ( ( px2 + pw2 ) >= iw * zoom )
		{
			// Update image + blank space outside

			pw2 = iw * zoom - px2;
		}
		if ( ( py2 + ph2 ) >= ih * zoom )
		{
			// Update image + blank space outside

			ph2 = ih * zoom - py2;
		}

		for ( y = niy; y < ph2; y++ )
		{
			dest = rgb + 3 * ( y * pw + nix );
			for ( x = nix; x < pw2; x++ )
			{
				pix = src + 3 * ( ( px2 + x ) / zoom +
					( py2 + y ) / zoom * iw );

				*dest++ = pix[0];
				*dest++ = pix[1];
				*dest++ = pix[2];
			}
		}
	}

	gdk_draw_rgb_image ( widget->window, widget->style->black_gc,
		px, py, pw, ph, GDK_RGB_DITHER_NONE, rgb, pw * 3 );

	free ( rgb );

	return TRUE;
}

static gboolean destroy_gimage (
	GtkWidget	* const	widget,
	GdkEvent	* const	ARG_UNUSED ( event ),
	gpointer	const	ARG_UNUSED ( user_data )
	)
{
	gexImage	* gimage;


	if ( mtgex_image_get_geximage ( widget, &gimage ) )
	{
		fprintf ( stderr,
			"mtGEX error: destroy_gimage unable to find gimage\n" );

		return FALSE;
	}

	if ( ! gimage )
	{
		return FALSE;
	}

	// Stop all callbacks to avoid any dangling references to to destroyed
	// gimage

	gtk_signal_disconnect_by_func ( GTK_OBJECT ( gimage->drawing_area ),
		GTK_SIGNAL_FUNC ( configure_gimage_canvas ), gimage );

	gtk_signal_disconnect_by_func ( GTK_OBJECT ( gimage->drawing_area ),
		GTK_SIGNAL_FUNC ( expose_gimage_canvas ), gimage );

	gtk_object_remove_data ( GTK_OBJECT ( gimage->scrolled_window ),
		GEXIMAGE_DATA_KEY );

	if ( gimage->own )
	{
		mtkit_image_destroy ( gimage->image );
	}

	free ( gimage );

	return FALSE;
}

GtkWidget * mtgex_image_new ( void )
{
	gexImage	* gimage;
	GtkWidget	* sw,
			* darea;


	gimage = calloc ( sizeof ( gexImage ), 1 );
	if ( ! gimage )
	{
		return NULL;
	}

	sw = gtk_scrolled_window_new ( NULL, NULL );
	if ( ! sw )
	{
		free ( gimage );

		return NULL;
	}

	gtk_widget_show ( sw );
	gtk_scrolled_window_set_policy ( GTK_SCROLLED_WINDOW ( sw ),
		GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC );

	gtk_object_set_data ( GTK_OBJECT ( sw ), GEXIMAGE_DATA_KEY, gimage );

	// Set up default values
	gimage->zoom = 1;
	gimage->background = 180;
	gimage->scrolled_window = sw;

	gtk_signal_connect ( GTK_OBJECT ( sw ), "destroy",
		GTK_SIGNAL_FUNC ( destroy_gimage ), gimage );

	darea = gtk_drawing_area_new ();
	if ( ! darea )
	{
		gtk_widget_destroy ( sw );

		return NULL;
	}

	gtk_widget_show ( darea );
	gimage->drawing_area = darea;

	gtk_scrolled_window_add_with_viewport ( GTK_SCROLLED_WINDOW ( sw ),
		darea );
	mtgex_fix_vport ( GTK_BIN ( sw )->child );

	gtk_signal_connect ( GTK_OBJECT ( darea ), "expose_event",
		GTK_SIGNAL_FUNC ( expose_gimage_canvas ), gimage );

	gtk_signal_connect ( GTK_OBJECT ( darea ), "configure_event",
		GTK_SIGNAL_FUNC ( configure_gimage_canvas ), gimage );

	return sw;
}

int mtgex_image_set_image (
	GtkWidget	* const	widget,
	mtImage		* const	image,
	int		const	own
	)
{
	int		w, h;
	gexImage	* gimage;


	if ( mtgex_image_get_geximage ( widget, &gimage ) )
	{
		return 1;
	}

	if ( ! gimage )
	{
		return 1;
	}

	// Destroy old mtImage if we own it
	if ( gimage->own )
	{
		mtkit_image_destroy ( gimage->image );
	}

	gimage->image = image;
	gimage->own = own;

	w = mtkit_image_get_width ( image );
	h = mtkit_image_get_height ( image );

	w *= gimage->zoom;
	h *= gimage->zoom;
	gtk_widget_set_usize ( gimage->drawing_area, w, h );
	gtk_widget_queue_draw ( gimage->drawing_area );

	return 0;
}

int mtgex_image_get_geximage (
	GtkWidget	*	const	widget,
	gexImage	**	const	geximage
	)
{
	if ( ! widget || ! geximage )
	{
		return 1;
	}

	geximage[0] = gtk_object_get_data ( GTK_OBJECT ( widget ),
		GEXIMAGE_DATA_KEY );

	return 0;
}

