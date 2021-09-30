// Copyright (C) 2016-2021
// This file is part of the Hierarchical Data Format query language (HDFql)
// For more information about HDFql, please visit the website http://www.hdfql.com

// $Rev: 358 $



#ifndef _HDFQL_CPP_HEADER
	#define _HDFQL_CPP_HEADER



/*! \mainpage HDFql C++ Documentation
 *
 *  Library of HDFql C++ functions to manage HDF5 files
 *
 *  \n
 *  \n
 *  \li \ref SectionGeneral
 *  \n
 *  \n
 *  \li \ref SectionObject
 *  \n
 *  \n
 *  \li \ref SectionDataType
 *  \n
 *  \n
 *  \li \ref SectionScope
 *  \n
 *  \n
 *  \li \ref SectionOrder
 *  \n
 *  \n
 *  \li \ref SectionStorageType
 *  \n
 *  \n
 *  \li \ref SectionStorageAllocation
 *  \n
 *  \n
 *  \li \ref SectionEndianness
 *  \n
 *  \n
 *  \li \ref SectionCharset
 *  \n
 *  \n
 *  \li \ref SectionFill
 *  \n
 *  \n
 *  \li \ref SectionLibraryBounds
 *  \n
 *  \n
 *  \li \ref SectionStatus
 *  \n
 *  \n
 *  \li \ref SectionProgrammingLanguages
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
*/



//===========================================================
// INCLUDE FILES
//===========================================================
#if __cplusplus > 199711L || (defined(_MSC_VER) && __cplusplus == 199711L)
	#define _HDFQL_STD_ARRAY
	#include <array>
#endif
#include <vector>
#include <string>
#include <cstring>
#include <sstream>
#include "HDFql.h"



//===========================================================
// NAMESPACE DECLARATION
//===========================================================
namespace HDFql
{



	//===========================================================
	// CLASSES DECLARATION
	//===========================================================
	class Cursor
	{

		public:
			HDFQL_CURSOR cursor;

			_HDFQL_EXPORT_SYMBOL Cursor();

			_HDFQL_EXPORT_SYMBOL ~Cursor();

	};



	/** @defgroup SectionGeneral General Constants
	 *  List of general constants.
	 *  @{
	*/

	const char Version[] = HDFQL_VERSION;

	const int Yes = HDFQL_YES;

	const int No = HDFQL_NO;

	const int Enabled = HDFQL_ENABLED;

	const int Disabled = HDFQL_DISABLED;

	const int Unlimited = HDFQL_UNLIMITED;

	const int Undefined = HDFQL_UNDEFINED;

	/** @} */



	/** @defgroup SectionObject Object Constants
	 *  List of object constants.
	 *  @{
	*/

	const int Directory = HDFQL_DIRECTORY;

	const int File = HDFQL_FILE;

	const int Group = HDFQL_GROUP;

	const int Dataset = HDFQL_DATASET;

	const int Attribute = HDFQL_ATTRIBUTE;

	const int SoftLink = HDFQL_SOFT_LINK;

	const int ExternalLink = HDFQL_EXTERNAL_LINK;

	/** @} */



	/** @defgroup SectionDataType Data Type Constants
	 *  List of data type constants.
	 *  @{
	*/

	const int Tinyint = HDFQL_TINYINT;

	const int UnsignedTinyint = HDFQL_UNSIGNED_TINYINT;

	const int Smallint = HDFQL_SMALLINT;

	const int UnsignedSmallint = HDFQL_UNSIGNED_SMALLINT;

	const int Int = HDFQL_INT;

	const int UnsignedInt = HDFQL_UNSIGNED_INT;

	const int Bigint = HDFQL_BIGINT;

	const int UnsignedBigint = HDFQL_UNSIGNED_BIGINT;

	const int Float = HDFQL_FLOAT;

	const int Double = HDFQL_DOUBLE;

	const int Char = HDFQL_CHAR;

	const int Vartinyint = HDFQL_VARTINYINT;

	const int UnsignedVartinyint = HDFQL_UNSIGNED_VARTINYINT;

	const int Varsmallint = HDFQL_VARSMALLINT;

	const int UnsignedVarsmallint = HDFQL_UNSIGNED_VARSMALLINT;

	const int Varint = HDFQL_VARINT;

	const int UnsignedVarint = HDFQL_UNSIGNED_VARINT;

	const int Varbigint = HDFQL_VARBIGINT;

	const int UnsignedVarbigint = HDFQL_UNSIGNED_VARBIGINT;

	const int Varfloat = HDFQL_VARFLOAT;

	const int Vardouble = HDFQL_VARDOUBLE;

	const int Varchar = HDFQL_VARCHAR;

	const int Opaque = HDFQL_OPAQUE;

	const int Bitfield = HDFQL_BITFIELD;

	const int Enumeration = HDFQL_ENUMERATION;

	const int Compound = HDFQL_COMPOUND;

	const int Reference = HDFQL_REFERENCE;

	/** @} */



	/** @defgroup SectionScope Scope Constants
	 *  List of scope constants.
	 *  @{
	*/

	const int Global = HDFQL_GLOBAL;

	const int Local = HDFQL_LOCAL;

	/** @} */



	/** @defgroup SectionOrder Order Constants
	 *  List of order constants.
	 *  @{
	*/

	const int Tracked = HDFQL_TRACKED;

	const int Indexed = HDFQL_INDEXED;

	/** @} */



	/** @defgroup SectionStorageType Storage Type Constants
	 *  List of storage type constants.
	 *  @{
	*/

	const int Contiguous = HDFQL_CONTIGUOUS;

	const int Compact = HDFQL_COMPACT;

	const int Chunked = HDFQL_CHUNKED;

	/** @} */



	/** @defgroup SectionStorageAllocation Storage Allocation Constants
	 *  List of storage allocation constants.
	 *  @{
	*/

	const int Early = HDFQL_EARLY;

	const int Incremental = HDFQL_INCREMENTAL;

	const int Late = HDFQL_LATE;

	/** @} */



	/** @defgroup SectionEndianness Endianness Constants
	 *  List of endianness constants.
	 *  @{
	*/

	const int LittleEndian = HDFQL_LITTLE_ENDIAN;

	const int BigEndian = HDFQL_BIG_ENDIAN;

	const int MixedEndian = HDFQL_MIXED_ENDIAN;

	/** @} */



	/** @defgroup SectionCharset Charset Constants
	 *  List of charset constants.
	 *  @{
	*/

	const int Ascii = HDFQL_ASCII;

	const int Utf8 = HDFQL_UTF8;

	/** @} */



	/** @defgroup SectionFill Fill Constants
	 *  List of fill constants.
	 *  @{
	*/

	const int FillDefault = HDFQL_FILL_DEFAULT;

	const int FillDefined = HDFQL_FILL_DEFINED;

	const int FillNever = HDFQL_FILL_NEVER;

	const int FillUndefined = HDFQL_FILL_UNDEFINED;

	/** @} */



	/** @defgroup SectionLibraryBounds Library Bounds Constants
	 *  List of library bounds constants.
	 *  @{
	*/

	const int Earliest = HDFQL_EARLIEST;

	const int Latest = HDFQL_LATEST;

	const int Version18 = HDFQL_VERSION_18;

	/** @} */



	/** @defgroup SectionStatus Status Constants
	 *  List of status constants.
	 *  @{
	*/

	const int Success = HDFQL_SUCCESS;

	const int ErrorParse = HDFQL_ERROR_PARSE;

	const int ErrorNotSpecified = HDFQL_ERROR_NOT_SPECIFIED;

	const int ErrorNotFound = HDFQL_ERROR_NOT_FOUND;

	const int ErrorNoAccess = HDFQL_ERROR_NO_ACCESS;

	const int ErrorNotOpen = HDFQL_ERROR_NOT_OPEN;

	const int ErrorInvalidName = HDFQL_ERROR_INVALID_NAME;

	const int ErrorInvalidFile = HDFQL_ERROR_INVALID_FILE;

	const int ErrorNotSupported = HDFQL_ERROR_NOT_SUPPORTED;

	const int ErrorNotEnoughSpace = HDFQL_ERROR_NOT_ENOUGH_SPACE;

	const int ErrorNotEnoughMemory = HDFQL_ERROR_NOT_ENOUGH_MEMORY;

	const int ErrorAlreadyExists = HDFQL_ERROR_ALREADY_EXISTS;

	const int ErrorEmpty = HDFQL_ERROR_EMPTY;

	const int ErrorFull = HDFQL_ERROR_FULL;

	const int ErrorBeforeFirst = HDFQL_ERROR_BEFORE_FIRST;

	const int ErrorAfterLast = HDFQL_ERROR_AFTER_LAST;

	const int ErrorOutsideLimit = HDFQL_ERROR_OUTSIDE_LIMIT;

	const int ErrorNoAddress = HDFQL_ERROR_NO_ADDRESS;

	const int ErrorUnexpectedType = HDFQL_ERROR_UNEXPECTED_TYPE;

	const int ErrorUnexpectedDataType = HDFQL_ERROR_UNEXPECTED_DATA_TYPE;

	const int ErrorUnexpectedStorageType = HDFQL_ERROR_UNEXPECTED_STORAGE_TYPE;

	const int ErrorDanglingLink = HDFQL_ERROR_DANGLING_LINK;

	const int ErrorNotRegistered = HDFQL_ERROR_NOT_REGISTERED;

	const int ErrorInvalidRegularExpression = HDFQL_ERROR_INVALID_REGULAR_EXPRESSION;

	const int ErrorInvalidSelection = HDFQL_ERROR_INVALID_SELECTION;

	const int ErrorUnknown = HDFQL_ERROR_UNKNOWN;

	/** @} */



	/** @defgroup SectionProgrammingLanguages Programming Languages Constants
	 *  List of programming languages constants.
	 *  @{
	*/

	const int _C = _HDFQL_C;

	const int _Cpp = _HDFQL_CPP;

	const int _Java = _HDFQL_JAVA;

	const int _Python = _HDFQL_PYTHON;

	const int _CSharp = _HDFQL_CSHARP;

	const int _Fortran = _HDFQL_FORTRAN;

	const int _R = _HDFQL_R;

	/** @} */



	/** @defgroup SectionExecute Execute Functions
	 *  List of execute functions.
	 *  @{
	*/

	_HDFQL_EXPORT_SYMBOL int execute(const char *script);

	_HDFQL_EXPORT_SYMBOL int execute(const std::string &script);

	_HDFQL_EXPORT_SYMBOL int execute(const std::stringstream &script);

	_HDFQL_EXPORT_SYMBOL int executeGetStatus(void);

	/** @} */



	/** @defgroup SectionError Error Functions
	 *  List of error functions.
	 *  @{
	*/

	_HDFQL_EXPORT_SYMBOL int errorGetLine(void);

	_HDFQL_EXPORT_SYMBOL int errorGetPosition(void);

	_HDFQL_EXPORT_SYMBOL char *errorGetMessage(void);

	/** @} */



	/** @defgroup SectionCursor Cursor Functions
	 *  List of cursor functions.
	 *  @{
	*/

	_HDFQL_EXPORT_SYMBOL int cursorInitialize(Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int cursorUse(const Cursor *cursor);

	_HDFQL_EXPORT_SYMBOL int cursorUseDefault(void);

	_HDFQL_EXPORT_SYMBOL int cursorClear(Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int cursorClone(Cursor *cursorClone);

	_HDFQL_EXPORT_SYMBOL int cursorClone(const Cursor *cursorOriginal, Cursor *cursorClone);

	_HDFQL_EXPORT_SYMBOL int cursorGetDataType(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int cursorGetCount(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int subcursorGetCount(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int cursorGetPosition(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int subcursorGetPosition(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int cursorFirst(Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int subcursorFirst(Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int cursorLast(Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int subcursorLast(Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int cursorNext(Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int subcursorNext(Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int cursorPrevious(Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int subcursorPrevious(Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int cursorAbsolute(int position);

	_HDFQL_EXPORT_SYMBOL int cursorAbsolute(Cursor *cursor, int position);

	_HDFQL_EXPORT_SYMBOL int subcursorAbsolute(int position);

	_HDFQL_EXPORT_SYMBOL int subcursorAbsolute(Cursor *cursor, int position);

	_HDFQL_EXPORT_SYMBOL int cursorRelative(int position);

	_HDFQL_EXPORT_SYMBOL int cursorRelative(Cursor *cursor, int position);

	_HDFQL_EXPORT_SYMBOL int subcursorRelative(int position);

	_HDFQL_EXPORT_SYMBOL int subcursorRelative(Cursor *cursor, int position);

	_HDFQL_EXPORT_SYMBOL char *cursorGetTinyint(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL char *subcursorGetTinyint(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL unsigned char *cursorGetUnsignedTinyint(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL unsigned char *subcursorGetUnsignedTinyint(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL short *cursorGetSmallint(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL short *subcursorGetSmallint(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL unsigned short *cursorGetUnsignedSmallint(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL unsigned short *subcursorGetUnsignedSmallint(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int *cursorGetInt(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL int *subcursorGetInt(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL unsigned int *cursorGetUnsignedInt(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL unsigned int *subcursorGetUnsignedInt(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL long long *cursorGetBigint(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL long long *subcursorGetBigint(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL unsigned long long *cursorGetUnsignedBigint(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL unsigned long long *subcursorGetUnsignedBigint(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL float *cursorGetFloat(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL float *subcursorGetFloat(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL double *cursorGetDouble(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL double *subcursorGetDouble(const Cursor *cursor = NULL);

	_HDFQL_EXPORT_SYMBOL char *cursorGetChar(const Cursor *cursor = NULL);

	/** @} */



	/** @defgroup SectionVariable Variable Functions
	 *  List of variable functions.
	 *  @{
	*/

	_HDFQL_EXPORT_SYMBOL int variableRegister(const void *variable);

	#ifdef _HDFQL_STD_ARRAY
		template <typename T, size_t SIZE>
		_HDFQL_EXPORT_SYMBOL int variableRegister(const std::array <T, SIZE> &variable)
		{

			return hdfql_variable_register(variable.data());

		}
	#endif

	template <typename T>
	_HDFQL_EXPORT_SYMBOL int variableRegister(const std::vector <T> &variable)
	{

		return hdfql_variable_register(variable.data());

	}

	_HDFQL_EXPORT_SYMBOL int variableTransientRegister(const void *variable);

	#ifdef _HDFQL_STD_ARRAY
		template <typename T, size_t SIZE>
		_HDFQL_EXPORT_SYMBOL int variableTransientRegister(const std::array <T, SIZE> &variable)
		{

			return hdfql_variable_transient_register(variable.data());

		}
	#endif

	template <typename T>
	_HDFQL_EXPORT_SYMBOL int variableTransientRegister(const std::vector <T> &variable)
	{

		return hdfql_variable_transient_register(variable.data());

	}

	_HDFQL_EXPORT_SYMBOL int variableUnregister(const void *variable);

	#ifdef _HDFQL_STD_ARRAY
		template <typename T, size_t SIZE>
		_HDFQL_EXPORT_SYMBOL int variableUnregister(const std::array <T, SIZE> &variable)
		{

			return hdfql_variable_unregister(variable.data());

		}
	#endif

	template <typename T>
	_HDFQL_EXPORT_SYMBOL int variableUnregister(const std::vector <T> &variable)
	{

		return hdfql_variable_unregister(variable.data());

	}

	_HDFQL_EXPORT_SYMBOL int variableUnregisterAll(void);

	_HDFQL_EXPORT_SYMBOL int variableGetNumber(const void *variable);

	#ifdef _HDFQL_STD_ARRAY
		template <typename T, size_t SIZE>
		_HDFQL_EXPORT_SYMBOL int variableGetNumber(const std::array <T, SIZE> &variable)
		{

			return hdfql_variable_get_number(variable.data());

		}
	#endif

	template <typename T>
	_HDFQL_EXPORT_SYMBOL int variableGetNumber(const std::vector <T> &variable)
	{

		return hdfql_variable_get_number(variable.data());

	}

	_HDFQL_EXPORT_SYMBOL int variableGetDataType(const void *variable);

	#ifdef _HDFQL_STD_ARRAY
		template <typename T, size_t SIZE>
		_HDFQL_EXPORT_SYMBOL int variableGetDataType(const std::array <T, SIZE> &variable)
		{

			return hdfql_variable_get_data_type(variable.data());

		}
	#endif

	template <typename T>
	_HDFQL_EXPORT_SYMBOL int variableGetDataType(const std::vector <T> &variable)
	{

		return hdfql_variable_get_data_type(variable.data());

	}

	_HDFQL_EXPORT_SYMBOL int variableGetCount(const void *variable);

	#ifdef _HDFQL_STD_ARRAY
		template <typename T, size_t SIZE>
		_HDFQL_EXPORT_SYMBOL int variableGetCount(const std::array <T, SIZE> &variable)
		{

			return hdfql_variable_get_count(variable.data());

		}
	#endif

	template <typename T>
	_HDFQL_EXPORT_SYMBOL int variableGetCount(const std::vector <T> &variable)
	{

		return hdfql_variable_get_count(variable.data());

	}

	_HDFQL_EXPORT_SYMBOL int variableGetSize(const void *variable);

	#ifdef _HDFQL_STD_ARRAY
		template <typename T, size_t SIZE>
		_HDFQL_EXPORT_SYMBOL int variableGetSize(const std::array <T, SIZE> &variable)
		{

			return hdfql_variable_get_size(variable.data());

		}
	#endif

	template <typename T>
	_HDFQL_EXPORT_SYMBOL int variableGetSize(const std::vector <T> &variable)
	{

		return hdfql_variable_get_size(variable.data());

	}

	_HDFQL_EXPORT_SYMBOL int variableGetDimensionCount(const void *variable);

	#ifdef _HDFQL_STD_ARRAY
		template <typename T, size_t SIZE>
		_HDFQL_EXPORT_SYMBOL int variableGetDimensionCount(const std::array <T, SIZE> &variable)
		{

			return hdfql_variable_get_dimension_count(variable.data());

		}
	#endif

	template <typename T>
	_HDFQL_EXPORT_SYMBOL int variableGetDimensionCount(const std::vector <T> &variable)
	{

		return hdfql_variable_get_dimension_count(variable.data());

	}

	_HDFQL_EXPORT_SYMBOL long long variableGetDimension(const void *variable, int index);

	#ifdef _HDFQL_STD_ARRAY
		template <typename T, size_t SIZE>
		_HDFQL_EXPORT_SYMBOL long long variableGetDimension(const std::array <T, SIZE> &variable, int index)
		{

			return hdfql_variable_get_dimension(variable.data(), index);

		}
	#endif

	template <typename T>
	_HDFQL_EXPORT_SYMBOL long long variableGetDimension(const std::vector <T> &variable, int index)
	{

		return hdfql_variable_get_dimension(variable.data(), index);

	}

	/** @} */



	/** @defgroup SectionMPI MPI Functions
	 *  List of MPI functions.
	 *  @{
	*/

	_HDFQL_EXPORT_SYMBOL int mpiGetSize(void);

	_HDFQL_EXPORT_SYMBOL int mpiGetRank(void);

	/** @} */



	/** @defgroup SectionInitializerFinalizer Initializer/Finalizer Functions
	 *  List of initializer/finalizer functions.
	 *  @{
	*/

	_HDFQL_EXPORT_SYMBOL void initialize(void);

	_HDFQL_EXPORT_SYMBOL void finalize(void);

	/** @} */

};



/** @} */



#endif   // _HDFQL_CPP_HEADER

