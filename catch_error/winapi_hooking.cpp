#include "stdafx.h"
#include <Dbghelp.h>

BOOL ModifyImportTable(IMAGE_IMPORT_DESCRIPTOR* iid, void* target, void* replacement)
{
	IMAGE_THUNK_DATA* itd = (IMAGE_THUNK_DATA*)(((char*)GetModuleHandle(NULL)) + iid->FirstThunk);

	while (itd->u1.Function)
	{
		if (((void*)itd->u1.Function) == target)
		{
			// Temporary change access to memory area to READWRITE
			MEMORY_BASIC_INFORMATION mbi;
			VirtualQuery(itd, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
			VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect);

			// Replace entry!!
			*((void**)itd) = replacement;

			// Restore memory permissions
			VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &mbi.Protect);

			return TRUE;
		}

		itd += 1;
	}
	return FALSE;
}

bool striequ(const char* s1, const char* s2) {
	for (;;) {
		auto c1 = *s1++;
		auto c2 = *s2++;
		if (c1 >= 'a' && c1 <= 'z') c1 -= 'a' - 'A';
		if (c2 >= 'a' && c2 <= 'z') c2 -= 'a' - 'A';
		if (c1 == 0) return c2 == 0;
		if (c1 != c2) return false;
	}
}

BOOL InstallHook(LPCSTR module, LPCSTR function, void* hook, void** original)
{
	HMODULE process = GetModuleHandle(NULL);

	// Save original address to function
	*original = (void*)GetProcAddress(GetModuleHandleA(module), function);

	ULONG entrySize;

	IMAGE_IMPORT_DESCRIPTOR* iid = (IMAGE_IMPORT_DESCRIPTOR*)ImageDirectoryEntryToData(process, 1, IMAGE_DIRECTORY_ENTRY_IMPORT, &entrySize);

	// Search for module
	while (iid->Name)
	{
		const char* name = ((char*)process) + iid->Name;
		if (striequ(name, module))
		{
			return ModifyImportTable(iid, *original, hook);
		}
		iid += 1;
	}

	return FALSE;
}