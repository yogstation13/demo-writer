#include "write_primitive.h"
#include "demo_writer.h"
#include "state_tracking.h"
#include "../core/core.h"

void write_vlq(std::vector<unsigned char> &buf, int value) {
	int val_buf;
	val_buf = value & 0x7f;
	while ((value >>= 7) > 0)
	{
		val_buf <<= 8;
		val_buf |= 0x80;
		val_buf += (value & 0x7f);
	}

	while (true)
	{
		buf.push_back(val_buf);
		if (val_buf & 0x80) val_buf >>= 8;
		else
			break;
	}
}

void write_vlq(int value) {
	int val_buf;
	val_buf = value & 0x7f;
	while ((value >>= 7) > 0)
	{
		val_buf <<= 8;
		val_buf |= 0x80;
		val_buf += (value & 0x7f);
	}

	while (true)
	{
		demo_file_handle.put(val_buf);
		if (val_buf & 0x80) val_buf >>= 8;
		else
			break;
	}
}

void write_byond_string(std::vector<unsigned char> &buf, int string_id) {
	if (string_id == 0 || string_id == 0xFFFF) {
		buf.push_back(0);
		return;
	}
	String *str = GetStringTableEntry(string_id);
	if (!str) {
		buf.push_back(0);
		return;
	}

	DemoWriterIdFlags& dif = get_demo_id_flags(string_id);
	bool do_write = !dif.string_written;
	dif.string_written = true;
	if (string_id < 256) {
		buf.push_back((int)do_write | 2);
		buf.push_back(string_id);
	}
	else if (string_id < 65536) {
		buf.push_back((int)do_write | 4);
		buf.push_back((string_id >> 8) & 0xFF);
		buf.push_back(string_id & 0xFF);
	}
	else {
		buf.push_back((int)do_write | 6);
		buf.push_back((string_id >> 16) & 0xFF);
		buf.push_back((string_id >> 8) & 0xFF);
		buf.push_back(string_id & 0xFF);
	}
	if (do_write) {
		char* chars = str->stringData;
		while (*chars != 0) {
			buf.push_back(*chars);
			chars++;
		}
		buf.push_back(0);
	}
}

void write_byond_resourceid(std::vector<unsigned char>& buf, int resource_id) {
	if (resource_id == 0xFFFF) {
		buf.push_back(0);
		return;
	}
	int string_id = ToString(RESOURCE, resource_id);
	String* str = GetStringTableEntry(string_id);

	DemoWriterIdFlags& dif = get_demo_id_flags(resource_id);
	bool do_write = string_id && !dif.resource_written;
	dif.resource_written = true;
	if (resource_id < 256) {
		buf.push_back((int)do_write | 2);
		buf.push_back(resource_id);
	}
	else if (resource_id < 65536) {
		buf.push_back((int)do_write | 4);
		buf.push_back((resource_id >> 8) & 0xFF);
		buf.push_back(resource_id & 0xFF);
	}
	else {
		buf.push_back((int)do_write | 6);
		buf.push_back((resource_id >> 16) & 0xFF);
		buf.push_back((resource_id >> 8) & 0xFF);
		buf.push_back(resource_id & 0xFF);
	}
	if (do_write) {
		char* chars = str->stringData;
		while (*chars != 0) {
			buf.push_back(*chars);
			chars++;
		}
		buf.push_back(0);
	}
}

float last_world_time = 0;
bool demo_time_override_enabled = false;
float demo_time_override = 0;

void update_demo_time() {
	float time = demo_time_override_enabled ? demo_time_override : GetVariable({ WORLD_D, {0} }, 0x4f).valuef; // world.time
	if (!(last_world_time < time)) return;
	last_world_time = time;
	demo_file_handle.put(0x00); // Chunk ID
	demo_file_handle.put(0x04); // Chunk Length
	demo_file_handle.write((char*)&time, sizeof(time));
}

void write_world_size() {
	struct {
		char chunk_id = 1;
		char chunk_length = 0x6;
		short maxx = Core::world_size->maxx;
		short maxy = Core::world_size->maxy;
		short maxz = Core::world_size->maxz;
	} data;
	demo_file_handle.write((char*)&data, sizeof(data));
}
