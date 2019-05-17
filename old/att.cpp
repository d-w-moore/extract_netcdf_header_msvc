/* This is part of the netCDF package.
   Copyright 2006 University Corporation for Atmospheric Research/Unidata.
   See COPYRIGHT file for conditions of use.

   This is a very simple example which writes a 2D array of
   sample data. To handle this in netCDF we create two shared
   dimensions, "x" and "y", and a netCDF variable, called "data".

   This example is part of the netCDF tutorial:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-tutorial

   Full documentation of the netCDF C++ API can be found at:
   http://www.unidata.ucar.edu/software/netcdf/docs/netcdf-cxx

   $Id: simple_xy_wr.cpp,v 1.14 2006/08/18 18:57:30 russ Exp $
*/

#include <iostream>
#include <netcdfcpp.h>

using namespace std;
using cstring = const char *;

// We are writing 2D data, a 6 x 12 grid. 
static const int NDIMS = 2;
static const int NX = 6;
static const int NY = 12;

// Return this in event of a problem.
static const int NC_ERR = 2;

int 
main(void)
{
   // This is the data array we will write. It will just be filled
   // with a progression of numbers for this example.
   int dataOut[NX][NY];
  
   // Create some pretend data. If this wasn't an example program, we
   // would have some real data to write, for example, model output.
   for(int i = 0; i < NX; i++) 
      for(int j = 0; j < NY; j++)
	 dataOut[i][j] = i * NY + j;

   // Create the file. The Replace parameter tells netCDF to overwrite
   // this file, if it already exists.
   NcFile dataFile("simple_xy.nc", NcFile::Replace);

   // You should always check whether a netCDF file creation or open
   // constructor succeeded.
   if (!dataFile.is_valid())
   {
      cout << "Couldn't open file!\n";
      return NC_ERR;
   }
  
   // For other method calls, the default behavior of the C++ API is
   // to exit with a message if there is an error.  If that behavior
   // is OK, there is no need to check return values in simple cases
   // like the following.

   // When we create netCDF dimensions, we get back a pointer to an
   // NcDim for each one.
   NcDim* xDim = dataFile.add_dim("x", NX);
   NcDim* yDim = dataFile.add_dim("y", NY);

   NcDim* Dim1 = dataFile.add_dim("y1", NY);
   NcDim* Dim2 = dataFile.add_dim("y2", 1);
   NcDim* Dim3 = dataFile.add_dim("y3", 2);
   NcDim* Dim4 = dataFile.add_dim("y4", 1);
   NcDim* Dim5 = dataFile.add_dim("y5", 1);
   NcDim* Dim6 = dataFile.add_dim("y6", 1);
   NcDim* Dim7 = dataFile.add_dim("y7", 1);
   NcDim* Dim8 = dataFile.add_dim("y8", 1);
   NcDim* Dim9 = dataFile.add_dim("y9", 1);
   NcDim* DimA = dataFile.add_dim("ya", 2);

   dataFile.add_att( "hello_float", 1, "whatsnew");
   //dataFile.add_att( "hello_float", 2, const_cast<const char **>( new cstring [2]{ "helo", "godbye" }));
   dataFile.add_att( "hello_float", 3, new float [3]{3.f,4.f,5.f} );
   dataFile.add_att( "hello_float", 3, new double [3]{3.,4.,5.} );

   ncbyte cstgring[] = { 'a','w','o','r','d' };
// const char* cstgrings[] = { "here", "are", "four", "words"};
// dataFile.add_att( "hello_string", 4, cstgrings );
  
   // Define a netCDF variable. The type of the variable in this case
   // is ncInt (32-bit integer).
   NcVar *data = dataFile.add_var("data", ncInt, xDim, yDim);

#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

   const NcDim* dims[] = { xDim,yDim,Dim1,Dim2,Dim3,Dim4,
                    Dim5,Dim6,Dim7,Dim8,Dim9,DimA  };

   NcVar *data2 = dataFile.add_var("data2", ncInt, ARRAY_SIZE(dims),dims);
     
   // Write the pretend data to the file. Although netCDF supports
   // reading and writing subsets of data, in this case we write all
   // the data in one operation.
   data->put(&dataOut[0][0], NX, NY);

   // The file will be automatically close when the NcFile object goes
   // out of scope. This frees up any internal netCDF resources
   // associated with the file, and flushes any buffers.
   cout << "*** SUCCESS writing example file simple_xy.nc!" << endl;

   return 0;
}
