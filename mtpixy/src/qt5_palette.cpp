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

#include "qt5.h"



void Mainwindow::press_palette_load ()
{
	QString const filename = QFileDialog::getOpenFileName ( this,
		"Load Palette File", mtQEX::qstringFromC (
			backend.mprefs.recent_image.filename().c_str() ),
		NULL, NULL, QFileDialog::DontUseNativeDialog );

	if ( filename.isEmpty () )
	{
		return;
	}

	operation_update ( backend.file.palette_load (filename.toUtf8().data()),
		"Palette Load", UPDATE_ALL );
}

void Mainwindow::press_palette_save_as ()
{
	QStringList	list;


	mtQEX::SaveFileDialog dialog ( this, "Save Palette File", list,
		0, backend.file.get_filename () );

	dialog.setOption ( QFileDialog::DontConfirmOverwrite );

	if ( ! backend.file.get_filename () )
	{
		dialog.setDirectory ( mtQEX::qstringFromC (
			backend.mprefs.recent_image.directory().c_str() ) );
	}

	while ( dialog.exec () )
	{
		QString filename = mtQEX::get_filename ( dialog );


		if ( filename.isEmpty () )
		{
			break;
		}

		char * correct = mtPixyUI::File::get_correct_filename (
			filename.toUtf8().data(), PIXY_FILE_TYPE_GPL );

		if ( correct )
		{
			filename = correct;
			free ( correct );
			correct = NULL;
		}

		if ( mtQEX::message_file_overwrite ( this, filename ) )
		{
			continue;
		}

		if ( 0 == backend.file.palette_save (
			filename.toUtf8 ().data () ) )
		{
			break;
		}

		QMessageBox::critical ( this, "Error",
			"Operation 'Palette Save' was unsuccessful." );
	}
}

void Mainwindow::press_palette_load_default ()
{
	operation_update ( backend.file.palette_load_default (
		mprefs.file_new_palette_type,
		mprefs.file_new_palette_num,
		mprefs.file_new_palette_file ),
		"Palette Load Default", UPDATE_ALL );
}

void Mainwindow::press_palette_store ()
{
	pixy_palette_copy ( &backend.palette, backend.file.get_palette () );
}

void Mainwindow::press_palette_restore ()
{
	operation_update ( backend.file.palette_set ( &backend.palette ),
		"Palette Restore", UPDATE_ALL);
}

void Mainwindow::press_palette_mask_all ()
{
	backend.file.palette_mask_all ();
	update_ui ( UPDATE_PALETTE );
}

void Mainwindow::press_palette_mask_none ()
{
	backend.file.palette_unmask_all ();
	update_ui ( UPDATE_PALETTE );
}

void Mainwindow::press_palette_create_gradient ()
{
	operation_update ( backend.file.palette_create_gradient (),
		"Palette Create Gradient", UPDATE_ALL );
}

void Mainwindow::press_palette_swap_ab ()
{
	backend.file.palette_swap_ab ();
	update_ui ( UPDATE_TOOLBAR | UPDATE_PALETTE );
}

void Mainwindow::press_palette_size ()
{
	int	const	min = PIXY_PALETTE_COLOR_TOTAL_MIN;
	int	const	max = PIXY_PALETTE_COLOR_TOTAL_MAX;
	int	const	num = pixy_pixmap_get_palette_size (
				backend.file.get_pixmap () );

	DialogGetInt dialog( this, min, max, num, "Palette Size", "Colours", 0);

	if ( dialog.exec () == QDialog::Accepted )
	{
		// Apply pressed
		operation_update ( backend.file.palette_set_size (
			dialog.get_int () ), "Palette Set Size", UPDATE_ALL );
	}
}

void Mainwindow::press_palette_merge_duplicates ()
{
	int 		tot = 0;

	if ( 0 == operation_update ( backend.file.palette_merge_duplicates (
		&tot ), "Palette Merge Duplicates", UPDATE_ALL ) )
	{
		QMessageBox::information ( this, "Information",
			QString ( "%1 identical colours merged." ).arg ( tot ));
	}
}

void Mainwindow::press_palette_remove_unused ()
{
	int 		tot = 0;

	if ( 0 == operation_update ( backend.file.palette_remove_unused (
		&tot ), "Palette Remove Unused", UPDATE_ALL ) )
	{
		QMessageBox::information ( this, "Information",
			QString ( "%1 unused colours removed." ).arg ( tot ) );
	}
}

void Mainwindow::press_palette_create_from_canvas ()
{
	operation_update ( backend.file.palette_create_from_canvas (),
		"Create from Canvas", UPDATE_ALL );
}

void Mainwindow::press_palette_quantize_pnn ()
{
	operation_update ( backend.file.palette_quantize_pnn (), "Quantize PNN",
		UPDATE_ALL );
}

PaletteHolder::PaletteHolder (
	Mainwindow	&mw
	)
	:
	m_palette_view	( new PaletteView ( mw ) ),
	mainwindow	( mw )
{
	setWidget ( m_palette_view );
	setBackgroundRole ( QPalette::Dark );
	setAlignment ( Qt::AlignLeft | Qt::AlignTop );

	update_canvas_easel_rgb ();
}

PaletteHolder::~PaletteHolder ()
{
}

void PaletteHolder::update_canvas_easel_rgb ()
{
	int	const	col = mainwindow.backend.uprefs.get_int (
				PREFS_CANVAS_EASEL_RGB );
	QPalette	pal = palette ();


	pal.setColor ( QPalette::Dark, QColor ( pixy_int_2_red ( col ),
		pixy_int_2_green ( col ), pixy_int_2_blue ( col ) ) );
	setPalette ( pal );
}

void PaletteHolder::resizeEvent (
	QResizeEvent	* const	ARG_UNUSED ( e )
	)
{
	rebuild ();
}

void PaletteHolder::rebuild ()
{
	mtPixmap const * const pixmap = mainwindow.backend.file.get_pixmap ();
	if ( ! pixmap )
	{
		return;
	}


	int	const	w = viewport ()->width ();
	int	const	h = viewport ()->height ();
	int	const	unit = 24 * mainwindow.backend.get_ui_scale_palette();
	int	const	paltot = pixy_pixmap_get_palette_size (pixmap);
	int		coltot, rowtot;

	if ( h > w )
	{
		coltot = MAX ( 1, w / unit );
		rowtot = MAX ( 1, paltot / coltot );

		if ( coltot * rowtot < paltot )
		{
			rowtot++;
		}
	}
	else
	{
		rowtot = MAX ( 1, h / unit );
		coltot = MAX ( 1, w / unit );

		if ( coltot * rowtot < paltot )
		{
			coltot = MAX ( 1, paltot / rowtot );

			if ( coltot * rowtot < paltot )
			{
				coltot++;
			}
		}
		else
		{
			rowtot = MAX ( 1, paltot / coltot );

			if ( coltot * rowtot < paltot )
			{
				rowtot++;
			}
		}
	}

	m_palette_view->set_swatch_size ( unit, coltot, rowtot );
	m_palette_view->resize ( unit * coltot, unit * rowtot );
	m_palette_view->updateGeometry ();
	m_palette_view->rebuild ();
}

PaletteView::PaletteView (
	Mainwindow	&mw
	)
	:
	m_swatch_size	( 0 ),
	m_swatch_rows	( 0 ),
	m_swatch_cols	( 0 ),
	m_drag_index	( -1 ),
	m_drag_change	( -1 ),
	mainwindow	( mw )
{
	setMouseTracking ( true );
}

PaletteView::~PaletteView ()
{
}

void PaletteView::set_swatch_size (
	int	const	unit,
	int	const	coltot,
	int	const	rowtot
	)
{
	m_swatch_size = unit;
	m_swatch_cols = coltot;
	m_swatch_rows = rowtot;
}

void PaletteView::rebuild ()
{
	int	const	w = m_swatch_size * m_swatch_cols;
	int	const	h = m_swatch_size * m_swatch_rows;

	m_pixmap.reset ( new QPixmap ( w, h ) );
	m_pixmap->fill( QWidget::palette().color( QWidget::backgroundRole() ) );

	mtPixmap const * const pixmap = mainwindow.backend.file.get_pixmap ();
	if ( ! pixmap )
	{
		update ();
		return;
	}


	QPainter	paint ( m_pixmap.get() );
	QFont		fnt ( "Sans" );
	char const * const msk = mainwindow.backend.file.palette_mask.color;
	int		xx = 0, yy = 0;
	int	const	ss3 = m_swatch_size / 3;
	int	const	paltot = pixy_pixmap_get_palette_size (pixmap);
	int	const	idx_a = mainwindow.backend.file.brush.
				get_color_a_index ();
	int	const	idx_b = mainwindow.backend.file.brush.
				get_color_b_index ();
	double	const	f = mainwindow.backend.uprefs.get_double (
				PREFS_PALETTE_NUMBER_OPACITY );
	double	const	ff = 1 - f;
	mtColor const * const col = &pixy_pixmap_get_palette_const (pixmap)->
				color[0];


	fnt.setPixelSize ( ss3 );
	fnt.setBold ( true );
	paint.setFont ( fnt );

	for ( int i = 0; i < paltot; i++ )
	{
		unsigned char const r = col[i].red;
		unsigned char const g = col[i].green;
		unsigned char const b = col[i].blue;

		paint.fillRect ( xx, yy, m_swatch_size, m_swatch_size,
			QColor ( r, g, b ) );

		int const tot = (r * 299 + 587 * g + 114 * b) / 1000;

		if ( f > 0.0 )
		{
			if ( tot > 128 )
			{
				paint.setPen ( QColor (
					(int)( r * ff ),
					(int)( g * ff ),
					(int)( b * ff ) ) );
			}
			else
			{
				paint.setPen ( QColor (
					(int)( r * ff + 255 * f ),
					(int)( g * ff + 255 * f ),
					(int)( b * ff + 255 * f ) ));
			}

			paint.drawText ( QPoint ( xx, yy + ss3 ),
				QString ( "%1" ).arg ( i ) );
		}

		if ( tot > 128 )
		{
			paint.setPen ( QColor ( 0, 0, 0 ) );
		}
		else
		{
			paint.setPen ( QColor ( 255, 255, 255 ) );
		}

		if ( 0 != msk[i] )
		{
			paint.drawText ( QPoint( xx + ss3, yy + 2*ss3 ), "X" );
		}

		if ( i == idx_a )
		{
			paint.drawText ( QPoint( xx, yy + 3*ss3 ), "A" );
		}

		if ( i == idx_b )
		{
			paint.drawText ( QPoint( xx + ss3, yy + 3*ss3 ), "B" );
		}

		xx += m_swatch_size;
		if ( xx >= w )
		{
			xx = 0;
			yy += m_swatch_size;
		}
	}

	update ();
}

void PaletteView::paintEvent (
	QPaintEvent	* const	ev
	)
{
	int	const	px = ev->rect ().x ();
	int	const	py = ev->rect ().y ();
	int	const	pw = ev->rect ().width ();
	int	const	ph = ev->rect ().height ();

	if ( pw < 1 || ph < 1 )
	{
		return;
	}

	QPainter	paint ( this );

	paint.drawPixmap ( px, py, pw, ph, *m_pixmap.get(), px, py, pw, ph );
}

void PaletteView::mouseEventRouter (
	QMouseEvent	* const	ev,
	int		const	caller	// 0 = Press 2 = Move
	)
{
	if ( m_swatch_size < 1 )
	{
		return;
	}

	mtPixmap const * const pixmap = mainwindow.backend.file.get_pixmap ();
	if ( ! pixmap )
	{
		return;
	}

	int		but = 0;

	if ( ev->buttons () & Qt::LeftButton )
	{
		but = 1;
	}
	else if ( ev->buttons () & Qt::RightButton )
	{
		but = 3;
	}


	int	const	paltot = pixy_pixmap_get_palette_size ( pixmap );
	int	const	xx = ev->x ();
	int	const	yy = ev->y ();
	int	const	swatch_r = MIN( xx / m_swatch_size, m_swatch_cols - 1 );
	int	const	swatch_c = yy / m_swatch_size;
	int	const	swatch_i = swatch_r + swatch_c * m_swatch_cols;
	mtColor const * const col = &pixy_pixmap_get_palette_const (pixmap)->
				color[0];
	unsigned char const idx = (unsigned char)swatch_i;

	if ( swatch_i < paltot )
	{
		unsigned char const r = col [ swatch_i ].red;
		unsigned char const g = col [ swatch_i ].green;
		unsigned char const b = col [ swatch_i ].blue;

		mainwindow.set_statusbar_cursor ( QString ( "%1 {%2,%3,%4)" )
			.arg( swatch_i ).arg( r ).arg( g ).arg( b ) );
	}
	else
	{
		but = 0;
		mainwindow.set_statusbar_cursor ( "" );
	}


	int const key_ctrl = (int)(ev->modifiers () & Qt::ControlModifier);
	int const key_shift = (int)(ev->modifiers () & Qt::ShiftModifier);


	switch ( caller )
	{
	case 0:	// Mouse press
		if ( 1 == but )
		{
			// Left mouse click
			if ( key_ctrl )
			{
				char * const a = mainwindow.backend.file.
						palette_mask.color;


				a[ idx ] = ! a[ idx ];

				mainwindow.update_ui (
					Mainwindow::UPDATE_PALETTE );
			}
			else if ( key_shift )
			{
				m_drag_index = idx;
			}
			else
			{
				mainwindow.backend.file.brush.set_color_a (
					idx, col );

				mainwindow.update_ui (
					Mainwindow::UPDATE_PALETTE
					| Mainwindow::UPDATE_TOOLBAR );
			}
		}
		else if ( but == 3 )
		{
			// Right mouse click
			mainwindow.backend.file.brush.set_color_b( idx, col );

			mainwindow.update_ui (
				Mainwindow::UPDATE_PALETTE
				| Mainwindow::UPDATE_TOOLBAR );
		}
		break;

	case 2:	// Mouse movement
		if ( 1 == but && key_shift )
		{
			if ( idx != m_drag_index )
			{
				// User has dragged a swatch
				m_drag_change = 1;
				pixy_pixmap_palette_move_color (
					mainwindow.backend.file.get_pixmap (),
					(unsigned char)m_drag_index,
					(unsigned char)idx );

				mainwindow.update_ui( Mainwindow::UPDATE_ALL );
				m_drag_index = idx;
			}
		}
		break;
	}
}

void PaletteView::mousePressEvent (
	QMouseEvent	* const	ev
	)
{
	mouseEventRouter ( ev, 0 );
}

void PaletteView::mouseMoveEvent (
	QMouseEvent	* const	ev
	)
{
	mouseEventRouter ( ev, 2 );
}

void PaletteView::leaveEvent (
	QEvent		* const	ARG_UNUSED ( ev )
	)
{
	mainwindow.set_statusbar_cursor ( "" );
}

void PaletteView::mouseReleaseEvent (
	QMouseEvent	* const	ev
	)
{
	if ( ev->button () & Qt::LeftButton )
	{
		if ( m_drag_change > 0 )
		{
			m_drag_change = -1;

			mainwindow.operation_update (
				mainwindow.backend.file.palette_changed (),
				"Palette Move", Mainwindow::UPDATE_ALL );
		}

		m_drag_index = -1;
	}
}

