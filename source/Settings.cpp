#include <Application.h>

#include "FFont.h"
#include "Settings.h"
#include "Colors.h"

// names for preferences
const char *POSITION = "-size";
const char *FONT = "-font";
const char *BFONT = "-bfont";
const char *FFONT = "-ffont";
const char *COLOR = "-color";

Preference Settings::pref;

BRect Settings::GetPosition(const char *name)
{
	BString pname(name);
	pname.Append(POSITION);
	BRect pos = pref.FindRect(pname.String());
	FitRectOnScreen(&pos);
	return pos;
}

int32 Settings::GetInt (const char *name) 
{
	int32 value;
	if (pref.FindInt32(name, &value) != B_OK) return 0;
	else return value;
}

bool Settings::GetBool (const char *name)
{
	bool value;
	if (pref.FindBool(name, &value) != B_OK) return false;
	else return value;
}

double Settings::GetFloat (const char *name)
{
	double value;
	if (pref.FindDouble(name, &value) != B_OK) return 0;
	else return value;
}

const char * Settings::GetString (const char *name)
{
	const char *str;
	if (pref.FindString(name, &str) != B_OK) return NULL;
	else return str;
}

rgb_color Settings::GetColor(const char *name)
{	
	BString cname(name); cname.Append(COLOR);
	rgb_color *color;
	ssize_t bytes = sizeof(rgb_color);	
	if (pref.FindData(cname.String(), B_RGB_COLOR_TYPE, (const void **)&color, &bytes) == B_OK)
		return (*color);
	else
		return (BeBackgroundGrey);
}

BFont * Settings::GetFont(const char *name, fontspec family)
{	
	BString pname(name);
	FFont f;
	if (family == M_PLAIN_FONT) pname.Append(FONT);
	else if (family == M_BOLD_FONT) pname.Append(BFONT);
	else pname.Append(FFONT);
	FindMessageFont(&pref, pname.String(), 0, &f);
	return (new FFont(f));
}

void Settings::SetPosition(const char *name, const BRect pos)
{
	BString pname(name);
	pname.Append(POSITION);
	if (!pref.HasRect(pname.String())) 
		pref.AddRect(pname.String(),  pos);
	else 
		pref.ReplaceRect(pname.String(), pos);
}

void Settings::SetFont(const char *name, const BFont *font, fontspec family)
{
	BString pname(name);
	FFont ff(font);
	FFont dummy;
	if (family == M_PLAIN_FONT) pname.Append(FONT);
	else if (family == M_BOLD_FONT) pname.Append(BFONT);
	else pname.Append(FFONT);
	if (FindMessageFont(&pref, pname.String(), 0, &dummy) != B_OK)
		AddMessageFont(&pref, pname.String(), &ff);
	else
		pref.ReplaceFlat(pname.String(), &ff);
}

void Settings::SetInt(const char *name, int32 val)
{
	if (!pref.HasInt32(name)) pref.AddInt32(name, val);
	else pref.ReplaceInt32(name, val);
}

void Settings::SetBool(const char *name, bool val)
{
	if (!pref.HasBool(name)) pref.AddBool(name, val);
	else pref.ReplaceBool(name, val);
}

void Settings::SetFloat(const char *name, double val)
{
	if (!pref.HasDouble(name)) pref.AddDouble(name, val);
	else pref.ReplaceDouble(name, val);
}

void Settings::SetString(const char *name, const char *str)
{
	if (!pref.HasString(name)) pref.AddString(name, str);
	else pref.ReplaceString(name, str);
}

void Settings::SetColor(const char *name, const rgb_color color)
{
	BString cname(name); cname.Append(COLOR);
	ssize_t bytes = sizeof(rgb_color);
	if (!pref.HasData(cname.String(), B_RGB_COLOR_TYPE)) 
		pref.AddData(cname.String(),  B_RGB_COLOR_TYPE, &color, bytes);
	else 
		pref.ReplaceData(cname.String(), B_RGB_COLOR_TYPE, &color, bytes);
}
