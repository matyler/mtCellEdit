/*
	Copyright (C) 2016-2018 Mark Tyler

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

#include "qt4.h"



void Mainwindow::update_menus ()
{
	bool	const	b = backend.clipboard.get_image () ? true : false;

	for ( int i = 0; i < CLIPBOARD_MENU_TOTAL; i++ )
	{
		act_edit_save_clipboard[i]->setEnabled ( b );
	}

	act_edit_paste->setEnabled ( b );
	act_edit_paste_centre->setEnabled ( b );

	int	const	undo_steps = backend.file.get_undo_steps ();
	int	const	redo_steps = backend.file.get_redo_steps ();


	if ( undo_steps > 0 )
	{
		act_edit_undo->setEnabled ( true );
		act_file_export_undo->setEnabled ( true );
	}
	else
	{
		act_edit_undo->setEnabled ( false );
		act_file_export_undo->setEnabled ( false );
	}

	if ( redo_steps > 0 )
	{
		act_edit_redo->setEnabled ( true );
	}
	else
	{
		act_edit_redo->setEnabled ( false );
	}

	if ( is_split_visible () )
	{
		act_options_split_switch->setEnabled ( true );
		act_options_zoom_split_in->setEnabled ( true );
		act_options_zoom_split_out->setEnabled ( true );
		act_options_zoom_split_3->setEnabled ( true );
		act_options_zoom_split_100->setEnabled ( true );
		act_options_zoom_split_3200->setEnabled ( true );
		options_menu_zoom_split->setEnabled ( true );
	}
	else
	{
		act_options_split_switch->setEnabled ( false );
		act_options_zoom_split_in->setEnabled ( false );
		act_options_zoom_split_out->setEnabled ( false );
		act_options_zoom_split_3->setEnabled ( false );
		act_options_zoom_split_100->setEnabled ( false );
		act_options_zoom_split_3200->setEnabled ( false );
		options_menu_zoom_split->setEnabled ( false );
	}

	switch ( backend.file.get_tool_mode () )
	{
	case mtPixyUI::File::TOOL_MODE_SELECT_RECTANGLE:
	case mtPixyUI::File::TOOL_MODE_SELECT_POLYGON:
	case mtPixyUI::File::TOOL_MODE_SELECTING_RECTANGLE:
	case mtPixyUI::File::TOOL_MODE_SELECTING_POLYGON:
		act_edit_cut->setEnabled ( false );
		act_edit_copy->setEnabled ( false );

		act_image_crop->setEnabled ( false );

		act_selection_none->setEnabled ( false );
		act_selection_lasso->setEnabled ( false );
		act_selection_fill->setEnabled ( false );
		act_selection_outline->setEnabled ( false );
		act_selection_flip_h->setEnabled ( false );
		act_selection_flip_v->setEnabled ( false );
		act_selection_rotate_c->setEnabled ( false );
		act_selection_rotate_a->setEnabled ( false );
		break;

	case mtPixyUI::File::TOOL_MODE_SELECTED_RECTANGLE:
	case mtPixyUI::File::TOOL_MODE_SELECTED_POLYGON:
		act_edit_cut->setEnabled ( true );
		act_edit_copy->setEnabled ( true );

		act_image_crop->setEnabled ( true );

		act_selection_none->setEnabled ( true );
		act_selection_lasso->setEnabled ( true );
		act_selection_fill->setEnabled ( true );
		act_selection_outline->setEnabled ( true );
		act_selection_flip_h->setEnabled ( false );
		act_selection_flip_v->setEnabled ( false );
		act_selection_rotate_c->setEnabled ( false );
		act_selection_rotate_a->setEnabled ( false );
		break;

	case mtPixyUI::File::TOOL_MODE_PASTE:
	case mtPixyUI::File::TOOL_MODE_PASTING:
		act_edit_cut->setEnabled ( false );
		act_edit_copy->setEnabled ( false );

		act_image_crop->setEnabled ( false );

		act_selection_none->setEnabled ( false );
		act_selection_lasso->setEnabled ( false );
		act_selection_fill->setEnabled ( false );
		act_selection_outline->setEnabled ( false );
		act_selection_flip_h->setEnabled ( true );
		act_selection_flip_v->setEnabled ( true );
		act_selection_rotate_c->setEnabled ( true );
		act_selection_rotate_a->setEnabled ( true );
		break;

	default:
		act_edit_cut->setEnabled ( false );
		act_edit_copy->setEnabled ( false );

		act_image_crop->setEnabled ( false );

		act_selection_none->setEnabled ( false );
		act_selection_lasso->setEnabled ( false );
		act_selection_fill->setEnabled ( false );
		act_selection_outline->setEnabled ( false );
		act_selection_flip_h->setEnabled ( false );
		act_selection_flip_v->setEnabled ( false );
		act_selection_rotate_c->setEnabled ( false );
		act_selection_rotate_a->setEnabled ( false );
		break;
	}


	mtPixy::Image	* const	im = backend.file.get_image ();


	if ( im && im->get_alpha () )
	{
		act_image_delete_alpha->setEnabled ( true );
	}
	else
	{
		act_image_delete_alpha->setEnabled ( false );
	}

	if ( im && im->get_type () == mtPixy::Image::TYPE_RGB )
	{
		act_file_export_colormap->setEnabled ( true );

		act_image_to_rgb->setEnabled ( false );
		act_image_to_indexed->setEnabled ( true );

		act_palette_merge_duplicates->setEnabled ( false );
		act_palette_remove_unused->setEnabled ( false );
		act_palette_create_from_canvas->setEnabled ( true );
		act_palette_quantize_pnn->setEnabled ( true );

		act_effects_edge_detect->setEnabled ( true );
		act_effects_sharpen->setEnabled ( true );
		act_effects_soften->setEnabled ( true );
		act_effects_emboss->setEnabled ( true );
		act_effects_normalize->setEnabled ( true );

		act_effects_bacteria->setEnabled ( false );
	}
	else if ( im && im->get_type () == mtPixy::Image::TYPE_INDEXED )
	{
		act_file_export_colormap->setEnabled ( false );

		act_image_to_rgb->setEnabled ( true );
		act_image_to_indexed->setEnabled ( false );

		act_palette_merge_duplicates->setEnabled ( true );
		act_palette_remove_unused->setEnabled ( true );
		act_palette_create_from_canvas->setEnabled ( false );
		act_palette_quantize_pnn->setEnabled ( false );

		act_effects_edge_detect->setEnabled ( false );
		act_effects_sharpen->setEnabled ( false );
		act_effects_soften->setEnabled ( false );
		act_effects_emboss->setEnabled ( false );
		act_effects_normalize->setEnabled ( false );

		act_effects_bacteria->setEnabled ( true );
	}
}

void Mainwindow::update_recent_files ()
{
	char		buf[ 2048 ];
	int		i, maxlen, c;


	maxlen = prefs.getInt ( PREFS_FILE_RECENT_MAXLEN );
	if ( maxlen < 50 || maxlen > 150 )
	{
		maxlen = 80;
	}

	for ( c = 0, i = 0; i < RECENT_MENU_TOTAL; i++ )
	{
		if ( 1 == mtkit_snip_filename (
			backend.recent_image.get_filename ( i + 1 ),
			buf, sizeof ( buf ), maxlen )
			)
		{
			// Hide if empty
			act_file_recent[ i ]->setVisible ( false );

			continue;
		}

		act_file_recent[ i ]->setText (
			mtQEX::qstringFromC ( buf ) );

		act_file_recent[ i ]->setVisible ( true );

		c++;		// Count items displayed
	}

	if ( c > 0 )
	{
		act_file_recent_separator->setVisible ( true );
	}
	else
	{
		// Hide separator if not needed
		act_file_recent_separator->setVisible ( false );
	}
}

void Mainwindow::create_menu ()
{
	QAction * act_file_new;
	QAction * act_file_open;
	QAction * act_file_save;
	QAction * act_file_save_as;
	QAction * act_file_quit;

	QEX_MENU ( file_new, "New ...", "Ctrl+N", "document-new" )
	QEX_MENU ( file_open, "Open ...", "Ctrl+O", "document-open" )
	QEX_MENU ( file_save, "Save", "Ctrl+S", "document-save" )
	QEX_MENU ( file_save_as, "Save As ...", "Shift+Ctrl+S",
		"document-save-as")
	QEX_MENU ( file_export_undo, "Export Undo Images ...", NULL,
		"document-save-as" )
	QEX_MENU ( file_export_colormap, "Export Colourmap ...", NULL,
		"document-save-as" )
	QEX_MENU ( file_quit, "Quit", "Ctrl+Q", "application-exit" )

	QMenu * file_menu = menuBar ()->addMenu ( "&File" );
	file_menu->setTearOffEnabled ( true );
	file_menu->addAction ( act_file_new );
	file_menu->addAction ( act_file_open );
	file_menu->addAction ( act_file_save );
	file_menu->addAction ( act_file_save_as );
	file_menu->addSeparator ();
	file_menu->addAction ( act_file_export_undo );
	file_menu->addAction ( act_file_export_colormap );
	act_file_recent_separator = file_menu->addSeparator ();

	QSignalMapper * signal_mapper = new QSignalMapper ( this );

	for ( int i = 0; i < RECENT_MENU_TOTAL; i++ )
	{
		act_file_recent[i] = new QAction ( "", this );

		connect ( act_file_recent[i], SIGNAL ( triggered () ),
			signal_mapper, SLOT ( map () ) );
		signal_mapper->setMapping ( act_file_recent [ i ], i + 1 );

		if ( i < 10 )
		{
			act_file_recent[i]->setShortcut (
				QString ( "Ctrl+%1" ).arg ( (i + 1) % 10 ) );
		}

		file_menu->addAction ( act_file_recent[i] );
	}

	connect ( signal_mapper, SIGNAL ( mapped ( int ) ),
		this, SLOT ( press_file_recent ( int ) ) );

	file_menu->addSeparator ();
	file_menu->addAction ( act_file_quit );

	QAction * act_edit_paste_text;

	QEX_MENU ( edit_undo, "Undo", "Ctrl+Z", "edit-undo" )
	QEX_MENU ( edit_redo, "Redo", "Ctrl+Y", "edit-redo" )
	QEX_MENU ( edit_cut, "Cut", "Ctrl+X", "edit-cut" )
	QEX_MENU ( edit_copy, "Copy", "Ctrl+C", "edit-copy" )
	QEX_MENU ( edit_paste, "Paste", "Ctrl+K", "edit-paste" )
	QEX_MENU ( edit_paste_centre, "Paste To Centre", "Ctrl+V", NULL )
	QEX_MENU ( edit_paste_text, "Paste Text ...", "T", NULL )

	QMenu * edit_menu = menuBar ()->addMenu ( "&Edit" );
	edit_menu->setTearOffEnabled ( true );
	edit_menu->addAction ( act_edit_undo );
	edit_menu->addAction ( act_edit_redo );
	edit_menu->addSeparator ();
	edit_menu->addAction ( act_edit_cut );
	edit_menu->addAction ( act_edit_copy );
	edit_menu->addAction ( act_edit_paste );
	edit_menu->addAction ( act_edit_paste_centre );
	edit_menu->addAction ( act_edit_paste_text );
	edit_menu->addSeparator ();

	QAction		* act_edit_load_clipboard[ CLIPBOARD_MENU_TOTAL ];

	QMenu * edit_menu_load_clip = edit_menu->addMenu( "Load Clipboard" );
	edit_menu_load_clip->setTearOffEnabled ( true );

	signal_mapper = new QSignalMapper ( this );

	for ( int i = 0; i < CLIPBOARD_MENU_TOTAL; i++ )
	{
		act_edit_load_clipboard[i] = new QAction (
			QString ( "%1" ).arg ( i + 1 ), this );

		connect ( act_edit_load_clipboard[i], SIGNAL ( triggered () ),
			signal_mapper, SLOT ( map () ) );
		signal_mapper->setMapping ( act_edit_load_clipboard[i], i + 1 );

		act_edit_load_clipboard[i]->setShortcut (
			QString ( "Shift+F%1" ).arg ( i + 1 ) );

		edit_menu_load_clip->addAction ( act_edit_load_clipboard[i] );
	}

	connect ( signal_mapper, SIGNAL ( mapped ( int ) ),
		this, SLOT ( press_edit_load_clipboard ( int ) ) );

	QMenu * edit_menu_save_clip = edit_menu->addMenu( "Save Clipboard" );
	edit_menu_save_clip->setTearOffEnabled ( true );

	signal_mapper = new QSignalMapper ( this );

	for ( int i = 0; i < CLIPBOARD_MENU_TOTAL; i++ )
	{
		act_edit_save_clipboard[i] = new QAction (
			QString ( "%1" ).arg ( i + 1 ), this );

		connect ( act_edit_save_clipboard[i], SIGNAL ( triggered () ),
			signal_mapper, SLOT ( map () ) );
		signal_mapper->setMapping ( act_edit_save_clipboard[i], i + 1 );

		act_edit_save_clipboard[i]->setShortcut (
			QString ( "Ctrl+F%1" ).arg ( i + 1 ) );

		edit_menu_save_clip->addAction ( act_edit_save_clipboard[i] );
	}

	connect ( signal_mapper, SIGNAL ( mapped ( int ) ),
		this, SLOT ( press_edit_save_clipboard ( int ) ) );

	QAction * act_image_scale;
	QAction * act_image_resize;
	QAction * act_image_flip_horizontally;
	QAction * act_image_flip_vertically;
	QAction * act_image_rotate_clockwise;
	QAction * act_image_rotate_anticlockwise;
	QAction * act_image_information;

	QEX_MENU ( image_to_rgb, "Convert To RGB", NULL, NULL )
	QEX_MENU ( image_to_indexed, "Convert To Indexed ...", NULL, NULL )
	QEX_MENU ( image_delete_alpha, "Delete Alpha Channel", NULL, NULL )
	QEX_MENU ( image_scale, "Scale ...", "Page Up", NULL )
	QEX_MENU ( image_resize, "Resize ...", "Page Down", NULL )
	QEX_MENU ( image_crop, "Crop", "Delete", NULL )
	QEX_MENU ( image_flip_horizontally, "Flip Horizontally", NULL,
		"object-flip-horizontal" )
	QEX_MENU ( image_flip_vertically, "Flip Vertically", NULL,
		"object-flip-vertical" )
	QEX_MENU ( image_rotate_clockwise, "Rotate Clockwise", NULL,
		"object-rotate-right" )
	QEX_MENU ( image_rotate_anticlockwise, "Rotate Anticlockwise", NULL,
		"object-rotate-left" )
	QEX_MENU ( image_information, "Information ...", "Ctrl+I",
		"dialog-information" )

	QMenu * image_menu = menuBar ()->addMenu ( "&Image" );
	image_menu->setTearOffEnabled ( true );
	image_menu->addAction ( act_image_to_rgb );
	image_menu->addAction ( act_image_to_indexed );
	image_menu->addAction ( act_image_delete_alpha );
	image_menu->addSeparator ();
	image_menu->addAction ( act_image_scale );
	image_menu->addAction ( act_image_resize );
	image_menu->addAction ( act_image_crop );
	image_menu->addSeparator ();
	image_menu->addAction ( act_image_flip_horizontally );
	image_menu->addAction ( act_image_flip_vertically );
	image_menu->addAction ( act_image_rotate_clockwise );
	image_menu->addAction ( act_image_rotate_anticlockwise );
	image_menu->addSeparator ();
	image_menu->addAction ( act_image_information );

	QEX_MENU ( selection_all, "Select All", "Ctrl+A", "edit-select-all" )
	QEX_MENU ( selection_none, "Select None", NULL, NULL )
	QEX_MENU ( selection_lasso, "Lasso", NULL, NULL )
	QEX_MENU ( selection_fill, "Fill", NULL, NULL )
	QEX_MENU ( selection_outline, "Outline", NULL, NULL )
	QEX_MENU ( selection_flip_h, "Flip Horizontally", NULL,
		"object-flip-horizontal" )
	QEX_MENU ( selection_flip_v, "Flip Vertically", NULL,
		"object-flip-vertical" )
	QEX_MENU ( selection_rotate_c, "Rotate Clockwise", NULL,
		"object-rotate-right" )
	QEX_MENU ( selection_rotate_a, "Rotate Anticlockwise", NULL,
		"object-rotate-left" )

	QMenu * selection_menu = menuBar ()->addMenu ( "&Selection" );
	selection_menu->setTearOffEnabled ( true );
	selection_menu->addAction ( act_selection_all );
	selection_menu->addAction ( act_selection_none );
	selection_menu->addAction ( act_selection_lasso );
	selection_menu->addSeparator ();
	selection_menu->addAction ( act_selection_fill );
	selection_menu->addAction ( act_selection_outline );
	selection_menu->addSeparator ();
	selection_menu->addAction ( act_selection_flip_h );
	selection_menu->addAction ( act_selection_flip_v );
	selection_menu->addAction ( act_selection_rotate_c );
	selection_menu->addAction ( act_selection_rotate_a );

	QAction * act_palette_new;
	QAction * act_palette_load;
	QAction * act_palette_save_as;
	QAction * act_palette_load_default;
	QAction * act_palette_store;
	QAction * act_palette_restore;
	QAction * act_palette_mask_all;
	QAction * act_palette_mask_none;
	QAction * act_palette_create_gradient;
	QAction * act_palette_swap_ab;
	QAction * act_palette_size;
	QAction * act_palette_sort;

	QEX_MENU ( palette_new, "New ...", NULL, "document-new" )
	QEX_MENU ( palette_size, "Set Palette Size ...", NULL, NULL )
	QEX_MENU ( palette_load, "Load ...", NULL, "document-open" )
	QEX_MENU ( palette_save_as, "Save As ...", NULL, "document-save-as" )
	QEX_MENU ( palette_load_default, "Load Default", NULL, NULL )
	QEX_MENU ( palette_store, "Store", NULL, NULL )
	QEX_MENU ( palette_restore, "Restore", NULL, NULL )
	QEX_MENU ( palette_mask_all, "Mask All", NULL, NULL )
	QEX_MENU ( palette_mask_none, "Mask None", NULL, NULL )
	QEX_MENU ( palette_swap_ab, "Swap A && B", "X", NULL )
	QEX_MENU ( palette_create_gradient, "Create Gradient A to B", NULL,NULL)
	QEX_MENU ( palette_merge_duplicates,"Merge Duplicate Colours",NULL,NULL)
	QEX_MENU ( palette_remove_unused, "Remove Unused Colours", NULL, NULL )
	QEX_MENU ( palette_create_from_canvas, "Create from Canvas", NULL, NULL)
	QEX_MENU ( palette_quantize_pnn, "Quantize", NULL, NULL )
	QEX_MENU ( palette_sort, "Sort Colours", NULL, "view-sort-ascending" )

	QMenu * palette_menu = menuBar ()->addMenu ( "&Palette" );
	palette_menu->setTearOffEnabled ( true );
	palette_menu->addAction ( act_palette_new );
	palette_menu->addAction ( act_palette_size );
	palette_menu->addAction ( act_palette_load );
	palette_menu->addAction ( act_palette_save_as );
	palette_menu->addAction ( act_palette_load_default );
	palette_menu->addSeparator ();
	palette_menu->addAction ( act_palette_store );
	palette_menu->addAction ( act_palette_restore );
	palette_menu->addSeparator ();
	palette_menu->addAction ( act_palette_mask_all );
	palette_menu->addAction ( act_palette_mask_none );
	palette_menu->addSeparator ();
	palette_menu->addAction ( act_palette_swap_ab );
	palette_menu->addAction ( act_palette_create_gradient );
	palette_menu->addAction ( act_palette_merge_duplicates );
	palette_menu->addAction ( act_palette_remove_unused );
	palette_menu->addSeparator ();
	palette_menu->addAction ( act_palette_create_from_canvas );
	palette_menu->addAction ( act_palette_quantize_pnn );
	palette_menu->addAction ( act_palette_sort );

	QAction * act_effects_transform_color;
	QAction * act_effects_invert;

	QEX_MENU ( effects_transform_color,"Transform Colour ...","Insert",NULL)
	QEX_MENU ( effects_invert, "Invert", NULL, NULL )
	QEX_MENU ( effects_edge_detect, "Edge Detect", NULL, NULL )
	QEX_MENU ( effects_sharpen, "Sharpen ...", NULL, NULL )
	QEX_MENU ( effects_soften, "Soften ...", NULL, NULL )
	QEX_MENU ( effects_emboss, "Emboss", NULL, NULL )
	QEX_MENU ( effects_normalize, "Normalize", NULL, NULL )
	QEX_MENU ( effects_bacteria, "Bacteria ...", NULL, NULL )

	QMenu * effects_menu = menuBar ()->addMenu ( "Effects" );
	effects_menu->setTearOffEnabled ( true );
	effects_menu->addAction ( act_effects_transform_color );
	effects_menu->addAction ( act_effects_invert );
	effects_menu->addSeparator ();
	effects_menu->addAction ( act_effects_edge_detect );
	effects_menu->addAction ( act_effects_sharpen );
	effects_menu->addAction ( act_effects_soften );
	effects_menu->addAction ( act_effects_emboss );
	effects_menu->addAction ( act_effects_normalize );
	effects_menu->addSeparator ();
	effects_menu->addAction ( act_effects_bacteria );

	QAction * act_options_full_screen;
	QAction * act_options_preferences;
	QAction * act_options_pan_window;
	QAction * act_options_zoom_main_in;
	QAction * act_options_zoom_main_out;
	QAction * act_options_zoom_main_3;
	QAction * act_options_zoom_main_100;
	QAction * act_options_zoom_main_3200;

	QEX_MENU ( options_full_screen, "Full Screen", "F11", "view-fullscreen")
	QEX_MENU ( options_preferences, "Preferences ...", "Ctrl+P",
		"preferences-other" )
	QEX_MENU ( options_statusbar, "Show Status Bar", NULL, NULL )
	QEX_MENU ( options_pan_window, "Pan Window", "End", NULL )
	QEX_MENU ( options_split_canvas, "Split Canvas", "V", NULL )
	QEX_MENU ( options_split_switch, "Split Canvas Switch", "H", NULL )
	QEX_MENU ( options_split_focus, "Split Canvas Focus", NULL, NULL )
	QEX_MENU ( options_zoom_main_in, "Zoom In", "+", "zoom-in" )
	QEX_MENU ( options_zoom_main_out, "Zoom Out", "-", "zoom-out" )
	QEX_MENU ( options_zoom_main_3, "3%", "4", NULL )
	QEX_MENU ( options_zoom_main_100, "100%", "5", "zoom-original" )
	QEX_MENU ( options_zoom_main_3200, "3200%", "6", NULL )
	QEX_MENU ( options_zoom_split_in, "Zoom Split In", "Shift++", "zoom-in")
	QEX_MENU ( options_zoom_split_out,"Zoom Split Out","Shift+-","zoom-out")
	QEX_MENU ( options_zoom_split_3, "3%", "1", NULL )
	QEX_MENU ( options_zoom_split_100, "100%", "2", "zoom-original" )
	QEX_MENU ( options_zoom_split_3200, "3200%", "3", NULL )
	QEX_MENU ( options_zoom_grid, "Show Zoom Grid", NULL, NULL )

	QMenu * options_menu = menuBar ()->addMenu ( "&Options" );
	options_menu->setTearOffEnabled ( true );
	options_menu->addAction ( act_options_full_screen );
	options_menu->addAction ( act_options_preferences );
	options_menu->addAction ( act_options_statusbar );
	options_menu->addAction ( act_options_pan_window );
	options_menu->addSeparator ();
	options_menu->addAction ( act_options_split_canvas );
	options_menu->addAction ( act_options_split_switch );
	options_menu->addAction ( act_options_split_focus );
	options_menu->addSeparator ();
	options_menu->addAction ( act_options_zoom_grid );

	QMenu * options_menu_zoom_main = options_menu->addMenu ( "Zoom Main" );
	options_menu_zoom_main->setTearOffEnabled ( true );
	options_menu_zoom_main->addAction ( act_options_zoom_main_in );
	options_menu_zoom_main->addAction ( act_options_zoom_main_out );
	options_menu_zoom_main->addAction ( act_options_zoom_main_3 );
	options_menu_zoom_main->addAction ( act_options_zoom_main_100 );
	options_menu_zoom_main->addAction ( act_options_zoom_main_3200 );

	options_menu_zoom_split = options_menu->addMenu ( "Zoom Split" );
	options_menu_zoom_split->setTearOffEnabled ( true );
	options_menu_zoom_split->addAction ( act_options_zoom_split_in );
	options_menu_zoom_split->addAction ( act_options_zoom_split_out );
	options_menu_zoom_split->addAction ( act_options_zoom_split_3 );
	options_menu_zoom_split->addAction ( act_options_zoom_split_100 );
	options_menu_zoom_split->addAction ( act_options_zoom_split_3200 );

	act_options_split_focus->setCheckable ( true );
	act_options_split_focus->setChecked ( true );

	act_options_statusbar->setCheckable ( true );
	act_options_statusbar->setChecked ( true );

	act_options_split_canvas->setCheckable ( true );
	act_options_split_canvas->setChecked ( false );

	act_options_zoom_grid->setCheckable ( true );
	// NOTE: the menu state is set later as it needs all of the canvas
	// widgets to have been created, which is untrue here.

	QAction * act_help_help;
	QAction * act_help_about_qt;
	QAction * act_help_about;

	QEX_MENU ( help_help, "Help ...", "F1", "help-contents" )
	QEX_MENU ( help_about_qt, "About Qt ...", NULL, NULL )
	QEX_MENU ( help_about, "About ...", NULL, "help-about" )

	QMenu * help_menu = menuBar ()->addMenu ( "&Help" );
	help_menu->setTearOffEnabled ( true );

	help_menu->addAction ( act_help_help );
	help_menu->addSeparator ();
	help_menu->addAction ( act_help_about_qt );
	help_menu->addAction ( act_help_about );
}

