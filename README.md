# multi-threaded-matrix-mutliplication-C

The command line utility written in ANSI C computes multiplication of two matrices using multi-threading. A maximum number of threads is set, such that the program will at any time use no more that the maximum number of threads. The program uses [pthreads](http://man7.org/linux/man-pages/man7/pthreads.7.html). The matrices are filled with random integers. 

To run the executable, first compile with the Makefile and paste this into Terminal:
`./main x y z`
where `x` represents the rows of matrix A, `y` represents the columns of A and `z` represents the columns of matrix B. Make sure the numbers are legal integers.

The format of printed matrices was specifically designed to enable usage of Wolfram Alpha in order to check the result of multiplication. The final result can be checked via [Wolfram Alpha](http://www.wolframalpha.com) by copying and pasting the printed matrices from Terminal as shown in the screenshot (in the repo).

## Main Challenge: ##

The main challenge was to use pthread condition variable in order to control max number of active threads.
