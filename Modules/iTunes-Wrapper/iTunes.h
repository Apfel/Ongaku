// Copyright (c) 2020 Apfel
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software.
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

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
