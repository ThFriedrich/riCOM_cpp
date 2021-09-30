@ ECHO OFF
REM Copyright (C) 2016-2021
REM This file is part of the Hierarchical Data Format query language (HDFql)
REM For more information about HDFql, please visit the website http://www.hdfql.com

REM $Rev: 309 $



REM changes to environment variables are local (i.e. their values are restored at the end of this batch file)
SETLOCAL

REM set path environment variable with the directory where the HDFql shared library is located
SET PATH=..\lib;%PATH%

REM launch HDFqlCLI (command-line interface) and pass all user-defined parameters to it
HDFqlCLI.exe %*

