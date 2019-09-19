//
// Coded by Xinning Zhang and Weiye Chen
//

#ifndef MINISQL_CATALOG_H
#define MINISQL_CATALOG_H
#include "../BufferManager/BufferManager.h"
#include <vector>
#include "Attribute.h"

class Catalog { //记得把map写回去
/* catalog: tablename.catalog // metadata
	select(tablename, ....)
	cin >>
	1. TABLE name; int FILE_BLOCK_CNT;
		2. TABLE attribute_cnt; // 属性的数目有几�? 是否为primary key or unique
		3. attributes[attribute_cnt] : len type  // 每个属性的名字, 长度, 类型
			4. USAGE(一个int) 最后一块有几个记录
			// 最后一块是哪块
*/
public:
	Catalog(){}
	Catalog(string tblName){ tablename = tblName; }
	
	string tablename;
	int table_block_cnt, attributes_cnt;
	vector<Attribute>* attributes;
	int usage; // /10000 => block_size  %10000 => offset
	void Build();
	void Read();
	void Write();
	void Modify(string filename, vector<Attribute>* t);
	void addBlock();
    void incrementUsage();
};

// table @ attri @ index

class Catalog_Index // 由于catalog无法读取B+tree的新节点信息，所以非常凉凉，这样我们先读入一下现在做了多少个B+ tree 另外，记得把map写回去
{
public:
	string tablename, attribute_name, index_name;
	int usage; // number of b+ tree
	void CreateCatalog(string tablename, string attribute_name, string index_name); // NEW Catalog
	void Read() //从磁盘中读入
	{
		fstream fin;
		// printf("! %s\n", index_name.c_str());
		if (!File_Exist(tablename+"@"+ attribute_name + ".catalog"))
		    puts("That index did't exist!");
		fin.open(tablename + "@" + attribute_name + ".catalog", ios::in | ios::binary);
		fin >> tablename >> attribute_name >> index_name;
		fin >> usage;
		fin.close();
	}
	void Write() 
	{
		fstream fout;
		// printf("Now Print %s\n", index_name.c_str());
		if (!File_Exist(tablename + "@" + attribute_name + ".catalog")) puts("Write index! did't exist!");
		fout.open(tablename + "@" + attribute_name + ".catalog", ios::out | ios::binary);
		fout << tablename << " " << attribute_name  << " " << index_name << "\n";
		fout << usage << "\n";
		fout.close();
	}
	void Add(int usg) { usage += usg; Write(); } // 这个可以快速更新usg！
};
// drop t;
// t = Getindex(indexname);
// table @ attri @ index

#endif //MINISQL_CATALOG_H
/*
if()
*/