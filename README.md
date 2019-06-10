
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

   * To submit sample data to the icat, run the following script (also requires docker):

      - run `./extract_netcdf_header_msvc/example/create_example_data`

      - this should create a number of .nc files in `.../example/data`

      - navigate here in one of the browser clients and upload some or all of the example files

      - queries may be made in the GUI or using the `iquest` command line:

        ```
        iquest '%s/%s  NAME %s   VALUE %s' "select COLL_NAME,DATA_NAME, META_DATA_ATTR_NAME, META_DATA_ATTR_VALUE where META_DATA_ATTR_NAME = 'irods::netcdf' and META_DATA_ATTR_VALUE like '%ATTR;_value=%South%' "
        ```
