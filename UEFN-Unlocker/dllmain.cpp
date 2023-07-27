// dllmain.cpp : Defines the entry point for the DLL application.
#include "pch.h"
#include <Windows.h>
#include <iostream>
#include <iomanip>
#include "Memory.h"
#include <map>
using namespace std;

HMODULE hModule;
HINSTANCE DllHandle;
HANDLE aoe_handle = GetCurrentProcess();

DWORD __stdcall EjectThread(LPVOID lpParameter) {
    Sleep(100);
    FreeLibraryAndExitThread(DllHandle, 0);
}

const int buff_patch_size = 6;
struct uefn_buff {
    const char* name;
    uintptr_t address;
    unsigned char modified_bytes[buff_patch_size];
    unsigned char original_bytes[buff_patch_size];
};

const int total_buffs = 6;
struct uefn_buff uefn_buffs[total_buffs];
map<string, uintptr_t> OneByteAddressesMap;

void init_buffs() {
    uefn_buffs[0] = {
        "\"Cannot modify cooked assets\" Error",
        Memory::GetAddressFromSig("0F 84 97 00 00 00 4C 8D 0D ?? ?? ?? ?? ?? 8D 05 ?? ?? ?? ?? 48 8D 15 ?? ?? ?? ?? 48 8D 4C 24 40 E8 ?? ?? ?? ?? 4C 8B C0"),
        { 0xE9, 0x98, 0x00, 0x00, 0x00, 0x90 },
        { 0x0F, 0x84, 0x97, 0x00, 0x00, 0x00 }
    };
    uefn_buffs[1] = {
        "\"Unable to Edit Cooked asset\" Error",
        Memory::GetAddressFromSig("0F 84 EF 00 00 00 E8 ?? ?? ?? F8"),
        { 0xE9, 0xF0, 0x00, 0x00, 0x00, 0x90 },
        { 0x0F, 0x84, 0xEF, 0x00, 0x00, 0x00 }
    };
    uefn_buffs[2] = {
        "\"Folder {folder} is read only...\" Error",
        Memory::GetAddressFromSig("0F 85 08 01 00 00 48 8D 44 24 40"),
        { 0xE9, 0x09, 0x01, 0x00, 0x00, 0x90 },
        { 0x0F, 0x85, 0x08, 0x01, 0x00, 0x00 }
    };
    uefn_buffs[3] = {
        "\"Alias asset '{asset}' is in a folder...\" Error",
        Memory::GetAddressFromSig("0F 84 E4 02 00 00 48 8D 45 50"),
        { 0xE9, 0xE5, 0x02, 0x00, 0x00, 0x90 },
        { 0x0F, 0x84, 0xE4, 0x02, 0x00, 0x00 }
    };
    uefn_buffs[4] = {
        "\"Asset '{asset}' is in a folder...\" Error",
        Memory::GetAddressFromSig("0F 85 04 01 00 00 48 8D 44 24 70"),
        { 0xE9, 0x05, 0x01, 0x00, 0x00, 0x90 },
        { 0x0F, 0x85, 0x04, 0x01, 0x00, 0x00 }
    };
    uefn_buffs[5] = {
        "\"Alias asset '{asset}' is in a read only folder...\" Error",
        Memory::GetAddressFromSig("0F 84 64 06 00 00 48 8D 45 20"),
        { 0xE9, 0x65, 0x06, 0x00, 0x00, 0x90 },
        { 0x0F, 0x84, 0x64, 0x06, 0x00, 0x00 }
    };
    OneByteAddressesMap["CMGC"] = Memory::GetAddressFromSig("74 3F 4C 8D 0D ?? ?? ?? ?? 4C 8D 05 ?? ?? ?? ?? 48 8D 15 ?? ?? ?? ?? 48 8D 4C 24 20");
    OneByteAddressesMap["CDCA"] = Memory::GetAddressFromSig("74 28 4C 8B 33");
    OneByteAddressesMap["PICO"] = Memory::GetAddressFromSig("74 ?? 44 8D 46 29");
    OneByteAddressesMap["FIL"] = Memory::GetAddressFromSig("74 26 49 8B 04 24 49 8B CC FF 50 40");
}

void writeMemory(uintptr_t address, void* to_patch, int patch_size, const char* name) {
    bool couldPatch = WriteProcessMemory(aoe_handle, (void*)address, (void*)to_patch, patch_size, 0);
    cout << (couldPatch ? "Succesfully patched " : "Couldn't patch ") << name << "!\n    (Address: " << "0x" << setbase(16) << address << ")\n";
}


bool patched = false;
void toggle_patches() {
    for (int i = 0; i < total_buffs; i++) {
        struct uefn_buff *current_buff = &uefn_buffs[i];

        void* to_patch = patched ? current_buff->original_bytes : current_buff->modified_bytes;
        writeMemory(current_buff->address, to_patch, buff_patch_size, current_buff->name);
    }

    unsigned char OneByteBytes[1]{ 0xEB };
    if (patched) {
        OneByteBytes[0] = 0x74;
    }
    
    writeMemory(OneByteAddressesMap["CMGC"], (void*)OneByteBytes, 1, "\"Cannot modify generated classes\" Error");
    writeMemory(OneByteAddressesMap["CDCA"], (void*)OneByteBytes, 1, "\"Cannot duplicate cooked asset\" 1st Error");
    writeMemory(OneByteAddressesMap["PICO"], (void*)OneByteBytes, 1, "\"Cannot duplicate cooked asset\" 2nd Error");
    writeMemory(OneByteAddressesMap["FIL"], (void*)OneByteBytes, 1, "\"Folder is locked\" Error");

    patched = !patched;
}

DWORD WINAPI main_func() {
    AllocConsole();
    FILE* fp;
    freopen_s(&fp, "CONOUT$", "w", stdout);
    const char* logo = (R"(
 _____  _____  ________  ________  ____  _____
|_   _||_   _||_   __  ||_   __  ||_   \|_   _|
  | |    | |    | |_ \_|  | |_ \_|  |   \ | |
  | '    ' |    |  _| _   |  _|     | |\ \| |
   \ \__/ /    _| |__/ | _| |_     _| |_\   |_
    `.__.'    |_________|_____|   |_____|_____|
 ____________         __                 __
|_   _||_   _|       [  |               [  |  _
  | |    | | _ .--.   | |  .--.   .---.  | | / ] .---.  _ .--.
  | '    ' |[ `.-. |  | |/ .'`\ \/ /'`\] | '' < / /__\\[ `/'`\]
   \ \__/ /  | | | |  | || \__. || \__.  | |`\ \| \__., | |
    `.__.'  [___||__][___]'.__.' '.___.'[__|  \_]'.__.'[___]

)");
    cout << logo << "Ma"<<"de "<<"by "<<"@"<<"g"<<"amer" << "b" << "ros" << "s_ " << "o"<<"n " << "T"<<"wit" << "te" << "r/X!" << endl << endl; 
    init_buffs();
    toggle_patches();
    while (true) {
        Sleep(100);
        if (GetAsyncKeyState(VK_END))
            break;
    }
    toggle_patches();
    fclose(fp);
    FreeConsole();
    CreateThread(0, 0, EjectThread, 0, 0, 0);
    return 0;
}

bool WINAPI DllMain(HINSTANCE hModule, DWORD dwReason, LPVOID lpReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:
            DllHandle = hModule;
            CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)main_func, NULL, 0, NULL);
        case DLL_THREAD_ATTACH | DLL_THREAD_DETACH:
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}