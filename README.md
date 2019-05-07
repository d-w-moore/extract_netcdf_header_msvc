# NetCDF header extraction microservice for iRODS


-----

## System requirements

   - netcdf
   - hdf5
   - zlib

## Centos7

Enable the [EPEL](https://fedoraproject.org/wiki/EPEL) repo
before installing the above mentioned libraries.

-----
# extract_netcdf_header_msvc


## Test run command line version
---
Download from [NetCDF examples](https://www.unidata.ucar.edu/software/netcdf/examples/files.html)
 to /tmp
 
```
$ ls /tmp
-rw-rw-r-- 1 dan dan 2767916 May  1 17:39 /tmp/sresa1b_ncar_ccsm3-example.nc
-rw-rw-r-- 1 dan dan   90645 May  4 23:22 /tmp/test_hgroups.nc
$ sh ../Build_cmdline.sh
$ ./a.out /tmp/s*nc
$ ./.out /tmp/t*nc
```

