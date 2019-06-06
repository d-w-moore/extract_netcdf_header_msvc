#include <ncFile.h>

using namespace netCDF;

main()
{

  NcFile n("../example1.nc", NcFile::write);
  const char* c [] = {"a","b","c","d","e"};
  n.putAtt("mymulti",5,c);

}
