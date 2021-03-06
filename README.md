Program to check list of AMSU-A/AMSU-B/MHS/HIRS satellite data files
for gaps and duplicates.

Usage
-----

Filenames have to be in the format:

```
*NSS.MHSX.M1.D13212.S2151.E2241.B0450707.MM
     |    |   | |    |     |
     |    |   | |    |     End time
     |    |   | |    Start time
     |    |   | Day of year
     |    |   Year
     |    Satellite
     Instrument
```

List of filenames is read from stdin, e.g.:

`cat filelist | nsscheck -g`

or

`find metopa_hirs_2019 -name "*NSS*" | nsscheck -g`

Compilation
-----------

```
mkdir build
cd build
cmake ..
make
./nsscheck -h
```

