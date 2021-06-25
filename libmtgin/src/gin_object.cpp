/*
	Copyright (C) 2021 Mark Tyler

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



mtGin::Object::Object()
{
}

mtGin::Object::~Object()
{
	while ( m_child_first )
	{
		m_child_first->detach_delete();
	}

	emit_signal ( GIN_SIGNAL_DESTROY_OBJECT, nullptr );
}

char const * mtGin::Object::name() const
{
	return "mtGin::Object";
}

int mtGin::Object::emit_signal (
	int		const	sig_type,
	CB_Data		* const	data
	)
{
	if ( m_block_signals )
	{
		return 0;
	}

	int tot = 0;

	for ( SignalInfo const * i = m_sig_stack.get(); i; i = i->next )
	{
		if ( i->type != sig_type )
		{
			continue;
		}

		if ( 0 == i->func ( data ) )
		{
			tot++;
		}
	}

	if ( tot < 1 && m_parent )
	{
		return m_parent->emit_signal ( sig_type, data );
	}

	return tot;
}

int mtGin::Object::add_signal (
	int		const	sig_type,
	SignalFunc	const	func
	)
{
	return m_sig_stack.add ( sig_type, func );
}

int mtGin::Object::remove_signal (
	int	const	sig_id
	)
{
	return m_sig_stack.remove ( sig_id );
}

void mtGin::Object::add_child (
	Object	* const	child
	)
{
	if ( ! child || child->m_parent )
	{
		return;
	}

	child->m_parent = this;

	if ( ! m_child_first )
	{
		m_child_first = child;
		m_child_last = child;

		return;
	}

	// m_child_first exists, so m_child_last also exists

	m_child_last->m_sibling_after = child;
	child->m_sibling_before = m_child_last;

	m_child_last = child;
}

void mtGin::Object::destroy_offspring()
{
	while ( m_child_first )
	{
		m_child_first->destroy_offspring();	// Recursion
	}
}

void mtGin::Object::detach_delete()
{
	destroy_offspring();

	// Detach from parent
	if ( m_parent )
	{
		if ( this == m_parent->m_child_first )
		{
			m_parent->m_child_first = this->m_sibling_after;
		}

		if ( this == m_parent->m_child_last )
		{
			m_parent->m_child_last = this->m_sibling_before;
		}
	}

	// Detach from siblings
	if ( m_sibling_after )
	{
		m_sibling_after->m_sibling_before = m_sibling_before;
	}

	if ( m_sibling_before )
	{
		m_sibling_before->m_sibling_after = m_sibling_after;
	}

	delete this;
}



/// ----------------------------------------------------------------------------



mtGin::Object::SignalStack::SignalStack ()
{
}

mtGin::Object::SignalStack::~SignalStack ()
{
	while ( m_sig )
	{
		remove ( m_sig->id );
	}
}

int mtGin::Object::SignalStack::add (
	int		const	sig_type,
	SignalFunc	const	func
	)
{
	if ( m_sig_total >= GIN_OBJECT_SIGNAL_MAX )
	{
		std::cerr << "SignalStack::add: maximum reached:"
			<< GIN_OBJECT_SIGNAL_MAX << "\n";
		return 1;
	}

	if ( m_sig_id_next >= GIN_OBJECT_SIGNAL_ID_MAX )
	{
		std::cerr << "SignalStack::add: maximum id:"
			<< GIN_OBJECT_SIGNAL_ID_MAX << "\n";
		return 1;
	}

	auto * sigi = (SignalInfo *)calloc ( 1, sizeof(SignalInfo) );
	if ( ! sigi )
	{
		std::cerr << "SignalStack::add: unable to calloc.\n";
		return 1;
	}

	sigi->type = sig_type;
	sigi->id = m_sig_id_next++;
	sigi->func = func;
	sigi->next = m_sig;

	m_sig = sigi;
	m_sig_total++;

	return 0;
}

int mtGin::Object::SignalStack::remove ( int const sig_id )
{
	SignalInfo * old = m_sig;

	for ( SignalInfo * i = m_sig; i; i = i->next )
	{
		if ( i->id == sig_id )
		{
			if ( i == m_sig )
			{
				m_sig = i->next;
			}
			else
			{
				old->next = i->next;
			}

			free ( i );

			return 0;
		}

		old = i;
	}

	return 1;
}

