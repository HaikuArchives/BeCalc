#ifndef VARIABLES_H
#define VARIABLES_H

#include <ListItem.h>
#include <String.h>

#include "TextRider.h"
#include "MyItem.h"
#include "MyListView.h"
#include "ExComplex.h"
#include "Printer.h"
#include "Settings.h"

// define a list of variables
class VarItem : public MyItem {
public:
	friend class Variables;
	VarItem (const char *var, const Complex &value): 
		MyItem(var), orgOrder(count), varValue(value) {count++;};
	VarItem (Preference *r);
	virtual const char* ContentStr() const;
	virtual bool IsHidden() const;
	virtual const char* DragStr() const {return LabelStr();};
	virtual float MaxWidth() {return width;};
	virtual void SetMaxWidth(float w) {width = w;};
	virtual bool Write (Preference *w);
private:
	static float width;
	static BString tmp;
	static int32 count;
	int32 orgOrder;			// used to maintain original list order
	Complex varValue;		// variables's current value
};

#define VARSORT	"Sorted list"
#define VARCASE	"Case sensitive"		

class Variables
{
public:	
	// methods
	static int32 Defined(void);
	static int32 Find(const char *var);
	static void Set(const char *var, const Complex& value);
	static void Get(const char *var, Complex& value, bool& ok);
	static void CopyVars(MyListView *list);
	static void Delete(int32 index);
	static void DeleteAll();
	static void Export(TextWriter &w) {Vars->Export(w);};
	static void SetSorted (bool sorted);
	static void SetCaseSensitive (bool areSensitive) {Settings::SetBool(VARCASE, areSensitive);};
	static bool IsSorted () {return Settings::GetBool(VARSORT);};
	static bool IsSensitive() {return Settings::GetBool(VARCASE);};
	static bool Write();
	static bool Read();
	static void InitSettings();
	static void DefaultVariables(void);
	
private:
	static const int32 NotFound = -1;
	static MyListView* Vars;
	static int Compare(const VarItem** first, const VarItem** second);
	static int Original(const VarItem** first, const VarItem** second);
};

#endif
