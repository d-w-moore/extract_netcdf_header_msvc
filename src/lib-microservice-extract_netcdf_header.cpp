// =-=-=-=-=-=-=-
// irods includes

#include "irods_error.hpp"
#include "irods_ms_plugin.hpp"

#include "icatHighLevelRoutines.hpp"
#include "rcMisc.h"
  
#include "irods_re_structs.hpp"

// =-=-=-=-=-=-=-
// stl includes

#include <map>
#include <vector>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <string>
//#include <exception>
#include <cstring>
#include <regex.h>
 
#include <netcdf.h>

#include "boost/format.hpp"
#include "string_functions.hpp"

#define s_i_pair(a) { #a, a }
#define i_s_pair(a) { a, #a }

#ifndef irods_build
#define irods_build 1
#endif

#if irods_build
#  define printf std::printf
#  define fprintf std::fprintf
#else
#  define printf
#  define fprintf
#endif

struct bad_input_file 
{
  int return_code = 0;
  std::string message ;
};

typedef std::map<std::string,std::string> string_to_string_map;
typedef std::map<std::string , int> NCTYPES_S_I_MAP_t;
typedef std::map<int , std::string> NCTYPES_I_S_MAP_t;

static NCTYPES_S_I_MAP_t  nc_type_stoi { 

    s_i_pair(NC_NAT), s_i_pair(NC_BYTE), s_i_pair(NC_CHAR), s_i_pair(NC_SHORT),
    s_i_pair(NC_INT), s_i_pair(NC_FLOAT), s_i_pair(NC_DOUBLE), s_i_pair(NC_UBYTE),
    s_i_pair(NC_USHORT), s_i_pair(NC_UINT), s_i_pair(NC_INT64), s_i_pair(NC_UINT64), s_i_pair(NC_STRING)
};

static NCTYPES_I_S_MAP_t  nc_type_itos {

    i_s_pair(NC_NAT), i_s_pair(NC_BYTE), i_s_pair(NC_CHAR), i_s_pair(NC_SHORT), 
    i_s_pair(NC_INT), i_s_pair(NC_FLOAT), i_s_pair(NC_DOUBLE), i_s_pair(NC_UBYTE),
    i_s_pair(NC_USHORT), i_s_pair(NC_UINT), i_s_pair(NC_INT64), i_s_pair(NC_UINT64), i_s_pair(NC_STRING)
};


std::string
add_hierarchy_for_ncid
    (
        int ncid,
        std::string         base_string,
        string_to_string_map & kvpCache 
    )
{
    std::string delimiter = "" ;
    auto delimited_append = [&](std::string new_stuff) -> void { 
        if (0 < base_string.size()) delimiter = ";";
        base_string += (delimiter + new_stuff);
    };
    delimited_append("a");
    delimited_append("b");
    return base_string;
}


int
open_netcdf_and_get_metadata (
  const char *filename,
  string_to_string_map & kvp_store)
{
    int file_ncid, parseStatus = -1-NC_NOERR;
    int openStatus = nc_open( filename, NC_NOWRITE, & file_ncid );
    int ndims,nvars,ngatts,unlimdimid;

    if (openStatus != NC_NOERR
        || (parseStatus = nc_inq(file_ncid, &ndims, &nvars, &ngatts, &unlimdimid)) != NC_NOERR)
    {
        throw bad_input_file {
          SYS_BAD_INPUT, 
          ( boost::format("Couldn't open NETCDF file. Status ( open = %1% , parse =  %2%)" ) 
            % openStatus % parseStatus
          ).str()
        };
    }

    // printf("unlimdimid -> %d\n",unlimdimid);

    int n_subgrps;
    int status = nc_inq_grps ( file_ncid, &n_subgrps, nullptr );    // find how many subgroups

    std::string junk = add_hierarchy_for_ncid( file_ncid , "", kvp_store );
    const char * junkc = junk.c_str();
        
    if (n_subgrps > 0) { 
        int *grpids = new int [n_subgrps];
        int group_inq_status = nc_inq_grps ( file_ncid, &n_subgrps, grpids );
/*
        if (group_inq_status == NC_NOERR) { 
            char grpname [NC_MAX_NAME]="";
            for (int i=0;i<n_subgrps;i++) { //if (NC_NOERR == 
                int stat = nc_inq_grpname (grpids[i], grpname) ;
                printf("\tncid #%d = %d group = '%s'\n",i,grpids[i],grpname);
            }
        }
*/
        delete [] grpids;
    }

/* *** get top-level variables for file/group
 *
    for (int rh_id = 0; rh_id < nvars; rh_id++) 
    {
        char name [NC_MAX_NAME]  = "";
        int rh_type,rh_ndims,rh_dimids[NC_MAX_DIMS],rh_natts;
        int status = nc_inq_var (ncid, rh_id, name, &rh_type, &rh_ndims, rh_dimids, &rh_natts);
        for (int dimId = 0; dimId < rh_ndims; dimId++)
        {
            std::size_t lenp;
            char dim_name [NC_MAX_NAME] = "*";
            int inq_dim_status = nc_inq_dim (ncid, dimId, dim_name, &lenp); 
            if (inq_dim_status == NC_NOERR) { 
                // -- DWM dim ids --  printf("\t - (dimid = %d) %s [%u]\n",rh_dimids[dimId],dim_name,unsigned(lenp));
            }
        }
    }
 * ***/
    return 0;
}

#if !(irods_build)
int main ( int argc , char* argv [] ) 
{
    string_to_string_map  key_value_pairs;
    int status = 0;
    if (argc > 1) {
        status = open_netcdf_and_get_metadata (argv[1] , key_value_pairs); 
    }
    else {

        status = 1;
    }
    return status;
}
#else // irods_build

int msiextract_netcdf_header(
    msParam_t*      inKeyValPair,
    msParam_t*      msp_netCDF_FileName,
    ruleExecInfo_t* rei )
{
    char *netCDF_FileName = NULL;
    //std::vector<std::string> keys, values; 

    string_to_string_map key_value_pairs;
   
    /* Parse filename */

    if ( ( netCDF_FileName = parseMspForStr( msp_netCDF_FileName ) ) == NULL ) {
        rodsLog( LOG_ERROR, "msiAddKeyVal: netCDF_FileName is NULL." );
        return USER__NULL_INPUT_ERR;
    }

    int status = 0;
    try {
        status = open_netcdf_and_get_metadata (netCDF_FileName , key_value_pairs); 
    }
    catch ( bad_input_file & Error ) {
        rodsLog(LOG_NOTICE , Error.message.c_str() );
        return Error.return_code;
    }
    catch(...) { 
        status = -1;
        rodsLog(LOG_NOTICE , "Unknown error in NetCDF metadata extraction");
    }

    if (status != 0) { return status; }

    /* Check for wrong parameter type */

    if ( inKeyValPair->type && strcmp( inKeyValPair->type, KeyValPair_MS_T ) ) {
        rodsLog( LOG_ERROR, "msiAddKeyVal: inKeyValPair is not of type KeyValPair_MS_T." );
        return USER_PARAM_TYPE_ERR;
    }
 
    /* Parse inKeyValPair. Create new one if empty. */

    if ( !inKeyValPair->inOutStruct )
    {
        void* newKVP = std::malloc( sizeof( keyValPair_t ) );
        std::memset( newKVP, 0, sizeof( keyValPair_t ) );
        inKeyValPair->inOutStruct = ( void* ) newKVP;

        if ( !inKeyValPair->type ) {
            inKeyValPair->type = strdup( KeyValPair_MS_T );
        }
    }

/*  std::size_t nKeys = keys.size();
    if ( nKeys != values.size() ) {
        const char * fn = netCDF_FileName;
        rodsLog( LOG_ERROR, "Something went wrong in header extraction in '%s'\n" , fn ? fn : "");
        return SYS_INTERNAL_ERR;
    }
 */

    rei->status = 0;

    for (auto const & kvp : key_value_pairs) {

        rei->status = addKeyVal( ( keyValPair_t* )inKeyValPair->inOutStruct, 
                                   kvp.first.c_str(),
                                   kvp.second.c_str() );

        if (rei->status != 0) { 
            rodsLog( LOG_ERROR, "Bad value from  addKeyVal \n"  );
            return (rei->status);
        }
    }

    return 0;
}

extern "C"
irods::ms_table_entry* plugin_factory() {
    irods::ms_table_entry* msvc = new irods::ms_table_entry(2);
    msvc->add_operation<
        msParam_t*,
        msParam_t*,
        ruleExecInfo_t*>("msiextract_netcdf_header",
                         std::function<int(
                             msParam_t*,
                             msParam_t*,
                             ruleExecInfo_t*)>(msiextract_netcdf_header));
    return msvc;
}

#endif // irods_build
