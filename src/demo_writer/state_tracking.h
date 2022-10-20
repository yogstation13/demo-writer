#pragma once
#include <vector>
#include "../core/core.h"

struct alignas(1) DemoWriterIdFlags {
	union {
		struct {
			// as you can see I care about memory usage... lol
			unsigned char string_written : 1;
			unsigned char appearance_written : 1;
			unsigned char obj_written : 1;
			unsigned char mob_written : 1;
			unsigned char turf_written : 1;
			unsigned char resource_written : 1;
			unsigned char filter_written : 1;
			unsigned char image_written : 1;
		};
		unsigned char byte = 0;
	};
	template<typename T> inline bool get_written();

	template<typename T> inline void set_written(bool f);
};


template<> inline bool DemoWriterIdFlags::get_written<Obj>() { return obj_written; }
template<> inline bool DemoWriterIdFlags::get_written<Mob>() { return mob_written; }
template<> inline bool DemoWriterIdFlags::get_written<Turf>() { return turf_written; }
template<> inline bool DemoWriterIdFlags::get_written<ImageOverlay> () { return image_written; }
template<> inline void DemoWriterIdFlags::set_written<Obj>(bool f) { obj_written = f; }
template<> inline void DemoWriterIdFlags::set_written<Mob>(bool f) { mob_written = f; }
template<> inline void DemoWriterIdFlags::set_written<Turf>(bool f) { turf_written = f; }
template<> inline void DemoWriterIdFlags::set_written<ImageOverlay>(bool f) { image_written = f; }

extern std::vector<DemoWriterIdFlags> demo_id_flags;
DemoWriterIdFlags& get_demo_id_flags(int id);
