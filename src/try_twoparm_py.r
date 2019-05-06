def main(*a): test(*a)

def P_(callback , to_print  ,strm='stdout'):
    callback.writeLine(strm, to_print)

def test(rule_args, callback, rei):

      print_ = lambda s: P_ (callback,s)

      types   = ("str",                       #0
                 "irods_types.GenQueryOut", #1
                 "irods_types.GenQueryInp", #2
                 "irods_types.KeyValPair",    #3
                 "irods_types.FileLseekOut_t", #4
                 "irods_types.BytesBuf",      #5
                 "irods_types.ExecCmdOut",    #6
                 "irods_types.RodsObjStat",   #7
                 "int",                       #8
                 "float",                     #9
      )

      which = global_vars['*which'][1:-1]

      try:
          b = types[ int(which) ]
      except:
          print_('bad type option in "which" - '+ repr(b))
          return

      print_('instantiating '+ repr(b))

      try:
          bObj = eval(b + "()")
      except:
          print_('expression error')
          return

      rv = callback.msiextract_netcdf_header( "mystring", bObj)
      retval = rv['arguments'][0] 
      print_( 'retval = ' + repr(retval))


INPUT  *which=$''
OUTPUT ruleExecOut
