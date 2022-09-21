/// @author YellowAfterlife

#include "stdafx.h"
#include "tiny_string.h"
#include "tiny_array.h"
#include "winapi_hooking.h"
#include "magic_patcher.h"
#include <shellapi.h>

// turns out that you can use a non-__stdcall handler and GameMaker does.
typedef INT_PTR(*YYDLGPROC)(HWND, UINT, WPARAM, LPARAM);

bool catch_error_proc;
bool catch_error_ready;
bool catch_error_is_fatal;
tiny_wstring catch_error_text;
HWND catch_error_dummy = nullptr;

tiny_string ret_string_buf;
const char* ret_string(const wchar_t* wstr) {
	return ret_string_buf.conv(wstr);
}
const char* ret_string(tiny_wstring& wstr) {
	return ret_string_buf.conv(wstr.c_str());
}
void initVars() {
	ret_string_buf.init();
	catch_error_proc = false;
	catch_error_ready = false;
	catch_error_is_fatal = false;
	catch_error_text.init(L"");
	catch_error_dummy = CreateWindowExW(WS_EX_LEFT, L"Message", L"Message", 0, 0, 0, 0, 0, HWND_MESSAGE, 0, 0, 0);
}

void ProcessMessages() {
	tagMSG m;
	while (true) {
		if (PeekMessageW(&m, NULL, 0, 0, PM_REMOVE)) {
			if (m.message != WM_QUIT) {
				TranslateMessage(&m);
				DispatchMessageW(&m);
			} else break;
		} else break;
	}
}
void WaitUp() {
	catch_error_proc = true;
	Sleep(100);
	ProcessMessages();
	Sleep(100);
	ProcessMessages();
	catch_error_proc = false;
}

#pragma region normal
///
enum catch_error_normal_t {
	catch_error_normal_show,
	catch_error_normal_ignore,
	catch_error_normal_queue,
	catch_error_normal_quit,
};
catch_error_normal_t catch_error_normal_mode;
dllg bool catch_error_set_normal(int catch_error_normal_) {
	catch_error_normal_mode = (catch_error_normal_t)catch_error_normal_;
	return true;
}
dllg int catch_error_get_normal() {
	return (int)catch_error_normal_mode;
}
#pragma endregion

#pragma region fatal
///
enum catch_error_fatal_t {
	catch_error_fatal_show,
	catch_error_fatal_ignore,
	catch_error_fatal_queue,
	catch_error_fatal_quit,
};
catch_error_fatal_t catch_error_fatal_mode;
dllg bool catch_error_set_fatal(int catch_error_fatal_) {
	catch_error_fatal_mode = (catch_error_fatal_t)catch_error_fatal_;
	return true;
}
///
dllg int catch_error_get_fatal() {
	return catch_error_fatal_mode;
}
#pragma endregion

#pragma region prompt
///
enum catch_error_prompt_t {
	catch_error_prompt_none,
	catch_error_prompt_message,
	catch_error_prompt_question,
};
catch_error_prompt_t catch_error_prompt_kind;
tiny_wstring catch_error_prompt_text;
tiny_wstring catch_error_prompt_title;
DWORD catch_error_prompt_flags;
dllg bool catch_error_set_prompt(int catch_error_prompt_, const char* text = "", const char* title = "", int MSDN_MessageBox_type = -1) {
	catch_error_prompt_kind = (catch_error_prompt_t)catch_error_prompt_;
	catch_error_prompt_text.conv(text);
	catch_error_prompt_title.conv(title);
	if (MSDN_MessageBox_type >= 0) catch_error_prompt_flags = (DWORD)MSDN_MessageBox_type;
	return true;
}
///
dllx double catch_error_get_prompt_kind() {
	return catch_error_prompt_kind;
}
///
dllx double catch_error_get_prompt_flags() {
	return catch_error_prompt_kind;
}
///
dllx const char* catch_error_get_prompt_text() {
	return ret_string(catch_error_prompt_text);
}
///
dllx const char* catch_error_get_prompt_title() {
	return ret_string(catch_error_prompt_title);
}
#pragma endregion

#pragma region dump
tiny_wstring catch_error_dump_path;
dllg bool catch_error_set_dump_path(const char* path = "") {
	catch_error_dump_path.conv(path);
	return 1;
}
///
dllx const char* catch_error_get_dump_path() {
	return ret_string(catch_error_dump_path);
}
#pragma endregion

#pragma region queue
tiny_array<wchar_t*> catch_error_queue;
/// Returns you the next error from queue (or "" if empty)
dllx const char* catch_error_dequeue() {
	if (catch_error_queue.size() == 0) return "";
	auto wstring = catch_error_queue[0];
	auto result = ret_string(wstring);
	free(wstring);
	catch_error_queue.remove(0);
	return result;
}
/// Clears any queued error messages, returns how many there were
dllx double catch_error_clear() {
	auto len = catch_error_queue.size();
	for (size_t i = 0; i < len; i++) free(catch_error_queue[i]);
	catch_error_queue.resize(0);
	return len;
}
/// Returns the number of error messages in queue
dllx double catch_error_size() {
	return catch_error_queue.size();
}
#pragma endregion

#pragma region exec
tiny_wstring catch_error_exec_path;
tiny_wstring catch_error_exec_args;
dllg bool catch_error_set_exec(const char* path = "", const char* args = "") {
	catch_error_exec_path.conv(path);
	catch_error_exec_args.conv(args);
	return true;
}
///
dllx const char* catch_error_get_exec_path() {
	return ret_string(catch_error_exec_path);
}
///
dllx const char* catch_error_get_exec_params() {
	return ret_string(catch_error_exec_args);
}
#pragma endregion

#pragma region newer
int catch_error_fatal_hooked;
bool catch_error_fatal_force;
dllx double catch_error_fatal_force_raw() {
	catch_error_fatal_force = true;
	return 1;
}

uint8_t* catch_error_suppress;
///
enum catch_error_newer_t {
	catch_error_newer_not_ready = -2,
	catch_error_newer_unavailable = -1,
	catch_error_newer_allow = 0,
	catch_error_newer_ignore = 1
};
catch_error_newer_t catch_error_newer_status;
/// ->catch_error_newer_*
dllx double catch_error_get_newer() {
	if (catch_error_fatal_hooked == 0) return catch_error_newer_not_ready;
	if (catch_error_suppress == nullptr) return catch_error_newer_unavailable;
	return *catch_error_suppress;
}
dllx double catch_error_set_newer_raw(double enable) {
	if (catch_error_fatal_hooked == 0) return catch_error_newer_not_ready;
	if (catch_error_suppress == nullptr) return catch_error_newer_unavailable;
	*catch_error_suppress = (enable > 0.5);
	return *catch_error_suppress;
}
#pragma endregion

void initModes() {
	catch_error_normal_mode = catch_error_normal_ignore;
	catch_error_fatal_mode = catch_error_fatal_quit;
	catch_error_prompt_kind = catch_error_prompt_none;
	catch_error_prompt_text.init(L"");
	catch_error_prompt_title.init(L"");
	catch_error_prompt_flags = 0x0;
	catch_error_dump_path.init(L"");
	catch_error_queue.init();
	//
	catch_error_exec_path.init(L"");
	catch_error_exec_args.init(L"");
	//
	catch_error_fatal_hooked = 0;
	catch_error_fatal_force = false;
	catch_error_suppress = nullptr;
	catch_error_newer_status = catch_error_newer_not_ready;
	//
}

void catch_error_soft_quit(HWND parent, bool showPrompt) {
	if (!catch_error_dump_path.empty()) {
		HANDLE df = CreateFileW(catch_error_dump_path.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		if (df != INVALID_HANDLE_VALUE) {
			auto data = ret_string(catch_error_text);
			DWORD written;
			WriteFile(df, data, ret_string_buf.size(), &written, NULL);
			SetEndOfFile(df);
			CloseHandle(df);
		} else trace("Failed to dump to %ls: code %d", catch_error_dump_path.c_str(), GetLastError());
	}
	//
	bool shouldExec = true;
	if (showPrompt) switch (catch_error_prompt_kind) {
		case catch_error_prompt_message:
		case catch_error_prompt_question:
			DWORD flags = catch_error_prompt_flags;
			if (catch_error_prompt_kind == catch_error_prompt_question) {
				flags |= MB_YESNO;
			} else flags |= MB_OK;
			WaitUp();
			int bt = MessageBoxW(parent, catch_error_prompt_text.c_str(), catch_error_prompt_title.c_str(), flags);
			if (bt == IDNO) shouldExec = false;
			break;
	}
	//
	if (shouldExec && !catch_error_exec_path.empty()) {
		HINSTANCE app = ShellExecuteW(NULL, NULL, catch_error_exec_path.c_str(), catch_error_exec_args.c_str(), NULL, SW_SHOW);
		if ((int)app < 32) trace("Failed to run: %d", (int)app);
	}
}

WinFuncHook(INT_PTR, DialogBoxParamW, HINSTANCE inst, LPCWSTR tpl, HWND parent, DLGPROC proc, LPARAM param) {
	if (lstrcmpW(tpl, L"IDD_ERROR_CODE") != 0) return DialogBoxParamW_base(inst, tpl, parent, proc, param);

	// so we "dry run" the error dialog first to grab the text and whether it's fatal or not
	catch_error_proc = true;
	catch_error_text = L"";
	catch_error_is_fatal = false;
	YYDLGPROC yyproc = (YYDLGPROC)proc;
	yyproc(parent, WM_INITDIALOG, 0, 0);
	catch_error_proc = false;

	//
	auto fatal_mode = catch_error_fatal_mode;
	bool fatal_hook = catch_error_is_fatal && (fatal_mode == catch_error_fatal_ignore || fatal_mode == catch_error_fatal_queue);
	if ((fatal_hook || catch_error_fatal_force) && catch_error_fatal_hooked == 0) initFatal();
	if (fatal_hook && catch_error_fatal_hooked < 0) fatal_mode = catch_error_fatal_quit;

	if (catch_error_is_fatal) switch (fatal_mode) {
		case catch_error_fatal_quit:
			catch_error_soft_quit(parent, true);
			return 1;
		case catch_error_fatal_ignore:
		case catch_error_fatal_queue:
			if (catch_error_fatal_hooked < 0) return 1;
			if (catch_error_fatal_mode == catch_error_fatal_queue) {
				auto size = catch_error_text.size() + 1;
				auto str = malloc_arr<wchar_t>(size);
				memcpy_arr(str, catch_error_text.c_str(), size);
				catch_error_queue.add(str);
			}
			return 0;
	}
	else switch (catch_error_normal_mode) {
		case catch_error_normal_ignore:
			return 0;
		case catch_error_normal_queue: {
			auto size = catch_error_text.size() + 1;
			auto str = malloc_arr<wchar_t>(size);
			memcpy_arr(str, catch_error_text.c_str(), size);
			catch_error_queue.add(str);
			return 0;
		};
		case catch_error_normal_quit:
			catch_error_soft_quit(parent, true);
			return 1;
	}
	WaitUp();
	// show the original message box
	INT_PTR result = DialogBoxParamW_base(inst, tpl, parent, proc, param);
	if (result == 1) catch_error_soft_quit(parent, false);
	return result;
}

WinFuncHook(HWND, GetDlgItem, HWND hDlg, int nIDDlgItem) {
	if (catch_error_proc && nIDDlgItem == 4/* "Ignore" */) {
		// for this trick, we'll return our dummy HWND instead of the button,
		// and check for it in the function below:
		return catch_error_dummy;
	}
	return GetDlgItem_base(hDlg, nIDDlgItem);
}
WinFuncHook(BOOL, ShowWindow, HWND hWnd, int nCmdShow) {
	if (catch_error_proc && hWnd == catch_error_dummy) {
		catch_error_is_fatal = nCmdShow == SW_HIDE;
		return TRUE;
	}
	return ShowWindow_base(hWnd, nCmdShow);
}
WinFuncHook(BOOL, SetDlgItemTextW, HWND hDlg, int nIDDlgItem, LPCWSTR lpString) {
	if (catch_error_proc) {
		// store the error message string
		if (nIDDlgItem == 1001) catch_error_text = lpString;
		// (no need to set text - the function will be called again normally)
		return true;
	}
	return SetDlgItemTextW_base(hDlg, nIDDlgItem, lpString);
}
WinFuncHook(void, Sleep, DWORD dwMilliseconds) {
	if (dwMilliseconds == 100) {
		/*
		Error function goes like this:
			Sleep(100);
			<process window messages>;
			Sleep(100);
			<process window messages>;
			result = DialogBoxW(_, L"IDD_ERROR_CODE", ...);
		so this is the best way I could think of to prevent the game from stalling for 200ms whenever an error is caught
		*/
		return;
	}
	return Sleep_base(dwMilliseconds);
}

///
dllx double catch_error_is_ready() {
	return catch_error_ready;
}
/// returns whether the DLL successfully loaded as such
dllx double catch_error_is_loaded() {
	return true;
}
///
dllx double catch_error_init() {
	if (catch_error_ready) return 2;
	WinFuncHookSet("User32.dll", DialogBoxParamW);
	WinFuncHookSet("User32.dll", GetDlgItem);
	WinFuncHookSet("User32.dll", ShowWindow);
	WinFuncHookSet("User32.dll", SetDlgItemTextW);
	WinFuncHookSet("Kernel32.dll", Sleep);
	trace("All hooked!");
	catch_error_ready = true;
	return 1;
}

void init() {
	initVars();
	initModes();
}
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
	if (ul_reason_for_call == DLL_PROCESS_ATTACH) init();
	return TRUE;
}
