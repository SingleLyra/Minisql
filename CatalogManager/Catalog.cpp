//
// Created by Robert Chen on 2019-04-30.
//

#include "Catalog.h"
//extern BufferManager bm;



map<string, Catalog_Index* > map_name_to_index; // indexname => index
map<string, Catalog_Index* > map_name_to_table; // 


void Catalog::Modify(string tablename, vector<Attribute>* t)
{
	attributes = t;
	this->tablename = tablename;
}

void Catalog::Build() // after modify then build the metadata
{
	table_block_cnt = 0;
	usage = 0;
	Write();
}

void Catalog::Read()
{
	fstream fin;
	if (!File_Exist(tablename + ".catalog")) Build();
	attributes = new vector<Attribute>();
	fin.open(tablename + ".catalog", ios::in | ios::binary);
	fin >> table_block_cnt >> attributes_cnt;
	for (int i = 0; i < attributes_cnt; ++i) {
		Attribute x; fin >> x;
		attributes->push_back(x);
	}
	fin >> usage;
}
void Catalog::Write()
{
	fstream fout;
	fout.open(tablename + ".catalog", ios::out | ios::binary | ios::trunc);
	fout << table_block_cnt << " " << attributes_cnt << "\n";
	for (int i = 0; i < attributes_cnt; ++i) {
		Attribute x = attributes->at(i); // (*attribute)[1]
		fout << x;
	}
	fout << usage << '\n';
}

void Catalog::addBlock()
{
	table_block_cnt++;
	usage = 0;
//	this->Write();
}

void Catalog::incrementUsage()
{
    usage++;
//    this->Write();
}

extern map<string, Catalog_Index> map_index_catlog; // index name => catalog

void Catalog_Index :: CreateCatalog(string tablename, string attribute_name, string index_name) // NEW Catalog
{
    cerr << "Create Catalog" << endl;
	this->tablename = tablename;
	this->attribute_name = attribute_name;
	this->index_name = index_name;
	this->usage = 1; // ������һ�����õĽڵ�
	Write();
}