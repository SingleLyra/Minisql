//
// Created by Weiye Chen on 2019-06-07.
//

#include "Records.h"

Records::Records(string tblName)
{
    tableName = tblName;
    sizePerRecord = 0;
    catalog = &(cm.getCatalogByName(tblName));
    attributes = catalog->attributes;
    for (auto it = attributes->begin(); it != attributes->end(); it++)
    {
        switch (it->type)
        {
            case AttributeType::INT:
            {
                sizePerRecord += sizeof(int);
                break;
            }
            case AttributeType::FLOAT:
            {
                sizePerRecord += sizeof(float);
                break;
            }
            case AttributeType::STRING:
            {
                int varcharLen = it->VARCHAR_LEN;
                sizePerRecord += (varcharLen * sizeof(char));
                break;
            }
            case AttributeType::UNDEF:
            {
                assert(0);
                break;
            }
        }
    }
    sizePerRecord++;
    maxRecordsNumPerBlock = int(BLOCK_SIZE/sizePerRecord);
}

bool Records::createRecordFile()
{
    fstream fout;
    fout.open( (this->tableName + ".record"), ios::out | ios::binary);
    if (fout.good())
    {
        return true;
    }
    return false;
}

bool Records::deleteAllRecords()
{
    remove((this->tableName + ".record").c_str());
    return true;
}

bool Records::insertRecord(Record * rec)
{
    int blockID = catalog->table_block_cnt - 1;
    if (blockID == -1)
    {
        catalog->addBlock();
        blockID++;
    }
    if (catalog->usage == maxRecordsNumPerBlock)
    {
        catalog->addBlock();
        blockID++;
    }
    BLOCKS & block = bm.ReadBlocks(tableName, blockID);

    int offset = catalog->usage * sizePerRecord;

    for (auto it = attributes->begin(); it != attributes->end(); it++)
    {
        string attrName = it->name;
        AttributeType attrType = it->type;

        KEY key;
        switch (attrType)
        {
            case AttributeType::INT:
            {
                // Find the corresponding key-value tuple in the record
                for (auto itt = rec->begin(); itt != rec->end(); itt++)
                {
                    if (itt->attrName == attrName)
                    {
                        block.Write_Int(itt->value.intValue, offset);
                        key.int_data = itt->value.intValue;
                        break;
                    }
                }
                offset += sizeof(int);
                break;
            }
            case AttributeType::FLOAT:
            {
                // Find the corresponding key-value tuple in the record
                for (auto itt = rec->begin(); itt != rec->end(); itt++)
                {
                    if (itt->attrName == attrName)
                    {
                        block.Write_Float(itt->value.floatValue, offset);
                        key.float_data = itt->value.floatValue;
                        break;
                    }
                }
                offset += sizeof(float);
                break;
            }
            case AttributeType::STRING:
            {
                int stringLen = it->VARCHAR_LEN;
                // Find the corresponding key-value tuple in the record
                for (auto itt = rec->begin(); itt != rec->end(); itt++)
                {
                    if (itt->attrName == attrName)
                    {
                        block.Write_String(itt->value.charValue, offset, stringLen);
                        strcpy(key.str_data, itt->value.charValue);
                        break;
                    }
                }
                offset += (sizeof(char) * stringLen);
            }
        }
        bool hasIndex = it->hasindex;
        if (hasIndex)
        {
            assert(offset >= 10000);
            int pt = blockID * 10000 + offset;
            // int blockID = pt / 10000, offset = pt % 10000;
            Index * idx = Get_Index(tableName, it->name, (tableName + "@" + it->indexName), it->type, it->getSize());
            
            idx->Insert(idx->ReadNode(ROOT), BPKEY{key, pt});
        }
    }
    catalog->incrementUsage();
    return true;
}

bool Records::insertRecord(vector<ValueWithType> * values)
{
    int blockID = catalog->table_block_cnt - 1;
    if (blockID == -1)
    {
        catalog->addBlock();
        blockID++;
    }
    if (catalog->usage == maxRecordsNumPerBlock)
    {
        catalog->addBlock();
        blockID++;
    }
    string tName = tableName + ".record";
    int offset = catalog->usage * sizePerRecord;

    BLOCKS & block = bm.ReadBlocks(tName, blockID);

//    vector<Attribute>* attributes = attributes;
    auto itAttr = attributes->begin();
    auto itVal = values->begin();
    // For presence mark: when deleted, change this to 0

    block.Write_String("1", offset++);
    int Init_BlockID = blockID, Init_offset = offset - 1;
    //offset += 4;
    while (itAttr != attributes->end() && itVal != values->end())
    {
        string attrName = itAttr->name;
        AttributeType attrType = itAttr->type;
        KEY key = KEY();
        switch (attrType)
        {
            case AttributeType::INT:
            {
                if (itVal->type != attrType)
                {
                    throw RecordsError("[Records Error] Type incompatible in attribute: " + attrName);
                }
                block.Write_Int(itVal->value.intValue, offset);
                key.ktype = attrType;
                key.int_data = itVal->value.intValue;
                key.len = sizeof(int);

                offset += sizeof(int);
                break;
            }
            case AttributeType::FLOAT:
            {
                float f;
                if (itVal->type == AttributeType::INT)
                {
                    f = (float)itVal->value.intValue;
                }
                else if (itVal->type == AttributeType::FLOAT)
                {
                    f = itVal->value.floatValue;
                }
                else
                    throw RecordsError("[Records Error] Type incompatible in attribute: " + attrName);
                block.Write_Float(f, offset);
                key.ktype = attrType;
                key.float_data = f;
                key.len = sizeof(int);

                offset += sizeof(float);
                break;
            }
            case AttributeType::STRING:
            {
                if (itVal->type != attrType)
                {
                    throw RecordsError("[Records Error] Type incompatible in attribute: " + attrName);
                }
                int stringLen = itAttr->VARCHAR_LEN;
                block.Write_String(itVal->value.charValue, offset, stringLen);
                key.ktype = attrType;
                strcpy(key.str_data, itVal->value.charValue);
                key.len = sizeof(char) * stringLen;

                offset += (sizeof(char) * stringLen);
                break;
            }
        }
        bool hasIndex = itAttr->hasindex;
        if (hasIndex)
        {
//            assert(offset >= 10000);
            int pt = Init_BlockID * 10000 + Init_offset;
            // int blockID = pt / 10000, offset = pt % 10000;
//            cerr << "Now Insert Index" << endl;
            Index * idx = Get_Index(tableName, itAttr->name, (tableName + "@" + itAttr->indexName), itAttr->type, itAttr->getSize());
            
            idx->Insert(idx->ReadNode(ROOT), BPKEY{key, pt});
        }
        itAttr++;
        itVal++;
    }

    catalog->incrementUsage();
    //bm.FlushAllBlocks();
    return true;
}

//bool Records::insertRecords(vector<Record> * vRec)
//{
//    for (auto it = vRec->begin(); it != vRec->end(); it++)
//    {
//        insertRecord(&(*it));
//    }
//    return true;
//}

bool Records::retrieveRecord(int recordID, Record& record)
{
    int blockID = floor(recordID * 1.0 / maxRecordsNumPerBlock);
    int offset = (recordID % maxRecordsNumPerBlock) * sizePerRecord; // TODO:
//    cerr << "OFFSET IN RETRIEVE RECORD: " << offset << endl;
    return retrieveRecord(blockID, offset, record);
}

bool Records::retrieveRecord(int blockID, int offset, Record& record)
{
	record = Record();
	string tb = tableName + ".record";
	BLOCKS & block = bm.ReadBlocks(tb, blockID);
	// cerr << "ReadBlock" << tb << " " << blockID << endl;
	// The offset input is based on record, while the offset on the disk is based on bytes
//    offset *= sizePerRecord;

	string presenceMark;
	block.Read_String(presenceMark, sizeof(char), offset++);
	// If the record is marked as deleted
	if (presenceMark == "0")
		return false;

	for (auto it = attributes->begin(); it != attributes->end(); it++)
	{
		KeyValueTuple kvt = KeyValueTuple(it->name);
		switch (it->type)
		{
		case AttributeType::INT:
		{
			int value;
			block.Read_Int(value, sizeof(int), offset);
			kvt.setValue(value);
			offset += sizeof(int);
			break;
		}
		case AttributeType::FLOAT:
		{
			float value;
			block.Read_Float(value, sizeof(float), offset);
			kvt.setValue(value);
			offset += sizeof(float);
			break;
		}
		case AttributeType::STRING:
		{
			//char* value = new char[it->VARCHAR_LEN + 1];
			string tmp;
			block.Read_String(tmp, it->VARCHAR_LEN, offset);
			//                kvt.setValue(value, it->VARCHAR_LEN);
			//                cerr <<"STRING ewscaedsvcx" << tmp << " " << it->VARCHAR_LEN<< endl;
			kvt.setValue(tmp, it->VARCHAR_LEN);
			offset += (sizeof(char) * it->VARCHAR_LEN);
			break;
		}
		}
		record.push_back(kvt);
	}
	return true;
}