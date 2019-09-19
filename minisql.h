#ifndef MINISQL_H
#define MINISQL_H
#include <iostream>
#include <cassert>
#include <fstream>
#include <map>
#include <cmath>
//为了防止它继续bug……我的debug信息暂时不删……
using namespace std;
#define BLOCK_SIZE 4096
#define BLOCK_CNT 1000
typedef char BYTE;

enum class KTP
{
	UNDEF = 0,
	INT,
	FLOAT,
	STRING
};
// BLOCKS x; x.Read_String(str, len, offset);
class BLOCKS
{
public:
	BLOCKS() : data(NULL), pin_tag(false), dirty_tag(false), busy_tag(false), file_name(""), block_id(-1)
	{
		data = new BYTE[BLOCK_SIZE + 1];
		for (int i = 0; i <= BLOCK_SIZE; ++ i) data[i] = 0;
	};
	~BLOCKS() { }//delete[] data; }
	bool Isdirty(); 
	bool Ispin();
	bool Isbusy();
	void SetPin(bool st);
	void SetBusy(bool st);
	void Init(const string& file_name, const int& block_id);
	void SetDirty(bool st);
	void Block_Flush(); // 输出整块到文件里
	void Read_String(string &s, const int &length, const int& offset); // 用法:从当前块的offset位置读入长度为l的data，并记录到string里.
	void Read_String(char *c, const int &length, const int& offset); // 用法:从当前块的offset位置读入长度为l的data，并记录到string里.
	void Read_Float(float &s, const int &length, const int& offset);
	void Read_Int(int &s, const int &length, const int& offset);
	
	void Write_String(const string &s, const int &offset); // 将当前string读进来
	void Write_String(const char* c, const int& offset, const int &len); //写出一个长度为len
	void Write_Int(const int &s, const int &offset);
	void Write_Float(const float &s, const int &offset);
	// 用法:从当前块的offset位置写入一个string，长度为len(比如"aaa"长度为4),注意要保证offset+len < BLOCK_SIZE

	string file_name;
	unsigned block_id;
	BYTE *data;
	void debug()
	{
		cerr << "NOW: debuging" << endl;
		cerr << data << endl << "pin_tag" << pin_tag << " " << "dirty:" << dirty_tag << " " << " Busy: " << busy_tag << endl;
	}
private:
	bool pin_tag, dirty_tag, busy_tag;
};

struct KEY // KeyType Finished 但是需要传入块的 id 和 offset 用之前set一下len
/*
	vector<KEY>vec;
	catalog: TTPE_INT TYPESTRING(5) FLOAT TYPESTRING(250) 
	4 columns => 4 KEY => Read(KEY1, offset); offset += KEY1.len; READ(KEY2, offset); 
*/
{
	KTP ktype;
	int int_data, len; // datalen
	float float_data;
	char str_data[256];
	char data[256];
	KEY()
	{
		ktype = KTP::UNDEF;
	}
	void Read(BLOCKS &x, int offset)
	{
		assert(ktype != KTP::UNDEF);
//		assert(offset + len < 4096);
		if (ktype == KTP::INT) x.Read_Int(int_data, len, offset);
		else if (ktype == KTP::FLOAT) x.Read_Float(float_data, len, offset);
		else x.Read_String(str_data, len, offset);
	}
	void Write(BLOCKS &x, int offset) // + offset
	{
		assert(ktype != KTP::UNDEF);
		if (ktype == KTP::INT) x.Write_Int(int_data, offset);
		else if (ktype == KTP::FLOAT) x.Write_Float(float_data, offset);
		else x.Write_String(str_data, offset, len);
	}
	bool operator<(const KEY &a)const;
	bool operator>(const KEY &a)const;
	bool operator==(const KEY &a)const;
	bool operator<=(const KEY &a)const;
	bool operator>=(const KEY &a) const;
	int cmp(const KEY &a) const;
};
bool File_Exist(const string &filename);

#endif