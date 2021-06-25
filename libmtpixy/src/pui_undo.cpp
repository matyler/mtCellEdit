/*
	Copyright (C) 2016-2021 Mark Tyler

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



int mtPixyUI::UndoStack::undo (
	mtPixy::Pixmap	& ppim
	)
{
	if ( ! m_step_current )
	{
		return 1;
	}

	UndoStep * const ps = m_step_current->get_step_previous ();

	if ( ! ps )
	{
		return 1;
	}

	if ( ps->step_restore ( ppim ) )
	{
		return 1;
	}

	m_step_current = ps;

	m_total_undo_steps--;
	m_total_redo_steps++;

	return 0;
}

int mtPixyUI::UndoStack::redo (
	mtPixy::Pixmap	& ppim
	)
{
	if ( ! m_step_current )
	{
		return 1;
	}

	UndoStep * const ns = m_step_current->get_step_next ();

	if ( ! ns )
	{
		return 1;
	}

	if ( ns->step_restore ( ppim ) )
	{
		return 1;
	}

	m_step_current = ns;

	m_total_undo_steps++;
	m_total_redo_steps--;

	return 0;
}

int mtPixyUI::UndoStack::add_next_step (
	mtPixmap const * const	pim
	)
{
	mtPixmap * i = pixy_pixmap_duplicate ( pim );
	if ( ! i )
	{
		return 1;
	}

	UndoStep * const step = new UndoStep (i);
	if ( ! m_step_first )
	{
		m_step_first = step;
	}

	if ( m_step_current )
	{
		m_step_current->delete_steps_next ();
		m_total_redo_steps = 0;

		m_step_current->insert_after ( step );

		m_total_undo_steps++;
	}

	m_step_current = step;
	int64_t undo_bytes = get_undo_bytes ();

	while (	m_step_first != m_step_current &&
		( m_total_undo_steps > m_max_steps ||
			undo_bytes > m_max_bytes )
		)
	{
		UndoStep	* const	ns = m_step_first->get_step_next ();

		undo_bytes -= m_step_first->get_canvas_bytes ();

		delete m_step_first;
		m_step_first = ns;

		m_total_undo_steps--;
	}

	return 0;
}

int64_t mtPixyUI::UndoStack::get_undo_bytes () const
{
	UndoStep	* step = m_step_current;

	if ( ! step )
	{
		return 0;
	}

	int64_t		tot = 0;

	for (	step = step->get_step_previous ();
		step;
		step = step->get_step_previous ()
		)
	{
		tot += step->get_canvas_bytes ();
	}

	return tot;
}

int64_t mtPixyUI::UndoStack::get_redo_bytes () const
{
	UndoStep	* step = m_step_current;

	if ( ! step )
	{
		return 0;
	}

	int64_t		tot = 0;

	for (	step = step->get_step_next ();
		step;
		step = step->get_step_next ()
		)
	{
		tot += step->get_canvas_bytes ();
	}

	return tot;
}

int64_t mtPixyUI::UndoStack::get_canvas_bytes () const
{
	if ( m_step_current )
	{
		return m_step_current->get_canvas_bytes ();
	}

	return 0;
}

int mtPixyUI::UndoStack::get_undo_steps () const
{
	return m_total_undo_steps;
}

int mtPixyUI::UndoStack::get_redo_steps () const
{
	return m_total_redo_steps;
}

mtPixyUI::UndoStep * mtPixyUI::UndoStack::get_step_current () const
{
	return m_step_current;
}

void mtPixyUI::UndoStack::set_max_bytes (
	int64_t	const	n
	)
{
	m_max_bytes = MAX ( MIN_BYTES, n );
	m_max_bytes = MIN ( MAX_BYTES, m_max_bytes );
}

void mtPixyUI::UndoStack::set_max_steps (
	int	const	n
	)
{
	m_max_steps = MAX ( (int)MIN_STEPS, n );
	m_max_steps = MIN ( (int)MAX_STEPS, m_max_steps );
}

void mtPixyUI::UndoStack::clear ()
{
	if ( ! m_step_first )
	{
		return;
	}

	m_step_first->delete_steps_next ();
	delete m_step_first;

	m_step_first = NULL;
	m_step_current = NULL;

	m_total_undo_steps = 0;
	m_total_redo_steps = 0;
}

mtPixmap * mtPixyUI::UndoStack::get_pixmap ()
{
	if ( ! m_step_current )
	{
		return NULL;
	}

	return m_step_current->get_pixmap ();
}



/// ----------------------------------------------------------------------------



mtPixyUI::UndoStep::UndoStep (
	mtPixmap	* const	pim
	)
	:
	m_pixmap	( pim )
{
	set_canvas_bytes ();
}

mtPixyUI::UndoStep::~UndoStep ()
{
	if ( m_step_previous )
	{
		m_step_previous->m_step_next = m_step_next;
	}

	if ( m_step_next )
	{
		m_step_next->m_step_previous = m_step_previous;
	}

	m_step_previous = NULL;
	m_step_next = NULL;
}

void mtPixyUI::UndoStep::insert_after (
	UndoStep	* const	us
	)
{
	if ( m_step_next )
	{
		m_step_next->m_step_previous = us;
	}

	us->m_step_next = m_step_next;
	us->m_step_previous = this;

	m_step_next = us;
}

int mtPixyUI::UndoStep::step_restore (
	mtPixy::Pixmap	& ppim
	) const
{
	mtPixmap const * const pixmap = get_pixmap ();
	mtPixmap * i = pixy_pixmap_duplicate ( pixmap );
	if ( ! i )
	{
		return 1;
	}

	ppim.reset ( i );

	return 0;
}

mtPixyUI::UndoStep * mtPixyUI::UndoStep::get_step_previous () const
{
	return m_step_previous;
}

mtPixyUI::UndoStep * mtPixyUI::UndoStep::get_step_next () const
{
	return m_step_next;
}

int64_t mtPixyUI::UndoStep::get_canvas_bytes () const
{
	return m_canvas_bytes;
}

void mtPixyUI::UndoStep::set_canvas_bytes ()
{
	m_canvas_bytes = 1024;	// Default size for an 'empty' step

	mtPixmap const * const pixmap = get_pixmap();
	if ( ! pixmap )
	{
		return;
	}

	int const can = pixmap->width * pixmap->height;

	if ( pixmap->alpha )
	{
		m_canvas_bytes += can;
	}

	if ( pixmap->canvas )
	{
		m_canvas_bytes += can * pixmap->bpp;
	}
}

void mtPixyUI::UndoStep::delete_steps_next ()
{
	while ( m_step_next )
	{
		delete m_step_next;
	}
}

int mtPixyUI::File::commit_undo_step ()
{
	m_modified = 1;

	return m_undo_stack.add_next_step ( get_pixmap() );
}

