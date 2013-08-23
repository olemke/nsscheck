/* Copyright (c) 2013 Oliver Lemke */

#include "nssgaps.h"


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


