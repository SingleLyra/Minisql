//
// Coded by Weiye Chen
//

#include "API.h"

extern map<string, Index*> Index_Map;

bool API::CreateTable (string tblName, vector<Attribute>* attributes)
{
    try
    {
        if (cm.ifTableExists(tblName))
        {
            throw API_Error(API_Error_Type::PARAMETER_ERROR, "Table \"" + tblName + "\" already exists.");
        }
        if (cm.addCatalog(cm.createCatalog(tblName, attributes)))
        {
            rm.createRecords(tblName);
            Index* index;
            bool existsPrimaryKey = false;
            for (auto it = attributes->begin(); it != attributes->end(); it++)
            {
                if (it->primaryKey == true)
                {
                    index = Create_Index(tblName, it->name, "PRIMARY@" + it->name, it->type, it->getSize());
                    Index_Map["PRIMARY@" + it->name] = index;
                    existsPrimaryKey = true;
                }
                else if (it->unique == true)
                {
                    index = Create_Index(tblName, it->name, "UNIQUE@" + it->name, it->type, it->getSize());
                    Index_Map["UNIQUE@" + it->name] = index;
                }
            }
            if (!existsPrimaryKey)
            {
                throw API_Error(API_Error_Type::PARAMETER_ERROR, "No primary key specified in the table.");
            }
            return true;
        }
        else
        {
            throw API_Error(API_Error_Type::INTERPRETATION_ERROR, "Catalog was not created.");
        }
    }
    catch (API_Error e)
    {
        cerr << e.message << endl;
        return false;
    }
}

bool API::CreateIndex (string tblName, string idxName, vector<string>* attrList)
{
    try
    {
        string attrName;
        for (auto it = attrList->begin(); it != attrList->end(); it++)
        {
            attrName = *it;
        }
        if (!cm.ifTableExists(tblName))
        {
            throw API_Error(API_Error_Type::PARAMETER_ERROR, "Table \"" + tblName + "\" doesn't exist.");
        }
        if (!cm.ifAttrExists(tblName, attrName))
        {
            throw API_Error(API_Error_Type::PARAMETER_ERROR, "The attribute specified doesn't exists");
        }
        Catalog& cat = cm.getCatalogByName(tblName);
        Attribute* pAttr = cm.getAttributeByName(cat, attrName);
        if (cm.ifIndexExistsOnAttr(pAttr))
        {
            throw API_Error(API_Error_Type::PARAMETER_ERROR, "The attribute already has an index");
        }
        if (cm.ifIndexExistsByName(cat, idxName))
        {
            throw API_Error(API_Error_Type::PARAMETER_ERROR, "The table " + tblName + " already has an index named " + idxName + ".");
        }
        Index* index = Create_Index(tblName, attrName, idxName + "@" + tblName, pAttr->type, pAttr->getSize());
        InsertAllRecordToIndex(index, tblName, attrName);
        pAttr->indexName = idxName;
        pAttr->hasindex = true;
        cat.Write();
        return true;
    }
    catch (API_Error e)
    {
        cerr << e.message << endl;
        return false;
    }
}

bool API::Select (string tblName, vector<Condition>* conditions, vector<string>* selectedAttrList)
{
    try
    {
        if (!cm.ifTableExists(tblName))
		{
			throw API_Error(API_Error_Type::PARAMETER_ERROR, "Table \"" + tblName + "\" doesn't exist.");
		}
        bool allFlag = false;
        if (selectedAttrList->size() == 0)
        {
            throw API_Error(API_Error_Type::INTERPRETATION_ERROR, "[Select Error] Failed to load the attributes list ");
        }

        for (auto it = selectedAttrList->begin(); !allFlag && it != selectedAttrList->end(); it++)
        {
            if (*it == "*")
            {
                if (selectedAttrList->size() != 1)
                {
                    throw API_Error(API_Error_Type::PARAMETER_ERROR, "[Select Error] No other assigned attributes are expected with \"*\".");
                }
                allFlag = true;
            }
            if (!allFlag && !cm.ifAttrExists(tblName, *it))
            {
                throw API_Error(API_Error_Type::PARAMETER_ERROR, "[Select Error] The attribute " + *it + " doesn't exists");
            }
        }
        vector<BP_POINTER> vPointers2SeletedRecords = vector<BP_POINTER>();
        if (!SelectRecord(tblName, conditions, vPointers2SeletedRecords))
        {
            throw API_Error(API_Error_Type::INTERPRETATION_ERROR, "[Select Error] Unable to get the selected records from the records pool.");
        }

        Records * pRecs;
        rm.getRecordsObject(tblName, pRecs);
        vector<Attribute> * attributes = pRecs->attributes;
        if (allFlag)
        {
            cout << "|";
            for (auto it = attributes->begin(); it != attributes->end(); it++)
            {
                cout << " --- " + it->name + " --- |";
            }
            cout << endl;
            if (vPointers2SeletedRecords.size() != 0)
            {
                for (auto & it : vPointers2SeletedRecords)
                {
                    Record rec;
                    pRecs->retrieveRecord(it/10000, it%10000, rec);
                    cout << "|";
                    for (auto & itr : rec)
                    {
                        switch (itr.type)
                        {
                            case AttributeType::INT:
                            {
                                cout << "     " + to_string(itr.value.intValue) + "     |";
                                break;
                            }
                            case AttributeType::FLOAT:
                            {
                                cout << "     " + to_string(itr.value.floatValue) + "     |";
                                break;
                            }
                            case AttributeType::STRING:
                            {
                                string s = itr.value.charValue;
                                cout << "     " + s + "     |";
                                break;
                            }
                            default:
                            {
                                break;
                            }
                        }
                    }
                    cout << endl;
                }
            }
        }
        else
        {
            int *recordAttributeSerialNo = new int[selectedAttrList->size()];
            int i = 0;
            for (int i = 0; i < selectedAttrList->size(); i++)
            {
                for (int j = 0; j < attributes->size(); j++)
                {
                    if ((*selectedAttrList)[i] == (*attributes)[j].name)
                    {
                        recordAttributeSerialNo[i] = j;
                    }
                }
            }
            cout << "|";
            for (auto it = selectedAttrList->begin(); it != selectedAttrList->end(); it++)
            {
                cout << " --- " + *it + " --- |";
            }
            cout << endl;
            for (auto & it : vPointers2SeletedRecords)
            {
                Record rec;
                pRecs->retrieveRecord(it/10000, it%10000, rec);
                cout << "|";

                for (int i = 0; i < selectedAttrList->size(); i++)
                {
                    KeyValueTuple& itr = rec[recordAttributeSerialNo[i]];
                    switch (itr.type)
                    {
                        case AttributeType::INT:
                        {
                            cout << "     " + to_string(itr.value.intValue) + "     |";
                            break;
                        }
                        case AttributeType::FLOAT:
                        {
                            cout << "     " + to_string(itr.value.floatValue) + "     |";
                            break;
                        }
                        case AttributeType::STRING:
                        {
                            string s = itr.value.charValue;
                            cout << "     " + s + "     |";
                            break;
                        }
                        default:
                        {
                            break;
                        }
                    }
                }
                cout << endl;
            }
        }

        cout << endl;
        cout << "Success, with " + to_string(vPointers2SeletedRecords.size()) + " row(s) returned." << endl;
        return true;

    }
    catch (API_Error e)
    {
        cerr << e.message << endl;
        return false;
    }
}

bool API::InsertAll (string tblName, vector<ValueWithType> * values)
{
    try
    {
        if (!cm.ifTableExists(tblName))
        {
            throw API_Error(API_Error_Type::PARAMETER_ERROR, "Table \"" + tblName + "\" doesn't exist.");
        }

        if (CheckDuplicateKey(tblName, values))
        {
            throw API_Error(API_Error_Type::INTERPRETATION_ERROR, "Duplicate Keys");
        }

        Records* pRecs;
        if (rm.getRecordsObject(tblName, pRecs))
        {
            if (pRecs->insertRecord(values))
            {
                return true;
            }
            else
            {
                throw API_Error(API_Error_Type::INTERPRETATION_ERROR, "Failed to insert record.");
            }
        }
        else
        {
            throw API_Error(API_Error_Type::INTERPRETATION_ERROR, "Unable to get the records object of the table " + tblName + ".");
        }
    }
    catch (API_Error e)
    {
        cerr << e.message << endl;
        return false;
    }
    catch (RecordsError e)
    {
        cerr << e.message << endl;
        return false;
    }
}

bool API::InsertWithAttr (string tblName, vector<KeyValueTuple>* keyValues)
{
	return true;
}

bool API::DropTable (string tblName)
{
    try
    {
        if (!cm.ifTableExists(tblName))
        {
            throw API_Error(API_Error_Type::PARAMETER_ERROR, "Table \"" + tblName + "\" doesn't exist.");
        }

        Catalog& cat = cm.getCatalogByName(tblName);
        vector<Attribute> * attributes = cat.attributes;
        for (auto it = attributes->begin(); it != attributes->end(); it++)
        {
            if (it->hasindex)
            {
                Drop_Index(tblName, it->name);
            }
            if (it->primaryKey || it->unique)
            {
                Drop_Index(tblName, it->name);
            }
        }
        Records * pRec;
        if (!rm.getRecordsObject(tblName, pRec))
        {
            throw API_Error(API_Error_Type::INTERPRETATION_ERROR, "Unable to get the records object of the table " + tblName + ".");
        }
        pRec->deleteAllRecords();
        rm.deleteRecords(tblName);
        if (!cm.deleteCatalog(tblName))
        {
            throw API_Error(API_Error_Type::INTERPRETATION_ERROR, "Cannot delete the catalog of the table " + tblName + ".");
        }
        return true;
    }
    catch (API_Error e)
    {
        cerr << e.message << endl;
        return false;
    }
}

bool API::DropIndex (string tblName, string idxName)
{
    try
    {
        if (!cm.ifTableExists(tblName))
        {
            throw API_Error(API_Error_Type::PARAMETER_ERROR, "Table \"" + tblName + "\" doesn't exist.");
        }

        Catalog& cat = cm.getCatalogByName(tblName);
        vector<Attribute> * attributes = cat.attributes;
        Attribute * pAttr;

        if (!cm.ifIndexExistsByName(attributes, idxName, pAttr))
        {
            throw API_Error(API_Error_Type::PARAMETER_ERROR, "Index \"" + idxName + "\" on table " + tblName + " doesn't exist.");
        }

        // Index* index = Get_Index(tblName, pAttr->name, idxName + "@" + tblName, pAttr->type, pAttr->getSize());
        // Index_Map[idxName + "@" + tblName] = index;
        if (pAttr->unique || pAttr->primaryKey)
        {
            pAttr->hasindex = false;
            pAttr->indexName = "";
            cat.Write();
            return true;
        }
        Drop_Index(tblName, pAttr->name);
        pAttr->hasindex = false;
        pAttr->indexName = "";
        cat.Write();
        string indexFileName = tblName + "@" + pAttr->name;
        bm.DeleteFile(indexFileName);
        return true;
    }
    catch (API_Error e)
    {
        cerr << e.message << endl;
        return false;
    }
}

bool API::Delete (string tblName, vector<Condition>* conditions)
{
    try
    {
        vector<BP_POINTER> vPointers2SeletedRecords = vector<BP_POINTER>();
        if (!SelectRecord(tblName, conditions, vPointers2SeletedRecords))
        {
            throw API_Error(API_Error_Type::INTERPRETATION_ERROR, "[Delete Error] Unable to get the selected records from the records pool.");
        }

        Records * pRecs;
        rm.getRecordsObject(tblName, pRecs);
        for (auto & it : vPointers2SeletedRecords)
        {
            // Record rec;
            // pRecs->retrieveRecord(it/10000, it%10000, rec);
            int blockID = it/10000;
            int offset = it%10000;
            string tName = tblName + ".record";
            BLOCKS & block = bm.ReadBlocks(tName, blockID);
            block.Write_String("0", offset);
        }
        cout << "Success, with " + to_string(vPointers2SeletedRecords.size()) + " row(s) deleted.";

        return true;
    }
    catch (API_Error e)
    {
        cerr << e.message << endl;
        return false;
    }
}
#include <algorithm>
void API::MergeConditions(vector<Condition> * conditions)
{
    for (auto it = conditions->begin(); it != conditions->end(); it++)
    {
        it->tableName = currentTableCat->tablename;
        Attribute * a = cm.getAttributeByName(*currentTableCat, it->attrName);
        if (a->type == AttributeType::FLOAT && it->type == AttributeType::INT)
        {
            it->value.floatValue = (float)it->value.intValue;
        }
    }
    sort(conditions->begin(), conditions->end(), API::conditionCmp);
}

bool API::validateCondition(Record& rec, Condition & cond)
{
    Attribute * a = cm.getAttributeByName(*currentTableCat, cond.attrName);
    AttributeType attrType = a->type;
    Value v = retrieveAttrValueFromRecord(rec, cond.attrName);
    Value condValue = cond.value;

    switch (cond.opType)
    {
        case operatorType::EQ:
        {
            return ValueCmp(v, condValue, attrType) == 0;
            break;
        }
        case operatorType::LT:
        {
            return ValueCmp(v, condValue, attrType) < 0;
            break;
        }
        case operatorType::GT:
        {
            return ValueCmp(v, condValue, attrType) > 0;
            break;
        }
        case operatorType::LTE:
        {
            return ValueCmp(v, condValue, attrType) <= 0;
            break;
        }
        case operatorType::GTE:
        {
            return ValueCmp(v, condValue, attrType) >= 0;
            break;
        }
        case operatorType::NEQ:
        {
            return ValueCmp(v, condValue, attrType) != 0;
            break;
        }
    }
}

Value& API::retrieveAttrValueFromRecord(Record & rec, string attrName)
{
    for (auto it = rec.begin(); it != rec.end(); it++)
    {
        if (it->attrName == attrName)
            return it->value;
    }
}

int API::ValueCmp(const Value &a, const Value &b, AttributeType type)
{
	switch (type)
	{
	case AttributeType::INT:
		return a.intValue - b.intValue;
		break;
	case AttributeType::FLOAT:
		if (fabs(a.floatValue - b.floatValue) < 1e-6) return 0;
		else if (a.floatValue - b.floatValue < 0) return -1;
		else return 1;
		break;
	case AttributeType::STRING:
		return strcmp(a.charValue, b.charValue);
		break;
	default:
		assert(false);
		break;
	}
}

bool API::SelectRecord (string tblName, vector<Condition>* conditions, vector<BP_POINTER>& vReturnPointers)
{
	try
	{
	    if (conditions == NULL || conditions->size() == 0)
        {
            Records* pRecs;
            rm.getRecordsObject(tblName, pRecs);
            int recordsSize = pRecs->getRecordSize();
            int maxRecordPerBlock = pRecs->maxRecordsNumPerBlock;
            for (int i = 0; i < recordsSize; i++)
            {
                Record rec;
                if (!pRecs->retrieveRecord(i, rec))
                {
                    // If the record couldn't be retrieved (i.e. deleted)
                    continue;
                }
                int BlockID = i/maxRecordPerBlock;
                int offset = i%maxRecordPerBlock* (pRecs->sizePerRecord);
                vReturnPointers.push_back(BlockID * 10000 + offset);
            }
            return true;
        }

		if (!cm.ifTableExists(tblName))
		{
			throw API_Error(API_Error_Type::PARAMETER_ERROR, "Table \"" + tblName + "\" doesn't exist.");
		}
        for (auto it = conditions->begin(); it != conditions->end(); it++)
        {
            if (!cm.ifAttrExists(tblName, it->attrName))
            {
                throw API_Error(API_Error_Type::PARAMETER_ERROR, "The attribute" + it->attrName + "doesn't exists");
            }
        }

        currentTableCat = & cm.getCatalogByName(tblName);
        MergeConditions(conditions);
        // cerr << "CATALOG MAP SIZE" << cm.catalogMap.size() <<endl;
        auto itCon = conditions->begin();
        Attribute * firstAttr = cm.getAttributeByName(*currentTableCat, itCon->attrName);
        vector<BP_POINTER> pts;
        bool indexSearched = false;
        Records* pRecs;
        rm.getRecordsObject(tblName, pRecs);
        if (firstAttr->hasindex)
        {
            Index * index = Get_Index(tblName, firstAttr->name, firstAttr->name, firstAttr->type, firstAttr->getSize());
            AttributeType attrType = firstAttr->type;
            KTP valueType = itCon->type;
            if (attrType != valueType)
            {
                if (attrType == AttributeType::FLOAT && valueType == AttributeType::INT)
                {
                    itCon->value.floatValue = (float)itCon->value.intValue;
                }
                throw API_Error(API_Error_Type::INTERPRETATION_ERROR, "[Select Error] Value types are incompatible.");
            }
            KEY key = KEY();
            key.ktype = attrType;
            switch (attrType)
            {
                case AttributeType::INT:
                {
                    key.int_data = itCon->value.intValue;
                    key.len = sizeof(int);
                    break;
                }
                case AttributeType::FLOAT:
                {
                    key.float_data = itCon->value.floatValue;
                    key.len = sizeof(float);
                    break;
                }
                case AttributeType::STRING:
                {
                    strcpy(key.str_data, itCon->value.charValue);
                    key.len = itCon->value.charValueLen * sizeof(char);
                    break;
                }
            }
            switch (itCon->opType)
            {
                case operatorType::EQ:
                {
                    index->RangeSearchInc(pts, key, key);
                    indexSearched = true;
                    break;
                }
                case operatorType::LT:
                {
                    index->SingleRangeSearch(pts, key, 0);
                    indexSearched = true;
                    break;
                }
                case operatorType::GT:
                {
                    index->SingleRangeSearch(pts, key, 1);
                    indexSearched = true;
                    break;
                }
                case operatorType::LTE:
                {
                    index->SingleRangeSearchInc(pts, key, 0);
                    indexSearched = true;
                    break;
                }
                case operatorType::GTE:
                {
                    index->SingleRangeSearchInc(pts, key, 1);
                    indexSearched = true;
                    break;
                }
                case operatorType::NEQ:
                {
                    indexSearched = false;
                    break;
                }
                default:
                {
                    throw API_Error(API_Error_Type::INTERPRETATION_ERROR, "[Select Error] Internel index search operation determination failed.");
                }
            }
        }

        vector<Attribute> * attributes = currentTableCat->attributes;
        if (indexSearched)
        {
            for (auto ipt = pts.begin(); ipt != pts.end(); ipt++)
            {
                Record rec;
                if (!pRecs->retrieveRecord(*ipt/10000, *ipt%10000, rec))
                {
                    // If the record couldn't be retrieved (i.e. deleted)
                    continue;
                }
                // Validate Conditions
                bool flag = true;
                for (auto it = conditions->begin(); flag && it != conditions->end(); it++)
                {
                    flag = validateCondition(rec, *it);
                }
                if (flag)
                {
                    vReturnPointers.push_back(*ipt);
                }
            }
        }
        else 
        {
            int recordsSize = pRecs->getRecordSize();
            int maxRecordPerBlock = pRecs->maxRecordsNumPerBlock;
            for (int i = 0; i < recordsSize; i++)
            {
                Record rec;
                if (!pRecs->retrieveRecord(i, rec))
                {
                    // If the record couldn't be retrieved (i.e. deleted)
                    continue;
                }
                // Validate conditions
                bool flag = true;
                for (auto it = conditions->begin(); flag && it != conditions->end(); it++)
                {
                    flag = validateCondition(rec, *it);
                }
                if (!flag)
                    continue;
                int BlockID = i/maxRecordPerBlock;
                int offset = i%maxRecordPerBlock* (pRecs->sizePerRecord);
//                cerr << "BlockID: " << BlockID << " offset:" << offset << endl;
                vReturnPointers.push_back(BlockID * 10000 + offset);
            }
        }
        
		return true;
	}
	catch (API_Error e)
	{
		cerr << e.message << endl;
		return false;
	}
}


bool API::conditionCmp(Condition& a, Condition& b)
{
    // NEQ conditions are of least precedence
    if (a.opType == operatorType::NEQ)
        return false;
    else if (b.opType == operatorType::NEQ)
        return true;
    if (a.attrName == b.attrName)
    {
        // Same attribute, then EQ is prioritized.
        if (a.opType == operatorType::EQ)
            return true;
        else if (b.opType == operatorType::EQ)
            return false;
        else if (a.opType == operatorType::NEQ)
            return false;
        else if (b.opType == operatorType::NEQ)
            return true;
        else
            return true;
    }
    Attribute* aA = cm.getAttributeByName(a.tableName, a.attrName);
    Attribute* bA = cm.getAttributeByName(b.tableName, b.attrName);
    if (aA->hasindex)
    {
        if (!bA->hasindex)
            return true;
        if (aA->primaryKey)
            return true;
    }
    else
    {
        if (bA->primaryKey)
            return false;
        if (bA->hasindex)
            return false;
    }
    // Now both aA and bA have index or don't have index at all
    if (a.opType == operatorType::EQ)
        return true;
    else if (b.opType == operatorType::EQ)
        return false;
    else if (a.opType == operatorType::NEQ)
        return false;
    else if (b.opType == operatorType::NEQ)
        return true;
    else
        return true;
}

// If duplicates are found, return true;
// As for attributes that's unique, this function tests if the record to insert fit these constraints.
bool API::CheckDuplicateKey(string tblName, vector<ValueWithType> * values)
{
    Catalog & cat = cm.getCatalogByName(tblName);
    auto attributes = cat.attributes;
    auto itV = values->begin();
    auto itA = attributes->begin();
    for (; itA != attributes->end() && itV != values->end(); )
    {
        if (itA->primaryKey || itA->unique)
        {
            KEY key = KEY();
            Index * index = Get_Index(tblName, itA->name, itA->name, itA->type, itA->getSize());
            AttributeType attrType = itA->type;
            KTP valueType = itV->type;
            if (attrType != valueType)
            {
                if (attrType == AttributeType::FLOAT && valueType == AttributeType::INT)
                {
                    itV->value.floatValue = (float)itV->value.intValue;
                }
                throw API_Error(API_Error_Type::INTERPRETATION_ERROR, "[Select Error] Value types are incompatible.");
            }
            key.ktype = attrType;
            switch (attrType)
            {
                case AttributeType::INT:
                {
                    key.int_data = itV->value.intValue;
                    key.len = sizeof(int);
                    break;
                }
                case AttributeType::FLOAT:
                {
                    key.float_data = itV->value.floatValue;
                    key.len = sizeof(float);
                    break;
                }
                case AttributeType::STRING:
                {
                    strcpy(key.str_data, itV->value.charValue);
                    key.len = itV->value.charValueLen * sizeof(char);
                    break;
                }
            }
            vector<BP_POINTER> pts = vector<BP_POINTER>();
            index->RangeSearchInc(pts, key, key);
            if (pts.size() == 1)
                return true;
        }
        itV++;
        itA++;
    }
    return false;
}

bool API::InsertAllRecordToIndex(Index * index, string tblName, string attrName)
{
    Records * pRecs;
    if (!rm.getRecordsObject(tblName, pRecs))
    {
        throw API_Error(API_Error_Type::INTERPRETATION_ERROR, "Unable to get the records object of the table " + tblName + ".");
    }
    int recordsSize = pRecs->getRecordSize();
    int maxRecordPerBlock = pRecs->maxRecordsNumPerBlock;
    auto attributes = pRecs->attributes;
    int ii;
    for (ii = 0; ii < attributes->size(); ii++)
    {
        if ((*attributes)[ii].name == attrName)
        {
            break;
        }
    }
    AttributeType attrType = (*attributes)[ii].type;
    for (int i = 0; i < recordsSize; i++)
    {
        Record rec;
        if (!pRecs->retrieveRecord(i, rec))
        {
            // If the record couldn't be retrieved (i.e. deleted)
            continue;
        }
        KEY key = KEY();
        switch (attrType)
        {
            case AttributeType::INT:
            {
                if (rec[ii].type != attrType)
                {
                    throw RecordsError("[Records Error] Type incompatible in attribute: " + attrName);
                }
                key.ktype = attrType;
                key.int_data = rec[ii].value.intValue;
                key.len = sizeof(int);
                break;
            }
            case AttributeType::FLOAT:
            {
                float f;
                if (rec[ii].type == AttributeType::INT)
                {
                    f = (float)rec[ii].value.intValue;
                }
                else if (rec[ii].type == AttributeType::FLOAT)
                {
                    f = rec[ii].value.floatValue;
                }
                else
                    throw RecordsError("[Records Error] Type incompatible in attribute: " + attrName);
                key.ktype = attrType;
                key.float_data = f;
                key.len = sizeof(int);
                break;
            }
            case AttributeType::STRING:
            {
                if (rec[ii].type != attrType)
                {
                    throw RecordsError("[Records Error] Type incompatible in attribute: " + attrName);
                }
                int stringLen = rec[ii].value.charValueLen;
                key.ktype = attrType;
                strcpy(key.str_data, rec[ii].value.charValue);
                key.len = sizeof(char) * stringLen;
                break;
            }
        }
        int blockID = floor(i * 1.0 / pRecs->maxRecordsNumPerBlock);
        int offset = (i % pRecs->maxRecordsNumPerBlock) * pRecs->sizePerRecord;
        int pt = blockID * 10000 + offset;
        index->Insert(index->ReadNode(ROOT), BPKEY{key, pt});
    }
}