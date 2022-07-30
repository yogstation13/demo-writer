#include "state_tracking.h"

std::vector<DemoWriterIdFlags> demo_id_flags;
DemoWriterIdFlags& get_demo_id_flags(int id) {
	if (demo_id_flags.size() <= id)
		demo_id_flags.resize(id + 1);
	return demo_id_flags[id];
}