/*

Copyright (c) 1993, 1994, 1998 The Open Group.

Permission to use, copy, modify, distribute, and sell this software and its
documentation for any purpose is hereby granted without fee, provided that
the above copyright notice appear in all copies and that both that
copyright notice and this permission notice appear in supporting
documentation.

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
OPEN GROUP BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the name of The Open Group shall not be
used in advertising or otherwise to promote the sale, use or other dealings
in this Software without prior written authorization from The Open Group.

*/

/* C_Groups hermes
   $Id$
   
   Copyright (2008) Sandia Corporation. Under the terms of
   Contract DE-AC04-94AL85000 with Sandia Corporation, the U.S.
   Government retains certain rights in this software.
   
   Hermes is free software: you can redistribute it and/or modify
   it under the terms of the GNU Lesser General Public License as
   published by the Free Software Foundation, either version 3 of
   the License, or (at your option) any later version.
   
   Hermes is distributed in the hope that it will be useful, but
   WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU Lesser General Public License for more details.
   
   You should have received a copy of the GNU Lesser General
   Public License along with Hermes.  If not, see
   <http://www.gnu.org/licenses/>.
   
   Hermes uses def.h with some modification:
     1. STANDALONE mode is hard-wired.
     2. "macro-ized" path name added to struct inclist.
     3. Add const attribute to argument of function "copy".

*/

#define STANDALONE

#ifndef STANDALONE
#include "Xos.h"
#include "Xfuncproto.h"
#endif
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#if 0
#ifndef X_NOT_POSIX
#ifndef _POSIX_SOURCE
#define _POSIX_SOURCE
#endif
#endif
#endif
#include <sys/types.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAXDEFINES	512
#define MAXFILES	1024
#define MAXINCFILES	128	/* "-include" files */
#define MAXDIRS		64
#define SYMTABINC	10	/* must be > 1 for define() to work right */
#define	TRUE		1
#define	FALSE		0

/* the following must match the directives table in main.c */
#define	IF		0
#define	IFDEF		1
#define	IFNDEF		2
#define	ELSE		3
#define	ENDIF		4
#define	DEFINE		5
#define	UNDEF		6
#define	INCLUDE		7
#define	LINE		8
#define	PRAGMA		9
#define ERROR           10
#define IDENT           11
#define SCCS            12
#define ELIF            13
#define EJECT           14
#define WARNING         15
#define INCLUDENEXT     16
#define IFFALSE         17     /* pseudo value --- never matched */
#define ELIFFALSE       18     /* pseudo value --- never matched */
#define INCLUDEDOT      19     /* pseudo value --- never matched */
#define IFGUESSFALSE    20     /* pseudo value --- never matched */
#define ELIFGUESSFALSE  21     /* pseudo value --- never matched */
#define INCLUDENEXTDOT  22     /* pseudo value --- never matched */

#ifdef DEBUG
extern int	_debugmask;
/*
 * debug levels are:
 * 
 *     0	show ifn*(def)*,endif
 *     1	trace defined/!defined
 *     2	show #include
 *     3	show #include SYMBOL
 *     4-6	unused
 */
#define debug(level,arg) { if (_debugmask & (1 << level)) warning arg; }
#else
#define	debug(level,arg) /**/
#endif /* DEBUG */

typedef	unsigned char boolean;

struct symtab {
	char	*s_name;
	char	*s_value;
};

/* possible i_flag */
#define DEFCHECKED	(1<<0)	/* whether defines have been checked */
#define NOTIFIED	(1<<1)	/* whether we have revealed includes */
#define MARKED		(1<<2)	/* whether it's in the makefile */
#define SEARCHED	(1<<3)	/* whether we have read this */
#define FINISHED	(1<<4)	/* whether we are done reading this */
#define INCLUDED_SYM	(1<<5)	/* whether #include SYMBOL was found
				   Can't use i_list if TRUE */
struct	inclist {
	char		*i_incstring;	/* string from #include line */
	char		*i_file;	/* path name of the include file */
        char            *m_file;        /* "macro-ized" path name of file */
	struct inclist	**i_list;	/* list of files it itself includes */
	int		i_listlen;	/* length of i_list */
	struct symtab	**i_defs;	/* symbol table for this file and its
					   children when merged */
	int		i_ndefs;	/* current # defines */
	boolean		*i_merged;      /* whether we have merged child
					   defines */
	unsigned char   i_flags;
};

struct filepointer {
	char	*f_name;
	char	*f_p;
	char	*f_base;
	char	*f_end;
	long	f_len;
	long	f_line;
	long	cmdinc_count;
	char	**cmdinc_list;
	long	cmdinc_line;
};

#include <stdlib.h>
#if defined(macII) && !defined(__STDC__)  /* stdlib.h fails to define these */
char *malloc(), *realloc();
#endif /* macII */

char			*copy(const char *str);
int                     match(char *str, char **list);
char			*base_name(char *file);
char			*getnextline(struct filepointer *fp);
struct symtab		**slookup(char *symbol, struct inclist *file);
struct symtab		**isdefined(char *symbol, struct inclist *file,
				    struct inclist **srcfile);
struct symtab		**fdefined(char *symbol, struct inclist *file,
				   struct inclist **srcfile);
struct filepointer	*getfile(char *file);
void                    included_by(struct inclist *ip, 
				    struct inclist *newfile);
struct inclist		*newinclude(char *newfile, char *incstring);
void                    inc_clean (void);
struct inclist		*inc_path(char *file, char *include, int type);

void                    freefile(struct filepointer *fp);

void                    define2(char *name, char *val, struct inclist *file);
void                    define(char *def, struct inclist *file);
void                    undefine(char *symbol, struct inclist *file);
int                     find_includes(struct filepointer *filep, 
				      struct inclist *file, 
				      struct inclist *file_red, 
				      int recursion, boolean failOK);

void                    recursive_pr_include(struct inclist *head, 
					     char *file, char *base);
void                    add_include(struct filepointer *filep, 
				    struct inclist *file, 
				    struct inclist *file_red, 
				    char *include, int type,
				    boolean failOK);

int                     cppsetup(char *filename,
				 char *line,
				 struct filepointer *filep,
				 struct inclist *inc);


extern void fatalerr(char *, ...);
extern void warning(char *, ...);
extern void warning1(char *, ...);
