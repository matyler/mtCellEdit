/*
	Copyright (C) 2020 Mark Tyler

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



namespace mtKit
{

// Private base class (callback & flags)
class UPref;

// Top level types
class UPrefInt;
class UPrefDouble;
class UPrefString;

// Derived types - UPrefInt
class UPrefBool;
class UPrefRGB;
class UPrefOption;

// Derived types - UPrefString
class UPrefStringMulti;
class UPrefFilename;
class UPrefDirectory;



class UPref : public UPrefBase
{
public:
	virtual std::string get_file_string () const = 0; // Stored to file
	virtual std::string get_ui_string () const = 0;	// Shown to user
	virtual char const * get_type_name () const = 0; // "int", "string", etc

	virtual void set_value_text ( char const * text ) = 0; // e.g. on load
	virtual void set_default_value ( bool cb ) = 0;
	virtual bool is_default () const = 0;

	inline void emit_callback () const
	{ if ( m_callback ) m_callback (); }

	inline void set_callback ( UPrefCB cb )
	{ m_callback = cb; }

	inline bool is_visible () const
	{ return (get_flag ( VISIBLE ) != 0); }

	inline void set_invisible ()
	{ m_flags &= ~VISIBLE; }

	inline int get_flag ( int const mask ) const
	{ return (m_flags & mask);}

	inline void set_description ( std::string const & info )
	{ m_description = info; }

	inline std::string get_description () const
	{ return m_description; }

	PrefType get_type () const;

	enum
	{
		ERROR		= -1,

		VISIBLE		= 1 << 0
	};

private:
	UPrefCB		m_callback = nullptr;
	int		m_flags = VISIBLE;
	std::string	m_description;
};

class UPrefInt : public UPref
{
public:
	UPrefInt ( int & var, int var_default, int min, int max )
		:
		m_var ( var ),
		m_var_default ( var_default ),
		m_min ( min ),
		m_max ( max )
	{
		var = var_default;
	}

	void set_value_text ( char const * const text )		override
	{
		int val = 0;

		if ( 0 == mtkit_strtoi ( text, & val, nullptr, 0 ) )
		{
			set_value_final ( val, false );
		}
	}

	void set_default_value ( bool const cb )		override
	{
		set_value_final ( m_var_default, cb );
	}

	bool is_default () const				override
	{
		return (m_var == m_var_default);
	}

	std::string get_file_string () const			override
	{
		char buf[32];
		snprintf ( buf, sizeof(buf), "%i", m_var );
		return std::string ( buf );
	}

	std::string get_ui_string () const			override
	{
		return get_file_string ();
	}

	char const * get_type_name () const			override
	{
		return "int";
	}

	void set_value ( int const v, bool const cb )
	{
		set_value_final ( v, cb );
	}

	inline int get_value () const { return m_var; }

	inline void get_range ( int & min, int & max ) const
	{
		min = m_min;
		max = m_max;
	}

private:
	void set_value_final ( int const v, bool const cb )
	{
		if (	m_min >= m_max
			|| (v >= m_min && v <= m_max )
			)
		{
			m_var = v;
		}

		if ( cb )
		{
			emit_callback ();
		}
	}

/// ----------------------------------------------------------------------------

	int		& m_var;
	int	const	m_var_default;
	int	const	m_min;
	int	const	m_max;
};

class UPrefBool : public UPrefInt
{
public:
	UPrefBool ( int & var, int var_default )
		:
		UPrefInt ( var, var_default, 0, 1 )
	{}

	std::string get_ui_string () const			override
	{
		if ( get_value () )
		{
			return "TRUE";
		}

		return "FALSE";
	}

	char const * get_type_name () const			override
	{
		return "boolean";
	}
};

class UPrefRGB : public UPrefInt
{
public:
	UPrefRGB ( int & var, int var_default )
		:
		UPrefInt ( var, var_default, 0, 0 )
	{}

	std::string get_ui_string () const			override
	{
		int const num = get_value ();

		char buf[32];

		snprintf ( buf, sizeof(buf), "( %i , %i , %i )",
			( (num >> 16) & 0xFF ),
			( (num >> 8) & 0xFF ),
			( num & 0xFF )
			);

		return std::string ( buf );
	}

	char const * get_type_name () const			override
	{
		return "RGB";
	}
};

class UPrefOption : public UPrefInt
{
public:
	UPrefOption (
		int & var,
		int var_default,
		std::vector<std::string> & items
		)
		:
		UPrefInt ( var, var_default, 0, (int)items.size() - 1 ),
		m_items ( std::move( items ) )
	{}

	std::string get_ui_string () const			override
	{
		int const num = get_value ();

		char buf[32];

		snprintf ( buf, sizeof(buf), "( %i ) = ", num );

		std::string txt ( buf );

		if ( num < 0 || num >= (int)m_items.size() )
		{
			txt += '?';
		}
		else
		{
			txt += m_items[ (size_t)num ];
		}

		return txt;
	}

	char const * get_type_name () const			override
	{
		return "option";
	}

	std::vector<std::string> const & get_items () const { return m_items; };

private:
	std::vector<std::string> const m_items;
};

class UPrefDouble : public UPref
{
public:
	UPrefDouble ( double & var, double var_default, double min, double max )
		:
		m_var ( var ),
		m_var_default ( var_default ),
		m_min ( min ),
		m_max ( max )
	{
		m_var = var_default;
	}

	void set_value_text ( char const * const text )		override
	{
		double v = 0.0;

		if ( 0 == mtkit_strtod ( text, &v, nullptr, 0 ) )
		{
			set_value_final ( v, false );
		}
	}

	void set_default_value ( bool const cb )		override
	{
		set_value_final ( m_var_default, cb );
	}

	bool is_default () const				override
	{
		return (m_var == m_var_default);
	}

	std::string get_file_string () const			override
	{
		char buf[32];
		snprintf ( buf, sizeof(buf), "%.15g", m_var );
		return std::string ( buf );
	}

	std::string get_ui_string () const			override
	{
		return get_file_string ();
	}

	char const * get_type_name () const			override
	{
		return "decimal";
	}

	void set_value ( double const v, bool const cb )
	{
		set_value_final ( v, cb );
	}

	inline double get_value () const { return m_var; }

	inline void get_range ( double & min, double & max ) const
	{
		min = m_min;
		max = m_max;
	}

private:
	void set_value_final ( double const v, bool const cb )
	{
		if (	m_max <= m_min
			|| (v >= m_min && v <= m_max )
			)
		{
			m_var = v;
		}

		if ( cb )
		{
			emit_callback ();
		}
	}

/// ----------------------------------------------------------------------------

	double		& m_var;
	double	const	m_var_default;
	double	const	m_min;
	double	const	m_max;
};

class UPrefString : public UPref
{
public:
	UPrefString (
		std::string & var,
		char const * const var_default,
		size_t max
		)
		:
		m_var ( var ),
		m_var_default ( var_default ? var_default : "" ),
		m_max ( max )
	{
		m_var = m_var_default;
	}

	void set_value ( std::string const & v, bool const cb )
	{
		set_value_final ( v, cb );
	}

	void set_value_text ( char const * const text )		override
	{
		set_value_final ( text, false );
	}

	void set_default_value ( bool const cb )		override
	{
		set_value_final ( m_var_default, cb );
	}

	bool is_default () const				override
	{
		return (m_var == m_var_default);
	}

	std::string get_file_string () const			override
	{
		return m_var;
	}

	std::string get_ui_string () const			override
	{
		return m_var;
	}

	char const * get_type_name () const			override
	{
		return "string";
	}

	inline std::string const & get_value () const { return m_var; }

	inline size_t get_max () const { return m_max; }

private:
	void set_value_final ( std::string const & v, bool const cb )
	{
		if (	m_max == 0 ||
			v.size () <= m_max
			)
		{
			m_var = v;
		}

		if ( cb )
		{
			emit_callback ();
		}
	}

/// ----------------------------------------------------------------------------

	std::string		& m_var;
	std::string	const	m_var_default;
	size_t		const	m_max;
};

class UPrefStringMulti : public UPrefString
{
public:
	UPrefStringMulti (
		std::string & var,
		char const * const var_default
		)
		:
		UPrefString ( var, var_default, 0 )
	{}

	char const * get_type_name () const			override
	{
		return "string multi-line";
	}
};

class UPrefFilename : public UPrefString
{
public:
	UPrefFilename (
		std::string & var,
		char const * const var_default
		)
		:
		UPrefString ( var, var_default, 0 )
	{}

	char const * get_type_name () const			override
	{
		return "filename";
	}
};

class UPrefDirectory : public UPrefString
{
public:
	UPrefDirectory (
		std::string & var,
		char const * const var_default
		)
		:
		UPrefString ( var, var_default, 0 )
	{}

	char const * get_type_name () const			override
	{
		return "directory";
	}
};



PrefType UPref::get_type () const
{
// NOTE - check derived types first
	if ( dynamic_cast<UPrefBool const *>(this) )
	{
		return PrefType::BOOL;
	}
	else if ( dynamic_cast<UPrefRGB const *>(this) )
	{
		return PrefType::RGB;
	}
	else if ( dynamic_cast<UPrefOption const *>(this) )
	{
		return PrefType::OPTION;
	}
	else if ( dynamic_cast<UPrefStringMulti const *>(this) )
	{
		return PrefType::STRING_MULTI;
	}
	else if ( dynamic_cast<UPrefFilename const *>(this) )
	{
		return PrefType::FILENAME;
	}
	else if ( dynamic_cast<UPrefDirectory const *>(this) )
	{
		return PrefType::DIRECTORY;
	}
// NOTE - check base types last
	else if ( dynamic_cast<UPrefInt const *>(this) )
	{
		return PrefType::INT;
	}
	else if ( dynamic_cast<UPrefDouble const *>(this) )
	{
		return PrefType::DOUBLE;
	}
	else if ( dynamic_cast<UPrefString const *>(this) )
	{
		return PrefType::STRING;
	}

	return PrefType::ERROR;
}



}	// namespace mtKit



/// ----------------------------------------------------------------------------



mtKit::UserPrefs::UserPrefs ()
{
}

mtKit::UserPrefs::~UserPrefs ()
{
	save ();
}

void mtKit::UserPrefs::add_int (
	char	const * const	key,
	int			& variable,
	int		const	default_value,
	int		const	min,
	int		const	max
	)
{
	add_pref ( key, new UPrefInt(variable, default_value, min, max) );
}

void mtKit::UserPrefs::add_bool (
	char	const * const	key,
	int			& variable,
	int		const	default_value
	)
{
	add_pref ( key, new UPrefBool(variable, default_value) );
}

void mtKit::UserPrefs::add_rgb (
	char	const * const	key,
	int			& variable,
	int		const	default_value
	)
{
	add_pref ( key, new UPrefRGB (variable, default_value) );
}

void mtKit::UserPrefs::add_option (
	char	const * const	key,
	int			& variable,
	int		const	default_value,
	std::vector<std::string> items
	)
{
	add_pref ( key, new UPrefOption (variable, default_value, items) );
}

void mtKit::UserPrefs::add_double (
	char	const * const	key,
	double			& variable,
	double		const	default_value,
	double		const	min,
	double		const	max
	)
{
	add_pref ( key, new UPrefDouble (variable, default_value, min, max) );
}

void mtKit::UserPrefs::add_string (
	char	const * const	key,
	std::string		& variable,
	char	const * const	default_value,
	size_t		const	max
	)
{
	add_pref ( key, new UPrefString (variable, default_value, max) );
}

void mtKit::UserPrefs::add_string_multi (
	char	const * const	key,
	std::string		& variable,
	char	const * const	default_value
	)
{
	add_pref ( key, new UPrefStringMulti (variable, default_value) );
}

void mtKit::UserPrefs::add_filename (
	char	const * const	key,
	std::string		& variable,
	char	const * const	default_value
	)
{
	add_pref ( key, new UPrefFilename (variable, default_value) );
}

void mtKit::UserPrefs::add_directory (
	char	const * const	key,
	std::string		& variable,
	char	const * const	default_value
	)
{
	add_pref ( key, new UPrefDirectory (variable, default_value) );
}

void mtKit::UserPrefs::add_ui_defaults ( UPrefUIEdit & data )
{
	add_int ( MTKIT_PREFS_COL1, data.col1, 0, 0, 0 );
	add_int ( MTKIT_PREFS_COL2, data.col2, 0, 0, 0 );
	add_int ( MTKIT_PREFS_COL3, data.col3, 0, 0, 0 );
	add_int ( MTKIT_PREFS_COL4, data.col4, 0, 0, 0 );

	set_invisible ( MTKIT_PREFS_COL1 );
	set_invisible ( MTKIT_PREFS_COL2 );
	set_invisible ( MTKIT_PREFS_COL3 );
	set_invisible ( MTKIT_PREFS_COL4 );

	add_int ( MTKIT_PREFS_WINDOW_X, data.window_x, 50, 0, 0 );
	add_int ( MTKIT_PREFS_WINDOW_Y, data.window_y, 50, 0, 0 );
	add_int ( MTKIT_PREFS_WINDOW_W, data.window_w, 800, 0, 0 );
	add_int ( MTKIT_PREFS_WINDOW_H, data.window_h, 600, 0, 0 );

	set_invisible ( MTKIT_PREFS_WINDOW_X );
	set_invisible ( MTKIT_PREFS_WINDOW_Y );
	set_invisible ( MTKIT_PREFS_WINDOW_W );
	set_invisible ( MTKIT_PREFS_WINDOW_H );
}



struct UTreeLoad
{
	explicit UTreeLoad ( std::string const & filename )
		:
		root ( mtkit_utree_load_file ( nullptr, filename.c_str (),
			nullptr, 0 ) )
	{}

	~UTreeLoad ()
	{
		mtkit_utree_destroy_node ( root );
	}

	mtUtreeNode * root;
};



int mtKit::UserPrefs::load (
	char	const * const	filename,
	char	const * const	bin_name
	)
{
	if ( filename )
	{
		m_filename = filename;
	}
	else if ( bin_name )
	{
		m_filename = mtkit_file_home ();
		m_filename += "/.config";

		mtkit_mkdir ( m_filename.c_str () );

		m_filename += '/';
		m_filename += bin_name;

		mtkit_mkdir ( m_filename.c_str () );

		m_filename += "/prefs.txt";
	}
	else
	{
		// No filename, so nothing to do
		return 0;
	}

	UTreeLoad utree ( m_filename );
	if ( ! utree.root )
	{
		return 0;
	}

	for ( mtUtreeNode * node = utree.root->child; node; node = node->next )
	{
		if (	node->type == MTKIT_UTREE_NODE_TYPE_ELEMENT
			&& node->child
			&& node->child->type == MTKIT_UTREE_NODE_TYPE_TEXT
			)
		{
			UPref * const ref = get_pref ( node->text );
			if ( ref )
			{
				ref->set_value_text ( node->child->text );
			}
		}
	}

	return 0;
}



struct UTreeSave
{
	explicit UTreeSave () : root ( mtkit_utree_new_root () )
	{}

	~UTreeSave ()
	{
		mtkit_utree_destroy_node ( root );
	}

	int save ( std::string const & filename )
	{
		return mtkit_utree_save_file ( root, filename.c_str(),
			MTKIT_UTREE_OUTPUT_DEFAULT, 0 );
	}

	mtUtreeNode * root;
};



int mtKit::UserPrefs::save () const
{
	if ( m_filename.empty() )
	{
		return 0;
	}

	UTreeSave utree;

	for ( auto && node : m_map )
	{
		auto const ref = dynamic_cast<UPref const *>(node.second.get());

		if ( ! ref || ref->is_default () )
		{
			continue;
		}

		std::string const & key = node.first;
		std::string const val = ref->get_file_string ();

		mtUtreeNode * const elem = mtkit_utree_new_element ( utree.root,
			key.c_str () );

		if ( ! mtkit_utree_new_text ( elem, val.c_str () ) )
		{
			std::cerr << "UserPrefs::save ERROR - "
				<< key << " = " << val << "\n";
			return 1;
		}
	}

	return utree.save ( m_filename );
}

static void cerr_invalid_pref_type (
	char	const * const	key,
	char	const * const	type
	)
{
	std::cerr << "Invalid pref type: " << key << " not " << type << "\n";
}

void mtKit::UserPrefs::set (
	char	const * const	key,
	int		const	val,
	bool		const	cb
	)
{
	auto * const pref = dynamic_cast<UPrefInt *>(get_pref ( key ));

	if ( pref )
	{
		pref->set_value ( val, cb );
		return;
	}

	cerr_invalid_pref_type ( key, "Int" );
}

void mtKit::UserPrefs::set (
	char	const * const	key,
	double		const	val,
	bool		const	cb
	)
{
	auto * const pref = dynamic_cast<UPrefDouble *>(get_pref ( key ));

	if ( pref )
	{
		pref->set_value ( val, cb );
		return;
	}

	cerr_invalid_pref_type ( key, "Double" );
}

void mtKit::UserPrefs::set (
	char	const * const	key,
	std::string	const & val,
	bool		const	cb
	)
{
	auto * const pref = dynamic_cast<UPrefString *>(get_pref ( key ));

	if ( pref )
	{
		pref->set_value ( val, cb );
		return;
	}

	cerr_invalid_pref_type ( key, "String" );
}

void mtKit::UserPrefs::set_callback (
	char	const * const	key,
	UPrefCB		const	cb
	)
{
	UPref * const pref = get_pref ( key );

	if ( pref )
	{
		pref->set_callback ( cb );
	}
}

void mtKit::UserPrefs::set_description (
	char	const * const	key,
	std::string	const	& info
	)
{
	UPref * const pref = get_pref ( key );

	if ( pref )
	{
		pref->set_description ( info );
	}
}

void mtKit::UserPrefs::set_invisible ( char const * const key )
{
	UPref * const pref = get_pref ( key );

	if ( pref )
	{
		pref->set_invisible ();
	}
}

void mtKit::UserPrefs::set_default_value (
	char	const * const	key,
	bool		const	cb
	)
{
	UPref * const pref = get_pref ( key );

	if ( pref )
	{
		pref->set_default_value ( cb );
	}
}

std::string mtKit::UserPrefs::get_description ( char const * key ) const
{
	UPref const * const pref = get_pref ( key );

	if ( pref )
	{
		return pref->get_description ();
	}

	return "";
}

bool mtKit::UserPrefs::is_default ( char const * key ) const
{
	UPref const * const pref = get_pref ( key );

	if ( pref )
	{
		return pref->is_default ();
	}

	return true;
}

std::string mtKit::UserPrefs::get_ui_string ( char const * key ) const
{
	UPref const * const pref = get_pref ( key );

	if ( pref )
	{
		return pref->get_ui_string ();
	}

	return "";
}

int mtKit::UserPrefs::get_int ( char const * const key ) const
{
	auto const pref = dynamic_cast<UPrefInt const *>(get_pref ( key ));

	if ( pref )
	{
		return pref->get_value ();
	}

	cerr_invalid_pref_type ( key, "Int" );

	return 0;
}

void mtKit::UserPrefs::get_int_range (
	char	const * const	key,
	int			& min,
	int			& max
	) const
{
	auto const pref = dynamic_cast<UPrefInt const *>(get_pref ( key ));

	if ( pref )
	{
		pref->get_range ( min, max );
		return;
	}

	cerr_invalid_pref_type ( key, "Int" );
}

double mtKit::UserPrefs::get_double ( char const * const key ) const
{
	auto const pref = dynamic_cast<UPrefDouble const *>(get_pref ( key ));

	if ( pref )
	{
		return pref->get_value ();
	}

	cerr_invalid_pref_type ( key, "Double" );

	return 0.0;
}

void mtKit::UserPrefs::get_double_range (
	char	const * const	key,
	double			& min,
	double			& max
	) const
{
	auto const pref = dynamic_cast<UPrefDouble const *>(get_pref ( key ));

	if ( pref )
	{
		pref->get_range ( min, max );
		return;
	}

	cerr_invalid_pref_type ( key, "Double" );
}

std::string const & mtKit::UserPrefs::get_string(
	char const * const key
	) const
{
	auto * const pref = dynamic_cast<UPrefString const *>(get_pref ( key ));

	if ( pref )
	{
		return pref->get_value ();
	}

	cerr_invalid_pref_type ( key, "String" );

	static std::string const empty;

	return empty;
}

size_t mtKit::UserPrefs::get_string_max ( char const * const key ) const
{
	auto * const pref = dynamic_cast<UPrefString const *>(get_pref ( key ));

	if ( pref )
	{
		return pref->get_max ();
	}

	cerr_invalid_pref_type ( key, "String" );

	return 0;
}

void mtKit::UserPrefs::scan_prefs ( UPrefScanCB const callback ) const
{
	for ( auto && node : m_map )
	{
		auto const ref = dynamic_cast<UPref const *>(node.second.get());

		if ( ! ref || ! ref->is_visible () )
		{
			continue;
		}

		callback ( ref->get_type (), node.first, ref->get_type_name (),
			ref->get_ui_string (), ref->is_default () );
	}
}

void mtKit::UserPrefs::scan_options (
	char	const * const	key,
	UPrefOptScanCB	const	callback
	) const
{
	auto pref = dynamic_cast<UPrefOption const *>(get_pref ( key ));

	if ( pref )
	{
		std::vector<std::string> const & items = pref->get_items ();

		for ( auto i : items )
		{
			callback ( i );
		}

		return;
	}

	cerr_invalid_pref_type ( key, "Option" );
}

void mtKit::UserPrefs::add_pref (
	char	const * const	key,
	UPref		* const	pref
	)
{
	// Temp store which destroys pref in the event of an exception
	std::unique_ptr<UPref> ptr ( pref );

	// NOTE: this code isn't used as pref duplications aren't detected.
//	m_map[ key ] = std::unique_ptr<UPref const>( pref );
//	ptr.release ();

	// NOTE: this code is used to help detect pref duplications at runtime.

	auto const ret = m_map.insert ( std::make_pair ( key, std::move (ptr)));

	if ( ret.second == false )
	{
		std::cerr << "Pref '" << key << "' already defined\n";
		throw 123;
	}
}

mtKit::UPref * mtKit::UserPrefs::get_pref ( char const * const key )
{
	auto const node = m_map.find ( key );

	if ( node == m_map.end () )
	{
		return nullptr;
	}

	return dynamic_cast<UPref *>(node->second.get());
}

mtKit::UPref const * mtKit::UserPrefs::get_pref (
	char const * const key
	) const
{
	auto const node = m_map.find ( key );

	if ( node == m_map.end () )
	{
		return nullptr;
	}

	return dynamic_cast<UPref const *>(node->second.get());
}

