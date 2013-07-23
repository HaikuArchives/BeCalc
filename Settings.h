#ifndef SETTINGS_H
#define SETTINGS_H

#include "layout-all.h"
#include "FFont.h"
#include "Preference.h"
#include <SupportKit.h>
#include <InterfaceKit.h>

class Settings
{
	public:	
		static status_t Load() {return pref.Load();}
		static status_t Save() {return pref.Save();}
		static status_t Use() {return pref.Use();}
	
		static BRect GetPosition (const char *name);
		static rgb_color GetColor(const char *name);
		static BFont * GetFont(const char *name, fontspec family);
		static int32 GetInt (const char *name);
		static bool GetBool (const char *name);
		static double GetFloat (const char *name);
		static const char * GetString(const char *name);
		
		static void SetPosition(const char *name, const BRect pos);
		static void SetColor(const char *name, const rgb_color color);
		static void SetFont(const char *name, const BFont *font, fontspec family);
		static void SetInt(const char *name, int32 val);
		static void SetBool(const char *name, bool b);
		static void SetFloat(const char *name, double f);
		static void SetString(const char *name, const char *str);
		
	private:
		static Preference pref;
};

#endif
