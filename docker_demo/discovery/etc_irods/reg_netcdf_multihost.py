from genquery import AS_LIST,AS_DICT,row_iterator

p=lambda cbk:\
  lambda s: cbk.writeLine('serverLog',s)

def pep_api_phy_path_reg_post(rule_args,callback,rei):
    pr = p(callback)
    pr( str(rule_args) )
    pr("REG ** post ** {!r}".format(rule_args))
    # - get file path and resc name for potentially remote netcdf extraction
    phyPath = ""
    rescName = ""
    cndI = rule_args[2].condInput
    for i in range(cndI.len):
      if   cndI.key[i] == 'filePath': phyPath = cnd.value[i]
      elif cndI.key[i] == 'destRescName': rescName = cnd.value[i]
    
    lgPath = str(rule_args[2].objPath)

    lgPath_host = ""

    for x in row_iterator('RESC_LOC' , "RESC_NAME = '{}'".format(rescName), AS_LIST, callback):
      logPath_host= x[0]

