/*
	Copyright (C) 2018 Mark Tyler

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



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



#ifdef __cplusplus



/*
METAPHOR: Random data is water.

* A well is the source of water.
* A butt holds water taken from the well.
* Soda is created by mixing water from the butt with flavour.
* A tap is used to funnel soda into a bottle.

TRANSLATION:
	well = non-reversible process to create quality HRNG data files.
		Quality HRNG = Hybrid Random Number Generator (PRNG XOR'd with
		entropy) which cannot be reversed, and is statistically random.

	butt = database of HRNG files from the well.

	soda = reversible file process: e.g.
		flavour XOR water => soda.
		soda XOR water => flavour.
		Flavour = User file.

	tap = reversible file process: get soda in and out of a bottle: e.g.
		input bottle filled with soda => output bottle.
		output bottle emptied => soda.
		bottle = PNG or FLAC file.

Mark Tyler May 2018
*/



namespace mtDW
{

class Butt;
class Soda;
class Tap;
class Well;

class SodaFile;
class TapFile;

class TapFileOp;	// Opaque / Pimpl
class ButtOp;		// Opaque / Pimpl
class SodaOp;		// Opaque / Pimpl
class TapOp;		// Opaque / Pimpl
class WellOp;		// Opaque / Pimpl



class Well
{
public:
	explicit Well ( char const * path = NULL );
		// Load well state from a persistent disk DB
		// path = nullptr => Use ~/.config/libmtDataWell/

	~Well ();

	void get_shifts ( int shifts[8] ) const;
	uint64_t get_seed () const;
	void set_seed ( uint64_t seed ) const;
	void set_seed_by_time () const;

	std::string const & get_path () const;
	int get_files_done () const;
	int get_files_todo () const;

	int add_path ( char const * path ) const;
	void empty () const;
	int save_file ( int const bytes, char const * filename ) const;

/// ----------------------------------------------------------------------------

	WellOp * const op;

private:
	Well ( const Well & );		// Disable copy constructor
};



class Butt
{
public:
	Butt ( mtKit::Random & random, char const * path = NULL );
		// Load butt state from a persistent disk DB
		// path = nullptr => Use ~/.config/libmtDataWell/

	~Butt ();

	int add_buckets ( Well * well, int tot ) const;

	int get_bucket_total () const;
	int get_bucket_used () const;
	int get_bucket_position () const;
	std::string const & get_path () const;
	std::string const & get_name () const;

	int add_name ( std::string const & name ) const;
	int set_name ( std::string const & name ) const;

	mtTree * get_name_list () const;	// Key = Directory name (char *)

	static int validate_butt_name ( std::string const & name );

	// Encoding (changes internal OTP bookkeeping)

	int otp_get_int ( int & res ) const;		// res=INT_MIN..INT_MAX
	int otp_get_int ( int modulo, int & res ) const;// res=0..(modulo - 1)
	int otp_get_data ( uint8_t * buf, size_t buflen ) const;

	// Decoding

	int read_set_butt (
		std::string const & name,
		int bucket,
		int pos
		) const;

	int read_get_data ( uint8_t * buf, size_t buflen ) const;

/// ----------------------------------------------------------------------------

	ButtOp * const op;

private:
	Butt ( const Butt & );		// Disable copy constructor
};



class Soda
{
public:
	explicit Soda ( char const * path = NULL );
		// Load soda state from a persistent disk DB
		// path = nullptr => Use ~/.config/libmtDataWell/

	~Soda ();

	static int decode (
		Butt * butt,
		char const * input,
		char const * output
		);

	int encode (
		Butt * butt,
		char const * input,
		char const * output
		) const;

	static int multi_decode (
		Butt * butt,
		char const * input,
		char const * output
		);

	int multi_encode (
		Butt * butt,
		char const * input,
		char const * output,
		char const * const * butt_names
		) const;

	void set_mode ( int m ) const;
	int get_mode () const;

/// ----------------------------------------------------------------------------

	SodaOp * const op;

private:
	Soda ( const Soda & );		// Disable copy constructor
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

	static int multi_decode (
		Butt * butt,
		char const * bottle_in,
		char const * file_out
		);

	int encode (
		Butt * butt,
		char const * bottle_in,
		char const * file_in,
		char const * bottle_out
		) const;

	static void info ( char const * filename );

/// ----------------------------------------------------------------------------

	TapOp * const op;

private:
	Tap ( const Tap & );		// Disable copy constructor
};



class SodaFile
{
public:
	SodaFile ();
	~SodaFile ();

	int open ( char const * filename );
		// 0 = Valid Soda file (header parsed correctly)
		// 1 = Error

/// ----------------------------------------------------------------------------

	mtKit::ChunkFile::Load	m_chunk;
	int			m_mode_raw;
	uint64_t		m_filesize;
	int			m_bucket_pos;
	int			m_bucket;
	std::string		m_butt_name;
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

	int open_info ( char const * filename );	// = FILE_TYPE_*
	int open_soda ( char const * filename );	// = FILE_TYPE_*

	size_t get_capacity () const;

	std::string const & get_soda_filename () const;

/// ----------------------------------------------------------------------------

	TapFileOp * const op;

private:
	TapFile ( const TapFile & );	// Disable copy constructor
};



}	// namespace mtDW



#endif		// __cplusplus



#endif		// MTDATAWELL_H_

