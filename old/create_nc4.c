#include <netcdf.h>
#include <stdlib.h>
#include <stdio.h>
main(int argc, char *argv[])
{
    int ncid;
    if(argc < 2) {
	fprintf(stderr,"usage: %s to_be_created.nc\n",argv[0]);
	exit(1);
    }

/*  for reference only
 *
     int nc_put_att_text      (int ncid, int varid, const char *name,
                               size_t len, const char *tp);
     int nc_put_att_uchar     (int ncid, int varid, const char *name, nc_type xtype,
                               size_t len, const unsigned char *up);
     int nc_put_att_schar     (int ncid, int varid, const char *name, nc_type xtype,
                               size_t len, const signed char *cp);
     int nc_put_att_short     (int ncid, int varid, const char *name, nc_type xtype,
                               size_t len, const short *sp);
     int nc_put_att_int       (int ncid, int varid, const char *name, nc_type xtype,
                               size_t len, const int *ip);
     int nc_put_att_long      (int ncid, int varid, const char *name, nc_type xtype,
                               size_t len, const long *lp);
     int nc_put_att_float     (int ncid, int varid, const char *name, nc_type xtype,
                               size_t len, const float *fp);
     int nc_put_att_double    (int ncid, int varid, const char *name, nc_type xtype,
                               size_t len, const double *dp);
     int nc_put_att_ubyte     (int ncid, int varid, const char *name, nc_type xtype,
                               size_t len, const unsigned char *op);
     int nc_put_att_ushort    (int ncid, int varid, const char *name, nc_type xtype,
                               size_t len, const unsigned short *op);
     int nc_put_att_uint      (int ncid, int varid, const char *name, nc_type xtype,
                               size_t len, const unsigned int *op);
     int nc_put_att_longlong  (int ncid, int varid, const char *name, nc_type xtype,
                               size_t len, const long long *op);
     int nc_put_att_ulonglong (int ncid, int varid, const char *name, nc_type xtype,
                               size_t len,
                               const unsigned long long *op);
     int nc_put_att_string    (int ncid, int varid, const char *name, size_t len,
                               const char **op);
     int nc_put_att           (int ncid, int varid, const char *name, nc_type xtype,
                               size_t len, const void *op);
 *
 */

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof((arr[0])))
#define ERRCHK(quoted,expr) \
    do { int y; \
        if ( (y=(expr)) != NC_NOERR) { \
           fprintf(stderr, "error %d - " quoted "\n", y); exit(1); \
        } \
    } while(0)

    if (NC_NOERR == nc_create(argv[1], NC_NETCDF4|NC_CLOBBER, &ncid)) {

	ERRCHK("text", nc_put_att_text (ncid, NC_GLOBAL, "mychar", 5, "abcde"));

	const char* strings [] =  {"visit","another","world","party"};
        ERRCHK("string", nc_put_att_string (ncid, NC_GLOBAL, "mystring", 4,  strings));

        const  signed  char schars[] = { 's', 'c', 'h', 'a', 'r', 's', 0, '*' };
	ERRCHK("signed char", nc_put_att_schar (ncid, NC_GLOBAL, "signed_chars", NC_BYTE, ARRAY_SIZE(schars), schars));

        const unsigned char ubytes[] = { 'u', 'b', 'y', 't', 'e', 's', -1 };
	ERRCHK("unsigned byte", nc_put_att_ubyte (ncid, NC_GLOBAL, "unsigned_bytes", NC_UBYTE, ARRAY_SIZE(ubytes), ubytes));

        const unsigned short ushorts[] = { 0xfeed, 0xA, 0xbeef };
	ERRCHK("unsigned short", nc_put_att_ushort (ncid, NC_GLOBAL, "unsigned_shorts", NC_USHORT, ARRAY_SIZE(ushorts), ushorts));

        const unsigned short shorts[] = { 0xfeed, 0xA, 0xbeef };
	ERRCHK("signed short", nc_put_att_short (ncid, NC_GLOBAL, "signed_shorts", NC_SHORT, ARRAY_SIZE(shorts), shorts));

	ERRCHK("close file", nc_close(ncid));
    }
    return 0;
}
