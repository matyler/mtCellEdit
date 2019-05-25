/*
	Copyright (C) 2008-2019 Mark Tyler

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



static char * load_gzip (
	char		const *	const	filename,
	FILE		*	const	fp,
	int		*	const	bytes,
	int				zero
	)
{
	unsigned char	txt[16];
	char		* buf;
	unsigned int	newsize;


	if ( fseek ( fp, -4, SEEK_END ) )
	{
		return NULL;
	}

	if ( fread ( txt, 1, 4, fp ) != 4 )
	{
		return NULL;
	}

	newsize =	( ((unsigned int)txt[3]) << 24 ) +
			( ((unsigned int)txt[2]) << 16 ) +
			( ((unsigned int)txt[1]) << 8 ) +
			txt[0];

	if (	newsize > MTKIT_FILESIZE_MAX ||
		newsize > INT_MAX
		)
	{
		return NULL;
	}

	if ( newsize < 1 )
	{
		// No bytes to read so just have a 1 byte buffer

		newsize = 0;
		zero = 1;
	}

	buf = (char *)calloc ( newsize + (size_t)zero, 1 );
	if ( ! buf )
	{
		return NULL;
	}

	if ( newsize > 0 )
	{
		int		i;
		gzFile		gzf;


		gzf = gzopen ( filename, "rb" );
		if ( ! gzf )
		{
			goto fail;
		}

		i = gzread ( gzf, (voidp)buf, newsize );
		gzclose ( gzf );

		if ( i != (int)newsize )
		{
			goto fail;
		}
	}

	bytes[0] = (int)newsize;

	return buf;			// Success

fail:
	free ( buf );

	return NULL;
}

int64_t mtkit_file_size (
	char	const *	const	filename
	)
{
	struct stat buf;


	if ( ! filename )
	{
		return -1;
	}

	if ( stat ( filename, &buf ) )
	{
		return -2;
	}

/*
	NOTE: int64_t is used as mtkit is compiled with -D_FILE_OFFSET_BITS=64.
	Linking app may only be compiled with 32 bits so using off_t isn't safe.
*/

	return buf.st_size;
}

char * mtkit_file_load (
	char	const *	const	filename,
	int	*	const	bytes,
	int		const	in_flag,
	int	*	const	out_flag
	)
{
	FILE		* fp;
	char		* buf;
	unsigned char	txt[16];
	int64_t		fsize;
	int		outi = 0, zero = 0, gzip = 0;


	if ( ! bytes )
	{
		return NULL;
	}

	fsize = mtkit_file_size ( filename );

	if ( fsize < 0 || fsize > MTKIT_FILESIZE_MAX )
	{
		return NULL;
	}

	if ( in_flag & MTKIT_FILE_ZERO )
	{
		zero = 1;		// Extra 0 byte at end
	}

	fp = fopen ( filename, "rb" );

	if ( ! fp )
	{
		return NULL;
	}

	if ( in_flag & MTKIT_FILE_GUNZIP )
	{
		size_t		i = fread ( txt, 1, 2, fp );


		if (	i == 2		&&
			txt[0] == 31	&&
			txt[1] == 139
			)
		{
			gzip = 1;	// GZIP header found
		}

		rewind ( fp );
	}

	bytes[0] = (int)fsize;

	if ( fsize == 0 )
	{
		// No bytes to read so just have a 1 byte buffer

		zero = 1;
	}

	if ( gzip && fsize > 20 )
	{
		buf = load_gzip ( filename, fp, bytes, zero );

		if ( buf )
		{
			// GZIP file successfully unpacked
			outi |= MTKIT_FILE_GUNZIP;

			goto finish;
		}

		// Failure to successfully open a gzip file leads to us loading
		// the raw file anyway.
	}

	buf = (char *)malloc ( (size_t)(fsize + zero) );

	if ( buf && fsize > 0 )
	{
		size_t		i = fread ( buf, 1, (size_t)fsize, fp );


		if ( i != (size_t)fsize )
		{
			free ( buf );
			buf = NULL;
		}
	}

	if ( buf && zero )
	{
		buf[ fsize ] = 0;
	}

finish:
	fclose ( fp );

	if ( out_flag )
	{
		out_flag[0] = outi;
	}

	return buf;
}

int mtkit_file_save (
	char	const *	const	filename,
	char	const *	const	buf,
	int		const	buf_size,
	int		const	out_flag
	)
{
	int		res = 0;


	if (	! filename			||
		! buf				||
		buf_size < 0			||
		buf_size > MTKIT_FILESIZE_MAX
		)
	{
		return 1;
	}

	if ( out_flag & MTKIT_FILE_GUNZIP )
	{
		gzFile		fp;


		fp = gzopen ( filename, "wb" );

		if ( ! fp )
		{
			return 1;
		}

		if (	buf_size > 0 &&
			gzwrite ( fp, buf, (unsigned int)buf_size ) != buf_size
			)
		{
			res = 1;
		}

		gzclose ( fp );
	}
	else
	{
		FILE		* fp;


		fp = fopen ( filename, "wb" );
		if ( ! fp )
		{
			return 1;
		}

		if (	buf_size > 0 &&
			fwrite ( buf, 1, (size_t)buf_size, fp ) !=
				(size_t)buf_size
			)
		{
			res = 1;
		}

		fclose ( fp );
	}

	return res;
}

char const * mtkit_file_home ( void )
{
	static char const * homedir;
	struct passwd	* p;


	if ( homedir )
	{
		return homedir;
	}

	homedir = getenv ( "HOME" );

	if ( ! homedir )
	{
		p = getpwuid ( getuid () );

		if ( p )
		{
			homedir = p->pw_dir;
		}
	}

	if ( ! homedir )
	{
		fprintf ( stderr, "Could not find home directory. Using "
			"current directory.");
		homedir = ".";
	}

	return homedir;
}

int mtkit_file_directory_exists (
	char	const *	const	path
	)
{
	if ( ! path )
	{
		return 0;
	}

	DIR * dir = opendir ( path );

	if ( dir )
	{
		closedir ( dir );

		return 1;
	}

	return 0;
}

int mtkit_file_readable (
	char	const *	const	filename
	)
{
	char path[ PATH_MAX ];

	// Don't count symlinks as a file, they could be for directories.
	if ( ! filename || ! realpath ( filename, path ) )
	{
		return 0;	// NOT a file
	}

	if (	0 == access ( path, F_OK )	&&
		0 == access ( path, R_OK )
		)
	{
		struct stat buf;
		if (	lstat ( path, &buf )	||
			S_ISDIR ( buf.st_mode )
			)
		{
			return 0;	// NOT a file
		}

		return 1;	// Exists, is file, and readable
	}

	return 0;
}

int mtkit_file_writable (
	char	const *	const	filename
	)
{
	if (	filename			&&
		( 0 != access ( filename, F_OK ) ||
		  0 == access ( filename, W_OK ) )
		)
	{
		return 1;
	}

	return 0;
}



#define BUFSIZE 100



char * mtkit_file_readline (
	FILE		* const	fp,
	int		* const	len,
	int		* const	len_nl
	)
{
	char		* line, * cp;
	int		bufsize, p, i, nl;


	if ( ! fp )
	{
		return NULL;
	}

	line = (char *)malloc ( BUFSIZE + 1 );
	if ( ! line )
	{
		return NULL;
	}

	bufsize = BUFSIZE;
	p = 0;
	nl = 0;

	// Put data from the file into the memory buffer
	do
	{
		i = fgetc ( fp );

		if ( i == EOF )
		{
			break;
		}

		if ( i < 0 )
		{
			goto error;
		}

		line[p] = (char)i;
		p++;

		if ( p >= bufsize )
		{
			bufsize += BUFSIZE;
			cp = (char *)realloc ( line, (size_t)bufsize );

			if ( ! cp )
			{
				goto error;
			}

			line = cp;
		}

		if ( i == '\n' )
		{
			p--;
			nl = 1;

			break;
		}

		if ( i == '\r' )
		{
			p--;
			nl = 1;

			// Check for DOS newline \r\n
			i = fgetc ( fp );

			if ( i == EOF )
			{
				break;
			}

			if ( i < 0 )
			{
				goto error;
			}

			// This isn't a DOS newline so give this byte back
			// (Mac newline)

			if ( i != '\n' )
			{
				ungetc ( i, fp );
			}
			else
			{
				nl ++;
			}

			break;
		}
	}
	while ( 1 );

	if ( (p + nl) < 1 )
	{
		goto error;		// No input
	}

	line[p] = 0;

	if ( len )
	{
		len[0] = p;
	}

	if ( len_nl )
	{
		len_nl[0] = nl;
	}

	if ( p < bufsize )		// Shrink wrap final string
	{
		cp = (char *)realloc ( line, (size_t)(p + 1) );
		if ( cp )
		{
			line = cp;
		}
	}

	return line;

error:
	free ( line );

	return NULL;
}

int mtkit_file_load_stdin (
	char	**	const	buf,
	size_t		* const	buflen
	)
{
	char		* dest;
	size_t	const	bufsize = 1000000;


	if ( ! buf || ! buflen )
	{
		return 1;
	}

	buf[0] = (char *)malloc ( bufsize );
	if ( ! buf[0] )
	{
		return 1;
	}

	buflen[0] = 0;

	while ( 1 )
	{
		dest = buf[0] + buflen[0];
		size_t new_len = fread ( dest, 1, bufsize, stdin );
		buflen[0] += new_len;

		if ( new_len < bufsize )
		{
			// Add NUL terminator
			dest [ new_len ] = 0;
			new_len ++;
			buflen[0] ++;

			if ( new_len < bufsize )
			{
				// realloc to remove unused memory at end
				dest = (char *)realloc ( buf[0], buflen[0] );
				if ( ! dest )
				{
					free ( buf[0] );
					return 1;
				}

				buf[0] = dest;
			}

			break;
		}

		// More data to come so realloc to create larger buffer.
		dest = (char *)realloc ( buf[0], buflen[0] + bufsize );
		if ( ! dest )
		{
			free ( buf );
			return 1;
		}

		buf[0] = dest;
	}

	return 0;			// buf populated
}



/*
	------- mtFile -------
*/

#define MTKIT_FILE_MEM_BUFLEN		32768



struct mtFile
{
	FILE		* fp;		// If NULL we are using memory file
					// below

// ---------	MEMORY FILE ONLY

	uint8_t		* buf_start,	// Start of memory buffer
			* buf_next	// Next byte (if buf_left > 0)
			;

	int		buf_left;	// Bytes left remaining unused in the
					// buffer
};



mtFile * mtkit_file_open_disk (
	char	const *	const	filename
	)
{
	mtFile * const mtfp = (mtFile *)(calloc ( sizeof ( mtFile ), 1 ));

	if ( ! mtfp )
	{
		return NULL;
	}

	mtfp->fp = fopen ( filename, "wb" );
	if ( ! mtfp->fp )
	{
		free ( mtfp );

		return NULL;
	}

	return mtfp;
}

mtFile * mtkit_file_open_mem ( void )
{
	mtFile * const mtfp = (mtFile *)(calloc ( sizeof ( mtFile ), 1 ) );

	if ( ! mtfp )
	{
		return NULL;
	}

	mtfp->buf_start = (uint8_t *)malloc ( MTKIT_FILE_MEM_BUFLEN );
	if ( ! mtfp->buf_start )
	{
		free ( mtfp );

		return NULL;
	}

	mtfp->buf_next = mtfp->buf_start;
	mtfp->buf_left = MTKIT_FILE_MEM_BUFLEN;

	return mtfp;
}

int mtkit_file_close (
	mtFile		* const	mtfp
	)
{
	if ( ! mtfp )
	{
		return 1;
	}

	if ( mtfp->fp )
	{
		fclose ( mtfp->fp );
	}
	else
	{
		free ( mtfp->buf_start );
	}

	// Dangling pointers left as we destroy structure
	free ( mtfp );

	return 0;			// Success
}

int mtkit_file_write (
	mtFile		*	const	mtfp,
	void		const *	const	mem,
	int64_t			const	mem_len
	)
{
	if (	! mtfp		||
		! mem		||
		mem_len < 0
		)
	{
		return 1;
	}

	if ( mem_len == 0 )
	{
		return 0;
	}

	// NOTE: We do not check for size of memory chunk getting too big

	if ( mtfp->fp )
	{
		if ( fwrite ( mem, 1, (size_t)mem_len, mtfp->fp ) !=
			(size_t)mem_len )
		{
			return 1;
		}
	}
	else
	{
		if ( mem_len > mtfp->buf_left )
		{
			uint8_t		* np;
			size_t		len;
			int64_t		todo;


			len = (size_t)(mtfp->buf_next - mtfp->buf_start);
			todo = mem_len + MTKIT_FILE_MEM_BUFLEN;

			np = (uint8_t *)realloc ( mtfp->buf_start,
				len + (size_t)todo );
			if ( ! np )
			{
				return 1;
			}

			mtfp->buf_start = np;
			mtfp->buf_next = np + len;
			mtfp->buf_left = (int)todo;
		}

		memcpy ( mtfp->buf_next, mem, (size_t)mem_len );

		mtfp->buf_next += mem_len;
		mtfp->buf_left -= (int)mem_len;
	}

	return 0;			// Success
}

int mtkit_file_write_string (
	mtFile		*	const	mtfp,
	char		const *	const	str
	)
{
	if ( ! str )
	{
		return 1;
	}

	return mtkit_file_write ( mtfp, str, (int64_t)strlen ( str ) );
}

int mtkit_file_get_mem (
	mtFile		*	const	mtfp,
	void		**	const	buf,
	int64_t		*	const	buf_len
	)
{
	if ( ! mtfp )
	{
		return 1;
	}

	if ( buf )
	{
		buf[0] = mtfp->buf_start;
	}

	if ( buf_len )
	{
		buf_len[0] = mtfp->buf_next - mtfp->buf_start;
	}

	return 0;
}

int mtkit_file_header_gz (
	unsigned char	const *	const	mem,
	int			const	mem_size
	)
{
	if (	mem		&&
		mem_size > 20	&&
		mem[0] == 31	&&
		mem[1] == 139
		)
	{
		return 1;
	}

	return 0;
}

int mtkit_file_header_zip (
	unsigned char	const *	const	mem,
	int			const	mem_size
	)
{
	if (	mem		&&
		mem_size > 30	&&
		mem[0] == 0x50	&&
		mem[1] == 0x4b	&&
		mem[2] == 0x03	&&
		mem[3] == 0x04
		)
	{
		return 1;
	}

	return 0;
}

static void trim_string (
	char		* const	buf,
	int		const	lim_tot
	)
{
	#define SNIP_LEN	5
	char	const	* snip = " ... ";	// length = SNIP_LEN

	char		* spa, * spb;
	int		res, lim_edge, slen;


	lim_edge = lim_tot / 2 - SNIP_LEN;
	slen = mtkit_utf8_len ( (unsigned char *)buf, 0 );

	if ( slen <= lim_tot )
	{
		// String is already short enough
		return;
	}

	res = mtkit_utf8_offset ( (unsigned char *)buf, lim_edge );

	if ( res < 0 )
	{
		return;
	}

	// Place snip text here later
	spa = buf + res;

	res = mtkit_utf8_offset ( (unsigned char *)spa, slen - 2 * lim_edge );

	if ( res < 0 )
	{
		return;
	}

	// Points to the last lim_edge chars in buf string
	spb = spa + res;

	// Move right hand side of snip: all chars and NUL terminator
	memmove ( spa + SNIP_LEN, spb, strlen ( spb ) + 1 );

	// Insert snip text
	memcpy ( spa, snip, SNIP_LEN );
}

int mtkit_snip_filename (
	char	const	* const	txt,
	char		* const	buf,
	size_t		const	buflen,
	int		const	lim_tot
	)
{
	char		* tmp;


	if ( ! txt || strlen ( txt ) < 2 )
	{
		return 1;		// buf not filled
	}

	tmp = mtkit_utf8_from_cstring ( txt );
	if ( ! tmp )
	{
		return 1;		// buf not filled
	}

	mtkit_strnncpy ( buf, tmp, buflen );

	free ( tmp );

	trim_string ( buf, lim_tot );

	return 0;			// buf filled
}

static int match_extension (
	char	const * const	filename,
	char	const * const	ext
	)
	// -3 = OS error
	// -2 = Arg error
	// -1 = no match, else returns offset in 'string' of first non wildcard
	// character match.
{
	char * txt = mtkit_string_join ( "*.", ext, NULL, NULL );

	if ( ! txt )
	{
		return -3;
	}

	int res = mtkit_strmatch ( filename, txt, 0 );

	free ( txt );

	return res;
}

static char * replace_extension (
	char	const	* filename,
	size_t	const	pos,	// Position of . preceding extension to replace
	char	const	* ext
	)
{
	size_t	const	extlen	= strlen ( ext );
	char	* const	nstr	= (char *)malloc ( pos + extlen + 1 + 1 );

	if ( ! nstr )
	{
		return NULL;
	}

	memcpy ( nstr, filename, pos );
	memcpy ( nstr + pos, ".", 1 );
	memcpy ( nstr + pos + 1, ext, extlen + 1 );	// ext & NUL terminator

	return nstr;
}

char * mtkit_set_filename_extension (
	char		const * const	filename,
	char		const * const	ext_a,
	char		const *	const	ext_b,
	char	const * const * const	ext_multi
	)
{
	if ( ! filename || ! ext_a )
	{
		return NULL;
	}

	{
		char const * const ext[] = { ext_a, ext_b, NULL };
		char const * const * st;

		for ( st = ext; st[0]; st++ )
		{
			int res = match_extension ( filename, st[0] );

			if ( res > -1 )
			{
				// Extension in filename
				return NULL;
			}
			else if ( res < -1 )
			{
				// Error
				return NULL;
			}
		}
	}

	// No valid extension detected so we must continue

	if ( ext_multi )
	{
		// Remove multi-tier extension, e.g. "tsv.zip"

		char const * const * st;

		for ( st = ext_multi; st[0]; st++ )
		{
			int res = match_extension ( filename, st[0] );

			if ( res > -1 )
			{
				// Extension in filename
				return replace_extension ( filename,
					(size_t)res, ext_a );
			}
			else if ( res < -1 )
			{
				// Error
				return NULL;
			}

			// res == -1 (no match) so we must continue
		}
	}

	// Replace current extension (if one exists)
	{
		char const * rchar = strrchr ( filename, '.' );
		char const * rchar_sep = strrchr ( filename, MTKIT_DIR_SEP );

		if ( rchar && rchar > rchar_sep )
		{
			return replace_extension ( filename,
				(size_t)(rchar - filename), ext_a );
		}
	}

	// At this point we know that there is no extension in filename
	return mtkit_string_join ( filename, ".", ext_a, NULL );
}

int mtkit_file_lock (
	char	const * const	filename,
	int		* const	file_id
	)
{
	*file_id = open ( filename, O_CREAT | O_RDWR, 0666 );

	if ( -1 == *file_id )
	{
		return 1;		// Failed to open lock file
	}

	if ( -1 == lockf ( *file_id, F_TLOCK, 0 ) )
	{
		mtkit_file_unlock ( file_id );

		return 1;		// Failed to lock the file
	}

	return 0;			// Success
}

void mtkit_file_unlock (
	int	* const	file_id
	)
{
	if ( -1 != *file_id )
	{
		close ( *file_id );
		*file_id = -1;
	}
}

void mtkit_mkdir ( char const * const path )
{
	if ( path )
	{
		mkdir ( path, S_IRWXU | S_IRWXG | S_IRWXO );
	}
}

