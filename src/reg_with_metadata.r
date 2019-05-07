main()
{
    test()
}

get_elem(*L,*i,*default) 
{
 *el = *default
  *sz = size(*L)
  if (*i < 0) {*i = *i + size( *L) }
  if (*i < *sz) { *el = elem( *L, *i ) }
  *el
}

test() 
{
    *targetColl = "/$rodsZoneClient/home/$userNameClient"
    *splitPath = split(*phyFile, "/")
    *fileBaseName = get_elem(*splitPath,-1,"")
    if (*fileBaseName != "") 
    {
        if (0==
            msiPhyPathReg ( "*targetColl/*fileBaseName",  "*resc", 
                               "*phyFile", "null",  *status)) 
        {
            writeLine("stdout","success registering file")
            if (0==msiextract_netcdf_header( *keyval_pairs , "*phyFile")) {
                msiAssociateKeyValuePairsToObj( *keyval_pairs, '*targetColl/*fileBaseName', "-d")
            }
            else {
                writeLine("stderr","failed to extract NetCDF header info as metadata for '*targetColl/*fileBaseName'")
            }
        }
        else {
            writeLine("stdout","msiPhyPath gave status  = *status"); }
    }

}

INPUT *phyFile=$"", *resc=$"demoResc"
OUTPUT ruleExecOut

