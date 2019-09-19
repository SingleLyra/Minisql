#include <iostream>
#include <string>
#include "minisql.h"
#include "Interpreter/Interpreter.h"
#include "BufferManager/BufferManager.h"

extern BufferManager bm;
using namespace std;
//void IndexTest();
int main() 
{
    cout << "--------- MiniSQL Implemented by Xinning Zhang and Weiye Chen ------------" << endl;
    cout << "********* Instructions ******* " << endl << "Input SQL statement followed by semiconlon and then press enter or return;" << endl;
    cout << "Characters in the same line after semicolon would be ignored." << endl << "*************" << endl;

	
	
    while(1)
    {
        Interpreter i = Interpreter();
        i.execInterpreter();
        // bm.FlushAllBlocks();
        // fstream fin;
        // fin.open("a.record", ios::in | ios::binary);
//        int a = -1;
//        string t = "a.record";
//        BLOCKS &x = bm.ReadBlocks(t,0);
//        //fin >> a;
//        x.Read_Int(a, sizeof(int), 1);
//        cout << "In a: " << a << endl;
    }
    return 0;
}


/*
BufferManager bm;
	string SQL;

	cout << "--------- MiniSQL Implemented by Xinning Zhang and Weiye Chen ------------" << endl;
	cout << "********* Instructions ******* " << endl << "Input SQL statement followed by semiconlon and then press enter or return;" << endl;
	cout << "Characters in the same line after semicolon would be ignored." << endl << "*************" << endl;
	cout << "SQL>>";
//	cout << (SQL.find(';') == string::npos);
	while (SQL.find(';') == string::npos)
	{
		string a;
		getline(cin, a);
		SQL.append(a + ' ');
	}
	SQL = SQL.substr(0, SQL.find(';'));
//	cout << SQL << endl;
	SQL_Statement* sql = Interpreter::parseStatement(SQL);
*/