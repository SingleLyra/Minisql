//
// Created by Robert Chen on 2019-06-02.
//

#ifndef MINISQL_SYNTAXERROR_H
#define MINISQL_SYNTAXERROR_H

#include <string>
#include <exception>

using namespace std;

enum class SyntaxErrorType
{
    Expecting,
    Unexpected,
    Fatal,
    Undefined
};

class SyntaxError : exception {
public:
    // Syntax Error (Default: Expecting syntax)
    SyntaxError(string SQL, string info, string::size_type pos)
    {
        this->type = SyntaxErrorType::Expecting;
        this->SQL = SQL;
        this->information = info;
        this->pos = pos;
        this->notice = "Syntax Error:\nIn \"" + SQL + "\"\n";
        for (int i = 0; i < pos + 4; i++)
        {
            this->notice += " ";
        }
        this->notice = this->notice + "^~~~~\n" "Position " + to_string(pos)
                + "\nExpecting " + info + ".";
    }

    // SyntaxError (with type specification)
    SyntaxError(string SQL, string info, string::size_type pos, SyntaxErrorType type)
    {
        this->type = type;
        this->SQL = SQL;
        this->information = info;
        this->pos = pos;
        this->notice = "SyntaxError:\nIn \"" + SQL + "\"\n";
        for (int i = 0; i < pos + 4; i++)
        {
            this->notice += " ";
        }
        switch (type)
        {
            case SyntaxErrorType::Expecting:
            {
                this->notice = this->notice + "^~~~~\n" "Position " + to_string(pos)
                               + "\nExpecting " + info + ".";
                break;
            }
            case SyntaxErrorType::Unexpected:
            {
                this->notice = this->notice + "^~~~~\n" "Position " + to_string(pos);
                this->notice = this->notice + "\nUnexpected syntax " + info + " found.";
            }
            case SyntaxErrorType::Fatal:
            {
                this->notice = this->notice + "^~~~~\n" "Position " + to_string(pos);
                this->notice = this->notice + "\nFatal Error: " + info + ".";
            }
        }
    }

    SyntaxErrorType type;
    string SQL;
    string information;
    string::size_type pos;
    string notice;
};

#endif //MINISQL_SYNTAXERROR_H
