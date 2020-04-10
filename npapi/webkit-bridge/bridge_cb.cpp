#include <windows.h>
#include "npfunctions.h"
#include <bridge.h>
#include <bridge_cb.h>

static BridgeCB_Callback callback_;
static void *user_data_;

VOID WINAPI BridgeCB_set_callback(BridgeCB_Callback callback, void *user_data) {
    callback_ = callback;
    user_data_ = user_data;
    //WriteLog("BridgeCB_set_callback: the callback is set.");
}

void BridgeCB_set_callback_if_none(BridgeCB_Callback callback, void *user_data) {
    if (callback_ == 0) {
        BridgeCB_set_callback(callback, user_data);
    } else {
        //WriteLog("BridgeCB_set_callback_if_none: the callback has been already set.");
    }
}

BOOL WINAPI BridgeCB_allocate(BridgeCB_String *string, unsigned size) {
    void *memory = NPN_MemAlloc(size);
    if (memory == 0) {
        //WriteLog("BridgeCB_allocate: NPN_MemAlloc(%u) returns 0.", size);
        return FALSE;
    }
    string->size = size;
    string->data = (char *)memory;
    return TRUE;
}

bool BridgeCB_call(const BridgeCB_String *request, BridgeCB_String *result) {
    return (*callback_)(request, result, user_data_) != 0;
}
