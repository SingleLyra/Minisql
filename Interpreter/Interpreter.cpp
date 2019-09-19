//
// Coded by Weiye Chen
// interpreter.cpp
//

#include "Interpreter.h"


#include <iostream>
#include <cstdio>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>

using namespace std;

/// Interpreter with syntax checking function
SQL_Statement* Interpreter::parseStatement(string SQL)
{
    string::size_type pos = 0;
    string::size_type lastCheckPos = 0;
    string operationStr = getNextSemanticUnit(SQL, pos, lastCheckPos);

    try
    {
        if (caseInsensitiveCompare(operationStr, "CREATE"))
        {

            string tableOrIndexWord = getNextSemanticUnit(SQL, pos, lastCheckPos);
            /// CREATE TABLE
            // Only support CREATE TABLE table_name (create_definitions, ...)
            if (caseInsensitiveCompare(tableOrIndexWord, "TABLE"))
            {
                /// [TABLE_NAME]
                string tableName = getNextSemanticUnit(SQL, pos, lastCheckPos);
                if (tableName == "")
                    throw SyntaxError(SQL, "a table name", pos);

                /// [ATTRIBUTE_LIST]
                if (!CheckLeftParenthesis(SQL, pos))
                    throw SyntaxError(SQL, "\"(\"", pos);
                bool flag = true;
                bool hasPrimaryKeySpecified = false;
                vector<Attribute> * attributes = new vector<Attribute>();
                while (flag) // Each loop represents a handling process of an attribute
                {
                    /// Get attribute name or key type
                    string attrName = getNextSemanticUnit(SQL, pos, lastCheckPos);
                    if (caseInsensitiveCompare(attrName, "PRIMARY"))
                    {
                        string wordOfKey = getNextSemanticUnit(SQL, pos, lastCheckPos);  // "key"
                        if (!caseInsensitiveCompare(wordOfKey, "KEY"))
                        {
                            throw SyntaxError(SQL, "\"KEY\"", lastCheckPos);
                        }
                        /// VARCHAR type requires integral length with enclosing parentheses.
                        if (!CheckLeftParenthesis(SQL, pos))
                            throw SyntaxError(SQL, "\"(\"", pos);
                        string primaryKeyAttr = getNextSemanticUnit(SQL, pos, lastCheckPos);
                        for (auto it = attributes->begin(); it != attributes->end(); it++)
                        {
                            if (it->name == primaryKeyAttr)
                            {
                                it->primaryKey = true;
                                it->hasindex = true;
                            }
                        }
                        if (!CheckRightParenthesis(SQL, pos))
                            throw SyntaxError(SQL, "\")\"", pos);
                        // Primary key specification must be followed by , or )
                        if (!CheckComma(SQL, pos))
                        {
                            if (SQL[pos] != ')')
                            {
                                throw SyntaxError(SQL, "\",\"", pos);
                            }
                            flag = false;
                        }
                        hasPrimaryKeySpecified = true;
                        continue;
                    }
                    Attribute* a = new Attribute();
                    a->name = attrName;
                    // Assume that we don't support out-of-line primary key declaration
                    /// Get attribute type
                    string dataType = getNextSemanticUnit(SQL, pos, lastCheckPos);
                    if (caseInsensitiveCompare(dataType, "INT"))
                        a->type = AttributeType::INT;
                    else if (caseInsensitiveCompare(dataType, "FLOAT"))
                        a->type = AttributeType::FLOAT;
                    else if (caseInsensitiveCompare(dataType, "VARCHAR") || caseInsensitiveCompare(dataType, "CHAR"))
                    {
                        /// VARCHAR type requires integral length with enclosing parentheses.
                        if (!CheckLeftParenthesis(SQL, pos))
                            throw SyntaxError(SQL, "\"(\"", pos);

                        string intLen = getNextSemanticUnit(SQL, pos, lastCheckPos);
                        if (!isInt(intLen))
                            throw SyntaxError(SQL, "an integral number", lastCheckPos);
                        if (!CheckRightParenthesis(SQL, pos))
                            throw SyntaxError(SQL, "\")\"", pos);
                        a->type = AttributeType::STRING;
                        a->VARCHAR_LEN = stoi(intLen);
                    }
                    /// Following end of the attribute type declaration, a key declaration or comma or right parenthesis is expected.
                    if (!CheckComma(SQL, pos)) /// If followed by key declaration or ')'
                    {
                        if (CheckRightParenthesis(SQL, pos))
                        {
                            flag = false;
                        }
                        else
                        {
                            string keyPropertyWord = getNextSemanticUnit(SQL, pos, lastCheckPos); // Primary (key) or Unique
                            if (caseInsensitiveCompare(keyPropertyWord, "PRIMARY"))
                            {

                                string wordOfKey = getNextSemanticUnit(SQL, pos, lastCheckPos);  // "key"
                                if (caseInsensitiveCompare(wordOfKey, "KEY"))
                                {
                                    a->primaryKey = true;
                                    a->hasindex = true;
                                    hasPrimaryKeySpecified = true;
                                }
                                else
                                    throw SyntaxError(SQL, "\"KEY\"", lastCheckPos);
                            }
                            else if (caseInsensitiveCompare(keyPropertyWord, "UNIQUE"))
                            {
                                a->unique = true;
                            }
                            // syntax other than primary key or unique or ')' is not expected, so comma must be added. in this loop, comma is not added.
                            else
                                throw SyntaxError(SQL, "\",\"", lastCheckPos);
                            // end if

                            // primary key or unique syntax must be followed by ',' or ')'
                            if (!CheckComma(SQL, pos))
                            {
                                if (SQL[pos] != ')')
                                {
                                    throw SyntaxError(SQL, "\",\"", pos);
                                }
                                flag = false;
                            }
                        }
                    }
                    attributes->push_back(*a);
                }
                if (!hasPrimaryKeySpecified)
                {
                    if (attributes->size() == 1)
                    {
                        auto it = attributes->begin();
                        it->hasindex = true;
                        it->primaryKey = true;
                    }
                    else
                    {
                        throw SyntaxError(SQL, "primary key specification", pos, SyntaxErrorType::Expecting);
                    }
                }

                SQL_Statement* sqlStatement = new SQL_Statement();
                sqlStatement->tableName = tableName;
                sqlStatement->type = statementType::CREATETABLE;
                sqlStatement->attributes = attributes;
                return sqlStatement;
            }

            /// CREATE INDEX
            // Currenly supporting CREATE INDEX idx_name ON tbl_name(key[, ...])
            else if (caseInsensitiveCompare(tableOrIndexWord, "INDEX"))
            {
                string indexName = getNextSemanticUnit(SQL, pos, lastCheckPos);
                if (indexName == "")
                    throw SyntaxError(SQL, "an index name", lastCheckPos);

                string onWord = getNextSemanticUnit(SQL, pos, lastCheckPos);
                if (!caseInsensitiveCompare(onWord, "ON"))
                {
                    throw SyntaxError(SQL, "\"ON\"", lastCheckPos + 1);
                }
                string tableName = getNextSemanticUnit(SQL, pos, lastCheckPos);
                if (tableName == "")
                    throw SyntaxError(SQL, "a table name", lastCheckPos);
                if (!CheckLeftParenthesis(SQL, pos))
                    throw SyntaxError(SQL, "\"(\"", pos);
                bool flag = true;
                vector<string>* indexingAttributesList = new vector<string>;
                while (flag)
                {
                    string attrName = getNextSemanticUnit(SQL, pos, lastCheckPos);
                    if (tableName == "")
                        throw SyntaxError(SQL, "an attribute name", lastCheckPos);
                    if (!CheckComma(SQL, pos))
                    {
                        if (SQL[pos] != ')')
                        {
                            throw SyntaxError(SQL, "\",\"", pos);
                        }
                        flag = false;
                    }
                    indexingAttributesList->push_back(attrName);
                }

                SQL_Statement* sqlStatement = new SQL_Statement();
                sqlStatement->tableName = tableName;
                sqlStatement->indexName = indexName;
                sqlStatement->selectedAttrList = indexingAttributesList;
                sqlStatement->type = statementType::CREATEINDEX;
                return sqlStatement;

            }
            /// 'create' Not followed with 'table' or 'index'
            else
            {
                throw SyntaxError(SQL, "\"table\" or \"index\"", lastCheckPos);
            }
        }

        else if (caseInsensitiveCompare(operationStr, "INSERT"))
        {
            /// INSERT INTO

            UnexpectingSyntax(SQL, pos);
            string intoWord = getNextSemanticUnit(SQL, pos, lastCheckPos);
            if (!caseInsensitiveCompare(intoWord, "INTO"))
            {
                throw SyntaxError(SQL, "\"INTO\"", lastCheckPos);
            }
            UnexpectingSyntax(SQL, pos);
            string tableName = getNextSemanticUnit(SQL, pos, lastCheckPos);

            /*
            /// INSERTwithAttr
            if (CheckPunctuation(SQL, pos, '('))
            {
                vector<string> attributeList = vector<string>();
                bool flag = true;
                while (!CheckPunctuation(SQL, pos, ')') && SQL[pos] != '\0' && flag)
                {
                    UnexpectingSyntax(SQL, pos);
                    attributeList.push_back(getNextSemanticUnit(SQL, pos, lastCheckPos));
                    if (!CheckComma(SQL, pos))
                    {
                        if (!CheckPunctuation(SQL, pos, ')'))
                            throw SyntaxError(SQL, ")", pos);
                        flag = false;
                    }
                }

                string valuesWord = getNextSemanticUnit(SQL, pos, lastCheckPos);
                if (!caseInsensitiveCompare(valuesWord, "VALUES"))
                {
                    throw SyntaxError(SQL, "\"VALUES\"", lastCheckPos);
                }

                if (!CheckPunctuation(SQL, pos, '('))
                {
                    throw SyntaxError(SQL, "(", pos);
                }

                bool flag1 = true;
                vector<KeyValueTuple> * keyValues = new vector<KeyValueTuple>();
                for (auto specifiedAttrListIt = attributeList.begin(); specifiedAttrListIt != attributeList.end(); specifiedAttrListIt++)
                {
                    if (flag1 == false)
                        throw SyntaxError(SQL, "\",\"", pos);
                    ValueWithType v = getValue(SQL, pos);
                    KeyValueTuple kvt = KeyValueTuple(*specifiedAttrListIt);
                    kvt.value = v.value;
                    kvt.type = v.type;
                    keyValues->push_back(kvt);
                    if (!CheckComma(SQL, pos))
                    {
                        flag1 = false;
                    }
                }
                if (!CheckPunctuation(SQL, pos, ')'))
                    throw SyntaxError(SQL, ")", pos);
                SQL_Statement * sqlStatement = new SQL_Statement();
                sqlStatement->type = statementType::INSERTwithAttr;
                sqlStatement->keyValues = keyValues;
                sqlStatement->tableName = tableName;

                return sqlStatement;
            }*/
            /// INSERT
//            else
            {
                UnexpectingSyntax(SQL, pos);
                string valuesWord = getNextSemanticUnit(SQL, pos, lastCheckPos);
                if (!caseInsensitiveCompare(valuesWord, "VALUES"))
                {
                    throw SyntaxError(SQL, "\"VALUES\"", lastCheckPos);
                }
                if (!CheckPunctuation(SQL, pos, '('))
                {
                    throw SyntaxError(SQL, "(", pos);
                }

                vector<ValueWithType> * values = new vector<ValueWithType>();
                while (!CheckPunctuation(SQL, pos, ')') && SQL[pos] != '\0')
                {
                    ValueWithType v = getValue(SQL, pos);
                    values->push_back(v);
                    if (!CheckComma(SQL, pos))
                    {
                        if (!CheckPunctuation(SQL, pos, ')'))
                            throw SyntaxError(SQL, ")", pos);
                    }
                }
                SQL_Statement * sqlStatement = new SQL_Statement();
                sqlStatement->type = statementType::INSERT;
                sqlStatement->values = values;
                sqlStatement->tableName = tableName;

                return sqlStatement;
            }

        }

        else if (caseInsensitiveCompare(operationStr, "SELECT"))
        {
            vector<string> * selectedAttrList = new vector<string>();
            do
            {
                UnexpectingSyntax(SQL, pos);
                string attr = getNextSemanticUnit(SQL, pos, lastCheckPos);
                selectedAttrList->push_back(attr);
            }
            while (CheckComma(SQL, pos));

            UnexpectingSyntax(SQL, pos);
            string fromWord = getNextSemanticUnit(SQL, pos, lastCheckPos);
            if (!caseInsensitiveCompare(fromWord, "FROM"))
                throw SyntaxError(SQL, "\"FROM\"", pos);

            UnexpectingSyntax(SQL, pos);
            string tableName = getNextSemanticUnit(SQL, pos, lastCheckPos);

            UnexpectingSyntax(SQL, pos);
            string whereWord = getNextSemanticUnit(SQL, pos, lastCheckPos);
            if (!caseInsensitiveCompare(whereWord, "WHERE"))
            {
                if (whereWord == "" || pos == SQL.length() - 1)
                {
                    SQL_Statement * sqlStatement = new SQL_Statement();
                    sqlStatement->tableName = tableName;
                    sqlStatement->type = statementType::SELECT;
                    sqlStatement->selectedAttrList = selectedAttrList;
                    sqlStatement->conditions = NULL;

                    return sqlStatement;
                }
                throw SyntaxError(SQL, "\"WHERE\"", lastCheckPos);
            }

            vector<Condition> * conditions = getConditions(SQL, pos);

            UnexpectingSyntax(SQL, pos);

            SQL_Statement * sqlStatement = new SQL_Statement();
            sqlStatement->tableName = tableName;
            sqlStatement->type = statementType::SELECT;
            sqlStatement->selectedAttrList = selectedAttrList;
            sqlStatement->conditions = conditions;

            return sqlStatement;
        }

        else if (caseInsensitiveCompare(operationStr, "DROP"))
        {
            UnexpectingSyntax(SQL, pos);
            string tableWord = getNextSemanticUnit(SQL, pos, lastCheckPos);
            if (caseInsensitiveCompare(tableWord, "TABLE"))
            {
                UnexpectingSyntax(SQL, pos);
                string tableName = getNextSemanticUnit(SQL, pos, lastCheckPos);

                UnexpectingSyntax(SQL, pos);
                if (SQL[pos] != '\0')
                {
                    string unexpected = getNextSemanticUnit(SQL, pos, lastCheckPos);
                    throw SyntaxError(SQL, unexpected, pos, SyntaxErrorType::Unexpected);
                }

                SQL_Statement* sqlStatement = new SQL_Statement();
                sqlStatement->tableName = tableName;
                sqlStatement->type = statementType::DROPTABLE;

                return sqlStatement;
            }
            else if (caseInsensitiveCompare(tableWord, "INDEX"))
            {
                UnexpectingSyntax(SQL, pos);
                string idxName = getNextSemanticUnit(SQL, pos, lastCheckPos);

                UnexpectingSyntax(SQL, pos);
                string onWord = getNextSemanticUnit(SQL, pos, lastCheckPos);
                if (!caseInsensitiveCompare(onWord, "ON"))
                    throw SyntaxError(SQL, "\"ON\"", lastCheckPos);

                UnexpectingSyntax(SQL, pos);
                string tableName = getNextSemanticUnit(SQL, pos, lastCheckPos);

                UnexpectingSyntax(SQL, pos);
                if (SQL[pos] != '\0')
                {
                    string unexpected = getNextSemanticUnit(SQL, pos, lastCheckPos);
                    throw SyntaxError(SQL, unexpected, pos, SyntaxErrorType::Unexpected);
                }

                SQL_Statement* sqlStatement = new SQL_Statement();
                sqlStatement->tableName = tableName;
                sqlStatement->indexName = idxName;
                sqlStatement->type = statementType::DROPINDEX;

                return sqlStatement;

            }
            else
                throw SyntaxError(SQL, "\"TABLE\" or \"INDEX\"", lastCheckPos);

        }

        else if (caseInsensitiveCompare(operationStr, "DELETE"))
        {
            UnexpectingSyntax(SQL, pos);
            string fromWord = getNextSemanticUnit(SQL, pos, lastCheckPos);
            if (!caseInsensitiveCompare(fromWord, "FROM"))
                throw SyntaxError(SQL, "\"FROM\"", pos);

            UnexpectingSyntax(SQL, pos);
            string tableName = getNextSemanticUnit(SQL, pos, lastCheckPos);

            UnexpectingSyntax(SQL, pos);
            string whereWord = getNextSemanticUnit(SQL, pos, lastCheckPos);
            if (!caseInsensitiveCompare(whereWord, "WHERE"))
            {
                if (whereWord == "" || pos == SQL.length() - 1)
                {
                    SQL_Statement * sqlStatement = new SQL_Statement();
                    sqlStatement->tableName = tableName;
                    sqlStatement->type = statementType::DELETE;
                    sqlStatement->conditions = NULL;

                    return sqlStatement;
                }
                throw SyntaxError(SQL, "\"WHERE\"", lastCheckPos);
            }

            vector<Condition> * conditions = getConditions(SQL, pos);

            UnexpectingSyntax(SQL, pos);

            SQL_Statement * sqlStatement = new SQL_Statement();
            sqlStatement->tableName = tableName;
            sqlStatement->type = statementType::DELETE;
            sqlStatement->conditions = conditions;

            return sqlStatement;
        }

        else if (caseInsensitiveCompare(operationStr, "EXECFILE"))
        {
            string fileName = getNextSemanticUnit(SQL, pos, lastCheckPos);
            if (CheckPunctuation(SQL, pos, '.'))
            {
                fileName += ".";
                string extension = getNextSemanticUnit(SQL, pos, lastCheckPos);
                fileName += extension;
            }
            UnexpectingSyntax(SQL, pos);
            if(!File_Exist(fileName))
            {
                throw SyntaxError(SQL, "File doesn't exist", pos, SyntaxErrorType::Fatal);
            }
            fstream fin;
            fin.open(fileName);

            while(!fin.eof())
            {
                string SQL1;
                while (SQL1.find(';') == string::npos && !fin.eof())
                {
                    string a;
                    getline(fin, a);
                    size_t slashRpos = a.find('\r');
                    size_t slashTpos = a.find('\t');
                    while (slashRpos != string::npos)
                    {
                        a = a.erase(slashRpos, sizeof('\r'));
                        slashRpos = a.find('\r');
                    }
                    while (slashTpos != string::npos)
                    {
                        a = a.erase(slashTpos, sizeof('\t'));
                        slashTpos = a.find('\t');
                    }
//                    a = a.substr(0, a.find("\r"));
                    SQL1.append(a + ' ');
                }
                cout << "Read SQL from file: " + SQL1 << endl;
                if (SQL1.find(';') == string::npos)
                {
                    continue;
                }
                SQL1 = SQL1.substr(0, SQL1.find(';'));
                SQL_Statement* sql1 = parseStatement(SQL1);
                time_t t = executeSQL(sql1);
                cout << "Statement has been executed in " + to_string(t) + " milliseconds" << endl;
            }
            fin.close();
            return NULL;
        }
        else if (caseInsensitiveCompare(operationStr, "EXIT"))
        {
            bm.FlushAllBlocks();
            exit(0);
            return NULL;
        }

        else
            throw SyntaxError(SQL, "a valid operation type, like \"SELECT\"", lastCheckPos);
    }
    catch (SyntaxError se)
    {
        cerr << se.notice << endl;
    }


}

time_t Interpreter::executeSQL(SQL_Statement* sqlStatement)
{
    if (sqlStatement == NULL)
    {
        return 0;
    }
    statementType type = sqlStatement->type;
    clock_t t;
    try
    {
        switch (type)
        {
            case statementType::CREATETABLE :
            {
                t = clock();
                bool success = api.CreateTable(sqlStatement->tableName, sqlStatement->attributes);
                t = clock() - t;
                break;
            }
            case statementType::CREATEINDEX :
            {
                t = clock();
                bool success = api.CreateIndex(sqlStatement->tableName, sqlStatement->indexName, sqlStatement->selectedAttrList);
                t = clock() - t;
                break;
            }
            case statementType::SELECT:
            {
                t = clock();
                bool success = api.Select(sqlStatement->tableName, sqlStatement->conditions, sqlStatement->selectedAttrList);
                t = clock() - t;
                break;
            }
            case statementType::INSERT:
            {
                t = clock();
                bool success = api.InsertAll(sqlStatement->tableName, sqlStatement->values);
                t = clock() - t;
                break;
            }
//            case statementType::INSERTwithAttr:
//            {
//                t = clock();
//                bool success = api.InsertWithAttr(sqlStatement->tableName, sqlStatement->keyValues);
//                t = clock() - t;
//                break;
//            }
            case statementType::DROPTABLE:
            {
                t = clock();
                bool success = api.DropTable(sqlStatement->tableName);
                t = clock() - t;
                break;
            }
            case statementType::DROPINDEX:
            {
                t = clock();
                bool success = api.DropIndex(sqlStatement->tableName, sqlStatement->indexName);
                t = clock() - t;
                break;
            }
            case statementType::DELETE:
            {
                t = clock();
                bool success = api.Delete(sqlStatement->tableName, sqlStatement->conditions);
                t = clock() - t;
                break;
            }
            case statementType::QUIT:
            {
                t = clock();
                t = clock() - t;
                break;
            }
            case statementType::EXECFILE:
            {
                t = clock();
                t = clock() - t;
                break;
            }
            default:
            {
                throw API_Error(API_Error_Type::INTERPRETATION_ERROR, "An internal error occurred during interpreter parsing. It's probably related to statement type you assigned.");
            }
        }
        cm.FlushAllCatalogs();
        // bm.FlushAllBlocks();
        return t;
    }
    catch (API_Error e)
    {
        cerr << e.message << endl;
    }
}

/// This function is designed for escaping those punctuation for simplifying our denotations of the SQL statements
string Interpreter::getNextSemanticUnit(string& SQL, string::size_type& pos)
{
    if (pos >= SQL.length() - 1)
    {
        return "";
    }

    string SemanticUnit = "";
    string::size_type pos1, pos2;
    pos1 = pos;
    while ((SQL[pos1] == ' ' || SQL[pos1] == ',' || SQL[pos1] == '\t')  && SQL[pos1] != 0 && SQL[pos1] != '(' && SQL[pos1] != ')' && SQL[pos1] != '\'' && SQL[pos1] != '<' && SQL[pos1] != '>' && SQL[pos1] != '=' && SQL[pos1] != '.')
    {
        pos1++;
    }
    pos2 = pos1 + 1;
    while ( !(SQL[pos2] == ' ' || SQL[pos2] == ',' || SQL[pos2] == '\t')  && SQL[pos2] != 0  && SQL[pos2] != '(' && SQL[pos2] != ')' && SQL[pos1] != '\'' && SQL[pos1] != '<' && SQL[pos1] != '>' && SQL[pos1] != '=' && SQL[pos2] != '.')
    {
        pos2++;
    }
    pos = pos2;

    SemanticUnit += SQL.substr(pos1, pos2 - pos1);
    return SemanticUnit;
}

/// This function is designed for escaping those punctuation for simplifying our denotations of the SQL statements
string Interpreter::getNextSemanticUnit(string& SQL, string::size_type& pos, string::size_type& lastCheckPos)
{
//    if (pos >= SQL.length() - 1)
//    {
//        return "";
//    }

    string SemanticUnit = "";
    string::size_type pos1, pos2;
    pos1 = pos;
    while ((SQL[pos1] == ' ' || SQL[pos1] == ',' || SQL[pos1] == '\t')  && SQL[pos1] != 0 && SQL[pos1] != '(' && SQL[pos1] != ')' && SQL[pos1] != '\'' && SQL[pos1] != '<' && SQL[pos1] != '>' && SQL[pos1] != '=' && SQL[pos1] != '.')
    {
        pos1++;
    }
    lastCheckPos = pos1;
//    if (SQL[pos1] == '(' && SQL[pos1] == '\'' && SQL[pos1] == ')')
//    {
//        return to_string((SQL[pos1]));
//    }
    pos2 = pos1 + 1;
    while ( !(SQL[pos2] == ' ' || SQL[pos2] == ',' || SQL[pos2] == '\t')  && SQL[pos2] != 0  && SQL[pos2] != '(' && SQL[pos2] != ')' && SQL[pos1] != '\'' && SQL[pos1] != '<' && SQL[pos1] != '>' && SQL[pos1] != '=' && SQL[pos2] != '.')
    {
        pos2++;
    }
    pos = pos2;

    SemanticUnit += SQL.substr(pos1, pos2 - pos1);
    return SemanticUnit;
}

/// A function for making comparison between 2 strings in case-insensitive mode
bool Interpreter::caseInsensitiveCompare(string str1, string str2)
{
    transform(str1.begin(), str1.end(), str1.begin(), ::tolower);
    transform(str2.begin(), str2.end(), str2.begin(), ::tolower);
    return !str1.compare(str2); // compare returns 0 if identical
}

bool Interpreter::CheckLeftParenthesis(string SQL, string::size_type& pos)
{
    while ((SQL[pos] == ' ' || SQL[pos] == '\t')  && SQL[pos] != 0)
    {
        pos++;
    }
    if (SQL[pos] != '(')
        return false;
    (pos)++;
    return true;
}

bool Interpreter::CheckRightParenthesis(string SQL, string::size_type& pos)
{
    while ((SQL[pos] == ' ' || SQL[pos] == '\t')  && SQL[pos] != 0)
    {
        pos++;
    }
    if (SQL[pos] != ')')
        return false;
    (pos)++;
    return true;
}

bool Interpreter::CheckComma(string SQL, string::size_type& pos)
{
    while ((SQL[pos] == ' ' || SQL[pos] == '\t')  && SQL[pos] != 0)
    {
        pos++;
    }
    if (SQL[pos] != ',')
        return false;
    (pos)++;
    return true;
}

bool Interpreter::CheckPunctuation(string SQL, string::size_type& pos)
{
    while ((SQL[pos] == ' ' || SQL[pos] == '\t')  && SQL[pos] != 0)
    {
        pos++;
    }
    if (SQL[pos] != ',' || SQL[pos] != '.' || SQL[pos] != '(' || SQL[pos] != ')' || SQL[pos] != '<' || SQL[pos] != '>' || SQL[pos] != '=' || SQL[pos] != '\'' || SQL[pos] != '.')
        return false;
    (pos)++;
    return true;
}

bool Interpreter::CheckPunctuation(string SQL, string::size_type& pos, const char& punc)
{
    while ((SQL[pos] == ' ' || SQL[pos] == '\t')  && SQL[pos] != 0)
    {
        pos++;
    }
    if (SQL[pos] != punc)
        return false;
    (pos)++;
    return true;
}

bool Interpreter::UnexpectingSyntax(string SQL, string::size_type& pos, const char& punc)
{
    if (CheckPunctuation(SQL, pos, punc))
    {
        throw SyntaxError(SQL, "\"" + to_string(punc) + "\"", pos, SyntaxErrorType::Expecting);
        return false;
    }
    else
    {
        return true;
    }
}

bool Interpreter::UnexpectingSyntax(string SQL, string::size_type& pos)
{
    if (CheckPunctuation(SQL, pos))
    {
        throw SyntaxError(SQL, "\"" + to_string(SQL[pos]) + "\"", pos, SyntaxErrorType::Expecting);
        return false;
    }
    else
    {
        return true;
    }
}


bool Interpreter::isInt(string intStr)
{
    try
    {
        stoi(intStr);
    }
    catch (exception e)
    {
        return false;
    }
    return true;
}

// Any value not in string / int / float would trigger SyntaxError
ValueWithType& Interpreter::getValue(string SQL, string::size_type& pos)
{
    ValueWithType res = ValueWithType();
    // String type
    if (CheckPunctuation(SQL, pos, '\''))
    {
        string s = "";
//        string::size_type lastPos = pos;
        while (!CheckPunctuation(SQL, pos, '\''))
        {
            s += SQL[pos];
            pos++;
        }
//        s = SQL.substr(lastPos, pos-lastPos);
//        res.value.charValue = (char *)malloc(sizeof(char) * 256);
        strcpy(res.value.charValue, s.c_str());
        res.type = AttributeType::STRING;
    }
    else
    {
        string::size_type lastCheckPos = pos;
        string numS1 = getNextSemanticUnit(SQL, pos, lastCheckPos);
        if (!isInt(numS1))
            throw SyntaxError(SQL, "a number", lastCheckPos+1);
        if (CheckPunctuation(SQL, pos, '.'))
        {
            string numS2 = getNextSemanticUnit(SQL, pos, lastCheckPos);
            if (!isInt(numS2))
                throw SyntaxError(SQL, "a number", lastCheckPos+1);
            float f;
            try
            {
                f = stof((numS1 + "." + numS2).c_str());
            }
            catch (exception e)
            {
                SyntaxError(SQL, "float conversion error", lastCheckPos, SyntaxErrorType::Unexpected);
            }
            res.value.floatValue = f;
            res.type = AttributeType::FLOAT;
        }
        else
        {
            res.value.intValue = atoi(numS1.c_str());
            res.type = AttributeType::INT;
        }
    }
    return res;
}

operatorType Interpreter::getOperator(string SQL, string::size_type& pos)
{
    // TODO: getting operator type.
    /// EQUAL
    if (CheckPunctuation(SQL, pos, '='))
    {
        if (CheckPunctuation(SQL, pos))
        {
            // \' is the only punctuation allowed after =
            if (SQL[pos-1] != '\'')
                throw SyntaxError(SQL, to_string(SQL[pos-1]), pos-1, SyntaxErrorType::Unexpected);
            pos--;
        }
        return operatorType::EQ;
    }
    /// LESS THAN or LESS THAN OR EQUAL TO or NOT EQUAL TO
    else if (CheckPunctuation(SQL, pos, '<'))
    {
        // = or \' or > could follow < without spaces.
        /// LTE <=
        if (SQL[pos] == '=')
        {
            pos++;
            if (CheckPunctuation(SQL, pos))
            {
                // \' is the only punctuation allowed after <=
                if (SQL[pos-1] != '\'')
                    throw SyntaxError(SQL, to_string(SQL[pos-1]), pos-1, SyntaxErrorType::Unexpected);
                pos--;
            }
            return operatorType::LTE;
        }
        /// NEQ <>
        else if (SQL[pos] == '>')
        {
            pos++;
            if (CheckPunctuation(SQL, pos))
            {
                // \' is the only punctuation allowed after <>
                if (SQL[pos-1] != '\'')
                    throw SyntaxError(SQL, to_string(SQL[pos-1]), pos-1, SyntaxErrorType::Unexpected);
                pos--;
            }
            return operatorType::NEQ;
        }
        /// LT < (followed by string with no space)
        else if (SQL[pos] == '\'')
        {
            pos--;
            return operatorType::LT;
        }
        // < followed by illegal punctuation or other expressions
        else
        {
            // illegal punctuation
            if (CheckPunctuation(SQL, ++pos))
            {
                throw SyntaxError(SQL, to_string(SQL[pos-1]), pos-1, SyntaxErrorType::Unexpected);
            }
        }
        return operatorType::LT;
    }
    /// GREATER THAN or GREATER THAN OR EQUAL TO
    else if (CheckPunctuation(SQL, pos, '>'))
    {
        // = or \' could follow > without spaces.
        /// GTE >=
        if (SQL[pos] == '=')
        {
            pos++;
            if (CheckPunctuation(SQL, pos))
            {
                // \' is the only punctuation allowed after <=
                if (SQL[pos-1] != '\'')
                    throw SyntaxError(SQL, to_string(SQL[pos-1]), pos-1, SyntaxErrorType::Unexpected);
                pos--;
            }
            return operatorType::GTE;
        }
        /// GT > (followed by string with no space)
        else if (SQL[pos] == '\'')
        {
            pos--;
            return operatorType::GT;
        }
        // > followed by illegal punctuation or other expressions
        else
        {
            // illegal punctuation
            if (CheckPunctuation(SQL, ++pos))
            {
                throw SyntaxError(SQL, to_string(SQL[pos-1]), pos-1, SyntaxErrorType::Unexpected);
            }
        }
        return operatorType::GT;
    }
    /// NOT EQUAL TO (!=)
    else if (CheckPunctuation(SQL, pos, '!'))
    {
        if (SQL[pos] == '=')
        {
            pos++;
            if (CheckPunctuation(SQL, pos))
            {
                // \' is the only punctuation allowed after !=
                if (SQL[pos-1] != '\'')
                    throw SyntaxError(SQL, to_string(SQL[pos-1]), pos-1, SyntaxErrorType::Unexpected);
                pos--;
            }
            return operatorType::NEQ;
        }
        else
            throw SyntaxError(SQL, "!", pos-1, SyntaxErrorType::Unexpected);
    }
    /// Not a valid operator
    else
        return operatorType::UNDEFINED;
}

vector<Condition> * Interpreter::getConditions(string SQL, string::size_type& pos)
{
    string andWord;
    vector<Condition> * conditions = new vector<Condition>();
    do
    {
        UnexpectingSyntax(SQL, pos);
        string attrName = getNextSemanticUnit(SQL, pos);
        operatorType opType = getOperator(SQL, pos);
        ValueWithType v = getValue(SQL, pos);
        Condition condition = Condition(attrName, opType, v);
        conditions->push_back(condition);
        andWord = getNextSemanticUnit(SQL, pos);
    }
    while (caseInsensitiveCompare(andWord, "AND"));
    return conditions;
}

void Interpreter::execInterpreter()
{
    string SQL;
    cout << "SQL>>";
    while (SQL.find(';') == string::npos)
    {
        string a;
        getline(cin, a);
        SQL.append(a + ' ');
    }
    SQL = SQL.substr(0, SQL.find(';'));
    SQL_Statement* sql = parseStatement(SQL);
    time_t time = executeSQL(sql);
    cout << "Statement has been executed in " + to_string(time) + " milliseconds" << endl;
}