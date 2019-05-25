#!/usr/bin/env python3

#	Copyright (C) 2017-2018 Mark Tyler
#
#	This program is free software; you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation; either version 3 of the License, or
#	(at your option) any later version.
#
#	This program is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.
#
#	You should have received a copy of the GNU General Public License
#	along with this program in the file COPYING.

import argparse
import os
import stat
import sys
import time



__version__ = "@@VERSION@@"



class Totalizer:
	def __init__ ( self, summarize, path ):
		self.files = 0
		self.bytes = 0
		self.subdirs = 0
		self.other = 0
		self.summarize = summarize
		self.path_len = 1 + len(path)



def calc_mkg ( dbyt ):
	if ( dbyt < (1024*1024) ):
		return "KB", dbyt / 1024
	elif ( dbyt < 1024*1024*1024 ):
		return "MB", dbyt / (1024*1024)

	return "GB", dbyt / (1024*1024*1024)


def clean_print_string ( string ):
	# Remove invalid UTF-8 chars from the filename
	return string.encode("utf-8", "ignore").decode("utf-8")


def print_farewell ( path, tot ):
	unit, val = calc_mkg ( tot.bytes )
	print ( "\n\n%s contains:" % clean_print_string ( path ) )

	print ( "\n  Subdirs         Links        Files        Bytes      %s" %
		unit )

	print ( "---------------------------------------------------------");

	print ( "%9d     %9d    %9d %12d %7.2f\n" % (tot.subdirs, tot.other,
		tot.files, tot.bytes, val ) )


def print_file_info ( buf, filename ):
	unit, val = calc_mkg ( buf.st_size )

	txt = str ( "%13.0f  " % (buf.st_size) )
	txt += str ( "%7.2f %s  " % (val, unit) )
	txt += time.strftime( "%d-%m-%Y  %H.%M  ", time.gmtime ( buf.st_mtime ))
	txt += filename

	print ( clean_print_string ( txt ) )


def print_titles ( tot ):
	print ( "" )

	if not tot.summarize:
		txt = "        Bytes    KB-MB-GB  Mdate       Mtime  Path"

		print ( txt )
		print ( "-" * (4 + len(txt)) )


def path_recurse ( path, tot ):
	try:
		for name in os.listdir ( path ):
			fullname = os.path.join ( path, name )
			st = os.lstat ( fullname )
			mode = st.st_mode

			if stat.S_ISDIR ( mode ):
				path_recurse ( fullname, tot )
				tot.subdirs += 1

			elif stat.S_ISLNK ( mode ) or not stat.S_ISREG ( mode ):
				tot.other += 1

			else:
				tot.files += 1
				tot.bytes += st.st_size

				if not tot.summarize:
					print_file_info ( st, fullname[tot.path_len:] )
	except IOError:
		return

	except PermissionError:
		return


def scan_path ( path, summarize ):
	if not os.path.isdir ( path ):
		print ( path, "is not a directory." )
		return

	path = os.path.abspath ( path )

	tot = Totalizer ( summarize, path )

	print_titles ( tot )
	path_recurse ( path, tot )
	print_farewell ( path, tot )


def main ():
	parser = argparse.ArgumentParser ( description =
		"%(prog)s scans the path given, searching for files and "
		"travelling through subdirectories.  Once it has found all "
		"files, it prints a summary of what it finds." )

	parser.add_argument ( '--version', action = 'version',
		version = '%(prog)s ' + str ( __version__ ) )

	parser.add_argument ( 'strings', metavar = 'P', type = str, nargs = '*',
		help = 'path to search' )

	parser.add_argument ( '-s', action = 'store_true',
		help = 'only print a summary' )

	args = parser.parse_args()

	for path in args.strings:
		scan_path ( path, args.s )


if __name__ == "__main__":
	main ();

