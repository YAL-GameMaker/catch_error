#include "stdafx.h"

extern uint8_t* catch_error_suppress;
extern bool catch_error_fatal_force;
extern int catch_error_fatal_hooked;

constexpr int searchBufSize = 16384;
uint8_t searchBuf[searchBufSize];
struct EBP {
	EBP* next;
	void* func;
};
#ifdef NDEBUG
__forceinline
#endif
void initFatal() { // welcome to heck
	catch_error_fatal_force = false;
	// So we need to do two things here:
	// 1. Make fatal errors recognize which button was pressed (bug? feature?)
	// 2. Find the address of "suppress errors" flag for YYError
	EBP* _ebp = nullptr;
	__asm { mov _ebp, ebp };
	if (_ebp == nullptr) {
		trace("Can't hook, EBP is NULL");
		return;
	}
	//
	#ifdef _DEBUG
	_ebp = _ebp->next; // 
	#endif
	_ebp = _ebp->next;
	uint8_t* calledFrom = (uint8_t*)_ebp->func;
	if (calledFrom[0] == 0x89) { // extra layer..?
		_ebp = _ebp->next;
		calledFrom = (uint8_t*)_ebp->func;
	}
	uint8_t newJmp = 8;
	if (calledFrom[0] == 0x83) { // has duplicated ADD ECX 8?
		calledFrom += 3;
		newJmp += 6;
	}
	do {
		catch_error_fatal_hooked = -1;
		// The caller goes like this:
		// +00  E8 ## ## ## ##       call        <our caller>
		// +05  EB 0A                jmp         <somewhere else>
		// +07  6A 01                push        1
		// +09  57                   push        edi
		// +0A  E8 ## ## ## ##       call        <our caller>
		// +0F  8A D8                mov         bl, al           (can be a fancier mov)
		// So, to retrieve result on first branch, we need to reroute jmp
		// to land on the mov after the second call.
		if (calledFrom[0] != 0xEB) { // we want a JMP
			trace("Can't hook fatal error handling - unusual caller? (v=%X, at=%X)",
				calledFrom[0], calledFrom);
			continue;
		}
		// for some reason, we can't straight up assign/memcpy this bit
		HANDLE hp = GetCurrentProcess();
		SIZE_T done;
		uint8_t orig = calledFrom[1];
		if (WriteProcessMemory(hp, calledFrom + 1, &newJmp, 1, &done) == 0) {
			trace("Failed to hook fatals - error %d", GetLastError());
			continue;
		} else {
			trace("Legacy format errors can now be non-fatal (address=%x, old=%x, new=%x)", calledFrom + 1, orig, calledFrom[1]);
		}
		//
		uint8_t* endsAt = nullptr;
		uint8_t endsSig[] = { 0xC3/*RET*/, 0xCC, 0xCC};
		for (int i = 0; i < 1024; i++) {
			if (memcmp(calledFrom + i, &endsSig, sizeof(endsSig)) != 0) continue;
			endsAt = calledFrom + i;
			break;
		}
		if (endsAt == nullptr) {
			trace("Couldn't find where the error function ends (??)");
			catch_error_fatal_hooked = 1;
			continue;
		}
		uint8_t* p = endsAt;
		while (--p > calledFrom) {
			// we're looking for CMP byte ptr <addr>, 0
			if (p[0] != 0x80/*CMP*/) continue;
			if (p[1] != 0x3D) continue;
			if (p[6] != 0x00) continue;
			//
			bool hasJNE = false;
			int n = endsAt - p;
			if (n > 40) n = 40;
			for (int i = 7; i < n; i++) {
				if (p[i] != 0x75) continue;
				if (p[i + 1] > 0x40) continue;
				hasJNE = true;
				break;
			}
			//
			if (!hasJNE) continue;
			catch_error_suppress = (uint8_t*)(*(uint32_t*)(p + 2));
			trace("Error suppression flag located (address=%x), first used at %x", catch_error_suppress, p + 2);

			// things get weird here. So we have a few functions in a static lib
			// that are used for showing formatted errors, and go like
			// void YYError(const char* format, ...) {
			//     if (suppress) return;
			//     sprintf(...);
			//     if (!suppress) {
			//         show_error(...); <- the function that calls our function
			//         exit(-1); <- CRT exit
			//     }
			// }
			// and we need to have them not call exit()
			// (because then we can't handle our errors from game code, you know)
			// but these are small enough for C++ compiler to not put them on EBP, so...
			// we kind of just scan surrounding memory to find them
			uint8_t cmp_sup_0__jne[] = { 0x80, 0x3D, p[2], p[3], p[4], p[5], 0 };
			const int tailSize = 128;
			const int stepSize = searchBufSize - tailSize;
			uint8_t* searchbuf = searchBuf;
			bool sign_ok[2] = { true, true };
			for (int step = 0; step < 128; step++) for (int sign = 0; sign < 2; sign++) {
				if (sign_ok[sign] == false) continue;
				uint8_t* chunkPos;
				if (sign) {
					chunkPos = p - (step + 1) * stepSize - 256;
				} else chunkPos = p + step * stepSize + 4;
				if (ReadProcessMemory(hp, chunkPos, searchbuf, searchBufSize, &done) == 0) {
					trace("Reached unreadable memory at side=%d adr=%x", sign, chunkPos);
					sign_ok[sign] = false;
					if (!sign_ok[0] && !sign_ok[1]) break;
				}
				//trace("check %x %d", chunkPos, chunkPos - p);
				for (int i = 0; i < stepSize; i++) {
					if (memcmp(searchbuf + i, cmp_sup_0__jne, sizeof(cmp_sup_0__jne)) != 0) continue;
					uint8_t* up = chunkPos + i;
					uint8_t cmpJmp = up[sizeof(cmp_sup_0__jne)];
					if (cmpJmp == 0x74 || cmpJmp == 0x8B) {
						uint8_t* setFlagAt = up - 7;
						if (cmpJmp == 0x8B) {
							// cmp Sup, 0; mov ecx, ?; mov Magic, 1
							setFlagAt = up + 14;
						} else if (setFlagAt[0] != 0xC6) {
							// oh, is it the other-way-around version
							setFlagAt = up + sizeof(cmp_sup_0__jne) + 2;
							if (setFlagAt[0] != 0xC6) {
								// come on now
								setFlagAt = up + sizeof(cmp_sup_0__jne) + 9;
							}
						}
						if (setFlagAt[0] != 0xC6) continue;
						if (setFlagAt[1] != 0x05) continue;
						if (setFlagAt[6] != 0x01) continue;
						uint8_t jumpOver[] = { 0xEB, 5 };
						if (WriteProcessMemory(hp, setFlagAt, &jumpOver, sizeof(jumpOver), &done) == 0) {
							trace("Failed to patch error flag at %x - error %d", GetLastError());
						} else {
							trace("Patched %x to not set the error magic flag", setFlagAt);
						}
						continue;
					} else if (cmpJmp != 0x75) continue;
					uint8_t* retAt = up + sizeof(cmp_sup_0__jne) + 2 + (int8_t)up[sizeof(cmp_sup_0__jne) + 1];

					//trace("Error suppression flag used at %x, return at %x", up, retAt);
					// look for cleanup after show_error:
					uint8_t* cleanupAt = up + (sizeof(cmp_sup_0__jne) + 2);
					uint8_t* cleanupTill = cleanupAt + 64;
					uint8_t cleanupSig[] = { 0x83, 0xC4, 0x08 }; // add ESP, 8
					for (; cleanupAt < cleanupTill; cleanupAt++) {
						if (memcmp(cleanupAt, cleanupSig, sizeof(cleanupSig)) != 0) continue;
						cleanupAt += sizeof(cleanupSig);
						int jumpToRetOfs = cleanupAt + 2 - retAt;
						if (jumpToRetOfs > 128) {
							trace("Can't patch exit() at %x - have to go too far", cleanupAt, jumpToRetOfs);
							break;
						}
						uint8_t jumpToRet[] = { 0xEB, (uint8_t)(0x100 - jumpToRetOfs) };
						if (WriteProcessMemory(hp, cleanupAt, &jumpToRet, sizeof(jumpToRet), &done) == 0) {
							trace("Failed to patch exit() at %x - error %d", GetLastError());
						} else {
							trace("Patched %x to not immediately exit on fatal errors", cleanupAt);
						}
						break;
					}
				}
			}
			break;
		}
		if (catch_error_suppress == nullptr) trace("Couldn't find error suppression flag.");
		//
		catch_error_fatal_hooked = 1;
	} while (false);
}