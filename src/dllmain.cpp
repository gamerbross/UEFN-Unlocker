#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <memcury.h>

#define WIN32_LEAN_AND_MEAN

const char* logo =
R"( _____  _____  ________  ________  ____  _____
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

)";

static auto currentProcess = GetCurrentProcess();

void PatchPlainJump(LPVOID address) {
    static const std::vector<BYTE> fromBuffer = { 0x74 };
    static const std::vector<BYTE> toBuffer = { 0xEB };

    WriteProcessMemory(currentProcess, address, toBuffer.data(), toBuffer.size(), NULL);
}

void Main(const HMODULE hModule) {
    AllocConsole();
    SetConsoleTitleA("UEFN Unlocker by gamerbross");
    FILE* pFile;
    freopen_s(&pFile, ("CONOUT$"), "w", stdout);

    std::cout << logo << "Made by @gamerbross_ on X/Twitter!\n";

    const wchar_t* JumpEqualStrings[] = // 0F 84 -> 0F 83
    {
          L"Error_CannotModifyCookedAssets" // Cannot modify cooked assets
        , L"NotifyBlockedByCookedAsset"     // Unable to Edit cooked asset
        , L"Alias asset '{0}' is in a read only folder. Unable to edit read only assets."
        , L"Alias asset '{0}' is in a folder that does not allow edits. Unable to edit read only assets."
    };
    for (auto jes : JumpEqualStrings) {
        static const std::vector<BYTE> fromBuffer = { 0x0F, 0x84 };
        static const std::vector<BYTE> toBuffer   = { 0x0F, 0x83 };

        WriteProcessMemory(currentProcess, Memcury::Scanner::FindStringRef(jes).ScanFor(fromBuffer, false).GetAs<LPVOID>(), toBuffer.data(), toBuffer.size(), NULL);
    }

    const wchar_t* JumpPlainStrings[] = // 74 -> EB
    {
          L"Error_CannotModifyGeneratedClasses" // Cannot modify generated classes
        , L"Package is cooked or missing editor data\n"
    };
    for (auto jps : JumpPlainStrings) {
        PatchPlainJump(Memcury::Scanner::FindStringRef(jps).ScanFor({ 0x74 }, false).GetAs<LPVOID>());
    }

    const char* JumpPlainSignatures[] =  // TODO: Get the jump address using a more dynamic way
    {
          "74 28 4C 8B 33" // 1st Cannot duplicate cooked asset
        , "74 26 49 8B 04 24 49 8B CC FF 50 40 FF" // Folder is locked
    };
    for (auto jpsig : JumpPlainSignatures) {
        PatchPlainJump(Memcury::Scanner::FindPattern(jpsig).GetAs<LPVOID>());
    }

    const wchar_t* JumpNotEqualStrings[] = // 0F 85 -> 0F 82
    {
            L"Folder '{0}' is read only and its contents cannot be edited"
          , L"Asset '{0}' is in a folder that does not allow edits. Unable to edit read only assets."
    };
    for (auto jnes : JumpNotEqualStrings) {
        static const std::vector<BYTE> fromBuffer = { 0x0F, 0x85 };
        static const std::vector<BYTE> toBuffer   = { 0x0F, 0x81 };

        WriteProcessMemory(currentProcess, Memcury::Scanner::FindStringRef(jnes).ScanFor(fromBuffer, false).GetAs<LPVOID>(), toBuffer.data(), toBuffer.size(), NULL);
    }

    std::cout << "Done!\n";
}

BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD dwReason, const LPVOID lpReserved)
{
    if (dwReason != DLL_PROCESS_ATTACH)
        return TRUE;

    auto mainThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)Main, hModule, 0, NULL);
    if (mainThread)
        CloseHandle(mainThread);

    return TRUE;
}