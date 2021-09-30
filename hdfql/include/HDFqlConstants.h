// Copyright (C) 2016-2021
// This file is part of the Hierarchical Data Format query language (HDFql)
// For more information about HDFql, please visit the website http://www.hdfql.com

// $Rev: 358 $



#ifndef _HDFQL_CONSTANTS_HEADER
	#define _HDFQL_CONSTANTS_HEADER



/*! \mainpage HDFql C Constants Documentation
 *
 *  Library of HDFql C constants to manage HDF5 files
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
*/



//===========================================================
// ENABLE USAGE FROM C++
//===========================================================
#ifdef __cplusplus
	extern "C"
	{
#endif



//===========================================================
// DEFINITIONS
//===========================================================
/** @defgroup SectionGeneral General Constants
 *  List of general constants.
 *  @{
*/

#define HDFQL_VERSION                          "2.4.0"

#define HDFQL_YES                              0

#define HDFQL_NO                               -1

#define HDFQL_ENABLED                          0

#define HDFQL_DISABLED                         -1

#define HDFQL_UNLIMITED                        -1

#define HDFQL_UNDEFINED                        -1

/** @} */



/** @defgroup SectionObject Object Constants
 *  List of object constants.
 *  @{
*/

#define HDFQL_DIRECTORY                        1

#define HDFQL_FILE                             2

#define HDFQL_GROUP                            4

#define HDFQL_DATASET                          8

#define HDFQL_ATTRIBUTE                        16

#define HDFQL_SOFT_LINK                        32

#define HDFQL_EXTERNAL_LINK                    64

/** @} */



/** @defgroup SectionDataType Data Type Constants
 *  List of data type constants.
 *  @{
*/

#define HDFQL_TINYINT                          1

#define HDFQL_UNSIGNED_TINYINT                 2

#define HDFQL_SMALLINT                         4

#define HDFQL_UNSIGNED_SMALLINT                8

#define HDFQL_INT                              16

#define HDFQL_UNSIGNED_INT                     32

#define HDFQL_BIGINT                           64

#define HDFQL_UNSIGNED_BIGINT                  128

#define HDFQL_FLOAT                            256

#define HDFQL_DOUBLE                           512

#define HDFQL_CHAR                             1024

#define HDFQL_VARTINYINT                       2048

#define HDFQL_UNSIGNED_VARTINYINT              4096

#define HDFQL_VARSMALLINT                      8192

#define HDFQL_UNSIGNED_VARSMALLINT             16384

#define HDFQL_VARINT                           32768

#define HDFQL_UNSIGNED_VARINT                  65536

#define HDFQL_VARBIGINT                        131072

#define HDFQL_UNSIGNED_VARBIGINT               262144

#define HDFQL_VARFLOAT                         524288

#define HDFQL_VARDOUBLE                        1048576

#define HDFQL_VARCHAR                          2097152

#define HDFQL_OPAQUE                           4194304

#define HDFQL_BITFIELD                         8388608

#define HDFQL_ENUMERATION                      16777216

#define HDFQL_COMPOUND                         33554432

#define HDFQL_REFERENCE                        67108864

/** @} */



/** @defgroup SectionScope Scope Constants
 *  List of scope constants.
 *  @{
*/

#define HDFQL_GLOBAL                           1

#define HDFQL_LOCAL                            2

/** @} */



/** @defgroup SectionOrder Order Constants
 *  List of order constants.
 *  @{
*/

#define HDFQL_TRACKED                          1

#define HDFQL_INDEXED                          2

/** @} */



/** @defgroup SectionStorageType Storage Type Constants
 *  List of storage type constants.
 *  @{
*/

#define HDFQL_CONTIGUOUS                       1

#define HDFQL_COMPACT                          2

#define HDFQL_CHUNKED                          4

/** @} */



/** @defgroup SectionStorageAllocation Storage Allocation Constants
 *  List of storage allocation constants.
 *  @{
*/

#define HDFQL_EARLY                            1

#define HDFQL_INCREMENTAL                      2

#define HDFQL_LATE                             4

/** @} */



/** @defgroup SectionEndianness Endianness Constants
 *  List of endianness constants.
 *  @{
*/

#define HDFQL_LITTLE_ENDIAN                    1

#define HDFQL_BIG_ENDIAN                       2

#define HDFQL_MIXED_ENDIAN                     4

/** @} */



/** @defgroup SectionCharset Charset Constants
 *  List of charset constants.
 *  @{
*/

#define HDFQL_ASCII                            1

#define HDFQL_UTF8                             2

/** @} */



/** @defgroup SectionFill Fill Constants
 *  List of fill constants.
 *  @{
*/

#define HDFQL_FILL_DEFAULT                     1

#define HDFQL_FILL_DEFINED                     2

#define HDFQL_FILL_NEVER                       4

#define HDFQL_FILL_UNDEFINED                   8

/** @} */



/** @defgroup SectionLibraryBounds Library Bounds Constants
 *  List of library bounds constants.
 *  @{
*/

#define HDFQL_EARLIEST                         1

#define HDFQL_LATEST                           2

#define HDFQL_VERSION_18                       4

/** @} */



/** @defgroup SectionStatus Status Constants
 *  List of status constants.
 *  @{
*/

#define HDFQL_SUCCESS                          0

#define HDFQL_ERROR_PARSE                      -1

#define HDFQL_ERROR_NOT_SPECIFIED              -2

#define HDFQL_ERROR_NOT_FOUND                  -3

#define HDFQL_ERROR_NO_ACCESS                  -4

#define HDFQL_ERROR_NOT_OPEN                   -5

#define HDFQL_ERROR_INVALID_NAME               -6

#define HDFQL_ERROR_INVALID_FILE               -7

#define HDFQL_ERROR_NOT_SUPPORTED              -8

#define HDFQL_ERROR_NOT_ENOUGH_SPACE           -9

#define HDFQL_ERROR_NOT_ENOUGH_MEMORY          -10

#define HDFQL_ERROR_ALREADY_EXISTS             -11

#define HDFQL_ERROR_EMPTY                      -12

#define HDFQL_ERROR_FULL                       -13

#define HDFQL_ERROR_BEFORE_FIRST               -14

#define HDFQL_ERROR_AFTER_LAST                 -15

#define HDFQL_ERROR_OUTSIDE_LIMIT              -16

#define HDFQL_ERROR_NO_ADDRESS                 -17

#define HDFQL_ERROR_UNEXPECTED_TYPE            -18

#define HDFQL_ERROR_UNEXPECTED_DATA_TYPE       -19

#define HDFQL_ERROR_UNEXPECTED_STORAGE_TYPE    -20

#define HDFQL_ERROR_DANGLING_LINK              -21

#define HDFQL_ERROR_NOT_REGISTERED             -22

#define HDFQL_ERROR_INVALID_REGULAR_EXPRESSION -23

#define HDFQL_ERROR_INVALID_SELECTION          -24

#define HDFQL_ERROR_UNKNOWN                    -99

/** @} */



/** @defgroup SectionProgrammingLanguages Programming Languages Constants
 *  List of programming languages constants.
 *  @{
*/

#define _HDFQL_C                               1

#define _HDFQL_CPP                             2

#define _HDFQL_JAVA                            4

#define _HDFQL_PYTHON                          8

#define _HDFQL_CSHARP                          16

#define _HDFQL_FORTRAN                         32

#define _HDFQL_R                               64

/** @} */



#ifdef __cplusplus
	}
#endif



#endif   // _HDFQL_CONSTANTS_HEADER

