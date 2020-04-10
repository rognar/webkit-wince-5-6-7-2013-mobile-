#ifndef BRIDGE_CB_H_
#define BRIDGE_CB_H_

typedef struct {
    unsigned size;
    char *data;
} BridgeCB_String;

typedef int (* BridgeCB_Callback)(const BridgeCB_String *request, BridgeCB_String *result, void *user_data);
typedef VOID (WINAPI *BridgeCB_SetCallback)(BridgeCB_Callback callback, void *user_data);
typedef BOOL (WINAPI *BridgeCB_Allocate)(BridgeCB_String *string, unsigned size);

extern "C" VOID WINAPI BridgeCB_set_callback(BridgeCB_Callback callback, void *user_data);
extern void BridgeCB_set_callback_if_none(BridgeCB_Callback callback, void *user_data);
extern "C" BOOL WINAPI BridgeCB_allocate(BridgeCB_String *string, unsigned size);
extern bool BridgeCB_call(const BridgeCB_String *request, BridgeCB_String *result);

#endif // BRIDGE_CB_H_
