//
// Coded by Weiye Chen
// interpreter.h includes classes designed
// for interpreting and decomposing SQL statement
//


#ifndef MINISQL_INTERPRETER_H
#define MINISQL_INTERPRETER_H

#include "../CatalogManager/Attribute.h"
#include "../API/API.h"
#include "SyntaxError.h"
#include "SQL_Satatement.h"
#include <string>
#include <vector>
#include <exception>

using namespace std;

class Interpreter {
public:
    SQL_Statement* parseStatement(string SQL);
    time_t executeSQL(SQL_Statement* sqlStatement);
    void execInterpreter();

private:
    API api = API();
    // Getting next Semantic Unit. Punctuation are spared. On success, the pos increments.
    string getNextSemanticUnit(string& SQL, string::size_type& pos);
    // Getting next Semantic Unit. Punctuation are spared. On success, the pos increments.
    string getNextSemanticUnit(string& SQL, string::size_type& pos, string::size_type& lastCheckPos);
    // Comparing two strings in case insensitive mode: return true if identical.
    bool caseInsensitiveCompare(string str1, string str2);
    // Checking if there's a left parenthesis '(' locating at or following the position specified. On true, the pos increments.
    bool CheckLeftParenthesis(string SQL, string::size_type& pos);
    // Checking if there's a right parenthesis ')' locating at or following the position specified. On true, the pos increments.
    bool CheckRightParenthesis(string SQL, string::size_type& pos);
    // Checking if there's a comma ',' locating at or following the position specified. On true, the pos increments.
    bool CheckComma(string SQL, string::size_type& pos);
    // Checking if there's any punctuation locating at or following the position specified. On true, the pos increments.
    bool CheckPunctuation(string SQL, string::size_type& pos);
    // Checking if there's a matching punctuation locating at or following the position specified. On true, the pos increments.
    bool CheckPunctuation(string SQL, string::size_type& pos, const char & punc);
    // Throwing a SyntaxError on unexpected punctuation.
    bool UnexpectingSyntax(string SQL, string::size_type& pos);
    // Throwing a SyntaxError on specified unexpected punctuation.
    bool UnexpectingSyntax(string SQL, string::size_type& pos, const char& punc);
    // Checking if a string can be converted to a integer number.
    bool isInt(string intStr);
    // Getting the next value starting from the position specified.
    ValueWithType& getValue(string SQL, string::size_type& pos);
    // Getting the next comparing operator starting from the position specified.
    operatorType getOperator(string SQL, string::size_type& pos);
    // Getting conditions from the SQL starting from the position specified.
    vector<Condition> * getConditions(string SQL, string::size_type& pos);
};



#endif //MINISQL_INTERPRETER_H
