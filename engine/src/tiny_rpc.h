#ifndef TINY_RPC_H
#define TINY_RPC_H

//#include "pch.h"
#include "tiny_defines.h"
#include "tiny_log.h"
#include <string>
#include <vector>
#include <windows.h>

// WIP. Not done
// this is supposed to be a dope implementation of a cool technique from Game Programming Gems
// where you can register and call arbitrary functions with arbitrary parameters given a function id and payload
// the payload contains the packed data that in actuality is the data for the function parameters, and you
// set up the function stack when calling the function with some asm 
// the idea being that you could do something like  call_rpc(my_function, arbitrary_param_1, arbitrary_data_2);
// and those params would be packed up into a buffer, sent to a remote client, and the client could recognize a function id
// or something and then call that function with the args sent


// thanks Live++

#define TINY_CONCATENATE_HELPER_HELPER(_a, _b)			_a##_b
#define TINY_CONCATENATE_HELPER(_a, _b)					TINY_CONCATENATE_HELPER_HELPER(_a, _b)
#define TINY_CONCATENATE(_a, _b)							TINY_CONCATENATE_HELPER(_a, _b)

// Generates a unique identifier inside a translation unit.
#define TINY_IDENTIFIER(_identifier)						TINY_CONCATENATE(_identifier, __LINE__)

#if defined(__clang__)
#	define TINY_HOOK_(_section, _function, ...)																												\
		extern void (* const TINY_IDENTIFIER(TINY_CONCATENATE(tiny_hook_function, _function)))(__VA_ARGS__) __attribute__((section(_section)));				\
		extern void (* const TINY_IDENTIFIER(TINY_CONCATENATE(tiny_hook_function, _function)))(__VA_ARGS__) __attribute__((section(_section))) = &_function
#elif defined(_MSC_VER)
#	define TINY_HOOK_(_section, _function, ...)																																				\
		__pragma(section(_section, read)) __declspec(allocate(_section)) extern void (* const TINY_IDENTIFIER(TINY_CONCATENATE(tiny_hook_function, _function)))(__VA_ARGS__) = &_function
#else
#	error("Unknown compiler");
#endif

namespace tiny_hooks
{
    typedef struct TinyHookId { char unused; } TinyHookId;
}


#define TINY_HOOK_SECTION						".tiny_hooks"

// Registers a pre-compile hook.
#define TINY_HOOK(_function)					TINY_HOOK_(TINY_HOOK_SECTION, _function, tiny_hooks::TinyHookId, const void* const data, unsigned int size)


struct Function
{
    enum VarType
    {
        VAR_VOID, VAR_BOOL, VAR_INT, VAR_FLOAT, VAR_STRING,
    };
    enum CallType
    {
        CALL_CDECL, CALL_FASTCALL, CALL_STDCALL, CALL_THISCALL,
    };
    typedef std::vector<VarType> ParamVec;
    std::string name;
    void* proc;
    u32 serialID;
    VarType returnType;
    ParamVec paramTypes;
    CallType callType;
};
typedef std::vector<Function> FunctionVec;
FunctionVec g_Functions;




/*
non-float returns stored in eax
eight-byte structures returned in edx and eax as a pair
flaots/doubles returned through FPU in ST0
ret values for user-defined types have addresses pushed onto stack last, but they will also be returned in eax

__cdecl -- caller cleans up the stack. Caller is responsible for popping it's own arguments off stack after call completes.
This is required for variable argument functions b/c called function might not have the info to pop correct num of args (this is default convention for static & global funcs in C/C++)

__stdcall -- the called function cleans up the stack. Standard for win32 api calls (more efficient in client code size)

__fastcall -- ? tbd
__thiscall -- ? tbd


 ---- for setting up function "marking"... maybe look at live++ and their function hook stuff
DbgHelp.dll exports UndecorateSymbolName() which unmangles an exported function signature
__declspec(dllexport) marks a function for exporting

https://en.wikipedia.org/wiki/X86_calling_conventions#Microsoft_x64_calling_convention
https://learn.microsoft.com/en-us/cpp/build/prolog-and-epilog?view=msvc-170&redirectedfrom=MSDN

*/
extern "C"
{
    __declspec(dllimport) void CallCDecl(const void* args, u64 size, void* func);
    typedef void(__cdecl* CallCDeclFunc)(const void* args, u64 size, void* func);
    void CallStdcall(const void* args, u64 size, void* func);
    float GetST0();
    u32 GetEIP();
}

void my_printer(int x)
{
    LOG_INFO("YESSSSSS  %i", x);
}

void tiny_rpc_test_init()
{
    HINSTANCE hinstLib = LoadLibraryA("lib/tiny_MASM.dll");
    CallCDeclFunc cdeclfunc;
    // If the handle is valid, try to get the function address.
    if (hinstLib != NULL)
    {
        cdeclfunc = (CallCDeclFunc)GetProcAddress(hinstLib, "CallCDecl");
    }
    LOG_INFO("tiny_rpc_test_init");
    int* args = new int[1]; 
    args[0] = 9;
    u64 size = 4;
    cdeclfunc(args, size, &my_printer);
    //CallCDecl(args, size, &my_printer); 
    //CallStdcall(0, 0, 0);
    //float x = GetST0();
    //DWORD eip = GetEIP();
    //std::cout << x << eip;
    LOG_INFO("rpc success");
}



#endif