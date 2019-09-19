//
// Coded By Xinning Zhang
//

#ifndef MINISQL_INDEX_H
#define MINISQL_INDEX_H
#include "node.hpp"
/*
1. create在catalog里面
2. catalog要调用file 是不是存在
*/
class Catalog_Index;
class Index // (ROOTPAGE)
{
public:
	Index(string tablename, string colname, string indexname, KTP ktp, int len); // get name from <table, col>
	string tablename, colname, index_name;
	KTP ktp;
	int len;
	// functions

	Node ReadNode(BP_POINTER pt); //get index file
	Node NewNode();
	Catalog_Index* q;

	//B+ tree:
	bool Insert(Node x, BPKEY val);
	bool InsertValue(Node &x, BPKEY val);
	// bool NeedSplit(Node x);
	void Split(Node x);
	Node SplitLeaf(Node &x);
	Node SplitNotLeaf(Node &x);
	KEY GetMinKey(Node x);
	bool Delete(Node x, BPKEY val);
	// bool DeleteValue(Node &x, BPKEY val);
	void RangeDelete(KEY L, KEY R); // 开区间
	void SingleRangeDelete(KEY X, bool TYPE);// 开区间 == 0 v:<= X / 1: v: >= X
	
	Node Search(Node x, const KEY &val);
	void RangeSearch(vector<BP_POINTER> &ans, KEY L, KEY R); // 开区间
	void RangeSearchInc(vector<BP_POINTER> &ans, KEY L, KEY R); // 闭区间
	void SingleRangeSearch(vector<BP_POINTER> &ans, KEY X, bool TYPE); // 开区间 == 0 v:<= X / 1: v: >= X
	void SingleRangeSearchInc(vector<BP_POINTER> &ans, KEY X, bool TYPE); // 闭区间 == 0 v:<= X / 1: v: >= X
	
	void dfs(Node x,int dep);
	Node GetMinLeaf(Node x);
};
void Drop_Index(string tablename, string attributename);
void IndexTest();
Index*  Create_Index(string tablename, string colname, string index, KTP ktp, int len);
// create in buffer! but no one knows when to flush.
Index*  Get_Index(string tablename, string index, string colname, KTP ktp, int len);



#endif //MINISQL_INDEX_H