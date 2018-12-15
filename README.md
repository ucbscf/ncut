ncut
====

ncut − print specified lines, columns and/or fields of a file

SYNOPSIS
--------

**ncut** \[ **−s***c* **-t***c* **-p -l***list* **-i***num* **-c***list*
**-f***list* ... \] files

DESCRIPTION
-----------

*ncut* will print selected columns and/or fields from selected lines in
a file or list of files. (Fields as defined here are separated by any
number of blanks or tabs. See the -t option below.) The selections are
specified by giving an optional list of lines (**−l**), followed by
(optional) lists of column (**−c**) or field numbers (**−f**). The
requested text will be printed to standard output in the order the
requests are made, with a **−l** request signalling a new group of
outputted lines. If no **−l** request is given, the columns and/or field
requests apply to all lines in the files. Likewise, if no column
(**−c**) or field (**−f**) requests are given, the requested lines will
be printed in their entirety. (See examples below) A *list* may be given
in three forms:

List      | Description
--------- | -----------
n1        | refers to line, column or field n1
n1,n2,... | refers to a list of n1, n2, etc.
n1-n2     | refers to a list from n1 through n2 inclusive. If n2 is omitted, the list goes from n1 to the end.


These lists can be combined; for example 4,5-9,12 means 4, 5 through 9
and 12.

OPTIONS
-------
 Option             | Description
------------------- | -----------
 **−i***num*        | specifies an increment for the preceding range of lines (**−l** option). For example, if *num* is 3, then the field and/or column requests for every third line, starting with the first in the **−l** list will be output. This option has no effect if it is not preceded by a **−l** list.
 **−s***c*          | By default, requested fields and columns are separated by a blank on output. (A ’’-’’ list of columns is treated as a single group, and only the group is separated from other items by the blank.) The output separator character can be changed to the character *c* by using the -s*c* option; it takes effect for the next column or field request, and continues until another -s*c* option is seen; to eliminate the output separator, use -s (with no trailing character).
 **−t***c*          | Redefine the input field separator to be *c*. When given alone, the **−t** option sets the input field separator to be a tab.
 **−T**             | When an input field separator is given with the -t option, by default **ncut** will not print the field separator along with the selected text. The -T option tells **ncut** to include the field separator in the output.
 **−p**             | When given with the **−t** option, pass through lines which do not contain the input field separator to standard output unchanged. (Note: to reproduce the behavior of the UNIX command cut, use ncut -p -t )   |
 **−b***num*        | specifies a buffer size to hold the lines processed by **ncut**. By default, **ncut** starts with a buffer of 2048 bytes, and reallocates memory for longer lines through a doubling strategy. If you know the maximum size of the lines to be processed by **ncut**, it may be more efficient to specify this size through this option than to let the program deal with it dynamically.       |

EXAMPLES
--------

Print out the first and third fields from the first 10 lines of the file
tfile, followed by columns 12 through the end of lines 6 and 8:

`ncut -l1-10 -f1,3 -l6,8 -c12- tfile`

Print out columns 5 through 12 of the output from prog:

`prog | ncut -c5-12`

Note that there will be no separator placed between the individual
columns.

Print out columns 5 through 12 and 18 and 20 of file foo:

`ncut -c5-12 -c18,20 foo`

Note that there will be spaces between columns 18 and 20. These could be
supressed by placing "-s" before "-c18,20"

Print out lines 10,15 and 23 from file bar:

`ncut -l10,15,23 bar`

Split 150 column lines from the file foo into 3 lines, each 50 columns
long. Notice that to specify a newline it is necessary to use a
backslash as an escape character:

```
ncut -s'\
' -c1-50,51-100,101-150 foo
```

AUTHOR
------

Phil Spector <spector@edithst.com>
