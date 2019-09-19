//
// Created by Weiye Chen on 2019-06-02.
//

#ifndef MINISQL_CATALOGMANAGER_H
#define MINISQL_CATALOGMANAGER_H

#include "Catalog.h"
#include "../BufferManager/BufferManager.h"
#include "../minisql.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <vector>
#include <map>
#include <set>

using namespace std;

extern BufferManager bm;

class CatalogManager {
public:

    CatalogManager()
    {
        initCatalogMap();
    }
    /// Since all its members are static, no overloading constructor is needed.

    /* --------- In-Memory catalog instances --------- */

//    FILE * catalogListFile;
    map<string, Catalog*> catalogMap;
    set<string> tableList;

    /* --------- Catalog List --------- */

    // Initializing the catalog map: every time the system starts, load all catalogs into memory.
    void initCatalogMap();

    /* --------- Existence test --------- */

    // Check if the table exists
    bool ifTableExists(string tblName);
    bool ifAttrExists(string tblName, string attrName);
    bool ifIndexExistsByName(Catalog & catalog, string idxName);
    bool ifIndexExistsByName(vector<Attribute> * attrs, string idxName);
    bool ifIndexExistsByName(vector<Attribute> * attrs, string idxName, Attribute*& attr_out);
    bool ifIndexExistsOnAttr(Attribute * attr);

    /* --------- Catalog Accessor --------- */

    // Getting a catalog object: Assume existence of the object, and use `ifTableExists` before calling this function.
    Catalog & getCatalogByName(string tblName);
    Attribute * getAttributeByName(string tblName, string attrName);
    Attribute * getAttributeByName(Catalog & catalog, string attrName);

    /* --------- Information Query --------- */

    void setTablesList();
    int getRecordSize(string tblName);

    /* --------- Methods --------- */

    // create an instance of Catalog for new table.
    Catalog& createCatalog(string tblName, vector<Attribute>* attributes);
    // Add catalog to in-memory map, and write back to disk
    bool addCatalog(Catalog & catalog);
    // delete catalog object;
    bool deleteCatalog(string tblName);

    void FlushAllCatalogs();
};


#endif //MINISQL_CATALOGMANAGER_H
