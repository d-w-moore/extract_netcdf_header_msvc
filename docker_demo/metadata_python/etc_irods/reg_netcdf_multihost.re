
pep_api_phy_path_reg_post( *INST,*CONTEXT, *DOBJ) {
  *r = *DOBJ.destRescName
  *f = *DOBJ.filePath
  *l = *DOBJ.obj_path
# *h = *DOBJ.resc_hier
  *loc = ""
  foreach (*host in select RESC_LOC where DATA_RESC_NAME = '*r' and DATA_PATH = '*f') 
  {
    *loc = *host.RESC_LOC
  }
  writeLine ("serverLog", "host = [*loc]")
}
