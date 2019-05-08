#ifndef GET_ATTRIBUTE_VALUES_HPP
#define GET_ATTRIBUTE_VALUES_HPP

#include <typeinfo>
#include <string>
#include <iostream>
#include <stdlib.h>
#include <netcdf.h>

template <typename T>
struct attribute_params {
  int ncid;
  int varid;
  std::string name;
  size_t dimension;
  T*p = 0;

  attribute_params (int i, int v,const std::string& s) : 
   ncid(i), varid(v), name(s.c_str())
  {
  }

  int get_attribute_values() 
  {
    nc_inq_attlen(ncid,varid,name.c_str(),&dimension);
    allocate_pointer_array();
    return nc_get_att(ncid,varid,name.c_str(),(void*)p);
  }
  
  ~attribute_params() {
    //std::cout << typeid(p).name() << " : in the original\n";
    free(p);
  }
private:
  void allocate_pointer_array() { p = static_cast<T*>(calloc(dimension, sizeof(T))); }

};


template <>
attribute_params<char*>::~attribute_params ()
{
  //std::cout << typeid(p).name() << " : in the special case\n";
  if (p) {
    nc_free_string( dimension, p);
    free(p);
  }
}

#endif // GET_ATTRIBUTE_VALUES_HPP
