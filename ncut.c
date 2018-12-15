#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <string.h>

#ifdef __gnu_linux__
#    include <values.h>
#endif

#include "lf.h"

#define COLUMN  1
#define FIELD   2
#define LIN     3

static char memmsg[] = "Error: couldn't get memory.\n";
static char valmsg[] =
  "Error: starting value must be less than finish value.\n";
static char nummsg[] = "Error: numbers must be greater than zero.\n";
static char *argv0;

int
main (int argc, char **argv)
{
  int i, j, k, l, nf, type, nfiles, tflag = 0, Tflag = 0,
    pflag, pass, inc, inc_all, strt;
  int bufsize;
  struct LINE *linehead = NULL, *linenow = NULL;
  struct ITEM *lineitem, *colitem;
  int argind[256];
  char valid;
  char *sepnow = " ";
  char fsep = '\t';
  char *buf;
  struct ITEM *getitem ();
  FILE *fp;

  argv0 = argv[0];
  i = inc_all = 1;
  bufsize = 2048;
  nfiles = 0;
  while (i < argc)
    {
      if (argv[i][0] == '-')
	goto doopt;
      argind[nfiles++] = i;
      i++;
      continue;

    doopt:
      valid = 1;
      if (argv[i][1] == 0)	/* signal stdin if - given without arg */
	argind[nfiles++] = -1;
      else if (argv[i][1] == 's')	/* output separator */
	sepnow = argv[i] + 2;
      else if (argv[i][1] == 'b')	/* buffer size */
	bufsize = atoi (argv[i] + 2);
      else if (argv[i][1] == 't')
	{			/* input separator (defaults to tab) */
	  tflag = 1;
	  if (argv[i][2])
	    fsep = argv[i][2];
	}
      else if (argv[i][1] == 'p')	/* pass lines with no input sep to stdout */
	pflag = 1;
      else if (argv[i][1] == 'T')	/* keep tab chars in line */
	Tflag = 1;
      else if (argv[i][1] == 'i')
	{			/* increment for line requests */
	  inc = atoi (argv[i] + 2);
	  if (inc <= 0)
	    {
	      fprintf (stderr, "(%s)increment must be positive\n", argv0);
	      exit (1);
	    }
	  lineitem = linenow ? linenow->itemhead : NULL;
	  if (lineitem != NULL)
	    {
	      while (lineitem->next != NULL)
		lineitem = lineitem->next;
	      lineitem->inc = inc;
	    }
	  else
	    inc_all = inc;
	}
      else
	{			/* process line, column and field requests */
	  if (linehead == NULL)
	    {
	      if ((linenow = linehead =
		   (struct LINE *) calloc (1, (unsigned)
					   sizeof (struct LINE))) == 0)
		{
		  fprintf (stderr, "(%s)%s", argv[0], memmsg);
		  exit (1);
		}
	      linenow->head = NULL;
	    }

	  if (argv[i][1] == 'l')
	    {
	      if (linenow->head)
		{
		  if ((linenow = linenow->next =
		       (struct LINE *) calloc (1, (unsigned)
					       sizeof (struct LINE))) == 0)
		    {
		      fprintf (stderr, "(%s)%s", argv[0], memmsg);
		      exit (1);
		    }
		}
	      lineitem = linenow->itemhead;
	      if (lineitem != NULL)
		{		/* get to end of line requests */
		  while (lineitem->next != NULL)
		    lineitem = lineitem->next;
		  lineitem->next =
		    getitem (argv[i][2] ==
			     0 ? argv[++i] : argv[i] + 2, LIN, (char *) NULL);
		}
	      else		/* set up linked list */
		linenow->itemhead =
		  getitem (argv[i][2] ==
			   0 ? argv[++i] : argv[i] + 2, LIN, (char *) NULL);
	    }

	  else if (argv[i][1] == 'c' || argv[i][1] == 'f')
	    {
	      type = argv[i][1] == 'c' ? COLUMN : FIELD;
	      colitem = linenow->head;
	      if (colitem != NULL)
		{
		  while (colitem->next != NULL)
		    colitem = colitem->next;
		  colitem = colitem->next =
		    getitem (argv[i][2] ==
			     0 ? argv[++i] : argv[i] + 2, type, sepnow);
		}
	      else
		colitem = linenow->head =
		  getitem (argv[i][2] ==
			   0 ? argv[++i] : argv[i] + 2, type, sepnow);
	      colitem->type = type;
	      colitem->sep = sepnow;
	    }
	  else
	    valid = 0;
	}

      if (!valid)
	fprintf (stderr, "(%s) %s unknown option.\n", argv[0], argv[i]);
      i++;
    }

#ifdef DEBUG
  linenow = linehead;
  i = 1;
  while (linenow != NULL)
    {
      fprintf (stderr, "Request %d:\n", i++);
      lineitem = linenow->itemhead;
      while (lineitem != NULL)
	{
	  fprintf (stderr, "start: %d  finish: %d\n",
		   lineitem->start, lineitem->finish);
	  lineitem = lineitem->next;
	}
      colitem = linenow->head;
      while (colitem != NULL)
	{
	  fprintf (stderr, "%c  start: %d finish %d\n",
		   colitem->type == COLUMN ? 'c' : 'f',
		   colitem->start, colitem->finish);
	  colitem = colitem->next;
	}
      linenow = linenow->next;
    }
#endif

  if ((buf = (char *) malloc ((unsigned) bufsize)) == NULL)
    {
      fprintf (stderr, "(%s)%s", argv[0], memmsg);
      exit (1);
    }

  linenow = linehead;		/* check for valid requests */
  while (linenow != NULL)
    {
      lineitem = linenow->itemhead;
      while (lineitem != NULL)
	{
	  if (lineitem->start > lineitem->finish)
	    {
	      fprintf (stderr, "(%s)%s", argv[0], valmsg);
	      exit (1);
	    }
	  lineitem->sep = NULL;
	  if (lineitem->next != NULL)	/* mark cases that don't need rewind */
	    if (lineitem->next->start > lineitem->finish)
	      lineitem->sep = (char *) 1;
	  lineitem = lineitem->next;
	}
      colitem = linenow->head;
      while (colitem != NULL)
	{
	  if (colitem->start > colitem->finish)
	    {
	      fprintf (stderr, "(%s)%s", argv[0], valmsg);
	      exit (1);
	    }
	  colitem = colitem->next;
	}
      linenow = linenow->next;
    }

  if (nfiles == 0)
    {				/* signal stdin if no files given */
      nfiles = 1;
      argind[0] = -1;
    }

  for (i = 0; i < nfiles; i++)
    {
      if (argind[i] == -1)
	fp = stdin;
      else if ((fp = fopen (argv[argind[i]], "r")) == NULL)
	{
	  fprintf (stderr, "(%s)can't open %s\n", argv[0], argv[argind[i]]);
	  continue;
	}
      linenow = linehead;
      k = 0;			/* k is the current line number */
      while (linenow != NULL)
	{
	  lineitem = linenow->itemhead;
	  if (lineitem)
	    {
	      strt = lineitem->start;
	      inc = lineitem->inc ? lineitem->inc : inc_all;
	    }
	  else
	    inc = inc_all;
	  while (fgets (buf, bufsize, fp) != NULL)
	    {
	      while (strlen (buf) == bufsize - 1 && buf[bufsize - 2] != '\n')
		{
		  if ((buf = realloc (buf, 2 * bufsize)) == NULL)
		    {
		      fprintf (stderr, "(%s)%s", argv0, memmsg);
		      exit (1);
		    }
		  /* read in the rest of the line over the null, and fill up buf for the while */
		  if (fgets (buf + bufsize - 1, bufsize + 1, fp) == NULL)
		    continue;
		  bufsize *= 2;
		}
	      pass = 0;
	      k++;
	      /* check to see if the line should be processed - either no line 
	         requests are given, or this line is within the range of the 
	         current request */
	      if (lineitem == NULL ||
		  ((k >= lineitem->start && k <= lineitem->finish)
		   && !((k - strt) % inc)))
		{
		  colitem = linenow->head;
		  l = strlen (buf);
		  if (colitem == NULL)
		    {		/* with no colitem, print everything */
		      buf[l - 1] = 0;
		      printf ("%s", buf);
		    }
		  while (colitem != NULL)
		    {
		      if (colitem->type == COLUMN)
			{
			  for (i =
			       colitem->start - 1; i < colitem->finish; i++)
			    {
			      if (i > l)
				break;
			      if (buf[i] == '\n')
				break;
			      printf ("%c", buf[i]);
			    }
			  if (colitem->sep != 0)
			    /* don't print the separator at the end of a line */
			    if (colitem->next)
			      printf ("%s", colitem->sep);
			}
		      else if (colitem->type == FIELD)
			{
			  nf = 1;
			  if (tflag)
			    {	/* look for input separator */
			      if (pflag && !pass)
				{
				  for (j = 0; j < l; j++)
				    if (buf[j] == fsep)
				      break;
				  if (j >= l)
				    {
				      buf[l - 1] = 0;
				      printf ("%s", buf);
				      pass = 1;
				    }
				}
			      if (!pass)
				{	/* at least one separator found */
				  j = 0;
				  while (nf < colitem->start && j < l)
				    {
				      while (buf[j++] != fsep && j < l);
				      nf++;
				    }
				  while (nf <= colitem->finish && j < l)
				    {
				      while (buf[j] != fsep && j < l - 1)
					printf ("%c", buf[j++]);
				      if (Tflag && buf[j] == fsep)
					printf ("%c", fsep);
				      if (colitem->sep != 0)
					if ((nf < colitem->finish
					     && j < l - 1) || colitem->next)
					  printf ("%s", colitem->sep);
				      j++;
				      nf++;
				    }
				}
			    }
			  else
			    {	/* default - fields sep. by white space */
			      for (j = 0; isspace (buf[j]) && j < l; j++);
			      while (nf < colitem->start && j < l)
				{
				  while (!isspace (buf[j++]));
				  for (; isspace (buf[j]) && j < l; j++);
				  nf++;
				}
			      while (nf <= colitem->finish && j < l)
				{
				  while (!isspace (buf[j]))
				    printf ("%c", buf[j++]);
				  if (colitem->sep != 0)
				    if ((nf < colitem->finish
					 && j < l - 1) || colitem->next)
				      printf ("%s", colitem->sep);
				  while (isspace (buf[j]))
				    j++;
				  nf++;
				}
			    }
			}
		      colitem = colitem->next;
		    }
		  printf ("\n");
		}		/* end of processing for this line */
	      if (lineitem == NULL)
		continue;
	      if (k >= lineitem->finish)
		{		/* past the current line request */
		  if (!lineitem->sep)
		    {
		      rewind (fp);
		      k = 0;
		    }
		  lineitem = lineitem->next;
		  if (lineitem)
		    {
		      strt = lineitem->start;
		      inc = lineitem->inc ? lineitem->inc : inc_all;
		    }
		  else
		    inc = inc_all;
		}
	      if (lineitem == NULL)
		break;
	    }			/* end of fgets loop */
	  /* if an n- line request was given, or a request with no line range, 
	     need to rewind.  can't recognize this case until we've finished the 
	     whole file, so it can't be incorporated into previous rewind check */
	  if (!lineitem || (lineitem && lineitem->finish == INT_MAX))
	    {
	      rewind (fp);
	      k = 0;
	    }
	  linenow = linenow->next;
	}			/* end of linenow loop */
      fclose (fp);
    }				/* end of i (nfiles) loop */
  exit (0);
}

struct ITEM *
getitem (char *str, int type, char *sepnow)
{
  int i, j;
  struct ITEM *newitem;
  char schar;

  i = j = 0;

  if (!*str)
    return (NULL);		/* terminates recursive calls */

  if ((newitem =
       (struct ITEM *) calloc (1, (unsigned) sizeof (struct ITEM))) == 0)
    {
      fprintf (stderr, "(%s)%s", argv0, memmsg);
      exit (1);
    }

  newitem->next = NULL;
  newitem->type = type;
  newitem->inc = 0;
  if (type != LIN)
    newitem->sep = sepnow;

  while (isdigit (str[i]))
    i++;			/* advance to end of first number */

  if (str[i] == ' ' || str[i] == 0)
    {				/* single number request */
      str[i] = 0;
      newitem->start = newitem->finish = atoi (str + j);
      if (newitem->start <= 0)
	{
	  fprintf (stderr, "(%s)%s", argv0, nummsg);
	  exit (0);
	}
    }

  else if (str[i] == '-')
    {				/* dash list */
      str[i++] = 0;		/* terminate for atoi */
      newitem->start = atoi (str + j);
      if (newitem->start <= 0)
	{
	  fprintf (stderr, "(%s)%s", argv0, nummsg);
	  exit (0);
	}
      j = i;			/* find what's next */
      schar = str[i];
      if (schar == 0 || schar == ',')	/* n- type request */
	newitem->finish = INT_MAX;
      else if (isdigit (str[i]))
	{			/* n-m type request */
	  while (isdigit (str[i]))
	    i++;
	  schar = str[i];
	  str[i] = 0;
	  newitem->finish = atoi (str + j);
	}
      if (!schar);
      /* for a comma list after a dash list, call getitem recursively */
      else if (schar == ',')
	newitem->next = getitem (str + ++i, type, sepnow);
      else
	{
	  fprintf (stderr, "(%s)Error: illegal range list.\n", argv0);
	  exit (0);
	}

    }

  else if (str[i] == ',')
    {				/* call getitem recursively */
      str[i] = 0;
      newitem->start = newitem->finish = atoi (str + j);
      if (newitem->start <= 0)
	{
	  fprintf (stderr, "(%s)%s", argv0, nummsg);
	  exit (0);
	}
      newitem->next = getitem (str + ++i, type, sepnow);
    }

  else
    {
      fprintf (stderr, "(%s)Error: illegal list.\n", argv0);
      exit (1);
    }

  return (newitem);
}
