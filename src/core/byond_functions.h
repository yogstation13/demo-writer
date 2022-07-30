#include "byond_structures.h"

typedef String* (*GetStringTableEntryPtr)(int stringId);
typedef void(*SetTurfAppearancePtr)(int appearance, int turf);
typedef void(*SetAppearancePtr)(Value atom, int appearance);
typedef void(__fastcall *SpliceAppearancePtr)(void* this_, int edx, int appearance); // it's actually __thiscall but c++ compilers are stupid
typedef void(*SpliceStringPtr)(unsigned int id);
typedef void(*SetPixelXPtr)(Value atom, short pixel_x);
typedef SetPixelXPtr SetPixelYPtr;
typedef SetPixelXPtr SetPixelZPtr;
typedef SetPixelXPtr SetPixelWPtr;
typedef void(*SetMovableDirPtr)(Value atom, unsigned char dir);
typedef void(*SetLocPtr)(Value atom, Value loc);
typedef unsigned int(*ToStringPtr)(int type, int value);
typedef unsigned int(*GetObjAppearancePtr)(unsigned int id);
typedef unsigned int(*GetMobAppearancePtr)(unsigned int id);
typedef Value(*GetVariablePtr)(Value datum, unsigned int varNameId);
typedef int(*AllocObjIdPtr)();
typedef int(*AllocMobIdPtr)();
typedef void(*ChangeTurfPtr)(int a, int b, int type, Value turf);

extern GetStringTableEntryPtr GetStringTableEntry;
extern SetTurfAppearancePtr SetTurfAppearance;
extern SetAppearancePtr SetAppearance;
extern SpliceAppearancePtr SpliceAppearance;
extern SpliceStringPtr SpliceString;
extern SetPixelXPtr SetPixelX;
extern SetPixelYPtr SetPixelY;
extern SetPixelWPtr SetPixelW;
extern SetPixelZPtr SetPixelZ;
extern SetMovableDirPtr SetMovableDir;
extern SetLocPtr SetLoc;
extern ToStringPtr ToString;
extern GetMobAppearancePtr GetMobAppearance;
extern GetObjAppearancePtr GetObjAppearance;
extern GetVariablePtr GetVariable;
extern AllocObjIdPtr AllocObjId;
extern AllocMobIdPtr AllocMobId;
extern ChangeTurfPtr ChangeTurf;
