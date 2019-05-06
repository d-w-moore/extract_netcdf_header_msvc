from __future__ import print_function
#import sys, os
#sys.path.append(os.getcwd())
import io
import myinspect
def main(*arg): my_rodsstat_test(*arg)


def my_rodsstat_test (rule_args, callback, rei):
    import os
    callback.writeLine("stdout","{}".format(os.getcwd()))
    source_file = global_vars['*SourceFile'][1:-1]
    debug = int(global_vars['*debug'][1:-1])

    import os
    (coll, file) = os.path.split(source_file)

    ret_val = callback.msiObjStat(source_file, irods_types.RodsObjStat())
    source_file_stat = ret_val['arguments'][1]

    rodsstat = irods_types.RodsObjStat()

    if debug:
        printDebug(callback, 'before ',rodsstat)

    ret_val = callback.msiObjStat(coll, rodsstat)
    coll_stat = ret_val['arguments'][1]
    callback.writeLine('stdout', 'Type of object is written into a RodsObjStat_PI structure')

    if debug == 1 : 
        printDebug(callback, 'after  ', source_file_stat)
    if debug == 2 : 
        iob = io.BytesIO()
        tc = [ irods_types.char_array , lambda obj: ("char_array with str={!r}".format(str(obj)),) ],
        myinspect.myInspect (source_file_stat, stream = iob, types_callback = tc)
        callback.writeLine('stdout', iob.getvalue())
    if debug == 3 : 
        iob = io.BytesIO()
        tc3 = [ irods_types.BytesBuf , lambda obj: ("char_array with buf {!r}".format(obj.buf),) ],
        myinspect.myInspect (irods_types.ExecCmdOut(), stream = iob, types_callback = tc3)
        callback.writeLine('stdout', iob.getvalue())


#   helper

def printDebug( callback, annotate, rodsstat ):

    callback.writeLine('stdout', '--- {} ---'.format(annotate))
    for attr in [x for x in dir(rodsstat) if not x.startswith('__')]:
        subobj = getattr(rodsstat,attr)
        callback.writeLine('stdout', '\t rodsstat.{0!s} = {1!r}'.format(attr, subobj))
        if attr in ['chksum', 'rescHier']:
            callback.writeLine('stdout', '\t\t == {0!r}'.format(str(subobj)))
           
 
INPUT *SourceFile="/tempZone/home/rods/sub1/foo3", *debug='0'
OUTPUT ruleExecOut
