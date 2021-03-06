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
#include <cstdint>
#include <string>
#include <cstring>
#include <regex.h>

#include <netcdf.h>

#include "boost/format.hpp"
#include "string_functions.hpp"

#include "get_attribute_values.hpp" // keep this as last include

#define s_i_pair(a) { #a, a }
#define i_s_pair(a) { a, #a }

#ifndef IRODS_NETCDF_ATTRS // if > 0, will == logbase2 (dump buffer length) - 4
                           // if == 0 ,will disable
#define IRODS_NETCDF_ATTRS 1
#elif IRODS_NETCDF_ATTRS > 0 &&\
      IRODS_NETCDF_ATTRS <= 7
#  define ATTR_BUFSIZE (1 << ((IRODS_NETCDF_ATTRS) + 4))
#elif IRODS_NETCDF_ATTRS == 0
#define ATTR_BUFSIZE 0
#else
#error IRODS_NETCDF_ATTRS must be in range [0,7]
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

namespace
{
  std::map<int,std::string> ncid_path;
  std::map<int,std::string> ncid_group_index;
}

struct ncid_retrieve_error {int nc_error_code;};

static std::string delimited (std::string s, char d=';')
{
    return s.empty() ? s : s+d;
}

/* 
 * call once per NetCDF input file:
 */

std::vector<int>
get_ncids_breadth_first (int top_ncid , std::string root_name, string_to_string_map & kvp)
{
    std::map<int,bool> seen;
    std::vector<int> ordered_ncids;
    std::deque<int> ncid_iter;

    /* Globals and side effects.
     * Clean this up! - dwm
     */
    ncid_path.clear();
    ncid_group_index.clear();
    
    ncid_path [top_ncid] = "/";
    ncid_group_index [top_ncid] = root_name;  // - indices of form "[root_name;]GROUP_x;GROUP_y;..."
                                              // - for nested groups numbered (x, y, ...)
    ordered_ncids.push_back( top_ncid );
    ncid_iter.push_back( top_ncid );

    int ncid = top_ncid;

    while (!seen[ncid]) {  // for each ncid (integer group id) found in file

        seen[ncid] = true;

        int n_subgrps;

        int status = nc_inq_grps ( ncid, &n_subgrps, nullptr );    // find how many subgroups
        if (status != NC_NOERR) { throw ncid_retrieve_error {status}; }

        const std::string  parent_index_path = ncid_group_index [ncid];

        if (n_subgrps > 0) {
            int *grp_ncids = new int [n_subgrps];
            int status = nc_inq_grps ( ncid, &n_subgrps, grp_ncids );    // enumerate subgroups
            if (status != NC_NOERR) { throw ncid_retrieve_error {status}; }
            else {
                char grpname [NC_MAX_NAME+1]="";
                for (int i=0;i<n_subgrps;i++)     // get and record group name for each ncid
                {
                    int found_ncid =  grp_ncids[i];

                    int stat = nc_inq_grpname (found_ncid, grpname) ;

                    //PRINTF("\tncid #%d = %d group = '%s'\n",i,found_ncid,grpname);

                    const std::string index_new { 
                        delimited(parent_index_path) + (boost::format("%1%_GROUP") % i ).str()
                    };

                    kvp [ index_new + ";ncid" ] = Stringize( found_ncid );
                    kvp [ index_new + ";name" ] = Stringize( grpname );

                    ncid_iter.push_back( found_ncid );

                    std::string parentString { ncid_path [ ncid ] };
                    std::string separator { parentString.size() > 0 && parentString.back() == '/' ? "" : "/" };
                    ncid_path [ found_ncid ] = parentString + separator + grpname;
                    ordered_ncids.push_back( found_ncid );
                    ncid_group_index [ found_ncid ] = index_new;
                }
            }

            delete [] grp_ncids;
            if (status != NC_NOERR) { throw ncid_retrieve_error {status}; }
        }

        ncid_iter.pop_front();
        if (ncid_iter.empty()) break;
        ncid = ncid_iter.front();
    }
    return  ordered_ncids;
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
            std::string  tmpStr, strKey = delimited(base_string) + (format("%1%_ATTR") % attI).str();
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

#if ATTR_BUFSIZE > 0
    for (int attI = 0; attI < *nattrs; attI++) {
        char name_A[NC_MAX_NAME+1];
        std::string  tmpStr, strKey = delimited(base_string) + (format("%1%_ATTR") % attI).str();
        int status =  nc_inq_attname(ncid, varid, attI, name_A);
        if (status != NC_NOERR) {
          break;
        }
        nc_type  xtype_A;
        char buf [ATTR_BUFSIZE];
        int offs = 0 ;
        if (NC_NOERR == nc_inq_atttype(ncid,varid,name_A,&xtype_A))  {

#define ATTRIBUTE_VALUES_TAG "_value"
#define dump_attribute_type( PRIMITIVE_TYPE ) { \
                attribute_params<PRIMITIVE_TYPE> sgParams (ncid,varid,name_A); \
                if (NC_NOERR==sgParams.get_attribute_values()) { \
                    sgParams.dump(buf,ATTR_BUFSIZE); \
                    kvp[ strKey + ";" + ATTRIBUTE_VALUES_TAG ] = buf; \
                } \
            }

/*
 #define NC_BYTE         1
 #define NC_CHAR         2
 #define NC_SHORT        3
 #define NC_INT          4
 #define NC_LONG         NC_INT
 #define NC_FLOAT        5
 #define NC_DOUBLE       6
 #define NC_UBYTE        7
 #define NC_USHORT       8
 #define NC_UINT         9
 #define NC_INT64        10
 #define NC_UINT64       11
 #define NC_STRING       12
 * */

#define outputtype__ ,

            if      (xtype_A == NC_FLOAT)  { dump_attribute_type(float)    }
            else if (xtype_A == NC_DOUBLE) { dump_attribute_type(double)   }
            else if (xtype_A == NC_SHORT)  { dump_attribute_type(short)    }
            else if (xtype_A == NC_USHORT) { dump_attribute_type(unsigned short) }
            else if (xtype_A == NC_INT)    { dump_attribute_type(int)      }
            else if (xtype_A == NC_UINT)   { dump_attribute_type(unsigned) }
            else if (xtype_A == NC_LONG)   { dump_attribute_type(long)     }
            else if (xtype_A == NC_INT64)  { dump_attribute_type(std::int64_t)  }
            else if (xtype_A == NC_UINT64) { dump_attribute_type(std::uint64_t) }
            else if (xtype_A == NC_BYTE)   { dump_attribute_type(char outputtype__ int) }
            else if (xtype_A == NC_UBYTE)  { dump_attribute_type(unsigned char outputtype__ UINT) }
            else if (xtype_A == NC_CHAR)   { dump_attribute_type(char)     }
            else if (xtype_A == NC_STRING) { dump_attribute_type(char*)    }
            else { snprintf(buf,ATTR_BUFSIZE,"%s","<unsupported-attribute-type>");
                   buf[ATTR_BUFSIZE - 1] = '\0';
                   kvp[ strKey + ";" + ATTRIBUTE_VALUES_TAG ] = buf;
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
                string dimLabel = delimited(base_string) + (format("%1%_DIM") % i).str();
                kvp[dimLabel + ";name" ] = dim_name;
                kvp[dimLabel + ";len" ] = Stringize(lenp);
                kvp[dimLabel + ";id" ] = Stringize(dimId);
            }
        }
    }

    vector<string> var_names ;

    for (int varI = 0; varI < nvars; varI++)
    {
        string VAR_n_label = delimited( base_string ) + (format("%1%_VAR") % varI).str();

        vector<int> var_dimlengths ;
        vector<std::string> var_dimNames ;

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
                //PRINTF("\tdimN %d (id %d) name = '%s' length = %u\n",dimN,var_dimids[dimN],dim_name,unsigned(lenp));
                var_dimlengths . push_back(lenp);
                var_dimNames . push_back(dim_name);
                string vardim_N_label = delimited(VAR_n_label) + (format("%1%_DIMS") % dimN).str();
                kvp[ vardim_N_label + ";id" ] = Stringize(var_dimids[dimN]);
                kvp[ vardim_N_label + ";name" ] = dim_name;
                kvp[ vardim_N_label + ";len" ] = Stringize(lenp);
            }
            else { FPRINTF(stderr,"Error: could not retrieve variable dimension %d\n", dimN); }
        }
        if (var_dimlengths.size() > 0) kvp[ delimited(VAR_n_label) + "_dimLengths" ] = Join( var_dimlengths );
        if (var_dimNames.size() > 0) kvp[ delimited(VAR_n_label) + "_dimNames" ] = Join( var_dimNames   );
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
    if ((openStatus = nc_open( filename, NC_NOWRITE, &file_ncid )) != NC_NOERR
        || (parseStatus = nc_inq(file_ncid, &ndims, &nvars, &ngatts, &unlimdimid)) != NC_NOERR)
    {
        throw bad_input_file {
          SYS_BAD_INPUT,
          (boost::format("Couldn't open NETCDF file. Status ( open = %1% , parse =  %2%)" )
            % openStatus % parseStatus).str()
        };
    }

    std::vector <int> all_ncids;
    try 
    {
      all_ncids = get_ncids_breadth_first (file_ncid ,"", kvp_store);
    }
    catch (ncid_retrieve_error& Error ) {
      status = Error.nc_error_code;
      FPRINTF(stderr, "NetCDF lib error while getting file group structure\n%s\n", nc_strerror(status));
    }

    FPRINTF (stderr,"NCIDs/groups in order:");
    for (int ncid : all_ncids)
    {
  
        std::string group_key =  ncid_group_index [ncid];
        std::string group_path =  ncid_path [ncid];

        FPRINTF (stderr,"\t %ld\t %s\t '%s'\n",long(ncid),group_path.c_str(),group_key.c_str());

        add_hierarchy_for_ncid ( ncid, group_key, kvp_store );
    }

    if (openStatus == NC_NOERR) { nc_close( file_ncid ); }
    return status;
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
