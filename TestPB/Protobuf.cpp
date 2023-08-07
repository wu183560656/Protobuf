#include "Protobuf.h"
#include <regex>
namespace pb
{
	static int pb_read_varint(const char* pbdata, int datalen, uint64_t& value)
	{
		uint64_t tmpValue = 0;
		for (int index = 0; index <= 9 && index < datalen;)
		{
			uint64_t temp = pbdata[index++];
			if (index == 9 && temp & 0b00000001)
			{
				//第十个字节只有一位
				break;
			}
			tmpValue |= (temp & 0b01111111) << ((index - 1) * 7);
			if ((temp & 0b10000000) == 0)
			{
				value = tmpValue;
				return index;
			}
		}
		return -1;
	}

	static int pb_write_varint(std::string& buf, uint64_t value)
	{
		int len = 0;
		while (value != 0)
		{
			uint8_t temp = value & 0b1111111;
			value = value >> 7;
			buf.push_back(value == 0 ? temp : (temp | 0b10000000));
			len++;
		}
		return len;
	}

	static uint64_t pb_maketagnum(uint64_t tag, PB_TYPE type)
	{
		switch (type)
		{
		case PB_TYPE::PB_TYPE_VARINT:
			return (tag << 3) | 0;
		case PB_TYPE::PB_TYPE_FIXED64:
			return (tag << 3) | 1;
		case PB_TYPE::PB_TYPE_FIXED32:
			return (tag << 3) | 5;
		default:
			return (tag << 3) | 2;
			break;
		}
	}

	static bool IsUTF8(const void* pBuffer, long size)
	{
		bool IsUTF8 = true;
		unsigned char* start = (unsigned char*)pBuffer;
		unsigned char* end = (unsigned char*)pBuffer + size;
		while (start < end)
		{
			if (*start < 0x80) // (10000000): 值小于0x80的为ASCII字符
			{
				start++;
			}
			else if (*start < (0xC0)) // (11000000): 值介于0x80与0xC0之间的为无效UTF-8字符
			{
				IsUTF8 = false;
				break;
			}
			else if (*start < (0xE0)) // (11100000): 此范围内为2字节UTF-8字符
			{
				if (start >= end - 1)
				{
					break;
				}

				if ((start[1] & (0xC0)) != 0x80)
				{
					IsUTF8 = false;
					break;
				}

				start += 2;
			}
			else if (*start < (0xF0)) // (11110000): 此范围内为3字节UTF-8字符
			{
				if (start >= end - 2)
				{
					break;
				}

				if ((start[1] & (0xC0)) != 0x80 || (start[2] & (0xC0)) != 0x80)
				{
					IsUTF8 = false;
					break;
				}

				start += 3;
			}
			else
			{
				IsUTF8 = false;
				break;
			}
		}
		return IsUTF8;
	}

	int Uint64_t::SetValue(const char* buffer, int buffer_len)
	{
		return pb_read_varint(buffer, buffer_len, *(uint64_t*)m_Value);
	}
	std::string Uint64_t::ToString()
	{
		std::string result;
		pb_write_varint(result, *(uint64_t*)m_Value);
		return result;
	}

	Protobuf::Protobuf(PB_TYPE type)
		:m_Type(type), m_Value()
	{}
	Protobuf::Protobuf(const Protobuf& _that)
	{
		m_Type = _that.m_Type;
		switch (m_Type)
		{
		case PB_TYPE::PB_TYPE_VARINT:
		{
			this->m_Value.Uint64 = _that.m_Value.Uint64;
			break;
		}
		case PB_TYPE::PB_TYPE_FIXED64:
		{
			this->m_Value.Fixed64 = _that.m_Value.Fixed64;
			break;
		}
		case PB_TYPE::PB_TYPE_FIXED32:
		{
			this->m_Value.Fixed32 = _that.m_Value.Fixed32;
			break;
		}
		case PB_TYPE::PB_TYPE_BINARY:
		{
			this->m_Value.Binary = _that.m_Value.Binary;
			break;
		}
		case PB_TYPE::PB_TYPE_ARRAY:
		{
			this->m_Value.Array = _that.m_Value.Array;
			break;
		}
		case PB_TYPE::PB_TYPE_OBJECT:
		{
			this->m_Value.Object = _that.m_Value.Object;
			break;
		}
		default:
			break;
		}
	}
	Protobuf& Protobuf::operator=(const Protobuf& _that)
	{
		this->m_Type = _that.m_Type;
		//赋值函数要考虑:this=this->item
		switch (_that.m_Type)
		{
		case PB_TYPE::PB_TYPE_VARINT:
		{
			this->m_Value.Uint64 = _that.m_Value.Uint64;
			break;
		}
		case PB_TYPE::PB_TYPE_FIXED64:
		{
			this->m_Value.Fixed64 = _that.m_Value.Fixed64;
			break;
		}
		case PB_TYPE::PB_TYPE_FIXED32:
		{
			this->m_Value.Fixed32 = _that.m_Value.Fixed32;
			break;
		}
		case PB_TYPE::PB_TYPE_BINARY:
		{
			this->m_Value.Binary = _that.m_Value.Binary;
			break;
		}
		case PB_TYPE::PB_TYPE_ARRAY:
		{
			this->m_Value.Array = _that.m_Value.Array;
			break;
		}
		case PB_TYPE::PB_TYPE_OBJECT:
		{
			this->m_Value.Object = _that.m_Value.Object;
			break;
		}
		default:
			break;
		}
		return *this;
	}

	bool Protobuf::operator==(const PB_TYPE& type)
	{
		return type == this->m_Type;
	}
	Protobuf& Protobuf::operator+=(const Protobuf& _that)
	{
		if (this->m_Type != PB_TYPE::PB_TYPE_ARRAY)
		{
			Protobuf tmp = *this;
			this->m_Type = PB_TYPE::PB_TYPE_ARRAY;
			this->m_Value.Array.Packed() = false;
			this->m_Value.Array.Items().push_back(tmp);
		}
		this->m_Value.Array.Items().push_back(_that);
		return *this;
	}
	Protobuf& Protobuf::operator[] (std::string array_index)
	{
		if (this == nullptr)
		{
			return *this;
		}
		std::regex reg(R"(^([oOaA]{1})\:(\d+)([iIdDfFaAoObB]?)$)");
		std::smatch m;
		if (std::regex_search(array_index, m, reg) && m.size() <= 0)
		{
			return *(Protobuf*)nullptr;
		}
		Protobuf* result = nullptr;
		uint64_t index = _atoi64(m[2].str().c_str());
		switch (m[1].str()[0])
		{
		case 'a': case 'A':
		{
			if (this->m_Type == PB_TYPE::PB_TYPE_BINARY && m[3] != "")
			{
				switch (m[3].str()[0])
				{
				case 'i': case 'I':
					this->ToArray(PB_TYPE::PB_TYPE_VARINT);
					break;
				case 'd': case 'D':
					this->ToArray(PB_TYPE::PB_TYPE_FIXED64);
					break;
				case 'f': case 'F':
					this->ToArray(PB_TYPE::PB_TYPE_FIXED32);
					break;
				default:
					break;
				}
			}
			if (this->m_Type == PB_TYPE::PB_TYPE_ARRAY && this->m_Value.Array.Items().size() > index)
			{
				result = &this->m_Value.Array.Items()[(unsigned int)index];
			}
			break;
		}
		case 'o': case 'O':
		{
			if (this->m_Type == PB_TYPE::PB_TYPE_BINARY)
			{
				this->ToObject();
			}
			if (this->m_Type == PB_TYPE::PB_TYPE_OBJECT)
			{
				auto find = this->m_Value.Object.find(index);
				if (find != this->m_Value.Object.end())
				{
					result = &find->second;
				}
			}
		}
		}
		if (&result && m[3] != "")
		{
			switch (m[3].str()[0])
			{
			case 'i': case 'I':
			{
				if (result->m_Type != PB_TYPE::PB_TYPE_VARINT)
				{
					result = nullptr;
				}
				break;
			}
			case 'd': case 'D':
			{
				if (result->m_Type != PB_TYPE::PB_TYPE_FIXED64)
				{
					result = nullptr;
				}
				break;
			}
			case 'f': case 'F':
			{
				if (result->m_Type != PB_TYPE::PB_TYPE_FIXED32)
				{
					result = nullptr;
				}
				break;
			}
			case 'b': case 'B':
			{
				if (result->m_Type != PB_TYPE::PB_TYPE_BINARY)
				{
					result = nullptr;
				}
				break;
			}
			case 'o': case 'O':
			{
				if (result->m_Type != PB_TYPE::PB_TYPE_OBJECT)
				{
					result = nullptr;
				}
				break;
			}
			case 'a': case 'A':
			{
				if (result->m_Type != PB_TYPE::PB_TYPE_ARRAY && result->m_Type != PB_TYPE::PB_TYPE_BINARY)
				{
					result = nullptr;
				}
				break;
			}
			default:
				return *(Protobuf*)nullptr;
				break;
			}
		}
		return *result;
	}

	Protobuf::Protobuf(uint64_t value)
		:Protobuf(PB_TYPE::PB_TYPE_VARINT)
	{
		this->m_Value.Uint64.AsUint64() = value;
	}
	Protobuf::Protobuf(double value)
		:Protobuf(PB_TYPE::PB_TYPE_FIXED64)
	{
		this->m_Value.Fixed64.AsDouble() = value;
	}
	Protobuf::Protobuf(float value)
		:Protobuf(PB_TYPE::PB_TYPE_FIXED32)
	{
		this->m_Value.Fixed32.AsFloat() = value;
	}
	Protobuf::Protobuf(std::string value)
		:Protobuf(PB_TYPE::PB_TYPE_BINARY)
	{
		this->m_Value.Binary = value;
	}
	PB_TYPE Protobuf::GetType()
	{
		return this->m_Type;
	}
	Uint64_t& Protobuf::Uint64()
	{
		if (m_Type != PB_TYPE::PB_TYPE_VARINT)
		{
			return *(Uint64_t*)nullptr;
		}
		return this->m_Value.Uint64;
	}
	Fixed64_t& Protobuf::Fixed64()
	{
		if (m_Type != PB_TYPE::PB_TYPE_FIXED64)
		{
			return *(Fixed64_t*)nullptr;
		}
		return this->m_Value.Fixed64;
	}
	Fixed32_t& Protobuf::Fixed32()
	{
		if (m_Type != PB_TYPE::PB_TYPE_FIXED32)
		{
			return *(Fixed32_t*)nullptr;
		}
		return this->m_Value.Fixed32;
	}

	std::string Protobuf::ToBinaryAsTag(uint64_t tag)
	{
		std::string result;
		switch (this->m_Type)
		{
		case PB_TYPE::PB_TYPE_VARINT:
		case PB_TYPE::PB_TYPE_FIXED64:
		case PB_TYPE::PB_TYPE_FIXED32:
		{
			this->ToBinary();
			pb_write_varint(result, pb_maketagnum(tag, this->m_Type));
			result.insert(result.end(), this->m_Value.Binary.begin(), this->m_Value.Binary.end());
		}
		case PB_TYPE::PB_TYPE_BINARY:
		case PB_TYPE::PB_TYPE_OBJECT:
		{
			this->ToBinary();
			if (this->m_Value.Binary.length() > 0)
			{
				pb_write_varint(result, pb_maketagnum(tag, this->m_Type));
				pb_write_varint(result, this->m_Value.Binary.length());
				result.insert(result.end(), this->m_Value.Binary.begin(), this->m_Value.Binary.end());
			}
		}
		case PB_TYPE::PB_TYPE_ARRAY:
		{
			this->m_Value.Binary.clear();
			if (this->m_Value.Array.Items().size() > 0)
			{
				//只有PB_TYPE_VARINT,PB_TYPE_FIXED64,PB_TYPE_FIXED32类型才支持Packed
				if ((this->m_Value.Array.Items()[0].m_Type == PB_TYPE::PB_TYPE_VARINT ||
					this->m_Value.Array.Items()[0].m_Type == PB_TYPE::PB_TYPE_FIXED64 ||
					this->m_Value.Array.Items()[0].m_Type == PB_TYPE::PB_TYPE_FIXED32)
					&& this->m_Value.Array.Packed())
				{
					//内存排列:tag|total_size|item0|item1|item2...
					this->ToBinary();
					pb_write_varint(result, pb_maketagnum(tag, this->m_Type));
					pb_write_varint(result, this->m_Value.Binary.length());
					result.insert(result.end(), this->m_Value.Binary.begin(), this->m_Value.Binary.end());
				}
				else
				{
					//内存排列:tag|item0_size|item0|tag|item1_size|item1|tag|item2_size|item2...
					for (auto& Item : this->m_Value.Array.Items())
					{
						Protobuf ItemBin = Item.ToBinaryAsTag(tag);
						this->m_Value.Binary.insert(this->m_Value.Binary.end(), ItemBin.Binary().begin(), ItemBin.Binary().end());
					}
					result = this->m_Value.Binary;
				}
			}
			this->m_Type = PB_TYPE::PB_TYPE_BINARY;
			break;
		}
		default:
		{
			break;
		}
		}
		return result;
	}

	std::string Protobuf::ToViewAsTitle(std::string title, bool brevity)
	{
		std::string result;
		char buffer[0x1000];
		switch (this->m_Type)
		{
		case PB_TYPE::PB_TYPE_VARINT:
		{
			memset(buffer, 0, sizeof(buffer));
			sprintf_s(buffer, "%s(I)->:%lld", title.c_str(), this->m_Value.Uint64.AsUint64());
			result = buffer;
			break;
		}
		case PB_TYPE::PB_TYPE_FIXED64:
		{
			memset(buffer, 0, sizeof(buffer));
			sprintf_s(buffer, "%s(D)->:%f", title.c_str(), this->m_Value.Fixed64.AsDouble());
			result = buffer;
			break;
		}
		case PB_TYPE::PB_TYPE_FIXED32:
		{
			memset(buffer, 0, sizeof(buffer));
			sprintf_s(buffer, "%s(F)->:%f", title.c_str(), this->m_Value.Fixed32.AsFloat());
			result = buffer;
			break;
		}
		case PB_TYPE::PB_TYPE_BINARY:
		{
			const char* TT = "0123456789ABCDEF";
			std::string tmp_str;
			for (unsigned long i = 0; i < this->m_Value.Binary.length(); i++)
			{
				tmp_str.push_back(TT[(this->m_Value.Binary[i] >> 4) & 0xF]);
				tmp_str.push_back(TT[(this->m_Value.Binary[i] >> 0) & 0xF]);
			}
			memset(buffer, 0, sizeof(buffer));
			sprintf_s(buffer, "%s(B:%d)->:%s", title.c_str(), this->m_Value.Binary.length(), tmp_str.c_str());
			result = buffer;

			if (this->ToObject())
			{
				result += "\r\n" + ToViewAsTitle(title, brevity);
			}
			else if (brevity && IsUTF8(this->m_Value.Binary.c_str(), this->m_Value.Binary.length()))
			{
				break;
			}
			else if (this->ToArray(PB_TYPE::PB_TYPE_VARINT) || this->ToArray(PB_TYPE::PB_TYPE_FIXED64) || this->ToArray(PB_TYPE::PB_TYPE_FIXED32))
			{
				result += "\r\n" + ToViewAsTitle(title, brevity);
			}
			break;
		}
		case PB_TYPE::PB_TYPE_ARRAY:
		{
			unsigned index = 0;
			result = "";
			for (auto& Item : this->m_Value.Array.Items())
			{
				memset(buffer, 0, sizeof(buffer));
				sprintf_s(buffer, "%s[%d]", title.c_str(), index++);
				if (result == "")
				{
					result += Item.ToViewAsTitle(buffer, brevity);
				}
				else
				{
					result += "\r\n" + Item.ToViewAsTitle(buffer, brevity);
				}
			}
			break;
		}
		case PB_TYPE::PB_TYPE_OBJECT:
		{
			result = "";
			for (auto member : this->m_Value.Object)
			{
				memset(buffer, 0, sizeof(buffer));
				if (title == "")
				{
					sprintf_s(buffer, "%lld", member.first);
				}
				else
				{
					sprintf_s(buffer, "%s.%lld", title.c_str(), member.first);
				}
				if (result == "")
				{
					result += member.second.ToViewAsTitle(buffer, brevity);
				}
				else
				{
					result += "\r\n" + member.second.ToViewAsTitle(buffer, brevity);
				}
			}
			break;
		}
		default:
			break;
		}
		return result;
	}

	void Protobuf::ToBinary()
	{
		if (this == nullptr)
		{
			return;
		}
		switch (this->m_Type)
		{
		case PB_TYPE::PB_TYPE_VARINT:
		{
			this->m_Value.Binary.clear();
			this->m_Value.Binary += this->m_Value.Uint64.ToString();
			this->m_Type = PB_TYPE::PB_TYPE_BINARY;
			break;
		}
		case PB_TYPE::PB_TYPE_FIXED64:
		{
			this->m_Value.Binary.clear();
			this->m_Value.Binary.insert(this->m_Value.Binary.end(), (char*)&this->m_Value.Fixed64, (char*)&this->m_Value.Fixed64 + sizeof(double));
			this->m_Type = PB_TYPE::PB_TYPE_BINARY;
			break;
		}
		case PB_TYPE::PB_TYPE_FIXED32:
		{
			this->m_Value.Binary.clear();
			this->m_Value.Binary.insert(this->m_Value.Binary.end(), (char*)&this->m_Value.Fixed32, (char*)&this->m_Value.Fixed32 + sizeof(float));
			this->m_Type = PB_TYPE::PB_TYPE_BINARY;
			break;
		}
		case PB_TYPE::PB_TYPE_BINARY:
		{
			break;
		}
		case PB_TYPE::PB_TYPE_ARRAY:
		{
			//这只处理Packed
			this->m_Value.Binary.clear();
			for (auto& Item : this->m_Value.Array.Items())
			{
				Item.ToBinary();
				this->m_Value.Binary.insert(this->m_Value.Binary.end(), Item.m_Value.Binary.begin(), Item.m_Value.Binary.end());
			}
			this->m_Type = PB_TYPE::PB_TYPE_BINARY;
			break;
		}
		case PB_TYPE::PB_TYPE_OBJECT:
		{
			this->m_Value.Binary.clear();
			for (auto& Member : this->m_Value.Object)
			{
				std::string sub = Member.second.ToBinaryAsTag(Member.first);
				this->m_Value.Binary.insert(this->m_Value.Binary.end(), sub.begin(), sub.end());
			}
			this->m_Type = PB_TYPE::PB_TYPE_BINARY;
			break;
		}
		default:
			break;
		}
	}
	bool Protobuf::ToObject()
	{
		if (m_Type == PB_TYPE::PB_TYPE_OBJECT)
		{
			return true;
		}
		if (m_Type != PB_TYPE::PB_TYPE_BINARY)
		{
			return false;
		}
		bool result = true;
		bool group_begin = false;
		size_t pos = 0;
		uint64_t varint = 0;
		int varint_len = 0;
		this->m_Value.Object.clear();
		while (pos < this->m_Value.Binary.length())
		{
			varint_len = pb_read_varint(&this->m_Value.Binary[pos], this->m_Value.Binary.length() - pos, varint);
			if (varint_len <= 0)
			{
				result = false;
			}
			else
			{
				uint64_t tag = varint >> 3;
				pos += varint_len;
				auto find_item = this->m_Value.Object.find(tag);
				//根据类型解析数据
				switch (varint & 0b111)
				{
				case 0:
				{
					varint_len = pb_read_varint(&this->m_Value.Binary[pos], this->m_Value.Binary.length() - pos, varint);
					if (varint_len <= 0)
					{
						result = false;
					}
					else
					{
						if (find_item == this->m_Value.Object.end())
						{
							this->m_Value.Object.insert(std::make_pair(tag, varint));
						}
						else
						{
							find_item->second += varint;
						}
						pos += varint_len;
					}
					break;
				}
				case 1:
				{
					if (this->m_Value.Binary.length() < pos + sizeof(double))
					{
						result = false;
					}
					else
					{
						if (find_item == this->m_Value.Object.end())
						{
							this->m_Value.Object.insert(std::make_pair(tag, *(double*)&this->m_Value.Binary[pos]));
						}
						else
						{
							find_item->second += *(double*)&this->m_Value.Binary[pos];
						}
						pos += sizeof(double);
					}
					break;
				}
				case 2:
				{
					varint_len = pb_read_varint(&this->m_Value.Binary[pos], this->m_Value.Binary.length() - pos, varint);
					if (varint_len <= 0)
					{
						result = false;
					}
					else
					{
						pos += varint_len;
						if (this->m_Value.Binary.length() < pos + varint)
						{
							result = false;
						}
						else
						{
							if (find_item == this->m_Value.Object.end())
							{
								this->m_Value.Object.insert(std::make_pair(tag, std::string(&this->m_Value.Binary[pos], (int)varint)));
							}
							else
							{
								find_item->second += std::string(&this->m_Value.Binary[pos], (int)varint);
							}
							pos += (int)varint;
						}
					}
					break;
				}
				case 5:
				{
					if (this->m_Value.Binary.length() < pos + sizeof(float))
					{
						result = false;
					}
					else
					{
						if (find_item == this->m_Value.Object.end())
						{
							this->m_Value.Object.insert(std::make_pair(tag, *(float*)&this->m_Value.Binary[pos]));
						}
						else
						{
							find_item->second += *(float*)&this->m_Value.Binary[pos];
						}
						pos += sizeof(float);
					}
					break;
				}
				default:
				{
					result = false;
					break;
				}
				}
			}
			if (!result)
			{
				break;
			}
		}
		if (result)
		{
			this->m_Type = PB_TYPE::PB_TYPE_OBJECT;
		}
		return result;
	}

	bool Protobuf::ToArray(PB_TYPE item_type)
	{
		if (this->m_Type == PB_TYPE::PB_TYPE_ARRAY)
		{
			return true;
		}
		if (this->m_Type != PB_TYPE::PB_TYPE_BINARY)
		{
			return false;
		}
		bool result = true;
		this->m_Value.Array.Packed() = true;
		this->m_Value.Array.Items().clear();
		int pos = 0;
		while (pos < (int)m_Value.Binary.length())
		{
			switch (item_type)
			{
			case PB_TYPE::PB_TYPE_VARINT:
			{
				uint64_t value = 0;
				int value_len = pb_read_varint(&this->m_Value.Binary[pos], this->m_Value.Binary.length() - pos, value);
				if (value_len <= 0)
				{
					result = false;
				}
				else
				{
					this->m_Value.Array.Items().push_back(value);
					pos += value_len;
				}
				break;
			}
			case PB_TYPE::PB_TYPE_FIXED64:
			{
				if (m_Value.Binary.length() < pos + sizeof(double))
				{
					result = false;
				}
				else
				{
					this->m_Value.Array.Items().push_back(*(double*)&m_Value.Binary[pos]);
					pos += sizeof(double);
				}
				break;
			}
			case PB_TYPE::PB_TYPE_FIXED32:
			{
				if (m_Value.Binary.length() < pos + sizeof(float))
				{
					result = false;
				}
				else
				{
					this->m_Value.Array.Items().push_back(*(float*)&m_Value.Binary[pos]);
					pos += sizeof(float);
				}
				break;
			}
			default:
			{
				result = false;
				break;
			}
			}
			if (!result)
			{
				break;
			}
		}
		if (result)
		{
			m_Type = PB_TYPE::PB_TYPE_ARRAY;
		}
		return result;
	}

	Binary_t& Protobuf::Binary()
	{
		this->ToBinary();
		return m_Value.Binary;
	}
	Array_t& Protobuf::Array()
	{
		if (m_Type == PB_TYPE::PB_TYPE_BINARY)
		{
			ToArray(PB_TYPE::PB_TYPE_BINARY);
		}
		if (m_Type != PB_TYPE::PB_TYPE_ARRAY)
		{
			return *(Array_t*)nullptr;
		}
		return m_Value.Array;
	}
	Object_t& Protobuf::Object()
	{
		if (m_Type == PB_TYPE::PB_TYPE_BINARY)
		{
			this->ToObject();
		}
		if (m_Type != PB_TYPE::PB_TYPE_OBJECT)
		{
			return *(Object_t*)nullptr;
		}
		return m_Value.Object;
	}
	std::string Protobuf::ToView(bool brevity)
	{
		if (this->m_Type != PB_TYPE::PB_TYPE_OBJECT)
		{
			this->ToObject();
		}
		return this->ToViewAsTitle("", brevity);
	}

	Protobuf& Protobuf::ValueByPath(const std::string path, PB_TYPE des_type)
	{
		if (this == nullptr)
		{
			return *this;
		}
		if (this->m_Type != PB_TYPE::PB_TYPE_OBJECT && this->m_Type != PB_TYPE::PB_TYPE_BINARY)
		{
			return *(Protobuf*)nullptr;
		}
		std::regex reg(R"(^(\d+)?(\[(\d+?)\])?.?(.*)$)");
		std::smatch m;
		if (std::regex_search(path, m, reg) && m.size() <= 0)
		{
			return *(Protobuf*)nullptr;
		}
		if (this->m_Type == PB_TYPE::PB_TYPE_BINARY && !this->ToObject())
		{
			return *(Protobuf*)nullptr;
		}
		auto item = this->m_Value.Object.find(atoi(m[1].str().c_str()));
		if (item == this->m_Value.Object.end())
		{
			return *(Protobuf*)nullptr;
		}
		Protobuf* the_item = &item->second;
		if (m[3] != "")
		{
			//数组处理
			unsigned index = atoi(m[3].str().c_str());
			if (item->second.m_Type != PB_TYPE::PB_TYPE_ARRAY && item->second.m_Type != PB_TYPE::PB_TYPE_BINARY)
			{
				return *(Protobuf*)nullptr;
			}
			if (item->second.m_Type == PB_TYPE::PB_TYPE_BINARY)
			{
				PB_TYPE the_type = m[4] == "" ? des_type : PB_TYPE::PB_TYPE_BINARY;
				//只有PB_TYPE_VARINT,PB_TYPE_FIXED64,PB_TYPE_FIXED32类型才支持Packed
				if (the_type != PB_TYPE::PB_TYPE_VARINT && the_type != PB_TYPE::PB_TYPE_FIXED64 && the_type != PB_TYPE::PB_TYPE_FIXED32)
				{
					return *(Protobuf*)nullptr;
				}
				if (!item->second.ToArray(the_type))
				{
					return *(Protobuf*)nullptr;
				}
			}
			if (item->second.m_Value.Array.Items().size() <= index)
			{
				return *(Protobuf*)nullptr;
			}
			the_item = &item->second.m_Value.Array.Items()[index];
		}
		if (m[4] == "")
		{
			return *the_item;
		}
		else
		{
			return the_item->ValueByPath(m[4], des_type);
		}
	}
	bool Protobuf::GetValueByPath(const std::string path, uint64_t& value)
	{
		Protobuf& pb = this->ValueByPath(path, PB_TYPE::PB_TYPE_VARINT);
		if (&pb == nullptr || pb.m_Type != PB_TYPE::PB_TYPE_VARINT)
		{
			return false;
		}
		value = pb.m_Value.Uint64.AsUint64();
		return true;
	}
	bool Protobuf::GetValueByPath(const std::string path, double& value)
	{
		Protobuf& pb = this->ValueByPath(path, PB_TYPE::PB_TYPE_FIXED64);
		if (&pb == nullptr || pb.m_Type != PB_TYPE::PB_TYPE_FIXED64)
		{
			return false;
		}
		value = pb.m_Value.Fixed64.AsDouble();
		return true;
	}
	bool Protobuf::GetValueByPath(const std::string path, float& value)
	{
		Protobuf& pb = this->ValueByPath(path, PB_TYPE::PB_TYPE_FIXED32);
		if (&pb == nullptr || pb.m_Type != PB_TYPE::PB_TYPE_FIXED32)
		{
			return false;
		}
		value = pb.m_Value.Fixed32.AsFloat();
		return true;
	}
	bool Protobuf::GetValueByPath(const std::string path, std::string& value)
	{
		Protobuf& pb = this->ValueByPath(path, PB_TYPE::PB_TYPE_BINARY);
		if (&pb == nullptr || pb.m_Type != PB_TYPE::PB_TYPE_BINARY)
		{
			return false;
		}
		value = pb.m_Value.Binary;
		return true;
	}

	bool Protobuf::SetValueByPath(const std::string path, uint64_t value)
	{
		Protobuf& pb = this->ValueByPath(path, PB_TYPE::PB_TYPE_VARINT);
		if (&pb == nullptr || pb.m_Type != PB_TYPE::PB_TYPE_VARINT)
		{
			return false;
		}
		pb.m_Value.Uint64.AsUint64() = value;
		return true;
	}
	bool Protobuf::SetValueByPath(const std::string path, double value)
	{
		Protobuf& pb = this->ValueByPath(path, PB_TYPE::PB_TYPE_FIXED64);
		if (&pb == nullptr || pb.m_Type != PB_TYPE::PB_TYPE_FIXED64)
		{
			return false;
		}
		pb.m_Value.Fixed64.AsDouble() = value;
		return true;
	}
	bool Protobuf::SetValueByPath(const std::string path, float value)
	{
		Protobuf& pb = this->ValueByPath(path, PB_TYPE::PB_TYPE_FIXED32);
		if (&pb == nullptr || pb.m_Type != PB_TYPE::PB_TYPE_FIXED32)
		{
			return false;
		}
		pb.m_Value.Fixed32.AsFloat() = value;
		return true;
	}
	bool Protobuf::SetValueByPath(const std::string path, std::string value)
	{
		Protobuf& pb = this->ValueByPath(path, PB_TYPE::PB_TYPE_BINARY);
		if (&pb == nullptr || pb.m_Type != PB_TYPE::PB_TYPE_BINARY)
		{
			return false;
		}
		pb.m_Value.Binary = value;
		return true;
	}
}