#include "hooking.h"
#include "subhook/subhook.h"
#include <unordered_map>

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

void Core::remove_all_hooks()
{
	for (auto iter = hooks.begin(); iter != hooks.end(); )
	{
		iter->second->Remove();
		delete iter->second;
		iter = hooks.erase(iter);
	}
}
