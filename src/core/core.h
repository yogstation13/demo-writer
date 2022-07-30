#pragma once

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

	extern std::unique_ptr<RefTable<Obj>> obj_table;
	//extern TableHolder2* datum_table;
	//extern TableHolder2* list_table; //list list honk
	extern std::unique_ptr<RefTable<Mob>> mob_table;
	extern TableHolder2* appearance_list_table;
	extern AppearanceTable** appearance_table;
	extern TurfTableHolder* turf_table;
	extern TurfHashtableHolder* turf_hashtable;
	extern TurfSharedInfo*** turf_shared_info_table;
}