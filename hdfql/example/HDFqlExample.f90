! Copyright (C) 2016-2021
! This file is part of the Hierarchical Data Format query language (HDFql)
! For more information about HDFql, please visit the website http://www.hdfql.com

! $Rev: 131 $



!===========================================================
! The following program provides an example on how to use HDFql in Fortran. The output of executing it would be similar to this:
!
! HDFql version: 2.4.0
! File in use: example.h5
! Attribute value: 12.400000
! Dataset value (through variable):
! 1
! 2
! 3
! 4
! 5
! 6
! Dataset value (through cursor):
! 1
! 2
! 3
! 4
! 5
! 6
! Dataset size (in bytes): 24
!
!===========================================================

PROGRAM HDFqlExample

    ! use HDFql module (make sure it can be found by the Fortran compiler)
    USE HDFql

    ! declare variables
    TYPE(HDFQL_CURSOR) :: my_cursor
    CHARACTER :: variable_number
    INTEGER, DIMENSION(3, 2) :: values
    INTEGER :: state
    INTEGER :: x
    INTEGER :: y

    ! display HDFql version in use
    WRITE(*, *) "HDFql version: ", HDFQL_VERSION

    ! create an HDF5 file named "example.h5"
    state = hdfql_execute("CREATE FILE example.h5")

    ! use (i.e. open) HDF5 file "example.h5"
    state = hdfql_execute("USE FILE example.h5")

    ! show (i.e. get) HDF5 file currently in use and populate HDFql default cursor with it
    state = hdfql_execute("SHOW USE FILE")

    ! display HDF5 file currently in use
    state = hdfql_cursor_first()
    WRITE(*, *) "File in use: ", hdfql_cursor_get_char()

    ! create an attribute named "example_attribute" of data type float with an initial value of 12.4
    state = hdfql_execute("CREATE ATTRIBUTE example_attribute AS FLOAT VALUES(12.4)")

    ! select (i.e. read) data from attribute "example_attribute" and populate HDFql default cursor with it
    state = hdfql_execute("SELECT FROM example_attribute")

    ! display value of attribute "example_attribute"
    state = hdfql_cursor_first()
    WRITE(*, *) "Attribute value: ", hdfql_cursor_get_float()

    ! create a dataset named "example_dataset" of data type int of two dimensions (size 3x2)
    state = hdfql_execute("CREATE DATASET example_dataset AS INT(3, 2)");

    ! populate variable "values" with certain values
    DO x = 1, 2
        DO y = 1, 3
            values(y, x) = x * 3 + y - 3
        END DO
    END DO

    ! register variable "values" for subsequent use (by HDFql)
    state = hdfql_variable_register(values)
    WRITE(variable_number, "(I0)") state

    ! insert (i.e. write) values from variable "values" into dataset "example_dataset"
    state = hdfql_execute("INSERT INTO example_dataset VALUES FROM MEMORY " // variable_number)

    ! populate variable "values" with zeros (i.e. reset variable)
    DO x = 1, 2
        DO y = 1, 3
            values(y, x) = 0
        END DO
    END DO

    ! select (i.e. read) data from dataset "example_dataset" and populate variable "values" with it
    state = hdfql_execute("SELECT FROM example_dataset INTO MEMORY " // variable_number)

    ! unregister variable "values" as it is no longer used/needed (by HDFql)
    state = hdfql_variable_unregister(values)

    ! display content of variable "values"
    WRITE(*, *) "Dataset value (through variable):"
    DO x = 1, 2
        DO y = 1, 3
            WRITE(*, *) values(y, x)
        END DO
    END DO

    ! select (i.e. read) data from dataset "example_dataset" again and populate HDFql default cursor with it
    state = hdfql_execute("SELECT FROM example_dataset")

    ! display content of HDFql default cursor
    WRITE(*, *) "Dataset value (through cursor):"
    DO WHILE(hdfql_cursor_next() .EQ. HDFQL_SUCCESS)
        WRITE(*, *) hdfql_cursor_get_int()
    END DO

    ! use cursor "my_cursor"
    state = hdfql_cursor_use(my_cursor)

    ! show (i.e. get) size (in bytes) of dataset "example_dataset" and populate cursor "my_cursor" with it
    state = hdfql_execute("SHOW SIZE example_dataset")

    ! display content of cursor "my_cursor"
    state = hdfql_cursor_first()
    WRITE(*, *) "Dataset size (in bytes): ", hdfql_cursor_get_bigint()

END PROGRAM

