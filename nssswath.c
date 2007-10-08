/* nssswath.c
 *
 * Copyright (C) 2007 Oliver Lemke
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <stdlib.h>
#include <string.h>
#include "nssswath.h"

nss_swath_list *nss_sort_swaths (const nss_swath_list *swath_list);
int nss_parse_filename (const char *fname, nss_swath_data *swath, int verbose);


int
is_leap_year (int year)
{
  int leap = 0;

  if (year % 400 == 0)
    leap = 1;
  else if (year % 100 == 0)
    leap = 0;
  else if (year % 4 == 0)
    leap = 1;
  else leap = 0;

  return leap;
}

/* Taken from http://www.c.happycodings.com/Sorting_Searching/code5.html */
nss_swath_list *nss_sort_swaths (const nss_swath_list *swath_list) {

  nss_swath_list *head;
  nss_swath_list *a = NULL;
  nss_swath_list *b = NULL; 
  nss_swath_list *c = NULL;
  nss_swath_list *e = NULL; 
  nss_swath_list *tmp = NULL; 

  /* 
  // the `c' node precedes the `a' and `e' node 
  // pointing up the node to which the comparisons
  // are being made. 
  */
  head = (nss_swath_list *)swath_list;
  while(e != head->next)
    {
      c = a = head;
      b = a->next;

      while(a != e)
        {
          if(a->swath && b->swath && a->swath->stime > b->swath->stime)
            {
              if(a == head)
                {
                  tmp = b -> next;
                  b->next = a;
                  a->next = tmp;
                  head = b;
                  c = b;
                }
              else
                {
                  tmp = b->next;
                  b->next = a;
                  a->next = tmp;
                  c->next = b;
                  c = b;
                }
            }
          else
            {
              c = a;
              a = a->next;
            }
          b = a->next;
          if(b == e)
            e = a;
        }
    }

  return (head);
}


nss_swath_list *
nss_build_swath_list (FILE *fp, int verbose)
{
  char buf[1024];
  nss_swath_list *slist, *current_swath;

  slist = malloc (sizeof (nss_swath_list));
  slist->swath = NULL;
  slist->next = NULL;
  current_swath = slist;

  while (!feof (fp))
    {
      nss_swath_data *swath = malloc (sizeof (nss_swath_data));

      fgets (buf, 1024, fp);
      if (feof (fp)) break;
      if (!nss_parse_filename (buf, swath, verbose))
        {
          nss_swath_list *new_swath = malloc (sizeof (nss_swath_list));
          new_swath->swath = NULL;
          new_swath->next = NULL;
          current_swath->swath = swath;
          current_swath->next = new_swath;
          current_swath = new_swath;
        }
      else
        fprintf (stderr, "Parse error: %s\n", buf);

    }

  slist = nss_sort_swaths (slist);

  return (slist);
}


void nss_check_timestamp (const nss_swath_list *swath_list, char *timestamp)
{
  char str[5];
  struct tm tstamp;
  time_t checktime;

  tstamp.tm_sec = 0;
  tstamp.tm_isdst = 0;

  str[4] = '\0'; strncpy (str, timestamp, 4);
  tstamp.tm_year = strtol (str, NULL, 10) - 1900;

  str[2] = '\0'; strncpy (str, timestamp + 5, 2);
  tstamp.tm_mon = strtol (str, NULL, 10) - 1;

  str[2] = '\0'; strncpy (str, timestamp + 8, 2);
  tstamp.tm_mday = strtol (str, NULL, 10);

  str[2] = '\0'; strncpy (str, timestamp + 11, 2);
  tstamp.tm_hour = strtol (str, NULL, 10);

  str[2] = '\0'; strncpy (str, timestamp + 14, 2);
  tstamp.tm_min = strtol (str, NULL, 10);

  checktime = mktime (&tstamp);
  if (checktime == (time_t)(-1))
    {
      printf ("Error converting time.\n");
    }
  else
    {
      char timestr[1024];
      nss_swath_list *cur;
      int found = 0;
      
      cur = (nss_swath_list *)swath_list;
      while (!found && cur && cur->swath)
        {
          if (cur->swath->stime <= checktime && cur->swath->etime >= checktime)
            {
              found = 1;
            }
          else
            {
              cur = cur->next;
            }
        }

      strftime (timestr, 1024, "%Y-%m-%d %H:%M", gmtime (&checktime));
      if (found)
        printf ("Timestamp %s covered by file %s.\n",
                timestr, cur->swath->filename);
      else
        printf ("Timestamp %s not found.\n", timestr);
    }
}


void
nss_detect_gaps (const nss_swath_list *swath_list, int gapsize, int refine)
{
  long gapcount = 0;
  long gapignorecount = 0;
  time_t gaptotal = 0;
  nss_swath_list *cur = (nss_swath_list *)swath_list;

  while (cur->next && cur->next->swath)
    {
      if (cur->next->swath->stime > cur->swath->etime)
        {
          nss_swath_data gap;
          nss_swath_list *it;

          gap.stime = cur->swath->etime;
          gap.etime = cur->next->swath->stime;

          /* To be 100% sure, check the whole swath list for
           * a file that fills the gap. We might have two files
           * with the same starting time but different end time.
           * This check ensures that we take both into account. */
          it = (nss_swath_list *)swath_list;
          while (refine && it && it->swath && gap.stime)
            {
              if (it->swath->stime <= gap.stime
                  && it->swath->etime >= gap.etime)
                {
                  gap.stime = gap.etime = 0;
                }

              if (it->swath->stime <= gap.etime
                  && it->swath->etime >= gap.etime)
                {
                  gap.etime = it->swath->stime;
                }

              if (it->swath->stime <= gap.stime
                  && it->swath->etime >= gap.stime)
                {
                  gap.stime = it->swath->etime;
                }

              it = it->next;
            }

          if (gap.etime - gap.stime > 0)
            {
              if (gap.etime - gap.stime >= gapsize * 60)
                {
                  char timestr[1024];

                  strftime (timestr, 1024, "%Y-%m-%d %H:%M", gmtime (&gap.stime));
                  printf ("Gap between %s", timestr);

                  strftime (timestr, 1024, "%Y-%m-%d %H:%M", gmtime (&gap.etime));
                  printf (" and %s, %ld mins\n", timestr, (gap.etime - gap.stime) / 60);
                  gapcount++;
                  gaptotal += gap.etime - gap.stime;
                }
              else
                gapignorecount++;
            }
        }

      cur = cur->next;
    }

  if (gapcount)
    {
      time_t hours;
      time_t mins;

      hours = gaptotal / (60 * 60);
      mins = (gaptotal / 60) - hours * 60;
      printf ("\n");
      if (hours)
        printf ("%ld gaps with a total length of %ld hours and %ld minutes found.\n",
                gapcount, hours, mins);
      else
        printf ("%ld gaps with a total length of %ld minutes found.\n\n",
                gapcount, mins);
    }

  if (gapsize)
    {
      if (gapignorecount)
        printf ("%ld gaps smaller than %d minutes ignored.\n",
                gapignorecount, gapsize);
      else
        printf ("No gaps smaller than %d minutes ignored.\n", gapsize);
    }

  printf ("\n");
}


void
nss_free_swath_list (const nss_swath_list *swath_list)
{
  nss_swath_list *cur;
  nss_swath_list *die;

  cur = (nss_swath_list *)swath_list;
  while (cur)
    {
      die = cur;
      cur = cur->next;
      if (die->swath && die->swath->filename)
        {
          free (die->swath->filename);
          free (die->swath);
        }
      free (die);
    }
}


/* Parse the start and end time of the swath and convert them to unix time
 * format (seconds elapsed since 1970-01-01 00:00:00)
 */
int
nss_parse_filename (const char *fname, nss_swath_data *swath, int verbose)
{
  char *nss_start;
  char *buf;
  char str[4];
  int i;
  time_t year;
  time_t day;
  time_t hour;
  time_t min;

  if (NULL == (nss_start = strstr (fname, "NSS."))) return 1;

  swath->filename = strdup (fname);
  if (NULL != (buf = strstr (swath->filename, "\n")))
      *buf = '\0';

  str[2] = '\0'; strncpy (str, nss_start + 13, 2);
  year = strtol (str, NULL, 10);
  if (year >= 70)
    year += 1900;
  else
    year += 2000;

  if (verbose) printf ("Filename: %s\n", swath->filename);
  if (verbose) printf ("Original Start: %4ld", year);

  str[3] = '\0'; strncpy (str, nss_start + 15, 3);
  day = strtol (str, NULL, 10);
  if (verbose) printf (" %3ld", day);

  str[2] = '\0'; strncpy (str, nss_start + 20, 2);
  hour = strtol (str, NULL, 10);
  if (verbose) printf (" %02ld", hour);

  str[2] = '\0'; strncpy (str, nss_start + 22, 2);
  min = strtol (str, NULL, 10);
  if (verbose) printf (":%02ld", min);

  swath->stime = (year - 1970) * 60 * 60 * 24 * 365
   + (day - 1)  * 60 * 60 * 24
   + hour * 60 * 60
   + min  * 60;

  for (i=1972; i<year; i++)
    swath->stime += is_leap_year (i) * 60 * 60 * 24;

  str[2] = '\0'; strncpy (str, nss_start + 26, 2);
  hour = strtol (str, NULL, 10);
  if (verbose) printf (" End:          %02ld", hour);

  str[2] = '\0'; strncpy (str, nss_start + 28, 2);
  min = strtol (str, NULL, 10);
  if (verbose) printf (":%02ld\n", min);

  swath->etime = (year - 1970) * 60 * 60 * 24 * 365
   + (day - 1)  * 60 * 60 * 24
   + hour * 60 * 60
   + min  * 60;

  for (i=1972; i<year; i++)
    swath->etime += is_leap_year (i) * 60 * 60 * 24;

  if (swath->etime < swath->stime) swath->etime += 60 * 60 * 24;

  if (verbose)
    {
      char timestr[1024];
      strftime (timestr, 1024, "%Y %j %H:%M", gmtime (&swath->stime));
      printf ("UNIX TS  Start: %s", timestr);
      strftime (timestr, 1024, "%Y %j %H:%M", gmtime (&swath->etime));
      printf (" End: %s\n", timestr);
    }

  return 0;
}


void nss_print_info (const nss_swath_list *swath_list)
{
  nss_swath_list *cur;
  long count = 1;
  time_t stime;
  time_t etime;

  cur = (nss_swath_list *)swath_list;
  if (cur->swath)
    {
      char timestr[1024];

      stime = cur->swath->stime;
      etime = cur->swath->etime;
      cur = cur->next;
      while (cur && cur->swath)
        {
          count++;
          if (cur->swath->stime < stime) stime = cur->swath->stime;
          if (cur->swath->etime > etime) etime = cur->swath->etime;
          cur = cur->next;
        }

      strftime (timestr, 1024, "%Y-%m-%d %H:%M", gmtime (&stime));
      printf ("Start time: %s\n", timestr);
      strftime (timestr, 1024, "%Y-%m-%d %H:%M", gmtime (&etime));
      printf ("End time  : %s\n", timestr);
      printf ("# of files: %ld\n\n", count);
    }
}


void nss_print_swath_list (const nss_swath_list *swath_list)
{
  nss_swath_list *cur = (nss_swath_list *)swath_list;

  while (cur && cur->swath)
    {
      char timestr[1024];
      printf ("Filename: %s\n", cur->swath->filename);
      strftime (timestr, 1024, "%Y-%m-%d %H:%M", gmtime (&cur->swath->stime));
      printf ("Start time: %s", timestr);
      strftime (timestr, 1024, "%Y-%m-%d %H:%M", gmtime (&cur->swath->etime));
      printf (" - End time: %s\n", timestr);

      cur = cur->next;
    }
}

