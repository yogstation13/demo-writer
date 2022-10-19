#pragma once
#include <vector>
#include "../core/byond_structures.h"

void write_vlq(std::vector<unsigned char>& buf, int value);
void write_vlq(int value);
void write_byond_string(std::vector<unsigned char>& buf, int string_id);
void write_byond_resourceid(std::vector<unsigned char>& buf, int resource_id);
void update_demo_time();
void write_world_size();

extern bool demo_time_override_enabled;
extern float demo_time_override;

template<class T>
class ByteVecRef {
public:
	ByteVecRef(std::vector<unsigned char>* vec, int num) {
		this->vec = vec;
		this->num = num;
	}
	T* operator->() {
		return (T*)&(*vec)[num];
	}
private:
	std::vector<unsigned char> *vec;
	int num;
};

template<class T>
ByteVecRef<T> write_primitive(std::vector<unsigned char>& buf, const T &value) {
	int n = buf.size();
	buf.resize(n + sizeof(T));
	memcpy(&buf[n], &value, sizeof(T));
	return ByteVecRef<T>(&buf, n);
}

inline unsigned int ref_int(Value val) {
	if (!val.type) return 0;
	return (val.type << 24) | (val.value & 0xFFFFFF);
}
inline unsigned int ref_int_relative(Value val, unsigned int parent) {
	if (!val.type) return 0;
	return (val.type << 24) | 0x80000000 | ((val.value - parent) & 0xFFFFFF);
}
