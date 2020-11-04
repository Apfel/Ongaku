#ifndef MODULES_ITUNES_WRAPPER_ITUNES_H
#define MODULES_ITUNES_WRAPPER_ITUNES_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <Windows.h>

#include <stdint.h>

#define ITUNES_API WINAPI

typedef enum
{
    iTunes_Result_Success,
    iTunes_Result_Invalid_Instance,
    iTunes_Result_Invalid_Arguments,
    iTunes_Result_COM_Failure,
    iTunes_Result_iTunes_Missing
} iTunes_Result;

typedef enum
{
    iTunes_Player_State_Stopped,
    iTunes_Player_State_Playing
} iTunes_Player_State;

typedef struct iTunes_Instance iTunes_Instance;

typedef struct _iTunes_State
{
    uint8_t     State;
    wchar_t*    Title;
    wchar_t*    Artist;
    wchar_t*    Album;
    long        Length;
    long        Position;
} iTunes_State;

iTunes_Instance* ITUNES_API iTunes_New();
uint8_t ITUNES_API iTunes_Initialize(iTunes_Instance* instance);
uint8_t ITUNES_API iTunes_Get_State(iTunes_Instance* instance, iTunes_State* state);
DWORD ITUNES_API iTunes_GetLastError_Proxy(iTunes_Instance* instance);
void ITUNES_API iTunes_Destroy(iTunes_Instance* instance);

#ifdef __cplusplus
}
#endif
#endif
