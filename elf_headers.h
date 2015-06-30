#ifndef _VHL_ELF_HEADERS_H_
#define _VHL_ELF_HEADERS_H_

#include <psp2/types.h>

#define MOD_INFO_VALID_ATTR     0x0000
#define MOD_INFO_VALID_VER      0x0101

#define MODULE_FILENAME_LENGTH 256
#define SEGMENT_COUNT 4

//Structs from UVLoader

typedef struct // thanks roxfan
{
        SceUShort16 size;       // size of this structure; 0x20 for Vita 1.x
        SceUInt8 lib_version[2]; //
        SceUShort16 attribute;  // ?
        SceUShort16 num_functions; // number of exported functions
        SceUInt num_vars;   // number of exported variables
        SceUInt num_tls_vars; // number of exported TLS variables?  <-- pretty sure wrong // yifanlu
        SceUInt module_nid; // NID of this specific export list; one PRX can export several names
        char    *lib_name;// name of the export module
        SceUInt   *nid_table;// array of 32-bit NIDs for the exports, first functions then vars
        void    **entry_table;// array of pointers to exported functions and then variables
} SceModuleExports;

/**
 * \brief SCE module import table (< 3.0 format)
 *
 * Can be found in an ELF file or loaded in
 * memory.
 */
typedef struct // thanks roxfan
{
        SceUShort16 size;           // size of this structure; 0x34 for Vita 1.x
        SceUShort16 lib_version;    //
        SceUShort16 attribute;      //
        SceUShort16 num_functions;  // number of imported functions
        SceUShort16 num_vars;       // number of imported variables
        SceUShort16 num_tls_vars;   // number of imported TLS variables
        SceUInt reserved1;      // ?
        SceUInt module_nid;     // NID of the module to link to
        char    *lib_name;    // name of module
        SceUInt reserved2;      // ?
        SceUInt   *func_nid_table;// array of function NIDs (numFuncs)
        void    **func_entry_table;// parallel array of pointers to stubs; they're patched by the loader to jump to the final code
        SceUInt   *var_nid_table;// NIDs of the imported variables (numVars)
        void    **var_entry_table;// array of pointers to "ref tables" for each variable
        SceUInt   *tls_nid_table;// NIDs of the imported TLS variables (numTlsVars)
        void    **tls_entry_table;// array of pointers to ???
} SceModuleImports_2x;

/**
 * \brief SCE module import table (>= 3.x format)
 *
 * Can be found in an ELF file or loaded in
 * memory.
 */
typedef struct
{
        SceUShort16 size;           // size of this structure; 0x24 for Vita 3.x
        SceUShort16 lib_version;    //
        SceUShort16 attribute;      //
        SceUShort16 num_functions;  // number of imported functions
        SceUShort16 num_vars;       // number of imported variables
        SceUShort16 unknown1;
        SceUShort16 module_nid;     // NID of the module to link to
        char    *lib_name;    // name of module
        SceUInt   *func_nid_table;// array of function NIDs (numFuncs)
        void    **func_entry_table;// parallel array of pointers to stubs; they're patched by the loader to jump to the final code
        SceUInt   *var_nid_table;// NIDs of the imported variables (numVars)
        void    **var_entry_table;// array of pointers to "ref tables" for each variable
} SceModuleImports_3x;

/**
 * \brief SCE module import table
 */
typedef union
{
        SceUShort16 size;
        SceModuleImports_2x old_version;
        SceModuleImports_3x new_version;
} SceModuleImports;

typedef struct
{
        SceUInt size;       // this structure size (0x18)
        SceUInt perms;      // probably rwx in low bits
        void            *vaddr;// address in memory
        SceUInt memsz;      // size in memory
        SceUInt flags;      // meanig unknown
        SceUInt res;        // unused?
} SceSegmentInfo;

typedef struct
{
        SceUInt size;                   // 0x1B8 for Vita 1.x
        SceUInt handle;                 // kernel module handle?
        SceUInt flags;                  // some bits. could be priority or whatnot
        char module_name[28];
        SceUInt unkn_28;
        void            *module_start;
        SceUInt unkn_30;
        void            *module_stop;
        void            *exidx_start;
        void            *exidx_end;
        SceUInt unkn_40;
        SceUInt unkn_44;
        void            *tls_init_data;
        SceUInt tls_init_size;
        SceUInt tls_area_size;
        char file_path[256];              //
        SceSegmentInfo segments[SEGMENT_COUNT];
        SceUInt type;           // 6 = user-mode PRX?
} SceLoadedModuleInfo;

typedef struct // thanks roxfan
{
        SceUInt16 modattribute; // ??
        SceUInt16 modversion;  // always 1,1?
        char modname[27];  ///< Name of the module
        SceUInt8 type;         // 6 = user-mode prx?
        void    *gp_value; // always 0 on ARM
        SceUInt ent_top;     // beginning of the export list (sceModuleExports array)
        SceUInt ent_end;     // end of same
        SceUInt stub_top;    // beginning of the import list (sceModuleStubInfo array)
        SceUInt stub_end;    // end of same
        SceUInt module_nid;  // ID of the PRX? seems to be unused
        SceUInt field_38;    // unused in samples
        SceUInt field_3C;    // I suspect these may contain TLS info
        SceUInt field_40;    //
        SceUInt mod_start;   // module start function; can be 0 or -1; also present in exports
        SceUInt mod_stop;    // module stop function
        SceUInt exidx_start; // ARM EABI style exception tables
        SceUInt exidx_end;   //
        SceUInt extab_start; //
        SceUInt extab_end;   //
} SceModuleInfo;

#endif
