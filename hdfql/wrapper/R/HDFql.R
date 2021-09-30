# Copyright (C) 2016-2021
# This file is part of the Hierarchical Data Format query language (HDFql)
# For more information about HDFql, please visit the website http://www.hdfql.com

# $Rev: 358 $



#===========================================================
# LOAD HDFQL R WRAPPER SHARED LIBRARY
#===========================================================
hdfql_operating_system = Sys.info()["sysname"]
if (hdfql_operating_system == "Windows")
{
	dyn.load("HDFqlR.dll")
	hdfql_shared_library <- "HDFqlR"
} else if (hdfql_operating_system == "Linux")
{
	dyn.load("libHDFqlR.so")
	hdfql_shared_library <- "libHDFqlR"
} else   # macOS
{
	dyn.load("libHDFqlR.dylib")
	hdfql_shared_library <- "libHDFqlR.dylib"
}
rm(hdfql_operating_system)



#===========================================================
# CONSTANTS
#===========================================================
HDFQL_VERSION <- "2.4.0"

HDFQL_YES <- 0

HDFQL_NO <- -1

HDFQL_ENABLED <- 0

HDFQL_DISABLED <- -1

HDFQL_UNLIMITED <- -1

HDFQL_UNDEFINED <- -1


HDFQL_DIRECTORY <- 1

HDFQL_FILE <- 2

HDFQL_GROUP <- 4

HDFQL_DATASET <- 8

HDFQL_ATTRIBUTE <- 16

HDFQL_SOFT_LINK <- 32

HDFQL_EXTERNAL_LINK <- 64


HDFQL_TINYINT <- 1

HDFQL_UNSIGNED_TINYINT <- 2

HDFQL_SMALLINT <- 4

HDFQL_UNSIGNED_SMALLINT <- 8

HDFQL_INT <- 16

HDFQL_UNSIGNED_INT <- 32

HDFQL_BIGINT <- 64

HDFQL_UNSIGNED_BIGINT <- 128

HDFQL_FLOAT <- 256

HDFQL_DOUBLE <- 512

HDFQL_CHAR <- 1024

HDFQL_VARTINYINT <- 2048

HDFQL_UNSIGNED_VARTINYINT <- 4096

HDFQL_VARSMALLINT <- 8192

HDFQL_UNSIGNED_VARSMALLINT <- 16384

HDFQL_VARINT <- 32768

HDFQL_UNSIGNED_VARINT <- 65536

HDFQL_VARBIGINT <- 131072

HDFQL_UNSIGNED_VARBIGINT <- 262144

HDFQL_VARFLOAT <- 524288

HDFQL_VARDOUBLE <- 1048576

HDFQL_VARCHAR <- 2097152

HDFQL_OPAQUE <- 4194304

HDFQL_BITFIELD <- 8388608

HDFQL_ENUMERATION <- 16777216

HDFQL_COMPOUND <- 33554432

HDFQL_REFERENCE <- 67108864


HDFQL_GLOBAL <- 1

HDFQL_LOCAL <- 2


HDFQL_TRACKED <- 1

HDFQL_INDEXED <- 2


HDFQL_CONTIGUOUS <- 1

HDFQL_COMPACT <- 2

HDFQL_CHUNKED <- 4


HDFQL_EARLY <- 1

HDFQL_INCREMENTAL <- 2

HDFQL_LATE <- 4


HDFQL_LITTLE_ENDIAN <- 1

HDFQL_BIG_ENDIAN <- 2

HDFQL_MIXED_ENDIAN <- 4


HDFQL_ASCII <- 1

HDFQL_UTF8 <- 2


HDFQL_FILL_DEFAULT <- 1

HDFQL_FILL_DEFINED <- 2

HDFQL_FILL_NEVER <- 4

HDFQL_FILL_UNDEFINED <- 8


HDFQL_EARLIEST <- 1

HDFQL_LATEST <- 2

HDFQL_VERSION_18 <- 4


HDFQL_SUCCESS <- 0

HDFQL_ERROR_PARSE <- -1

HDFQL_ERROR_NOT_SPECIFIED <- -2

HDFQL_ERROR_NOT_FOUND <- -3

HDFQL_ERROR_NO_ACCESS <- -4

HDFQL_ERROR_NOT_OPEN <- -5

HDFQL_ERROR_INVALID_NAME <- -6

HDFQL_ERROR_INVALID_FILE <- -7

HDFQL_ERROR_NOT_SUPPORTED <- -8

HDFQL_ERROR_NOT_ENOUGH_SPACE <- -9

HDFQL_ERROR_NOT_ENOUGH_MEMORY <- -10

HDFQL_ERROR_ALREADY_EXISTS <- -11

HDFQL_ERROR_EMPTY <- -12

HDFQL_ERROR_FULL <- -13

HDFQL_ERROR_BEFORE_FIRST <- -14

HDFQL_ERROR_AFTER_LAST <- -15

HDFQL_ERROR_OUTSIDE_LIMIT <- -16

HDFQL_ERROR_NO_ADDRESS <- -17

HDFQL_ERROR_UNEXPECTED_TYPE <- -18

HDFQL_ERROR_UNEXPECTED_DATA_TYPE <- -19

HDFQL_ERROR_UNEXPECTED_STORAGE_TYPE <- -20

HDFQL_ERROR_DANGLING_LINK <- -21

HDFQL_ERROR_NOT_REGISTERED <- -22

HDFQL_ERROR_INVALID_REGULAR_EXPRESSION <- -23

HDFQL_ERROR_INVALID_SELECTION <- -24

HDFQL_ERROR_UNKNOWN <- -99



#===========================================================
# CLASSES
#===========================================================
hdfql_cursor_ <- setRefClass("hdfql_cursor_", field = list(address = "numeric"), method = list(finalize = function(){.Call("_hdfql_cursor_destroy_R", .self$address, PACKAGE = hdfql_shared_library)}))



#===========================================================
# GENERAL FUNCTIONS
#===========================================================
hdfql_execute <- function(script)
{

	if (is.character(script) == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_DATA_TYPE)
	}

	return (.Call("hdfql_execute_R", script, PACKAGE = hdfql_shared_library))

}



hdfql_execute_get_status <- function()
{

	return (.Call("hdfql_execute_get_status_R", PACKAGE = hdfql_shared_library))

}



hdfql_error_get_line <- function()
{

	return (.Call("hdfql_error_get_line_R", PACKAGE = hdfql_shared_library))

}



hdfql_error_get_position <- function()
{

	return (.Call("hdfql_error_get_position_R", PACKAGE = hdfql_shared_library))

}



hdfql_error_get_message <- function()
{

	return (.Call("hdfql_error_get_message_R", PACKAGE = hdfql_shared_library))

}



#===========================================================
# CURSOR FUNCTIONS
#===========================================================
hdfql_cursor <- function()
{

	cursor <- hdfql_cursor_()

	cursor$address = .Call("_hdfql_cursor_create_R", PACKAGE = hdfql_shared_library)

	return (cursor)

}



hdfql_cursor_initialize <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_initialize_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_initialize_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_use <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_use_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_use_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_use_default <- function()
{

	return (.Call("hdfql_cursor_use_default_R", PACKAGE = hdfql_shared_library))

}



hdfql_cursor_clear <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_clear_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_clear_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_clone <- function(cursor_original = NULL, cursor_clone)
{

	if (is.null(cursor_original) == TRUE)
	{
		if (is(cursor_clone, "hdfql_cursor_") == FALSE)
		{
			return (HDFQL_ERROR_UNEXPECTED_TYPE)
		}
		return (.Call("hdfql_cursor_clone_R", NULL, cursor_clone$address, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor_original, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	if (is(cursor_clone, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_clone_R", cursor_original$address, cursor_clone$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_get_data_type <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_get_data_type_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_get_data_type_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_get_count <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_get_count_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_get_count_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_get_count <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_get_count_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_get_count_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_get_position <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_get_position_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_get_position_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_get_position <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_get_position_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_get_position_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_first <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_first_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_first_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_first <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_first_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_first_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_last <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_last_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_last_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_last <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_last_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_last_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_next <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_next_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_next_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_next <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_next_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_next_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_previous <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_previous_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_previous_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_previous <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_previous_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_previous_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_absolute <- function(cursor = NULL, position)
{

	if (is.null(cursor) == TRUE)
	{
		if (is.integer(position) == FALSE)
		{
			return (HDFQL_ERROR_UNEXPECTED_DATA_TYPE)
		}
		return (.Call("hdfql_cursor_absolute_R", NULL, position, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	if (is.integer(position) == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_DATA_TYPE)
	}

	return (.Call("hdfql_cursor_absolute_R", cursor$address, position, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_absolute <- function(cursor = NULL, position)
{

	if (is.null(cursor) == TRUE)
	{
		if (is.integer(position) == FALSE)
		{
			return (HDFQL_ERROR_UNEXPECTED_DATA_TYPE)
		}
		return (.Call("hdfql_subcursor_absolute_R", NULL, position, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	if (is.integer(position) == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_DATA_TYPE)
	}

	return (.Call("hdfql_subcursor_absolute_R", cursor$address, position, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_relative <- function(cursor = NULL, position)
{

	if (is.null(cursor) == TRUE)
	{
		if (is.integer(position) == FALSE)
		{
			return (HDFQL_ERROR_UNEXPECTED_DATA_TYPE)
		}
		return (.Call("hdfql_cursor_relative_R", NULL, position, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	if (is.integer(position) == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_DATA_TYPE)
	}

	return (.Call("hdfql_cursor_relative_R", cursor$address, position, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_relative <- function(cursor = NULL, position)
{

	if (is.null(cursor) == TRUE)
	{
		if (is.integer(position) == FALSE)
		{
			return (HDFQL_ERROR_UNEXPECTED_DATA_TYPE)
		}
		return (.Call("hdfql_subcursor_relative_R", NULL, position, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	if (is.integer(position) == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_DATA_TYPE)
	}

	return (.Call("hdfql_subcursor_relative_R", cursor$address, position, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_get_tinyint <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_get_tinyint_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_get_tinyint_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_get_tinyint <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_get_tinyint_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_get_tinyint_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_get_unsigned_tinyint <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_get_unsigned_tinyint_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_get_unsigned_tinyint_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_get_unsigned_tinyint <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_get_unsigned_tinyint_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_get_unsigned_tinyint_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_get_smallint <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_get_smallint_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_get_smallint_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_get_smallint <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_get_smallint_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_get_smallint_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_get_unsigned_smallint <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_get_unsigned_smallint_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_get_unsigned_smallint_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_get_unsigned_smallint <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_get_unsigned_smallint_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_get_unsigned_smallint_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_get_int <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_get_int_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_get_int_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_get_int <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_get_int_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_get_int_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_get_unsigned_int <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_get_unsigned_int_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_get_unsigned_int_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_get_unsigned_int <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_get_unsigned_int_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_get_unsigned_int_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_get_bigint <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_get_bigint_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_get_bigint_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_get_bigint <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_get_bigint_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_get_bigint_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_get_unsigned_bigint <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_get_unsigned_bigint_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_get_unsigned_bigint_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_get_unsigned_bigint <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_get_unsigned_bigint_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_get_unsigned_bigint_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_get_float <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_get_float_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_get_float_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_get_float <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_get_float_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_get_float_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_get_double <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_get_double_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_get_double_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_subcursor_get_double <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_subcursor_get_double_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_subcursor_get_double_R", cursor$address, PACKAGE = hdfql_shared_library))

}



hdfql_cursor_get_char <- function(cursor = NULL)
{

	if (is.null(cursor) == TRUE)
	{
		return (.Call("hdfql_cursor_get_char_R", NULL, PACKAGE = hdfql_shared_library))
	}

	if (is(cursor, "hdfql_cursor_") == FALSE)
	{
		return (HDFQL_ERROR_UNEXPECTED_TYPE)
	}

	return (.Call("hdfql_cursor_get_char_R", cursor$address, PACKAGE = hdfql_shared_library))

}



#===========================================================
# VARIABLE FUNCTIONS
#===========================================================
hdfql_variable_register <- function(variable)
{

	return (.Call("hdfql_variable_register_R", variable, PACKAGE = hdfql_shared_library))

}



hdfql_variable_transient_register <- function(variable)
{

	return (.Call("hdfql_variable_transient_register_R", variable, PACKAGE = hdfql_shared_library))

}



hdfql_variable_unregister <- function(variable)
{

	return (.Call("hdfql_variable_unregister_R", variable, PACKAGE = hdfql_shared_library))

}



hdfql_variable_unregister_all <- function()
{

	return (.Call("hdfql_variable_unregister_all_R", PACKAGE = hdfql_shared_library))

}



hdfql_variable_get_number <- function(variable)
{

	return (.Call("hdfql_variable_get_number_R", variable, PACKAGE = hdfql_shared_library))

}



hdfql_variable_get_data_type <- function(variable)
{

	return (.Call("hdfql_variable_get_data_type_R", variable, PACKAGE = hdfql_shared_library))

}



hdfql_variable_get_count <- function(variable)
{

	return (.Call("hdfql_variable_get_count_R", variable, PACKAGE = hdfql_shared_library))

}



hdfql_variable_get_size <- function(variable)
{

	return (.Call("hdfql_variable_get_size_R", variable, PACKAGE = hdfql_shared_library))

}



hdfql_variable_get_dimension_count <- function(variable)
{

	return (.Call("hdfql_variable_get_dimension_count_R", variable, PACKAGE = hdfql_shared_library))

}



hdfql_variable_get_dimension <- function(variable, index)
{

	return (.Call("hdfql_variable_get_dimension_R", variable, index, PACKAGE = hdfql_shared_library))

}



#===========================================================
# MPI FUNCTIONS
#===========================================================
hdfql_mpi_get_size <- function()
{

	return (.Call("hdfql_mpi_get_size_R", PACKAGE = hdfql_shared_library))

}



hdfql_mpi_get_rank <- function()
{

	return (.Call("hdfql_mpi_get_rank_R", PACKAGE = hdfql_shared_library))

}



#===========================================================
# REFRESH R OBJECT TABLE
#===========================================================
cacheMetaData(1)

