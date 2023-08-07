#pragma once
#include <string>
#include <vector>
#include <map>

namespace pb
{
	enum class PB_TYPE
	{
		PB_TYPE_VARINT,
		PB_TYPE_FIXED64,
		PB_TYPE_FIXED32,
		PB_TYPE_BINARY,
		PB_TYPE_ARRAY,
		PB_TYPE_OBJECT
	};

	class Uint64_t
	{
	public:
		inline Uint64_t()
			:m_Value{ 0 }
		{}
		int SetValue(const char* buffer, int buffer_len);
		std::string ToString();
		inline uint64_t& AsUint64()
		{
			return *(uint64_t*)m_Value;
		}
		inline int64_t& AsInt64()
		{
			return *(int64_t*)m_Value;
		}
	private:
		char m_Value[8];
	};
	class Fixed64_t {
	public:
		inline Fixed64_t()
			:m_Value{ 0 }
		{}
		inline int SetValue(const char* buffer, int buffer_len)
		{
			if (buffer_len < sizeof(m_Value))
				return -1;
			memcpy(m_Value, buffer, sizeof(m_Value));
			return sizeof(m_Value);
		}
		inline std::string ToString()
		{
			return std::string(m_Value, sizeof(m_Value));
		}
		inline uint64_t& AsUint64()
		{
			return *(uint64_t*)m_Value;
		}
		inline int64_t& AsInt64()
		{
			return *(int64_t*)m_Value;
		}
		inline double& AsDouble()
		{
			return *(double*)m_Value;
		}
	private:
		char m_Value[8];
	};

	class Fixed32_t {
	public:
		inline Fixed32_t()
			:m_Value{ 0 }
		{}
		inline int SetValue(const char* buffer, int buffer_len)
		{
			if (buffer_len < sizeof(m_Value))
				return -1;
			memcpy(m_Value, buffer, sizeof(m_Value));
			return sizeof(m_Value);
		}
		inline std::string ToString()
		{
			return std::string(m_Value, sizeof(m_Value));
		}
		inline uint32_t& AsUint32()
		{
			return *(uint32_t*)m_Value;
		}
		inline int32_t& AsInt32()
		{
			return *(int32_t*)m_Value;
		}
		inline float& AsFloat()
		{
			return *(float*)m_Value;
		}
	private:
		char m_Value[4];
	};

	typedef std::string Binary_t;

	class Array_t {
	public:
		inline Array_t()
			:m_Packed(false), m_Items()
		{}
		inline bool& Packed()
		{
			return m_Packed;
		}
		inline std::vector<class Protobuf>& Items()
		{
			return m_Items;
		}
	private:
		bool m_Packed;
		std::vector<class Protobuf> m_Items;
	};

	typedef std::map<uint64_t, class Protobuf> Object_t;

	class Protobuf
	{
	public:
		Protobuf(PB_TYPE type);
		Protobuf(const Protobuf& _that);
		Protobuf& operator=(const Protobuf& _that);
		bool operator==(const PB_TYPE& type);
		Protobuf& operator+=(const Protobuf& _that);
		//index_src��ʽ:(O|A):(number){I|D|F|A|O|B}  ():����,{}��ѡ
		//I:Uint64��D:Fixed64��F:Fixed32��A:Array��O:Object��B:Bytes
		//(O|A):��ʾ����ǰProtobuf��ʽ����ʲô��ʽ
		//(number):��ʾȡObject��tag=number��Array�е�index=number
		//{I|D|F|A|O|B}:��ʾȡ���������Ӧ����ʲô��ʽ
		Protobuf& operator[] (std::string index_src);
		Protobuf(uint64_t value);
		Protobuf(double value);
		Protobuf(float value);
		Protobuf(std::string value);
		//��ȡ����
		PB_TYPE GetType();
		//���ʸ�������
		Uint64_t& Uint64();
		Fixed64_t& Fixed64();
		Fixed32_t& Fixed32();
		Binary_t& Binary();
		Array_t& Array();
		Object_t& Object();
		//ת��Ϊ�ַ�����ͼ
		std::string ToView(bool brevity = true);
		//��ʽnumber[index].number[index].number[index]......	number:����,[index]��ѡ
		//���·��ʹ��.�ָ�
		//�����ǰ�㼶������,ʹ��[]ָ��index
		Protobuf& ValueByPath(const std::string path, PB_TYPE des_type);
		bool GetValueByPath(const std::string path, uint64_t& value);
		bool GetValueByPath(const std::string path, double& value);
		bool GetValueByPath(const std::string path, float& value);
		bool GetValueByPath(const std::string path, std::string& value);
		bool SetValueByPath(const std::string path, uint64_t value);
		bool SetValueByPath(const std::string path, double value);
		bool SetValueByPath(const std::string path, float value);
		bool SetValueByPath(const std::string path, std::string value);
	private:
		void ToBinary();
		bool ToObject();
		bool ToArray(PB_TYPE item_type);
		std::string ToBinaryAsTag(uint64_t tag);
		std::string ToViewAsTitle(std::string title, bool brevity);
		PB_TYPE m_Type;
		struct
		{
			Uint64_t Uint64;
			Fixed64_t Fixed64;
			Fixed32_t Fixed32;
			Binary_t Binary;
			Array_t Array;
			Object_t Object;
		}m_Value;
	};
}