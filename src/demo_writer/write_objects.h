#pragma once
#include "../core/core.h"

void reset_atom_buffers();
void flush_atom_updates();
void mark_atom_dirty(Value atom);