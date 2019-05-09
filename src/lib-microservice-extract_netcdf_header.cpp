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
#include <cstring>
#include <regex.h>
 
#include <netcdf.h>

#include "boost/format.hpp"
#include "string_functions.hpp"
#include "get_attribute_values.hpp"

#define s_i_pair(a) { #a, a }
#define i_s_pair(a) { a, #a }

#ifndef GET_ATTR_VALUES
#define GET_ATTR_VALUES 0
#endif

#ifndef irods_build
#define irods_build 1
#endif

#if irods_build
namespace {
  int  PRINTF(...)  { return 0; }
  int  FPRINTF(...) { return 0; }
  int  FFLUSH(...)  { return 0; }
}
#else
#  define PRINTF std::printf
#  define FPRINTF std::fprintf
#  define FFLUSH std::fflush
#endif

struct bad_input_file 
{
  int return_code = 0;
  std::string message ;
};

typedef std::map< std::string,
                  std::string >  string_to_string_map;

typedef std::map<std::string , int> Str_Int_MAP_t;
typedef std::map<int , std::string> Int_Str_MAP_t;

static Str_Int_MAP_t  nc_type_stoi { 

    s_i_pair(NC_NAT), s_i_pair(NC_BYTE), s_i_pair(NC_CHAR), s_i_pair(NC_SHORT),
    s_i_pair(NC_INT), s_i_pair(NC_FLOAT), s_i_pair(NC_DOUBLE), s_i_pair(NC_UBYTE),
    s_i_pair(NC_USHORT), s_i_pair(NC_UINT), s_i_pair(NC_INT64), s_i_pair(NC_UINT64), s_i_pair(NC_STRING)
};

static Int_Str_MAP_t  nc_type_itos {

    i_s_pair(NC_NAT), i_s_pair(NC_BYTE), i_s_pair(NC_CHAR), i_s_pair(NC_SHORT), 
    i_s_pair(NC_INT), i_s_pair(NC_FLOAT), i_s_pair(NC_DOUBLE), i_s_pair(NC_UBYTE),
    i_s_pair(NC_USHORT), i_s_pair(NC_UINT), i_s_pair(NC_INT64), i_s_pair(NC_UINT64), i_s_pair(NC_STRING)
};

static std::string delimited (std::string s, char d=';') 
{
    return s.empty() ? s : s+d; 
}

int do_attributes (std::string base_string,
                   string_to_string_map & kvp,
                   int ncid,
                   int varid = NC_GLOBAL, // default to "global" or >= 0 for an actual variable
                   int *nattrs = 0)       // NULL pointer means unspecified
{
    using boost::format;
    char name_V[NC_MAX_NAME+1];
    int type_V,ndims_V,dimids_V[NC_MAX_DIMS],nattrs_V;

    if (nattrs ==  0) {
        int status = nc_inq_natts( ncid, (nattrs = &nattrs_V));
        if (status != NC_NOERR) return status;
    }

    for (int attI = 0; attI < *nattrs; attI++) {
        char name_A[NC_MAX_NAME+1];
        int status =  nc_inq_attname(ncid, varid, attI, name_A);
        if (status == NC_NOERR) { int id  = -1;
            std::string  tmpStr, strKey = delimited(base_string) + (format("ATTR=%1%") % attI).str();
            nc_type  xtype_A;
            size_t   plen_A; 

            kvp[ tmpStr = strKey + ";name" ] = name_A;
            PRINTF ("%s\t%s\n" ,tmpStr.c_str(), name_A );

            if (NC_NOERR == nc_inq_atttype(ncid,varid,name_A,&xtype_A))  {
                kvp[ tmpStr = strKey + ";type" ] = nc_type_itos [xtype_A];
                PRINTF("%s\t%s\n",tmpStr.c_str(),nc_type_itos[xtype_A].c_str());
            }

            if (NC_NOERR == nc_inq_attlen(ncid,varid,name_A,&plen_A)) {
                kvp[ tmpStr = strKey + ";len" ] = Stringize(plen_A);
                PRINTF("%s\t%lu\n",tmpStr.c_str(),(unsigned long)plen_A);
            }
        }
    }

#if GET_ATTR_VALUES
    for (int attI = 0; attI < *nattrs; attI++) {
        char name_A[NC_MAX_NAME+1];
        int status =  nc_inq_attname(ncid, varid, attI, name_A);
        if (status != NC_NOERR) {
          break;
        }
        nc_type  xtype_A;
        char buf [1024];
        int offs = 0 ;
        if (NC_NOERR == nc_inq_atttype(ncid,varid,name_A,&xtype_A))  {
            if (xtype_A == NC_INT ) {
                attribute_params<int> sgParams (ncid,varid,name_A);
                sgParams.dump(buf,1024);
            }
            else if (xtype_A == NC_CHAR) {
                attribute_params<char> sgParams (ncid,varid,name_A);
                sgParams.dump(buf,1024);
            }
            else if (xtype_A == NC_STRING) {
                attribute_params<char*> sgParams (ncid,varid,name_A);
                sgParams.dump(buf,1024);
/*
                if (0 == sgParams.get_attribute_values()) {
                    for (int i = 0; i < sgParams.dimension; i++) {
                        FPRINTF(stderr, "%s|", sgParams.p[i]);
                    }
                    FPRINTF(stderr,"\n");
                }
*/
            }
        }
        
    }
#endif 

    return 0;
}

int add_hierarchy_for_ncid
    (
        int ncid,
        std::string         base_string,
        string_to_string_map & kvp
    )
{
    using std::string;
    using std::vector;
    using boost::format;

    int ndims,nvars,ngatts,unlimdimid;
    int parseStatus = nc_inq(ncid, &ndims, &nvars, &ngatts, &unlimdimid);
    if (parseStatus !=  NC_NOERR) { return parseStatus; }

    // - get top-level attributes, dimensions & variables for a group / file NCID

    /* do attributes */

    if (NC_NOERR != do_attributes ( base_string,kvp,ncid )) { return 1; }

    /* do dimensions */

    string k = delimited( base_string )+ "unlimdimid";
    PRINTF("_unlimdimid -> %d\n",unlimdimid);
    kvp [ k ] = Stringize(unlimdimid);

    int group_ndims;
    int group_dimids[NC_MAX_DIMS];
    if (NC_NOERR == nc_inq_dimids(ncid, &group_ndims, group_dimids, 0/* include_parents*/)) 
    {
        for (int i=0;i<group_ndims;i++) {
            int dimId = group_dimids[i];
            size_t lenp;
            char dim_name[NC_MAX_NAME+1];
            int status = nc_inq_dim (ncid, dimId, dim_name, &lenp); 
            PRINTF("INQ_DIM @ ncid=%d dimI= %d => dim_name = '%s' lenp='%d'\n",ncid,dimId,dim_name,(int)lenp);
            if (status==NC_NOERR) {
                string dimLabel = delimited(base_string) + (format("DIM_%1%") % i).str();
                kvp[dimLabel + ";name" ] = dim_name;
                kvp[dimLabel + ";len" ] = Stringize(lenp);
                kvp[dimLabel + ";id" ] = Stringize(dimId);
            }
        }
    }

    vector<string> var_names ;

    for (int varI = 0; varI < nvars; varI++) 
    {
        string VAR_n_label = delimited( base_string ) + (format("VAR=%1%") % varI).str();

        vector<int> var_dimlengths ;

        char name [NC_MAX_NAME+1]  = "";

        int var_type,var_ndims,var_dimids[NC_MAX_VAR_DIMS],var_natts;
        int status = nc_inq_var (ncid, varI, name, &var_type, &var_ndims, var_dimids, &var_natts);

        if (NC_NOERR != do_attributes (VAR_n_label , kvp , ncid , varI , &var_natts)) { 
            FPRINTF(stderr , "Error: could not get attributes for variable");
            return 2;
        }

        if (status != NC_NOERR) { FPRINTF(stderr,"Error: bad get on variable %d\n",varI); continue; }

        PRINTF ("var name = %s\n", name);

        var_names.push_back(name);

        for (int dimN = 0; dimN < var_ndims; dimN++)
        {
            std::size_t lenp;
            char dim_name [NC_MAX_NAME+1] = "*";
            int inq_dim_status = nc_inq_dim (ncid, var_dimids[dimN], dim_name, &lenp); 
            if (inq_dim_status == NC_NOERR) {
                PRINTF("\tdimN %d (id %d) name = '%s' length = %u\n",dimN,var_dimids[dimN],dim_name,unsigned(lenp));
                var_dimlengths . push_back(lenp);
                string vardim_N_label = delimited(VAR_n_label) + (format("DIM_%1%") % dimN).str();
                kvp[ vardim_N_label + ";id" ] = Stringize(var_dimids[dimN]);
                kvp[ vardim_N_label + ";name" ] = dim_name;
                kvp[ vardim_N_label + ";len" ] = Stringize(lenp);
            } 
            else { FPRINTF(stderr,"Error: could not retrieve dimension %d\n", dimN); }
        }
        if (var_dimlengths.size() > 0) kvp[ delimited(VAR_n_label) + "_dimlengths" ] = Join( var_dimlengths );
        kvp[ delimited(VAR_n_label) + "name" ] = name;
        kvp[ delimited(VAR_n_label) + "type" ] = nc_type_itos [ var_type ];
    }
    kvp[ delimited( base_string ) + "_varnames" ] = Join( var_names );

    return 0;
}

int open_netcdf_and_get_metadata ( const char *filename, string_to_string_map & kvp_store)
{
    int file_ncid, openStatus, parseStatus = -1-NC_NOERR;
    int ndims,nvars,ngatts,unlimdimid;
    int status = 0;
    if ((openStatus = nc_open( filename, NC_NOWRITE, & file_ncid )) != NC_NOERR
        || (parseStatus = nc_inq(file_ncid, &ndims, &nvars, &ngatts, &unlimdimid)) != NC_NOERR)
    {
        throw bad_input_file {
          SYS_BAD_INPUT, 
          ( boost::format("Couldn't open NETCDF file. Status ( open = %1% , parse =  %2%)" ) 
            % openStatus % parseStatus
          ).str()
        };
    }

    status = add_hierarchy_for_ncid( file_ncid , "", kvp_store );

    int n_subgrps;
    status = nc_inq_grps ( file_ncid, &n_subgrps, nullptr );    // find how many subgroups
        
    if (n_subgrps > 0) {
        int *group_ncids = new int [n_subgrps];
        int group_inq_status = nc_inq_grps ( file_ncid, &n_subgrps, group_ncids );

        if (group_inq_status == NC_NOERR) { 
            char grpname [NC_MAX_NAME+1]="";
            for (int i=0;i<n_subgrps;i++) 
            {
                int stat = nc_inq_grpname (group_ncids[i], grpname) ;
                PRINTF("\tncid #%d = %d group = '%s'\n",i,group_ncids[i],grpname);
                const std::string root_group { (boost::format("GROUP=%1%") % i).str() };

                kvp_store [ root_group + ";ncid" ] = Stringize( group_ncids [i] );
                kvp_store [ root_group + ";name" ] = Stringize( grpname );

                add_hierarchy_for_ncid ( group_ncids[i], root_group, kvp_store );
            }
        }
        delete [] group_ncids;
    }

    return 0;
}

#if !(irods_build)
int main ( int argc , char* argv [] ) 
{
    using namespace std;
    string_to_string_map  key_value_pairs;
    int status = 0;
    if (argc > 1) {
        status = open_netcdf_and_get_metadata (argv[1] , key_value_pairs); 
    }
    else {
        status = 1;
    }
    cerr << "\n\n\t************** KEY_VALUE_PAIRS **************\n\n";
    for (auto const &pr : key_value_pairs) {
		cerr << "\t" << pr.first << "\t\t" << pr.second << endl;
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

    rei->status = 0;

    for (auto const & kvp : key_value_pairs) 
    {
        rei->status = addKeyVal( (keyValPair_t*) inKeyValPair->inOutStruct, 
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
