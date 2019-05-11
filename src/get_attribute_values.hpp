#ifndef GET_ATTRIBUTE_VALUES_HPP
#define GET_ATTRIBUTE_VALUES_HPP

#include <typeinfo>
#include <string>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <netcdf.h>

template <typename T>
struct attribute_params {
  int ncid;
  int varid;
  std::string name;
  size_t dimension;
  T* p = 0;

  attribute_params (int i, int v,const std::string& s) : 
   ncid(i), varid(v), name(s.c_str())
  {
  }

  ~attribute_params() {
    free(p);
  }

private:

  void allocate_pointer_array() { p = static_cast<T*>(calloc(dimension, sizeof(T))); }

  int get_values_ () //  -- the generic (non type-safe) version --
  {
      return nc_get_att (ncid, varid, name.c_str(), (void*) p);
  }

  const char *fmt () {return "%p|";} //useless, but safe, and detects unimplemented types
  int final_offset () {return -1;}
  int output_element_(int idx, char* buf, int &offset, int chremain);

public:

  /* our API , such as it is */

  int get_attribute_values()  
  {
      int status = nc_inq_attlen(ncid,varid,name.c_str(),&dimension);
      if (status == NC_NOERR) {
          allocate_pointer_array();
          //return nc_get_att(ncid,varid,name.c_str(),(void*)p);
          return get_values_ ();
      }
      else {
          return status;
      }
  }

  int dump(char *, int ) ;

};

template <typename T>
int attribute_params<T>::output_element_(int idx, char* buf, int &offset, int chremain) 
{
    int headroom = chremain - 1 + final_offset();
    int i = headroom ;
    if (headroom > 0) { 
	i = snprintf(buf + offset, headroom ,fmt(), p[idx]);
    }
    if  (i >= headroom) {buf [offset + headroom] = '\0'; return 0;}
    offset += i;
    return 1;
}

template <typename T>
int attribute_params<T>::dump(char *buf, int bufsize) 
{
    int sz = 0;
    int x = 0;
    int easyfit = 1;
    if (bufsize > 0) { buf[bufsize] = '\0'; } // in case dimension is be zero
    while (x < dimension) {
        if (0 == (easyfit = output_element_( x++, buf, sz, bufsize - sz )))
        {
            break;   
        }
    }
    return easyfit;
}

template <>
attribute_params<char*>::~attribute_params ()
{
  // -- documented in the NetCDF API as a special case
  if (p) {
    nc_free_string( dimension, p);
    free(p);
  }
}

template<> int attribute_params<char>::final_offset () {return 0;}

template<> const char *attribute_params<char>::fmt               () {return "%c";}
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

#undef ATT_GET_FN
#undef concat2args

/*** compare:
     int nc_get_att_string    (int ncid, int varid, const char *name, char **ip);
  // int nc_get_att           (int ncid, int varid, const char *name, void *ip); 
  // int nc_get_att_text      (int ncid, int varid, const char *name, char *tp);
  //
  //-- look into these a bit more
     int nc_get_att_uchar     (int ncid, int varid, const char *name, unsigned char *up);
     int nc_get_att_ubyte     (int ncid, int varid, const char *name, unsigned char *ip);
     int nc_get_att_schar     (int ncid, int varid, const char *name, signed char *cp);
 ***/

#endif // GET_ATTRIBUTE_VALUES_HPP
