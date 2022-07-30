#include "demo_writer.h"
#include "write_primitive.h"
#include "write_objects.h"
#include "state_tracking.h"
#include "../core/core.h"
#include <stdio.h>

std::ofstream demo_file_handle;

Value parse_ref(const char* ref) {
	Value val;
	val.type = 0;
	val.value = 0;

	unsigned int refy_bit = 0;
	if(ref[0] != '[' || ref[1] != '0' || ref[2] != 'x') return val;
	ref += 3;
	while(*ref != ']' && *ref != 0) {
		if(*ref >= '0' && *ref <= '9') {
			refy_bit = (refy_bit << 4) | (*ref - '0');
		} else if(*ref >= 'a' && *ref <= 'f') {
			refy_bit = (refy_bit << 4) | (*ref - 'a' + 0xa);
		} else if(*ref >= 'A' && *ref <= 'F') {
			refy_bit = (refy_bit << 4) | (*ref - 'A' + 0xa);
		}
		ref++;
	}
	if(ref[0] != ']' || ref[1] != 0) return val;
	val.type = (refy_bit >> 24);
	val.value = (refy_bit & 0xFFFFFF);
	return val;
}


const char* pixel_things[] = { "pixel_x", "pixel_y", "pixel_w", "pixel_z" };

#define SET_PIXEL_HOOK(X) SetPixel##X##Ptr oSetPixel##X; void hSetPixel##X(Value atom, short pixel_x) {mark_atom_dirty(atom); oSetPixel##X (atom, pixel_x);}
/*SetPixelXPtr oSetPixelX;
void hSetPixel(Value atom, short pixel_x) {
	mark_atom_dirty(atom);
	oSetPixelX(atom, pixel_x);
}*/
SET_PIXEL_HOOK(X)
SET_PIXEL_HOOK(Y)
SET_PIXEL_HOOK(W)
SET_PIXEL_HOOK(Z)

SetMovableDirPtr oSetMovableDir;
void hSetMovableDir(Value atom, unsigned char dir) {
	mark_atom_dirty(atom);
	oSetMovableDir(atom, dir);
}

SetLocPtr oSetLoc;
void hSetLoc(Value atom, Value loc) {
	mark_atom_dirty(atom);
	oSetLoc(atom, loc);
}

SetAppearancePtr oSetAppearance;
void hSetAppearance(Value atom, int appearance) {
	oSetAppearance(atom, appearance);
	mark_atom_dirty(atom);
}

SpliceAppearancePtr oSpliceAppearance;
void __fastcall hSpliceAppearance(void* this_, int edx, int appearance) {
	get_demo_id_flags(appearance).appearance_written = false;
	oSpliceAppearance(this_, edx, appearance);
}

SpliceStringPtr oSpliceString;
void hSpliceString(unsigned int id) {
	get_demo_id_flags(id).string_written = false;
	oSpliceString(id);
}

AllocObjIdPtr oAllocObjId;
int hAllocObjId() {
	int id = oAllocObjId();
	mark_atom_dirty({ OBJ, {id} });
	return id;
}

AllocMobIdPtr oAllocMobId;
int hAllocMobId() {
	int id = oAllocMobId();
	mark_atom_dirty({ MOB, {id} });
	return id;
}

ChangeTurfPtr oChangeTurf;
void hChangeTurf(int a, int b, int type, Value turf) {
	oChangeTurf(a, b, type, turf);
	mark_atom_dirty(turf);
}

char return_buf[64];

bool demo_started = false;
extern "C" EXPORT const char* demo_start(int n_args, const char** args)
{
	if (demo_started) return "Demo already started";
	if (n_args < 2) return "Missing arguments";

	const char* out_file = args[0];
	const char* revdata = args[1];

	demo_file_handle.open(out_file, std::ios::binary | std::ios::trunc);
	if (demo_file_handle.fail()) {
		return "Failed to open file";
	}

	if (!(Core::find_functions()))
	{
		return "Couldn't find functions";
	}

	oSetAppearance = Core::install_hook(SetAppearance, hSetAppearance);
	oSpliceAppearance = Core::install_hook(SpliceAppearance, hSpliceAppearance);
	oSpliceString = Core::install_hook(SpliceString, hSpliceString);
	//oFlick = Core::install_hook(Flick, hFlick);
	//oAnimate = Core::install_hook(Animate, hAnimate);
	oSetPixelX = Core::install_hook(SetPixelX, hSetPixelX);
	oSetPixelY = Core::install_hook(SetPixelY, hSetPixelY);
	oSetPixelW = Core::install_hook(SetPixelW, hSetPixelW);
	oSetPixelZ = Core::install_hook(SetPixelZ, hSetPixelZ);
	oSetMovableDir = Core::install_hook(SetMovableDir, hSetMovableDir);
	oSetLoc = Core::install_hook(SetLoc, hSetLoc);
	oAllocObjId = Core::install_hook(AllocObjId, hAllocObjId);
	oAllocMobId = Core::install_hook(AllocMobId, hAllocMobId);
	oChangeTurf = Core::install_hook(ChangeTurf, hChangeTurf);

	demo_file_handle.put(0xCB);
	demo_file_handle.put(0x0); // version number
	demo_file_handle.put(0x0);

	while (*revdata != 0) {
		demo_file_handle.put(*revdata);
		revdata++;
	}
	demo_file_handle.put(0x0);
	std::streampos demo_loadsize_ptr = demo_file_handle.tellp();
	demo_file_handle.put(0);
	demo_file_handle.put(0);
	demo_file_handle.put(0);
	demo_file_handle.put(0);

	demo_time_override_enabled = true;
	demo_time_override = 0;

	write_world_size();
	flush_atom_updates();

	demo_time_override_enabled = false;

	std::streampos end_ptr = demo_file_handle.tellp();
	unsigned int loadsize = end_ptr;
	demo_file_handle.seekp(demo_loadsize_ptr);
	demo_file_handle.write((const char*)&loadsize, 4);
	demo_file_handle.seekp(end_ptr);
	demo_file_handle.flush();

	demo_started = true;
	return "SUCCESS";
}

extern "C" EXPORT const char* demo_embed_resource(int n_args, const char** args) {
	if (n_args < 2 || !demo_started) return "FAIL";
	Value resource = parse_ref(args[0]);
	if (resource.type != RESOURCE || resource.value == 0xFFFF) return "FAIL";
	std::ifstream file(args[1], std::ios::binary | std::ios::ate);
	unsigned int size = file.tellg();
	file.seekg(0, std::ios::beg);
	std::vector<char> buf(size);
	file.read(buf.data(), size);
	if (file.good()) {
		file.close();
		demo_file_handle.put(0xA);
		write_vlq(4 + buf.size());
		demo_file_handle.write((char*)&resource.value, 4);
		demo_file_handle.write((char*)&buf[0], buf.size());
		return "SUCCESS";
	}
	file.close();
	return "FAIL";
}

extern "C" EXPORT const char* demo_end(int n_args, const char** args) {
	if (!demo_started) return "";
	flush_atom_updates();
	demo_file_handle.close();
	demo_started = false;
	Core::remove_all_hooks();
	return "";
}

extern "C" EXPORT const char* demo_flush(int n_args, const char** args) {
	if (!demo_started) return "0";
	flush_atom_updates();
	demo_file_handle.flush();
	int size = demo_file_handle.tellp();
	snprintf(return_buf, sizeof(return_buf), "%d", size);
	return return_buf;
}

extern "C" EXPORT const char* demo_get_size(int n_args, const char** args) {
	if (!demo_started) return "0";
	int size = demo_file_handle.tellp();
	snprintf(return_buf, sizeof(return_buf), "%d", size);
	return return_buf;
}
