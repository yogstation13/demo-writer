#include "find_functions.h"
#include "sigscan/sigscan.h"
#include "byond_functions.h"
#include "core.h"

#ifdef _WIN32
#include <windows.h>
#endif

#ifdef _WIN32
#define BYONDCORE "byondcore.dll"
#else
#define BYONDCORE "libbyond.so"
#endif

#define FIND_OR_DIE(name, sig) name = (name##Ptr)Pocket::Sigscan::FindPattern(BYONDCORE, sig); if(!name) { Core::Alert("demo-writer: Failed to locate " #name); return false; }
#define TRUE_OR_DIE(thing) if(!thing) { Core::Alert("demo-writer: Failed to locate " #thing); return false; }

bool Core::find_functions()
{	
#ifdef _WIN32
	FIND_OR_DIE(GetStringTableEntry, "55 8B EC 8B 4D 08 3B 0D ?? ?? ?? ?? 73 10 A1");
	FIND_OR_DIE(ToString, "55 8B EC 6A FF 68 ?? ?? ?? ?? 64 A1 ?? ?? ?? ?? 50 83 EC 18 53 56 57 A1 ?? ?? ?? ?? 33 C5 50 8D 45 ?? 64 A3 ?? ?? ?? ?? 8B 4D ?? 0F B6 C1");
	FIND_OR_DIE(SetAppearance, "55 8b ec 8b 45 08 83 ec 08 0f b6 c8 53 8d 41 ff 56 57 83 f8 53 0f 87 ?? ?? ?? ?? 0f b6 80 ?? ?? ?? ?? ff 24 85 ?? ?? ?? ?? ff 75 0c ff 75 10 e8 ?? ?? ?? ?? 83 c4 08 5f 5e 5b 8b e5 5d c3");
	FIND_OR_DIE(SetPixelX, "55 8b ec 8b 45 08 56 3c 03 75 16 8b 75 0c 3b 35 ?? ?? ?? ?? 0f 83 ad 00 00 00 a1 ?? ?? ?? ?? eb 18 3c 02 75 78 8b 75 0c 3b 35 ?? ?? ?? ?? 0f 83 ?? ?? ?? ?? a1 ?? ?? ?? ?? 8b 34 b0 85 f6 0f 84 ?? ?? ?? ?? 8b 45 10 66 3b 46 24 74 7a 66 89 46 24");
	FIND_OR_DIE(SetPixelY, "55 8b ec 8b 45 08 56 3c 03 75 16 8b 75 0c 3b 35 ?? ?? ?? ?? 0f 83 ad 00 00 00 a1 ?? ?? ?? ?? eb 18 3c 02 75 78 8b 75 0c 3b 35 ?? ?? ?? ?? 0f 83 ?? ?? ?? ?? a1 ?? ?? ?? ?? 8b 34 b0 85 f6 0f 84 ?? ?? ?? ?? 8b 45 10 66 3b 46 26 74 7a 66 89 46 26");
	FIND_OR_DIE(SetPixelW, "55 8b ec 8b 45 08 56 3c 03 75 16 8b 75 0c 3b 35 ?? ?? ?? ?? 0f 83 ad 00 00 00 a1 ?? ?? ?? ?? eb 18 3c 02 75 78 8b 75 0c 3b 35 ?? ?? ?? ?? 0f 83 ?? ?? ?? ?? a1 ?? ?? ?? ?? 8b 34 b0 85 f6 0f 84 ?? ?? ?? ?? 8b 45 10 66 3b 46 28 74 7a 66 89 46 28");
	FIND_OR_DIE(SetPixelZ, "55 8b ec 8b 45 08 56 3c 03 75 16 8b 75 0c 3b 35 ?? ?? ?? ?? 0f 83 ad 00 00 00 a1 ?? ?? ?? ?? eb 18 3c 02 75 78 8b 75 0c 3b 35 ?? ?? ?? ?? 0f 83 ?? ?? ?? ?? a1 ?? ?? ?? ?? 8b 34 b0 85 f6 0f 84 ?? ?? ?? ?? 8b 45 10 66 3b 46 2a 74 7a 66 89 46 2a");
	FIND_OR_DIE(SetMovableDir, "55 8b ec 6a ff 68 ?? ?? ?? ?? 64 a1 00 00 00 00 50 81 ec ?? ?? ?? ?? a1 ?? ?? ?? ?? 33 c5 89 45 f0 53 56 57 50 8d 45 f4 64 a3 00 00 00 00 8b 45 08 8b 7d 0c 89 85 00 ff ff ff 0f b6 c0");
	FIND_OR_DIE(SetLoc, "55 8b ec ff 75 14 ff 75 10 e8 ?? ?? ?? ?? 83 c4 08 84 c0 75 04 32 c0 5d c3");
	FIND_OR_DIE(SpliceAppearance, "55 8b ec 53 56 8b d9 57 8b 7d 08 8b f7 8b 4b 38 d3 ee 3b 73 28");
	FIND_OR_DIE(SpliceString, "55 8b ec 83 ec 08 56 57 8b 7d 08 57 e8 ?? ?? ?? ?? 8b f0 83 c4 04 85 f6 0f 84 ?? ?? ?? ?? 57 e8 ?? ?? ?? ?? 83 c4 04");
	FIND_OR_DIE(GetVariable, "55 8B EC 8B 4D ?? 0F B6 C1 48 83 F8 ?? 0F 87 ?? ?? ?? ?? 0F B6 80 ?? ?? ?? ?? FF 24 85 ?? ?? ?? ?? FF 75 ?? FF 75 ?? E8 ?? ?? ?? ??");
	FIND_OR_DIE(CreateObj, "55 8b ec 83 ec 0c 53 56 57 e8 ?? ?? ?? ?? 8b d8 3b 1d ?? ?? ?? ?? 72 04 33 ff eb 08 a1 ?? ?? ?? ??");
	FIND_OR_DIE(CreateMob, "55 8b ec 8b 45 10 83 ec 10 53 56 57 3b 05 ?? ?? ?? ?? 72 04 33 ff eb 0b");
	FIND_OR_DIE(ChangeTurf, "55 8b ec 83 ec 1c ff 75 18 ff 75 14 e8 ?? ?? ?? ?? 83 c4 08");
	FIND_OR_DIE(Animate, "55 8b ec 81 ec ?? ?? ?? ?? a1 ?? ?? ?? ?? 33 c5 89 45 fc 8b 45 08 8b 0d ?? ?? ?? ?? 8b 55 0c 53 56");
	FIND_OR_DIE(GetAssocElement, "55 8B EC 51 8B 4D 08 C6 45 FF 00 80 F9 05 76 11 80 F9 21 74 10 80 F9 0D 74 0B 80 F9 0E 75 65 EB 04 84 C9 74 5F 6A 00 8D 45 FF 50 FF 75 0C 51 6A 00 6A 7B");
	FIND_OR_DIE(Output, "55 8b ec 81 ec ?? ?? ?? ?? a1 ?? ?? ?? ?? 33 c5 89 45 ?? 8b 45 08 8b 4d 10 89 8d ?? ?? ?? ??");
	FIND_OR_DIE(GetVisContents, "55 8b ec 8b 45 08 0f b6 c0 48 83 f8 53 0f 87 ?? ?? ?? ??");
	FIND_OR_DIE(AddToScreen, "55 8b ec 66 8b 45 10 56 66 3b 05 ?? ?? ?? ?? 73 4a 0f b7 c8 a1 ?? ?? ?? ??");
	FIND_OR_DIE(RemoveFromScreen, "55 8b ec 66 8b 45 10 66 3b 05 ?? ?? ?? ?? 73 2d");
	FIND_OR_DIE(DelFilter, "55 8b ec 53 56 8b d9 57 8b 7d 08 8b f7 8b 4b 6c");
	FIND_OR_DIE(SendMapsClient, "55 8b ec 83 ec ?? 53 57 8b 7d 08 57 e8 ?? ?? ?? ?? 8b d8 83 c4 04 85 db 0f 84 ?? ?? ?? ??");
	FIND_OR_DIE(GetAppearance, "55 8b ec 8b 4d 08 0f b6 c1 83 c0 fe 83 f8 4e 0f 87 ?? ?? ?? ?? 0f b6 80 ?? ?? ?? ?? ff 24 85 ?? ?? ?? ?? ff 75 0c e8 ?? ?? ?? ?? 83 c4 04 5d c3 ff 75 0c e8 ?? ?? ?? ?? 83 c4 04 5d c3");
	
	char *x_ref_count_call = (char *)Pocket::Sigscan::FindPattern(BYONDCORE, "3D ?? ?? ?? ?? 74 14 50 E8 ?? ?? ?? ?? FF 75 0C FF 75 08 E8", 20);
	TRUE_OR_DIE(x_ref_count_call)
	DecRefCount = (DecRefCountPtr)(x_ref_count_call + *(int *)x_ref_count_call + 4); //x_ref_count_call points to the relative offset to DecRefCount from the call site
	x_ref_count_call = (char *)Pocket::Sigscan::FindPattern(BYONDCORE, "FF 75 10 E8 ?? ?? ?? ?? FF 75 0C 8B F8 FF 75 08 E8 ?? ?? ?? ?? 57", 17);
	TRUE_OR_DIE(x_ref_count_call)
	IncRefCount = (IncRefCountPtr)(x_ref_count_call + *(int *)x_ref_count_call + 4);
	//TRUE_OR_DIE(get_appearance_ptr)
	int GetObjAppearance = (int)((int)GetAppearance + *(int*)((int)GetAppearance + 39) + 43);
	int GetMobAppearance = (int)((int)GetAppearance + *(int*)((int)GetAppearance + 52) + 56);

	//obj_table = std::make_unique<RefTable<Obj>>(Pocket::Sigscan::FindPattern(BYONDCORE, "55 8b ec 56 8b 75 08 6a 02 56 e8 ?? ?? ?? ?? ff 75 10 ff 75 0c 52 50 e8 ?? ?? ?? ?? 83 c4 18 84 c0 75 5a 3b 35 ?? ?? ?? ?? 73 52 a1 ?? ?? ?? ??"), 44, 37);
	//mob_table = std::make_unique<RefTable<Mob>>(Pocket::Sigscan::FindPattern(BYONDCORE, "55 8b ec 56 8b 75 08 6a 03 56 e8 ?? ?? ?? ?? ff 75 10 ff 75 0c 52 50 e8 ?? ?? ?? ?? 83 c4 18 84 c0 75 5a 3b 35 ?? ?? ?? ?? 73 52 a1 ?? ?? ?? ??"), 44, 37);
	obj_table = std::make_unique<RefTable<Obj>>((void*)GetObjAppearance, 16, 9);
	mob_table = std::make_unique<RefTable<Mob>>((void*)GetMobAppearance, 16, 9);
	void* del_datum_ptr = Pocket::Sigscan::FindPattern(BYONDCORE, "?? ?? ?? ?? 73 55 A1 ?? ?? ?? ?? 8B 04 88 85 C0 74 49 8B 50 18 81 FA 00 00 00 70");
	TRUE_OR_DIE(del_datum_ptr);
	datum_table = std::make_unique<RefTable<Datum>>((void*)del_datum_ptr, 7, 0);
	client_table = std::make_unique<RefTable<Client, unsigned short>>((void*)AddToScreen, 21, 11);
	image_table = std::make_unique<RefTable<ImageOverlay>>((void*)GetVisContents, 0x86, 0x7F);
	appearance_table = *(AppearanceTable***)Pocket::Sigscan::FindPattern(BYONDCORE, "55 8b ec a1 ?? ?? ?? ?? 8b 4d 08 3b 48 44 73 08 8b 40 40 8b 04 88 5d c3", 4);
	void* get_id_list_ptr = Pocket::Sigscan::FindPattern(BYONDCORE, "55 8b ec 83 ec 0c 53 56 57 ff 75 08 e8 ?? ?? ?? ?? 83 c4 04 85 c0", 13);
	TRUE_OR_DIE(get_id_list_ptr)
	void* get_id_list = (void*)(*(int*)get_id_list_ptr + (int)get_id_list_ptr + 4);
	appearance_list_table = *(TableHolder2**)((int)get_id_list + 19);
	char* get_shared_turf_ptr = (char*)Pocket::Sigscan::FindPattern(BYONDCORE, "55 8b ec 8b 4d 08 3b 0d ?? ?? ?? ?? 73 12 a1 ?? ?? ?? ?? 8b 0c 88 a1 ?? ?? ?? ?? 8b 04 88 5d c3 33 c0 5d c3");
	TRUE_OR_DIE(get_shared_turf_ptr)
	turf_table = *(int***)(get_shared_turf_ptr + 15);
	turf_existence_table = ((int)turf_table + 4);
	world_size = (WorldSizeHolder*)((int)turf_table + 8);
	turf_shared_info_table = *(TurfSharedInfo****)(get_shared_turf_ptr + 23);
	turf_hashtable = *(Turf****)Pocket::Sigscan::FindPattern(BYONDCORE, "55 8b ec a1 ?? ?? ?? ?? 8b 55 08 23 c2 0f b7 c8 a1 ?? ?? ?? ??", 17);
	turf_hashtable_mask = (int)(turf_hashtable + 8);
	TRUE_OR_DIE(turf_hashtable);
#else
	FIND_OR_DIE(GetStringTableEntry, "55 89 E5 83 EC 18 8B 45 ?? 39 05 ?? ?? ?? ?? 76 ?? 8B 15 ?? ?? ?? ?? 8B 04 ??");
	FIND_OR_DIE(ToString, "55 89 E5 83 EC 68 A1 ?? ?? ?? ?? 8B 15 ?? ?? ?? ?? 8B 0D ?? ?? ?? ?? 89 5D ??");
	FIND_OR_DIE(GetVariable, "55 89 E5 81 EC ?? ?? ?? ?? 8B 55 ?? 89 5D ?? 8B 5D ?? 89 75 ?? 8B 75 ??");
	FIND_OR_DIE(GetAssocElement, "55 89 E5 83 EC ?? 89 4D C4 B9 7B 00 00 00 89 5D F4 89 D3 89 75 F8 89 C6 89 D0 8B 55 C4 89 7D FC C7 44 24 04 00 00 00 00 C7 04 24 00 00 00 00 E8 ?? ?? ?? ?? 3D FF FF 00 00");
	FIND_OR_DIE(DecRefCount, "55 89 E5 83 EC ?? 8B 45 08 89 5D ?? 8B 5D 0C 89 75 ?? 3C 54");
	FIND_OR_DIE(IncRefCount, "55 89 E5 83 EC ?? 8B 55 08 8b 45 0C 80 FA 54 76 07 c9 c3 90");
	FIND_OR_DIE(SetAppearance, "55 89 e5 83 ec 38 3c 54 89 5d f4 89 cb");
	FIND_OR_DIE(SetPixelX, "55 89 e5 57 56 53 81 ec 4c 01 00 00 8b 75 08 8b 5d 0c 0f b7 7d 10");
	FIND_OR_DIE(SetPixelY, "55 89 e5 57 56 53 81 ec 6c 01 00 00 8b 75 08 8b 5d 0c 0f b7 7d 10");
	FIND_OR_DIE(SetPixelW, "55 89 e5 81 ec 38 01 00 00 89 7d fc 8b 7d 08 89 5d f4 0f b7 5d 10 89 75 f8 8b 75 0c 89 f8 3c 03 0f 84 d2 01 00 00 3c 02 0f 84 f2 00 00 00 89 3c 24 89 74 24 04 e8 ?? ?? ?? ?? 8b 15 ?? ?? ?? ?? 89 85 f0 fe ff ff 8b 8d f0 fe ff ff 31 c0 3b 4a 44 0f 82 b9 01 00 00 8d 8d 04 ff ff ff 89 44 24 04 89 0c 24 e8 ?? ?? ?? ?? 89 d8 66 2b 85 5c ff ff ff");
	FIND_OR_DIE(SetPixelZ, "55 89 e5 81 ec 38 01 00 00 89 7d fc 8b 7d 08 89 5d f4 0f b7 5d 10 89 75 f8 8b 75 0c 89 f8 3c 03 0f 84 d2 01 00 00 3c 02 0f 84 f2 00 00 00 89 3c 24 89 74 24 04 e8 ?? ?? ?? ?? 8b 15 ?? ?? ?? ?? 89 85 f0 fe ff ff 8b 8d f0 fe ff ff 31 c0 3b 4a 44 0f 82 b9 01 00 00 8d 8d 04 ff ff ff 89 44 24 04 89 0c 24 e8 ?? ?? ?? ?? 89 d8 66 2b 85 5e ff ff ff");
	FIND_OR_DIE(SetMovableDir, "55 89 e5 81 ec 38 01 00 00 8b 45 08 89 75 f8 0f b6 55 10 89 7d fc 8b 7d 0c 89 5d f4 3c 03 89 c6 89 85 f4 fe ff ff");
	FIND_OR_DIE(SetLoc, "55 89 e5 83 ec 38 89 5d f4 8b 45 0c 89 75 f8 8b 5d 10 8b 75 14 89 7d fc 0f b6 7d 08 89 45 e4 89 1c 24 89 74 24 04");
	FIND_OR_DIE(SpliceAppearance, "55 89 e5 83 ec 18 89 5d f8 8b 5d 08 89 75 fc 8b 75 0c 8b 4b 38 89 f0 d3 e8 3b 43 28 89 c1");
	FIND_OR_DIE(SpliceString, "55 89 e5 57 56 53 83 ec 3c 8b 45 08 89 04 24 e8 ?? ?? ?? ?? 85 c0 89 c7");
	FIND_OR_DIE(ChangeTurf, "55 89 e5 57 56 53 81 ec 8c 00 00 00 8b 55 18 8b 45 14 89 54 24 04 89 04 24");
	FIND_OR_DIE(Output, "55 89 e5 81 ec 98 01 00 00 89 5d f4 89 c3 8b 45 0c 80 fb 23 8b 4d 08 89 75 f8 8b 75 10 89 7d fc 8b 7d 14 89 95 94 fe ff ff 89 85 a4 fe ff ff");
	FIND_OR_DIE(GetVisContents, "55 89 e5 83 ec 08 8b 55 08 8b 45 0c 0f b6 4d 10 80 fa 54 76 0b 31 c0 c9 c3");
	FIND_OR_DIE(AddToScreen, "55 89 e5 83 ec 48 89 75 f8 0f b7 45 10 8b 75 08 66 3b 05 ?? ?? ?? ?? 89 7d fc 8b 7d 0c 89 5d f4 89 f2");
	FIND_OR_DIE(RemoveFromScreen, "55 89 e5 53 83 ec 14 0f b7 45 10 66 3b 05 ?? ?? ?? ??");
	FIND_OR_DIE(DelFilter, "55 89 e5 83 ec 18 89 5d f8 8b 5d 08 89 75 fc 8b 75 0c 8b 4b 6c 89 f0 d3 e8 3b 43 5c 89 c1");
	FIND_OR_DIE(SendMapsClient, "55 0f b7 c0 89 e5 57 56 53 81 ec ?? ?? ?? ?? 89 04 24 89 85 ?? ?? ?? ?? e8 ?? ?? ?? ?? 85 c0 89 45 84 0f");
	FIND_OR_DIE(CreateObj, "55 89 e5 57 56 53 83 ec 4c a1 ?? ?? ?? ?? 8b 7d 10 85 c0 0f 84 ?? ?? ?? ??");
	FIND_OR_DIE(CreateMob, "55 89 e5 57 56 31 f6 53 83 ec 5c 8b 45 10 3b 05 ?? ?? ?? ??");
	FIND_OR_DIE(GetAppearance, "55 89 e5 53 83 ec 24 8b 45 08 8b 55 0c 3c 50");

	void* datum_table_ptr = Pocket::Sigscan::FindPattern(BYONDCORE, "8B 45 08 3B 05 ?? ?? ?? ?? 73 2C 8B 15 ?? ?? ?? ?? 8B 0C 82 85 C9 74 1F 8B 51 ??");
	TRUE_OR_DIE(datum_table_ptr);
	datum_table = std::make_unique<RefTable<Datum>>(datum_table_ptr, 13, 5);
	client_table = std::make_unique<RefTable<Client, unsigned short>>((void*)AddToScreen, 0x26, 0x13);
	obj_table = std::make_unique<RefTable<Obj>>((void*)GetVisContents, 0x7a, 0x72);
	mob_table = std::make_unique<RefTable<Mob>>((void*)GetVisContents, 0x5a, 0x52);
	image_table = std::make_unique<RefTable<ImageOverlay>>((void*)GetVisContents, 0x3a, 0x32);
	void* appearance_table_ptr = Pocket::Sigscan::FindPattern(BYONDCORE, "a1 ?? ?? ?? ?? 8b 04 b0 85 c0 89 45 e4 0f 84 ?? ?? ?? ?? a1 ?? ?? ?? ?? 8b 48 44", 20);
	TRUE_OR_DIE(appearance_table_ptr);
	appearance_table = *(AppearanceTable***)appearance_table_ptr;
	void* appearance_list_ptr = Pocket::Sigscan::FindPattern(BYONDCORE, "c7 46 3c ff ff 00 00 a1 ?? ?? ?? ?? 39 45 e0 73 50 8b 15 ?? ?? ?? ??", 19);
	TRUE_OR_DIE(appearance_list_ptr);
	appearance_list_table = *(TableHolder2**)appearance_list_ptr;

	turf_table = *(int***)((int)ChangeTurf + 0x2a);
	world_size = *(WorldSizeHolder**)((int)ChangeTurf + 0x22);
	turf_shared_info_table = *(TurfSharedInfo****)((int)ChangeTurf + 0x32);
	turf_hashtable = *(Turf****)((int)GetVisContents + 0xb9);
	turf_hashtable_mask = *(unsigned short**)((int)GetVisContents + 0xb3);
	turf_existence_table = *(unsigned char***)((int)GetVisContents + 0xa2);

	// i love inlining i love inlining
	animate_start_call = (unsigned int*)Pocket::Sigscan::FindPattern(BYONDCORE, "e8 ?? ?? ?? ?? 8b b5 ?? ?? ?? ?? 8b bd ?? ?? ?? ?? 89 34 24 89 7c 24 04 e8 ?? ?? ?? ?? 89 c7 31 f6 8d 85 ?? ?? ?? ??", 1);
	TRUE_OR_DIE(animate_start_call);
	AnimateStartFun = (AnimateStartFunPtr)RELATIVE_CALL_RESOLVE(animate_start_call);
	animate_end_jump = (unsigned int*)Pocket::Sigscan::FindPattern(BYONDCORE, "89 7c 24 04 e8 ?? ?? ?? ?? a1 ?? ?? ?? ?? 8b 0d ?? ?? ?? ?? 8b 1d ?? ?? ?? ?? 0f b7 50 42 8b 78 3c 0f b7 f2 83 c2 01 89 0c f7 89 5c f7 04 66 89 50 42 a1 ?? ?? ?? ?? 0f b7 50 42 8b 40 3c 8d 54 d0 f8 8b 02 8b 52 04 89 04 24 89 54 24 04 e8 ?? ?? ?? ?? 8b 3d ?? ?? ?? ?? e9 ?? ?? ?? ??", 90);
	TRUE_OR_DIE(animate_end_jump);
	original_animate_end_jump = (unsigned int)RELATIVE_CALL_RESOLVE(animate_end_jump);

#endif
	return true;
}