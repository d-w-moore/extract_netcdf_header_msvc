
from genquery import AS_LIST, AS_DICT, row_iterator
from pprint import pformat

def main(_arg,callback,rei):

    opn = global_vars['*Operation'][1:-1]
    collname = global_vars['*CollName'][1:-1]
    objname  = global_vars['*ObjName'][1:-1]
#   callback.writeLine("stdout",'********* {}.{}.{}'.format(opn,collname,objname ))

    targets = _list_data_objects( callback, collname, objname )

    callback.writeLine("stdout",'********* {targets!r}'.format(**locals()))

    if opn == 'Erase':
        
        for x in targets : _erase_meta(callback, x ) 

#=========================================================

def _list_data_objects( callback, coll_match, obj_match ):

  coll_condn =  "="
  obj_condn  =  "="

  if '%' in coll_match: coll_condn = 'like'
  if '%' in obj_match:  obj_condn  = 'like'

  return [ (coll,obj) for coll,obj in
               row_iterator( ["COLL_NAME","DATA_NAME"],
                             "COLL_NAME {coll_condn} '{coll_match}' " \
                             "and DATA_NAME {obj_condn} '{obj_match}'".format(**locals()),
                             AS_LIST, callback 
               ) ]

def _erase_meta( callback , split_path ):

  collpath = split_path [0]
  objname  = split_path [1]

  kvp = callback.msiString2KeyValPair("", irods_types.KeyValPair())['arguments'][1]

  for coll, data, key, value in row_iterator(
   ["COLL_NAME", "DATA_NAME", "META_DATA_ATTR_NAME", "META_DATA_ATTR_VALUE"], 
   "COLL_NAME = '{}' and  DATA_NAME = '{}'".format(collpath,objname), AS_LIST, callback
  ):
      kvp = callback.msiAddKeyVal ( kvp, key, value) ['arguments'] [0]

  for i in range(kvp.len):
   callback.writeLine("stdout", "{} -> {}".format(kvp.key[i], kvp.value[i]))

  callback.msiRemoveKeyValuePairsFromObj ( kvp  ,  "{}/{}".format(collpath,objname) , "-d" )

INPUT  *Operation='Erase', *CollName=$'/tempZone/home/rods', *ObjName=$'%.nc'
OUTPUT ruleExecOut
