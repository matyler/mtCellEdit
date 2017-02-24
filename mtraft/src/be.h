/*
	Copyright (C) 2004-2016 Mark Tyler

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

extern "C" {

	#include <stdlib.h>
	#include <string.h>
	#include <dirent.h>
	#include <sys/stat.h>
	#include <sys/types.h>
	#include <unistd.h>
}

#include <mtkit.h>
#include <mtcelledit.h>



class busyState;



enum	// Column numbers for each field in results sheet
{
	RAFT_COL_NAME		= 1,
	RAFT_COL_FILES		= 2,
	RAFT_COL_FILES_PERCENT	= 3,
	RAFT_COL_BYTES		= 4,
	RAFT_COL_MB		= 5,
	RAFT_COL_MB_PERCENT	= 6,
	RAFT_COL_SUBDIRS	= 7,
	RAFT_COL_OTHER		= 8,

	RAFT_COL_TOTAL		= 9
};



#define PREFS_WINDOW_X		"window_x"
#define PREFS_WINDOW_Y		"window_y"
#define PREFS_WINDOW_W		"window_w"
#define PREFS_WINDOW_H		"window_h"



typedef int (* raftScanFunc) (
	CedSheet	* sheet,
	int		row,		// Row calculated in sheet
	void		* user_data
	);
	// 0 = Keep scanning
	// 1 = Stop



// Functions return: 0 = success, NULL = fail; unless otherwise stated.

int raft_scan_sheet (
	char	const	* path,		// Path to scan all files & subdirs
	CedSheet	** sheet,	// Put results here
	raftScanFunc	callback,	// Option callback for each subdir
	void		* user_data	// Passed to raftScanFunc
	);
	// -1 = Error
	//  0 = Success - sheet created
	//  1 = User termination

char const * raft_cline (		// Parse command line
	int			argc,
	char	const * const *	argv
	);
	// Returns pointer to path, or NULL if none was passed

char * raft_path_check (		// Check path, put into new string
	char	const	* path
	);
	// Caller must free any result after use.

char * raft_path_merge (		// Merge the current path and selected row
	char	const	* path,		// Must have a / at the end
	CedSheet	* sheet,
	int		row
	);
	// Caller must free any result after use.

char * raft_get_clipboard (		// Create UTF8 text for clipboard
	CedSheet	* sheet		// from this sheet
	);
	// Caller must free any result after use.


class busyState
{
public:
	enum Status
	{
		IDLE,
		WORKING,
		STOPPED
	};


	busyState	();
	~busyState	();

	void		setWorking ();
	void		setStopped ();
	void		setIdle ();
	Status		getStatus () const;

private:
	Status		status;
};

