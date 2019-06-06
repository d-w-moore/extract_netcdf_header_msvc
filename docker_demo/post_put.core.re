acPostProcForPut 
{
    writeLine("serverLog", "INFO *** data object [$objPath] has been PUT ; --> [$filePath] is physical path")
    if ($objPath like "*.nc") {

        if (0==msiextract_netcdf_header( *kv_pairs , "$filePath")) {
            writeLine("serverLog", "NETCDF header written:\n\t FROM physical [$filePath]\n\t TO logical [$objPath]")
        
            foreach ( *key in *kv_pairs )
            {
                *value = *kv_pairs.*key
                *pair."irods::netcdf" = "*key=*value"
                msiAssociateKeyValuePairsToObj( *pair, "$objPath", "-d")
            }
        }
    }
}


