#ifdef BYOND_FUNCTIONS_VARS
#define F(ret, attrs, name, ...) name##Ptr name;
#define V
#else
#define F(ret, attrs, name, ...) typedef ret (attrs *name##Ptr)(__VA_ARGS__); extern name##Ptr name;
#define V extern
#endif

F(String*, , GetStringTableEntry, int stringId)
F(void, , SetTurfAppearance, int appearance, int turf)
F(void, REGPARM3, SetAppearance, Value atom, int appearance)
F(void, , SpliceString, unsigned int id)
F(void, , SetPixelX, Value atom, short pixel_x)
F(void, , SetPixelY, Value atom, short pixel_y)
F(void, , SetPixelZ, Value atom, short pixel_z)
F(void, , SetPixelW, Value atom, short pixel_w)
F(void, , SetMovableDir, Value atom, unsigned char dir)
F(void, , SetLoc, Value atom, Value loc)
F(unsigned int, , ToString, int type, int value)
F(unsigned int, , GetAppearance, Value thing)
F(Value, , GetVariable, Value datum, unsigned int varNameId)
F(int, , CreateObj, int p1, int p2, int p3, int p4, int p5, int p6)
F(int, , CreateMob, int p1, int p2, int p3, int p4, int p5, int p6)
F(void, , ChangeTurf, int a, int b, int type, Value turf)
F(Value, REGPARM3, GetAssocElement, Value list, Value key)
F(void, , IncRefCount, Value thing)
F(void, , DecRefCount, Value thing)
F(void, REGPARM2, Output, Value target, Value unk, Value thing)
F(void*, , GetVisContents, Value thing, bool modify)
F(bool, , AddToScreen, Value thing, unsigned short client)
F(void, , RemoveFromScreen, Value thing, unsigned short client)
F(void, REGPARM3, SendMapsClient, unsigned short client)

#ifdef _WIN32
F(void, _fastcall, SpliceAppearance, void* this_, int edx, int appearance) // it's actually __thiscall but c++ compilers are stupid
F(void, _fastcall, DelFilter, void* this_, int edx, int id) // also actually __thiscall

F(void, , Animate, Value args)
#else
F(void, , SpliceAppearance, void* this_, int appearance) // it's actually __thiscall but c++ compilers are stupid
F(void, , DelFilter, void* this_, int id) // also actually __thiscall

										  // Animate doesn't exist on linux because it's inlined into the function that's a loop with a giant fucking switch statement.
F(void, , AnimateStartFun, void *some_struct, Value args)
V unsigned int* animate_start_call;
V unsigned int* animate_end_jump;
V unsigned int original_animate_end_jump;
#endif

#undef F
#undef V
