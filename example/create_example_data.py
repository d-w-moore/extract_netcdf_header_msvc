#!/usr/bin/env python

from __future__ import print_function

from netCDF4 import Dataset
from numpy import arange, dtype
import numpy as np
import sys

region_names = \
         {   "STHL" : "Southland",
             "CTBY" : "Canterbury",
             "MRLB" : "Marlborough",
         }

lat_rg = {   "STHL": (-46.1,-45.7),
             "CTBY": (-43.8,-43.1),
             "MRLB": (-41.8,-41.5),
         }

lon_rg = {   "STHL" : (167.5,168.8),
             "CTBY" : (171.0,172.2),
             "MRLB" : (173.5,173.9),
        }

def sample_X_Y (lt_points,lg_points,lat):
  n = np.ndarray ((lt_points,lg_points),np.dtype('float32').char)
  for a in range(n.shape[0]):
    for b in range(n.shape[1]):
       n[a,b] = np.random.uniform()*2 + 63 - abs(lat)
  return n

lat_step = 0.1
lon_step = 0.2

lat_lon_incr = 0.02

Lt = 5; Lg = 10 

if __name__ == '__main__':

  import os
  print('--> creating example files in {}'.format(os.getcwd()))

  for region_code in region_names.keys():

    for lat_alpha in 'a','b':

      for lon_alpha  in 'n','o':

        lat_anchor = (ord(lat_alpha) - ord('a')) * lat_step + lat_rg[region_code][0]
        lon_anchor = (ord(lon_alpha) - ord('n')) * lon_step + lon_rg[region_code][0]

        ncfile = Dataset("{}_{}_{}.nc".format(region_code,lat_alpha,lon_alpha),'w') 

        ncfile.createDimension('lat',Lt)
        ncfile.createDimension('lon',Lg)

        ncfile.latitude_anchor  =  lat_anchor
        ncfile.longitude_anchor =  lon_anchor
        ncfile.region_name  = region_names [region_code]

        data = ncfile.createVariable('data',dtype('float32').char,('lat','lon'))

        data[:] =  sample_X_Y(Lt, Lg, lat_anchor)

        ncfile.close()

