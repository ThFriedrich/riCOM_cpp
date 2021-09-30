// Copyright (C) 2016-2021
// This file is part of the Hierarchical Data Format query language (HDFql)
// For more information about HDFql, please visit the website http://www.hdfql.com

// $Rev: 355 $



//===========================================================
// The following program provides an example on how to use HDFql in C++. The output of executing it would be similar to this:
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

// include HDFql C++ header file (make sure it can be found by the C++ compiler)
#include <cstdlib>
#include <iostream>
#include "HDFql.hpp"

int main(int argc, char *argv[])
{

	// declare variables
	HDFql::Cursor myCursor;
	std::stringstream script;
	int values[3][2];
	int x;
	int y;

	// display HDFql version in use
	std::cout << "HDFql version: " << HDFql::Version << std::endl;

	// create an HDF5 file named "example.h5"
	HDFql::execute("CREATE FILE example.h5");

	// use (i.e. open) file "example.h5"
	HDFql::execute("USE FILE example.h5");

	// show (i.e. get) HDF5 file currently in use and populate HDFql default cursor with it
	HDFql::execute("SHOW USE FILE");

	// display HDF5 file currently in use
	HDFql::cursorFirst();
	std::cout << "File in use: " << HDFql::cursorGetChar() << std::endl;

	// create an attribute named "example_attribute" of data type float with an initial value of 12.4
	HDFql::execute("CREATE ATTRIBUTE example_attribute AS FLOAT VALUES(12.4)");

	// select (i.e. read) data from attribute "example_attribute" and populate HDFql default cursor with it
	HDFql::execute("SELECT FROM example_attribute");

	// display value of attribute "example_attribute"
	HDFql::cursorFirst();
	std::cout << "Attribute value: " << *HDFql::cursorGetFloat() << std::endl;

	// create a dataset named "example_dataset" of data type int of two dimensions (size 3x2)
	HDFql::execute("CREATE DATASET example_dataset AS INT(3, 2)");

	// populate variable "values" with certain values
	for(x = 0; x < 3; x++)
	{
		for(y = 0; y < 2; y++)
		{
			values[x][y] = x * 2 + y + 1;
		}
	}

	// register variable "values" for subsequent use (by HDFql)
	HDFql::variableRegister(values);

	// insert (i.e. write) values from variable "values" into dataset "example_dataset"
	script << "INSERT INTO example_dataset VALUES FROM MEMORY " << HDFql::variableGetNumber(values);
	HDFql::execute(script);

	// populate variable "values" with zeros (i.e. reset variable)
	for(x = 0; x < 3; x++)
	{
		for(y = 0; y < 2; y++)
		{
			values[x][y] = 0;
		}
	}

	// select (i.e. read) data from dataset "example_dataset" and populate variable "values" with it
	script.str("");
	script << "SELECT FROM example_dataset INTO MEMORY " << HDFql::variableGetNumber(values);
	HDFql::execute(script);

	// unregister variable "values" as it is no longer used/needed (by HDFql)
	HDFql::variableUnregister(values);

	// display content of variable "values"
	std::cout << "Dataset value (through variable):" << std::endl;
	for(x = 0; x < 3; x++)
	{
		for(y = 0; y < 2; y++)
		{
			std::cout << values[x][y] << std::endl;
		}
	}

	// select (i.e. read) data from dataset "example_dataset" again and populate HDFql default cursor with it
	HDFql::execute("SELECT FROM example_dataset");

	// display content of HDFql default cursor
	std::cout << "Dataset value (through cursor):" << std::endl;
	while(HDFql::cursorNext() == HDFql::Success)
	{
		std::cout << *HDFql::cursorGetInt() << std::endl;
	}

	// use cursor "myCursor"
	HDFql::cursorUse(&myCursor);

	// show (i.e. get) size (in bytes) of dataset "example_dataset" and populate cursor "myCursor" with it
	HDFql::execute("SHOW SIZE example_dataset");

	// display content of cursor "myCursor"
	HDFql::cursorFirst();
	std::cout << "Dataset size (in bytes): " << *HDFql::cursorGetBigint() << std::endl;

	return EXIT_SUCCESS;

}

