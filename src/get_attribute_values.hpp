#ifndef GET_ATTRIBUTE_VALUES_HPP
#define GET_ATTRIBUTE_VALUES_HPP

#include <typeinfo>
#include <string>
#include <algorithm>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <netcdf.h>

template <typename T, typename U = T>
struct attribute_params {
  int ncid;
  int varid;
  std::string name;
  size_t attr_vec_len;
  T* p = 0;

  attribute_params (int i, int v,const std::string& s) : 
   ncid(i), varid(v), name(s.c_str())
  {
  }

  ~attribute_params() {
    free(p);
  }

private:

  void allocate_pointer_array() { p = static_cast<T*>(calloc(attr_vec_len+1, sizeof(T))); } //+1 is for '\0'-term (T = char)

  int get_values_ () //  -- the generic (non type-safe) version --
  {
      return nc_get_att (ncid, varid, name.c_str(), (void*) p);
  }

  const char *fmt () {return "%p|";} // useless, but safe, and helps detect unimplemented types
  int final_offset () {return -1;}
  int output_element_(int idx, char* buf, int &offset, int chremain);

public:

  /* our API , such as it is */

  int get_attribute_values()  
  {
      int status = nc_inq_attlen(ncid,varid,name.c_str(),&attr_vec_len);
      if (status == NC_NOERR) {
          allocate_pointer_array();
          //return nc_get_att(ncid,varid,name.c_str(),(void*)p);
          return get_values_ ();
      }
      else {
          return status;
      }
  }

  int dump(char *, int, bool = true ) ;

};

template <typename T, typename U>
int attribute_params<T,U>::output_element_(int idx, char* buf, int &offset, int chremain) 
{
    int headroom = chremain - 1 ;
    int i = headroom ;
    const char* const Format = fmt();
    const char* const is_pointer = std::strstr( Format, "%p");

    if (headroom > 0) { 
        if ( is_pointer == nullptr ) {
            i = snprintf(buf + offset, headroom, Format, static_cast<U>(p[idx]));
        }
        else {
            i = snprintf(buf + offset, headroom, Format, p[idx] );
        }
    }

    if  (i >= headroom) {
        buf [offset + headroom] = '\0'; // cap snprintf's truncated output
        return 0;
    }
    offset += std::min(i,headroom);
    return 1;
}

template <typename T, typename U>
int attribute_params<T,U>::dump(char *buf, int bufsize, bool use_ellipsis) 
{
    const char* Ellipsis = "...";

    int easyfit = 1, sz = 0;
    if (bufsize <= 0) { return 0; } // mm'k

    buf[0] = '\0'; // in case attr_vec_len is zero

    for (int x=0, atr_count = attr_vec_len ; --atr_count >= 0 ; ++x) 
    {
        easyfit = output_element_( x, buf, sz, bufsize - sz );

        if (easyfit) { // still within buffer limit

            int last_delimiter_offset = sz + final_offset();

            if (atr_count == 0 && last_delimiter_offset >= 0)  {
                buf[last_delimiter_offset] = '\0';   // ERASE last delimiter
            }
        }
        else { // ran against buffer limit

            int ellipsis_len = std::strlen(Ellipsis);

            if (sz >= ellipsis_len && use_ellipsis) 
            {
                std::strcpy( buf + sz - ellipsis_len, Ellipsis );
            }
            else {  buf [sz] = '\0'; }
        }
        if (easyfit == 0) { break; }
    }
    return easyfit;
}

template <>
attribute_params<char*>::~attribute_params ()
{
  // -- documented in the NetCDF API as a special case
  if (p) {
    nc_free_string( attr_vec_len, p);
    free(p);
  }
}

typedef unsigned int UINT;
typedef unsigned char UCHAR;

template<> int attribute_params<char,char>::final_offset () {return 0;}    // NC_CHAR

template<> int attribute_params<char,int>::final_offset () {return -1;}    // NC_BYTE
template<> int attribute_params<UCHAR,UINT>::final_offset () {return -1;}  // NC_UBYTE

template<> const char *attribute_params<char,char>::fmt          () {return "%c";}
template<> const char *attribute_params<char,int>::fmt           () {return "%d|";} // NC_BYTE
template<> const char *attribute_params<UCHAR,UINT>::fmt         () {return "%u|";} // NC_UBYTE

template<> const char *attribute_params<char*>::fmt              () {return "%s|";}

template<> const char *attribute_params<unsigned>::fmt           () {return "%u|";}
template<> const char *attribute_params<int>::fmt                () {return "%d|";}
template<> const char *attribute_params<short>::fmt              () {return "%hd|";}
template<> const char *attribute_params<unsigned short>::fmt     () {return "%hu|";}
//template<> const char *attribute_params<unsigned long>::fmt      () {return "%lu|";}
template<> const char *attribute_params<long>::fmt               () {return "%ld|";}
template<> const char *attribute_params<unsigned long long>::fmt () {return "%llu|";}
template<> const char *attribute_params<long long>::fmt          () {return "%lld|";}
template<> const char *attribute_params<float>::fmt              () {return "%g|";}
template<> const char *attribute_params<double>::fmt             () {return "%lg|";}
template<> const char *attribute_params<long double>::fmt        () {return "%Lg|";}

#define specialized_attribute_getter(TYPE,Suffix)		\
template<>							\
int attribute_params<TYPE>::get_values_() {			\
  return ATT_GET_FN(Suffix) (ncid, varid, name.c_str(), p); 	\
}

#undef ATT_GET_FN
#undef concat2args

#define concat2args(x,y) x ## y
#define ATT_GET_FN(func_type_suffix) concat2args(nc_get_att_ , func_type_suffix)

// partial specialization for each attribute type supported

#define outputtype__ ,

specialized_attribute_getter( double, double )
specialized_attribute_getter( float, float )
specialized_attribute_getter( short, short )
specialized_attribute_getter( unsigned short, ushort )
specialized_attribute_getter( int, int )
specialized_attribute_getter( long, long )
specialized_attribute_getter( long long , longlong )
specialized_attribute_getter( unsigned int, uint )
specialized_attribute_getter( unsigned long long, ulonglong )
specialized_attribute_getter( char*, string )
specialized_attribute_getter( signed char, schar )
specialized_attribute_getter( unsigned char, uchar )
specialized_attribute_getter( signed char outputtype__ int , schar )  // NC_BYTE
specialized_attribute_getter( UCHAR outputtype__ UINT, uchar )        // NC_UBYTE

#undef specialized_attribute_getter
#undef outputtype__ 

/*
 *   NETCDF C library's native, type-safe attribute "getters":
 *   
     int nc_get_att_text      (int ncid, int varid, const char *name, char *tp);
     int nc_get_att_uchar     (int ncid, int varid, const char *name, unsigned char *up);
     int nc_get_att_schar     (int ncid, int varid, const char *name, signed char *cp);
     int nc_get_att_short     (int ncid, int varid, const char *name, short *sp);
     int nc_get_att_int       (int ncid, int varid, const char *name, int *ip);
     int nc_get_att_long      (int ncid, int varid, const char *name, long *lp);
     int nc_get_att_float     (int ncid, int varid, const char *name, float *fp);
     int nc_get_att_double    (int ncid, int varid, const char *name, double *dp);
     int nc_get_att_ubyte     (int ncid, int varid, const char *name, unsigned char *ip);
     int nc_get_att_ushort    (int ncid, int varid, const char *name, unsigned short *ip);
     int nc_get_att_uint      (int ncid, int varid, const char *name, unsigned int *ip);
     int nc_get_att_longlong  (int ncid, int varid, const char *name, long long *ip);
     int nc_get_att_ulonglong (int ncid, int varid, const char *name, unsigned long long *ip);
     int nc_get_att           (int ncid, int varid, const char *name, void *ip);

     // - extra free call required when disposing of char** pointer list (see API docs):

     int nc_get_att_string    (int ncid, int varid, const char *name, char **ip);

 *
 *   NETCDF C library's native types:
 *

 #define NC_BYTE         1       
 #define NC_CHAR         2       
 #define NC_SHORT        3       
 #define NC_INT          4       
 #define NC_LONG         NC_INT  
 #define NC_FLOAT        5       
 #define NC_DOUBLE       6       
 #define NC_UBYTE        7       
 #define NC_USHORT       8       
 #define NC_UINT         9       
 #define NC_INT64        10      
 #define NC_UINT64       11      
 #define NC_STRING       12  
 ***/

#undef ATT_GET_FN
#undef concat2args

#endif // GET_ATTRIBUTE_VALUES_HPP
