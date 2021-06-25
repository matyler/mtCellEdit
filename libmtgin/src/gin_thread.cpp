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

#define MTGIN_PRIVATE_THREAD
#include "private.h"



namespace {

static int thread_func ( void * ptr )
{
	auto * const thread = static_cast<mtGin::Thread *>(ptr);

	thread->run_func ();

	return 0;
}

}		// namespace {



/// ----------------------------------------------------------------------------



mtGin::Thread::Thread ()
	:
	m_mutex		( SDL_CreateMutex () )
{
	if ( ! m_mutex )
	{
		std::cerr << "mtGin::Thread SDL_CreateMutex failure\n";
		throw 123;
	}
}

mtGin::Thread::~Thread ()
{
	terminate ();		// Request thread finishes
	join ();		// Wait for thread to stop

	SDL_DestroyMutex ( m_mutex );
}

int mtGin::Thread::init (
	std::function<void()>	const	func,
	char		const *	const	name
	)
{
	int res = 1;

	if ( 0 == SDL_LockMutex ( m_mutex ) )
	{
		if (	m_status == THREAD_NOT_STARTED
			|| m_status == THREAD_FINISHED
			)
		{
			res = 0;	// Success

			m_func = func;
			m_name = name ? name : "";
		}
		else
		{
			std::cerr << "Thread::init: '" << m_name
				<< "' is still active!\n";
		}

		SDL_UnlockMutex ( m_mutex );
	}

	return res;
}

int mtGin::Thread::start ()
{
	int res = 1;

	if ( 0 == SDL_LockMutex ( m_mutex ) )
	{
		if (	m_status == THREAD_NOT_STARTED
			|| m_status == THREAD_FINISHED
			)
		{
			m_thread = SDL_CreateThread ( thread_func,
				m_name.c_str(), (void *)this );

			if ( m_thread )
			{
				res = 0;	// Success
				m_status = THREAD_RUNNING;
			}
			else
			{
				std::cerr << "Unable to start thread: '"
					<< m_name
					<< "'\n"
					<< SDL_GetError()
					<< "\n";
			}
		}
		else
		{
			std::cerr << "Thread::start: '" << m_name
				<< "' has already started!\n";
		}

		SDL_UnlockMutex ( m_mutex );
	}

	return res;
}

int mtGin::Thread::pause ()
{
	int res = 1;

	if ( 0 == SDL_LockMutex ( m_mutex ) )
	{
		switch ( m_status )
		{
		case THREAD_RUNNING:
			m_status = THREAD_PAUSED;
			res = 0;
			break;
		}

		SDL_UnlockMutex ( m_mutex );
	}

	return res;
}

int mtGin::Thread::resume ()
{
	int res = 1;

	if ( 0 == SDL_LockMutex ( m_mutex ) )
	{
		switch ( m_status )
		{
		case THREAD_PAUSED:
			m_status = THREAD_RUNNING;
			res = 0;
			break;
		}

		SDL_UnlockMutex ( m_mutex );
	}

	return res;
}

int mtGin::Thread::terminate ()
{
	int res = 1;

	if ( 0 == SDL_LockMutex ( m_mutex ) )
	{
		switch ( m_status )
		{
		case THREAD_RUNNING:
		case THREAD_PAUSED:
			m_status = THREAD_TERMINATED;
			res = 0;
			break;
		}

		SDL_UnlockMutex ( m_mutex );
	}

	return res;
}

int mtGin::Thread::join () const
{
	int res = 1;

	if ( 0 == SDL_LockMutex ( m_mutex ) )
	{
		if ( m_status == THREAD_RUNNING )
		{
			res = 0;	// Wait for thread to end
		}
		else
		{
//			std::cerr << "Thread::join: '" << m_name
//				<< "' is not running!\n";
		}

		SDL_UnlockMutex ( m_mutex );
	}

	if ( res )
	{
		return res;
	}

	SDL_WaitThread ( m_thread, nullptr );

	return 0;
}

int mtGin::Thread::get_status () const
{
	int res = 0;

	if ( 0 == SDL_LockMutex ( m_mutex ) )
	{
		res = m_status;
		SDL_UnlockMutex ( m_mutex );
	}

	return res;
}

void mtGin::Thread::get_progress (
	double	& done,
	double	& tot
	) const
{
	if ( 0 == SDL_LockMutex ( m_mutex ) )
	{
		done = m_progress;
		tot = m_progress_tot;
		SDL_UnlockMutex ( m_mutex );
	}
}

void mtGin::Thread::set_progress (
	double	const	done,
	double	const	tot
	)
{
	if ( 0 == SDL_LockMutex ( m_mutex ) )
	{
		m_progress = done;
		m_progress_tot = tot;
		SDL_UnlockMutex ( m_mutex );
	}
}

void mtGin::Thread::run_func ()
{
	m_func ();

	if ( 0 == SDL_LockMutex ( m_mutex ) )
	{
		m_status = THREAD_FINISHED;
		SDL_UnlockMutex ( m_mutex );
	}
}

