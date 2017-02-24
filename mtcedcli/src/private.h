/*
	Copyright (C) 2012-2016 Mark Tyler

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
	#include <math.h>
	#include <unistd.h>
	#include <time.h>
	#include <errno.h>
	#include <ctype.h>
}

#include <mtkit.h>
#include <mtcelledit.h>
#include <mtcedui.h>



#define	MAIN_ROW_PAD		"main_row_pad"
#define	MAIN_FONT_NAME		"main_font_name"
#define	MAIN_FONT_SIZE		"main_font_size"



#define CEDCLI_FILE_MIN		0
#define CEDCLI_FILE_MAX		4
#define CEDCLI_FILE		( state->file_list[ state->booknum ] )
#define CEDCLI_USER_QUIT	-1

#define ABOUT_PRINTF \
"%s\n" \
"Copyright (C) 2012-2016 Mark Tyler\n" \
"Type 'help' for command hints.  Read the manual for more specific info.\n" \
"\n" \
, VERSION



typedef struct mtCliItem	mtCliItem;
typedef struct mtCliTab		mtCliTab;
typedef struct CedCli_STATE	CedCli_STATE;



typedef int (* jtFunc) (
	CedCli_STATE	* state,
	char		** args		// NULL terminated
					// Points to next unparsed item
	);
	// 0 = success



struct CedCli_STATE
{
	int		exit,		// Set to nonzero to exit program
					// -1 = return 0, else return (exit)
			booknum		// Current active book
			;

	CuiFile		* file_list[CEDCLI_FILE_MAX + 1];
	CuiClip		* clipboard;

	mtPrefs		* prefs;
	mtCliTab	* clitab;

};

struct mtCliItem
{
	char	const	* command;
	jtFunc		func;
	int		func_args;	// -1 = 0 or more,
					// -2 = 1 or more, else as given
	char	const	* help_args;
};



// Functions return: 0 = success, NULL = fail; unless otherwise stated.

int cedcli_jumptable_init (
	CedCli_STATE	* state
	);

int cedcli_jumptable_free (
	CedCli_STATE	* state
	);

int cedcli_parse (
	char	const	* input,
	CedCli_STATE	* state
	);

int jtf_clip_flip_h ( CedCli_STATE * state, char ** args );
int jtf_clip_flip_v ( CedCli_STATE * state, char ** args );
int jtf_clip_rotate_a ( CedCli_STATE * state, char ** args );
int jtf_clip_rotate_c ( CedCli_STATE * state, char ** args );
int jtf_clip_transpose ( CedCli_STATE * state, char ** args );
int jtf_clip_load ( CedCli_STATE * state, char ** args );
int jtf_clip_save ( CedCli_STATE * state, char ** args );
int jtf_copy ( CedCli_STATE * state, char ** args );
int jtf_copy_output ( CedCli_STATE * state, char ** args );
int jtf_copy_values ( CedCli_STATE * state, char ** args );
int jtf_cut ( CedCli_STATE * state, char ** args );
int jtf_clear ( CedCli_STATE * state, char ** args );
int jtf_clear_content ( CedCli_STATE * state, char ** args );
int jtf_clear_prefs ( CedCli_STATE * state, char ** args );
int jtf_paste ( CedCli_STATE * state, char ** args );
int jtf_paste_content ( CedCli_STATE * state, char ** args );
int jtf_paste_prefs ( CedCli_STATE * state, char ** args );

int jtf_load ( CedCli_STATE * state, char ** args );
int jtf_save ( CedCli_STATE * state, char ** args );
int jtf_save_as ( CedCli_STATE * state, char ** args );

int jtf_import_book ( CedCli_STATE * state, char ** args );
int jtf_import_graph ( CedCli_STATE * state, char ** args );

int jtf_export_graph ( CedCli_STATE * state, char ** args );
int jtf_export_output_graph ( CedCli_STATE * state, char ** args );
int jtf_export_output_sheet ( CedCli_STATE * state, char ** args );
int jtf_export_sheet ( CedCli_STATE * state, char ** args );

int jtf_about ( CedCli_STATE * state, char ** args );
int jtf_help ( CedCli_STATE * state, char ** args );
int jtf_info ( CedCli_STATE * state, char ** args );
int jtf_quit ( CedCli_STATE * state, char ** args );

int jtf_list_files ( CedCli_STATE * state, char ** args );
int jtf_list_graphs ( CedCli_STATE * state, char ** args );
int jtf_list_sheets ( CedCli_STATE * state, char ** args );

int jtf_print_cell_num ( CedCli_STATE * state, char ** args );
int jtf_print_cell_text ( CedCli_STATE * state, char ** args );
int jtf_print_cell_type ( CedCli_STATE * state, char ** args );
int jtf_print ( CedCli_STATE * state, char ** args );

int jtf_print_prefs_cell ( CedCli_STATE * state, char ** args );

int jtf_delete_column ( CedCli_STATE * state, char ** args );
int jtf_delete_row ( CedCli_STATE * state, char ** args );
int jtf_find ( CedCli_STATE * state, char ** args );
int jtf_set_book ( CedCli_STATE * state, char ** args );
int jtf_set_cell ( CedCli_STATE * state, char ** args );
int jtf_select ( CedCli_STATE * state, char ** args );
int jtf_set_graph ( CedCli_STATE * state, char ** args );
int jtf_set_sheet ( CedCli_STATE * state, char ** args );
int jtf_set_width ( CedCli_STATE * state, char ** args );
int jtf_undo ( CedCli_STATE * state, char ** args );
int jtf_redo ( CedCli_STATE * state, char ** args );
int jtf_recalc_book ( CedCli_STATE * state, char ** args );
int jtf_recalc_sheet ( CedCli_STATE * state, char ** args );

int undo_report_updates (
	int		error
	);

void update_changes_chores (		// Update volatile cells
	CedCli_STATE	* state,	// after a change
	CedSheet	* sheet
	);

CedSheet * cedcli_get_sheet (		// Get active sheet + report error
	CedCli_STATE	* state
	);

CedBookFile * cedcli_get_graph (	// Get active graph + report error
	CedCli_STATE	* state
	);

int jtf_delete_graph ( CedCli_STATE * state, char ** args );
int jtf_delete_sheet ( CedCli_STATE * state, char ** args );
int jtf_duplicate_sheet ( CedCli_STATE * state, char ** args );
int jtf_insert_column ( CedCli_STATE * state, char ** args );
int jtf_insert_row ( CedCli_STATE * state, char ** args );
int jtf_new_book ( CedCli_STATE * state, char ** args );
int jtf_new_sheet ( CedCli_STATE * state, char ** args );
int jtf_rename_graph ( CedCli_STATE * state, char ** args );
int jtf_rename_sheet ( CedCli_STATE * state, char ** args );
int jtf_set_2dyear ( CedCli_STATE * state, char ** args );

int jtf_sort_column ( CedCli_STATE * state, char ** args );
int jtf_sort_row ( CedCli_STATE * state, char ** args );

int jtf_set_prefs_book ( CedCli_STATE * state, char ** args );
int jtf_set_prefs_cell ( CedCli_STATE * state, char ** args );
int jtf_set_prefs_cellborder ( CedCli_STATE * state, char ** args );
int jtf_set_prefs_sheet ( CedCli_STATE * state, char ** args );
int jtf_set_prefs_state ( CedCli_STATE * state, char ** args );

int jtf_print_prefs_book ( CedCli_STATE * state, char ** args );
int jtf_print_prefs_sheet ( CedCli_STATE * state, char ** args );
int jtf_print_prefs_state ( CedCli_STATE * state, char ** args );

int cedcli_prefs_init (
	CedCli_STATE	* state
	);

int cedcli_prefs_free (
	CedCli_STATE	* state
	);


/*
NOTE: mtkit_cli_new () assumes the caller knows what he is doing.  For example
these two entries might be logically impossible to differentiate:

	{ "save",		jtf_save,		1, "<OS FILENAME>" },
	{ "save as",		jtf_save_as },

The input "save as"  could be interpreted as either, so results are undefined.
Beware!
*/

mtCliTab * mtkit_cli_new (		// Create a new CLI table
	const mtCliItem	* list		// NULL terminated static array
	);

int mtkit_cli_free (			// Destroy a CLI table
	mtCliTab	* table
	);

const mtCliItem * mtkit_cli_match (	// Try to match a command line + args
	mtCliTab	* table,
	char		** argv,	// NULL terminated
	int		* cli_error,	// NULL = don't pass error, else return
					//  0 = correct match with correct
					// number of args
					// -1 = too few args
					// -2 = too many args
					// -3 = other error
					// 1,2,3,etc = unmatched command
	int		* ncargs	// Number of command arguments found if
					// matched
	);
	// NULL = No command match

int mtkit_cli_help (			// Output command list and associated
					// help
	mtCliTab	* table,
	char		** argv		// NULL terminated
	);
	// 0 = success
	// 1 = unmatched command at this point

mtCliTab * mtkit_clitab_new (		// Create empty mtCliTab
	void
	);

