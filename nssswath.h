/* nssswath.h
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

#ifndef NSSSWATH_H
#define NSSSWATH_H

#include <stdio.h>
#include <string.h>
#include <time.h>


typedef struct _nss_swath_data
{
  char * filename;
  time_t stime;
  time_t etime;
} nss_swath_data;

typedef struct _nss_swath_list
{
  nss_swath_data *swath;
  struct _nss_swath_list *next;
} nss_swath_list;


int is_leap_year (int year);

nss_swath_list * nss_build_swath_list (FILE *fp, int verbose);
void nss_detect_gaps (const nss_swath_list *swath_list,
                      int gapsize, int refine);
void nss_free_swath_list (const nss_swath_list *swath_list);
void nss_print_swath_list (const nss_swath_list *swath_list);

#endif /* NSSSWATH_H */

