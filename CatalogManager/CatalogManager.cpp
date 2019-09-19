//
// Created by Weiye Chen on 2019-06-02.
//

#include "CatalogManager.h"
using namespace std;

CatalogManager cm = CatalogManager();

void CatalogManager::initCatalogMap()
{
    catalogMap = map<string, Catalog*>();
    tableList = set<string>();
    fstream fin;
    if (!File_Exist("CatalogList"))
    {
        fstream fout;
        fout.open("CatalogList", ios::out | ios::binary);
        fout << ""; 
        fout.close();
    }
    fin.open("CatalogList", ios::in | ios::binary);
    string tblName;
    while (getline(fin, tblName))
    {
        Catalog* cat = new Catalog(tblName);
        cat->Read();
//        catalogMap.emplace(tblName, cat);
        catalogMap[tblName] = cat;
    }
    setTablesList();
}


bool CatalogManager::ifTableExists(string tblName)
{
    map<string, Catalog*>::iterator it;
    it = catalogMap.find(tblName);
    if (it == catalogMap.end())
    {
        return false;
    }
    return true;
}

bool CatalogManager::ifAttrExists(string tblName, string attrName)
{
    if (NULL != getAttributeByName(tblName, attrName))
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CatalogManager::ifIndexExistsByName(Catalog & catalog, string idxName)
{
    return ifIndexExistsByName(catalog.attributes, idxName);
}

bool CatalogManager::ifIndexExistsByName(vector<Attribute> * attrs, string idxName)
{
    vector<Attribute>::iterator it = attrs->begin();
    while (it != attrs->end())
    {
        if (it->indexName == idxName)
            return true;
        it++;
    }
    return false;
}

bool CatalogManager::ifIndexExistsByName(vector<Attribute> * attrs, string idxName, Attribute*& attr_out)
{
    vector<Attribute>::iterator it = attrs->begin();
    while (it != attrs->end())
    {
        if (it->indexName == idxName)
        {
            attr_out = &*it;
            return true;
        }
        it++;
    }
    return false;
}

bool CatalogManager::ifIndexExistsOnAttr(Attribute * attr)
{
    return (attr->hasindex);
}

Catalog & CatalogManager::getCatalogByName(string tblName)
{
    map<string, Catalog*>::iterator it;
    it = catalogMap.find(tblName);
    return *it->second;
}

Attribute * CatalogManager::getAttributeByName(string tblName, string attrName)
{
    if (!ifTableExists(tblName))
        return NULL;
    Catalog& cat = getCatalogByName(tblName);
    return getAttributeByName(cat, attrName);
}

Attribute * CatalogManager::getAttributeByName(Catalog & catalog, string attrName)
{
    vector<Attribute>* attrs = catalog.attributes;
    auto it = attrs->begin();
    while (it != attrs->end())
    {
        if (it->name == attrName)
            return &(*it);
        it++;
    }
    return NULL;
}

void CatalogManager::setTablesList()
{
    auto it = catalogMap.begin();
    while (it != catalogMap.end())
    {
        tableList.insert(it->first);
        it++;
    }
}

Catalog& CatalogManager::createCatalog(string tblName, vector<Attribute>* attributes)
{
   Catalog *cat = new Catalog(tblName);
   assert(cat != NULL);
   cat->attributes = attributes;
   cat->usage = 0;
   cat->attributes_cnt = attributes->size();
   cat->table_block_cnt = 0;
   tableList.insert(tblName);
   return *cat;
}
#include <cassert>
bool CatalogManager::addCatalog(Catalog & catalog)
{
    string tblName = catalog.tablename;
//    catalogMap.insert(pair<string, Catalog*>(tblName, &catalog));
    catalogMap[tblName] = &catalog;
    assert(catalogMap.find(tblName) != catalogMap.end());
    catalog.Build();
    fstream fout;
    fout.open("CatalogList", ios::out | ios::trunc | ios::binary);
    for (auto it = tableList.begin(); it != tableList.end(); it++)
    {
        fout << *it << "\n";
    }
    return true;
}

bool CatalogManager::deleteCatalog(string tblName)
{
    catalogMap.erase(tblName);
    remove((tblName + ".catalog").c_str());

    // remove from file 'CatalogList': a violent implementation
    fstream fout;
    fout.open("CatalogList", ios::out | ios::trunc);    // discard all content in previous file.
    tableList.erase(tblName);
    for (auto it = tableList.begin(); it != tableList.end(); it++)
    {
        fout << *it << "\n";
    }

	return true;
	// WARNING: return true or false?
    // TODO: in record manager, the record data should also be deleted.
}

void CatalogManager::FlushAllCatalogs()
{
    for (auto & it : catalogMap)
    {
        it.second->Write();
    }
}