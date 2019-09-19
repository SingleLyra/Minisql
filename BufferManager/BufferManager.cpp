//
// Created by Robert Chen on 2019-04-30.

#include "BufferManager.h"

map<pair<string, unsigned>, BLOCKS*> check_buffer; // <file, file_blockid, pos>
typedef map<pair<string, unsigned>, BLOCKS*> ::iterator BLOCKS_IT;

BufferManager bm;
BLOCKS& BufferManager::ReadBlocks(string &filename, int block_id)
//BLOCKS &x = ReadBlocks(filename, block_id);
// for(i <= SIZE / k) x.ReadString(str[i],k,offset = k * i);

// catalog -> readblocks() -> &blocks -> read/write record
// 从某个文件里获取第i个块
{
	BLOCKS_IT it = check_buffer.find(make_pair(filename, block_id));
	if (it != check_buffer.end())
	{
		it->second->SetBusy(true);
		// cerr << &((it->second)->data) << endl;
		// cout << (it->second)->data << endl;
		return *(it->second);
	}
//	cerr << "buffer!" << endl;
	BLOCKS &cur_block = GetFreeBufferPrefer(); // new block for data
	cur_block.Init(filename, block_id); // Init
	fstream fin;
	fin.open(filename, ios::in | ios::binary);
	if (!fin.good()) // 
	{
		fin.close();
		cerr << "file open error! The File doesn't exist! We'll create one!" << endl;
		fstream fout;
		fout.open(filename, ios::out | ios::binary);
		fout.close();
		fin.open(filename, ios::in | ios::binary);
	}
	
	fin.seekg(ios::beg + block_id * BLOCK_SIZE);
	if (fin.peek() == EOF)
	{
//		cerr << "FIND EOF! We'll initialize data." << endl;
		for (int i = 0; i < BLOCK_SIZE; ++i)
			cur_block.data[i] = 0;
	}
	else 
	{
		fin.read((char*)cur_block.data, BLOCK_SIZE); // read_block
		fin.close();
	}
	check_buffer[make_pair(filename, block_id)] = &cur_block;

	return cur_block;
}

BLOCKS& BufferManager::GET_LRU(int p)
{
	BLOCKS &B = blocks[p];
	//cerr << "GetLRU" << p << endl;
	if (check_buffer.count(make_pair(B.file_name, B.block_id)) > 0)
		check_buffer.erase(make_pair(B.file_name, B.block_id));
	if(B.block_id >= 0) B.Block_Flush();
	return B;
}

BLOCKS& BufferManager :: GetFreeBufferPrefer()  //LRU 改进版
{
	static int p = 0;
	while (1)
	{
		int LOOP_CNT = BLOCK_CNT;
		while (LOOP_CNT--)
		{
			if (blocks[p].Isbusy() == false && blocks[p].Isdirty() == false)
				return GET_LRU(p);
			else if (blocks[p].Ispin());
			else blocks[p].SetBusy(false);
			p = (p + 1) % BLOCK_CNT;
		}
		LOOP_CNT = BLOCK_CNT;
		while (LOOP_CNT--)
		{
			if (blocks[p].Isbusy() == false && blocks[p].Isdirty() == true)
				return GET_LRU(p);
			else if (blocks[p].Ispin());
			else blocks[p].SetBusy(false);
			p = (p + 1) % BLOCK_CNT;
		}
	}
	cerr << "LRU FAILED" << endl;
	return GET_LRU(p);
}
void BufferManager::DeleteFile(string &filename)
{
	for (int i = 0; i < BLOCK_CNT; ++i)
	{
		if (blocks[i].file_name == filename)
		{
			// cerr << "Flush " << i << endl;
			blocks[i].Block_Flush();
			blocks[i] = BLOCKS();
		}
	}
}
void BufferManager::FlushAllBlocks() // 程序结束时，需要清空内存池
{
	for (int i = BLOCK_CNT - 1;~ i ; --i)
	{
		// cerr << "Flush " << i << endl;
		blocks[i].Block_Flush();
	}
}


/*#include <string>

void buffer_for_test()
{
	BufferManager bm;
	using namespace std;
	string filename = "buffertest";
	while (1)
	{
		string u; int p;
		cin >> p;
		if (p < 0) { bm.FlushAllBlocks(); break; }
		
		BLOCKS& st = bm.ReadBlocks(filename, p);
		string s; int U = -1;
		// st.Block_Flush();
		// st.Block_Flush();
		cerr << &st.data << endl;
		
		cout << "Before Read: " << st.data << endl;
		st.Read_Int(U, sizeof(U), 233);
		cout << U << endl;
		st.Read_String(s, BLOCK_SIZE / 2, 0);
		cout << s << endl;
		cout << "! SIZE: " << s.size() << endl;
		cout << "After Read: " << st.data << endl;
		st.Write_String("DEBUG", 0);
		st.Read_String(s, 10, 0);
		cout << s << endl;
		cout << st.data << endl;
		st.debug();
		// st.Block_Flush();
	}
	puts("Test_Ended");
}*/

// 1.希望能有个函数去找到 metadata 读入格式与它一样
// 2.我会在index中实现一个 read metadata.