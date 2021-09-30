// Copyright (C) 2016-2021
// This file is part of the Hierarchical Data Format query language (HDFql)
// For more information about HDFql, please visit the website http://www.hdfql.com

// $Rev: 309 $



#ifndef _HDFQL_STRUCTURES_HEADER
	#define _HDFQL_STRUCTURES_HEADER



/*! \mainpage HDFql C Structures Documentation
 *
 *  Library of HDFql C structures to manage HDF5 files
 *
 *  \n
 *  \n
 *  \li \ref SectionCursor
 *  \n
 *  \n
 *  \li \ref SectionVariable
*/



//===========================================================
// ENABLE USAGE FROM C++
//===========================================================
#ifdef __cplusplus
	extern "C"
	{
#endif



//===========================================================
// STRUCTURES
//===========================================================
/** @defgroup SectionCursor Structures
 *  List of cursor structures.
 *  @{
*/

typedef struct
{
	int                         data_type;
	int                         count;
	int                         position;
	struct hdfql_cursor_element *head;
	struct hdfql_cursor_element *tail;
	struct hdfql_cursor_element *current;
}HDFQL_CURSOR;

struct hdfql_cursor_element
{
	void                        *data;
	int                         data_type;
	int                         count;
	int                         position;
	int                         size;
	struct hdfql_cursor_element *next;
	struct hdfql_cursor_element *previous;
};

/** @} */



/** @defgroup SectionVariable Structures
 *  List of variable structures.
 *  @{
*/

#if defined(__GNUC__) || defined(__GNUG__) || defined(__clang__) || ((defined(__ICC) || defined(__INTEL_COMPILER)) && !defined(_WIN32))   // GNU GCC/G++, Clang and Intel ICC/ICPC (for Linux/macOS)

	typedef struct
	{
		#if __x86_64__
			unsigned long long count;
		#else   // 32 bit
			unsigned int       count;
		#endif
		void                   *address;
	}HDFQL_VARIABLE_LENGTH;

#elif defined(_MSC_VER) || ((defined(__ICC) || defined(__INTEL_COMPILER)) && defined(_WIN32))   // Microsoft Visual Studio and Intel ICC/ICPC (for Windows)

	typedef struct
	{
		#if _WIN64
			unsigned long long count;
		#else   // 32 bit
			unsigned int       count;
		#endif
		void                   *address;
	}HDFQL_VARIABLE_LENGTH;

#else   // UNKNOWN COMPILER/ARCHITECTURE (ASSUMED TO BE 64 bit)

	typedef struct
	{
		unsigned long long count;
		void               *address;
	}HDFQL_VARIABLE_LENGTH;

#endif

/** @} */



#ifdef __cplusplus
	}
#endif



#endif   // _HDFQL_STRUCTURES_HEADER

