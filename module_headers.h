#ifndef VHL_MODULE_HEADERS_H
#define VHL_MODULE_HEADERS_H

#include "common.h"

#define MOD_INFO_VALID_ATTR     0x0000
#define MOD_INFO_VALID_VER      0x0101

#define MODULE_FILENAME_LENGTH 256

//Structs from UVLoader
/*
 * resolve.h - Performs SCE ELF relocations
 * Copyright 2015 Yifan Lu
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
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
        SceNID   *func_nid_table;// array of function NIDs (numFuncs)
        void    **func_entry_table;// parallel array of pointers to stubs; they're patched by the loader to jump to the final code
        SceNID   *var_nid_table;// NIDs of the imported variables (numVars)
        void    **var_entry_table;// array of pointers to "ref tables" for each variable
        SceNID   *tls_nid_table;// NIDs of the imported TLS variables (numTlsVars)
        void    **tls_entry_table;// array of pointers to ???
} SceModuleImports_2x;

typedef struct
{
        SceUShort16 size;           // size of this structure; 0x24 for Vita 3.x
        SceUShort16 lib_version;    //
        SceUShort16 attribute;      //
        SceUShort16 num_functions;  // number of imported functions
        SceUShort16 num_vars;       // number of imported variables
        SceUShort16 unknown1;
        SceUInt module_nid;     // NID of the module to link to
        char    *lib_name;    // name of module
        SceNID   *func_nid_table;// array of function NIDs (numFuncs)
        void    **func_entry_table;// parallel array of pointers to stubs; they're patched by the loader to jump to the final code
        SceNID   *var_nid_table;// NIDs of the imported variables (numVars)
        void    **var_entry_table;// array of pointers to "ref tables" for each variable
} SceModuleImports_3x;

typedef union
{
        SceUShort16 size;
        SceModuleImports_2x old_version;
        SceModuleImports_3x new_version;
} SceModuleImports;

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




static inline unsigned int GET_FUNCTION_COUNT(SceModuleImports *x) {
        return ((x->size == sizeof(SceModuleImports_3x)) ? x->new_version.num_functions : x->old_version.num_functions);
}

static inline SceNID* GET_FUNCTIONS_NIDTABLE(SceModuleImports *x){
        return ((x->size == sizeof(SceModuleImports_3x)) ? x->new_version.func_nid_table : x->old_version.func_nid_table);
}

static inline void** GET_FUNCTIONS_ENTRYTABLE(const SceModuleImports *x){
        return ((x->size == sizeof(SceModuleImports_3x)) ? x->new_version.func_entry_table : x->old_version.func_entry_table);
}

static inline int GET_VARIABLE_COUNT(SceModuleImports *x){
        return ((x->size == sizeof(SceModuleImports_3x)) ? x->new_version.num_vars : x->old_version.num_vars);
}

static inline SceNID* GET_VARIABLE_NIDTABLE(SceModuleImports *x){
        return ((x->size == sizeof(SceModuleImports_3x)) ? x->new_version.var_nid_table : x->old_version.var_nid_table);
}

static inline void** GET_VARIABLE_ENTRYTABLE(SceModuleImports *x){
        return ((x->size == sizeof(SceModuleImports_3x)) ? x->new_version.var_entry_table : x->old_version.var_entry_table);
}

static inline char* GET_LIB_NAME(SceModuleImports *x) {
        return ((x->size == sizeof(SceModuleImports_3x)) ? x->new_version.lib_name : x->old_version.lib_name);
}

static inline SceNID GET_NID(SceModuleImports *x) {
        return ((x->size == sizeof(SceModuleImports_3x)) ? x->new_version.module_nid : x->old_version.module_nid);
}

static inline SceModuleImports* GET_NEXT_IMPORT(SceModuleImports *x){
        return (SceModuleImports*)((SceUInt)x + x->size);
}

static inline SceModuleImports* GET_FIRST_IMPORT(SceUInt base, const SceModuleInfo *x){
        return (SceModuleImports*)(base + (SceUInt)x->stub_top);
}

static inline int IS_LAST_IMPORT(SceUInt base, const SceModuleInfo *x, const SceModuleImports *import){
        return  (SceUInt)import < (base + (SceUInt)x->stub_end);
}

#define FOREACH_IMPORT(base, mod_info, var_name)  for(SceModuleImports *var_name = GET_FIRST_IMPORT(base, mod_info); IS_LAST_IMPORT(base, mod_info, var_name); var_name = GET_NEXT_IMPORT(var_name))

#define FOREACH_EXPORT(base, mod_info, var_name)  for(SceModuleExports *var_name = (SceModuleExports*)(base + mod_info->ent_top); (SceUInt)var_name < (SceUInt)(base + mod_info->ent_end); var_name++)

#endif
