/*	mtGenTex example data.

	Unrecognised names do not output anything (see OTHER below)
	Element names & attribute names actioned:

	int			Output random integer
		min	int	default = -1,000
		max	int	default = 1,000

	double			Output random floating point number
		min	double	default = -1,000
		max	double	default = 1,000

	char			Output random character(s)
		range	string	default = ASCII printable
		tot	int
		totmin	int
		totmax	int

	newline			Output newline(s)
		tot	int
		totmin	int
		totmax	int

	---

	OTHER			Any other elements apart from the above
		tot	int
		totmin	int
		totmax	int
*/

{ mtSoakTest

	/* Repeat everything below 'tot' times*/
	tot="4000"

	{ set_cells

		tot="10"

		/* Select a random cell somewhere */
		"select "	"r" { int min="1" max="1000" }
				"c" { int min="1" max="1000" }
		{ newline }
		/* Fill the cell with 25 random characters */
		"set cell "
		{ char
			range="abcdefghijklmnopqrstuvwxyz"
			totmin="1"
			totmax="25"
		}
		{ newline }
	}

	{ set_cells

		tot="10"
		/* Select a random cell somewhere */
		"select "	"r" { int min="1" max="1000" }
				"c" { int min="1" max="1000" }
		{ newline }
		/* Random double in this range - 1,000,000 subpoints */
		"set cell "
		{ double min="-1.1" max="2.1" }
		{ newline }
	}

	{ insert_rows

		tot="3"

		/* Select a random cell somewhere */
		"select "	"r" { int min="1" max="1000" }
				"c" { int min="1" max="1000" }
		{ newline }

		"insert row"
		{ newline }
	}

	{ insert_cols

		tot="3"

		/* Select a random cell somewhere */
		"select "	"r" { int min="1" max="1000" }
				"c" { int min="1" max="1000" }
		{ newline }

		"insert column"
		{ newline }
	}

	{ delete_rows

		tot="3"

		/* Select a random cell somewhere */
		"select "	"r" { int min="1" max="1000" }
				"c" { int min="1" max="1000" }
		{ newline }

		"delete row"
		{ newline }
	}

	{ delete_cols

		tot="3"

		/* Select a random cell somewhere */
		"select "	"r" { int min="1" max="1000" }
				"c" { int min="1" max="1000" }
		{ newline }

		"delete column"
		{ newline }
	}
}

{ undo_redo_save
	"export sheet tmp/soak_01.tsv tsv"
	{ newline }

	{ undo tot="105" "undo" { newline } }

	"export sheet tmp/soak_02.tsv tsv"
	{ newline }

	{ undo tot="105" "redo" { newline } }

	"export sheet tmp/soak_03.tsv tsv"
	{ newline }

	{ undo tot="105" "undo" { newline } }

	"export sheet tmp/soak_04.tsv tsv"
	{ newline }

	{ undo tot="105" "redo" { newline } }

	"export sheet tmp/soak_05.tsv tsv"
	{ newline }
}

{ reload
	"new"
	{ newline }

	"load tmp/soak_01.tsv tsv"
	{ newline }
}





/*
	NOTHING MORE TO DO!!
*/

{ quit
	"q"
	{ newline }
}

