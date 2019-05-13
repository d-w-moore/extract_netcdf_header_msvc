#ifndef STRING_FUNCTIONS_HPP
#define STRING_FUNCTIONS_HPP

#include <vector>
#include <string>
#include <boost/lexical_cast.hpp>

template <typename T>
  std::string
  Stringize( T inp ) {
    std::string retval = "()";
    try {
      retval = boost::lexical_cast<std::string>(inp);
    }
    catch(boost::bad_lexical_cast&  e) { retval = "<no_serializer>"; }
    return retval;
  }

template <>
  std::string
  Stringize( const char* inp ) { return  std::string("\042")  + inp  + std::string("\042"); }

template <>
  std::string
  Stringize( std::string inp ) { return  std::string("\042")  + inp  + std::string("\042"); }

template <typename T>
std::string Join(const std::vector<T>& v, char c=',') 
{
   std::string s="[";
   for (auto p = v.begin(); p != v.end(); ++p)
   {
      s += Stringize(*p);
      if (p != v.end() - 1)
        s += c;
   }
   s += "]";
   return s;
}

// - miscellaneous function to fill a vector of any type from C-style memory array

template <typename T>
void Fill_vector (std::vector<T> & v_dest, const T*  ptr , int n)
{
    while (n-- > 0) {
        v_dest.push_back(*ptr++);
    }
}

template <typename T>
std::vector<T> Filled_vector (const T*  ptr , int n)
{
    std::vector<T> v;
    while (n-- > 0) {
        v.push_back(*ptr++);
    }
    return v;
}

#endif // STRING_FUNCTIONS_HPP
