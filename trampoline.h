#ifndef TRAMPOLINE_H
#define TRAMPOLINE_H

#include <windows.h>

// Registers the hook, creates the trampoline, and patches the target address
void* register_trampoline(void* hook_func, DWORD target_addr, size_t stolen_len);

// Internal C-style helpers for the map lookup (safe to call from asm)
void* get_trampoline_addr(void* hook_func);
DWORD get_return_addr(void* hook_func);

#define NAKED_HOOK(name) void __declspec(naked) name()

#endif
