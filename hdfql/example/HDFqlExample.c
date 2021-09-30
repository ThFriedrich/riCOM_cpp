// Copyright (C) 2016-2021
// This file is part of the Hierarchical Data Format query language (HDFql)
// For more information about HDFql, please visit the website http://www.hdfql.com

// $Rev: 329 $



//===========================================================
// The following program provides an example on how to use HDFql in C. The output of executing it would be similar to this:
//
// HDFql version: 2.4.0
// File in use: example.h5
// Attribute value: 12.400000
// Dataset value (through variable):
// 1
// 2
// 3
// 4
// 5
// 6
// Dataset value (through cursor):
// 1
// 2
// 3
// 4
// 5
// 6
// Dataset size (in bytes): 24
//
//===========================================================

// include HDFql C header file (make sure it can be found by the C compiler)
#include <stdlib.h>
#include <stdio.h>
#include "HDFql.h"

int main(int argc, char *argv[])
{

	// declare variables
	HDFQL_CURSOR my_cursor;
	char script[1024];
	int values[3][2];
	int x;
	int y;

	// display HDFql version in use
	printf("HDFql version: %s\n", HDFQL_VERSION);

	// create an HDF5 file named "example.h5"
	hdfql_execute("CREATE FILE example.h5");

	// use (i.e. open) HDF5 file "example.h5"
	hdfql_execute("USE FILE example.h5");

	// show (i.e. get) HDF5 file currently in use and populate HDFql default cursor with it
	hdfql_execute("SHOW USE FILE");

	// display HDF5 file currently in use
	hdfql_cursor_first(NULL);
	printf("File in use: %s\n", hdfql_cursor_get_char(NULL));

	// create an attribute named "example_attribute" of data type float with an initial value of 12.4
	hdfql_execute("CREATE ATTRIBUTE example_attribute AS FLOAT VALUES(12.4)");

	// select (i.e. read) data from attribute "example_attribute" and populate HDFql default cursor with it
	hdfql_execute("SELECT FROM example_attribute");

	// display value of attribute "example_attribute"
	hdfql_cursor_first(NULL);
	printf("Attribute value: %f\n", *hdfql_cursor_get_float(NULL));

	// create a dataset named "example_dataset" of data type int of two dimensions (size 3x2)
	hdfql_execute("CREATE DATASET example_dataset AS INT(3, 2)");

	// populate variable "values" with certain values
	for(x = 0; x < 3; x++)
	{
		for(y = 0; y < 2; y++)
		{
			values[x][y] = x * 2 + y + 1;
		}
	}

	// register variable "values" for subsequent use (by HDFql)
	hdfql_variable_register(values);

	// insert (i.e. write) values from variable "values" into dataset "example_dataset"
	sprintf(script, "INSERT INTO example_dataset VALUES FROM MEMORY %d", hdfql_variable_get_number(values));
	hdfql_execute(script);

	// populate variable "values" with zeros (i.e. reset variable)
	for(x = 0; x < 3; x++)
	{
		for(y = 0; y < 2; y++)
		{
			values[x][y] = 0;
		}
	}

	// select (i.e. read) data from dataset "example_dataset" and populate variable "values" with it
	sprintf(script, "SELECT FROM example_dataset INTO MEMORY %d", hdfql_variable_get_number(values));
	hdfql_execute(script);

	// unregister variable "values" as it is no longer used/needed (by HDFql)
	hdfql_variable_unregister(values);

	// display content of variable "values"
	printf("Dataset value (through variable):\n");
	for(x = 0; x < 3; x++)
	{
		for(y = 0; y < 2; y++)
		{
			printf("%d\n", values[x][y]);
		}
	}

	// select (i.e. read) data from dataset "example_dataset" again and populate HDFql default cursor with it
	hdfql_execute("SELECT FROM example_dataset");

	// display content of HDFql default cursor
	printf("Dataset value (through cursor):\n");
	while(hdfql_cursor_next(NULL) == HDFQL_SUCCESS)
	{
		printf("%d\n", *hdfql_cursor_get_int(NULL));
	}

	// initialize cursor "my_cursor"
	hdfql_cursor_initialize(&my_cursor);

	// use cursor "my_cursor"
	hdfql_cursor_use(&my_cursor);

	// show (i.e. get) size (in bytes) of dataset "example_dataset" and populate cursor "my_cursor" with it
	hdfql_execute("SHOW SIZE example_dataset");

	// display content of cursor "my_cursor"
	hdfql_cursor_first(NULL);
	printf("Dataset size (in bytes): %lld\n", *hdfql_cursor_get_bigint(NULL));

	return EXIT_SUCCESS;

}

