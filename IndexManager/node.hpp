#ifndef NODE_H
#define NODE_H
#include <vector>
#include "../minisql.h"
#include "../BufferManager/BufferManager.h"
enum class BPTP
{
    UNDEF = 0,
    LEAF,
    NOTLEAF
};
//叶子： k个指针 k个值
//非叶子： k个指针 k-1个值[0...k-1] [ < val[k] 向左 否则向右 ]
// usage: records which are inserted
typedef int BP_POINTER;
const BP_POINTER BP_NULL = -1;
const BP_POINTER ROOT = 0;
struct BPKEY
{
    KEY key;
    BP_POINTER pt; //指向文件的位置
};
// [header | record1 | 2 | 3 | ..] [header | 1 | 2 | 3 | ..]
struct BPHEADER
{
    BPTP isleaf;
    KTP ktp; int len, usage;
    BP_POINTER sib;
    BPHEADER()
    {
        isleaf = BPTP::UNDEF;
        ktp = KTP::UNDEF;
        len = 0; usage = 1; //只有一个空指针QAQ
        sib = BP_NULL;
    }
};
struct Node //源于buffer 高于buffer！
{
    /*BPTP isleaf; KTP ktp;
    int len, usage;
    BP_POINTER par, sib;*/
    BPHEADER header; // 从header的数据进行访问
    vector<BPKEY> sons;
    BLOCKS *msg; //指向一个data……之后可能要搞事情T^T应该是data->... 一定是要绑定的QAQAQ
    Node()
    {
        header = BPHEADER();
        sons.clear();
        msg = NULL;
    }
    Node(const Node& a)
    {
        header = a.header;
        for (auto i : a.sons) sons.push_back(i);
        msg = a.msg;
    }
    ~Node() { ; }
    void ReadHeader();
    void WriteHeader();
    void ReadData(); // 从msg里面要数据.jpg
    void WriteData(); // write数据到msg中
    bool NeedSplit(){
        //cerr << header.len << endl;
        // return header.usage > ;
        bool t = (sizeof(header) + (sizeof(int) + header.len) * (header.usage+2)) >= BLOCK_SIZE;
        // cerr << sizeof(header) << endl;
        // if (t == 1) puts("NeedSplit!");
        return t;
    }
    bool NeedMerge() { return sizeof(header) + 2 * (4 + header.len) * (header.usage + 1) < BLOCK_SIZE; }
    //里面有一些函数，还没想好
};
// Read_Node from BLOCKS.
// WRITE_Node to BLOCKS.
#endif

/*
B+ tree的部分之后再想
create B+tree: 
drop B+tree:
Insert val:
delete val:
find val < one / more >
*/