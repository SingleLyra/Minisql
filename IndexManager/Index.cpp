//
// Coded by Xinning Zhang
// Warning: for every upd, don't forget to writedata

#include "Index.h"
#include "node.hpp"
#include "../BufferManager/BufferManager.h"
#include "../CatalogManager/Catalog.h"
#include <cassert>
using namespace std;
/*
Update On 6.5: add element: Index name
// To load one index into your memory, you should use Get_Index function to get a pointer. [if exists]
*/
#include <stack>
map<string, Index*> Index_Map; // 写回
map<string, Catalog_Index> map_index_catlog; // index_name Catalog_index
static stack<Node> node_stack;


// WARNING: 一定要开一个 map<string, string> 表示插入的时候 index name 和 index col table 联系起来

void PrintNode(Node &x)
{
	printf("%d %d\n", x.header.usage, x.header.isleaf);
	int cur = 0;
	for (auto i : x.sons)
	{
		// if(x.header.isleaf == BPTP ::NOTLEAF)
		if (cur != 0)
			for (int j = 0; j < 6; ++j)
				printf("%c", i.key.str_data[j]);
		++cur;
		if (cur != 1)printf(" ");
	}
	puts("");
}
extern BufferManager bm;
Index::Index(string tablename, string colname, string indexname, KTP ktp, int len) // get name from <table, col>
{
	assert(tablename != "");
	assert(colname != "");
	assert(ktp != KTP::UNDEF);
	this->q = new Catalog_Index; // new node
	this->tablename = tablename, this->colname = colname, this->ktp = ktp, this->len = len, this->index_name = indexname;
	this->q->tablename = tablename, this->q->attribute_name = colname, this->q->index_name = indexname;
	// 然后别read！！
	Node t(ReadNode(ROOT)); // (in disk index will be read in this step)
	if (t.header.isleaf == BPTP::UNDEF) // NULL Node (no indices ever created)
	{
		//.. Create BPNode;
		// 加入一下 Index_map 代表读入了这个index
		// assert(t.sons.size() == 1);
		//cerr << "UNDEF???" << this->colname << this->q->usage << endl;
		q->usage = 0;
		t.header.usage = 1; KEY x;
		t.sons.push_back(BPKEY{ x, BP_NULL });
		// init Node(t).header
		t.header.isleaf = BPTP::LEAF;
		t.header.ktp = ktp; t.header.len = len;
		t.header.sib = BP_NULL;
		t.WriteData();
	}
	else
    {
	    q->Read();
	  //  cerr << "Read!" << q->usage << endl;
    }
	
}

Index*  Get_Index(string tablename, string colname, string index, KTP ktp, int len) // WARNING: index didn't exist
//从硬盘里读取一个Index 如果没有就新建 如果有就直接加载 总能返回一个index
{
	if (Index_Map[tablename +"@"+ colname+".index"] != NULL) return Index_Map[tablename + "@" + colname + ".index"];
	Index* p;// = Get_Index(tablename, colname, ktp, len);
	//cerr << "???" << endl;
	p = new Index(tablename, colname, index, ktp, len); // 不存在则new一个！
	Index_Map[tablename + "@" + colname + ".index"] = p; // 已经读入的index
	// p->q->Read(); // WARNING: 这个好像不一定存在
	puts("Index Loaded OK!");
	//cerr << p->q->usage << endl;
	return p;
}

// 1. index not exist
// 2. index exists but not in memory
// 3. index exists and in memory

Index* Create_Index(string tablename, string colname, string index, KTP ktp, int len)
{
	Index* p;
	p = Get_Index(tablename, colname, index, ktp, len);
	puts("Index Created! Now Create Catalog");
	p->q->CreateCatalog(tablename, colname, index); // Create Index
	return p;
}

Node Index::ReadNode(BP_POINTER pt) //unfinished;
{
	Node t; string filename = "";
	filename += tablename; filename += "@"; filename += colname; filename += ".index";
	//提供一种构造方式
	t.msg = &bm.ReadBlocks(filename, (int)pt); //绑上
	// t.msg->SetPin(true); // WARNING: UNPIN
	t.ReadData(); 
	return t;
}

// KEY Index::Search(BP_POINTER pt); //unfinished;
Node Index::NewNode()
{
    //cerr << "NewNode" << endl;
	this->q->usage ++;
	Node t; string filename = ""; // warning: rewrite
	filename += tablename; filename += "@"; filename += colname; filename += ".index";
	t.msg = &bm.ReadBlocks(filename, this->q->usage);
	t.header.usage = 1;
	t.header.ktp = ktp;
	t.header.len = len;
	t.header.sib = BP_NULL;
	t.header.isleaf = BPTP::UNDEF;
	KEY x; t.sons.push_back(BPKEY{ x, BP_NULL });
	return t;
}

// finished
void Drop_Index(string tablename, string attributename) // WARNING: unfinished
{
	Catalog_Index q;
	q.tablename = tablename, q.attribute_name = attributename;
	// q.index_name = index_name;
	q.Read();
	string index_path = q.tablename + "@" + q.attribute_name + ".index";
	remove(index_path.c_str());
	string catalog_path = q.tablename + "@" + q.attribute_name + ".catalog";
	remove(catalog_path.c_str());
}

// the data haven't been written back but "&&&&&"
bool Index::InsertValue(Node &x, BPKEY val)
{
	for (vector<BPKEY> ::iterator it = x.sons.begin() + 1; it != x.sons.end(); ++ it)
	{
		BPKEY t = (*it);
		if (t.key < val.key);
		else if (t.key == val.key) return false;
		else return x.sons.insert(it,val), x.header.usage ++, true;
	}
	x.sons.push_back(val), x.header.usage ++;
	return true;
} //单纯塞进去

// bool NeedSplit()

// stack<Node> node_stack;  //WARNING!!!!
// warning: stack clear after !insert
bool Index :: Insert(Node x, BPKEY val)
{
	node_stack.push(x);
	assert(x.header.isleaf != BPTP::UNDEF);
	if (x.header.isleaf == BPTP::LEAF) // not a leaf
	{
	    // assert();

	    //
		bool flag = InsertValue (x, val);

		if (flag == false) return flag;

		if (x.NeedSplit()) return Split(x), 1; // split and update its parent
		else return x.WriteData(), 1;
	}
	else
	{
		if (x.header.usage > 1 && val.key < x.sons[1].key) // maybe don't need
		{
			// puts("小于第一个值：");
			// PrintNode(x);
			return Insert(ReadNode(x.sons[0].pt), val);
		}
		else 
		{
			// printf("VECTOR SIZE:%d\n", x.sons.size());
			vector<BPKEY> ::iterator last_it, it;
			for (it = x.sons.begin() + 1, last_it = x.sons.begin(); it != x.sons.end(); last_it = it, ++it)
			{
				BPKEY t = (*it); 
				//printf("%c | %c | %d (> t)\n", t.key.str_data[0], val.key.str_data[0], t.pt);
				if (t.key < val.key);
				else if (t.key == val.key) return false;
				else
				{
					//cout << "Look\n";
					//printf("%c | %c | %d (> t)\n", t.key.str_data[0],val.key.str_data[0],t.pt);
					// Node o = ReadNode((*last_it).pt);
					// PrintNode(o);
					// cout << "!!!!" << endl;
					return Insert(ReadNode((*last_it).pt), val);
				}
			}
			return Insert(ReadNode((*last_it).pt), val);
		}
	}
	assert(false);
	return false;
}

bool IsRoot(Node x) { return x.msg->block_id == 0; } 

Node Index::SplitLeaf(Node &x)
{
	Node y = NewNode();
	assert(y.msg != NULL);
	assert(x.header.isleaf == BPTP::LEAF);
	int num_sons = x.header.usage; // that means: usage of pointers
	y.header = x.header;
	y.header.usage = 1; // 目前这个x只是个叶子
	assert(!y.sons.empty());
	assert(y.header.isleaf != BPTP::UNDEF);
	x.header.sib = y.msg->block_id;
	for (int i = num_sons / 2; i < num_sons; ++i)
		--x.header.usage, ++y.header.usage, y.sons.push_back(x.sons[i]);
	for (int i = num_sons / 2; i < num_sons; ++i) // x是左边
		x.sons.pop_back();
	x.WriteData(); // Write the data
	y.WriteData();
	//先写一下data，大不了以后再改
//	PrintNode(x), PrintNode(y);
	return y;
}

Node Index::SplitNotLeaf(Node &x)
{
	Node y = NewNode();
	assert(y.msg != NULL);
	assert(x.header.isleaf == BPTP::NOTLEAF);
	int num_sons = x.header.usage; // that means: usage of pointers
	y.header = x.header;
    assert(y.header.isleaf != BPTP::UNDEF);
	y.header.usage = 0;
	assert(!y.sons.empty()); y.sons.pop_back();
	x.header.sib = y.msg->block_id;
	for (int i = num_sons / 2; i < num_sons; ++i)
		--x.header.usage, ++y.header.usage, y.sons.push_back(x.sons[i]);
	for (int i = num_sons / 2; i < num_sons; ++i) // x是左边
		x.sons.pop_back();
	x.WriteData(); // Write the data
	y.WriteData();
	//先写一下data，大不了以后再改
//	PrintNode(x), PrintNode(y);
	return y;
}

KEY Index::GetMinKey(Node x)
{
	assert(x.header.ktp != KTP::UNDEF);
	assert(x.header.usage != 1);
	if (x.header.isleaf == BPTP::NOTLEAF)return GetMinKey(ReadNode(x.sons[0].pt));
	return x.sons[1].key;
}

void ClearStack() { while (!node_stack.empty())node_stack.pop(); }

void Index :: Split(Node x) // WARNING: 1.data haven't been written yet 2. then Split par.. left is x right is y
{
	assert(x.header.ktp != KTP::UNDEF);
	assert(x.header.usage != 1);
	Node par;
	//puts("Split");
	// Node y = SplitLeaf(x); // x is left! y is right!
	if (IsRoot(x)) // warning : copy
	{
		Node LN = NewNode(), RN; KEY temp_key;
		LN.header = x.header; LN.sons = x.sons; RN = SplitLeaf(LN);
		x.header.isleaf = BPTP::NOTLEAF;
		x.header.sib = BP_NULL; x.header.usage = 2; // clear node and Init ROOT MSG
		x.sons.clear(); x.sons.push_back(BPKEY { temp_key, (BP_POINTER)LN.msg->block_id});
		x.sons.push_back(BPKEY{ GetMinKey(RN), (BP_POINTER)RN.msg->block_id });
		LN.WriteData(); RN.WriteData(); x.WriteData();
		ClearStack();
		return;
	}
	else // warning : copy
	{
		node_stack.pop(); // find par
		assert(!node_stack.empty()); // not root but still empty
		par = node_stack.top(); Node y = SplitLeaf(x);
		InsertValue(par, BPKEY{ GetMinKey(y),(BP_POINTER)y.msg->block_id });
	}
	if (par.NeedSplit())
	{
		swap(x, par);
		assert(x.header.isleaf == BPTP::NOTLEAF);
		do
		{
			if (IsRoot(x)) // is root
			{
			//    puts("split root");
			   // cerr<<x.header.usage << endl;
				Node LN = NewNode(), RN; KEY temp_key; // COPY x to LN except of msg
				LN.header = x.header; LN.sons = x.sons; RN = SplitNotLeaf(LN);
				x.header.isleaf = BPTP::NOTLEAF;
				x.header.sib = BP_NULL; x.header.usage = 2; // clear node and Init ROOT MSG
				x.sons.clear(); 
//				cerr << RN.header.usage <<" " <<LN.header.usage\
//				<< endl;
				x.sons.push_back(BPKEY{ temp_key, (BP_POINTER)LN.msg->block_id });
				x.sons.push_back(BPKEY{ GetMinKey(RN), (BP_POINTER)RN.msg->block_id });
//
				LN.WriteData(); RN.WriteData(); x.WriteData();
				//debug
				//puts("--------------------");
				// puts("SplitNodeRoot");
				// PrintNode(x);
				// PrintNode(LN); PrintNode(RN);
				// puts("--------------------");
				//debug
				return;
			}
			else
			{
				node_stack.pop();
				assert(!node_stack.empty());
				par = node_stack.top(); Node y = SplitNotLeaf(x);
				InsertValue(par, BPKEY{ GetMinKey(y), (BP_POINTER)y.msg->block_id });
				x.WriteData(), y.WriteData();
				// puts("-----------------");
				// puts("adding~~~~~");
				// PrintNode(par);
				// PrintNode(x), PrintNode(y);
				// puts("------------------");
				swap(x, par); // x's data has been writed down
			}
		} while (x.NeedSplit());
		x.WriteData(), ClearStack();
	}
	else par.WriteData(), ClearStack();
}

Node Index::Search(Node x, const KEY &val) // range, return lowerbound(L)
{
	if (x.header.isleaf == BPTP::LEAF) return x;
	vector<BPKEY> ::iterator last_it, it;
	for (it = x.sons.begin() + 1, last_it = x.sons.begin(); it != x.sons.end(); last_it = it, ++it)
	{
		BPKEY t = (*it);
		if (t.key < val);
		else if (t.key == val) // make sure into Rson

			return Search(ReadNode((*it).pt), val);
		else

			return Search(ReadNode((*last_it).pt), val);

	}
	return Search(ReadNode((*last_it).pt), val);
} 

bool Index::Delete(Node x, BPKEY val) // FOR only single value
{
	vector<BPKEY> ::iterator last_it, it;
	if (x.header.isleaf == BPTP::LEAF)
	{
		for (it = x.sons.begin() + 1, last_it = x.sons.begin(); it != x.sons.end(); last_it = it, ++it)
		{
			BPKEY t = (*it);
			if (t.key == val.key)
			{
				x.sons.erase(it); --x.header.usage;
				x.WriteData();
				return 1;
			}
		}
		return 0;
	}
	for (it = x.sons.begin() + 1, last_it = x.sons.begin(); it != x.sons.end(); last_it = it, ++it)
	{
		BPKEY t = (*it);
		if (t.key < val.key);
		else if (t.key == val.key) // make sure into Rson
			return Delete(ReadNode((*it).pt), val);
		else
			return Delete(ReadNode((*last_it).pt), val);

	}
	return Delete(ReadNode((*last_it).pt), val);
}

void Index::RangeDelete(KEY L, KEY R) //开区间
{
	if (L > R) return;
	Node x = Search(this->ReadNode(ROOT),L);
	//vector<Node>vec;
	while (1)
	{
		vector<BPKEY> ::iterator it;
		bool END = 0;
		for (it = x.sons.begin() + 1; it != x.sons.end(); )
		{
			BPKEY t = (*it);
			if (t.key > L && t.key < R)
				it = x.sons.erase(it), --x.header.usage;
			else if (t.key > R)
			{
				END = 1;
				break;
			}
			else ++it;
		}
		x.WriteData();
		if (END) break;
		if (x.header.sib != BP_NULL) x = ReadNode(x.header.sib);
		else break;
	}
}

void Index::SingleRangeDelete(KEY X, bool TYPE)  // 开区间！
{
	if(TYPE == 0)
	{
		Node x = GetMinLeaf(this->ReadNode(ROOT));
		while (1)
		{
			vector<BPKEY> ::iterator it;
			bool END = 0;
			for (it = x.sons.begin() + 1; it != x.sons.end(); )
			{
				BPKEY t = (*it);
				if (t.key < X)
					it = x.sons.erase(it), --x.header.usage;
				else if (t.key > X)
				{
					END = 1;
					break;
				}
				else ++it;
			}
			x.WriteData();
			if (END) break;
			if (x.header.sib != BP_NULL) x = ReadNode(x.header.sib);
			else break;
		}
	}
	else 
	{
		Node x = Search(this->ReadNode(ROOT), X);
		//vector<Node>vec;
		while (1)
		{
			vector<BPKEY> ::iterator it;
			bool END = 0;
			for (it = x.sons.begin() + 1; it != x.sons.end(); )
			{
				BPKEY t = (*it);
				if (t.key > X)
					it = x.sons.erase(it), --x.header.usage;
				else ++it;
			}
			x.WriteData();
			if (END) break;
			if (x.header.sib != BP_NULL) x = ReadNode(x.header.sib);
			else break;
		}
	}
}

Node Index::GetMinLeaf(Node x)
{
	if (x.header.isleaf == BPTP::LEAF) return x;
	return GetMinLeaf(ReadNode(x.sons[0].pt));
}

void Index ::SingleRangeSearchInc(vector<BP_POINTER> &ans, KEY X, bool TYPE) // 0: v < x 1: v > x
{
	if (TYPE == 0)
	{
		Node x = GetMinLeaf(ReadNode(ROOT));
		while (1)
		{
			vector<BPKEY> ::iterator it;
			bool END = 0;
			for (it = x.sons.begin() + 1; it != x.sons.end(); ++it)
			{
				BPKEY t = (*it);
				if (t.key <= X)
					ans.push_back(t.pt);
				else if (t.key > X)
				{
					END = 1;
					break;
				}
			}
			if (END) break;
			if (x.header.sib != BP_NULL) x = ReadNode(x.header.sib);
			else break;
		}
	}
	else 
	{
		Node x = Search(this->ReadNode(ROOT), X);
		while (1)
		{
			vector<BPKEY> ::iterator it;
			bool END = 0;
			for (it = x.sons.begin() + 1; it != x.sons.end(); ++it)
			{
				BPKEY t = (*it);
				if (t.key >= X)
					ans.push_back(t.pt);
			}
			if (END) break;
			if (x.header.sib != BP_NULL) x = ReadNode(x.header.sib);
			else break;
		}
	}
}

void Index ::SingleRangeSearch(vector<BP_POINTER> &ans, KEY X, bool TYPE) // 0: v < x 1: v > x
{
	if (TYPE == 0)
	{
		Node x = GetMinLeaf(ReadNode(ROOT));
		while (1)
		{
			vector<BPKEY> ::iterator it;
			bool END = 0;
			for (it = x.sons.begin() + 1; it != x.sons.end(); ++it)
			{
				BPKEY t = (*it);
				if (t.key < X)
					ans.push_back(t.pt);
				else if (t.key >= X)
				{
					END = 1;
					break;
				}
			}
			if (END) break;
			if (x.header.sib != BP_NULL) x = ReadNode(x.header.sib);
			else break;
		}
	}
	else 
	{
		Node x = Search(this->ReadNode(ROOT), X);
		while (1)
		{
			vector<BPKEY> ::iterator it;
			bool END = 0;
			for (it = x.sons.begin() + 1; it != x.sons.end(); ++it)
			{
				BPKEY t = (*it);
				if (t.key > X)
					ans.push_back(t.pt);
			}
			if (END) break;
			if (x.header.sib != BP_NULL) x = ReadNode(x.header.sib);
			else break;
		}
	}
}

void Index::dfs(Node x, int dep)
{
	//puts("dfs!");
	assert(x.header.ktp != KTP::UNDEF);
	assert(x.header.isleaf != BPTP::UNDEF);
	if (x.header.isleaf == BPTP::LEAF)
	{
		for (int Q = 0; Q < dep; ++Q) printf("     "); 
		cout << "Sons:" << x.header.usage << " " << x.header.len << endl;
		int cnt = 0;
		// cout << "Sons.size" << x.sons.size() << endl;
		for (int Q = 0; Q < dep; ++Q) printf("     ");
		for (auto i : x.sons)
		{
			if ((cnt++) != 0)
				cout << i.key.str_data[0] << i.key.str_data[1] << i.key.str_data[2] << i.key.str_data[3] << i.key.str_data[4] << i.key.str_data[5] << " ";
		}
		cout << endl;
		return;
	}
	else
	{
		for (int i = 0; i < dep; ++i) printf("     ");
		cerr << "This Is Not Leaf" << endl;
		int cur = 0;
		for (auto i : x.sons)
		{
			for (int Q = 0; Q < dep; ++Q) printf("     ");
			if (cur)cout << "Entry:" << i.key.str_data[0] <<i.key.str_data[1] << i.key.str_data[2] << i.key.str_data[3] << i.key.str_data[4] << i.key.str_data[5]<< " " << i.pt << " " << endl;
			else ++cur, cout << "!First LEAF" << i.pt << endl;
			dfs(ReadNode(i.pt), dep + 1);
		}
	}
}

// instance
// insert [ Primarykey, pt ]     blockid: 10 offset: 234
// insert( Primarykey, pt=> 10 * 10000 + offset);

void Index::RangeSearchInc(vector<BP_POINTER> &ans, KEY L, KEY R)
{
	if (L > R) return;
	Node x = Search(this->ReadNode(ROOT), L);
	//vector<Node>vec;
	while (1)
	{
		vector<BPKEY> ::iterator it;
		bool END = 0;
		for (it = x.sons.begin() + 1; it != x.sons.end(); ++it)
		{
			BPKEY t = (*it);
			if (t.key >= L && t.key <= R)
				ans.push_back(t.pt);
			else if (t.key > R)
			{
				END = 1;
				break;
			}
		}
		if (END) break;
		if (x.header.sib != BP_NULL) x = ReadNode(x.header.sib);
		else break;
	}
}

void Index::RangeSearch(vector<BP_POINTER> &ans, KEY L, KEY R)
{
	if (L > R) return;
	Node x = Search(this->ReadNode(ROOT), L);
	//vector<Node>vec;
	while (1)
	{
		vector<BPKEY> ::iterator it;
		bool END = 0;
		for (it = x.sons.begin() + 1; it != x.sons.end(); ++ it)
		{
			BPKEY t = (*it);
			if (t.key > L && t.key < R)
				ans.push_back(t.pt);
			else if (t.key > R)
			{
				END = 1;
				break;
			}
		}
		if (END) break;
		if (x.header.sib != BP_NULL) x = ReadNode(x.header.sib);
		else break;
	}
}

/*
#include "time.h"
void IndexTest()
{
	string table, col, index;
	cout << "Please Input tablename column name index name" << endl;
	// cin >> table >> col >> index;
	srand(time(0));
	int tt = rand(); table = "";
	for (int i = 0; i < 30; ++i) if (tt & 1)table += '0', tt >>= 1; else table += '1', tt >>= 1;
	col = "col", index = table + ".index";
	Node t;
	// string test = "IndexTest.index";
	Index *QAQ;
	QAQ = Create_Index((string)table, (string)col, (string)index, KTP::STRING, 6); // create finished // 5B
	// printf("%d %d %d\n", q.header.len, q.header.ktp, q.header.isleaf, q.header.par);
	// Node q(QAQ->ReadNode(ROOT));
	// printf("%d %d %d\n", q.header.len, q.header.ktp, q.header.isleaf);
	// q.WriteData();
	// q.msg->Block_Flush(); // yes
	int tp; while (cin >> tp, tp != -1)
	{
		string s; //一定是长度为5的
		BPKEY x; x.key.ktype = KTP::STRING;
		cout << "Input data and address" << endl;
		cin >> x.key.str_data; x.key.len = 6; cin >> x.pt;
		QAQ->Insert(QAQ->ReadNode(ROOT), x);
		// if (!node_stack.empty())puts("not empty");
		while (!node_stack.empty())node_stack.pop();
	}
	QAQ->dfs(QAQ->ReadNode(ROOT), 0);
	puts("Input: SearchString");
	string T;  
	while (0)
	{
		cin >> T;
		KEY val; memcpy(val.str_data, T.c_str(), T.size());
		val.ktype = KTP::STRING;
		Node Q = QAQ->Search(QAQ->ReadNode(ROOT), val);
		for (auto i : Q.sons)
		{
			for (int j = 0; j < 6; ++j)
				printf("%c", i.key.str_data[j]);
			puts("");
		}
	}
	KEY L; L.ktype = KTP::STRING, L.len = 6;
	memcpy(L.str_data, "abcdeg", 6);
	KEY R; R.ktype = KTP::STRING, R.len = 6;
	memcpy(R.str_data, "engksj", 6);
	vector<KEY>p;  QAQ->RangeSearch(p, L, R);
	for (auto i : p)
	{
		for (int j = 0; j < 6; ++j)
			printf("%c", i.str_data[j]);
		puts("");
	}
	QAQ->RangeDelete(L, R);
	QAQ->dfs(QAQ->ReadNode(ROOT), 0);
	bm.FlushAllBlocks(); // no
	//QAQ->dfs(QAQ->ReadNode(ROOT), 0);

	while (1);
	//QAQ

}
*/
/*
//在程序运行第一条相关<TABLENAME>指令的时候看一下map< TABLENAME, class* > 存在否？ 不存在就加载 存在的话就更改上面的值 在main函数结束时，遍历map把INT写出
catalog: tablename.catalog // metadata
select(tablename, .... )
cin >>
1. TABLE name ; int FILE_BLOCK_CNT;
2. TABLE attribute_cnt; // 属性的数目有几个, 是否为primary key or unique
3. attributes[attribute_cnt] : len type  // 每个属性的名字, 长度, 类型
4. USAGE(一个int) 最后一块有几个记录
// 最后一块是哪块
create new index [已经有数据了]
	for(i <= catalog :: BLOCK_CNT_IN_FILE)
		x = READBLOCKS(i)
		catalog :: KEY_NUMBERS
		for(j <=  KEY_NUMBERS)
			catalog :: KEY_LEN[j];
			[record: stu_id, name, grade]
			[name 建立]
			split(KEY_j); // 下层实现不了
			KEY insert_val;
			READKEY(insert_val, );
			offset += KEY_LEN[j];
			insert_index(filename, colname, insert_val);
*/