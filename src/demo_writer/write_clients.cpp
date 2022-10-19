#include "write_clients.h"
#include "write_primitive.h"
#include "demo_writer.h"
#include <vector>

struct alignas(1) UpdateClientFlags {
	union {
		struct {
			unsigned char login : 1;
			unsigned char logout : 1;
			unsigned char update_mob : 1;
			unsigned char update_view : 1;
			unsigned char update_eye : 1;
			unsigned char update_screen : 1;
		};
		unsigned char byte = 0;
	};
};

struct alignas(1) ClientView {
	unsigned char width = 15;
	unsigned char height = 15;
};

struct RunningClient {
	RunningClient(int in_key, int in_ckey) {
		key = in_key;
		ckey = in_ckey;

		if (key != 0xFFFF) IncRefCount({ STRING, {key} });
		if (ckey != 0xFFFF) IncRefCount({ STRING, {ckey} });
	}
	int key; int ckey;
	unsigned char view_width;
	unsigned char view_height;
	Value mob;
	Value eye;
	std::vector<unsigned int> screen;
	std::vector<unsigned int> images;
	bool screen_dirty = true;

	bool needs_relogin(Client *client) {
		if (client->key != key) return true;
		if (client->ckey != ckey) return true;
		return false;
	}
	bool needs_update(Client *client) {
		if (needs_relogin(client)) return true;
		if (view_width != (unsigned char)client->view_width || view_height != (unsigned char)client->view_height) return true;
		if (mob.type != client->mob.type || mob.value != client->mob.value) return true;
		if (eye.type != client->eye.type || eye.value != client->eye.value) return true;
		if (screen_dirty) return true;
		return false;
	}

	~RunningClient() {
		if (key != 0xFFFF) DecRefCount({ STRING, {key} });
		if (ckey != 0xFFFF) DecRefCount({ STRING, {ckey} });
	}
};

std::vector<std::unique_ptr<RunningClient>> running_clients;

void write_list_diff(std::vector<unsigned char> &buf, std::vector<unsigned int> &running, Value list) {
	int screen_len = (int)GetVariable(list, 0x39).valuef;
	std::vector<unsigned int> to_add;
	std::vector<unsigned int> to_del;
	int screen_index = 0;
	int old_index = 0;
	while (old_index < running.size()) {
		if (screen_index < screen_len && running[old_index] == ref_int(Core::DecReturn(GetAssocElement(list, ValueFloat((float)(screen_index + 1)))))) {
			screen_index++;
			old_index++;
			continue;
		}
		to_del.push_back(running[old_index]);
		running.erase(running.begin() + old_index);
	}
	while (screen_index < screen_len) {
		unsigned int ref = ref_int(Core::DecReturn(GetAssocElement(list, ValueFloat((float)(screen_index + 1)))));
		to_add.push_back(ref);
		running.push_back(ref);
		screen_index++;
	}
	write_vlq(buf, to_del.size());
	for (int j = 0; j < to_del.size(); j++) {
		write_primitive(buf, to_del[j]);
	}
	write_vlq(buf, to_add.size());
	for (int j = 0; j < to_add.size(); j++) {
		write_primitive(buf, to_add[j]);
	}
}

void write_logout(int id) {
	update_demo_time();
	std::vector<unsigned char> buf;
	write_primitive(buf, (unsigned short)id);
	auto flags = write_primitive(buf, UpdateClientFlags());

	flags->logout = true;

	demo_file_handle.put(0x09);
	write_vlq(buf.size());
	demo_file_handle.write((char*)&buf[0], buf.size());
}

void update_clients() {
	if (running_clients.size() != Core::client_table->length) {
		for (int i = Core::client_table->length; i < running_clients.size(); i++) {
			write_logout(i);
		}
		running_clients.resize(Core::client_table->length);
	}
	for (int i = 0; i < running_clients.size(); i++) {
		Client *client = Core::client_table->elements[i];
		std::unique_ptr<RunningClient> &running = running_clients[i];
		if (!client || client->key == 0 || client->ckey == 0xFFFF) {
			if (running) {
				write_logout(i);
				running.reset();
			}
			continue;
		}
		if (running && !running->needs_update(client)) continue;
		std::vector<unsigned char> buf;
		write_primitive(buf, (unsigned short)i);
		auto flags = write_primitive(buf, UpdateClientFlags());
		if (running && running->needs_relogin(client)) {
			running.reset();
			flags->logout = true;
		}
		if (!running) {
			running = std::unique_ptr<RunningClient>(new RunningClient(client->key, client->ckey));
			flags->login = true;
			flags->update_mob = true;
			flags->update_view = true;
			flags->update_eye = ((client->mob.type != client->eye.type) || (client->mob.value != client->eye.value));
			flags->update_screen = true;
		}
		else {
			if (running->mob.type != client->mob.type || running->mob.value != client->mob.value) {
				flags->update_mob = true;
				if (client->mob.type != client->eye.type || client->mob.value != client->eye.value) {
					flags->update_eye = true;
				}
			}
			else if (running->eye.type != client->mob.type || running->eye.value != client->mob.value) {
				flags->update_eye = true;
			}
			if (running->view_width != client->view_width || running->view_height != client->view_height) flags->update_view = true;
			if (running->screen_dirty) flags->update_screen = true;
		}
		if (flags->login) {
			write_byond_string(buf, client->ckey);
			write_byond_string(buf, client->key);
		}
		if (flags->update_mob) {
			write_primitive(buf, ref_int(client->mob));
			running->mob = client->mob;
			running->eye = client->eye;
		}
		if (flags->update_view) {
			write_primitive(buf, (unsigned char)client->view_width);
			write_primitive(buf, (unsigned char)client->view_height);
			running->view_width = client->view_width;
			running->view_height = client->view_height;
		}
		if (flags->update_eye) {
			write_primitive(buf, ref_int(client->eye));
			running->eye = client->eye;
		}
		if (flags->update_screen) {
			running->screen_dirty = false;
			write_list_diff(buf, running->screen, { LIST_CLIENT_SCREEN, {i} });
			write_list_diff(buf, running->images, { LIST_CLIENT_IMAGES, {i} });
		}

		update_demo_time();
		demo_file_handle.put(0x09);
		write_vlq(buf.size());
		demo_file_handle.write((char*)&buf[0], buf.size());
	}
}

void update_client_screen(unsigned short id) {
	if (id >= running_clients.size()) return;
	if (!running_clients[id]) return;
	running_clients[id]->screen_dirty = true;
}

struct alignas(1) SoundRecipientFlags {
	union {
		struct {
			unsigned char send_to_clients : 1;
			unsigned char send_to_world : 1;
			unsigned char send_to_viewer : 1;
		};
		unsigned char byte = 0;
	};
};
struct alignas(1) SoundFlags {
	union {
		struct {
			unsigned char repeat : 2;
			unsigned char wait : 1;
			unsigned char sound_mute : 1;
			unsigned char sound_paused : 1;
			unsigned char sound_stream : 1;
			unsigned char sound_update : 1;
			unsigned char include_3d : 1;
		};
		unsigned char byte = 0;
	};
};

void handle_sound_output(Value target, Value sound) {
	if (sound.type == DATUM) {
		Datum *datum = Core::datum_table->GetItem(sound.value);
		if (!datum || !(datum->dm_interface & 0x20)) return;
	}
	else if (sound.type != RESOURCE) {
		return;
	}

	std::vector<unsigned short> clients;
	bool world = false;
	if (target.type == WORLD_D) {
		world = true;
	}
	else {
		if (target.type == CLIENT) clients.push_back(target.value);
		else if (target.type == MOB) {
			Mob *mob = Core::mob_table->GetItem(target.value);
			if (mob && mob->client != 0xFFFF) {
				clients.push_back(mob->client);
			}
		}
		else if (target.type == LIST) {
			int len = (int)GetVariable(target, 0x39).valuef;
			for (int i = 0; i < len; i++) {
				Value elem = Core::DecReturn(GetAssocElement(target, ValueFloat((float)(i + 1))));
				if (elem.type == CLIENT) clients.push_back(target.value);
				else if (elem.type == MOB) {
					Mob *mob = Core::mob_table->GetItem(elem.value);
					if (mob && mob->client != 0xFFFF) {
						clients.push_back(mob->client);
					}
				}
			}
		}
	}
	if (!world && !clients.size()) return;

	SoundRecipientFlags recipient_flags;
	recipient_flags.send_to_clients = clients.size();
	recipient_flags.send_to_world = world;
	std::vector<unsigned char> buf;
	write_primitive(buf, recipient_flags);
	if (recipient_flags.send_to_clients) {
		write_vlq(buf, clients.size());
		for (int i = 0; i < clients.size(); i++) {
			write_primitive(buf, clients[i]);
		}
	}
	if (sound.type == RESOURCE) {
		write_vlq(buf, 1);
		write_byond_resourceid(buf, sound.value);
		write_primitive(buf, (unsigned char)0);
		write_primitive(buf, (unsigned short)0);
		write_primitive(buf, (unsigned char)100);
		write_primitive(buf, (float)0);
		write_primitive(buf, (float)0);
	}
	else {
		Value sound_file = GetVariable(sound, 0x41);
		if (sound_file.type == RESOURCE) {
			write_vlq(buf, 1);
			write_byond_resourceid(buf, sound_file.value);
		}
		else if (sound_file.type == LIST) {
			std::vector<unsigned int> resourceids;
			int len = (int)GetVariable(sound_file, 0x39).valuef;
			for (int i = 0; i < len; i++) {
				Value elem = Core::DecReturn(GetAssocElement(sound_file, ValueFloat((float)(i + 1))));
				if (elem.type == RESOURCE) resourceids.push_back(elem.value);
			}
			write_vlq(buf, resourceids.size());
			for (int i = 0; i < resourceids.size(); i++) {
				write_byond_resourceid(buf, resourceids[i]);
			}
		}
		else {
			write_vlq(buf, 0);
		}
		auto sf = write_primitive(buf, SoundFlags());
		sf->repeat = GetVariable(sound, 0x9c).valuef;
		sf->wait = GetVariable(sound, 0x9b).valuef;
		int flags = GetVariable(sound, 0x70).valuef;
		if (flags & 1) sf->sound_mute = true;
		if (flags & 2) sf->sound_paused = true;
		if (flags & 4) sf->sound_stream = true;
		if (flags & 16) sf->sound_update = true;
		write_primitive(buf, (unsigned short)GetVariable(sound, 0x8d).valuef);
		write_primitive(buf, (unsigned char)GetVariable(sound, 0xc7).valuef);
		write_primitive(buf, GetVariable(sound, 0xc5).valuef);
		write_primitive(buf, GetVariable(sound, 0xc6).valuef);
		float x = GetVariable(sound, 0x33).valuef;
		float y = GetVariable(sound, 0x34).valuef;
		float z = GetVariable(sound, 0x35).valuef;
		float falloff = GetVariable(sound, 0xc9).valuef;
		if (x != 0 || y != 0 || z != 0 || falloff != 1) {
			sf->include_3d = true;
			write_primitive(buf, x);
			write_primitive(buf, y);
			write_primitive(buf, z);
			write_primitive(buf, falloff);
		}
	}
	update_demo_time();
	demo_file_handle.put(0x0B);
	write_vlq(buf.size());
	demo_file_handle.write((char*)&buf[0], buf.size());
}
