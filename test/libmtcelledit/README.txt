thrd_*

---------
Threading
---------

By design mtCellEdit aims to be "small, light, and reliable" (see the philosophy section in the handbook).

Therefore I have deliberately avoided anything that threatens these qualities (e.g. optimization, which increases code size and complexity, increases the probability of bugs, and therefore reduces reliability).

One way of optimizing a program is to use threads, which can dramatically speed up operations, at the cost of code size and simplicity.

Although multi-threading is not officially supported by the library for the aforementioned philosophical reasons, with care it can be achieved in certain situations for both reading and writing sheets.  This document outlines how this works, and the code in this directory implements these ideas.

-------
READING
-------

All reads from a sheet are thread safe as long as the sheet does not get modified for the duration of the threads.

See the program "tcread" for an example of how this can work.

-------
WRITING
-------

Under very specific conditions sheets can be written safely by different threads.  For example if a row already exists (and no others are added or removed for the thread duration), and the row is only changed by a single thread for the entire duration of the thread, then this is safe.

See the program "tcwrite" for an example of how this can work.

-------
RESULTS
-------

Here are some sample results from my 8 thread CPU:

Read
------------
Threads	Time
------------
Single	0.026
1	0.029
2	0.018
3	0.014
4	0.013
5	0.012
6	0.011
7	0.010
8	0.014
9	0.014
10	0.012

Write
------------
Threads	Time
------------
Single	0.94
1	0.96
2	0.55
3	0.45
4	0.41
5	0.40
6	0.40
7	0.39
8	0.38
9	0.38
10	0.38


