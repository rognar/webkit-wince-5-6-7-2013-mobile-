#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#include "npapi.h"
#include "npruntime.h"
#include "nullplugin.h"
#include "ScriptablePluginObjectBase.h"

class CBridgePlugin
{
private:
  NPP m_pNPInstance;

  NPWindow * m_Window;
  NPStream * m_pNPStream;
  NPBool m_bInitialized;
  NPObject *m_pScriptableObject;
  PluginInstance nullPluginInstance;

public:
  CBridgePlugin(NPP pNPInstance);
  ~CBridgePlugin();

  NPBool init(NPWindow* pNPWindow);
  void shut();
  NPBool isInitialized();
  
  NPError SetWindow(NPWindow* pNPWindow);

  NPObject *GetScriptableObject();
};

class BridgePluginObject : public ScriptablePluginObjectBase
{
public:
	BridgePluginObject (NPP npp);
	virtual ~BridgePluginObject();

  virtual bool Construct(const NPVariant *args, uint32_t argCount,
                         NPVariant *result);
  	char* npStrDup(const char* sIn);
	virtual bool HasMethod(NPIdentifier name);
	virtual bool HasProperty(NPIdentifier name);
	virtual bool GetProperty(NPIdentifier name, NPVariant *result);
	virtual bool SetProperty(NPIdentifier name, const NPVariant *value);
	virtual bool Invoke(NPIdentifier name, const NPVariant *args,
		uint32_t argCount, NPVariant *result);
	virtual bool InvokeDefault(const NPVariant *args, uint32_t argCount,
		NPVariant *result);
};

extern void WriteLog(char *pformat, ...);

#endif // __PLUGIN_H__
