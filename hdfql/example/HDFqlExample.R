# Copyright (C) 2016-2021
# This file is part of the Hierarchical Data Format query language (HDFql)
# For more information about HDFql, please visit the website http://www.hdfql.com

# $Rev: 143 $



#===========================================================
# The following program provides an example on how to use HDFql in R. The output of executing it would be similar to this:
#
# HDFql version: 2.4.0
# File in use: example.h5
# Attribute value: 12.400000
# Dataset value (through variable):
# 1
# 2
# 3
# 4
# 5
# 6
# Dataset value (through cursor):
# 1
# 2
# 3
# 4
# 5
# 6
# Dataset size (in bytes): 24
#
#===========================================================

# load HDFql R wrapper (make sure it can be found by the R interpreter)
source("HDFql.R")

# display HDFql version in use
print(paste("HDFql version:", HDFQL_VERSION))

# create an HDF5 file named "example.h5"
hdfql_execute("CREATE FILE example.h5")

# use (i.e. open) HDF5 file "example.h5"
hdfql_execute("USE FILE example.h5")

# show (i.e. get) HDF5 file currently in use and populate HDFql default cursor with it
hdfql_execute("SHOW USE FILE")

# display HDF5 file currently in use
hdfql_cursor_first()
print(paste("File in use:", hdfql_cursor_get_char()))

# create an attribute named "example_attribute" of data type float with an initial value of 12.4
hdfql_execute("CREATE ATTRIBUTE example_attribute AS FLOAT VALUES(12.4)")

# select (i.e. read) data from attribute "example_attribute" and populate HDFql default cursor with it
hdfql_execute("SELECT FROM example_attribute")

# display value of attribute "example_attribute"
hdfql_cursor_first()
print(paste("Attribute value:", hdfql_cursor_get_float()))

# create a dataset named "example_dataset" of data type int of two dimensions (size 3x2)
hdfql_execute("CREATE DATASET example_dataset AS INT(3, 2)")

# create variable "values" and populate it with certain values
values <- array(dim = c(3, 2))
for(x in 1:2)
{
	for(y in 1:3)
	{
		values[y, x] <- as.integer(x * 3 + y - 3)
	}
}

# register variable "values" for subsequent use (by HDFql)
hdfql_variable_register(values)

# insert (i.e. write) values from variable "values" into dataset "example_dataset"
hdfql_execute(paste("INSERT INTO example_dataset VALUES FROM MEMORY", hdfql_variable_get_number(values)))

# unregister variable "values" as it is no longer used/needed (by HDFql)
hdfql_variable_unregister(values)

# populate variable "values" with zeros (i.e. reset variable)
for(x in 1:2)
{
	for(y in 1:3)
	{
		values[y, x] <- as.integer(0)
	}
}

# register variable "values" for subsequent use (by HDFql)
hdfql_variable_register(values)

# select (i.e. read) data from dataset "example_dataset" and populate variable "values" with it
hdfql_execute(paste("SELECT FROM example_dataset INTO MEMORY", hdfql_variable_get_number(values)))

# unregister variable "values" as it is no longer used/needed (by HDFql)
hdfql_variable_unregister(values)

# display content of variable "values"
print("Dataset value (through variable):")
for(x in 1:2)
{
	for(y in 1:3)
	{
		print(values[y, x])
	}
}

# select (i.e. read) data from dataset "example_dataset" again and populate HDFql default cursor with it
hdfql_execute("SELECT FROM example_dataset")

# display content of HDFql default cursor
print("Dataset value (through cursor):")
while(hdfql_cursor_next() == HDFQL_SUCCESS)
{
	print(hdfql_cursor_get_int())
}

# create cursor "my_cursor"
my_cursor <- hdfql_cursor()

# use cursor "my_cursor"
hdfql_cursor_use(my_cursor)

# show (i.e. get) size (in bytes) of dataset "example_dataset" and populate cursor "my_cursor" with it
hdfql_execute("SHOW SIZE example_dataset")

# display content of cursor "my_cursor"
hdfql_cursor_first()
print(paste("Dataset size (in bytes):", hdfql_cursor_get_bigint()))

