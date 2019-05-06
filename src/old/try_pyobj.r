def main(rule_args, callback, rei):

      print_ = lambda x:Print_(x,callback)

      types   = ("str", #0
                 "irods_types.genQueryOut_t",  #1
                 "irods_types.genQueryInp_t",  #2
                 "irods_types.keyValPair_t",   #3
                 "irods_types.fileLseekOut_t", #4
                 "irods_types.BytesBuf",       #5
                 "irods_types.ExecCmdOut",     #6
                 "irods_types.RodsObjStat",    #7
                 "int",                        #8
                 "float",                      #9
      )

      which = global_vars['*which'][1:-1]

      bObj = None

      try:
          b = types[ int(which) ]
      except:
          b = None

      print_('instantiating '+ repr(b))
      if b is not None:
          try:
              bObj = eval(b + "()")
          except:
              print_('expression error')

      if bObj is not None:
          rv = callback.msiextract_netcdf_header( "mystring", bObj)
          retval = rv['arguments'][0] 
          print_( 'retval = ' + repr(retval))

def Print_(s, cbk, strm='stdout'): cbk.writeLine(strm, s)

INPUT  *which=$''
OUTPUT ruleExecOut
