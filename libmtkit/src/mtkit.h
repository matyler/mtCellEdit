/*
	Copyright (C) 2008-2016 Mark Tyler

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

#ifndef MTKIT_H_
#define MTKIT_H_

extern "C" {

// gcc 4.8 needs these in a C++ context
#define __STDC_LIMIT_MACROS
#define __STDC_FORMAT_MACROS

	#include <stdio.h>
	#include <stdint.h>
	#include <limits.h>
	#include <inttypes.h>
}



typedef struct mtArg		mtArg;
typedef struct mtBulkDouble	mtBulkDouble;
typedef struct mtBulkInt	mtBulkInt;
typedef struct mtBulkStr	mtBulkStr;
typedef struct mtFile		mtFile;
typedef struct mtPrefTable	mtPrefTable;
typedef struct mtPrefValue	mtPrefValue;
typedef struct mtPrefTrans	mtPrefTrans;
typedef struct mtPrefs		mtPrefs;
typedef struct mtString		mtString;
typedef struct mtTree		mtTree;
typedef struct mtTreeNode	mtTreeNode;
typedef struct mtUtreeNode	mtUtreeNode;
typedef struct mtZip		mtZip;



#undef  MAX
#define MAX(a,b)	( ( (a) > (b) ) ? (a) : (b) )

#undef  MIN
#define MIN(a,b)	( ( (a) < (b) ) ? (a) : (b) )

#define MTKIT_DIR_SEP			'/'
#define MTKIT_FILESIZE_MAX		1234567890
#define MTKIT_DDT_MIN_DATE_YEAR		0
#define MTKIT_DDT_MAX_DATE_YEAR		5879609

#define ARG_UNUSED(xxx) ARG_UNUSED_ ## xxx __attribute__((unused))

// Used to silence the -Wunused-parameter for callbacks.  Name mangling ensures
// that if the argument is used in the future then the compiler causes an error
// and reminds the programmer to remove the macro.



enum
{
	MTKIT_FILE_NONE		= 0,
	MTKIT_FILE_ZERO		= 1<<0,	// Place an extra 0 after the file in
					// memory
	MTKIT_FILE_GUNZIP	= 1<<1	// Enable/allow/actioned gzip
					// compress/decompress
};

enum
{
	MTKIT_UTREE_NODE_TYPE_NONE	= 0,

	MTKIT_UTREE_NODE_TYPE_ROOT,
	MTKIT_UTREE_NODE_TYPE_ELEMENT,
	MTKIT_UTREE_NODE_TYPE_TEXT,
	MTKIT_UTREE_NODE_TYPE_COMMENT,

	MTKIT_UTREE_NODE_TOTAL
};

enum
{
	MTKIT_UTREE_OUTPUT_INDENTS	= 1 << 0,
	MTKIT_UTREE_OUTPUT_NEWLINES	= 1 << 1,
	MTKIT_UTREE_OUTPUT_TEXT_NEWLINES = 1 << 2
};

#define MTKIT_UTREE_OUTPUT_DEFAULT \
		(MTKIT_UTREE_OUTPUT_INDENTS | MTKIT_UTREE_OUTPUT_NEWLINES)



enum
{
	MTKIT_PREF_TYPE_NONE,

	MTKIT_PREF_TYPE_INT,		// If opt used: "min	max"
					// else = +/-1000000
	MTKIT_PREF_TYPE_BOOL,		// Stored as int: 0 = false 1 = true
	MTKIT_PREF_TYPE_RGB,		// Stored as int: (((R) << 16) +
					// ((G) << 8) + (B))
	MTKIT_PREF_TYPE_OPTION,		// Stored as int: 0 = o1 1 = o2
					// opt = "o1	o2	..."
	MTKIT_PREF_TYPE_DOUBLE,		// If opt used: "min	max	step"
					// else = +/-1000000,0.001
	MTKIT_PREF_TYPE_STR,		// NUL terminated string.  If opt used,
					// it sets the max chars
	MTKIT_PREF_TYPE_STR_MULTI,	// NUL terminated multi line string.
	MTKIT_PREF_TYPE_FILE,		// Stored as string: implies that file
					// picker can be used
	MTKIT_PREF_TYPE_DIR,		// Same as filename but picker only
					// stores directory

	MTKIT_PREF_TYPE_TOTAL
};

enum
{
	MTKIT_ARG_NONE,

	MTKIT_ARG_SWITCH,		// Switch - No data follows
	MTKIT_ARG_INT,			// Integer follows in next argument
	MTKIT_ARG_STRING,		// String follows in next argument
	MTKIT_ARG_DOUBLE,		// Double precision float follows in
					// next argument
	MTKIT_ARG_TOTAL
};

enum
{
	MTKIT_ARG_ERROR_NONE,

	MTKIT_ARG_ERROR_UNKNOWN,	// Switch in arguments not found in list
	MTKIT_ARG_ERROR_DATA,		// Data passed in extra argument not
					// recognised
	MTKIT_ARG_ERROR_TERMINATED,	// Premature termination - extra
					// argument expected
	MTKIT_ARG_ERROR_BAD_TYPE,	// Bad argument type found in list

	MTKIT_ARG_ERROR_TOTAL
};

enum
{
	MTKIT_ZIP_OK_DONT_FREE	=	-10,	// When loading, user callback
						// can keep memory
	MTKIT_ZIP_STOP		=	-1,
	MTKIT_ZIP_OK		=	0,
	MTKIT_ZIP_ERROR		=	1,
	MTKIT_ZIP_ERROR_MEM	=	11,
	MTKIT_ZIP_ERROR_FILE	=	12,
	MTKIT_ZIP_ERROR_HEADER	=	13,
	MTKIT_ZIP_ERROR_ZLIB	=	20,

	MTKIT_ZIP_ERROR_USER	=	100
	// All values over 100 are reserved for user callback errors
};



typedef int (* mtTreeFuncCmp) (		// Compare 2 node keys
	void	const	* k1,
	void	const	* k2
	);
	// -1 : k1 < k2
	//  0 : k1 = k2
	//  1 : k1 > k2

typedef void (* mtTreeFuncDel) (	// Delete key/value data before node is
	mtTreeNode	* node		// destroyed
	);

typedef int (* mtTreeFuncDup) (
	mtTreeNode	* src,		// Duplicate data & key from here ...
	mtTreeNode	* dest		// ... and put them here
	);
	// 0 = success
	// 1 = fail

typedef int (* mtTreeFuncScan) (	// Don't add/remove anything from the
	mtTreeNode	* node,		// tree during this callback
	void		* user_data
	);
	// 0 = continue
	// 1 = stop

typedef void (* mtPrefCB) (		// Called when a preference changes
	mtPrefValue	* piv,
	int		callback_data,
	void		* callback_ptr
	);

typedef int (* mtArgFunc) (		// Called after an argument is actioned
	mtArg		const *	mtarg,	// Argument encountered
	int			arg,	// Arg number
	int			argc,	// Total args
	char	const *	const *	argv,	// Arg list
	void			* user_data
	);
	// 0 = continue parsing
	// 1 = Stop

typedef int (* mtArgFileFunc) (		// Callback for when a filename is
					// encountered
	char	const *	filename,
	void		* user_data
	);
	// 0 = continue parsing
	// 1 = Stop

typedef int (* mtArgErrorFunc) (	// Callback for when an error occurs
	int			error,	// Error number
	int			arg,	// Arg number that causes the error
	int			argc,
	char	const *	const *	argv,
	void			* user_data
	);
	// 0 = continue parsing
	// 1 = Stop

typedef int (* mtZipLoadFunc) (
	char	const	* name,
	void		* buf,
	int32_t		buf_len,	// 0 NUL byte always at buf[buf_len]
	int32_t		year,
	int8_t		month,
	int8_t		day,
	int8_t		hour,
	int8_t		minute,
	int8_t		second,
	void		* user_data
	);
	// MTKIT_ZIP_OK - Continue, library free's allocated memory
	// MTKIT_ZIP_OK_DONT_FREE - Continue, caller free's allocated memory
	// MTKIT_ZIP_STOP - Stop, library free's allocated memory
	// Any other value stops, and is returned from mtkit_zip_load function



struct mtBulkInt
{
	char	const	* name;
	int		* var;
};

struct mtBulkDouble
{
	char	const	* name;
	double		* var;
};

struct mtBulkStr
{
	char	const	* name;
	char		** var;
};

struct mtTreeNode
{
	void		* key,
			* data
			;
	mtTreeNode	* left,		// Child with a lower key
			* right		// Child with a higher key
			;
	int		balance		// Balance between nodes below
			;		// (left = -1 right = +1)
};

struct mtUtreeNode
{
	int		type;		// MTKIT_UTREE_NODE_TYPE_*
	char		* text;
	mtTree		* attribute_tree;
	mtUtreeNode	* previous,	// Basic linked list (NOT circular)
			* next,
			* parent,
			* child,
			* child_last;
};

struct mtTree
{
	mtTreeNode	* root;
	mtTreeFuncCmp	cmp;		// Compares 2 node keys
	mtTreeFuncDel	del;		// Destroys key/value data
};



/*
	Prefs tables terminated by name = NULL.
	If (bulk.var == NULL) it is skipped.
*/

struct mtPrefTable
{
	char	const	* key;		// Unique index, valid chars:
					// (a-z A-Z 0-9 _ - . : ) or
					// (anything < 0) checked using
					// mtkit_utree_valid_name ()
	int	const	type;		// As MTKIT_PREF_TYPE_*
	char	const	* def;		// Default value
	char	const	* description;	// Human readable description for user
	mtPrefCB	callback;	// Called when preference changes
	int	const	callback_data;	// Callback data
	char	const	* opt;		// Used for options in an option list
	void		* callback_ptr;	// Callback pointer
};

/*
	opt is also used by double in the prefs GUI for:
	double: decimal places || min, max, decimal places
	By default: min = -1000000, max = 1000000, dp = 3
	Checking is *NOT* done on loading prefs from a file.
*/

struct mtPrefValue			// Each data pointer in an mtPrefs tree
{					// node points to this
	char		* value;	// Allocated string for current value

	// Allocated by duplicating the info from mtPrefTable item
	char		* key;
	int		type;
	char		* def;
	char		* description;
	mtPrefCB	callback;
	int		callback_data;
	char		* opt;
	void		* callback_ptr;
};

struct mtPrefTrans
{
	char	const	* dest;
	char	const	* src;		// Key names.  If either is NULL, stop.
};

struct mtArg
{
	char		const *	argument;
					// String passed as argument after '-'
	int		const	type;	// Type of argument
	void			* variable;
					// Pointer to variable to change
	int		const	value;	// Value to place in integer variable
					// for ARG_TYPE_SWITCH
	mtArgFunc	const	callback;
					// Called after an argument is actioned
};



// Functions return: 0 = success, NULL = fail; unless otherwise stated.



char * mtkit_file_load (
/*
	Load a whole file into memory (possibly gunzip'd). Can only load files
	less than MTKIT_FILESIZE_MAX in size.
*/
	char	const	* filename,
	int		* bytes,	// Bytes read stored here
	int		in_flag,	// Input flags as per MTKIT_FILE_*
	int		* out_flag	// Optional output results as per
	);				// MTKIT_FILE_*

int mtkit_file_save (			// Save a memory chunk to a file
					// (possibly gzip'd)
	char	const	* filename,
	char	const	* buf,		// Bytes to save
	int		buf_size,	// Number of bytes to save.
					// 0 <= buf_size <= MTKIT_FILESIZE_MAX
	int		out_flag	// As per MTKIT_FILE_*
	);

char const * mtkit_file_home (		// Get the home directory
	void
	);

int mtkit_file_readable (
	char	const	* filename
	);
	// 0 = Bad arg or file not readable
	// 1 = File exists and is readable

int mtkit_file_writable (
	char	const	* filename
	);
	// 0 = Bad arg or file not writable
	// 1 = File writable or doesn't exist

char * mtkit_file_readline (		// Read a line of input and allocate to
					// new NUL terminated string.
	FILE		* fp,		// Excludes newline char(s) at end for
					// Unix/Mac/DOS, i.e. \n \r \r\n
					// fp can be stdin to read input from
					// the command line or a pipe
	int		* len,		// Optional: put length of string here
	int		* len_nl	// Optional: put length of newline
					// string here
	);

int mtkit_file_load_stdin (		// Load stdin into a memory chunk
	char		** buf,
	size_t		* buflen
	);
	// 0 = buf populated with buflen bytes (including terminating NUL).
	// 1 = Error.

/*
The mtFile structure is designed to allow the writing of a file to disk
or memory.  It is useful for code that wants to write to either at different
times, and makes switching easy and seemless.

The memory file write avoids the need for a temp file on disk, which may be
costly or undesirable.
*/

mtFile * mtkit_file_open_disk (		// Open a new disk file
	char	const	* filename
	);

mtFile * mtkit_file_open_mem (		// Open a new memory file
	void
	);

int mtkit_file_close (			// Close an mtFile & destroy structure
	mtFile		* mtfp
	);

int mtkit_file_write (			// Write a chunk of data to an mtFile
	mtFile		* mtfp,
	void	const	* mem,		// Start of memory to write
	int64_t		mem_len		// Number of bytes to write
	);

int mtkit_file_write_string (		// Write a C string to an mtFile
	mtFile		* mtfp,
	char	const	* str		// NUL terminated C string
	);

int mtkit_file_get_mem (
	mtFile		* mtfp,
	void		** buf,		// Put buffer start here (NULL = don't)
	int64_t		* buf_len	// Put buffer length here (NULL = don't)
	);

int mtkit_file_header_gz (			// Is this a .gz file header?
	unsigned char	const	* mem,		// 31, 139
	int			mem_size	// > 20
	);
	// 0 = No
	// 1 = Yes

int mtkit_file_header_zip (			// Is this a .zip file header?
	unsigned char	const	* mem,		// 0x50, 0x4b, 0x03, 0x04
	int			mem_size	// > 30
	);
	// 0 = No
	// 1 = Yes

mtString * mtkit_string_new (
	char	const	* cs		// C String or NULL
	);

int mtkit_string_destroy (
	mtString	* str
	);

char * mtkit_string_destroy_get_buf (	// Destroy mtString, give buffer to
	mtString	* str		// caller.
	);

char const * mtkit_string_get_buf (
	mtString	* str
	);

size_t mtkit_string_get_len (
	mtString	* str
	);

int mtkit_string_append (
	mtString	* str,
	char	const	* cs		// C String
	);

char * mtkit_string_join (
	char	const	* sta,
	char	const	* stb,
	char	const	* stc,
	char	const	* std
	);

int mtkit_strnncpy (			// dest = src
	char		* dest,		// On succes dest is NUL terminated, but
	char	const	* src,		// possibly truncated.
	size_t		destSize	// destSize > 0.  All args checked.
	);

int mtkit_strnncat (			// dest = dest + src
	char		* dest,		// On succes dest is NUL terminated, but
	char	const	* src,		// possibly truncated.
	size_t		destSize	// destSize > 0.  All args checked.
	);

int mtkit_strtod (			// Read in a string as a double
	char	const	* input,	// Input string to parse NUL terminated
	double		* result,	// Put the numerical result here
					// (NULL = don't)
	char		** next,	// Put next unparsed char here
					// (NULL = don't)
	int		strict		// 1 = string must only contain
					// number/whitespaces
	);

int mtkit_strtoi (			// Read in a string as an int
	char	const	* input,	// Input string to parse NUL terminated
	int		* result,	// Put the numerical result here
					// (NULL = don't)
	char		** next,	// Put next unparsed char here
					// (NULL = don't)
	int		strict		// 1 = string must only contain
					// number/whitespaces
	);

char * mtkit_strtok (			// Extract the n'th token and create a
					// new string of it
	char	const	* input,	// Input string, NUL terminated
	char	const	* delim,	// List of character delimeters, NUL
					// terminated
	int		ntok		// n'th token (0 = first)
	);
	// Pointer to newly allocated string

int mtkit_strtok_count (		// Count the number of tokens in a
					// string
	char	const	* input,	// Input string, NUL terminated
	char	const	* delim		// List of character delimeters, NUL
					// terminated
	);
	// -1 = fail, else number of tokens

int mtkit_strtok_num (			// Extract the n'th token and convert it
					// to a double
	char	const	* input,	// Input string, NUL terminated
	char	const	* delim,	// List of character delimeters, NUL
					// terminated
	int		n,		// n'th token (0 = first)
	double		* num		// Put result here
	);

char * mtkit_strcasestr (		// Find a string in another string
					// regardless of case
	char	const	* haystack,
	char	const	* needle
	);
	// NULL = not found, else position in haystack where needle is

int mtkit_strfreedup (			// Free an old string, put in
					// duplicated string.
	char		** spp,		// Pointer to a current string pointer
					// (to be freed)
	char	const	* str		// New string to be dup'd, or NULL
	);

char * mtkit_strtohtml (		// Create a valid HTML string from a C
					// string
	char	const	* input		// chars < 32 => 32, < > & " get
					// substituted
	);

int mtkit_strmatch (			// String match with wildcards
	char	const	* string,	// C string
	char	const	* pattern,	// Pattern to match, including ? *
	int		mode		// 1 = case sensitive
					// 0 = case insensitive
	);
	// -3 = OS error
	// -2 = Arg error
	// -1 = no match, else returns offset in 'string' of first non wildcard
	// character match.

/*
? = any single char, * = any number of chars.  To represent these or \ chars in
'pattern' they must be preceded by a \ character, e.g. "\*\\\?" will search for
characters "*\?".

	\n = newline
	\t = tab
	\r = carriage return

Results when putting \ before any other chars are undefined.

If pattern is "*text*" then 'text' could be anywhere in the string, "*text"
'text' must be at the end and "text*" means 'text' must be at the beginning.

	"abcdef", "*c*"		= 2
	"abcdef", "*?c*"	= 2
	"abcdef", "*c"		= -1
	"abcdef", "c"		= -1
*/

int mtkit_strnonspaces (		// Does a string contain any
					// non-whitespace characters?
	char	const	* input		// Input string to parse NUL terminated
	);
	// 0 = String only has whitespaces
	// 1 = Has non-whitespace characters

/*
	Add thousand separator(s) to a string containing a number, e.g.
	str2thousands ( dest, "1234567", 32, ',', '-', '.', 3, 0 );
	Creates:

	1234567		->	1,234,567
	-123456		->	-123,456
	-123456.7891	->	-123,456.7891


	Can also be used to separate any text between two delimeters, e.g.
	str2thousands ( dest, "aaa>270907<bbb", 32, '-', '>', '<', 2, 0 );
	Creates:

	aaa>27-09-07<bbb

*/

int mtkit_strtothou (
	char		* dest,		// Output buffer
	char	const	* src,		// Input string (can be same as
					// destination)
	int		dest_size,	// Size of output buffer
	char		separator,	// Separator character to use,
					// typically ','
	char		minus,		// Minus character, typically '-'
	char		dpoint,		// Decimal point character,
					// typically '.'
	int		sep_num,	// Numbers to contain between
					// separators, typically 3
	int		right_justify	// Should output string be right
					// justified?
	);

/*
	ddt = decimal day time => each 1.0 is a day.
	e.g.
	00:00			= 0.00
	00:00:00		= 0.00
	1/1/0 00:00:00		= 0.00
	2/1/0			= 1.00
	1/1/2000		= 730485.00
	1/1/2000 12:34:56	= 730485.524259

	String Formats:
	h:m			= Shortened time format
					(seconds = 0, date = 1/1/0)
	h:m:s			= Full time format (date = 1/1/0)
	d/m/y			= Full date format (time = 0:0:0)
	d-m-y			= Alternative date separators
	d/m/y h:m:s		= Complete date/time format (:s optional)
*/

int mtkit_strtoddt (			// String (above formats) to double
					// date/time
	char	const	* input,	// Input string to parse NUL terminated
					// ([d/m/y] [h:m:s])
					// If input day is > 31 then interpret
					// as y/m/d
	double		* result	// Put result here
	);

int mtkit_itoddt (
	int		day,		// 1..31
	int		month,		// 1..12
	int		year,		// 0..
	int		hour,		// 0..23
	int		minute,		// 0..59
	int		second,		// 0..59
	double		* result	// Put result here
	);

int mtkit_ddttoi (
	double		datetime,	// (>= 0.0)  0.* => only populate time
	int		* day,
	int		* month,
	int		* year,
	int		* hour,
	int		* minute,
	int		* second
	);

int mtkit_ddt_weekday (
	double		datetime
	);
	// -1 = error
	//  0 = Sun
	//  1 = Mon
	//  2 = Tue
	//  3 = Wed
	//  4 = Thu
	//  5 = Fri
	//  6 = Sat

int mtkit_string_encoding_conversion (	// Convert a string from one encoding to
					// another, allocating a new string for
					// the result
	char	const	* text_in,	// NUL terminated string to convert
	char	const	* text_in_encoding,
					// e.g. "ASCII", "ISO-8859-1", "UTF-8"
	char		** text_out,	// Pointer to storage space for result
	char	const	* text_out_encoding
					// e.g. "ASCII", "ISO-8859-1", "UTF-8"
	);
	// 0 = success
	// < 0 = failure

char ** mtkit_string_argv (		// Split C string into arguments
					// separated by whitespaces.
	char	const	* input		// Args can be contained inside
					// " quotes.
					// Backslashes imply literal char
					// follows, e.g. \\ = \, \" = "
	);
	// NULL = fail, else NULL terminated argv of allocated strings

int mtkit_string_argv_free (		// Free strings array, and its strings
	char		** args
	);

/*
	UTF-8 convenience functions
*/

int mtkit_utf8_string_legal (
	unsigned char	const	* src,	// Is this a valid UTF-8 string?  Must
					// be NUL terminated
	size_t			bytes	// Check this many bytes (<1 => stop
					// when NUL reached)
	);
	// 1 = legal

int mtkit_utf8_to_utf32 (
	unsigned char	const	* src,	// Start of UTF-8 character
	uint32_t		* unicode // Where the code is to be put
					// (NULL = don't use)
	);
	// -1 = error, otherwise bytes used for character 1-4

char * mtkit_utf8_from_cstring (	// Create new UTF8 string.
	char	const	* cstring	// If not UTF8 convert from ISO8859-1.
	);
	// Caller must free() the result.

int mtkit_utf8_len (			// Count the number of characters in
					// this UTF-8 string
	unsigned char	const	* src,	// Valid UTF-8 string
	size_t			bytes	// Total bytes in src to check
					// ( 0 => stop when NUL reached )
	);
	// 0 = error

int mtkit_utf8_offset (			// Count the number of bytes for a given
					// number of characters
	unsigned char	const	* src,	// Valid UTF-8 string
	int			num	// UTF-8 Characters to count
	);
	// -1 = fail, else the byte offset for the num'th character

/*
	Do a conversion of an ISO-8859-1 chunk to UTF-8.
	This is slightly faster and much less memory intensive than using
	mtkit_string_encoding_conversion.
*/

char * mtkit_iso8859_to_utf8 (		// Allocate new UTF-8 memory chunk
					// (+ extra trailing NUL)
	char	const	* input,
	size_t		bytes,		// 0 = use strlen ( input )
	size_t		* new_size	// Size of new allocated memory chunk
	);				// (NULL = don't use)

mtTree * mtkit_tree_new (
	mtTreeFuncCmp	cmp_func,	// Function to compare 2 keys
	mtTreeFuncDel	del_func	// Function to release key & data
					// structures (NULL = none)
	);

int mtkit_tree_destroy (		// Destroy a whole tree
	mtTree		* tree
	);

mtTree * mtkit_tree_duplicate (
	mtTree		* tree,
	mtTreeFuncDup	duplicate	// NULL = copy key/data as static data
	);

int mtkit_tree_node_add (
	mtTree		* tree,
	void		* key,
	void		* data
	);
	// 0 = not added
	// 1 = added
	// 2 = overwritten old node

mtTreeNode * mtkit_tree_node_find (
	mtTree			* tree,
	void		const	* key
	);				// Find a node

int mtkit_tree_node_remove (
	mtTree			* tree,
	void		const	* key
	);
	// 0 = not removed
	// 1 = removed

int mtkit_tree_scan (			// Scan each node in tree
	mtTree		* tree,
	mtTreeFuncScan	callback,
	void		* user_data,	// Passed to callback
	int		direction	// 0 = Left to right; 1 = Right to left
	);
	// 0 = success
	// 1 = error
	// 2 = user termination

mtUtreeNode * mtkit_utree_load_mem (
	mtUtreeNode	* parent,	// NULL = create new root
	char		* buf,		// Memory buffer containing data
	size_t		size,
	char		** breakpoint	// Put pointer to parsing error in buf
					// here, NULL = don't
	);
	// NULL = nothing loaded, else something loaded

mtFile * mtkit_utree_save_file_mem (
	mtUtreeNode	* node,		// Node to save
	int		output		// MTKIT_UTREE_OUTPUT_*
	);

mtUtreeNode * mtkit_utree_new_root (	// Create new root node
	void
	);

mtUtreeNode * mtkit_utree_new_element (
	mtUtreeNode		* parent,
	char		const	* name
	);

mtUtreeNode * mtkit_utree_get_node (	// Find a child of this node that has
					// this name/type
	mtUtreeNode		* parent,
	char		const	* text,	// NULL = Don't check
	int			type	// 0 = Any type
	);

mtUtreeNode * mtkit_utree_get_node_next ( // Find next node after
					// mtkit_utree_get_node
	mtUtreeNode		* start,
	char		const	* text,	// NULL = Don't check
	int			type	// 0 = Any type
	);

int mtkit_utree_destroy_node (
	mtUtreeNode	* node
	);
	// 0 = destroyed
	// 1 = not destroyed

int mtkit_utree_get_attribute_int (
	mtUtreeNode		* const	node,
	char		const	* const	name,
	int			* const	value
	);
	// -1 = Attribute not found
	//  0 = Attribute value returned
	//  1 = Attribute found, value not int

int mtkit_utree_get_attribute_double (
	mtUtreeNode		* const	node,
	char		const	* const	name,
	double			* const	value
	);
	// -1 = Attribute not found
	//  0 = Attribute value returned
	//  1 = Attribute found, value not double

int mtkit_utree_get_attribute_str (
	mtUtreeNode		* node,
	char		const	* name,
	char		const	** value
	);

int mtkit_utree_set_attribute_str (
	mtUtreeNode		* node,
	char		const	* name,
	char		const	* value
	);

int mtkit_utree_bulk_get (		// Get a bunch of attribute values
					// (missing items skipped)
	mtUtreeNode	* node,
	mtBulkInt	* table_i,	// NULL = skip
	mtBulkDouble	* table_d,	// NULL = skip
	mtBulkStr	* table_s	// NULL = skip.  All strings created
					// via mtkit_strfreedup ()
	);

int mtkit_utree_bulk_set (		// Set a bunch of attribute values
	mtUtreeNode	* node,
	mtBulkInt	* table_i,	// NULL = skip
	mtBulkDouble	* table_d,	// NULL = skip
	mtBulkStr	* table_s	// NULL = skip
	);

mtPrefs * mtkit_prefs_new (		// Create new prefs structure
	mtPrefTable	const	* table	// Table to add (NULL = don't)
	);

int mtkit_prefs_destroy (		// Destroy a prefs structure
	mtPrefs		* prefs
	);

int mtkit_prefs_block_callback (	// Stop all callbacks
	mtPrefs		* prefs
	);

int mtkit_prefs_unblock_callback (	// Restart callbacks after blocking
	mtPrefs		* prefs
	);

int mtkit_prefs_set_callback (		// Change callback & user data
	mtPrefs		* prefs,
	char	const	* key,
	mtPrefCB	callback,
	void		* callback_ptr
	);

int mtkit_prefs_get_int (		// Retrieve an integer
	mtPrefs		* prefs,
	char	const	* key,
	int		* value		// Put the value here
	);

int mtkit_prefs_get_double (
	mtPrefs		* prefs,
	char	const	* key,
	double		* value		// Put the value here
	);

int mtkit_prefs_get_str (
	mtPrefs		* prefs,
	char	const	* key,
	char	const	** value	// Put the value here
	);

int mtkit_prefs_set_default (		// Set to default
	mtPrefs		* prefs,
	char	const	* key
	);

int mtkit_prefs_set_int (		// Set an integer
	mtPrefs		* prefs,
	char	const	* key,
	int		value
	);

int mtkit_prefs_set_double (
	mtPrefs		* prefs,
	char	const	* key,
	double		value
	);

int mtkit_prefs_set_str (
	mtPrefs		* prefs,
	char	const	* key,
	char	const	* value
	);

char const * mtkit_prefs_type_text (	// Return a static string
					// representation of type
	int		type
	);

/*
This puts a value into a string.  Simple direct conversion for
string/numerical types but does fiddly work for boolean, RGB, and option types.
*/
void mtkit_prefs_get_str_val (
	mtPrefValue		* piv,		// Item
	char		const	* value,	// Value to convert
	char			* buf,		// Destination buffer
	size_t			buf_size	// Bytes in buffer
	);

int mtkit_prefs_bulk_get (		// Get a bunch of values (missing items
					// skipped)
	mtPrefs			* prefs,
	mtBulkInt	const	* table_i,	// NULL = skip
	mtBulkDouble	const	* table_d,	// NULL = skip
	mtBulkStr	const	* table_s	// NULL = skip.  All strings
						// created via mtkit_strfreedup
	);

int mtkit_prefs_bulk_set (		// Set a bunch of values
	mtPrefs			* prefs,
	mtBulkInt	const	* table_i,	// NULL = skip
	mtBulkDouble	const	* table_d,	// NULL = skip
	mtBulkStr	const	* table_s	// NULL = skip
	);

/*
	Tables terminated by name = NULL.
	If (bulk.var == NULL) it is skipped.
*/

int mtkit_prefs_value_mirror (		// Copy values from src to dest (all
					// items in table)
	mtPrefs			* dest,
	mtPrefs			* src,
	mtPrefTable	const	* table
	);

int mtkit_prefs_value_copy (		// Copy values from src to dest (all
					// items in table)
	mtPrefs			* dest,
	mtPrefs			* src,
	mtPrefTrans	const	* table
	);

mtTree * mtkit_prefs_get_tree (		// Return core tree structure
	mtPrefs		* prefs
	);

int mtkit_arg_parse (
	int			argc,
	char	const *	const *	argv,
	mtArg	const *		arg_list,
				// Terminated when
				// arg_list[].argument == NULL

	mtArgFileFunc		file_func,
				// Callback for when a filename is
				// encountered. NULL = skip

	mtArgErrorFunc		error_func,
				// Callback for error. NULL = skip

	void			* user_data
				// Passed to all callbacks
	);
	// 0 = success
	// 1 = fail
	// 2 = callback termination

int mtkit_arg_int_boundary_check (
	char	const	* arg_name,
	int		arg_val,
	int		min,
	int		max
	);
	// -1 = Bad argument
	// 0  = Within bounds
	// 1  = Out of bounds, reported error to stderr

/*
int mtkit_arg_double_boundary_check (
	char	const	* arg_name,
	double		arg_val,
	double		min,
	double		max
	);
	// -1 = Bad argument
	// 0  = Within bounds
	// 1  = Out of bounds, reported error to stderr
*/

int mtkit_arg_string_boundary_check (
	char	const	* arg_name,
	char	const	* arg_val,
	int		min,		// strlen, -1 = don't check
	int		max		// strlen, -1 = don't check
	);
	// -1 = Bad argument
	// 0  = Valid string
	// 1  = Invalid string, reported error to stderr


/*
The following code is a simple implementation of the ZIP file format.
Its goal is to provide a simply way to create and read ZIP archives for general
programming tasks.  Due to its simplicity it does have limitations.

mtkit_zip_load () cannot read all types of ZIP file created with other
programs.  Only those ZIP archives created with mtkit_file_save_zip () or other
programs using the old PKZIP standard (i.e. version 2 using deflate compression)
are guaranteed to work.	If you plan to manipulate ZIP archives containing large
files (e.g. >1GB) on a system with less memory than this you should not use
these functions. You will need to use code that is more memory frugal.

My goal with this work is simplicity, which means expanding each file in the
ZIP archive as a single chunk of memory.  This vastly simplifies the API here,
and also the code in the library, and works well for 99% of ZIP archives on
modern computers (i.e. most PC's have many megabytes of free RAM and very few
files inside a ZIP archive are likely to exceed this, especially as this library
is aimed at small scale data storage rather than multimedia files).

Can only access files less than 2GB in size.
*/

mtZip * mtkit_zip_save_open (		// Start saving a ZIP file
	char	const	* filename	// Must be a valid string until
					// mtkit_file_save_zip_close ()
	);

int mtkit_zip_save_file (		// Add this memory chunk to a ZIP file
	mtZip		* zip,
	char	const	* name,
	void		* mem,		// Memory chunk to save
	int32_t		mem_len,	// Bytes in chunk
	int8_t		deflate,	// 1 = Try to deflate memory chunk
	int32_t		year,
	int8_t		month,
	int8_t		day,
	int8_t		hour,
	int8_t		minute,
	int8_t		second
	);

/*
This must always be called to:
	1. End saving (or)
	2. Cleanup after a failure of mtkit_file_save_zip_file ()
*/
int mtkit_zip_save_close (
	mtZip		* zip
	);
	// 0 = successfully saved a zip file

int mtkit_zip_load (			// Read a ZIP file created by
					// mtkit_file_save_zip ()
	char	const	* filename,
	mtZipLoadFunc	callback,	// Called for each chunk in ZIP file,
					// return MTKIT_ZIP_STOP
					// to stop or 0 to request a new chunk
	void		* user_data	// Passed to callback
	);
	// 0 = success else MTKIT_ZIP_ERROR*



namespace mtKit
{

class CliItem;
class CliTab;
class Prefs;
class RecentFile;



typedef struct CharInt		CharInt;

typedef int (* CliFunc) (
	char const * const * args	// NULL terminated argument list
	);



struct CharInt
{
	char	const *	name;
	int		num;
};



class CliItem
{
public:
	CliItem ();
	~CliItem ();

	int add_item ( CliItem * item );
		// 0 = Added, 1 = Error, 2 = Already exists

	int set_data (
		char const * key,	// NULL = Don't set
		CliFunc func,		// NULL = Don't set
		int arg_min,
		int arg_max,
		char const * arg_help,	// NULL = Don't set
		int arg_scale
		);

	CliItem * find_item ( char const * command ) const;
	CliItem const * match_args (
		char ** argv,
		int * cli_error,
		int * ncargs
		) const;
	int callback ( char ** argv ) const;
	int print_help () const;
	int print_help_item () const;

private:

	char		* m_key;
	CliFunc		m_func;
	int		m_arg_min;
	int		m_arg_max;
	char		* m_arg_help;
	int		m_arg_scale;

	// Key = (char const *)(m_key), data = (CliTabItem *)
	mtTree		* m_tree;
};



class CliTab
{
public:
	CliTab ();
	~CliTab ();

	int add_item (
		char const * command,
		CliFunc func,
		int arg_min = 0,
		int arg_max = 0,
		char const * arg_help = NULL,
		int arg_scale = 1	// 2 = pairs, 3 = triplets
		);

	int parse ( char const * cline ) const;
	int print_help ( char const * const * argv ) const;

private:
	CliItem		m_root;
};



class Prefs
{
public:
	Prefs ();
	~Prefs ();

	int initWindowPrefs ();		// Initialize default prefs for
					// preferences window
					// Call this before load ()

	int load (
		char const * filename,	// If NULL use bin_name
		char const * bin_name	// ~/.config/bin_name/prefs.txt
		);
	int save ();

	mtPrefs * getPrefsMem ();

	int addTable ( mtPrefTable const * table );

	int getInt ( char const * key );
	double getDouble ( char const * key );
	char const * getString ( char const * key );

	void set ( char const * key, int value );
	void set ( char const * key, double value );
	void set ( char const * key, char const * value );

private:
	mtPrefs		* prefsMem;
	char		* prefsFilename;
};



class RecentFile
{
public:
	RecentFile ( char const * prefix, int tot = 20 );
	~RecentFile ();

	int init_prefs ( mtKit::Prefs * pr );

	char const * get_filename ( int idx );
	void set_filename ( char const * name );

private:
	void set_filename_idx ( int idx, char const * name );
	char * create_key ( int idx );

/// ----------------------------------------------------------------------------

	int	const	total;
	char	* const	prefs_prefix;
	mtKit::Prefs	* prefs;
};



int prefsInitWindowPrefs (	// Initialize default prefs for
	mtPrefs * prefs		// preferences window
	);			// Call this before load ()

int prefsWindowMirrorPrefs (
	mtPrefs		* dest,
	mtPrefs		* src
	);

int snip_filename (
	char	const	* txt,
	char		* buf,
	size_t		buflen,
	int		lim_tot
	);

int cli_parse_int (
	char	const *	input,
	int		&output,
	int		min,		// If max < min don't check bounds
	int		max
	);

int cli_parse_charint (
	char		const *	input,
	CharInt	const * const	chint,	// NULL terminated table
	int			&result
	);
	// 0 = Found
	// 1 = Not found



}		// namespace mtKit



#endif		// MTKIT_H_

