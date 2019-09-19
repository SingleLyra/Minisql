cmake_minimum_required(VERSION 3.12)
project(MINISQL)

set(CMAKE_CXX_STANDARD 14)

include_directories(.)
include_directories(API)
include_directories(BufferManager)
include_directories(CatalogManager)
include_directories(IndexManager)
include_directories(Interpreter)
include_directories(RecordManager)

add_executable(MINISQL
        API/API.cpp
        API/API.h
        BufferManager/BufferManager.cpp
        BufferManager/BufferManager.h
        CatalogManager/Attribute.cpp
        CatalogManager/Attribute.h
        CatalogManager/Catalog.cpp
        CatalogManager/Catalog.h
        CatalogManager/CatalogManager.cpp
        CatalogManager/CatalogManager.h
        IndexManager/Index.cpp
        IndexManager/Index.h
        IndexManager/node.cpp
        IndexManager/node.hpp
        Interpreter/Interpreter.cpp
        Interpreter/Interpreter.h
        Interpreter/SQL_Satatement.cpp
        Interpreter/SQL_Satatement.h
        Interpreter/SyntaxError.cpp
        Interpreter/SyntaxError.h
        RecordManager/RecordManager.cpp
        RecordManager/RecordManager.h
        RecordManager/Records.cpp
        RecordManager/Records.h
        CMakeLists.txt
        main.cpp
        minisql.cpp
        minisql.h)
