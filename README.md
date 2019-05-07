# NetCDF header extraction microservice for iRODS


## Running the microservice demo script
----

### System requirements 

The devel packages for the following support libraries should be installed
before building and installing the microservice:

   - netcdf-devel
   - hdf5-devel
   - zlib-devel

### Demonstration of usage on CentOS 7

   - Enable the [EPEL](https://fedoraproject.org/wiki/EPEL) repo
     and install the above mentioned libraries
   - copy `src/reg_with_metadata.r` to /tmp
   - as irods service account make sure we're in /tmp and download an example NetCDF binary data file
     ```
     myuser@host:~$ su - irods
     irods@host:~$ cd /tmp
     irods@host:/tmp$ wget https://www.unidata.ucar.edu/software/netcdf/examples/test_hgroups.nc`
     irods@host:/tmp$ irule -F reg_with_metadata.r "*phyFile='$(pwd)/test_hgroups.nc'"
     ```
