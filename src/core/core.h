#pragma once

#include <memory>
#include "byond_structures.h"
#include <string>
#include "find_functions.h"
#include "hooking.h"
#include "byond_functions.h"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT __attribute__((visibility("default")))
#endif

namespace Core {
	void Alert(const std::string& what);
	void Alert(int what);

	std::string GetStringFromId(unsigned int id);
	Value DecReturn(Value val);

	extern std::unique_ptr<RefTable<Obj>> obj_table;
	extern std::unique_ptr<RefTable<Datum>> datum_table;
	//extern TableHolder2* list_table; //list list honk
	extern std::unique_ptr<RefTable<Client, unsigned short>> client_table;
	extern std::unique_ptr<RefTable<Mob>> mob_table;
	extern std::unique_ptr<RefTable<ImageOverlay>> image_table;
	extern TableHolder2* appearance_list_table;
	extern AppearanceTable** appearance_table;
	
	extern int** turf_table;
	extern Turf*** turf_hashtable;
	extern unsigned short* turf_hashtable_mask;
	extern unsigned char** turf_existence_table;
	extern TurfSharedInfo*** turf_shared_info_table;
	extern WorldSizeHolder* world_size;
}