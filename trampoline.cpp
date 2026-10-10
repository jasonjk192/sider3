#include "trampoline.h"
#include <unordered_map>

struct HookData 
{
    void* trampoline;
    DWORD return_addr;
};

// Internal storage hidden inside trampoline.cpp
static std::unordered_map<void*, HookData> g_hookMap;

void* register_trampoline(void* hook_func, DWORD target_addr, size_t stolen_len)
{
    BYTE* trampoline = (BYTE*)VirtualAlloc(NULL, 64, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    if (!trampoline) return nullptr;

    // 1. Copy the stolen bytes
    memcpy(trampoline, (void*)target_addr, stolen_len);

    // 2. Append a RET
    trampoline[stolen_len] = 0xC3; // RET

    // 3. Patch the original game location with a JMP to your custom callback
    DWORD protection = 0;
    VirtualProtect((void*)target_addr, stolen_len, PAGE_EXECUTE_READWRITE, &protection);

    *(BYTE*)target_addr = 0xE9; // JMP rel32
    *(DWORD*)(target_addr + 1) = (DWORD)hook_func - (target_addr + 5);

    for (size_t i = 5; i < stolen_len; i++)
    {
        *(BYTE*)(target_addr + i) = 0x90; // NOP out the rest
    }

    VirtualProtect((void*)target_addr, stolen_len, protection, &protection);

    // 4. Store in the central map mapped to the hook function pointer
    g_hookMap[hook_func] = { trampoline, target_addr + stolen_len };
    return trampoline;
}

void* get_trampoline_addr(void* hook_func)
{
    auto it = g_hookMap.find(hook_func);
    return (it != g_hookMap.end()) ? it->second.trampoline : nullptr;
}

DWORD get_return_addr(void* hook_func)
{
    auto it = g_hookMap.find(hook_func);
    return (it != g_hookMap.end()) ? it->second.return_addr : 0;
}