//
// Created by Weiye Chen on 2019-05-28.
//

#ifndef MINISQL_ATTRIBUTE_H
#define MINISQL_ATTRIBUTE_H

#include <cstdio>
#include <string>
#include <vector>

//#include "../RecordManager/RecordManager.h"
#include "../minisql.h"

using namespace std;

// Type of table attributes
#define AttributeType KTP
//typedef KTP AttributeType;

class Attribute{
public:
    Attribute() {};
//	Attribute(const Attribute &a) { ; } //;
//    ~Attribute() ;

    AttributeType type;         // Data type of the current table attribute
    int VARCHAR_LEN = 0;        // If the attribute is varchar, this field is applicable.
    string name;                // Name of the attribute
    bool primaryKey = false;    // if the attribute is primary key
    bool unique = false;        // if the attribute is unique
	bool hasindex = false;
	string indexName = "";		// !!! index sometimes should have a name
	int getSize()
    {
	    switch (type)
        {
            case AttributeType::INT:
            {
                return sizeof(int);
            }
            case AttributeType::FLOAT:
            {
                return sizeof(float);
            }
            case AttributeType::STRING:
            {
                return VARCHAR_LEN * sizeof(char);
            }
        }
    }
	friend fstream& operator>> (fstream& fin, Attribute &c) 
	{
		fin >> (int &)c.type >> c.VARCHAR_LEN >> c.name >> (int &)c.primaryKey >> (int &)c.unique >> (int &)c.hasindex;
		return fin;
	}
	friend fstream& operator<< (fstream& fout, Attribute &c) 
	{
		fout << (int &)c.type << " " << c.VARCHAR_LEN
			<< " " << c.name << " " << (int)c.primaryKey << " " << (int)c.unique << " " << (int)c.hasindex << "\n";
		return fout;
	}
};

#endif //MINISQL_ATTRIBUTE_H
