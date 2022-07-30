#pragma once

#include <string>
#include "byond_constants.h"

#ifdef _WIN32
#define REGPARM3
#define REGPARM2
#else
#define REGPARM3 __attribute__((regparm(3)))
#define REGPARM2 __attribute__((regparm(2)))
#endif

struct String
{
	char* stringData;
	int unk1;
	int unk2;
	unsigned int refcount;
};

struct Value
{
	char type;
	union
	{
		int value;
		float valuef;
	};
};

struct TableHolder2
{
	void** elements;
	unsigned int length;
};

template<class T>
struct RefTable
{
	RefTable(T*** e, unsigned int* l) : elements(*e), length(*l) {}
	RefTable(TableHolder2* th) : elements(*(T***)&th->elements), length(th->length) {}
	RefTable(void* base, int elements_offset, int length_offset) : elements(**(T****)((int)base+elements_offset)), length(**(unsigned int**)((int)base + length_offset)) {}
	RefTable() : elements(dummy_elements), length(dummy_length) {}
	T**& elements;
	unsigned int& length;
private:
	T** dummy_elements = nullptr;
	unsigned int dummy_length = 0;
};

struct TableHolder3
{
	void* elements;
	std::uint32_t size;
	std::uint32_t capacity;
	TableHolder3* next; //probably?
	char unknown[8];
};

struct VarListEntry
{
	std::uint32_t unknown;
	std::uint32_t name_id;
	Value value;
};

struct Obj
{
	Value loc; // 0
	char unknown[8]; // 8
	short bound_x; // 10
	short bound_y; // 12
	short bound_width; // 14
	short bound_height; // 16
	float step_x; // 18
	float step_y; // 1c
	float step_size; // 20
	short pixel_x; // 24
	short pixel_y; // 26
	short pixel_w; // 28
	short pixel_z; // 2a
	void* some_other_linked_list;
	VarListEntry* modified_vars; // 30
	std::uint16_t modified_vars_count;
	std::uint16_t modified_vars_capacity;
	char unknown2[4]; // 38
	void* some_linked_list; //3c
	char unknown3[16];
	TableHolder3* vis_contents; // 50
	TableHolder3* vis_locs; // 54
	char unknown4[12];
	int appearance; // 64
	int appearance2; // 68
	int appearance3; // 6c
	char unknown5[64];
};

struct Datum
{
	std::uint32_t type;
	VarListEntry* modified_vars;
	std::uint16_t modifier_vars_count;
	std::uint16_t modified_vars_capacity;
	std::uint32_t flags;
	std::uint32_t refcount;
};

struct Mob
{
	Value loc; // 0
	char unknown[8]; // 8
	short bound_x; // 10
	short bound_y; // 12
	short bound_width; // 14
	short bound_height; // 16
	float step_x; // 18
	float step_y; // 1c
	float step_size; // 20
	short pixel_x; // 24
	short pixel_y; // 26
	short pixel_w; // 28
	short pixel_z; // 2a
	void* some_other_linked_list;
	VarListEntry* modified_vars; // 30
	std::uint16_t modified_vars_count;
	std::uint16_t modified_vars_capacity;
	char unknown2[4]; // 38
	void* some_linked_list; //3c
	char unknown3[16];
	TableHolder3* vis_contents; // 50
	TableHolder3* vis_locs; // 54
	char unknown4[12];
	int appearance; // 64
	int appearance2; // 68
	int appearance3; // 6c
	char unknown5[0x4C];
	void* unknown_list3;
	char unknown6[0x10];
};

struct Turf { // According to lummox, this struct also includes info about atoms overhanging and animations too
	int id;
	Turf* next;
	int obj_contents;
	int mob_contents;
	int unk_10;
	TableHolder3* vis_contents; // vis_contents
	TableHolder3* vis_locs;
	int unk_1c;
	int unk_20;
	int unk_24;
	int unk_28;
	int unk_2c;
};
struct TurfVars {
	int id;
	TurfVars* next;
	VarListEntry* modified_vars;
	std::uint16_t modified_vars_count;
	std::uint16_t modified_vars_capacity;
};

struct TurfSharedInfo {
	int typepath_id;
	int appearance;
	int area;
	int unk_0c;
	short unk_10;
	short unk_12;
	int unk_14;
	short unk_18;
	short unk_1a;
	int unk_1c;
};

struct TurfTableHolder {
	int* shared_info_id_table;
	unsigned char* existence_table;
	int turf_count;
	int maxx;
	int maxy;
	int maxz;
};
struct TurfHashtableHolder {
	Turf** elements;
	int size;
	int mask;
};

enum class AppearanceRbtColor : bool
{
	Black = true,
	Red = false,
};

struct Appearance
{
	Appearance* left_node; // 0
	Appearance* right_node; // 4
	Appearance* parent_node; // 8
	Appearance* prev_node; // c
	Appearance* next_node; // 10
	int id; // 14
	AppearanceRbtColor node_color; // 18
	int name_str; // 1c
	int desc_str; // 20
	int suffix_str; // 24
	int screen_loc_str; // 28
	int text_str; // 2c
	int icon_res; // 30
	int icon_state_str; // 34
	int overlays_list; // 38
	int underlays_list; // 3c
	int verbs_list; // 40
	int unk_44;

	unsigned long long opacity : 1; // 48
	unsigned long long density : 1;
	unsigned long long unk_48_4 : 4;
	unsigned long long gender : 2; // 0=neuter, 0=male, 0=female, 0=plural

	unsigned long long mouse_drop_zone : 1; // 49
	unsigned long long dir_override : 1; // internal flag for whether dir is inherited or not
	unsigned long long unk_49_4 : 2;
	unsigned long long mouse_opacity : 2;
	unsigned long long animate_movement : 2; // add one and bitwise-and 3 to get actual value
	
	unsigned long long unk_4a : 2; // 4a
	unsigned long long override : 1;
	unsigned long long unk_4a_8 : 2;
	
	unsigned long long appearance_flags : 12;
	
	unsigned char dir; // 50
	unsigned char invisibility;
	unsigned char infra_luminosity;
	unsigned char luminosity;
	
	short pixel_x; // 54
	short pixel_y;
	
	short pixel_w; // 58
	short pixel_z;
	
	float glide_size; // 5c
	float layer; // 60?
	int maptext_str; // 64?
	
	short maptext_x; // 68?
	short maptext_y;
	
	short maptext_width; // 6c?
	short maptext_height;
	
	Value mouse_over_pointer; // 70
	Value mouse_drag_pointer; // 78
	Value mouse_drop_pointer; // 80
	int unk_88; // an appearance
	int unk_8c; // an appearance
	int unk_90; // an appearance
	
	float transform[6]; // 94
	
	union { // ac
		struct {
			unsigned char color_r;
			unsigned char color_g;
			unsigned char color_b;
			unsigned char alpha;
		};
		unsigned int color_alpha;
	};
	
	
	unsigned char blend_mode; // b0
	unsigned char matrix_flag; // b1
	short plane; // b2
	
	int unk_b4; // probably filters imo
	int render_source_str; // b8
	int render_target_str; // bc
	
	unsigned short vis_flags; // c0
	short unk_c2;
	
	int unk_c4;
	int unk_c8;
	int unk_cc;
	int unk_d0;
	int unk_d4;
	int unk_d8;
	int unk_dc;
	float *color_matrix; // e0
	int refcount; // e4
};

struct AppearanceTable
{
	char unk[0x40];
	Appearance** elements;
	int length;
};

struct AppearanceList // used for overlays, underlays, etc
{
	short len;
	int unk;
	int* ids;
};