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
		//index_src格式:(O|A):(number){I|D|F|A|O|B}  ():必须,{}可选
		//I:Uint64，D:Fixed64，F:Fixed32，A:Array，O:Object，B:Bytes
		//(O|A):表示将当前Protobuf格式化成什么格式
		//(number):表示取Object中tag=number或Array中的index=number
		//{I|D|F|A|O|B}:表示取出后的数据应该是什么格式
		Protobuf& operator[] (std::string index_src);
		Protobuf(uint64_t value);
		Protobuf(double value);
		Protobuf(float value);
		Protobuf(std::string value);
		//获取类型
		PB_TYPE GetType();
		//访问各种数据
		Uint64_t& Uint64();
		Fixed64_t& Fixed64();
		Fixed32_t& Fixed32();
		Binary_t& Binary();
		Array_t& Array();
		Object_t& Object();
		//转换为字符串视图
		std::string ToView(bool brevity = true);
		//格式number[index].number[index].number[index]......	number:必须,[index]可选
		//多层路径使用.分割
		//如果当前层级是数组,使用[]指定index
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