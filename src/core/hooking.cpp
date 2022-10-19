#include "hooking.h"
#include "subhook/subhook.h"
#include <unordered_map>
#include "byond_functions.h"

std::unordered_map<void*, subhook::Hook*> hooks;

void* Core::untyped_install_hook(void* original, void* hook)
{
	subhook::Hook* /*I am*/ shook = new subhook::Hook;
	shook->Install(original, hook);
	hooks[original] = shook;
	return shook->GetTrampoline();
}

void Core::remove_hook(void* func)
{
	hooks[func]->Remove();
	delete hooks[func];
	hooks.erase(func);
}

extern "C" void *subhook_unprotect(void *address, size_t size);

void Core::remove_all_hooks()
{
	for (auto iter = hooks.begin(); iter != hooks.end(); )
	{
		iter->second->Remove();
		delete iter->second;
		iter = hooks.erase(iter);
	}

	//F(void, , AnimateStartFun, void *some_struct, Value args)
	//	V unsigned int* animate_start_call;
	//V unsigned int* animate_end_jump;
	//V unsigned int original_animate_end_jump;
#ifndef _WIN32
	if (animate_start_call && RELATIVE_CALL_RESOLVE(animate_start_call) != (void*)AnimateStartFun) {
		subhook_unprotect(animate_start_call, 4);
		RELATIVE_CALL_SET(animate_start_call, AnimateStartFun);
	}
	if (animate_end_jump && *animate_end_jump != original_animate_end_jump) {
		subhook_unprotect(animate_end_jump, 4);
		RELATIVE_CALL_SET(animate_end_jump, original_animate_end_jump);
	}
#endif
}
