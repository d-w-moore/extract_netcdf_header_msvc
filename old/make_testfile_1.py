#!/usr/bin/env python

from __future__ import print_function

import netCDF4
import numpy as np

root_group = netCDF4.Dataset("testfile1.nc","w","NETCDF4")

root_group.createDimension( "dim1" ,  3  )
root_group.createDimension( "dim2" ,  4  )
#root_group.createAttribute

xf32 = root_group.createVariable('xf32',np.float32, ('dim1','dim2'))
xf64 = root_group.createVariable('xf64',np.float64, ('dim2',))

#group_a =   root_group.createGroup("hello0")
#group_b =   root_group.createGroup("hello0")

root_group.close()
