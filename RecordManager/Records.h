//
// Created by Weiye Chen on 2019-06-07.
//

#ifndef MINISQL_RECORDS_H
#define MINISQL_RECORDS_H

#include <cstdlib>
#include <string>
#include <vector>
#include <map>
#include "../CatalogManager/Attribute.h"
#include "../minisql.h"
#include "../Interpreter/SQL_Satatement.h"
#include "../CatalogManager/Catalog.h"
#include "../CatalogManager/CatalogManager.h"
#include "../IndexManager/Index.h"

using namespace std;
// A record is a vector of key-value tuples.
typedef vector<KeyValueTuple> Record;

extern CatalogManager cm;

class Records
{
public:

    Records(){};
    // `Records` Constructor: Existing table, retrieve record content.
    Records(string tblName);
    // The name of the table which records belong to.
    string tableName;
    // The size of each record.
    int sizePerRecord;
    // Maximum number of records per block;
    int maxRecordsNumPerBlock;
    // A pointer to vector of all fields in the table. (Records.h)
    vector<Attribute>* attributes;
    // A reference to the catalog object of the table
    Catalog* catalog;

    /* ------------- Methods ------------- */
    // Create a new record file.
    bool createRecordFile();
    // Delete all records from a table.
    bool deleteAllRecords();
    // Insert a single record into the table.
    bool insertRecord(Record * rec);
    bool insertRecord(vector<ValueWithType> * values);
    // Insert multiple records into the table.
//    bool insertRecords(vector<Record> * vRec);
    // Retrieve a single record. (The record needs to be read in buffer before actually being retrieved.)
    bool retrieveRecord(int recordID, Record& record);
    bool retrieveRecord(int blockID, int offset, Record& record);
    // Get value of a specified attribute from record
    // Value& retrieveAttrValueFromRecord(Record & rec, string attrName);
    int getRecordSize() {return (catalog->table_block_cnt-1) * maxRecordsNumPerBlock + catalog->usage;}
};

class RecordsError : exception
{
public:
    RecordsError(string message){this->message = message;}
    string message;
};

#endif //MINISQL_RECORDS_H
