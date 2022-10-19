#pragma once
#include "byond_structures.h"
#include "byond_functions.inl"

#define RELATIVE_CALL_RESOLVE(ptr) ((void*)((unsigned int)ptr + 4 + *((unsigned int*)ptr)))
#define RELATIVE_CALL_SET(ptr, target) (int*)ptr = (unsigned int)(target) - 4 - (unsigned int)ptr
