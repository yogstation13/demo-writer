#include "core.h"
#ifdef _WIN32
#include <windows.h>
#endif
#include <stdio.h>

std::unique_ptr<RefTable<Obj>> Core::obj_table;
//TableHolder2* Core::datum_table = nullptr;
//TableHolder2* Core::list_table = nullptr;

std::unique_ptr<RefTable<Mob>> Core::mob_table;
TableHolder2* Core::appearance_list_table = nullptr;
AppearanceTable** Core::appearance_table = nullptr;
TurfTableHolder* Core::turf_table = nullptr;
TurfHashtableHolder* Core::turf_hashtable = nullptr;
TurfSharedInfo*** Core::turf_shared_info_table = nullptr;

void Core::Alert(const std::string& what) {
#ifdef _WIN32
	MessageBoxA(NULL, what.c_str(), "Ouch!", MB_OK);
#else
	printf("Ouch!: %s\n", what.c_str());
#endif
}

void Core::Alert(int what)
{
	Alert(std::to_string(what));
}

std::string Core::GetStringFromId(unsigned int id)
{
	return GetStringTableEntry(id)->stringData;
}
