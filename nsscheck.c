/* nsscheck: Check NSS data for continuity
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
 */


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <time.h>

static int gapdetection_flag = 1;
static int gapsize = 0;
static int printlist_flag = 0;
static int verbose_flag = 0;

typedef struct _nss_swath_data
{
  time_t stime;
  time_t etime;
} nss_swath_data;

typedef struct _nss_swath_list
{
  nss_swath_data *swath;
  struct _nss_swath_list *next;
} nss_swath_list;


int is_leap_year (int year);
void parse_options (int argc, char *argv[]);

nss_swath_list * nss_build_swathlist (FILE *fp);
void nss_detect_gaps (const nss_swath_list *swath_list, int gap_size);
int nss_parse_filename (const char *buf, nss_swath_data *swath);
void nss_print_swathlist (const nss_swath_list *swath_list);


int
main (int argc, char *argv[])
{
  nss_swath_list *swath_list;

  parse_options (argc, argv);

  swath_list = nss_build_swathlist (stdin);

  if (printlist_flag) nss_print_swathlist (swath_list);

  if (gapdetection_flag) nss_detect_gaps (swath_list, gapsize);

  return (EXIT_SUCCESS);
}


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


void
parse_options (int argc, char *argv[])
{
  int c;

  while (1)
    {
      static struct option long_options[] =
        {
          /* These options set a flag. */
            {"gap",     required_argument, 0, 'g'},
            {"help",    no_argument,       0, 'h'},
            {"list",    no_argument,       0, 'l'},
            {"nogap",   no_argument,       0, 'n'},
            {"verbose", no_argument,       0, 'v'},
            {0, 0, 0, 0}
        };

      static char helptext[] =
        "  -g, --gap=GAPSIZE      Ignore gaps smaller than GAPSIZE minutes.\n" \
        "  -h, --help             Print this help\n" \
        "  -l, --list             Print swath list.\n" \
        "  -n, --nogap            Skip gap detection.\n" \
        "  -v, --verbose          Be more verbose. (not implemented)\n" \
        "\n" \
        "Report bugs to Oliver Lemke <olemke@core-dump.info>.\n";

      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "g:hlnv",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c)
        {
        case 0:
          /* If this option set a flag, do nothing else now. */
          if (long_options[option_index].flag != 0)
            break;
          printf ("option %s", long_options[option_index].name);
          if (optarg)
            printf (" with arg %s", optarg);
          printf ("\n");
          break;

        case 'g':
          gapsize = strtol (optarg, NULL, 10);
          break;

        case 'h':
          printf ("Usage: %s OPTIONS\n", argv[0]);
          printf ("\n");
          printf ("%s", helptext);
          exit (EXIT_SUCCESS);

        case 'l':
          printlist_flag = 1;
          break;

        case 'n':
          gapdetection_flag = 0;
          break;

        case 'v':
          verbose_flag = 1;
          break;

        case '?':
          /* getopt_long already printed an error message. */
          exit (EXIT_FAILURE);

        default:
          exit (EXIT_FAILURE);
        }
    }
}


nss_swath_list *
nss_build_swathlist (FILE *fp)
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
      if (!nss_parse_filename (buf, swath))
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
nss_detect_gaps (const nss_swath_list *swath_list, int gap_size)
{
  nss_swath_list *cur = (nss_swath_list *)swath_list;

  while (cur->next && cur->next->swath)
    {
      if (cur->next->swath->stime > cur->swath->etime
          && cur->next->swath->stime - cur->swath->etime >= gap_size*60)
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
  printf ("Gaps larger than %d minutes.\n", gap_size);
}


/* Parse the start and end time of the swath and convert them to unix time
 * format (seconds elapsed since 1970-01-01 00:00:00)
 */
int
nss_parse_filename (const char *buf, nss_swath_data *swath)
{
  char *nss_start;
  char str[4];
  time_t year;
  time_t day;
  time_t hour;
  time_t min;
  int leap = 0;

  if (NULL == (nss_start = strstr (buf, "NSS."))) return 1;

  str[2] = '\0'; strncpy (str, nss_start + 13, 2);
  year = strtol (str, NULL, 10);
  if (year >= 70)
    year += 1900;
  else
    year += 2000;

  if (verbose_flag) printf ("Original Start: %4ld", year);

  leap = is_leap_year (year);

  str[3] = '\0'; strncpy (str, nss_start + 15, 3);
  day = strtol (str, NULL, 10);
  if (verbose_flag) printf (" %3ld", day);

  str[2] = '\0'; strncpy (str, nss_start + 20, 2);
  hour = strtol (str, NULL, 10);
  if (verbose_flag) printf (" %02ld", hour);

  str[2] = '\0'; strncpy (str, nss_start + 22, 2);
  min = strtol (str, NULL, 10);
  if (verbose_flag) printf (":%02ld", min);

  swath->stime = (year - 1970) * 60 * 60 * 24 * 365
   + leap * 60 * 60 * 24
   /* FIXME: Why are we 8 days short here??????? */
   + (day+8)  * 60 * 60 * 24
   + hour * 60 * 60
   + min  * 60;

  str[2] = '\0'; strncpy (str, nss_start + 26, 2);
  hour = strtol (str, NULL, 10);
  if (verbose_flag) printf (" End:          %02ld", hour);

  str[2] = '\0'; strncpy (str, nss_start + 28, 2);
  min = strtol (str, NULL, 10);
  if (verbose_flag) printf (":%02ld\n", min);

  swath->etime = (year - 1970) * 60 * 60 * 24 * 365
   + leap * 60 * 60 * 24
   /* FIXME: Why are we 8 days short here??????? */
   + (day+8)  * 60 * 60 * 24
   + hour * 60 * 60
   + min  * 60;

  if (swath->etime < swath->stime) swath->etime += 60 * 60 * 24;

  if (verbose_flag)
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

