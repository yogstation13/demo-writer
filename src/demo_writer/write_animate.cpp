#include "write_animate.h"
#include "write_appearance.h"
#include "write_primitive.h"
#include "demo_writer.h"
#include <vector>

struct AnimateData {
	Value object;
	int appearance_before;
	int appearance_after;
	int flags;
	unsigned char easing;
	int loop;
	float time;
	bool did_redefine;
};

std::vector<AnimateData> animate_structs;

void flush_animate() {
	if (animate_structs.empty()) return;
	update_demo_time();

	std::vector<unsigned char> buf;
	for (int i = 0; i < animate_structs.size(); i++) {
		AnimateData &data = animate_structs[i];
		write_primitive(buf, ref_int(data.object));
		write_appearance(buf, data.appearance_before);
		unsigned char animation_flags = 0;
		if (data.flags & 1) animation_flags |= 1;
		if (data.flags & 4) animation_flags |= 2;
		write_primitive(buf, animation_flags);
		unsigned short loop = data.loop;
		if (data.loop == 0) loop = 1;
		if (data.loop < 0) loop = 0;
		write_primitive(buf, loop);
		int animation_end = i + 1;
		for (int j = i + 1; j < animate_structs.size(); j++) {
			if (!animate_structs[j].did_redefine && !(animate_structs[j].flags & 4)) {
				animation_end = j + 1;
			}
			else {
				break;
			}
		}
		write_vlq(buf, animation_end - i);
		for (; i < animation_end; i++) {
			AnimateData &step_data = animate_structs[i];
			write_appearance(buf, step_data.appearance_after);
			write_primitive(buf, step_data.time);
			write_primitive(buf, step_data.easing);
			unsigned char step_flags = 0;
			if (data.flags & 2) step_flags |= 1;
			write_primitive(buf, step_flags);
			DecRefCount({ APPEARANCE, {step_data.appearance_before} });
			DecRefCount({ APPEARANCE, {step_data.appearance_after} });
		}
		i--;
	}
	animate_structs.clear();

	demo_file_handle.put(0x7);
	write_vlq(buf.size());
	demo_file_handle.write((char*)&buf[0], buf.size());
}

Value last_animate_obj = { 0, {0} };

#ifndef _WIN32
// I sure love trying to hook into the middle of a function
Value animate_args;
Value last_animate_appearance;

char animate_end_trampoline[] = {
	(char)0xE8, 0, 0, 0, 0,
	(char)0xE9, 0, 0, 0, 0
};

void pre_animate_hook(void *some_struct, Value args) {
	DecRefCount(animate_args);
	animate_args = args;
	IncRefCount(animate_args);
	DecRefCount(last_animate_appearance);
	last_animate_appearance = GetVariable(last_animate_obj, 0x106);
	IncRefCount(last_animate_appearance);
	AnimateStartFun(some_struct, args);
}
#endif

#ifdef _WIN32
#define ANIMATE_ABORT return oAnimate(animate_args)
AnimatePtr oAnimate;
void hAnimate(Value animate_args) {
#else
#define ANIMATE_ABORT return
void animate_hook() {
#endif
	if (animate_args.type != LIST) ANIMATE_ABORT;
	AnimateData data;
	Value len_value = GetVariable(animate_args, 0x39);
	int len = len_value.valuef;
	Value firstArg = { NULL_D, {0} };
	if (len) firstArg = Core::DecReturn(GetAssocElement(animate_args, ValueFloat(1.0f)));
	bool did_redefine = false;
	if (firstArg.type == MOB || firstArg.type == OBJ || firstArg.type == TURF || firstArg.type == IMAGE) {
		last_animate_obj = firstArg;
		did_redefine = true;
	}
	else if (firstArg.type != STRING && firstArg.type != NUMBER) ANIMATE_ABORT;
	if (last_animate_obj.type == NULL_D) ANIMATE_ABORT;
	
	data.object = last_animate_obj;
	data.did_redefine = did_redefine;
#ifdef _WIN32
	data.appearance_before = GetVariable(last_animate_obj, 0x106).value;
#else
	data.appearance_before = last_animate_appearance.value;
#endif
	bool did_time = false, did_loop = !did_redefine, did_easing = false, did_flags = false;
	Value time_value, loop_value, easing_value, flags_value;
	data.time = 0; data.loop = 0; data.flags = 0; data.easing = 0;
	time_value = GetAssocElement(animate_args, { STRING, {0x4F} });
	loop_value = GetAssocElement(animate_args, { STRING, {0xF4} });
	easing_value = GetAssocElement(animate_args, { STRING, {0xFD} });
	flags_value = GetAssocElement(animate_args, { STRING, {0x10f} });
	if (time_value.type == NUMBER) { data.time = time_value.valuef; did_time = true; }
	if (loop_value.type == NUMBER) { data.loop = loop_value.valuef; did_loop = true; }
	if (easing_value.type == NUMBER) { data.easing = easing_value.valuef; did_easing = true; }
	if (flags_value.type == NUMBER) { data.flags = flags_value.valuef; did_flags = true; }
	for (int i = (did_redefine ? 1 : 0); i < len; i++) {
		Value item = Core::DecReturn(GetAssocElement(animate_args, ValueFloat((float)i+1)));
		if (item.type == NUMBER) {
			if (!did_time) { did_time = true; data.time = item.valuef; }
			if (!did_loop) { did_loop = true; data.loop = item.valuef; }
			if (!did_easing) { did_easing = true; data.easing = item.valuef; }
			if (!did_flags) { did_flags = true; data.flags = item.valuef; }
		}
	}

#ifdef _WIN32
	oAnimate(animate_args);
#endif
	
	data.appearance_after = GetVariable(last_animate_obj, 0x106).value;

	IncRefCount({ APPEARANCE, {data.appearance_before} });
	IncRefCount({ APPEARANCE, {data.appearance_after} });
	animate_structs.push_back(data);
}

extern "C" void *subhook_unprotect(void *address, size_t size);

#ifdef _WIN32
void make_animate_hook() {
	oAnimate = Core::install_hook(Animate, hAnimate);
}
#else
void make_animate_hook() {
	subhook_unprotect((void*)animate_end_trampoline, 10);
	subhook_unprotect((void*)animate_start_call, 4);
	subhook_unprotect((void*)animate_end_jump, 4);

	RELATIVE_CALL_SET(&animate_end_trampoline[1], animate_hook);
	RELATIVE_CALL_SET(&animate_end_trampoline[6], original_animate_end_jump);
	RELATIVE_CALL_SET(animate_end_jump, animate_end_trampoline);

	RELATIVE_CALL_SET(animate_start_call, pre_animate_hook);
}
#endif
