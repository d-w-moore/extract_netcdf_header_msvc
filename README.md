# NetCDF header extraction for iRODS

This is a simple microservice , callable from any iRODS rule base, which can read the header data 
from a NetCDF-formatted file in the filesystem. 

The hierarchical structure of the NetCDF formatted
data in the file (variable types, dimensions, etc.) is gathered into a KeyValuePair object as a list
of key-value pairs, which can then be associated to the file's data object once it is registered into iRODS.

The above use-case is codified in the demonstration script `src/reg_with_metadata.r`.



## Running the microservice demonstration script
----

### System requirements 

The following support libraries should be installed before building and installing the microservice:

   - netcdf-devel
   - hdf5-devel
   - zlib-devel

(As always when building iRODS software, make sure irods-devel is installed as well.)

### Demonstration of usage on CentOS 7

   - Enable the [EPEL](https://fedoraproject.org/wiki/EPEL) repo and install the above mentioned libraries.
     For example, in a `centos:7` docker container, we can do the following:
     ```
     # yum install wget
     # wget https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm
     # rpm -ivh epel-release-latest-7.noarch.rpm
     # yum install {netcdf,hdf5,zlib}-devel
     ```
   - install iRODS from packages.  In Docker, from basic centos:7, you can do this with the following script:
     ```
     #!/bin/bash

     db_server_start_and_ready_wait() 
     {
       su - postgres -c '/usr/bin/pg_ctl -D /var/lib/pgsql/data -l logfile start'
       while ! su postgres -c "psql -c '\l'" >/dev/null 2>&1
       do
	 sleep 1
       done 
     }

     yum install -y https://dl.fedoraproject.org/pub/epel/epel-release-latest-7.noarch.rpm 
     yum install -y postgresql-server 

     su - postgres -c '/usr/bin/pg_ctl initdb ' 
     su - postgres -c 'sed -i.orig "s/^\(host.*\)trust$/\1md5/" /var/lib/pgsql/data/pg_hba.conf'

     db_server_start_and_ready_wait;

     cat >/tmp/db_commands.txt <<'EOF'
     CREATE DATABASE "ICAT";
     CREATE USER irods WITH PASSWORD 'testpassword';
     GRANT ALL PRIVILEGES ON DATABASE "ICAT" to irods;
     \q
     EOF

     su - postgres -c 'psql -f /tmp/db_commands.txt'
     yum install -y wget
     rpm --import https://packages.irods.org/irods-signing-key.asc
     wget -qO - https://packages.irods.org/renci-irods.yum.repo | tee /etc/yum.repos.d/renci-irods.yum.repo
     yum install -y irods-server irods-database-plugin-postgres
     python /var/lib/irods/scripts/setup_irods.py   < /var/lib/irods/packaging/localhost_setup_postgres.input
     ```

   - cd into the repo top level directory, then `mkdir build; cd build`
   - Build the microservice as a shared object. (May also need to change irods version number in the find_package directive in 
     CMakeLists.txt)
     ```
     cmake ../  && make && sudo install lib*.so /usr/lib/irods/plugins/microservices
     ```
   - copy `src/reg_with_metadata.r` to /tmp
   - as irods service account make sure we're in /tmp and download an example NetCDF binary data file
     ```
     myuser@host:~$ su - irods
     irods@host:~$ cd /tmp
     irods@host:/tmp$ wget https://www.unidata.ucar.edu/software/netcdf/examples/test_hgroups.nc
     irods@host:/tmp$ irule -F reg_with_metadata.r "*phyFile='$(pwd)/test_hgroups.nc'"
     irods@host:/tmp$ imeta ls -d "~/test_hgroups.nc"
     ```
