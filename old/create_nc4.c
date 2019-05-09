#include <netcdf.h>
#include <stdio.h>
main()
{
  int ncid;
  if (NC_NOERR == nc_create("teststch.nc", NC_NETCDF4|NC_CLOBBER, &ncid)) {
    int status = nc_put_att_text (ncid, NC_GLOBAL, "mychar", 5, "abcde");

    printf("tx %d\n",status);

    const char* strings [] =  {"abcde","another","world","party"};
    status = nc_put_att_string (ncid, NC_GLOBAL, "mystring", 4,  strings);

    printf("sg %d\n",status);

    status = nc_close(ncid);

    printf("cl %d\n",status);
  }
  return 0;
}
