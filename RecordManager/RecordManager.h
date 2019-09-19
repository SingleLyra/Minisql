//
// Created by Weiye Chen on 2019-04-30.
//

#ifndef MINISQL_RECORDMANAGER_H
#define MINISQL_RECORDMANAGER_H

#include <string>
#include <map>
#include "../RecordManager/Records.h"

using namespace std;

class RecordManager {
public:
    RecordManager()
    {
        recordsMap = map<string, Records>();
    }
    map<string, Records> recordsMap;

    bool createRecords(string tableName);
    bool loadRecords(string tableName);
    bool deleteRecords(string tableName);
    bool CheckRecordInMap(string tableName);
    bool getRecordsObject(string tableName, Records*& pRec);
};

#endif //MINISQL_RECORDMANAGER_H
