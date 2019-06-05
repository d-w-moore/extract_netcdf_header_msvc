#!/usr/bin/env python2

from netCDF4 import *

d = Dataset ("/tmp/example1.nc", "w")

gp_aa = d.createGroup("aa")
gp_cc = d.createGroup("cc")
gp_aa_bb = gp_aa.createGroup('bb')

d.createDimension('e',6)
gp_aa.createDimension('o',4)
gp_aa_bb.createDimension('n',5)

gp_aa_bb.createVariable ('var1',float,('o','n','e'))

d.close()

