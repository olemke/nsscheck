/* nssdups.c
 *
 * Copyright (C) 2008 Oliver Lemke
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

#include "nssdups.h"


void nss_detect_duplicates (const nss_swath_list *swath_list)
{
  nss_swath_list *cur = (nss_swath_list *)swath_list;
  nss_swath_list *dups = malloc (sizeof (nss_swath_list));
  nss_swath_list *current_dup = dups;
  dups->swath = NULL;
  dups->next = NULL;

  while (cur->next && cur->next->swath)
    {
      if (!strcmp (cur->swath->basestring, cur->next->swath->basestring))
        {
          if (!dups->swath)
            {
              current_dup->swath = cur->swath;
            }
          nss_swath_list *new_dup = malloc (sizeof (nss_swath_list));
          new_dup->swath = cur->next->swath;
          new_dup->next = NULL;
          current_dup->next = new_dup;
          current_dup = new_dup;
        }
      else if (dups->swath)
        {
          current_dup = dups;
          printf ("Duplicates:\n");
          while (current_dup)
            {
              printf ("  %s\n", current_dup->swath->filename);
              current_dup = current_dup->next;
            }
          nss_free_swath_list (dups, 0);
          dups = malloc (sizeof (nss_swath_list));
          current_dup = dups;
          dups->swath = NULL;
          dups->next = NULL;
        }

      cur = cur->next;
    }
}

