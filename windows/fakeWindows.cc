/*
 * Copyright (c) 1993 Martin Birgmeier
 * All rights reserved.
 *
 * You may redistribute unmodified or modified versions of this source
 * code provided that the above copyright notice and this and the
 * following conditions are retained.
 *
 * This software is provided ``as is'', and comes with no warranties
 * of any kind. I shall in no event be liable for anything that happens
 * to anyone/anything when using this software.
 *  
 *  $Id: fakeWindows.cc,v 1.1 2009-09-07 05:14:29 ueshiba Exp $
 */
#ifdef WIN32
#include "fakeWindows.h"
#include <windows.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

/************************************************************************
*  getopt()								*
************************************************************************/
#define _(msgid)      (msgid)
/* For communication from `getopt' to the caller.
   When `getopt' finds an option that takes an argument,
   the argument value is returned here.
   Also, when `ordering' is RETURN_IN_ORDER,
   each non-option ARGV-element is returned here.  */

__PORT char *optarg;

/* Index in ARGV of the next element to be scanned.
   This is used for communication to and from the caller
   and for communication between successive calls to `getopt'.

   On entry to `getopt', zero means this is the first call; initialize.

   When `getopt' returns -1, this is the index of the first of the
   non-option elements that the caller should itself scan.

   Otherwise, `optind' communicates from one call to the next
   how much of ARGV has been scanned so far.  */

/* 1003.2 says this must be 1 before any call.  */
__PORT int optind = 1;

/* Formerly, initialization of getopt depended on optind==0, which
   causes problems with re-calling getopt as programs generally don't
   know that. */

int __getopt_initialized;

/* The next char to be scanned in the option-element
   in which the last option character we returned was found.
   This allows us to pick up the scan where we left off.

   If this is zero, or a null string, it means resume the scan
   by advancing to the next ARGV-element.  */

static char *nextchar;

/* Callers store zero here to inhibit the error message
   for unrecognized options.  */

__PORT int opterr = 1;

/* Set to an option character which was unrecognized.
   This must be initialized on some systems to avoid linking in the
   system's own getopt implementation.  */

__PORT int optopt = '?';

/* Describe how to deal with options that follow non-option ARGV-elements.

   If the caller did not specify anything,
   the default is REQUIRE_ORDER if the environment variable
   POSIXLY_CORRECT is defined, PERMUTE otherwise.

   REQUIRE_ORDER means don't recognize them as options;
   stop option processing when the first non-option is seen.
   This is what Unix does.
   This mode of operation is selected by either setting the environment
   variable POSIXLY_CORRECT, or using `+' as the first character
   of the list of option characters.

   PERMUTE is the default.  We permute the contents of ARGV as we scan,
   so that eventually all the non-options are at the end.  This allows options
   to be given in any order, even with programs that were not written to
   expect this.

   RETURN_IN_ORDER is an option available to programs that were written
   to expect options and other ARGV-elements in any order and that care about
   the ordering of the two.  We describe each non-option ARGV-element
   as if it were the argument of an option with character code 1.
   Using `-' as the first character of the list of option characters
   selects this mode of operation.

   The special argument `--' forces an end of option-scanning regardless
   of the value of `ordering'.  In the case of RETURN_IN_ORDER, only
   `--' can cause `getopt' to return -1 with `optind' != ARGC.  */

static enum
{
  REQUIRE_ORDER, PERMUTE, RETURN_IN_ORDER
} ordering;

/* Value of POSIXLY_CORRECT environment variable.  */
static char *posixly_correct;

#ifdef  __GNU_LIBRARY__
/* We want to avoid inclusion of string.h with non-GNU libraries
   because there are many ways it can cause trouble.
   On some systems, it contains special magic macros that don't work
   in GCC.  */
# include <string.h>
# define my_index       strchr
#else

# if HAVE_STRING_H || WIN32 /* Pete Wilson mod 7/28/02 */
#  include <string.h>
# else
#  include <strings.h>
# endif

/* Avoid depending on library functions or files
   whose names are inconsistent.  */

#ifndef getenv
extern char *getenv ();
#endif

static char *
my_index (const char* str, int chr)
{
  while (*str)
    {
      if (*str == chr)
        return (char *) str;
      str++;
    }
  return 0;
}

/* If using GCC, we can safely declare strlen this way.
   If not using GCC, it is ok not to declare it.  */
#ifdef __GNUC__
/* Note that Motorola Delta 68k R3V7 comes with GCC but not stddef.h.
   That was relevant to code that was here before.  */
# if (!defined __STDC__ || !__STDC__) && !defined strlen
/* gcc with -traditional declares the built-in strlen to return int,
   and has done so at least since version 2.4.5. -- rms.  */
extern int strlen (const char *);
# endif /* not __STDC__ */
#endif /* __GNUC__ */

#endif /* not __GNU_LIBRARY__ */

/* Handle permutation of arguments.  */

/* Describe the part of ARGV that contains non-options that have
   been skipped.  `first_nonopt' is the index in ARGV of the first of them;
   `last_nonopt' is the index after the last of them.  */

static int first_nonopt;
static int last_nonopt;

#ifdef _LIBC
/* Stored original parameters.
   XXX This is no good solution.  We should rather copy the args so
   that we can compare them later.  But we must not use malloc(3).  */
extern int __libc_argc;
extern char **__libc_argv;

/* Bash 2.0 gives us an environment variable containing flags
   indicating ARGV elements that should not be considered arguments.  */

# ifdef USE_NONOPTION_FLAGS
/* Defined in getopt_init.c  */
extern char *__getopt_nonoption_flags;

static int nonoption_flags_max_len;
static int nonoption_flags_len;
# endif

# ifdef USE_NONOPTION_FLAGS
#  define SWAP_FLAGS(ch1, ch2) \
  if (nonoption_flags_len > 0)                                                \
    {                                                                         \
      char __tmp = __getopt_nonoption_flags[ch1];                             \
      __getopt_nonoption_flags[ch1] = __getopt_nonoption_flags[ch2];          \
      __getopt_nonoption_flags[ch2] = __tmp;                                  \
    }
# else
#  define SWAP_FLAGS(ch1, ch2)
# endif
#else   /* !_LIBC */
# define SWAP_FLAGS(ch1, ch2)
#endif  /* _LIBC */

/* Exchange two adjacent subsequences of ARGV.
   One subsequence is elements [first_nonopt,last_nonopt)
   which contains all the non-options that have been skipped so far.
   The other is elements [last_nonopt,optind), which contains all
   the options processed since those non-options were skipped.

   `first_nonopt' and `last_nonopt' are relocated so that they describe
   the new indices of the non-options in ARGV after they are moved.  */

#if defined __STDC__ && __STDC__
static void exchange (char **);
#endif

static void
exchange (char** argv)
{
  int bottom = first_nonopt;
  int middle = last_nonopt;
  int top = optind;
  char *tem;

  /* Exchange the shorter segment with the far end of the longer segment.
     That puts the shorter segment into the right place.
     It leaves the longer segment in the right place overall,
     but it consists of two parts that need to be swapped next.  */

#if defined _LIBC && defined USE_NONOPTION_FLAGS
  /* First make sure the handling of the `__getopt_nonoption_flags'
     string can work normally.  Our top argument must be in the range
     of the string.  */
  if (nonoption_flags_len > 0 && top >= nonoption_flags_max_len)
    {
      /* We must extend the array.  The user plays games with us and
         presents new arguments.  */
      char *new_str = malloc (top + 1);
      if (new_str == NULL)
        nonoption_flags_len = nonoption_flags_max_len = 0;
      else
        {
          memset (__mempcpy (new_str, __getopt_nonoption_flags,
                             nonoption_flags_max_len),
                  '\0', top + 1 - nonoption_flags_max_len);
          nonoption_flags_max_len = top + 1;
          __getopt_nonoption_flags = new_str;
        }
    }
#endif

  while (top > middle && middle > bottom)
    {
      if (top - middle > middle - bottom)
        {
          /* Bottom segment is the short one.  */
          int len = middle - bottom;
          register int i;

          /* Swap it with the top part of the top segment.  */
          for (i = 0; i < len; i++)
            {
              tem = argv[bottom + i];
              argv[bottom + i] = argv[top - (middle - bottom) + i];
              argv[top - (middle - bottom) + i] = tem;
              SWAP_FLAGS (bottom + i, top - (middle - bottom) + i);
            }
          /* Exclude the moved bottom segment from further swapping.  */
          top -= len;
        }
      else
        {
          /* Top segment is the short one.  */
          int len = top - middle;
          register int i;

          /* Swap it with the bottom part of the bottom segment.  */
          for (i = 0; i < len; i++)
            {
              tem = argv[bottom + i];
              argv[bottom + i] = argv[middle + i];
              argv[middle + i] = tem;
              SWAP_FLAGS (bottom + i, middle + i);
            }
          /* Exclude the moved top segment from further swapping.  */
          bottom += len;
        }
    }

  /* Update records for the slots the non-options now occupy.  */

  first_nonopt += (optind - last_nonopt);
  last_nonopt = optind;
}

/* Initialize the internal data when the first call is made.  */

#if defined __STDC__ && __STDC__
static const char *_getopt_initialize (int, char *const *, const char *);
#endif
static const char *
_getopt_initialize (int argc, char *const *argv, const char* optstring)
{
  /* Start processing options with ARGV-element 1 (since ARGV-element 0
     is the program name); the sequence of previously skipped
     non-option ARGV-elements is empty.  */

  first_nonopt = last_nonopt = optind;

  nextchar = NULL;

  posixly_correct = getenv ("POSIXLY_CORRECT");

  /* Determine how to handle the ordering of options and nonoptions.  */

  if (optstring[0] == '-')
    {
      ordering = RETURN_IN_ORDER;
      ++optstring;
    }
  else if (optstring[0] == '+')
    {
      ordering = REQUIRE_ORDER;
      ++optstring;
    }
  else if (posixly_correct != NULL)
    ordering = REQUIRE_ORDER;
  else
    ordering = PERMUTE;

#if defined _LIBC && defined USE_NONOPTION_FLAGS
  if (posixly_correct == NULL
      && argc == __libc_argc && argv == __libc_argv)
    {
      if (nonoption_flags_max_len == 0)
        {
          if (__getopt_nonoption_flags == NULL
              || __getopt_nonoption_flags[0] == '\0')
            nonoption_flags_max_len = -1;
          else
            {
              const char *orig_str = __getopt_nonoption_flags;
              int len = nonoption_flags_max_len = strlen (orig_str);
              if (nonoption_flags_max_len < argc)
                nonoption_flags_max_len = argc;
              __getopt_nonoption_flags =
                (char *) malloc (nonoption_flags_max_len);
              if (__getopt_nonoption_flags == NULL)
                nonoption_flags_max_len = -1;
              else
                memset (__mempcpy (__getopt_nonoption_flags, orig_str, len),
                        '\0', nonoption_flags_max_len - len);
            }
        }
      nonoption_flags_len = nonoption_flags_max_len;
    }
  else
    nonoption_flags_len = 0;
#endif

  return optstring;
}

/* Scan elements of ARGV (whose length is ARGC) for option characters
   given in OPTSTRING.

   If an element of ARGV starts with '-', and is not exactly "-" or "--",
   then it is an option element.  The characters of this element
   (aside from the initial '-') are option characters.  If `getopt'
   is called repeatedly, it returns successively each of the option characters
   from each of the option elements.

   If `getopt' finds another option character, it returns that character,
   updating `optind' and `nextchar' so that the next call to `getopt' can
   resume the scan with the following option character or ARGV-element.

   If there are no more option characters, `getopt' returns -1.
   Then `optind' is the index in ARGV of the first ARGV-element
   that is not an option.  (The ARGV-elements have been permuted
   so that those that are not options now come last.)

   OPTSTRING is a string containing the legitimate option characters.
   If an option character is seen that is not listed in OPTSTRING,
   return '?' after printing an error message.  If you set `opterr' to
   zero, the error message is suppressed but we still return '?'.

   If a char in OPTSTRING is followed by a colon, that means it wants an arg,
   so the following text in the same ARGV-element, or the text of the following
   ARGV-element, is returned in `optarg'.  Two colons mean an option that
   wants an optional arg; if there is text in the current ARGV-element,
   it is returned in `optarg', otherwise `optarg' is set to zero.

   If OPTSTRING starts with `-' or `+', it requests different methods of
   handling the non-option ARGV-elements.
   See the comments about RETURN_IN_ORDER and REQUIRE_ORDER, above.

   Long-named options begin with `--' instead of `-'.
   Their names may be abbreviated as long as the abbreviation is unique
   or is an exact match for some defined option.  If they have an
   argument, it follows the option name in the same ARGV-element, separated
   from the option name by a `=', or else the in next ARGV-element.
   When `getopt' finds a long-named option, it returns 0 if that option's
   `flag' field is nonzero, the value of the option's `val' field
   if the `flag' field is zero.

   The elements of ARGV aren't really const, because we permute them.
   But we pretend they're const in the prototype to be compatible
   with other systems.

   LONGOPTS is a vector of `struct option' terminated by an
   element containing a name which is zero.

   LONGIND returns the index in LONGOPT of the long-named option found.
   It is only valid when a long-named option has been found by the most
   recent call.

   If LONG_ONLY is nonzero, '-' as well as '--' can introduce
   long-named options.  */

static int
_getopt_internal (int argc, char *const *argv, const char* optstring, const struct option* longopts, int* longind, int long_only)
{
  int print_errors = opterr;
  if (optstring[0] == ':')
    print_errors = 0;

  if (argc < 1)
    return -1;

  optarg = NULL;

  if (optind == 0 || !__getopt_initialized)
    {
      if (optind == 0)
        optind = 1;     /* Don't scan ARGV[0], the program name.  */
      optstring = _getopt_initialize (argc, argv, optstring);
      __getopt_initialized = 1;
    }

  /* Test whether ARGV[optind] points to a non-option argument.
     Either it does not have option syntax, or there is an environment flag
     from the shell indicating it is not an option.  The later information
     is only used when the used in the GNU libc.  */
#if defined _LIBC && defined USE_NONOPTION_FLAGS
# define NONOPTION_P (argv[optind][0] != '-' || argv[optind][1] == '\0'       \
                      || (optind < nonoption_flags_len                        \
                          && __getopt_nonoption_flags[optind] == '1'))
#else
# define NONOPTION_P (argv[optind][0] != '-' || argv[optind][1] == '\0')
#endif

  if (nextchar == NULL || *nextchar == '\0')
    {
      /* Advance to the next ARGV-element.  */

      /* Give FIRST_NONOPT and LAST_NONOPT rational values if OPTIND has been
         moved back by the user (who may also have changed the arguments).  */
      if (last_nonopt > optind)
        last_nonopt = optind;
      if (first_nonopt > optind)
        first_nonopt = optind;

      if (ordering == PERMUTE)
        {
          /* If we have just processed some options following some non-options,
             exchange them so that the options come first.  */

          if (first_nonopt != last_nonopt && last_nonopt != optind)
            exchange ((char **) argv);
          else if (last_nonopt != optind)
            first_nonopt = optind;

          /* Skip any additional non-options
             and extend the range of non-options previously skipped.  */

          while (optind < argc && NONOPTION_P)
            optind++;
          last_nonopt = optind;
        }

      /* The special ARGV-element `--' means premature end of options.
         Skip it like a null option,
         then exchange with previous non-options as if it were an option,
         then skip everything else like a non-option.  */

      if (optind != argc && !strcmp (argv[optind], "--"))
        {
          optind++;

          if (first_nonopt != last_nonopt && last_nonopt != optind)
            exchange ((char **) argv);
          else if (first_nonopt == last_nonopt)
            first_nonopt = optind;
          last_nonopt = argc;

          optind = argc;
        }

      /* If we have done all the ARGV-elements, stop the scan
         and back over any non-options that we skipped and permuted.  */

      if (optind == argc)
        {
          /* Set the next-arg-index to point at the non-options
             that we previously skipped, so the caller will digest them.  */
          if (first_nonopt != last_nonopt)
            optind = first_nonopt;
          return -1;
        }

      /* If we have come to a non-option and did not permute it,
         either stop the scan or describe it to the caller and pass it by.  */

      if (NONOPTION_P)
        {
          if (ordering == REQUIRE_ORDER)
            return -1;
          optarg = argv[optind++];
          return 1;
        }

      /* We have found another option-ARGV-element.
         Skip the initial punctuation.  */

      nextchar = (argv[optind] + 1
                  + (longopts != NULL && argv[optind][1] == '-'));
    }

  /* Decode the current option-ARGV-element.  */

  /* Check whether the ARGV-element is a long option.

     If long_only and the ARGV-element has the form "-f", where f is
     a valid short option, don't consider it an abbreviated form of
     a long option that starts with f.  Otherwise there would be no
     way to give the -f short option.

     On the other hand, if there's a long option "fubar" and
     the ARGV-element is "-fu", do consider that an abbreviation of
     the long option, just like "--fu", and not "-f" with arg "u".

     This distinction seems to be the most useful approach.  */

  if (longopts != NULL
      && (argv[optind][1] == '-'
          || (long_only && (argv[optind][2] || !my_index (optstring, argv[optind][1])))))
    {
      char *nameend;
      const struct option *p;
      const struct option *pfound = NULL;
      int exact = 0;
      int ambig = 0;
      int indfound = -1;
      int option_index;

      for (nameend = nextchar; *nameend && *nameend != '='; nameend++)
        /* Do nothing.  */ ;

      /* Test all long options for either exact match
         or abbreviated matches.  */
      for (p = longopts, option_index = 0; p->name; p++, option_index++)
        if (!strncmp (p->name, nextchar, nameend - nextchar))
          {
            if ((unsigned int) (nameend - nextchar)
                == (unsigned int) strlen (p->name))
              {
                /* Exact match found.  */
                pfound = p;
                indfound = option_index;
                exact = 1;
                break;
              }
            else if (pfound == NULL)
              {
                /* First nonexact match found.  */
                pfound = p;
                indfound = option_index;
              }
            else if (long_only
                     || pfound->has_arg != p->has_arg
                     || pfound->flag != p->flag
                     || pfound->val != p->val)
              /* Second or later nonexact match found.  */
              ambig = 1;
          }

      if (ambig && !exact)
        {
          if (print_errors)
            {
#if defined _LIBC && defined USE_IN_LIBIO
              char *buf;

              __asprintf (&buf, _("%s: option `%s' is ambiguous\n"),
                          argv[0], argv[optind]);

              if (_IO_fwide (stderr, 0) > 0)
                __fwprintf (stderr, L"%s", buf);
              else
                fputs (buf, stderr);

              free (buf);
#else
              fprintf (stderr, _("%s: option `%s' is ambiguous\n"),
                       argv[0], argv[optind]);
#endif
            }
          nextchar += strlen (nextchar);
          optind++;
          optopt = 0;
          return '?';
        }

      if (pfound != NULL)
        {
          option_index = indfound;
          optind++;
          if (*nameend)
            {
              /* Don't test has_arg with >, because some C compilers don't
                 allow it to be used on enums.  */
              if (pfound->has_arg)
                optarg = nameend + 1;
              else
                {
                  if (print_errors)
                    {
#if defined _LIBC && defined USE_IN_LIBIO
                      char *buf;
#endif

                      if (argv[optind - 1][1] == '-')
                        {
                          /* --option */
#if defined _LIBC && defined USE_IN_LIBIO
                          __asprintf (&buf, _("\
%s: option `--%s' doesn't allow an argument\n"),
                                      argv[0], pfound->name);
#else
                          fprintf (stderr, _("\
%s: option `--%s' doesn't allow an argument\n"),
                                   argv[0], pfound->name);
#endif
                        }
                      else
                        {
                          /* +option or -option */
#if defined _LIBC && defined USE_IN_LIBIO
                          __asprintf (&buf, _("\
%s: option `%c%s' doesn't allow an argument\n"),
                                      argv[0], argv[optind - 1][0],
                                      pfound->name);
#else
                          fprintf (stderr, _("\
%s: option `%c%s' doesn't allow an argument\n"),
                                   argv[0], argv[optind - 1][0], pfound->name);
#endif
                        }

#if defined _LIBC && defined USE_IN_LIBIO
                      if (_IO_fwide (stderr, 0) > 0)
                        __fwprintf (stderr, L"%s", buf);
                      else
                        fputs (buf, stderr);

                      free (buf);
#endif
                    }

                  nextchar += strlen (nextchar);

                  optopt = pfound->val;
                  return '?';
                }
            }
          else if (pfound->has_arg == 1)
            {
              if (optind < argc)
                optarg = argv[optind++];
              else
                {
                  if (print_errors)
                    {
#if defined _LIBC && defined USE_IN_LIBIO
                      char *buf;

                      __asprintf (&buf,
                                  _("%s: option `%s' requires an argument\n"),
                                  argv[0], argv[optind - 1]);

                      if (_IO_fwide (stderr, 0) > 0)
                        __fwprintf (stderr, L"%s", buf);
                      else
                        fputs (buf, stderr);

                      free (buf);
#else
                      fprintf (stderr,
                               _("%s: option `%s' requires an argument\n"),
                               argv[0], argv[optind - 1]);
#endif
                    }
                  nextchar += strlen (nextchar);
                  optopt = pfound->val;
                  return optstring[0] == ':' ? ':' : '?';
                }
            }
          nextchar += strlen (nextchar);
          if (longind != NULL)
            *longind = option_index;
          if (pfound->flag)
            {
              *(pfound->flag) = pfound->val;
              return 0;
            }
          return pfound->val;
        }

      /* Can't find it as a long option.  If this is not getopt_long_only,
         or the option starts with '--' or is not a valid short
         option, then it's an error.
         Otherwise interpret it as a short option.  */
      if (!long_only || argv[optind][1] == '-'
          || my_index (optstring, *nextchar) == NULL)
        {
          if (print_errors)
            {
#if defined _LIBC && defined USE_IN_LIBIO
              char *buf;
#endif

              if (argv[optind][1] == '-')
                {
                  /* --option */
#if defined _LIBC && defined USE_IN_LIBIO
                  __asprintf (&buf, _("%s: unrecognized option `--%s'\n"),
                              argv[0], nextchar);
#else
                  fprintf (stderr, _("%s: unrecognized option `--%s'\n"),
                           argv[0], nextchar);
#endif
                }
              else
                {
                  /* +option or -option */
#if defined _LIBC && defined USE_IN_LIBIO
                  __asprintf (&buf, _("%s: unrecognized option `%c%s'\n"),
                              argv[0], argv[optind][0], nextchar);
#else
                  fprintf (stderr, _("%s: unrecognized option `%c%s'\n"),
                           argv[0], argv[optind][0], nextchar);
#endif
                }

#if defined _LIBC && defined USE_IN_LIBIO
              if (_IO_fwide (stderr, 0) > 0)
                __fwprintf (stderr, L"%s", buf);
              else
                fputs (buf, stderr);

              free (buf);
#endif
            }
          nextchar = (char *) "";
          optind++;
          optopt = 0;
          return '?';
        }
    }

  /* Look at and handle the next short option-character.  */

  {
    char c = *nextchar++;
    char *temp = my_index (optstring, c);

    /* Increment `optind' when we start to process its last character.  */
    if (*nextchar == '\0')
      ++optind;

    if (temp == NULL || c == ':')
      {
        if (print_errors)
          {
#if defined _LIBC && defined USE_IN_LIBIO
              char *buf;
#endif

            if (posixly_correct)
              {
                /* 1003.2 specifies the format of this message.  */
#if defined _LIBC && defined USE_IN_LIBIO
                __asprintf (&buf, _("%s: illegal option -- %c\n"),
                            argv[0], c);
#else
                fprintf (stderr, _("%s: illegal option -- %c\n"), argv[0], c);
#endif
              }
            else
              {
#if defined _LIBC && defined USE_IN_LIBIO
                __asprintf (&buf, _("%s: invalid option -- %c\n"),
                            argv[0], c);
#else
                fprintf (stderr, _("%s: invalid option -- %c\n"), argv[0], c);
#endif
              }

#if defined _LIBC && defined USE_IN_LIBIO
            if (_IO_fwide (stderr, 0) > 0)
              __fwprintf (stderr, L"%s", buf);
            else
              fputs (buf, stderr);

            free (buf);
#endif
          }
        optopt = c;
        return '?';
      }
    /* Convenience. Treat POSIX -W foo same as long option --foo */
    if (temp[0] == 'W' && temp[1] == ';')
      {
        char *nameend;
        const struct option *p;
        const struct option *pfound = NULL;
        int exact = 0;
        int ambig = 0;
        int indfound = 0;
        int option_index;

        /* This is an option that requires an argument.  */
        if (*nextchar != '\0')
          {
            optarg = nextchar;
            /* If we end this ARGV-element by taking the rest as an arg,
               we must advance to the next element now.  */
            optind++;
          }
        else if (optind == argc)
          {
            if (print_errors)
              {
                /* 1003.2 specifies the format of this message.  */
#if defined _LIBC && defined USE_IN_LIBIO
                char *buf;

                __asprintf (&buf, _("%s: option requires an argument -- %c\n"),
                            argv[0], c);

                if (_IO_fwide (stderr, 0) > 0)
                  __fwprintf (stderr, L"%s", buf);
                else
                  fputs (buf, stderr);

                free (buf);
#else
                fprintf (stderr, _("%s: option requires an argument -- %c\n"),
                         argv[0], c);
#endif
              }
            optopt = c;
            if (optstring[0] == ':')
              c = ':';
            else
              c = '?';
            return c;
          }
        else
          /* We already incremented `optind' once;
             increment it again when taking next ARGV-elt as argument.  */
          optarg = argv[optind++];

        /* optarg is now the argument, see if it's in the
           table of longopts.  */

        for (nextchar = nameend = optarg; *nameend && *nameend != '='; nameend++)
          /* Do nothing.  */ ;

        /* Test all long options for either exact match
           or abbreviated matches.  */
        for (p = longopts, option_index = 0; p->name; p++, option_index++)
          if (!strncmp (p->name, nextchar, nameend - nextchar))
            {
              if ((unsigned int) (nameend - nextchar) == strlen (p->name))
                {
                  /* Exact match found.  */
                  pfound = p;
                  indfound = option_index;
                  exact = 1;
                  break;
                }
              else if (pfound == NULL)
                {
                  /* First nonexact match found.  */
                  pfound = p;
                  indfound = option_index;
                }
              else
                /* Second or later nonexact match found.  */
                ambig = 1;
            }
        if (ambig && !exact)
          {
            if (print_errors)
              {
#if defined _LIBC && defined USE_IN_LIBIO
                char *buf;

                __asprintf (&buf, _("%s: option `-W %s' is ambiguous\n"),
                            argv[0], argv[optind]);

                if (_IO_fwide (stderr, 0) > 0)
                  __fwprintf (stderr, L"%s", buf);
                else
                  fputs (buf, stderr);

                free (buf);
#else
                fprintf (stderr, _("%s: option `-W %s' is ambiguous\n"),
                         argv[0], argv[optind]);
#endif
              }
            nextchar += strlen (nextchar);
            optind++;
            return '?';
          }
        if (pfound != NULL)
          {
            option_index = indfound;
            if (*nameend)
              {
                /* Don't test has_arg with >, because some C compilers don't
                   allow it to be used on enums.  */
                if (pfound->has_arg)
                  optarg = nameend + 1;
                else
                  {
                    if (print_errors)
                      {
#if defined _LIBC && defined USE_IN_LIBIO
                        char *buf;

                        __asprintf (&buf, _("\
%s: option `-W %s' doesn't allow an argument\n"),
                                    argv[0], pfound->name);

                        if (_IO_fwide (stderr, 0) > 0)
                          __fwprintf (stderr, L"%s", buf);
                        else
                          fputs (buf, stderr);

                        free (buf);
#else
                        fprintf (stderr, _("\
%s: option `-W %s' doesn't allow an argument\n"),
                                 argv[0], pfound->name);
#endif
                      }

                    nextchar += strlen (nextchar);
                    return '?';
                  }
              }
            else if (pfound->has_arg == 1)
              {
                if (optind < argc)
                  optarg = argv[optind++];
                else
                  {
                    if (print_errors)
                      {
#if defined _LIBC && defined USE_IN_LIBIO
                        char *buf;

                        __asprintf (&buf, _("\
%s: option `%s' requires an argument\n"),
                                    argv[0], argv[optind - 1]);

                        if (_IO_fwide (stderr, 0) > 0)
                          __fwprintf (stderr, L"%s", buf);
                        else
                          fputs (buf, stderr);

                        free (buf);
#else
                        fprintf (stderr,
                                 _("%s: option `%s' requires an argument\n"),
                                 argv[0], argv[optind - 1]);
#endif
                      }
                    nextchar += strlen (nextchar);
                    return optstring[0] == ':' ? ':' : '?';
                  }
              }
            nextchar += strlen (nextchar);
            if (longind != NULL)
              *longind = option_index;
            if (pfound->flag)
              {
                *(pfound->flag) = pfound->val;
                return 0;
              }
            return pfound->val;
          }
          nextchar = NULL;
          return 'W';   /* Let the application handle it.   */
      }
    if (temp[1] == ':')
      {
        if (temp[2] == ':')
          {
            /* This is an option that accepts an argument optionally.  */
            if (*nextchar != '\0')
              {
                optarg = nextchar;
                optind++;
              }
            else
              optarg = NULL;
            nextchar = NULL;
          }
        else
          {
            /* This is an option that requires an argument.  */
            if (*nextchar != '\0')
              {
                optarg = nextchar;
                /* If we end this ARGV-element by taking the rest as an arg,
                   we must advance to the next element now.  */
                optind++;
              }
            else if (optind == argc)
              {
                if (print_errors)
                  {
                    /* 1003.2 specifies the format of this message.  */
#if defined _LIBC && defined USE_IN_LIBIO
                    char *buf;

                    __asprintf (&buf,
                                _("%s: option requires an argument -- %c\n"),
                                argv[0], c);

                    if (_IO_fwide (stderr, 0) > 0)
                      __fwprintf (stderr, L"%s", buf);
                    else
                      fputs (buf, stderr);

                    free (buf);
#else
                    fprintf (stderr,
                             _("%s: option requires an argument -- %c\n"),
                             argv[0], c);
#endif
                  }
                optopt = c;
                if (optstring[0] == ':')
                  c = ':';
                else
                  c = '?';
              }
            else
              /* We already incremented `optind' once;
                 increment it again when taking next ARGV-elt as argument.  */
              optarg = argv[optind++];
            nextchar = NULL;
          }
      }
    return c;
  }
}

extern "C" __PORT int
getopt (int argc, char *const *argv, const char* optstring)
{
  return _getopt_internal (argc, argv, optstring,
                           (const struct option *) 0,
                           (int *) 0,
                           0);
}

/************************************************************************
*  gettimeofday()							*
************************************************************************/
#define EPOCHFILETIME	(116444736000000000i64)

extern "C" __PORT int
gettimeofday(struct timeval* tv, struct timezone* tz)
{
    static int		tzflag;

    if (tv)
    {
	FILETIME	tagFileTime;
	GetSystemTimeAsFileTime(&tagFileTime);

	LARGE_INTEGER	largeInt;
	largeInt.LowPart  = tagFileTime.dwLowDateTime;
	largeInt.HighPart = tagFileTime.dwHighDateTime;

	__int64		val64 = (largeInt.QuadPart - EPOCHFILETIME) / 10;

	tv->tv_sec  = (long)(val64 / 1000000);
	tv->tv_usec = (long)(val64 % 1000000);
    }

    if (tz)
    {
	if (!tzflag)
        {
	    _tzset();
	    tzflag++;
	}

      //Visual C++ 6.0でＯＫだった・・
      //tz->tz_minuteswest = _timezone / 60;
      //tz->tz_dsttime = _daylight;

	long	_Timezone = 0;
	_get_timezone(&_Timezone);
	tz->tz_minuteswest = _Timezone / 60;
         
	int	_Daylight = 0;
	_get_daylight(&_Daylight);
	tz->tz_dsttime = _Daylight;
    }

    return 0;
}

/************************************************************************
*  srand48(), erand48(), drand48()					*
************************************************************************/
struct _rand48
{
    unsigned short	_seed[3];
    unsigned short	_mult[3];
    unsigned short	_add;
};
static struct _rand48	__RAND48;

#define __rand48_seed	(r->_seed)
#define __rand48_mult	(r->_mult)
#define __rand48_add	(r->_add)

#define _RAND48_SEED_0	(0x330e)
#define _RAND48_SEED_1	(0xabcd)
#define _RAND48_SEED_2	(0x1234)
#define _RAND48_MULT_0	(0xe66d)
#define _RAND48_MULT_1	(0xdeec)
#define _RAND48_MULT_2	(0x0005)
#define _RAND48_ADD	(0x000b)

static void
__dorand48(struct _rand48* r, unsigned short xseed[3])
{
    unsigned long	accu;
    unsigned short	temp[2];

    accu = (unsigned long) __rand48_mult[0] * (unsigned long) xseed[0] +
	   (unsigned long) __rand48_add;
    temp[0] = (unsigned short) accu;     /* lower 16 bits */
    accu >>= sizeof(unsigned short) * 8;
    accu += (unsigned long) __rand48_mult[0] * (unsigned long) xseed[1] +
	    (unsigned long) __rand48_mult[1] * (unsigned long) xseed[0];
    temp[1] = (unsigned short) accu;     /* middle 16 bits */
    accu >>= sizeof(unsigned short) * 8;
    accu += __rand48_mult[0] * xseed[2] + __rand48_mult[1] * xseed[1]
	  + __rand48_mult[2] * xseed[0];
    xseed[0] = temp[0];
    xseed[1] = temp[1];
    xseed[2] = (unsigned short) accu;
}

static void
_srand48_r(struct _rand48* r, long seed)
{
    __rand48_seed[0] = _RAND48_SEED_0;
    __rand48_seed[1] = (unsigned short) seed;
    __rand48_seed[2] = (unsigned short) ((unsigned long)seed >> 16);
    __rand48_mult[0] = _RAND48_MULT_0;
    __rand48_mult[1] = _RAND48_MULT_1;
    __rand48_mult[2] = _RAND48_MULT_2;
    __rand48_add     = _RAND48_ADD;
}

static double
_erand48_r(struct _rand48* r, unsigned short xseed[3])
{
    __dorand48(r, xseed);
    return ldexp((double) xseed[0], -48) +
	   ldexp((double) xseed[1], -32) +
	   ldexp((double) xseed[2], -16);
}

static double
_drand48_r(struct _rand48* r)
{
    return _erand48_r(r, __rand48_seed);
}

extern "C" __PORT void
srand48(long seed)
{
    _srand48_r(&__RAND48, seed);
}

extern "C" __PORT double
erand48(unsigned short xseed[3])
{
    return _erand48_r(&__RAND48, xseed);
}

extern "C" __PORT double
drand48()
{
    return _drand48_r(&__RAND48);
}

#endif	/* WIN32	*/
