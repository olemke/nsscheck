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

#include <stdlib.h>
#include "nssdups.h"


void nss_detect_duplicates (const nss_swath_list *swath_list,
                            const char *movedirectory, int checkfiles)
{
  int error = 0;
  int broken_files = 0, good_files = 0, safedel_files = 0;

  nss_swath_list *cur = (nss_swath_list *)swath_list;
  nss_swath_list *dups = malloc (sizeof (nss_swath_list));
  nss_swath_list *safedel = malloc (sizeof (nss_swath_list));
  nss_swath_list *current_safedel = safedel;
  nss_swath_list *broken = malloc (sizeof (nss_swath_list));
  nss_swath_list *current_broken = broken;
  nss_swath_list *current_dup = dups;

  safedel->swath = NULL;
  safedel->next = NULL;

  broken->swath = NULL;
  broken->next = NULL;

  dups->swath = NULL;
  dups->next = NULL;

  while (!error)
    {
      if (cur->next && cur->next->swath && !strcmp (cur->swath->basestring, cur->next->swath->basestring))
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
          int found_not_broken = 0;
          current_dup = dups;
          printf ("Duplicates:\n");
          while (!error && current_dup && current_dup->swath)
            {
              if (checkfiles)
                {
                  int ret;
                  char cmd[2048] = "";

                  strcat (cmd, "zamsu2l1c.sh ");
                  strcat (cmd, current_dup->swath->filename);
                  strcat (cmd, " >/dev/null 2>&1");
                  ret = system (cmd);

                  printf ("  %s (", current_dup->swath->filename);
                  switch (ret)
                    {
                    case 2:
                      printf ("zamsu2l1c cancelled, bailing out");
                      error = 1;
                      break;
                    default:
                      switch (WEXITSTATUS(ret))
                        {
                        case 0:
                          good_files++;
                          printf ("OK");
                          if (found_not_broken)
                            {
                              safedel_files++;
                              nss_swath_list *new_safedel = malloc (sizeof (nss_swath_list));
                              new_safedel->swath = NULL;
                              new_safedel->next = NULL;
                              current_safedel->swath=current_dup->swath;
                              current_safedel->next = new_safedel;
                              current_safedel = new_safedel;
                            }
                          else
                            {
                              found_not_broken = 1;
                            }
                          break;
                        case 1:
                          broken_files++;
                          nss_swath_list *new_broken = malloc (sizeof (nss_swath_list));
                          new_broken->swath = NULL;
                          new_broken->next = NULL;
                          current_broken->swath=current_dup->swath;
                          current_broken->next = new_broken;
                          current_broken = new_broken;
                          printf ("BROKEN");
                          break;
                        default:
                          printf ("Error (%d) running zamsu2l1c.sh. Check your atovs_tools installation", ret);
                          break;
                        }
                      break;
                    }

                  printf (")\n");

                }
              else
                {
                  printf ("  %s\n", current_dup->swath->filename);
                }
              current_dup = current_dup->next;
            }
          nss_free_swath_list (dups, 0);
          dups = malloc (sizeof (nss_swath_list));
          current_dup = dups;
          dups->swath = NULL;
          dups->next = NULL;
        }

      if (!(cur->next && cur->next->swath))
        break;
      else
        cur = cur->next;
    }

  if (!error && checkfiles)
    {
      if (safedel_files)
        {
          if (movedirectory && good_files)
            {
              printf ("\nMoving files:\n");
            }
          else
            {
              printf ("\nFiles safe to delete:\n");
            }

          if (movedirectory && !good_files && broken_files)
            {
              printf ("No good files found. Not going to move anything!!!\n");
            }

          int domove = 1;
          current_safedel = safedel;
          if (current_safedel && current_safedel->swath
              && movedirectory && good_files)
            {
              int ret;
              char cmd[2048] = "";

              strcat (cmd, "mkdir -p ");
              strcat (cmd, movedirectory);
              strcat (cmd, "/dups/");
              printf ("  %s\n", cmd);
              ret = system (cmd);
              switch (ret)
                {
                case 2:
                  printf ("FATAL ERROR: Can't execute mkdir command for %s/dups\n",
                          movedirectory);
                  break;
                default:
                  if (WEXITSTATUS(ret))
                    {
                      printf ("FATAL ERROR: Can't create directory %s/dups\n",
                              movedirectory);
                      domove = 0;
                    }
                }
            }

          while (current_safedel && current_safedel->swath)
            {
              if (domove && movedirectory && good_files)
                {
                  int ret;
                  char cmd[2048] = "";

                  strcat (cmd, "mv ");
                  strcat (cmd, current_safedel->swath->filename);
                  strcat (cmd, " ");
                  strcat (cmd, movedirectory);
                  strcat (cmd, "/dups/");
                  //strcat (cmd, " >/dev/null 2>&1");
                  printf ("  %s\n", cmd);
                  ret = system (cmd);
                  switch (ret)
                    {
                    case 2:
                      printf ("FATAL ERROR: Can't execute mv command for %s\n",
                              current_safedel->swath->filename);
                      break;
                    default:
                      if (WEXITSTATUS(ret))
                        {
                          printf ("FATAL ERROR: Can't move file %s\n",
                                  current_safedel->swath->filename);
                        }
                    }
                }
              else
                {
                  printf ("  %s\n", current_safedel->swath->filename);
                }
              current_safedel = current_safedel->next;
            }
        }

      if (broken_files)
        {
          if (movedirectory && !good_files)
            {
              printf ("No good files found. Not going to move anything!!!\n");
            }

          int domove = 1;
          current_broken = broken;
          if (current_broken && current_broken->swath
              && movedirectory && good_files)
            {
              int ret;
              char cmd[2048] = "";

              strcat (cmd, "mkdir -p ");
              strcat (cmd, movedirectory);
              strcat (cmd, "/broken/");
              printf ("  %s\n", cmd);
              ret = system (cmd);
              switch (ret)
                {
                case 2:
                  printf ("FATAL ERROR: Can't execute mkdir command for %s/broken\n",
                          movedirectory);
                  break;
                default:
                  if (WEXITSTATUS(ret))
                    {
                      printf ("FATAL ERROR: Can't create directory %s/broken\n",
                              movedirectory);
                      domove = 0;
                    }
                }
            }

          printf ("\nBroken files:\n");

          while (current_broken && current_broken->swath)
            {
              printf ("  %s\n", current_broken->swath->filename);
              if (domove && movedirectory && good_files)
                {
                  int ret;
                  char cmd[2048] = "";

                  strcat (cmd, "mv ");
                  strcat (cmd, current_broken->swath->filename);
                  strcat (cmd, " ");
                  strcat (cmd, movedirectory);
                  strcat (cmd, "/broken/");
                  //strcat (cmd, " >/dev/null 2>&1");
                  printf ("  %s\n", cmd);
                  ret = system (cmd);
                  switch (ret)
                    {
                    case 2:
                      printf ("FATAL ERROR: Can't execute mv command for %s\n",
                              current_broken->swath->filename);
                      break;
                    default:
                      if (WEXITSTATUS(ret))
                        {
                          printf ("FATAL ERROR: Can't move file %s\n",
                                  current_broken->swath->filename);
                        }
                    }
                }
              else
                {
                  printf ("  %s\n", current_broken->swath->filename);
                }
              current_broken = current_broken->next;
            }
        }

      if (!good_files && broken_files)
        {
          printf ("\nWARNING!!!! Found %d broken files but no good files.\n", broken_files);
          printf ("This might indicate a problem with your zamsu2l1c.sh.\n");
          printf ("Make sure it is working properly!!!\n");
        }
    }

  nss_free_swath_list (safedel, 0);
  nss_free_swath_list (broken, 0);
}

