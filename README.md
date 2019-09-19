# MINISQL Project

**A Lab Project in the Course of Database System in ZJU implemented by Weiye Chen and Xinning Zhang**

**浙江大学数据库系统课程实验作业 完成人：陈玮烨 张辛宁**

Advisor 指导老师: Jianling Sun 孙建伶

> Updated: June 18, 2019
>
> 更新时间：2019年6月18日

The goal of this project is to simulate a simple DBMS with C++.

该项目主要内容是使用C++模拟一个简易和初级的数据库管理系统。



**Table of Content 目录**

[TOC]

## Capacity 功能

* #### Supported SQL statements 支持的SQL语句

  * CREATE TABLE *table_name* (*attribute_name1* *data_type1* \[PRIMARY KEY | UNIQUE] \[, …(other create definitions)][\[PRIMARY KEY (attribute_name_PK)]);
  * CREATE INDEX *index_name* ON *table_name* (*attribute_name*);
  * INSERT INTO *table_name* VALUES (*value1*[, value2[, ….]]);
  * SELECT * FROM *table_name* [WHERE *condition1*[ AND *condition2* [ AND… ]]];
  * SELECT *attribute_name1* [, *attribute_name2* [, …...]] FROM *table_name* [WHERE *condition1*[ AND *condition2* [ AND… ]]];
  * DELETE FROM *table_name* [WHERE *condition1*[ AND *condition2* [ AND… ]]];
  * DROP TABLE *table_name*;
  * DROP INDEX *index_name* ON *table_name*;
  * EXECFILE *file_name*;
  * QUIT;

* #### Features 系统特征

  * Syntax validation 语法检查
    * Errors in syntax would be thrown, and the execution of the SQL command stops.
    * Errorless statements would be parsed.
  * Disk-memory data exchange (data persistence and buffer management) 数据持久化和内存管理
    * Data are stored on disk.
    * Data in memory would be written to disk, while data in disk can be loaded into memory.
    * Advanced LRU for memory management.
  * Indexing 索引
    * Implemented with B+ tree structure.

## Structure 架构

The system is comprised of several parts, namely: 系统由以下几个部分组成：

* #### Interpreter 解释器
  
    * This part acts as the driver of the entire system.
        * Initializing the program
        * Retrieving instructions specified by the users by using SQL statements and interpreting the syntaxes to instructions which could be handled by the parts underneath.
        * The interpreter is responsible for raising for syntax error in SQL statement.
    * If the statements provided by the users are executable ones, the interpreter calls interfaces provided by the API to conduct specific queries on the data.
    
* #### API 接口
  
    * API is responsible for calling specific functions provided by data engines by following the instructions interpreted by the interpreter.
    * API is also responsible for making execution plans with the information offered by the catalog manager.
    
* #### Data Engines 数据引擎
  
    * ##### Catalog Manager 目录管理器
      
        * Catalog Manager is responsible for maintaining all information on the schemas of the databases. Including:
            * Table names, attributes, primary key, indices.
            * Record size, block usage.
            * Accessor of the information including but not limited to those listed above.
        * Catalog Manager also manages the changes to tables — including creations, deletions, insertions to the tables. 
        
    * ##### Index Manager 索引管理器
      
        * Index manager is designed to maintain indices on tables using B+ trees. 
        * For querying with criteria containing equivalency and range.
        
    * ##### Record Manager 记录管理器
    
        * Record Manager creates record file for data storage, though data are firstly handled in the buffer, i.e. the writing process is handled by the Buffer Engine.
        * Therefore, the record manager is built as we integrate buffer management module with processes of data retrievals and changes. The record manager specifies the way they integrate. 
        * Record Manager takes instructions where accesses on record-level data are required, e.g. insertions and deletions of records, retrievals of records, modifications to make on records.
        * Record Manager also measures the size of each record as well as the number of records in tables. In addition, record manager stipulates the way we store data in BLOCKs (the basic units we store data on disk) as well.
    
* #### Buffer Engine 内存管理引擎

    * When the system starts running, the data are primarily processed in memory. However, memory size is often limited, and it's not possible to load every record in a big data table onto the memory, a well-managed buffer engine is essential. Buffer Engine, in our project named Buffer Manager, is in charge of getting data to memory from disk as well as writing dirty data blocks back to disk to make the changes persist. 
    * The minimum data-unit handled by buffer engine is a block. A block has a size of 4096 bytes. Record data are transferred from disk to memory in blocks. Any changes within a block must be reflected on the disk, which, however, doesn't necessarily have to be done as soon as each change happens, since we can hold those dirty blocks with multiple changes in memory and flush them all together to reduce I/O cost.

* #### Data file 数据文件

    * Our data are stored on disk, with a format specified in Record Manager. Record Manager would keep track of the size of records and data, and the offsets in blocks required to access data.



## Implementation Document 系统实现说明

Implementation document is written in Chinese. 实现说明使用中文描述。

See Group report. (Updated June 19, 2019)

### 1. Interpreter 解释器



### 2. API 接口



### 3. Catalog Manager 目录管理器



### 4. Record Manager 记录管理器



### 5. Index Manager 索引管理器



### 6. Buffer Manager 内存管理器



## Diligent contributors to this project 努力奋斗的队员们

#### 陈玮烨

主要负责Interpreter, API, Catalog Maneger和Record Manager的编写。

> 感谢靠谱的队友！两人合作很愉快，在这期间也学习到了很多。

#### 张辛宁

主要负责Catalog Manager, Index Manage和Buffer Manager的编写，并大力支持了上层的API的构建。

> 



### 