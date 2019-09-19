//
// Created by Robert Chen on 2019-04-30.
//

#include "RecordManager.h"

using namespace std;

RecordManager rm = RecordManager();

bool RecordManager::createRecords(string tableName)
{
    Records rec = Records(tableName);
    rec.createRecordFile();
//    recordsMap.emplace(tableName, rec);
    recordsMap[tableName] = rec;

    return true;
}

bool RecordManager::loadRecords(string tableName)
{
    Records rec = Records(tableName);
    recordsMap[tableName] = rec;

    return true;
}

bool RecordManager::deleteRecords(string tableName)
{
    Records * pRec;
    if (getRecordsObject(tableName, pRec))
    {
        pRec->deleteAllRecords();
        remove((tableName + ".record").c_str());
        return true;
    }
    else
    {
        return false;
    }
}

bool RecordManager::CheckRecordInMap(string tableName)
{
    auto it = recordsMap.find(tableName);
    if (it != recordsMap.end())
    {
        return true;
    }
    return false;
}

bool RecordManager::getRecordsObject(string tableName, Records*& pRec)
{
    auto it = recordsMap.find(tableName);
    if (it != recordsMap.end())
    {
        pRec = &it->second;
        return true;
    }
    else
    {
        if (loadRecords(tableName))
        {
            auto it = recordsMap.find(tableName);
            if (it != recordsMap.end())
            {
                pRec = &it->second;
                return true;
            }
        }
    }
    return false;
}