// Copyright (C) 2016-2021
// This file is part of the Hierarchical Data Format query language (HDFql)
// For more information about HDFql, please visit the website http://www.hdfql.com

// $Rev: 329 $



//===========================================================
// The following program provides an example on how to use HDFql in Java. The output of executing it would be similar to this:
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

// import HDFql package (make sure it can be found by the Java compiler/JVM)
import as.hdfql.*;

public class HDFqlExample
{
	public static void main(String args[])
	{
		// declare variables
		HDFqlCursor myCursor;
		int values[][];
		int x;
		int y;

		// display HDFql version in use
		System.out.println("HDFql version: " + HDFql.VERSION);

		// create an HDF5 file named "example.h5"
		HDFql.execute("CREATE FILE example.h5");

		// use (i.e. open) HDF5 file "example.h5"
		HDFql.execute("USE FILE example.h5");

		// show (i.e. get) HDF5 file currently in use and populate HDFql default cursor with it
		HDFql.execute("SHOW USE FILE");

		// display HDF5 file currently in use
		HDFql.cursorFirst();
		System.out.println("File in use: " + HDFql.cursorGetChar());

		// create an attribute named "example_attribute" of data type float with an initial value of 12.4
		HDFql.execute("CREATE ATTRIBUTE example_attribute AS FLOAT VALUES(12.4)");

		// select (i.e. read) data from attribute "example_attribute" and populate HDFql default cursor with it
		HDFql.execute("SELECT FROM example_attribute");

		// display value of attribute "example_attribute"
		HDFql.cursorFirst();
		System.out.println("Attribute value: " + HDFql.cursorGetFloat());

		// create a dataset named "example_dataset" of data type int of two dimensions (size 3x2)
		HDFql.execute("CREATE DATASET example_dataset AS INT(3, 2)");

		// create variable "values" and populate it with certain values
		values = new int[3][2];
		for(x = 0; x < 3; x++)
		{
			for(y = 0; y < 2; y++)
			{
				values[x][y] = x * 2 + y + 1;
			}
		}

		// register variable "values" for subsequent use (by HDFql)
		HDFql.variableRegister(values);

		// insert (i.e. write) values from variable "values" into dataset "example_dataset"
		HDFql.execute("INSERT INTO example_dataset VALUES FROM MEMORY " + HDFql.variableGetNumber(values));

		// populate variable "values" with zeros (i.e. reset variable)
		for(x = 0; x < 3; x++)
		{
			for(y = 0; y < 2; y++)
			{
				values[x][y] = 0;
			}
		}

		// select (i.e. read) data from dataset "example_dataset" and populate variable "values" with it
		HDFql.execute("SELECT FROM example_dataset INTO MEMORY " + HDFql.variableGetNumber(values));

		// unregister variable "values" as it is no longer used/needed (by HDFql)
		HDFql.variableUnregister(values);

		// display content of variable "values"
		System.out.println("Dataset value (through variable):");
		for(x = 0; x < 3; x++)
		{
			for(y = 0; y < 2; y++)
			{
				System.out.println(values[x][y]);
			}
		}

		// select (i.e. read) data from dataset "example_dataset" again and populate HDFql default cursor with it
		HDFql.execute("SELECT FROM example_dataset");

		// display content of HDFql default cursor
		System.out.println("Dataset value (through cursor):");
		while(HDFql.cursorNext() == HDFql.SUCCESS)
		{
			System.out.println(HDFql.cursorGetInt());
		}

		// create cursor "myCursor"
		myCursor = new HDFqlCursor();

		// use cursor "myCursor"
		HDFql.cursorUse(myCursor);

		// show (i.e. get) size (in bytes) of dataset "example_dataset" and populate cursor "myCursor" with it
		HDFql.execute("SHOW SIZE example_dataset");

		// display content of cursor "myCursor"
		HDFql.cursorFirst();
		System.out.println("Dataset size (in bytes): " + HDFql.cursorGetBigint());
	}
}

