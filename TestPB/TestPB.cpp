// TestPB.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include <Windows.h>
#include <iostream>
#include "Protobuf.h"
#include "test.pb.h"
#ifdef _DEBUG
#pragma comment(lib,"libprotocd.lib")
#pragma comment(lib,"libprotobufd.lib")
#else

#pragma comment(lib,"libprotoc.lib")
#pragma comment(lib,"libprotobuf.lib")
#endif // _DEBUG

std::string HexToStr(const char* hexdata, unsigned long hexdatalen)
{
    const char* TT = "0123456789ABCDEF";
    std::string result;
    for (unsigned long i = 0; i < hexdatalen; i++)
    {
        result.push_back(TT[(hexdata[i] >> 4) & 0xF]);
        result.push_back(TT[(hexdata[i] >> 0) & 0xF]);
    }
    return result;
}

std::string Ansi_To_UTF8(const std::string& str)
{
    int nwLen = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), -1, NULL, 0);

    wchar_t* pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
    ZeroMemory(pwBuf, nwLen * 2 + 2);

    ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), str.length(), pwBuf, nwLen);

    int nLen = ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

    char* pBuf = new char[nLen + 1];
    ZeroMemory(pBuf, nLen + 1);

    ::WideCharToMultiByte(CP_UTF8, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

    std::string retStr(pBuf);

    delete[]pwBuf;
    delete[]pBuf;

    pwBuf = NULL;
    pBuf = NULL;

    return retStr;
}
std::string UTF8_To_Ansi(const std::string& str)
{
    int nwLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), -1, NULL, 0);

    wchar_t* pwBuf = new wchar_t[nwLen + 1];//一定要加1，不然会出现尾巴
    memset(pwBuf, 0, nwLen * 2 + 2);

    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), str.length(), pwBuf, nwLen);

    int nLen = WideCharToMultiByte(CP_ACP, 0, pwBuf, -1, NULL, NULL, NULL, NULL);

    char* pBuf = new char[nLen + 1];
    memset(pBuf, 0, nLen + 1);

    WideCharToMultiByte(CP_ACP, 0, pwBuf, nwLen, pBuf, nLen, NULL, NULL);

    std::string retStr = pBuf;

    delete[]pBuf;
    delete[]pwBuf;

    pBuf = NULL;
    pwBuf = NULL;

    return retStr;
}
int main()
{
    Protocol::UserGroup users;
    Protocol::User* user = users.add_userlist();
    const char bb[] = { 0x8A,0x8B,0x8C };
    //user->set_name(std::string(bb,sizeof(bb)));
    user->set_name("aaaa");
    user->add_age(111);
    user->add_age(112);
    user->add_age(113);
    user->add_age(114);
    user = users.add_userlist();
    user->set_name("bbbb");
    user->add_age(222);
    user->add_age(223);
    user = users.add_userlist();
    user->set_name("cccc");
    user->add_age(333);
    std::string data = users.SerializeAsString();
    std::cout << "系统生成:" << HexToStr(data.c_str(), data.length()) << std::endl;


    pb::Protobuf pb(data);
    std::string view_str = pb.ToView(false);
    std::cout << "------view begin--------" << std::endl;
    std::cout << view_str;
    std::cout << std::endl;
    std::cout << "------view end--------" << std::endl;

    view_str = pb.ToView(true);
    std::cout << "------view begin--------" << std::endl;
    std::cout << view_str;
    std::cout << std::endl;
    std::cout << "------view end--------" << std::endl;


    std::string name_1$2_1;
    uint64_t age_1$2_2$1;
    float age3_1$2_2$1;
    bool ret_bool;
    ret_bool = pb.GetValueByPath("1[2].1", name_1$2_1);
    //下标解析
    std::string name_1$2_2 = pb["O:1"]["A:2"]["O:1B"].Binary();
    //路径解析
    ret_bool = pb.GetValueByPath("1[0].2[1]", age_1$2_2$1);
    ret_bool = pb.GetValueByPath("1[0].4[1]", age3_1$2_2$1);
    std::cout << "序列化后:" << HexToStr(pb.Binary().c_str(), pb.Binary().length()) << std::endl;
    //设置值
    ret_bool = pb.SetValueByPath("1[2].1", "CCCC");

    std::cout << "修改值后:" << HexToStr(pb.Binary().c_str(), pb.Binary().length()) << std::endl;

    pb::Protobuf my_pb(pb::PB_TYPE::PB_TYPE_OBJECT);
    auto root = my_pb.Object().insert(std::make_pair(1, pb::PB_TYPE::PB_TYPE_ARRAY));

    root.first->second.Array().Items().push_back(pb::PB_TYPE::PB_TYPE_OBJECT);
    root.first->second.Array().Items()[0].Object().insert(std::make_pair(1,"aaaa"));
    auto user1_age = root.first->second.Array().Items()[0].Object().insert(std::make_pair(2, pb::PB_TYPE::PB_TYPE_ARRAY));
    user1_age.first->second.Array().Packed() = true;
    user1_age.first->second.Array().Items().push_back((uint64_t)111);
    user1_age.first->second.Array().Items().push_back((uint64_t)112);
    user1_age.first->second.Array().Items().push_back((uint64_t)113);
    user1_age.first->second.Array().Items().push_back((uint64_t)114);

    root.first->second.Array().Items().push_back(pb::PB_TYPE::PB_TYPE_OBJECT);
    root.first->second.Array().Items()[1].Object().insert(std::make_pair(1, "bbbb"));
    auto user2_age = root.first->second.Array().Items()[1].Object().insert(std::make_pair(2, pb::PB_TYPE::PB_TYPE_ARRAY));
    user2_age.first->second.Array().Packed() = true;
    user2_age.first->second.Array().Items().push_back((uint64_t)222);
    user2_age.first->second.Array().Items().push_back((uint64_t)223);

    root.first->second.Array().Items().push_back(pb::PB_TYPE::PB_TYPE_OBJECT);
    root.first->second.Array().Items()[2].Object().insert(std::make_pair(1, "cccc"));
    auto user3_age = root.first->second.Array().Items()[2].Object().insert(std::make_pair(2, pb::PB_TYPE::PB_TYPE_ARRAY));
    user3_age.first->second.Array().Packed() = true;
    user3_age.first->second.Array().Items().push_back((uint64_t)333);
    std::cout << "自己生成:" << HexToStr(my_pb.Binary().c_str(), my_pb.Binary().length()) << std::endl;

    if (users.ParseFromString(my_pb.Binary()))
    {
        ((Protocol::User*)(&users.userlist().Get(2)))->set_name("CCCC");
        data = users.SerializeAsString();
        std::cout << "修改值后:" << HexToStr(data.c_str(), data.length()) << std::endl;
    }

    const char text_str[] = { 0x0a,0x0c,0x7a,0x68,0x75,0x7a,0x68,0x75,0xe4,0xb8,0xad,0x31,0x32,0x33,0x0a,0x03,0x32,0x32,0x32,0x12,0x0c,0x01,0x00,0x00,0x00,0x02,0x00,0x00,0x00,0x03,0x00,0x00,0x00 };
    
    pb::Protobuf pb_text(std::string(text_str, sizeof(text_str)));
    //std::string test_name = UTF8_To_Ansi(pb_text["M:1"].Binary());
    uint64_t test_ii;
    test_ii = pb_text["O:2"]["A:2F"].Fixed32().AsInt32();
}