#include "write_appearance.h"
#include "write_primitive.h"
#include "state_tracking.h"
#include "../core/core.h"

struct alignas(1) DemoAppearanceFlags {
	union {
		struct {
			unsigned char dir : 3;
			unsigned char opacity : 1;
			unsigned char density : 1;
			unsigned char gender : 2;
			unsigned char dir_override : 1;

			unsigned char include_name : 1;
			unsigned char include_desc : 1;
			unsigned char include_screen_loc : 1;
			unsigned char include_maptext : 1;
			unsigned char include_overlays_and_underlays : 1;
			unsigned char include_layer : 1;
			unsigned char include_invisibility : 1;
			unsigned char include_glide_size : 1;

			unsigned char include_pixel_xy : 1;
			unsigned char include_pixel_wz : 1;
			unsigned char include_plane : 1;
			unsigned char include_transform : 1;
			unsigned char include_color_alpha : 1;
			unsigned char include_color_matrix : 1;
			unsigned char animate_movement : 2;

			unsigned char blend_mode : 3;
			unsigned char mouse_opacity : 2;
		};
		unsigned char bytes[4] = { 0,0,0,0 };
	};
};

struct alignas(1) ColorMatrixFormat {
	union {
		struct {
			unsigned char is_float : 1;
			unsigned char include_cr_cg_cb : 1;
			unsigned char include_ca : 1;
			unsigned char ca_one : 1;
			unsigned char include_ar_ag_ab : 1;
			unsigned char include_aa : 1;
			unsigned char include_ra_ga_ba : 1;
			unsigned char include_color : 1;
		};
		unsigned char byte = 0;
	};
};

int dir_encode_lut[] = {
	0, 1, 0, 0,
	2, 6, 4, 0,
	3, 7, 5, 0,
	0, 0, 0, 0,
};

int write_appearance(std::vector<unsigned char> &buf, int appearance_id) {
	if (appearance_id == 0xFFFF || appearance_id >= (**Core::appearance_table).length) {
		write_primitive(buf, 0xFFFF);
		return 0xFFFF;
	}
	Appearance* appearance = (**Core::appearance_table).elements[appearance_id];
	if (!appearance) {
		write_primitive(buf, 0xFFFF);
		return 0xFFFF;
	}
	DemoWriterIdFlags& dif = get_demo_id_flags(appearance_id);
	bool do_write = !dif.appearance_written;
	dif.appearance_written = 1;
	write_primitive(buf, (appearance_id & 0xFFFFFF) | (do_write << 24));
	if (!do_write) {
		return appearance_id;
	}
	auto daf = write_primitive(buf, DemoAppearanceFlags());
	daf->dir = dir_encode_lut[appearance->dir & 0xF];
	daf->opacity = appearance->opacity;
	daf->density = appearance->density;
	daf->gender = appearance->gender;
	daf->dir_override = appearance->dir_override;
	daf->animate_movement = (appearance->animate_movement + 1) & 3;
	daf->blend_mode = appearance->blend_mode;
	daf->mouse_opacity = appearance->mouse_opacity;
	write_primitive(buf, (unsigned short)appearance->appearance_flags);
	if (appearance->name_str != 0xFFFF) {
		daf->include_name = true;
		write_byond_string(buf, appearance->name_str);
	}
	if (appearance->desc_str != 0xFFFF) {
		daf->include_desc = true;
		write_byond_string(buf, appearance->desc_str);
	}
	if (appearance->screen_loc_str != 0xFFFF) {
		daf->include_screen_loc = true;
		write_byond_string(buf, appearance->screen_loc_str);
	}
	write_byond_resourceid(buf, appearance->icon_res);
	write_byond_string(buf, appearance->icon_state_str);
	if (appearance->maptext_str != 0xFFFF) {
		daf->include_maptext = true;
		write_byond_string(buf, appearance->maptext_str);
		write_primitive(buf, appearance->maptext_x);
		write_primitive(buf, appearance->maptext_y);
		write_primitive(buf, appearance->maptext_width);
		write_primitive(buf, appearance->maptext_height);
	}
	if (appearance->overlays_list != 0xFFFF || appearance->underlays_list != 0xFFFF) {
		daf->include_overlays_and_underlays = true;
		int o_id = appearance->overlays_list;
		int u_id = appearance->underlays_list;
		TableHolder2 *id_lists = Core::appearance_list_table;
		if (o_id != 0xFFFF && id_lists->length > o_id && id_lists->elements[o_id]) {
			AppearanceList* l = (AppearanceList*)id_lists->elements[o_id];
			write_vlq(buf, l->len);
			for (int i = 0; i < l->len; i++) write_appearance(buf, l->ids[i]);
		}
		else {
			buf.push_back(0);
		}
		if (u_id != 0xFFFF && id_lists->length > u_id&& id_lists->elements[u_id]) {
			AppearanceList* l = (AppearanceList*)id_lists->elements[u_id];
			write_vlq(buf, l->len);
			for (int i = 0; i < l->len; i++) write_appearance(buf, l->ids[i]);
		}
		else {
			buf.push_back(0);
		}
	}
	if (appearance->invisibility > 0) {
		daf->include_invisibility = true;
		buf.push_back(appearance->invisibility);
	}
	if (appearance->pixel_x != 0 || appearance->pixel_y != 0) {
		daf->include_pixel_xy = true;
		write_primitive(buf, appearance->pixel_x);
		write_primitive(buf, appearance->pixel_y);
	}
	if (appearance->pixel_w != 0 || appearance->pixel_z != 0) {
		daf->include_pixel_wz = true;
		write_primitive(buf, appearance->pixel_w);
		write_primitive(buf, appearance->pixel_z);
	}
	if (appearance->glide_size != 0) {
		daf->include_glide_size = true;
		write_primitive(buf, appearance->glide_size);
	}
	if (appearance->layer != 3) {
		daf->include_layer = true;
		write_primitive(buf, appearance->layer);
	}
	if (appearance->plane != -32767) {
		daf->include_plane = true;
		write_primitive(buf, appearance->plane);
	}
	if (
		!appearance->matrix_flag
		|| appearance->transform[0] != 1
		|| appearance->transform[1] != 0
		|| appearance->transform[2] != 0
		|| appearance->transform[3] != 0
		|| appearance->transform[4] != 1
		|| appearance->transform[5] != 0) {
		daf->include_transform = true;
		for (int i = 0; i < 6; i++) {
			write_primitive(buf, appearance->transform[i]);
		}
	}
	if (appearance->alpha != 255 || (appearance->color_matrix == nullptr && (appearance->color_r != 255 || appearance->color_g != 255 || appearance->color_b != 255))) {
		daf->include_color_alpha = true;
		write_primitive(buf, appearance->color_alpha);
	}
	if (appearance->color_matrix != nullptr) {
		daf->include_color_matrix = true;
		float* cm = appearance->color_matrix;
		ColorMatrixFormat cmf;
		for (int i = 0; i < 20; i++) {
			if (cm[i] < 0.0f || cm[i] > 1.0f) {
				cmf.is_float = true;
				break;
			}
		}
		if (cm[16] != 0 || cm[17] != 0 || cm[18] != 0) cmf.include_cr_cg_cb = true;
		if (cm[19] != 0) {
			if (cm[19] == 1) cmf.ca_one = true;
			else cmf.include_ca = true;
		}
		if (cm[12] != 0 || cm[13] != 0 || cm[14] != 0) cmf.include_ar_ag_ab = true;
		if (cm[15] != 1) cmf.include_aa = true;
		if (cm[3] != 0 || cm[7] != 0 || cm[11] != 0) cmf.include_ra_ga_ba = true;
		if (cm[0] != cm[1] || cm[1] != cm[2] || cm[4] != cm[5] || cm[5] != cm[6] || cm[8] != cm[9] || cm[9] != cm[10] || cm[12] != cm[13] || cm[13] != cm[14]) cmf.include_color = true;
		write_primitive(buf, cmf);
		if (cmf.is_float) {
			write_primitive(buf, cm[0]);
			if (cmf.include_color) { write_primitive(buf, cm[1]); write_primitive(buf, cm[2]); }
			if (cmf.include_ra_ga_ba) write_primitive(buf, cm[3]);
			write_primitive(buf, cm[4]);
			if (cmf.include_color) { write_primitive(buf, cm[5]); write_primitive(buf, cm[6]); }
			if (cmf.include_ra_ga_ba) write_primitive(buf, cm[7]);
			write_primitive(buf, cm[8]);
			if (cmf.include_color) { write_primitive(buf, cm[9]); write_primitive(buf, cm[10]); }
			if (cmf.include_ra_ga_ba) write_primitive(buf, cm[11]);
			if (cmf.include_ar_ag_ab) { write_primitive(buf, cm[12]); }
			if (cmf.include_ar_ag_ab && cmf.include_color) { write_primitive(buf, cm[13]); write_primitive(buf, cm[14]); }
			if (cmf.include_aa) write_primitive(buf, cm[15]);
			if (cmf.include_cr_cg_cb) {
				write_primitive(buf, cm[16]);
				write_primitive(buf, cm[17]);
				write_primitive(buf, cm[18]);
			}
			if (cmf.include_ca) write_primitive(buf, cm[19]);
		}
		else {
			unsigned char cmb[20];
			for (int i = 0; i < 20; i++) cmb[i] = (unsigned char)(cm[i] * 255);
			buf.push_back(cmb[0]);
			if (cmf.include_color) { buf.push_back(cmb[1]); buf.push_back(cmb[2]); }
			if (cmf.include_ra_ga_ba) buf.push_back(cmb[3]);
			buf.push_back(cmb[4]);
			if (cmf.include_color) { buf.push_back(cmb[5]); buf.push_back(cmb[6]); }
			if (cmf.include_ra_ga_ba) buf.push_back(cmb[7]);
			buf.push_back(cmb[8]);
			if (cmf.include_color) { buf.push_back(cmb[9]); buf.push_back(cmb[10]); }
			if (cmf.include_ra_ga_ba) buf.push_back(cmb[11]);
			if (cmf.include_ar_ag_ab) { buf.push_back(cmb[12]); }
			if (cmf.include_ar_ag_ab && cmf.include_color) { buf.push_back(cmb[13]); buf.push_back(cmb[14]); }
			if (cmf.include_aa) buf.push_back(cmb[15]);
			if (cmf.include_cr_cg_cb) {
				buf.push_back(cmb[16]);
				buf.push_back(cmb[17]);
				buf.push_back(cmb[18]);
			}
			if (cmf.include_ca) buf.push_back(cmb[19]);
		}
	}
	return appearance_id;
}
