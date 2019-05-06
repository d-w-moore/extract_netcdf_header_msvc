if [ "$(dirname $0)" != ".." ]
then
    echo >&2 "Please execute $0 from a build subdirectory" ; exit 1
fi

if [ "$1" = clean ]
then
    rm -f *.o a.out
else
  {
    /opt/irods-externals/clang3.8-0/bin/clang++   -DBOOST_SYSTEM_NO_DEPRECATED \
    -DENABLE_RE -DRODS_SERVER -D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE   \
    -D_LARGEFILE_SOURCE -D_LARGE_FILES -Dlinux_platform                      \
    -Dirods_build=0 \
    -Dmsiextract_netcdf_header_EXPORTS -I/opt/irods-externals/clang3.8-0/include/c++/v1 \
    -I/usr/include/irods -I/opt/irods-externals/qpid-with-proton0.34-0/include \
    -I/opt/irods-externals/boost1.60.0-0/include -I/opt/irods-externals/jansson2.7-0/include \
    -I/opt/irods-externals/libarchive3.3.2-0/include  -O3 -DNDEBUG -nostdinc++ \
    -std=c++14 -Wno-write-strings -std=gnu++14 \
    -o lib-microservice-extract_netcdf_header.cpp.o \
    -c ../src/lib-microservice-extract_netcdf_header.cpp -g

# -fPIC
  } && \
       \
  {

LD_RUN=/opt/irods-externals/clang-runtime3.8-0/lib

    /opt/irods-externals/clang3.8-0/bin/clang++ -O3 -DNDEBUG  -stdlib=libc++ \
    lib-microservice-extract_netcdf_header.cpp.o \
    -lc++abi -lnetcdf -lhdf5 -lz /opt/irods-externals/boost1.60.0-0/lib/libboost_filesystem.so \
    /opt/irods-externals/boost1.60.0-0/lib/libboost_system.so \
    /opt/irods-externals/libarchive3.3.2-0/lib/libarchive.so /usr/lib64/libcrypto.so \
    -Wl,-rpath,/opt/irods-externals/boost1.60.0-0/lib:/opt/irods-externals/libarchive3.3.2-0/lib:$LD_RUN -g
  }

fi

# -shared
## CMakeFiles/msiextract_netcdf_header.dir/src/lib-microservice-extract_netcdf_header.cpp.o \
