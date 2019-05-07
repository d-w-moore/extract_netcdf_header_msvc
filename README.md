# NetCDF header extraction microservice for iRODS

The microservice
-----

-----



## System requirements 

The devel packages for the following support libraries should be installed
before building and installing the microservice:

   - netcdf
   - hdf5
   - zlib

## Demonstration of usage on CentOS 7

Enable the [EPEL](https://fedoraproject.org/wiki/EPEL) repo
before installing the above mentioned libraries.


   - copy `src/reg_with_metadata.r` to /tmp
   - as irods service account make sure we're in /tmp and download an example NetCDF binary data file
     ```
     myuser@host:~$ su - irods
     irods@host:~$ cd /tmp
     irods@host:/tmp$ wget https://www.unidata.ucar.edu/software/netcdf/examples/test_hgroups.nc`
     irods@host:/tmp$ irule -F reg_with_metadata.r "*phyFile='$(pwd)/test_hgroups.nc'"
     ```
