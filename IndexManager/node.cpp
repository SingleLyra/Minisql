#include "node.hpp"

void Node :: ReadHeader() { memcpy((char *)&this->header, this->msg->data, sizeof(this->header)); } // offset == 0
void Node :: WriteHeader() { msg->SetDirty(true);  memcpy(this->msg->data, (const char *)&this->header, sizeof(this->header)); } // offset == 0

void Node :: ReadData() // ��msg����Ҫ����.jpg ����Ҫsetdirty
{
    assert(msg != NULL);
    ReadHeader(); // read header : then you got the offset:
    int offset = sizeof(header);
    for (int i = 0; i < header.usage; ++i) // <key, pointer>��
    {
        BPKEY T;
        KEY &val = T.key; val.ktype = header.ktp, val.len = header.len;
        BP_POINTER &p = T.pt;
        val.Read(*msg, offset);
        offset += val.len; // add
        msg->Read_Int((int&)p, sizeof(BP_POINTER), offset);
        offset += sizeof(BP_POINTER);
        sons.push_back(T); // add
    }
}
void Node::WriteData() // write���ݵ�msg��
{
    assert(msg != NULL);
    msg->SetDirty(true);
    WriteHeader(); // read header : then you got the offset:
    int offset = sizeof(header);
    for(auto T : sons)
    {
        KEY val = T.key; val.ktype = header.ktp, val.len = header.len;
        BP_POINTER &p = T.pt;
        val.Write(*msg, offset);
        offset += val.len; // add
        msg->Write_Int((int)p, offset);
        offset += sizeof(BP_POINTER);
    }
}