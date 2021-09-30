# Copyright (C) 2016-2021
# This file is part of the Hierarchical Data Format query language (HDFql)
# For more information about HDFql, please visit the website http://www.hdfql.com

# $Rev: 329 $



#===========================================================
# The following script provides an example on how to use HDFql in Python. The output of executing it would be similar to this:
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

# import HDFql module (make sure it can be found by the Python interpreter)
import HDFql
import numpy

# display HDFql version in use
print("HDFql version: %s" % HDFql.VERSION)

# create an HDF5 file named "example.h5"
HDFql.execute("CREATE FILE example.h5")

# use (i.e. open) HDF5 file "example.h5"
HDFql.execute("USE FILE example.h5")

# show (i.e. get) HDF5 file currently in use and populate HDFql default cursor with it
HDFql.execute("SHOW USE FILE")

# display HDF5 file currently in use
HDFql.cursor_first()
print("File in use: %s" % HDFql.cursor_get_char())

# create an attribute named "example_attribute" of data type float with an initial value of 12.4
HDFql.execute("CREATE ATTRIBUTE example_attribute AS FLOAT VALUES(12.4)")

# select (i.e. read) data from attribute "example_attribute" and populate HDFql default cursor with it
HDFql.execute("SELECT FROM example_attribute")

# display value of attribute "example_attribute"
HDFql.cursor_first()
print("Attribute value: %f" % HDFql.cursor_get_float())

# create a dataset named "example_dataset" of data type int of two dimensions (size 3x2)
HDFql.execute("CREATE DATASET example_dataset AS INT(3, 2)")

# create variable "values" and populate it with certain values
values = numpy.zeros((3, 2), dtype = numpy.int32)
for x in range(3):
	for y in range(2):
		values[x][y] = x * 2 + y + 1

# register variable "values" for subsequent use (by HDFql)
HDFql.variable_register(values)

# insert (i.e. write) values from variable "values" into dataset "example_dataset"
HDFql.execute("INSERT INTO example_dataset VALUES FROM MEMORY %d" % HDFql.variable_get_number(values))

# populate variable "values" with zeros (i.e. reset variable)
for x in range(3):
	for y in range(2):
		values[x][y] = 0

# select (i.e. read) data from dataset "example_dataset" and populate variable "values" with it
HDFql.execute("SELECT FROM example_dataset INTO MEMORY %d" % HDFql.variable_get_number(values))

# unregister variable "values" as it is no longer used/needed (by HDFql)
HDFql.variable_unregister(values)

# display content of variable "values"
print("Dataset value (through variable):")
for x in range(3):
	for y in range(2):
		print(values[x][y])

# select (i.e. read) data from dataset "example_dataset" again and populate HDFql default cursor with it
HDFql.execute("SELECT FROM example_dataset")

# display content of HDFql default cursor
print("Dataset value (through cursor):")
while HDFql.cursor_next() == HDFql.SUCCESS:
	print(HDFql.cursor_get_int())

# create cursor "my_cursor"
my_cursor = HDFql.Cursor()

# use cursor "my_cursor"
HDFql.cursor_use(my_cursor)

# show (i.e. get) size (in bytes) of dataset "example_dataset" and populate cursor "my_cursor" with it
HDFql.execute("SHOW SIZE example_dataset")

# display content of cursor "my_cursor"
HDFql.cursor_first()
print("Dataset size (in bytes): %d" % HDFql.cursor_get_bigint())

