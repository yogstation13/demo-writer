#include "write_objects.h"
#include "../core/core.h"
#include "demo_writer.h"
#include "state_tracking.h"
#include "write_primitive.h"
#include "write_appearance.h"
#include <queue>

struct alignas(1) DemoAtomFlags {
	union {
		struct {
			unsigned char include_appearance : 1;
			unsigned char include_loc : 1;
			unsigned char include_vis_contents : 1;
			unsigned char include_step_xy : 1;
			unsigned char copy_loc : 1;
			unsigned char copy_vis_contents : 1;
		};
		unsigned char byte = 0;
	};
};

template<class T> struct LastListItem {
	int last_loc = 0;
	int last_appearance = 0xFFFF;
	int last_vis_contents_hash = 0;
	short last_step_x = 0;
	short last_step_y = 0;
};
template<> struct LastListItem<Turf> {
	int last_appearance = 0xFFFF;
	int last_vis_contents_hash = 0;
};
/*template<> struct LastListItem<Image> {
	int last_loc = 0;
	int last_appearance = 0xFFFF;
	int last_vis_contents_hash = 0;
};*/

inline unsigned int ref_int(Value val) {
	if (!val.type) return 0;
	return (val.type << 24) | (val.value & 0xFFFFFF);
}
inline unsigned int ref_int_relative(Value val, unsigned int parent) {
	if (!val.type) return 0;
	return (val.type << 24) | 0x80000000 | ((val.value - parent) & 0xFFFFFF);
}

template<class Atom, unsigned char update_chunk_id, bool includes_loc = true, bool includes_stepxy = true>
class AtomUpdateBuffer {
private:
	int dirty_floor = 0;
	int last_atom_loc_ref = 0;
	int last_atom_abs_loc_ref = 0;
	int last_atom_rel_loc_ref = 0;
	std::priority_queue<int, std::vector<int>, std::greater<int>> dirty_list;
	std::vector<LastListItem<Atom>> last_list;
	void write_update(std::vector<unsigned char>& buf, int id) {
		DemoWriterIdFlags& dif = get_demo_id_flags(id);
		dif.set_written<Atom>(true);
		Atom* atom = get_element(id);
		ByteVecRef<DemoAtomFlags> daf = write_primitive(buf, DemoAtomFlags());
		if (last_list.size() <= id) {
			last_list.resize(id + 1);
		}
		LastListItem<Atom>& lli = last_list[id];
		int appearance = get_appearance(atom, id);
		if (appearance != lli.last_appearance || !(get_demo_id_flags(appearance).appearance_written)) {
			daf->include_appearance = true;
			lli.last_appearance = write_appearance(buf, appearance);
		}
		if constexpr (includes_loc) {
			Value loc_val = { NULL_D, { 0 } };
			if(atom)loc_val = atom->loc;
			int abs_loc = ref_int(loc_val);
			if (abs_loc != lli.last_loc) {
				int relative_loc = ref_int_relative(loc_val, id);
				int loc = (abs_loc != 0 && relative_loc == last_atom_rel_loc_ref) ? relative_loc : abs_loc;
				if (loc == last_atom_loc_ref) {
					daf->copy_loc = true;
				} else {
					daf->include_loc = true;
					write_primitive(buf, loc);
				}
				lli.last_loc = abs_loc;
				last_atom_abs_loc_ref = abs_loc;
				last_atom_loc_ref = loc;
				last_atom_rel_loc_ref = relative_loc;
			}
		}
		if constexpr (true) {
			unsigned int vis_contents_hash = 0;
			TableHolder3 *vis_contents = atom ? atom->vis_contents : nullptr;
			if (vis_contents) {
				for (int i = 0; i < vis_contents->size; i++) {
					Value thing = ((Value*)vis_contents->elements)[i];
					unsigned int ref = ref_int(thing);
					unsigned long long extended = ((unsigned long long)ref + (unsigned long long)(vis_contents_hash * 31));
					vis_contents_hash = (extended) + (extended >> 32);
				}
			}
			if (vis_contents_hash != lli.last_vis_contents_hash) {
				daf->include_vis_contents = true;
				lli.last_vis_contents_hash = vis_contents_hash;
				if (vis_contents) {
					write_vlq(buf, vis_contents->size);
					for (int i = 0; i < vis_contents->size; i++) {
						Value thing = ((Value*)vis_contents->elements)[i];
						unsigned int ref = ref_int(thing);
						write_primitive(buf, ref);
					}
				}
				else {
					write_vlq(buf, 0);
				}
			}
		}
		if constexpr (includes_stepxy) {
			if (atom) {
				short step_x = (short)(atom->step_x * 256);
				short step_y = (short)(atom->step_y * 256);
				if (step_x != lli.last_step_x || step_y != lli.last_step_y) {
					daf->include_step_xy = true;
					write_primitive(buf, step_x);
					write_primitive(buf, step_y);
					lli.last_step_x = step_x;
					lli.last_step_y = step_y;
				}
			}
		}
	}

	int next_item() {
		if (!dirty_list.empty()) {
			int popped = dirty_list.top();
			dirty_list.pop();
			return popped;
		}
		while (dirty_floor < get_table_length()) {
			int r = dirty_floor++;
			if (!get_demo_id_flags(r).get_written<Atom>() && does_element_exist(r)) {
				return r;
			} else {
				DemoWriterIdFlags& dif = get_demo_id_flags(r);
				dif.set_written<Atom>(true);
			}
		}
		return -1;
	}

	inline int get_table_length();
	inline Atom *get_element(int id);
	inline int get_appearance(Atom* atom, int id) { return atom ? atom->appearance : 0xFFFF; }
	inline bool does_element_exist(int id) { return get_element(id); }
public:
	void flush() {
		int curr_ref = next_item();
		if (curr_ref < 0) return;
		std::vector<unsigned char> vec;
		write_vlq(vec, curr_ref);
		int len_pos = vec.size();
		int amt_in = 0;;
		while (curr_ref >= 0) {
			write_update(vec, curr_ref);
			amt_in++;
			int ideal_next = curr_ref + 1;
			curr_ref = next_item();
			while (curr_ref < ideal_next && curr_ref >= 0) {
				DemoWriterIdFlags& dif = get_demo_id_flags(curr_ref);
				dif.set_written<Atom>(true);
				curr_ref = next_item(); // dang it going down is bad
			}
			if (curr_ref != ideal_next) {
				std::vector<unsigned char> vlq_vec;
				write_vlq(vlq_vec, amt_in);
				amt_in = 0;
				vec.insert(vec.begin() + len_pos, vlq_vec.begin(), vlq_vec.end());
				if (curr_ref > ideal_next) {
					write_vlq(vec, curr_ref - ideal_next);
				}
				len_pos = vec.size();
			}
		}
		vec.push_back(0);
		update_demo_time();
		demo_file_handle.put(update_chunk_id);
		write_vlq(vec.size());
		demo_file_handle.write((char*)&vec[0], vec.size());
	}

	void mark_dirty(int id) {
		if (id >= dirty_floor) return;
		DemoWriterIdFlags& dif = get_demo_id_flags(id);
		if (dif.get_written<Atom>()) {
			dif.set_written<Atom>(false);
			dirty_list.push(id);
		}
	}
};

inline int AtomUpdateBuffer<Turf, 0x2, false, false>::get_table_length() {
	return Core::turf_table->turf_count;
}
inline int AtomUpdateBuffer<Obj, 0x3, true, true>::get_table_length() {
	return Core::obj_table->length;
}
inline int AtomUpdateBuffer<Mob, 0x4, true, true>::get_table_length() {
	return Core::mob_table->length;
}
inline Turf *AtomUpdateBuffer<Turf, 0x2, false, false>::get_element(int id) {
	if (id < Core::turf_table->turf_count || Core::turf_table->existence_table[id] == 0) {
		return nullptr;
	}
	Turf* ref = Core::turf_hashtable->elements[id & Core::turf_hashtable->mask];
	while (ref && ref->id != id) {
		ref = ref->next;
	}
	return ref;
}
inline Obj* AtomUpdateBuffer<Obj, 0x3, true, true>::get_element(int id) {
	return id < Core::obj_table->length ? Core::obj_table->elements[id] : nullptr;
}
inline Mob* AtomUpdateBuffer<Mob, 0x4, true, true>::get_element(int id) {
	return id < Core::mob_table->length ? Core::mob_table->elements[id] : nullptr;
}
inline int AtomUpdateBuffer<Turf, 0x2, false, false>::get_appearance(Turf* turf, int id) {
	if (id >= Core::turf_table->turf_count) return 0xFFFF;
	int shared_id = Core::turf_table->shared_info_id_table[id];
	return (*Core::turf_shared_info_table)[shared_id]->appearance;
}
inline int AtomUpdateBuffer<Obj, 0x3, true, true>::get_appearance(Obj* obj, int id) {
	return obj ? GetObjAppearance(id) : 0xFFFF;
}
inline int AtomUpdateBuffer<Mob, 0x4, true, true>::get_appearance(Mob* mob, int id) {
	return mob ? GetMobAppearance(id) : 0xFFFF;
}
inline bool AtomUpdateBuffer<Turf, 0x2, false, false>::does_element_exist(int id) {
	return true;
}

AtomUpdateBuffer<Turf, 0x2, false, false> turf_update_buffer;
AtomUpdateBuffer<Obj, 0x3, true, true> obj_update_buffer;
AtomUpdateBuffer<Mob, 0x4, true, true> mob_update_buffer;

void flush_atom_updates() {
	turf_update_buffer.flush();
	obj_update_buffer.flush();
	mob_update_buffer.flush();
}

void mark_atom_dirty(Value atom) {
	switch (atom.type) {
	case TURF:
	case LIST_TURF_VERBS:
	case LIST_TURF_OVERLAYS:
	case LIST_TURF_UNDERLAYS:
		turf_update_buffer.mark_dirty(atom.value);
		break;
	case OBJ:
	case LIST_VERBS:
	case LIST_OVERLAYS:
	case LIST_UNDERLAYS:
		obj_update_buffer.mark_dirty(atom.value);
		break;
	case MOB:
	case LIST_MOB_VERBS:
	case LIST_MOB_OVERLAYS:
	case LIST_MOB_UNDERLAYS:
		mob_update_buffer.mark_dirty(atom.value);
		break;
	}
}
