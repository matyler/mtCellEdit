/*
	Copyright (C) 2018-2020 Mark Tyler

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

#ifndef MTDATAWELL_H_
#define MTDATAWELL_H_



#include <mtkit.h>
#include <mtpixy.h>



#ifdef __cplusplus

#include <vector>
#include <map>



namespace mtDW
{

class Database;

class Butt;
class Soda;
class Tap;
class Well;

class AppPassword;
class ButtSaveState;
class GlyphIndex;
class GlyphNode;
class Homoglyph;
class OTPanalysis;
class OTPinfo;
class SodaFile;
class SodaTransaction;
class TapFile;
class Utf8Font;
class WellSaveState;



static int const OTP_NAME_LEN_MIN	= 1;
static int const OTP_NAME_LEN_MAX	= 20;



// Functions return: 0 = success, NULL = fail; unless otherwise stated.

// Some int returns can use this function to get more info:
char const * get_error_text ( int error );

// By default the library reports error messages to stderr. Change this here:
// These are reference counted, so 3 "less" will require 3 "more" to re-enable.
void set_stderr_less ();
void set_stderr_more ();



class Database
{
public:
	Database ();
	~Database ();

	int open ( char const * path );

	inline mtDW::Well * get_well () const { return m_well.get (); }
	inline mtDW::Butt * get_butt () const { return m_butt.get (); }
	inline mtDW::Soda * get_soda () const { return m_soda.get (); }
	inline mtDW::Tap * get_tap () const { return m_tap.get (); }

	inline std::string const & get_path () const { return m_path; }

private:
	std::string			m_path;

	std::unique_ptr<mtDW::Well>	m_well;
	std::unique_ptr<mtDW::Butt>	m_butt;
	std::unique_ptr<mtDW::Soda>	m_soda;
	std::unique_ptr<mtDW::Tap>	m_tap;

	MTKIT_RULE_OF_FIVE( Database )
};



class Well
{
public:
	explicit Well ( char const * path = NULL );
		// Load well state from a persistent disk DB
		// path = NULL => Use ~/.config/libmtDataWell/

	~Well ();

	// Low level data
	void get_data ( uint8_t * buf, size_t buflen ) const; // buf != NULL
	int get_int () const;			// = INT_MIN..INT_MAX
	int get_int ( int modulo ) const;	// = 0..(modulo - 1)
	void shuffle ( std::vector<int> &items ) const;

	// Actions
	void add_path ( char const * path ) const;
	void empty () const;
	void save_state () const;		// Save state to disk

	// Get / Set state
	int get_files_done () const;
	int get_files_todo () const;
	void get_shifts ( int shifts[8] ) const;
	uint64_t get_seed () const;
	void set_seed ( uint64_t seed ) const;
	void set_seed_by_time () const;
	void set_shifts () const;

	// Random apps

	void app_card_shuffle ( std::string &output ) const;

	static int const COIN_TOTAL_MIN = 1;
	static int const COIN_TOTAL_MAX = 1000;
	static int const COIN_TOTAL_DEFAULT = 5;

	void app_coin_toss ( std::string &output, int total ) const;

	static int const DECLIST_TOTAL_MIN = 10;
	static int const DECLIST_TOTAL_MAX = 100000;
	static int const DECLIST_TOTAL_DEFAULT = 100;

	static double const DECLIST_MIN_LO;
	static double const DECLIST_MIN_HI;
	static double const DECLIST_MIN_DEFAULT;
	static double const DECLIST_MAX_LO;
	static double const DECLIST_MAX_HI;
	static double const DECLIST_MAX_DEFAULT;

	void app_declist (
		std::string &output,
		int total,
		double min,
		double max
		) const;

	static int const DICE_TOTAL_MIN = 1;
	static int const DICE_TOTAL_MAX = 1000;
	static int const DICE_TOTAL_DEFAULT = 5;
	static int const DICE_FACES_MIN = 2;
	static int const DICE_FACES_MAX = 1000;
	static int const DICE_FACES_DEFAULT = 6;

	void app_dice_rolls (
		std::string &output,
		int total,
		int faces
		) const;

	static int const INTLIST_TOTAL_MIN = 10;
	static int const INTLIST_TOTAL_MAX = 100000;
	static int const INTLIST_TOTAL_DEFAULT = 100;
	static int const INTLIST_MIN_LO = -2147483647;
	static int const INTLIST_MIN_HI = 2147483646;
	static int const INTLIST_MIN_DEFAULT = 0;
	static int const INTLIST_RANGE_MIN = 2;
	static int const INTLIST_RANGE_MAX = 2147483647;
	static int const INTLIST_RANGE_DEFAULT = 100;

	void app_intlist (
		std::string &output,
		int total,
		int min,
		int range
		) const;

	static int const NUMSHUFF_TOTAL_MIN = 4;
	static int const NUMSHUFF_TOTAL_MAX = 100000;
	static int const NUMSHUFF_TOTAL_DEFAULT = 10;

	void app_number_shuffle ( std::string &output, int total ) const;

	static int const PASSWORD_TOTAL_MIN = 10;
	static int const PASSWORD_TOTAL_MAX = 1000;
	static int const PASSWORD_TOTAL_DEFAULT = 100;
	static int const PASSWORD_CHAR_MIN = 4;
	static int const PASSWORD_CHAR_MAX = 32;
	static int const PASSWORD_CHAR_DEFAULT = 8;
	static int const PASSWORD_OTHER_MIN = 0;
	static int const PASSWORD_OTHER_MAX = 20;
	static char const * const PASSWORD_OTHER_DEFAULT;

	void app_passwords (
		AppPassword const &app_pass,
		int char_tot,
		std::string &output,
		int total
		) const;

	static int const PIN_TOTAL_MIN = 10;
	static int const PIN_TOTAL_MAX = 1000;
	static int const PIN_TOTAL_DEFAULT = 100;
	static int const PIN_DIGITS_MIN = 4;
	static int const PIN_DIGITS_MAX = 12;
	static int const PIN_DIGITS_DEFAULT = 4;

	void app_pins ( std::string &output, int total, int digits ) const;

	int save_file ( int const bytes, char const * filename ) const;
		// Returns error code.  Interpret via get_error_text()

/// ----------------------------------------------------------------------------

	class Op;	// Opaque / Pimpl
private:
	std::unique_ptr<Op> const m_op;

	MTKIT_RULE_OF_FIVE( Well )
};



class WellSaveState
{
public:
	inline explicit WellSaveState ( Well const * well ) : m_well ( well ) {}
	inline ~WellSaveState () { if ( m_well ) m_well->save_state (); }

private:
	Well	const * const m_well;
};



class OTPinfo
{
public:
	OTPinfo (
		char const * const name,
		char const * const comment,
		int const status,
		int const buckets
		)
		:
		m_name		( name ),
		m_comment	( comment ),
		m_status	( status ),
		m_buckets	( buckets )
	{
	}

/// ----------------------------------------------------------------------------

	std::string	m_name;
	std::string	m_comment;
	int		m_status;
	int		m_buckets;
};



class OTPanalysis
{
public:
	explicit OTPanalysis ( Butt &butt );
	~OTPanalysis ();

	static int init (		// Prepare graphics, memory, etc.
		mtPixy::Pixmap &im_8bit,
		mtPixy::Pixmap &im_16bit
		);
		// Returns error code.  Interpret via get_error_text()

	int analyse_bucket (
		mtPixmap const * image_8bit,
		mtPixmap const * image_16bit,
		int bucket
		) const;
		// Returns error code.  Interpret via get_error_text()

	int analyse_all_buckets (
		mtPixmap const * image_8bit,
		mtPixmap const * image_16bit
		) const;
		// Returns error code.  Interpret via get_error_text()

	void get_bit_percents ( double &b1, double list[8] ) const;
	double get_byte_mean () const;
	double const * get_byte_list () const;	// 256 items
	int64_t get_bucket_size () const;

/// ----------------------------------------------------------------------------

	class Op;	// Opaque / Pimpl
private:
	std::unique_ptr<Op> const m_op;

	MTKIT_RULE_OF_FIVE( OTPanalysis )
};



class Butt
{
public:
	explicit Butt ( char const * path = NULL );
		// Load butt state from a persistent disk DB
		// path = NULL => Use ~/.config/libmtDataWell/

	~Butt ();

///	Buckets
	int add_buckets ( Well * well, int tot ) const;
		// Returns error code.  Interpret via get_error_text()
	int empty_buckets () const;
		// Returns error code.  Interpret via get_error_text()

	int get_bucket_total () const;
	int get_bucket_used () const;
	int get_bucket_position () const;

///	OTP
	int add_otp ( std::string const & name ) const;
		// Returns error code.  Interpret via get_error_text()
	int delete_otp ( std::string const & name ) const;
		// Returns error code.  Interpret via get_error_text()
	int set_otp ( std::string const & name ) const;
		// Returns error code.  Interpret via get_error_text()
	int import_otp ( std::string const & path ) const;
		// Returns error code.  Interpret via get_error_text()

	// Get / Set
	char const * get_comment () const;
	void get_new_name ( Well const * well, std::string &result ) const;
	void get_otp_list ( std::vector<OTPinfo> &list ) const;
	std::string const & get_otp_name () const;
	bool is_read_only () const;
	int set_comment ( std::string const & comment ) const;
		// Returns error code.  Interpret via get_error_text()
	void set_read_only () const;
	void set_read_write () const;
	static int validate_otp_name ( std::string const & name );
		// Returns error code.  Interpret via get_error_text()

///	Encoding (changes internal OTP bookkeeping)

	int get_otp_data ( uint8_t * buf, size_t buflen ) const;
		// Returns error code.  Interpret via get_error_text()

///	Decoding

	int read_set_otp (
		std::string const & name,
		int bucket,
		int pos
		) const;
		// Returns error code.  Interpret via get_error_text()

	int64_t read_get_bucket_size () const;

	int read_get_data ( uint8_t * buf, size_t buflen ) const;
		// Returns error code.  Interpret via get_error_text()

	void save_state () const;			// Save state to disk

/// ----------------------------------------------------------------------------

	class Op;	// Opaque / Pimpl

private:
	std::unique_ptr<Op> const m_op;

	MTKIT_RULE_OF_FIVE( Butt )
};



class ButtSaveState
{
public:
	inline explicit ButtSaveState ( Butt * butt ) : m_butt ( butt ) {}
	inline ~ButtSaveState () { if ( m_butt ) m_butt->save_state (); }

private:
	Butt	* const	m_butt;
};



class Soda
{
public:
	explicit Soda ( char const * path = NULL );
		// Load soda state from a persistent disk DB
		// path = NULL => Use ~/.config/libmtDataWell/

	~Soda ();

	static int decode (
		Butt * butt,
		char const * input,
		char const * output
		);
		// Returns error code.  Interpret via get_error_text()

	int encode (
		Butt * butt,
		char const * input,
		char const * output
		) const;
		// Returns error code.  Interpret via get_error_text()

	static int multi_decode (
		Butt * butt,
		char const * input,
		char const * output
		);
		// Returns error code.  Interpret via get_error_text()

	int multi_encode (
		Butt * butt,
		char const * input,
		char const * output,
		char const * const * otp_names
		) const;
		// Returns error code.  Interpret via get_error_text()

	void set_mode ( int m ) const;
	int get_mode () const;

/// ----------------------------------------------------------------------------

	class Op;	// Opaque / Pimpl
	std::unique_ptr<Op> const m_op;

private:

	MTKIT_RULE_OF_FIVE( Soda )
};



class SodaTransaction
{
public:
	explicit SodaTransaction ( Soda & soda );
	~SodaTransaction ();

	class Op;	// Opaque / Pimpl

private:
	std::unique_ptr<Op> const m_op;

	MTKIT_RULE_OF_FIVE( SodaTransaction )
};



class Tap
{
public:
	Tap ();
	~Tap ();

	static int decode (
		Butt * butt,
		char const * bottle_in,
		char const * file_out
		);
		// Returns error code.  Interpret via get_error_text()

	static int multi_decode (
		Butt * butt,
		char const * bottle_in,
		char const * file_out
		);
		// Returns error code.  Interpret via get_error_text()

	int encode (
		Well * well,
		Butt * butt,
		Soda * soda,
		char const * bottle_in,
		char const * file_in,
		char const * bottle_out
		) const;
		// Returns error code.  Interpret via get_error_text()

	static void info ( char const * filename );

/// ----------------------------------------------------------------------------

	class Op;	// Opaque / Pimpl

private:
	std::unique_ptr<Op> const m_op;

	MTKIT_RULE_OF_FIVE( Tap )
};



class SodaFile
{
public:
	SodaFile ();

	int open ( char const * filename );
		// Returns error code.  Interpret via get_error_text()

/// ----------------------------------------------------------------------------

	mtKit::ChunkFile::Load	m_chunk;
	int			m_mode_raw;
	uint64_t		m_filesize;
	int			m_bucket_pos;
	int			m_bucket;
	std::string		m_otp_name;
};



class TapFile
{
public:
	TapFile ();
	~TapFile ();

	enum
	{
		TYPE_SND	= -20,	// Empty audio file
		TYPE_RGB	= -10,	// Empty RGB image file

		TYPE_INVALID	= 0,	// Not a valid TapFile

		TYPE_RGB_1	= 101,	// RGB + Soda, 1 bit per channel
		TYPE_SND_1	= 201	// Audio + Soda, 1 bit per frame
	};

	int open_info ( char const * filename, int & type );
		// Returns error code.  Interpret via get_error_text()
		// type = TYPE_*

	int open_soda ( char const * filename, int & type );
		// Returns error code.  Interpret via get_error_text()
		// type = TYPE_*

	size_t get_capacity () const;

	std::string const & get_soda_filename () const;

/// ----------------------------------------------------------------------------

	class Op;	// Opaque / Pimpl
	std::unique_ptr<Op> const m_op;

private:
	MTKIT_RULE_OF_FIVE( TapFile )
};



class AppPassword
{
public:
	AppPassword (
		bool lowercase,
		bool uppercase,
		bool numbers,
		std::string const &other	// "" = Don't use
		);
		// Invalid args will be quietly reset.

	void get_password (
		Well const * well,
		int char_tot,
		std::string &output
		) const;
		// Invalid args will be quietly reset.

private:
	std::vector<std::string> m_chr_list;
};



class GlyphNode
{
public:
	explicit GlyphNode ( std::string const &nodes );
		// Caller must have validated this string

	inline char get_root () const { return m_root; }

	int get_index (
		std::string const &node,
		int * bit_total,
		int * node_count	// Including root
		) const;
		// -1 = error, else 0-63

	int get_node ( int index, std::string &node ) const;
		// 0=node is populated, 1=error

/// ----------------------------------------------------------------------------

//	static size_t	const	NODE_MAX = 64;

private:
	int add ( std::string const &node );
	void recalc_bit_total ();

/// ----------------------------------------------------------------------------

	char			const	m_root;
	int				m_bit_total;
	std::map<std::string, int>	m_nodes;	// Key = node
	std::map<int, std::string>	m_index;	// Key = index
};



class GlyphIndex
{
public:
	int file_clean (
		char const * input,
		char const * output,
		std::string &info
		) const;
	int utf8_clean (
		std::string const &input,
		std::string &info,
		std::string &output
		) const;

	int get_index (
		std::string const & node,
		int * bit_total,
		int * node_count
		) const;
		// -1 = error, else 0-63 for index

	int get_root_bits (
		std::string const &node,
		char * root,		// Optional
		int * bit_total,	// Optional - Bit code capacity
		int * node_count	// Optional - Arithmetic coder capacity
		) const;

	int get_node ( char root, int index, std::string &node ) const;
		// -1 = error, else node contains the item

/// ----------------------------------------------------------------------------

protected:
	void add_root_nodes (
		std::string const &nodes	// e.g. "AÀÁÂÃÄÅꓮ"
			// First glyph must be ASCII, and is the root.
			// Length in UTF-8 must be 2,4,8,16,32,64.
			// Each glyph must be new to the m_nodes index.
		);
		// Throws on error

/// ----------------------------------------------------------------------------

	std::map<char, GlyphNode> m_root;		// Key = root
	std::map<std::string, GlyphNode const *> m_nodes;	// Key = node
};



class Homoglyph : public GlyphIndex
{
public:
	Homoglyph ();

	int file_analyse ( char const * filename, std::string &info ) const;
	int file_encode (
		char const * input_utf8,
		char const * input_bin,
		char const * output_utf8,
		Well * well,		// NULL => Don't pad with random data
		std::string &info
		) const;
	int file_decode (
		char const * input_utf8,
		char const * output_bin,
		std::string &info
		) const;

	int utf8_analyse (
		std::string const &input,
		std::string const &covert,
		std::string &info
		) const;
	int utf8_decode (
		std::string const &input,
		std::string &covert,
		std::string &info,
		std::string &output
		) const;
	int utf8_encode (
		std::string const &input,
		std::string const &covert,
		std::string &info,
		std::string &output,
		Well * well		// NULL => Don't pad with random data
		) const;

/// ----------------------------------------------------------------------------

	static int	const	INPUT_FILESIZE_MAX = 16777215;

};



class Utf8Font : public GlyphIndex
{
public:
	Utf8Font ();

	enum
	{
		// Fonts have A-Z & a-z unless otherwise stated

		TYPE_ERROR		= -1,
		TYPE_MIN		= 0,

		TYPE_ASCII		= 0,	// & 0-9
		TYPE_BOLD		= 1,	// & 0-9
		TYPE_ITALIC		= 2,
		TYPE_BOLD_ITALIC	= 3,
		TYPE_SCRIPT		= 4,
		TYPE_SCRIPT_BOLD	= 5,
		TYPE_FRAKTUR		= 6,
		TYPE_FRAKTUR_BOLD	= 7,
		TYPE_SANS		= 8,	// & 0-9
		TYPE_SANS_BOLD		= 9,	// & 0-9
		TYPE_SANS_ITALIC	= 10,
		TYPE_SANS_BOLD_ITALIC	= 11,
		TYPE_MONOSPACE		= 12,	// & 0-9
		TYPE_DOUBLE_STRIKE	= 13,	// & 0-9

		TYPE_MAX		= 13
	};

	int file_encode (
		char const * input_utf8,
		int type,
		char const * output_utf8,
		std::string &info
		);

	int utf8_encode (
		std::string const &input,
		int type,
		std::string &info,
		std::string &output
		);

	static int get_type_name ( int type, std::string &name );
		// 0 = Found; 1 = Not found

	static void get_font_list ( std::string &txt );

private:

/// ----------------------------------------------------------------------------

};



}	// namespace mtDW



#endif		// __cplusplus



#endif		// MTDATAWELL_H_

