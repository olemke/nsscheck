/* Copyright (c) 2013 Oliver Lemke */

#ifndef NSSSWATH_H
#define NSSSWATH_H

#include <stdio.h>
#include <string.h>
#include <time.h>


typedef struct _nss_swath_data
{
    char *filename;
    char basestring[28];
    char satellite[3];
    char instrument[5];
    time_t stime;
    time_t etime;
} nss_swath_data;

typedef struct _nss_swath_list
{
    nss_swath_data *swath;
    struct _nss_swath_list *next;
} nss_swath_list;


int is_leap_year(int year);

nss_swath_list *nss_build_swath_list(FILE *fp, int verbose);

void nss_check_timestamp(const nss_swath_list *swath_list, char *timestamp);

void nss_free_swath_list(const nss_swath_list *swath_list, int free_data);

void nss_print_info(const nss_swath_list *swath_list);

void nss_print_swath_list(const nss_swath_list *swath_list);

#endif /* NSSSWATH_H */