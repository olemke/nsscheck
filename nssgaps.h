/* Copyright (c) 2013 Oliver Lemke */

#ifndef NSSGAPS_H
#define NSSGAPS_H

#include "nssswath.h"


void nss_detect_gaps(const nss_swath_list *swath_list,
                     int gapsize, int refine);

#endif /* NSSGAPS_H */
