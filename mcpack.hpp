#ifndef    _MCPACKKAKASASA_H_
#define    _MCPACKKAKASASA_H_

#include <vector>
#include <string>

//两端默认端序相同  内部无异常处理  ！！！！
class MCPack
{
public:
	MCPack() {
	};
	~MCPack() {
	};

	std::vector<uint8_t> getWriteData();
	std::vector<uint8_t>& getWriteDataRef();
	size_t getWriteDataSize();
	//std::string printHex();

	void setReadData(std::vector<uint8_t>& bin);
	void setReadData(uint8_t* ptr, size_t len);
	void clear();//置空


	template<class T>
	T read();
	template<class T>
	T& read(T& val);

	template<class T>
	void write(T val);
	template<class T>
	void write_front(T val);//前面写数据 

	//特化 string
	template<>
	std::string read();
	template<>
	std::string& read(std::string& val);
	template<>
	void write(std::string val);
	template<>
	void write_front(std::string val);

	//特化 std::vector<uint8_t>
	template<>
	std::vector<uint8_t> read();
	template<>
	std::vector<uint8_t>& read(std::vector<uint8_t>& val);
	template<>
	void write(std::vector<uint8_t> val);
	template<>
	void write_front(std::vector<uint8_t> val);

private:
	int mPos = 0;//当前偏移
	std::vector<uint8_t> mBinaryW;//写数据
	std::vector<uint8_t> mBinaryR;//读数据

};

inline std::vector<uint8_t> MCPack::getWriteData()
{
	return mBinaryW;
}

inline std::vector<uint8_t>& MCPack::getWriteDataRef()
{
	return mBinaryW;
}

inline size_t MCPack::getWriteDataSize()
{
	return mBinaryW.size();
}

//std::string MCPack::printHex()
//{
//	std::string hexstr = "";
//	if (mBinaryW.size())
//	{
//		hexstr = utils::bin2hex(&mBinaryW[0],mBinaryW.size());
//	}
//	return hexstr;
//}


inline void MCPack::setReadData(std::vector<uint8_t>& bin)
{
	mBinaryR = bin;
	mPos = 0;
}
inline void MCPack::setReadData(uint8_t* ptr, size_t len)
{
	if (ptr && len)
	{
		mBinaryR.resize(len);
		memcpy(&mBinaryR[0], ptr, len);
		mPos = 0;
	}
}
//置空
inline void MCPack::clear()
{
	std::vector<uint8_t> v1, v2;
	mBinaryW.swap(v1);
	mBinaryR.swap(v2);
	mPos = 0;
}


//只支持sizeof(T)能得到准确大小的类型即基本类型及其构成的结构体
template<class T>
inline T MCPack::read()
{
	T val = *(T*)&mBinaryR[mPos];
	mPos = mPos + sizeof(T);
	return val;
}

template<typename T>
inline T& MCPack::read(T& val)
{
	val = read<T>();
	return val;
}


template<typename T>
inline void MCPack::write(T val)
{
	size_t len = sizeof(T);
	uint8_t* p = (uint8_t*)&val;
	size_t len_old = mBinaryW.size();

	std::vector<uint8_t> temp;
	temp.resize(len_old + len);

	if (len_old)
	{
		memcpy(&temp[0], &mBinaryW[0], len_old);
	}
	memcpy(&temp[len_old], p, len);

	mBinaryW.swap(temp);
}

template<class T>
inline void MCPack::write_front(T val)
{
	size_t len = sizeof(T);
	uint8_t* p = (uint8_t*)&val;
	size_t len_old = mBinaryW.size();

	std::vector<uint8_t> temp;
	temp.resize(len_old + len);

	memcpy(&temp[0], p, len);
	if (len_old)
	{
		memcpy(&temp[len], &mBinaryW[0], len_old);
	}
	mBinaryW.swap(temp);

}
//特化 string
template<>
inline std::string MCPack::read()
{
	std::string val = "";
	if (mPos < mBinaryR.size())
	{
		//strlen 空指针可能破坏堆
		int len = strlen((char*)&mBinaryR[mPos]);
		val = std::string((char*)&mBinaryR[mPos], len);
		mPos = mPos + len + 1;
	}
	return val;
}

template<>
inline std::string& MCPack::read(std::string& val)
{
	val = read<std::string>();
	return val;
}

template<>
inline void MCPack::write<std::string>(std::string val)
{
	size_t len = val.length() + 1;
	uint8_t* p = (uint8_t*)val.c_str();
	size_t len_old = mBinaryW.size();

	std::vector<uint8_t> temp;
	temp.resize(len_old + len);

	if (len_old)
	{
		memcpy(&temp[0], &mBinaryW[0], len_old);
	}
	memcpy(&temp[len_old], p, len);

	mBinaryW.swap(temp);

}

template<>
inline void MCPack::write_front(std::string val)
{
	size_t len = val.length() + 1;
	uint8_t* p = (uint8_t*)val.c_str();
	size_t len_old = mBinaryW.size();

	std::vector<uint8_t> temp(len_old + len,0);
	if (len_old)
	{
		memcpy(&temp[len], &mBinaryW[0], len_old);
	}
	memcpy(&temp[0], p, len);

	mBinaryW.swap(temp);
}

//特化 std::vector<uint8_t>
template<>
std::vector<uint8_t> MCPack::read()
{
	int32_t len = read<int32_t>();
	std::vector<uint8_t> val(len);
	if (len)
	{
		memcpy(&val[0], &mBinaryR[mPos], len);
	}
	mPos += len;
	return val;
}

template<>
inline std::vector<uint8_t>& MCPack::read(std::vector<uint8_t>& val)
{
	val = read<std::vector<uint8_t>>();
	return val;
}

template<>
inline void MCPack::write(std::vector<uint8_t> val)
{
	size_t len_append = val.size();
	write<int32_t>(len_append);//写入长度
	size_t len_old = mBinaryW.size();
	std::vector<uint8_t> temp(len_old + len_append, 0);//直接初始化匹配大小
	if (len_old)
	{
		memcpy(&temp[0], &mBinaryW[0], len_old);
	}
	if (len_append)
	{
		memcpy(&temp[len_old], val.data(), len_append);
	}
	mBinaryW.swap(temp);
}

template<>
inline void MCPack::write_front(std::vector<uint8_t> val)
{
	size_t len_append = val.size();
	write<int32_t>(len_append);//写入长度
	size_t len_old = mBinaryW.size();
	std::vector<uint8_t> temp;
	temp.resize(len_old + len_append);
	if (len_old)
	{
		memcpy(&temp[len_append], &mBinaryW[0], len_old);
	}
	if (len_append)
	{
		memcpy(&temp[0], val.data(), len_append);
	}
	mBinaryW.swap(temp);
}



#endif


//example
/*

#include <iostream>
#include "mcpack.hpp"

using namespace std;
int main()
{
	struct DEMOSTRUCT {
		BYTE a;
		short b;
		int c;
		double d;
		float e;
		BYTE barr[5];
	};

	DEMOSTRUCT vdemo = { 0x01,0x0202,0x03030303,4.4444,5.55,{0x01,0x02,0x03,0x04,0x05} };

	MCPack pac;

	pac.write<bool>(true);
	pac.write<BYTE>(0xEE);
	pac.write<short>(0xf0f0);
	pac.write<unsigned int>(0xAABBCCDD);
	pac.write<uint64_t>(0x00000000aabbccdd);
	pac.write<double>(3.1415);
	pac.write<std::string>("aaaaaaaaaa");
	std::vector<BYTE> vec(0);
	pac.write<std::vector<BYTE>>(vec);
	pac.write<std::string>("aaaaaaaaaa");
	std::vector<BYTE> vec1(5, 0xff);
	pac.write<std::vector<BYTE>>(vec1);
	pac.write<std::string>("");
	pac.write<DEMOSTRUCT>(vdemo);

	auto bin = pac.getWriteData();
	//cout << pac.printHex()<< endl;

	pac.setReadData(bin);
	auto b= pac.read<bool>();
	auto byte= pac.read<BYTE>();
	auto w= pac.read<short>();
	auto dw= pac.read<unsigned int>();
	auto i64= pac.read<uint64_t>();
	auto db= pac.read<double>();
	auto str= pac.read<std::string>();
	auto vec2 = pac.read< std::vector<BYTE>>();
	auto str2 = pac.read<std::string>();
	auto vec3 = pac.read< std::vector<BYTE>>();
	auto str3 = pac.read<std::string>();
	auto vd = pac.read<DEMOSTRUCT>();


	bool rbool;
	BYTE rb;
	short rshort;
	unsigned int rdw;
	uint64_t ri64;
	double rdb;
	std::string rstr, rstr1, rstr2;
	std::vector<BYTE> rvec, rvec1;
	DEMOSTRUCT vd1;
	pac.setReadData(bin);
	pac.read(rbool);
	pac.read(rb);
	pac.read(rshort);
	pac.read(rdw);
	pac.read(ri64);
	pac.read(rdb);
	pac.read(rstr);
	pac.read(rvec);
	pac.read(rstr1);
	pac.read(rvec1);
	pac.read(rstr2);
	pac.read(vd1);

	getchar();
}
*/