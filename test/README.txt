-----
USAGE
-----

./configure
./configure debug

make		Run test suite
make valg	Run test suite with Valgrind to expose runtime errors/leaks.
make time	Run test suite with GNU time to measure resource consumption.
make clean	Clear all temp files

./configure flush


-------
GENERAL
-------
These tests must be run on a GNU/Linux system which has had the whole of the mtCellEdit suite installed.

Each subdirectory does its own battery of tests to test each part of the mtCellEdit suite.  The target is not to use more than 200MB of RAM (per running program) or 2GB of temporary disk space (in total).  Using more resource than this is a warning sign that something has gone wrong and the tests are running outside of their pre-determined course.


--------
mtcedcli
--------
After running the tests, various files will appear in 'tmp', 'output' and possibly ./valg_log.txt.  The 'output' directory is automatically checked against 'results' and should be identical.  'tmp' contains PDF files and other output formats that need to be manually checked.  The valg_log.txt file contains all output messages relating to Valgrind.

