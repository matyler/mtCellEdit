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

#include "private.h"



mtPixyUI::UndoStack::UndoStack ()
	:
	m_step_first	(),
	m_step_current	(),
	m_max_bytes	( 1000000000 ),
	m_max_steps	( 100 ),
	m_total_bytes	( 0 ),
	m_total_undo_steps ( 0 ),
	m_total_redo_steps ( 0 )
{
}

mtPixyUI::UndoStack::~UndoStack ()
{
	clear ();
}

int mtPixyUI::UndoStack::undo (
	mtPixy::Image ** const	ppim
	)
{
	if ( ! m_step_current )
	{
		return 1;
	}


	UndoStep	* const ps = m_step_current->get_step_previous ();


	if ( ! ps )
	{
		return 1;
	}

	if ( ps->step_restore ( ppim ) )
	{
		return 1;
	}

	m_total_bytes -= m_step_current->get_byte_size ();

	m_step_current = ps;

	m_total_undo_steps--;
	m_total_redo_steps++;

	return 0;
}

int mtPixyUI::UndoStack::redo (
	mtPixy::Image ** const	ppim
	)
{
	if ( ! m_step_current )
	{
		return 1;
	}


	UndoStep	* const ns = m_step_current->get_step_next ();


	if ( ! ns )
	{
		return 1;
	}

	if ( ns->step_restore ( ppim ) )
	{
		return 1;
	}

	m_step_current = ns;

	m_total_bytes += m_step_current->get_byte_size ();
	m_total_undo_steps++;
	m_total_redo_steps--;

	return 0;
}

int mtPixyUI::UndoStack::add_next_step (
	mtPixy::Image * const	pim
	)
{
	if ( ! pim )
	{
		return 1;
	}


	mtPixy::Image * i = pim->duplicate ();


	if ( ! i )
	{
		return 1;
	}

	add_step ( new UndoStep ( i ) );

	return 0;
}

void mtPixyUI::UndoStack::add_step (
	UndoStep	* const	step
	)
{
	if ( ! m_step_first )
	{
		m_step_first = step;
	}

	if ( m_step_current )
	{
		m_step_current->delete_steps_next ();
		m_total_redo_steps = 0;

		m_step_current->insert_after ( step );

		m_total_bytes += m_step_current->get_byte_size ();
		m_total_undo_steps++;
	}

	m_step_current = step;

	while (	m_step_first != m_step_current &&
		( m_total_undo_steps > m_max_steps ||
			m_total_bytes > m_max_bytes )
		)
	{
		UndoStep	* const	ns = m_step_first->get_step_next ();


		m_total_bytes -= m_step_first->get_byte_size ();

		delete m_step_first;
		m_step_first = ns;

		m_total_undo_steps--;
	}
}

int64_t mtPixyUI::UndoStack::get_undo_bytes () const
{
	return m_total_bytes;
}

int mtPixyUI::UndoStack::get_undo_steps () const
{
	return m_total_undo_steps;
}

int mtPixyUI::UndoStack::get_redo_steps () const
{
	return m_total_redo_steps;
}

mtPixyUI::UndoStep * mtPixyUI::UndoStack::get_step_current ()
{
	return m_step_current;
}

void mtPixyUI::UndoStack::set_max_bytes (
	int64_t	const	n
	)
{
	m_max_bytes = MAX ( 1048576, n );
	m_max_bytes = MIN ( 10485760000, m_max_bytes );
}

void mtPixyUI::UndoStack::set_max_steps (
	int	const	n
	)
{
	m_max_steps = MAX ( 1, n );
	m_max_steps = MIN ( 1000, m_max_steps );
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

	m_total_bytes = 0;
	m_total_undo_steps = 0;
	m_total_redo_steps = 0;
}

mtPixy::Image * mtPixyUI::UndoStack::get_current_image ()
{
	if ( ! m_step_current )
	{
		return NULL;
	}

	return m_step_current->get_image ();
}

mtPixyUI::UndoStep::UndoStep (
	mtPixy::Image	* const	pim
	)
	:
	m_step_previous	(),
	m_step_next	(),
	m_image		( pim ),
	m_byte_size	( 0 )
{
	set_byte_size ();
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

	delete m_image;
	m_image = NULL;
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
	mtPixy::Image ** const	ppim
	)
{
	if ( ! ppim || ! ppim[0] || ! m_image )
	{
		return 1;
	}

	mtPixy::Image * i = m_image->duplicate ();
	if ( ! i )
	{
		return 1;
	}

	delete ppim[0];
	ppim[0] = i;

	return 0;
}

mtPixyUI::UndoStep * mtPixyUI::UndoStep::get_step_previous ()
{
	return m_step_previous;
}

mtPixyUI::UndoStep * mtPixyUI::UndoStep::get_step_next ()
{
	return m_step_next;
}

int64_t mtPixyUI::UndoStep::get_byte_size () const
{
	return m_byte_size;
}

void mtPixyUI::UndoStep::set_byte_size ()
{
	m_byte_size = 1024;	// Default size for an 'empty' step


	if ( m_image )
	{
		int const can = m_image->get_width () * m_image->get_height ();


		if ( m_image->get_alpha () )
		{
			m_byte_size += can;
		}

		if ( m_image->get_canvas () )
		{
			switch ( m_image->get_type () )
			{
			case mtPixy::Image::INDEXED:
				m_byte_size += can;
				break;

			case mtPixy::Image::RGB:
				m_byte_size += can * 3;
				break;

			default:
				break;
			}
		}
	}
}

void mtPixyUI::UndoStep::delete_steps_next ()
{
	while ( m_step_next )
	{
		delete m_step_next;
	}
}

mtPixy::Image * mtPixyUI::UndoStep::get_image ()
{
	return m_image;
}

int mtPixyUI::File::commit_undo_step ()
{
	m_modified = 1;

	return m_undo_stack.add_next_step ( m_image );
}

