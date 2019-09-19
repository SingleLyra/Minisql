//
// Created by Robert Chen on 2019-06-02.
//

#ifndef MINISQL_SQL_SATATEMENT_H
#define MINISQL_SQL_SATATEMENT_H

#include "../CatalogManager/Attribute.h"
#include "SyntaxError.h"
#include <string>
#include <vector>

using namespace std;

/// Type of SQL statement
enum class statementType {
    SELECT,
    INSERT,
    INSERTwithAttr,
    CREATETABLE,
    CREATEINDEX,
    DROPTABLE,
    DROPINDEX,
    DELETE,
    QUIT,
    EXECFILE
};


/// Type of operator in conditions
enum class operatorType{
    UNDEFINED,
    EQ,     // Equal to
    GT,     // Greater than
    LT,     // Less than
    GTE,    // Greater than or equal to
    LTE,    // Less than or equal to
    NEQ,    // Not equal to
    ISNULL, // is null
    ISNOTNULL // is not null
};

typedef struct Value {
    int intValue;        // KeyValueTuple for attributes of int
    float floatValue;    // Data for attributes of float
    char charValue[256]; // Data for attributes of varchar
    int charValueLen;    // Length of varchar
} Value;

typedef struct ValueWithType {
    Value value;
    AttributeType type;
} ValueWithType;

/// Attribute-value tuple
class KeyValueTuple {
public:
    KeyValueTuple(){
//        assert(1);
//        cerr << "God Damn it!" << endl;
    };
    KeyValueTuple(string a){
        attrName = a;assert(1);
//        cerr << "God Damn it! 2" << endl;
    }
    ~KeyValueTuple(){
//        cerr << "God Damn it! Delete" << endl;
    }
    // setValue function is not available with int
    bool setValue(const int & i)
    {
        value.intValue = i;
        this->type = AttributeType::INT;
		return true;
    }
    bool setValue(const float & f)
    {
        value.floatValue = f;
        this->type = AttributeType::FLOAT;
        return true;
    };
    bool setValue(char* ch, int l)
    {
#ifdef _WIN32
        strcpy_s(value.charValue, ch);
#elif defined(__APPLE__) || defined(__MACH__) || defined(unix) || defined(__unix__) || defined(__unix)
        strcpy(value.charValue, ch);
#endif
        this->type = AttributeType::STRING;
        value.charValueLen = l;
        return true;
    };
    bool setValue(string &s, int l)
    {
        int len = s.size();
        for (int ii = 0; ii < l; ii++)
        {
            if (ii > len)
            {
                value.charValue[ii] = 0;
            }
            else
                value.charValue[ii] = s[ii];
        }
        this->type = AttributeType::STRING;
        value.charValueLen = l;
        return true;
    }
    string attrName;     // Name of the specified attribute
    Value value;
    AttributeType type;

private:
//    AttributeType getAttrType(){ return this->attr.type; }
};


/// This class concludes instances of conditions in the WHERE clause of SQL statements
class Condition : public KeyValueTuple{
public:
    Condition() {};
    Condition(string attr, operatorType opType, ValueWithType v)
    {
        this->attrName = attr;
        this->type = v.type;
        this->value = v.value;
        this->opType = opType;
    }
    operatorType opType;
    string tableName;
};

// TODO: Would it be better if we use different derived classes for each type of SQL_Statement?
class SQL_Statement {
public:
    SQL_Statement(){};
    statementType type;
    string tableName;

    /*************** Conditions/Values *******************/
    // DROP TABLE and DROP INDEX don't have conditions
    // CREATE TABLE
    vector<Attribute>* attributes;
    // SELECT, ALTER TABLE, DELETE
    vector<Condition> * conditions;
    // ALTER TABLE, INSERTwithAttr
//    vector<KeyValueTuple> * keyValues;
    // CREATE INDEX, DROP INDEX
    string indexName;
    // CREATE INDEX, SELECT
    vector<string> * selectedAttrList;
    // INSERT
    vector<ValueWithType> * values;
};



#endif //MINISQL_SQL_SATATEMENT_H
