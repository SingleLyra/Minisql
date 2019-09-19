//
// Created by Weiye Chen on 2019-04-30.
//

#ifndef MINISQL_API_H
#define MINISQL_API_H

#include <cstdio>
#include <string>
#include <vector>
#include <exception>
#include "../CatalogManager/Attribute.h"
#include "../CatalogManager/Catalog.h"
#include "../CatalogManager/CatalogManager.h"
#include "../Interpreter/SQL_Satatement.h"
#include "../IndexManager/Index.h"
#include "../RecordManager/RecordManager.h"

extern CatalogManager cm;
extern RecordManager rm;

enum class API_Error_Type{
    INTERPRETATION_ERROR,
    REFERENCE_ERROR,
    PARAMETER_ERROR
};

class API_Error : exception{
public:
    API_Error(API_Error_Type type, string message){
        this->type = type;
        this->message = message;
    }
    API_Error_Type type;
    string message;
};

class currentCatalog
{
    static Catalog* currentTableCat;
};

class API {
public:
    API(){}
    bool CreateTable (string tblName, vector<Attribute>* attributes);
    bool CreateIndex (string tblName, string idxName, vector<string>* attrList);
    bool Select (string tblName, vector<Condition>* conditions, vector<string>* selectedAttrList);
    bool InsertAll (string tblName, vector<ValueWithType> * values);
    bool InsertWithAttr (string tblName, vector<KeyValueTuple>* keyValues);
    bool DropTable (string tblName);
    bool DropIndex (string tblName, string idxName);
    bool Delete (string tblName, vector<Condition>* conditions);
private:
    static bool conditionCmp(Condition& a, Condition& b);
    void MergeConditions(vector<Condition> * conditions);
    bool validateCondition(Record& rec, Condition & cond);
    int ValueCmp(const Value &a, const Value &b, AttributeType type);
    bool SelectRecord (string tblName, vector<Condition>* conditions, vector<BP_POINTER>& vReturnPointers);
    Value& retrieveAttrValueFromRecord(Record & rec, string attrName);
    // If duplicates are found, return true;
    bool CheckDuplicateKey(string tblName, vector<ValueWithType> * values);
    Catalog* currentTableCat;
    bool InsertAllRecordToIndex(Index * index, string tblName, string attrName);
};

bool conditionCmp(Condition& a, Condition& b);

#endif //MINISQL_API_H
