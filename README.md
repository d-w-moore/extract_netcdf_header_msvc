
# NetCDF header extraction microservice for iRODS


## General description

This repository contains a microservice for pulling the relevant descriptive metadata from files
as they are placed under management by iRODS.

This microservice

   * can be called from any rule file or rule-base, allowing both manual and policy-driven use.

   * is self-contained and located in the the docker_demo/extractor_Msvc

   * at this point, targets CentOS 7

   * may be demonstrated  from both GUI and command-line query viewpoints by running the docker-compose demo, also included in this repo.


---

## To run the demonstration:
 
   * install docker-compose

   * clone this repository

   * cd into ./extract_netcdf_header_msvc/docker_demo and run:

     ```
     docker-compose up
     ```

     at a terminal command line.


   * the demo may take a good half-minute to come up; after the start-up is done, one can log in to
     the Metalnx and Cloudbrowser graphical interfaces by bringing up a browser on the host machine and
     using these URL's respectively:

      - http://localhost:8080/metalnx

      - http://localhost:9090/irods-cloud-backend


     The login is rods/rods

   * `docker exec -it docker_demo_irodsprov_{XX_YY} bash` where XX and YY will most likely be instance numbers or perhaps a partial hexadecimal hash, depending on the  version of docker-compose being used . Once inside the provider node, place  a bit of policy into  to that node's `/etc/irods/core.re` which will cause NetCDF header to be entered into the ICAT database after the successful 'put' of any  file with an extension of `.nc`
      - open up a separate terminal and open up `/home/dmoore/extract_netcdf_header_msvc/docker_demo/post_put.core.re`
      - in the iRODS provider's root shell, enter : `su - irods` and use a text editor to append the contents of that file to the system's `/etc/irods/core.re`
      
   * To generate submit sample data containing NetCDF headers to be scanned into the iRODS metadata catalog:

      - run the script : `./extract_netcdf_header_msvc/example/create_example_data`  (also requires docker be installed)

      - this should create a number of .nc files in the `data` subdirectory located in the same directory as the script.

      - navigate to these data files when prompted by Cloudbrowser or Metalnx during an upload ("PUT") operation.

      - both browser clients allow inspection of the iRODS metadata  attached to the \*.nc files. This appears as a set of multiple NAME/VALUE combinations with a NAME value of `irods::netcdf` and a VALUE which describes the hierarchical path and value of an individual NetCDF entity ( dimension,variable,attribute,etc.)  Each NetCDF entity thus described may have several "structure members" enumerated. ( eg. \_value, name, length)
      - queries may be made either in the GUI or by using an approprate argument on the `iquest` command line:

        ```
        iquest '%s/%s  NAME %s   VALUE %s' "select COLL_NAME,DATA_NAME, META_DATA_ATTR_NAME, META_DATA_ATTR_VALUE where META_DATA_ATTR_NAME = 'irods::netcdf' and META_DATA_ATTR_VALUE like '%ATTR;_value=%South%' "
        ```
