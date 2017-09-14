[![Build Status](https://travis-ci.org/olemke/nsscheck.svg?branch=master)](https://travis-ci.org/olemke/nsscheck)

Program to check list of AMSU-A/AMSU-B/MHS/HIRS satellite data files
for gaps and duplicates.

Filenames have to be in the format:

  NSS.MHSX.M1.D13212.S2151.E2241.B0450707.MM
      |    |   | |    |     |
      |    |   | |    |     End time
      |    |   | |    Start time
      |    |   | Day of year
      |    |   Year
      |    Satellite
      Instrument

List of filenames is read from stdin, e.g.:

`cat filelist | nsscheck -g`

or

`find . -name "NSS*" | nsscheck -g`

