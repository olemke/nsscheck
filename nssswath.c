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
#include "nssswath.h"


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


nss_swath_list *
nss_build_swathlist (FILE *fp, int verbose)
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

  return (slist);
}


void
nss_detect_gaps (const nss_swath_list *swath_list, int gapsize)
{
  nss_swath_list *cur = (nss_swath_list *)swath_list;

  while (cur->next && cur->next->swath)
    {
      if (cur->next->swath->stime > cur->swath->etime
          && cur->next->swath->stime - cur->swath->etime >= gapsize*60)
        {
          char timestr[1024];
          strftime (timestr, 1024, "%Y-%m-%d %H:%M",
                    gmtime (&cur->swath->etime));
          printf ("Gap between %s", timestr);

          strftime (timestr, 1024, "%Y-%m-%d %H:%M",
                    gmtime (&cur->next->swath->stime));
          printf (" and %s, %ld mins\n", timestr,
                  (cur->next->swath->stime - cur->swath->etime) / 60);
        }

      cur = cur->next;
    }

  printf ("\n");
  printf ("Gaps larger than %d minutes.\n", gapsize);
}


/* Parse the start and end time of the swath and convert them to unix time
 * format (seconds elapsed since 1970-01-01 00:00:00)
 */
int
nss_parse_filename (const char *buf, nss_swath_data *swath, int verbose)
{
  char *nss_start;
  char str[4];
  time_t year;
  time_t day;
  time_t hour;
  time_t min;

  if (NULL == (nss_start = strstr (buf, "NSS."))) return 1;

  str[2] = '\0'; strncpy (str, nss_start + 13, 2);
  year = strtol (str, NULL, 10);
  if (year >= 70)
    year += 1900;
  else
    year += 2000;

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

  for (int i=1972; i<year; i++)
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

  for (int i=1972; i<year; i++)
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


void nss_print_swathlist (const nss_swath_list *swath_list)
{
  nss_swath_list *cur = (nss_swath_list *)swath_list;

  while (cur && cur->swath)
    {
      printf ("Start: %10ld - End: %10ld\n",
              cur->swath->stime, cur->swath->etime);
      cur = cur->next;
    }
}

