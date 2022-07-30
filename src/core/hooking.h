#pragma once

namespace Core
{
	void* untyped_install_hook(void* original, void* hook);

	// Used to ensure everything is the same function pointer type.
	template<typename FnPtr>
	FnPtr install_hook(FnPtr original, FnPtr hook)
	{
		return (FnPtr)untyped_install_hook((void*)original, (void*)hook);
	}

	void remove_hook(void* func);
	void remove_all_hooks();
}