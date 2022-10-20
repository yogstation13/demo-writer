#include "demo_writer.h"
#include "write_primitive.h"
#include "write_objects.h"
#include "write_animate.h"
#include "write_clients.h"
#include "state_tracking.h"
#include "../core/core.h"
#include <stdio.h>
#include <stdlib.h>

std::ofstream demo_file_handle;

Value parse_ref(const char* ref, const char **next = nullptr) {
	Value val;
	val.type = 0;
	val.value = 0;
	if (next) *next = nullptr;

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
	if (next && ref[0] == ']' && ref[1] == ',') {
		ref += 2;
		*next = ref;
	} else if (ref[0] != ']' || ref[1] != 0) return val;
	val.type = (refy_bit >> 24);
	val.value = (refy_bit & 0xFFFFFF);
	return val;
}

#define HOOK_DEF(type, name) name##Ptr o##name; type h##name
#define INSTALL_HOOK(name) o##name = Core::install_hook(name, h##name)

const char* pixel_things[] = { "pixel_x", "pixel_y", "pixel_w", "pixel_z" };

#define SET_PIXEL_HOOK(X) HOOK_DEF(void, SetPixel##X)(Value atom, short pixel_x) {mark_atom_dirty(atom); oSetPixel##X (atom, pixel_x);}
/*SetPixelXPtr oSetPixelX;
void hSetPixel(Value atom, short pixel_x) {
	mark_atom_dirty(atom);
	oSetPixelX(atom, pixel_x);
}*/
SET_PIXEL_HOOK(X)
SET_PIXEL_HOOK(Y)
SET_PIXEL_HOOK(W)
SET_PIXEL_HOOK(Z)

HOOK_DEF(void, SetMovableDir)(Value atom, unsigned char dir) {
	mark_atom_dirty(atom);
	oSetMovableDir(atom, dir);
}

HOOK_DEF(void, SetLoc)(Value atom, Value loc) {
	mark_atom_dirty(atom);
	oSetLoc(atom, loc);
}

HOOK_DEF(void, SetAppearance)(Value atom, int appearance) {
	oSetAppearance(atom, appearance);
	mark_atom_dirty(atom);
}

#ifdef _WIN32
HOOK_DEF(void __fastcall, SpliceAppearance)(FASTCALL_THIS, int appearance) {
#else
HOOK_DEF(void, SpliceAppearance)(FASTCALL_THIS, int appearance) {
#endif
	get_demo_id_flags(appearance).appearance_written = false;
	oSpliceAppearance(PASS_FASTCALL_THIS, appearance);
}

#ifdef _WIN32
HOOK_DEF(void _fastcall, DelFilter)(FASTCALL_THIS, int id) {
#else
HOOK_DEF(void, DelFilter)(FASTCALL_THIS, int id) {
#endif
	get_demo_id_flags(id).filter_written = false;
	oDelFilter(PASS_FASTCALL_THIS, id);
}

HOOK_DEF(void, SpliceString)(unsigned int id) {
	get_demo_id_flags(id).string_written = false;
	oSpliceString(id);
}

HOOK_DEF(int, CreateObj)(int p1, int p2, int p3, int p4, int p5, int p6) {
	int id = oCreateObj(p1, p2, p3, p4, p5, p6);
	mark_atom_dirty({ OBJ, {id} });
	return id;
}

HOOK_DEF(int, CreateMob)(int p1, int p2, int p3, int p4, int p5, int p6) {
	int id = oCreateMob(p1, p2, p3, p4, p5, p6);
	mark_atom_dirty({ MOB, {id} });
	return id;
}

HOOK_DEF(void, ChangeTurf)(int a, int b, int type, Value turf) {
	oChangeTurf(a, b, type, turf);
	mark_atom_dirty(turf);
}

HOOK_DEF(void*, GetVisContents)(Value thing, bool modify) {
	void *vis_contents = oGetVisContents(thing, modify);
	/*if (modify) */mark_atom_dirty(thing); // If this gets too performance heavy uncomment the modify bit and find another way to hook vis_contents.Cut()
	return vis_contents;
}

HOOK_DEF(bool, AddToScreen)(Value thing, unsigned short client) {
	update_client_screen(client);
	return oAddToScreen(thing, client);
}
HOOK_DEF(void, RemoveFromScreen)(Value thing, unsigned short client) {
	update_client_screen(client);
	oRemoveFromScreen(thing, client);
}
HOOK_DEF(void, SendMapsClient)(unsigned short client) {
	Client *clientobj = Core::client_table->GetItem(client);
	if (clientobj && clientobj->update_flags & 8) {
		update_client_screen(client);
	}
	oSendMapsClient(client);
}
HOOK_DEF(void REGPARM2, Output)(Value target, Value unk, Value thing) {
	handle_sound_output(target, thing);
	oOutput(target, unk, thing);
}

char return_buf[64];

bool is_restart = false;
bool demo_started = false;
extern "C" EXPORT const char* demo_start(int n_args, const char** args)
{
	if (demo_started) return "Demo already started!";
	if (is_restart) return "Demo restarted without dll reload!";
	if (n_args < 2) return "Missing arguments";

	const char* out_file = args[0];
	const char* revdata = args[1];

	demo_file_handle.open(out_file, std::ios::binary | std::ios::trunc);
	if (demo_file_handle.fail()) {
		return "Failed to open file";
		return "Failed to open file";
	}

	if (!(Core::find_functions()))
	{
		demo_file_handle.close();
		return "Couldn't find functions";
	}

	INSTALL_HOOK(SetAppearance);
	INSTALL_HOOK(SpliceAppearance);
	INSTALL_HOOK(DelFilter);
	INSTALL_HOOK(SpliceString);
	INSTALL_HOOK(SetPixelX);
	INSTALL_HOOK(SetPixelY);
	INSTALL_HOOK(SetPixelW);
	INSTALL_HOOK(SetPixelZ);
	INSTALL_HOOK(SetMovableDir);
	INSTALL_HOOK(SetLoc);
	INSTALL_HOOK(CreateObj);
	INSTALL_HOOK(CreateMob);
	INSTALL_HOOK(ChangeTurf);
	//INSTALL_HOOK(Flick);
	INSTALL_HOOK(GetVisContents);
	INSTALL_HOOK(AddToScreen);
	INSTALL_HOOK(RemoveFromScreen);
	INSTALL_HOOK(SendMapsClient);
	INSTALL_HOOK(Output);
	make_animate_hook();

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
	update_clients();

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
	is_restart = true;
	Core::remove_all_hooks();
	return "";
}

extern "C" EXPORT const char* demo_flush(int n_args, const char** args) {
	if (!demo_started) return "0";
	flush_atom_updates();
	flush_animate();
	update_clients();
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

extern "C" EXPORT const char* demo_set_time_override(int n_args, const char** args) {
	if (n_args < 1 || args[0][0] == 'n') {
		demo_time_override_enabled = false;
		return "";
	}
	demo_time_override = atof(args[0]);
	demo_time_override_enabled = true;
	return "";
}

struct alignas(1) ChatFlags {
	union {
		struct {
			unsigned char send_to_clients : 1;
			unsigned char send_to_world : 1;
			unsigned char send_to_viewer : 1;
			unsigned char send_to_turfs : 1;
			unsigned char is_plain_text : 1;
		};
		unsigned char byte = 0;
	};
};

extern "C" EXPORT const char* demo_chat(int n_args, const char** args) {
	if (n_args < 3) return "FAIL";
	if (!demo_started) return "FAIL";
	std::vector<unsigned short> clients;
	std::vector<unsigned int> turfs;
	bool world = false;
	bool viewer = false;

	const char *target_ref = args[0];
	while (target_ref && *target_ref) {
		if (target_ref[0] == 'd') {
			viewer = true;
			if (target_ref[1] == ',') target_ref += 2;
			else target_ref = nullptr;
			continue;
		}
		Value ref = parse_ref(target_ref, &target_ref);
		if (ref.type == WORLD_D) world = true;
		else if (ref.type == CLIENT) clients.push_back(ref.value);
		else if (ref.type == TURF) turfs.push_back(ref_int(ref));
	}

	if (world) {
		clients.clear();
		viewer = false;
		turfs.clear();
	} else if (viewer) turfs.clear();

	if (!world && !viewer && clients.empty() && turfs.empty()) return "SUCCESS";

	update_demo_time();
	std::vector<unsigned char> buf;
	auto flags = write_primitive(buf, ChatFlags());
	flags->send_to_world = world;
	flags->send_to_viewer = viewer;
	if (!clients.empty()) {
		flags->send_to_clients = true;
		write_vlq(buf, clients.size());
		for (int i = 0; i < clients.size(); i++) write_primitive(buf, clients[i]);
	}
	if (!turfs.empty()) {
		flags->send_to_turfs = true;
		write_vlq(buf, turfs.size());
		for (int i = 0; i < turfs.size(); i++) write_primitive(buf, turfs[i]);
	}
	flags->is_plain_text = args[2][0] == '1';
	write_byond_string(buf, parse_ref(args[1]).value);

	demo_file_handle.put(0x8);
	write_vlq(buf.size());
	demo_file_handle.write((char *)&buf[0], buf.size());

	return "SUCCESS";
}
