test() {

      *Param1 = "string"
      msiextract_netcdf_header( *Param1, *Param2 )
      writeLine("stdout", *Param1)
      
}

INPUT  null
OUTPUT ruleExecOut
