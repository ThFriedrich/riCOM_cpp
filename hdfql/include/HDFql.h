// Copyright (C) 2016-2021
// This file is part of the Hierarchical Data Format query language (HDFql)
// For more information about HDFql, please visit the website http://www.hdfql.com

// $Rev: 352 $



#ifndef _HDFQL_HEADER
	#define _HDFQL_HEADER



/*! \mainpage HDFql C Functions Documentation
 *
 *  Library of HDFql C functions to manage HDF5 files
 *
 *  \n
 *  \n
 *  \li \ref SectionExecute
 *  \n
 *  \n
 *  \li \ref SectionError
 *  \n
 *  \n
 *  \li \ref SectionCursor
 *  \n
 *  \n
 *  \li \ref SectionVariable
 *  \n
 *  \n
 *  \li \ref SectionMPI
 *  \n
 *  \n
 *  \li \ref SectionInitializerFinalizer
 *  \n
 *  \n
 *  \li \ref SectionAuxiliary
*/



//===========================================================
// INCLUDE FILES
//===========================================================
#include <stdint.h>
#include "HDFqlConstants.h"
#include "HDFqlStructures.h"



//===========================================================
// DEFINITIONS
//===========================================================
#ifdef _WIN32
	#define _HDFQL_EXPORT_SYMBOL __declspec(dllexport)
#else
	#define _HDFQL_EXPORT_SYMBOL
#endif



//===========================================================
// ENABLE USAGE FROM C++
//===========================================================
#ifdef __cplusplus
	extern "C"
	{
#endif



//===========================================================
// FUNCTIONS PROTOTYPE
//===========================================================
/** @defgroup SectionExecute Execute Functions
 *  List of execute functions.
 *  @{
*/

_HDFQL_EXPORT_SYMBOL int hdfql_execute(const char *script);

_HDFQL_EXPORT_SYMBOL int hdfql_execute_get_status(void);

/** @} */



/** @defgroup SectionError Error Functions
 *  List of error functions.
 *  @{
*/

_HDFQL_EXPORT_SYMBOL int hdfql_error_get_line(void);

_HDFQL_EXPORT_SYMBOL int hdfql_error_get_position(void);

_HDFQL_EXPORT_SYMBOL char *hdfql_error_get_message(void);

/** @} */



/** @defgroup SectionCursor Cursor Functions
 *  List of cursor functions.
 *  @{
*/

_HDFQL_EXPORT_SYMBOL int hdfql_cursor_initialize(HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_cursor_use(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_cursor_use_default(void);

_HDFQL_EXPORT_SYMBOL int hdfql_cursor_clear(HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_cursor_clone(const HDFQL_CURSOR *cursor_original, HDFQL_CURSOR *cursor_clone);

_HDFQL_EXPORT_SYMBOL int hdfql_cursor_get_data_type(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_cursor_get_count(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_subcursor_get_count(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_cursor_get_position(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_subcursor_get_position(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_cursor_first(HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_subcursor_first(HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_cursor_last(HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_subcursor_last(HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_cursor_next(HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_subcursor_next(HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_cursor_previous(HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_subcursor_previous(HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int hdfql_cursor_absolute(HDFQL_CURSOR *cursor, int position);

_HDFQL_EXPORT_SYMBOL int hdfql_subcursor_absolute(HDFQL_CURSOR *cursor, int position);

_HDFQL_EXPORT_SYMBOL int hdfql_cursor_relative(HDFQL_CURSOR *cursor, int position);

_HDFQL_EXPORT_SYMBOL int hdfql_subcursor_relative(HDFQL_CURSOR *cursor, int position);

_HDFQL_EXPORT_SYMBOL char *hdfql_cursor_get_tinyint(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL char *hdfql_subcursor_get_tinyint(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL unsigned char *hdfql_cursor_get_unsigned_tinyint(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL unsigned char *hdfql_subcursor_get_unsigned_tinyint(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL short *hdfql_cursor_get_smallint(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL short *hdfql_subcursor_get_smallint(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL unsigned short *hdfql_cursor_get_unsigned_smallint(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL unsigned short *hdfql_subcursor_get_unsigned_smallint(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int *hdfql_cursor_get_int(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL int *hdfql_subcursor_get_int(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL unsigned int *hdfql_cursor_get_unsigned_int(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL unsigned int *hdfql_subcursor_get_unsigned_int(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL long long *hdfql_cursor_get_bigint(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL long long *hdfql_subcursor_get_bigint(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL unsigned long long *hdfql_cursor_get_unsigned_bigint(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL unsigned long long *hdfql_subcursor_get_unsigned_bigint(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL float *hdfql_cursor_get_float(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL float *hdfql_subcursor_get_float(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL double *hdfql_cursor_get_double(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL double *hdfql_subcursor_get_double(const HDFQL_CURSOR *cursor);

_HDFQL_EXPORT_SYMBOL char *hdfql_cursor_get_char(const HDFQL_CURSOR *cursor);

/** @} */



/** @defgroup SectionVariable Variable Functions
 *  List of variable functions.
 *  @{
*/

_HDFQL_EXPORT_SYMBOL int hdfql_variable_register(const void *variable);

_HDFQL_EXPORT_SYMBOL int hdfql_variable_transient_register(const void *variable);

_HDFQL_EXPORT_SYMBOL int hdfql_variable_unregister(const void *variable);

_HDFQL_EXPORT_SYMBOL int hdfql_variable_unregister_all(void);

_HDFQL_EXPORT_SYMBOL int hdfql_variable_get_number(const void *variable);

_HDFQL_EXPORT_SYMBOL int hdfql_variable_get_data_type(const void *variable);

_HDFQL_EXPORT_SYMBOL int hdfql_variable_get_count(const void *variable);

_HDFQL_EXPORT_SYMBOL int hdfql_variable_get_size(const void *variable);

_HDFQL_EXPORT_SYMBOL int hdfql_variable_get_dimension_count(const void *variable);

_HDFQL_EXPORT_SYMBOL long long hdfql_variable_get_dimension(const void *variable, int index);

/** @} */



/** @defgroup SectionMPI MPI Functions
 *  List of MPI functions.
 *  @{
*/

_HDFQL_EXPORT_SYMBOL int hdfql_mpi_get_size(void);

_HDFQL_EXPORT_SYMBOL int hdfql_mpi_get_rank(void);

/** @} */



/** @defgroup SectionInitializerFinalizer Initializer/Finalizer Functions
 *  List of initializer/finalizer functions.
 *  @{
*/

_HDFQL_EXPORT_SYMBOL void hdfql_initialize(void);

_HDFQL_EXPORT_SYMBOL void hdfql_finalize(void);

/** @} */



/** @defgroup SectionAuxiliary Auxiliary Functions
 *  List of auxiliary functions.
 *  @{
*/
_HDFQL_EXPORT_SYMBOL int _hdfql_execute(const char *script, int script_size, int programming_language);

_HDFQL_EXPORT_SYMBOL int _hdfql_execute_reset(void);

_HDFQL_EXPORT_SYMBOL char *_hdfql_show_use_group(void);

_HDFQL_EXPORT_SYMBOL char *_hdfql_cursor_get_char_with_size(HDFQL_CURSOR *cursor, int *size);

_HDFQL_EXPORT_SYMBOL uintptr_t _hdfql_cursor_create(void);

_HDFQL_EXPORT_SYMBOL void _hdfql_cursor_destroy(uintptr_t address);

_HDFQL_EXPORT_SYMBOL unsigned long long _hdfql_get_last_data_count(void);

_HDFQL_EXPORT_SYMBOL char *_hdfql_get_compiler(void);

_HDFQL_EXPORT_SYMBOL char *_hdfql_get_architecture(void);

/** @} */



#ifdef __cplusplus
	}
#endif



#endif   // _HDFQL_HEADER

