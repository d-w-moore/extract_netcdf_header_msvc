def set_or_clear_gnx_test_meta_on_coll(collnpath , operation, N, callback,
                                       partial_pages = True):
    import irods_types
    if partial_pages:
        lower_bound = 0
    else:
        lower_bound = 1000 - 768
    thousands = 0
    meta_string = "kvp_page={}%" .format(thousands) + \
                  "%".join("{:04d}={}".format(i,-(i%10+1)) for i in range(lower_bound,1000))

    for thousands in range(int(N)):
        kvp = callback.msiString2KeyValPair(meta_string, irods_types.KeyValPair())['arguments'][1]
        if operation.upper() in ('ASSOC','SET','+'):
            retval = callback.msiAssociateKeyValuePairsToObj( kvp, collnpath,  "-C")
        elif operation.upper() in ('CLEAR', 'CLR', '-'):
            retval = callback.msiRemoveKeyValuePairsFromObj( kvp, collnpath,  "-C")
        meta_string = meta_string.replace(
                       "%{}".format(thousands),"%{}".format(thousands+1)).replace(
                       "={}".format(thousands),"={}".format(thousands+1))

