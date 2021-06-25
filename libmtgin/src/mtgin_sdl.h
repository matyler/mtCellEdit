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

#ifndef MTGIN_SDL_H_
#define MTGIN_SDL_H_

#include <SDL.h>

#include <mtkit.h>
#include <mtpixy.h>



//	mtGin = Mark Tyler's Graphical Interface Nexus



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifdef __cplusplus
extern "C" {
#endif

// C API



enum	// Object types
{
	GIN_OBJECT_TYPE_ERROR		= -1,
	GIN_OBJECT_TYPE_OK		= 0,

	GIN_OBJECT_SIGNAL_MAX		= 16,	// Per object limit
	GIN_OBJECT_SIGNAL_ID_MAX	= 1000000,

	// Core object types
	GIN_OBJECT_TYPE_OBJECT,
	GIN_OBJECT_TYPE_WINDOW,

	GIN_OBJECT_TYPE_USER		= 1000000
};



enum	// Signal types
{
	GIN_SIGNAL_ERROR		= -1,
	GIN_SIGNAL_OK			= 0,

	// Core signal types
	GIN_SIGNAL_DESTROY_OBJECT,		// About to be destroyed
	GIN_SIGNAL_DESTROY_WINDOW,		// Destroyed
	GIN_SIGNAL_SDL_EVENT,			// ptr=(ObjectData *)

	GIN_SIGNAL_USER			= 1000000
};



enum	// Misc bit flags & other values
{
	GIN_FPS_MIN			= 1,
	GIN_FPS_DEFAULT			= 60,
	GIN_FPS_MAX			= 240,

	GIN_WINDOW_CANVAS_2D		= 0,
	GIN_WINDOW_CANVAS_OPENGL	= 1,

	// Default minimums.  Caller can raise them via window_size_min().
	GIN_WINDOW_WIDTH_MIN		= 100,
	GIN_WINDOW_HEIGHT_MIN		= 100
};



#ifdef __cplusplus
}

// C++ API

namespace mtGin
{

/* NOTES:
All audio, video, and thread functions and classes can only be used after the
App singleton object is created via SDL.
Once this App object is destroyed, everything else must stop also.

Timer and thread callbacks must not touch anything in the GUI thread app!

OpenGL functionality doesn't require an App object, such as for a Qt app.
Whenever using OpenGL always set the context before the calls, and restrict to
the GUI thread.
*/

class App;
class AudioFileRead;
class AudioFileWrite;
class AudioPlay;
class AudioRecord;
class AudioVU;
class CB_Data;
class CB_SDLEvent;
class Callback;
class Object;
class Thread;
class Window;

template< typename T > class ThreadWork;



typedef std::function< int(CB_Data *) >			SignalFunc;
	// 0=Handled, 1=Not handled

typedef std::function< void() >				CallbackFunc;
typedef std::function< void(SDL_Event const &) >	SDLEventFunc;



/// Misc -----------------------------------------------------------------------

std::string clipboard ();
void clipboard_set ( char const * text );

int cpu_cores ();	// How many threads are available?
int cpu_endian ();	// 0=little 1=big

SDL_Surface * surface_from_pixmap ( mtPixmap const * pixmap );



class Callback
{
public:
	Callback ();
	~Callback ();

	void set ( CallbackFunc func );
	void start ();
	void run () const;
	void stop ();

	inline int is_running () const { return m_active; }

private:
	CallbackFunc	m_func		= nullptr;
	int		m_active	= 0;

	MTKIT_RULE_OF_FIVE( Callback )
};



class AudioVU
{
public:
	std::vector<int>	m_level;
	uint64_t		m_tick		= 0;
};



class AudioFileRead
{
public:
	AudioFileRead ();
	~AudioFileRead ();

	int open ( char const * filename, SDL_AudioSpec const & spec ) const;
	size_t read ( short * buf, size_t buflen ) const;
		// Returns number of frames (channel * short)

private:
	class Op;	// Opaque / Pimpl
	std::unique_ptr<Op> const m_op;

	MTKIT_RULE_OF_FIVE( AudioFileRead )
};



class AudioFileWrite
{
public:
	AudioFileWrite ();
	~AudioFileWrite ();

	int open ( char const * filename, SDL_AudioSpec const & spec ) const;
	int write ( short const * buf, size_t buflen ) const;

private:
	class Op;	// Opaque / Pimpl
	std::unique_ptr<Op> const m_op;

	MTKIT_RULE_OF_FIVE( AudioFileWrite )
};



class AudioPlay
{
public:
	AudioPlay ();
	~AudioPlay ();

	// e.g. Dev 0 (-1=default); Stereo, 48kHz, AUDIO_S16SYS, 4096 (85ms).
	int open_device ( int device, SDL_AudioSpec const & spec );
	void set_file ( AudioFileRead * file );

	int queue_data ( short const * buf, size_t buflen );
				// Push raw data to queue (now)
	int queue_file_data ();	// Push audio file data to queue (if required)
		// 0=Success; 1=Wrong mode; -1=File I/O error (EOF)

	int pause ();
	int resume ();
	void stop ();		// Stop using this device
	void toggle_pause_resume ();

	inline int get_status () const { return m_status; }
	inline AudioVU const & get_vu () const { return m_vu; }
	inline SDL_AudioSpec const & get_spec () const { return m_spec; }

private:
	int		m_status	= SDL_AUDIO_STOPPED;

	SDL_AudioSpec	m_spec;
	SDL_AudioDeviceID m_dev_out	= 0;

	AudioFileRead	* m_file	= nullptr;

	size_t		m_bufframes	= 0;
	Uint32		m_bufbytes	= 0;
	short		* m_buf		= nullptr;

	AudioVU		m_vu;

	MTKIT_RULE_OF_FIVE( AudioPlay )
};



class AudioRecord
{
public:
	AudioRecord ();
	~AudioRecord ();

	// e.g. Dev 0 (-1=default); Stereo, 48kHz, 4096 (85ms).
	// Format should always be AUDIO_S16SYS (if not it will fail!)
	// spec.callback is set by libmtgin.
	int open_device ( int device, SDL_AudioSpec & spec );
	void set_file ( AudioFileWrite * file );

	int pause ();
	int resume ();
	void toggle_pause_resume ();

	int get_status ();
	inline AudioVU const & get_vu () const { return m_vu; }
	inline SDL_AudioSpec const & get_spec () const { return m_spec; }

/// ----------------------------------------------------------------------------

	// NOTE: this is the callback and so is on a worker thread! Not to be
	// called by the app!
	void receive_audio (
		Uint8	const	* stream,
		int		len
		);

private:
	void stop ();			// Stop using this device & file

/// ----------------------------------------------------------------------------

	int		m_status	= SDL_AUDIO_STOPPED;
	int		m_cb_panic	= 0;	// Only callback can set!

	AudioFileWrite	* m_file	= nullptr;

	SDL_AudioSpec	m_spec;
	SDL_AudioDeviceID m_dev_in	= 0;

	AudioVU		m_vu;

	MTKIT_RULE_OF_FIVE( AudioRecord )
};



/* All objects must be created dynamically on the heap and not be held in any
smart pointer-like structures.
All objects are owned by their parents, which should all sit beneath a Window.
*/

class Object
{
public:
	Object();
	virtual ~Object();

/* SIGNALS emitted:
	GIN_SIGNAL_DESTROY_OBJECT	When Object has been deleted
*/

	virtual char const * name() const;

	inline uint64_t id() const { return (uint64_t)this; }

	int emit_signal (
		int sig_type,		// SIGNAL_TYPE_*
		CB_Data * data
		);
		// 0=Not handled, else number of successful callbacks

	int add_signal ( int sig_type, SignalFunc func );
		// =sig_id (or <0=Error)
	int remove_signal ( int sig_id );

	inline void block_signals ( bool const block )
	{
		m_block_signals = block;
	}

	void add_child ( Object * child );

private:
	void destroy_offspring();
	void detach_delete();

/// ----------------------------------------------------------------------------

	struct SignalInfo
	{
		int		type;
		int		id;
		SignalFunc	func;
		SignalInfo	* next;
	};

	class SignalStack
	{
	public:
		SignalStack ();
		~SignalStack ();

		inline SignalInfo const * get () const { return m_sig; }

		int add ( int sig_type, SignalFunc func );
		int remove ( int sig_id );

	private:
		SignalInfo	* m_sig		= nullptr;	// Linked list
		int		m_sig_total	= 0;
		int		m_sig_id_next	= 0;

		MTKIT_RULE_OF_FIVE( SignalStack )
	};

	bool		m_block_signals		= false;

	SignalStack	m_sig_stack;

	Object		* m_parent		= nullptr;
	Object		* m_child_first		= nullptr;
	Object		* m_child_last		= nullptr;
	Object		* m_sibling_before	= nullptr;
	Object		* m_sibling_after	= nullptr;

	MTKIT_RULE_OF_FIVE( Object )
};



class CB_Data
{
public:
	virtual ~CB_Data() {}
};



class CB_SDLEvent : public CB_Data
{
public:
	SDL_Event const * sdl_event = nullptr;
};



class Window : public Object
{
public:
	explicit Window (
		int canvas		// GIN_WINDOW_CANVAS_*
		);
	~Window ();

/* SIGNALS emitted:
	GIN_SIGNAL_DESTROY_WINDOW	When SDL Window is about to be deleted
	GIN_SIGNAL_SDL_EVENT		Keyboard, Mouse, etc. handler
*/

	char const * name() const		override;

	void get_geometry ( int &x, int &y, int &w, int &h, Uint32 &flags );
	void set_size_min ( int w, int h );

	void init ( char const * title,
		int x,
		int y,
		int w,
		int h,
		Uint32 flags
		);
		// Throws on error

	inline SDL_Window * sdl() const { return m_sdl_window; }
	inline SDL_Renderer * renderer() const { return m_sdl_renderer; }
	inline void set_opengl_current () const
	{
		SDL_GL_MakeCurrent ( m_sdl_window, m_glcontext );
	}

	int set_icon ( mtPixmap const * pixmap );
	int set_icon ( char const * filename );

	mtPixmap * dump_to_pixmap () const;

/// ----------------------------------------------------------------------------

	Callback	m_render;

private:
	// This hack is needed on X11 so we can restore the position correctly
	Uint32 border_store_hack ();	// Set border off
	Uint32 border_restore_hack ();	// Restores previous border setting

/// ----------------------------------------------------------------------------

	SDL_Window	* m_sdl_window		= nullptr;
	SDL_GLContext	m_glcontext;
	SDL_Renderer	* m_sdl_renderer	= nullptr;
	Uint32		m_border_hack		= 0;
	int	const	m_canvas;
};



/// App ------------------------------------------------------------------------

/* The App:
	- starts/stops SDL
	- nexus for windows
	- main loop marshalling of events to the windows or through the "App"
	- frame timing (FPS: set max or gets current rate)
	- audio
*/

class App
{
public:
	explicit App ( Uint32 flags );	// SDL init flags
	~App ();

	void set_fps_max (
		int	fps		// GIN_FPS_MIN..GIN_FPS_MAX
		);
	inline int get_fps () const { return m_frames_per_sec; }

	void main_loop ();		// Call only once at the start of app
	void main_loop_iterate ();
	inline void stop_main_loop() { m_quit = 1; }

	void window_add ( Window & window );

	// Event handler for non-window events like joysticks, controllers, etc.
	inline void set_sdl_event_callback ( SDLEventFunc const func )
	{
		m_sdl_event_cb = func;
	}

/// ----------------------------------------------------------------------------

	// Called at the beginning of each main loop iteration
	Callback	frame_callback;

private:
	void main_loop ( int loop );
	void poll_events ();

	void frame_time_delay ();
	mtGin::Window * get_window_from_sdl ( SDL_Window * sdl );
	mtGin::Window * get_window_from_id ( Uint32 id );
	Uint32 get_window_event_id ( Uint32 id );

/// ----------------------------------------------------------------------------

	std::map<SDL_Window *, mtGin::Window *> m_window_map;

	Uint32		m_frame_delay		= 0;
	Uint32		m_old_tick		= 0;
	Uint32		m_old_tick_sec		= 0;
	int		m_frames		= 0;
	int		m_frames_old		= 0;
	int		m_frames_per_sec	= 0;

	Uint32		m_win_id_old		= 0;
	SDLEventFunc	m_sdl_event_cb		= nullptr;

	int		m_main_loop_nest	= 0;
	int		m_quit			= 0;

	MTKIT_RULE_OF_FIVE( App )
};



class Thread
{
public:
	Thread ();
	~Thread ();	// Always waits for a started thread to stop

	enum
	{
		THREAD_NOT_STARTED,
		THREAD_RUNNING,
		THREAD_PAUSED,
		THREAD_TERMINATED,
		THREAD_FINISHED
	};

/// Functions called by owning thread, **NOT** to be used by worker thread -----

	int init ( std::function<void()> func, char const * name = nullptr );

	int start ();
		// 0=Started; 1=Error: not started as already running/terminated

	int resume ();
		// 0=Restarted; 1=Error: not currently paused

	int terminate ();		// Ask the worker to terminate.
		// 0=Status changed; 1=Error: not started / already terminated

	int join () const;		// Wait for thread to finish
		// 0=Thread finished; 1=Error: already finished / not started

	void get_progress ( double & done, double & tot ) const;

/// Functions called by worker thread, **NOT** to be used by owning thread -----

	void set_progress ( double done, double tot );

/// Functions called by either worker or owning thread -------------------------

	int get_status () const;	// =THREAD_*

	int pause ();
		// 0=Paused; 1=Error: not running

/// Used internally, **NOT** to be used by calling app! ------------------------

#ifdef MTGIN_PRIVATE_THREAD
	void run_func ();
#endif

private:
	SDL_mutex		* const	m_mutex;
	std::function<void()>		m_func	= nullptr;
	std::string			m_name;

	// Only available after start()
	SDL_Thread	* m_thread	= nullptr;

	// Items can change at any time so mutex lock needed for safe usage
	int		m_status	= THREAD_NOT_STARTED;
	double		m_progress	= 0.0;
	double		m_progress_tot	= 0.0;

	MTKIT_RULE_OF_FIVE( Thread )
};



/* When dealing with multiple worker threads, a queue system is needed.
Here, the GUI thread (owner) pushes work tickets into a queue ready for the next
available worker thread to take.
*/

enum
{
	THREADWORK_PUSH_ADDED,
	THREADWORK_PUSH_NOT_ADDED,
	THREADWORK_PUSH_ERROR,

	THREADWORK_POP_EMPTY_WORK_TO_COME,
	THREADWORK_POP_EMPTY_WORK_FINISHED,
	THREADWORK_POP_HAS_WORK,
	THREADWORK_POP_ERROR
};



template< typename T >
class ThreadWork
{
public:
	explicit ThreadWork ( size_t const queue_size )
		:
		m_mutex		( SDL_CreateMutex () )
	{
		m_queue.resize ( queue_size );

		if ( ! m_mutex )
		{
			std::cerr <<
				"mtGin::ThreadWork SDL_CreateMutex failure\n";
			throw 123;
		}
	}

	~ThreadWork ()
	{
		SDL_DestroyMutex ( m_mutex );
	}

	// Functions called by GUI thread, **NOT** to be used by worker thread

	int push ( T & work )
	{
		int res;

		if ( 0 == SDL_LockMutex ( m_mutex ) )
		{
			if ( m_size < m_queue.size () )
			{
				m_queue[ m_head ] = std::move ( work );

				m_head++;
				m_size++;

				if ( m_head >= m_queue.size() )
				{
					m_head = 0;
				}

				res = THREADWORK_PUSH_ADDED;
			}
			else
			{
				res = THREADWORK_PUSH_NOT_ADDED;
			}

			SDL_UnlockMutex ( m_mutex );
		}
		else
		{
			res = THREADWORK_PUSH_ERROR;
		}

		return res;
	}

	inline void finished ()	// Once the owner has finished giving orders
	{
		m_finished = true;
	}

	void clear ()
	{
		if ( 0 == SDL_LockMutex ( m_mutex ) )
		{
			m_finished = false;
			m_head = 0;
			m_tail = 0;
			m_size = 0;

			SDL_UnlockMutex ( m_mutex );
		}
	}

/// ----------------------------------------------------------------------------

	// Functions called by worker thread, **NOT** to be used by GUI thread

	int pop ( T & work )
	{
		int res;

		if ( 0 == SDL_LockMutex ( m_mutex ) )
		{
			if ( m_size > 0 )
			{
				work = std::move ( m_queue[ m_tail ] );

				m_tail++;
				m_size--;

				if ( m_tail >= m_queue.size() )
				{
					m_tail = 0;
				}

				res = THREADWORK_POP_HAS_WORK;
			}
			else
			{
				if ( ! m_finished )
				{
					res = THREADWORK_POP_EMPTY_WORK_TO_COME;
				}
				else
				{
					res =THREADWORK_POP_EMPTY_WORK_FINISHED;
				}
			}

			SDL_UnlockMutex ( m_mutex );
		}
		else
		{
			res = THREADWORK_POP_ERROR;
		}

		return res;
	}

private:
	SDL_mutex * const m_mutex;

	// Items can change at any time so mutex lock needed for safe usage
	bool		m_finished	= false;
	size_t		m_head		= 0;	// Next push
	size_t		m_tail		= 0;	// Next pop
	size_t		m_size		= 0;
	std::vector<T>	m_queue;

	MTKIT_RULE_OF_FIVE( ThreadWork )
};



}		// namespace mtGin



#endif		// C++ API



#endif		// MTGIN_SDL_H_

