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
#include "nssswath.h"

static int gapdetection_flag = 1;
static int gapsize = 0;
static int printlist_flag = 0;
static int verbose_flag = 0;

void parse_options (int argc, char *argv[]);


int
main (int argc, char *argv[])
{
  nss_swath_list *swath_list;

  parse_options (argc, argv);

  swath_list = nss_build_swathlist (stdin, verbose_flag);

  if (printlist_flag) nss_print_swathlist (swath_list);

  if (gapdetection_flag) nss_detect_gaps (swath_list, gapsize);

  return (EXIT_SUCCESS);
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

