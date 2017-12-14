/* Check NSS data for continuity
 *
 * Copyright (c) 2013 Oliver Lemke
 */


#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "nssdups.h"
#include "nssgaps.h"

static int check_flag = 0;
static int dupdetection_flag = 0;
static int gapdetection_flag = 0;
static long gapsize = 0;
static int printinfo_flag = 0;
static int printlist_flag = 0;
static int refine_flag = 1;
static int verbose_flag = 0;
static char *timestamp = NULL;
static char *movedirectory = NULL;


void parse_options(int argc, char *argv[]);

void print_usage(char *argv[]);


int
main(int argc, char *argv[])
{
    nss_swath_list *swath_list;

    parse_options(argc, argv);

    if (!(printlist_flag
          || printinfo_flag
          || dupdetection_flag
          || gapdetection_flag
          || timestamp))
    {
        print_usage(argv);
        printf("\nYou must at least specify -d, -g, -i, -l or -t.\n");

        return (EXIT_FAILURE);
    }

    swath_list = nss_build_swath_list(stdin, verbose_flag);

    if (!swath_list)
    {
        fprintf(stderr, "Error building swath list\n");
        return (EXIT_FAILURE);
    }

    if (printlist_flag) nss_print_swath_list(swath_list);
    if (dupdetection_flag)
        nss_detect_duplicates(swath_list, movedirectory,
                              check_flag);
    if (gapdetection_flag) nss_detect_gaps(swath_list, gapsize, refine_flag);
    if (timestamp) nss_check_timestamp(swath_list, timestamp);
    if (printinfo_flag) nss_print_info(swath_list);

    nss_free_swath_list(swath_list, 1);

    return (EXIT_SUCCESS);
}


void print_usage(char *argv[])
{
    static char helptext[] =
            "  -c, --check            Run zamsu2l1c on duplicate files.\n" \
    "  -d, --duplicates       Find duplicate files.\n" \
    "  -g, --gaps             Perform gap detection.\n" \
    "  -h, --help             Print this help\n" \
    "  -i, --info             Print info about the input data.\n" \
    "  -l, --list             Print swath list.\n" \
    "      --norefine         Only do simple gap detection. (FOR TESTING ONLY)\n" \
    "  -m, --move=DIRECTORY   Move duplicate files to DIRECTORY. Requires -c.\n" \
    "  -s, --size=GAPSIZE     Ignore gaps smaller than GAPSIZE minutes.\n" \
    "  -t, --timestamp=TIME   Check if any file provides data for the\n" \
    "                         given timestamp. TIME needs to be specified\n" \
    "                         the format yyyy-mm-dd hh:mm. NOTE: The time\n" \
    "                         you give is converted from localtime to GMT.\n" \
    "  -v, --verbose          Be more verbose.\n" \
    "\n" \
    "Report bugs to Oliver Lemke <olemke@core-dump.info>.\n";

    printf("Usage: %s OPTIONS\n", argv[0]);
    printf("\n");
    printf("%s", helptext);
}


void
parse_options(int argc, char *argv[])
{
    int c;

    static struct option long_options[] =
            {
                    /* These options set a flag. */
                    {"check",      no_argument,       0, 'c'},
                    {"duplicates", no_argument,       0, 'd'},
                    {"gaps",       no_argument,       0, 'g'},
                    {"help",       no_argument,       0, 'h'},
                    {"info",       no_argument,       0, 'i'},
                    {"list",       no_argument,       0, 'l'},
                    {"move",       required_argument, 0, 'm'},
                    {"norefine",   no_argument,       0, 'r'},
                    {"size",       required_argument, 0, 's'},
                    {"timestamp",  required_argument, 0, 't'},
                    {"verbose",    no_argument,       0, 'v'},
                    {0, 0,                            0, 0}
            };

    while (1)
    {
        /* getopt_long stores the option index here. */
        int option_index = 0;

        c = getopt_long(argc, argv, "cdghilm:s:t:v",
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
                printf("option %s", long_options[option_index].name);
                if (optarg)
                    printf(" with arg %s", optarg);
                printf("\n");
                break;

            case 'h':
                print_usage(argv);
                exit(EXIT_SUCCESS);

            case 'm':
                movedirectory = optarg;
                break;

            case 's':
                gapsize = strtol(optarg, NULL, 10);
                break;

            case 't':
                timestamp = optarg;
                break;

                /* Flags */
            case 'c':
                check_flag = 1;
                break;
            case 'd':
                dupdetection_flag = 1;
                break;
            case 'g':
                gapdetection_flag = 1;
                break;
            case 'i':
                printinfo_flag = 1;
                break;
            case 'l':
                printlist_flag = 1;
                break;
            case 'r':
                refine_flag = 0;
                break;
            case 'v':
                verbose_flag = 1;
                break;

            case '?':
                /* getopt_long already printed an error message. */
                exit(EXIT_FAILURE);

            default:
                exit(EXIT_FAILURE);
        }
    }
}
