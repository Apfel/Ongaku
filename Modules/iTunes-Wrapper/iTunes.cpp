#include "iTunes.h"

#include <Windows.h>
#include <comdef.h>

#include <iTunesCOMInterface.h>

struct iTunes_Instance
{
    IiTunes*    itunes;
    DWORD       last_error;
};

iTunes_Instance* iTunes_New()
{
    iTunes_Instance* instance = new iTunes_Instance;
    if (instance == NULL) return NULL;

    return instance;
}

uint8_t iTunes_Initialize(iTunes_Instance* instance)
{
    if (instance == NULL) return iTunes_Result_Invalid_Instance;

    HRESULT result = ::CoInitialize(NULL);
    if (FAILED(result))
    {
        instance->last_error = GetLastError();
        return iTunes_Result_COM_Failure;
    }

    result = ::CoCreateInstance(CLSID_iTunesApp, NULL, CLSCTX_LOCAL_SERVER, IID_IiTunes, (PVOID*)&instance->itunes);
    if (FAILED(result))
    {
        instance->last_error = GetLastError();

        switch (result)
        {
        case REGDB_E_CLASSNOTREG:
            return iTunes_Result_iTunes_Missing;

        default:
            return iTunes_Result_COM_Failure;
        }
    }

    return iTunes_Result_Success;
}

uint8_t iTunes_Get_State(iTunes_Instance* instance, iTunes_State* state)
{
    if (instance == NULL) return iTunes_Result_Invalid_Instance;
    else if (state == NULL) return iTunes_Result_Invalid_Arguments;

    ITPlayerState player_state;
    HRESULT result = instance->itunes->get_PlayerState(&player_state);
    if (FAILED(result))
    {
        instance->last_error = result;
        return iTunes_Result_COM_Failure;
    }

    switch (player_state)
    {
    case ITPlayerStatePlaying:
        state->State = iTunes_Player_State_Playing;
        break;
    
    default:
        state->State = iTunes_Player_State_Stopped;
        break;
    }

    IITTrack* track = NULL;
    result = instance->itunes->get_CurrentTrack(&track);
    if (FAILED(result))
    {
        instance->last_error = result;
        return iTunes_Result_COM_Failure;
    }

    result = track->get_Name(&state->Title);
    if (FAILED(result))
    {
        instance->last_error = result;
        return iTunes_Result_COM_Failure;
    }

    result = track->get_Artist(&state->Artist);
    if (FAILED(result))
    {
        instance->last_error = result;
        return iTunes_Result_COM_Failure;
    }

    result = track->get_Album(&state->Album);
    if (FAILED(result))
    {
        instance->last_error = result;
        return iTunes_Result_COM_Failure;
    }

    result = track->get_Duration(&state->Length);
    if (FAILED(result))
    {
        instance->last_error = result;
        return iTunes_Result_COM_Failure;
    }

    result = instance->itunes->get_PlayerPosition(&state->Position);
    if (FAILED(result))
    {
        instance->last_error = result;
        return iTunes_Result_COM_Failure;
    }

    return iTunes_Result_Success;
}

DWORD iTunes_GetLastError_Proxy(iTunes_Instance* instance)
{
    return instance == NULL ? 0 : instance->last_error;
}

void iTunes_Destroy(iTunes_Instance* instance)
{
    if (instance->itunes != NULL) instance->itunes->Release();
}
