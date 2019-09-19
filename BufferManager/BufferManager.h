//
// Coded by Xinning Zhang.
//
// 记得set_dirty

#ifndef MINISQL_BUFFERMANAGER_H
#define MINISQL_BUFFERMANAGER_H
#include "../minisql.h"
/*
步骤：
1. ReadBlocks -> BLOCK x
2. vector<Key> get record from x
3. Modify records => 基于内存 自己随便改 但是不写回就不会更新 : vec[i].Key = Key(xxx); 这个是不会写回的
4. Write Blocks -> x
// 注意在exit函数最后 加一句 FlushAllBlocks应用所有更改
*/



class BufferManager {
public:
	
	BufferManager() { blocks = new BLOCKS[BLOCK_CNT]; }
	~BufferManager() { }//delete[] blocks; }
	BLOCKS& ReadBlocks(string &filename, int block_id); //读取文件中的一块到内存中
	BLOCKS& GetFreeBufferPrefer();
	void FlushAllBlocks(); //读取文件中的一块到内存中并返回
	BLOCKS& GET_LRU(int p);
	void DeleteFile(string &filename);
private:
	BLOCKS *blocks; //存储整个buffer pool
};

void buffer_for_test();

#endif //MINISQL_BUFFERMANAGER_H
