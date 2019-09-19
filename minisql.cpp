#include "minisql.h"
#include <cmath>
// function for KEYS
int KEY :: cmp(const KEY &a) const
{
	assert(this->ktype == a.ktype);
	assert(this->ktype != KTP::UNDEF);
	switch (this->ktype)
	{
	case KTP::INT:
		return this->int_data - a.int_data;
		break;
	case KTP::FLOAT:
		if (fabs(this->float_data - a.float_data) < 1e-6) return 0;
		else if (this->float_data - a.float_data < 0) return -1;
		else return 1;
		break;
	case KTP::STRING:
		return strcmp(this->str_data,a.str_data);
		break;
	default:
		assert(false);
		break;
	}
}

bool KEY ::operator<(const KEY &a)const  { return this->cmp(a) < 0; }
bool KEY ::operator>(const KEY &a)const  { return this->cmp(a) > 0; }
bool KEY ::operator==(const KEY &a)const { return this->cmp(a) == 0; }
bool KEY ::operator<=(const KEY &a)const { return this->cmp(a) <= 0; }
bool KEY ::operator>=(const KEY &a) const { return this->cmp(a) >= 0; }


// functions for BLOCKS
bool BLOCKS :: Isdirty() { return this->dirty_tag; }
bool BLOCKS :: Ispin() { return this->pin_tag; }
bool BLOCKS :: Isbusy() { return this->busy_tag; }
void BLOCKS :: SetPin(bool st) { this->pin_tag = st; }
void BLOCKS :: SetBusy(bool st) { this->busy_tag = st; }
void BLOCKS :: SetDirty(bool st) { this->dirty_tag = st; }
void BLOCKS :: Init(const string& file_name, const int& block_id) //��ÿ��readblocks��ʱ�򶼻����óɳ�ʼ�Ŀ�
{
	for (int i = 0; i <= BLOCK_SIZE; ++i) data[i] = 0;
	this->file_name = file_name, this->block_id = block_id;
	pin_tag = dirty_tag = false; busy_tag = true;
}
#include "string"
void BLOCKS::Block_Flush() // Flush And Erase That Block Done
{
	if (false == this->Isdirty()) 
		return;
	string filename = this->file_name;
	//fstream fout;
	FILE *fp;                               //文件指针
	if ((fp = fopen(filename.c_str(), "r+b")) == NULL) {
		cout << "Open file error!" << endl; // #Todo
		return;
	}
	fseek(fp, this->block_id * BLOCK_SIZE, SEEK_SET);  // 将文件指针指向偏移量计算之后
	fwrite(data, BLOCK_SIZE, 1, fp);    // 将buffer 中的 values 写回到文件当中去
	fclose(fp);
}

void BLOCKS::Read_String(string &s, const int &length, const int& offset)
{
	char *c = new char[length + 1];
	memcpy(c, (const char*)(this->data + offset), length);
	c[length] = '\0';  s = c; delete[] c;
}
void BLOCKS::Read_String(char *c, const int &length, const int& offset)
{
	memcpy(c, (const char*)(this->data + offset), length);
}

void BLOCKS::Read_Float(float &s, const int &length, const int& offset) // length: sizeof(double)
{
	memcpy((char*)&s, (const char*)(this->data + offset), length);
}

void BLOCKS::Read_Int(int &s, const int &length, const int& offset) // length: sizeof(int)
{
	memcpy((char*)&s, (const char*)(this->data + offset), length);
}


void BLOCKS::Write_String(const string &s, const int& offset) 
{
	this->SetDirty(true);
	const char* c = s.c_str();
	memcpy(this->data + offset, c, s.size());
	// memset(this->data + offset + s.size(), 0, length - s.size()); // ���ڴ����ַ�����ĩβ0
}
void BLOCKS::Write_String(const char* c, const int& offset,const int& len)
{
	this->SetDirty(true);
	// const char* c = s.c_str();
	memcpy(this->data + offset, c, len);
	// memset(this->data + offset + s.size(), 0, length - s.size()); // ���ڴ����ַ�����ĩβ0
}
void BLOCKS::Write_Int(const int &s, const int& offset) // ����һ��char*ָ�� �� ���͵ĳ���
{
	this->SetDirty(true);
	memcpy(this->data + offset, (const char *)&s, sizeof(s));
	// memset(this->data + offset + strlen(c), 0, length - strlen(c)); // ���ڴ����ַ�����ĩβ0 �ƺ�ûʲô��Ҫ
}
void BLOCKS::Write_Float(const float &s, const int& offset) // ����һ��char*ָ�� �� ���͵ĳ���
{
	this->SetDirty(true);
	memcpy(this->data + offset, (const char *)&s, sizeof(s));
	// memset(this->data + offset + strlen(c), 0, length - strlen(c)); // ���ڴ����ַ�����ĩβ0 �ƺ�ûʲô��Ҫ
}

bool File_Exist(const string &filename)
{
	fstream fin;
	fin.open(filename, ios::in | ios::binary);
	if (!fin.good()) return fin.close(), 0;
	return fin.close(), 1;
}