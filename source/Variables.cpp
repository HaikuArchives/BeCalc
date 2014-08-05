
#include <Path.h>
#include <FindDirectory.h>
#include <Directory.h>
#include <File.h>
#include <View.h>
#include <stdio.h>
#include <strings.h>

#include "BinaryRider.h"
#include "MListView.h"
#include "Colors.h"
#include "Variables.h"

BString VarItem::tmp;
int32 VarItem::count = 0;
float VarItem::width = 32.0;

static const char *VarName = "variables";
static const char *VARS = "var";

MyListView *Variables::Vars;

// VarItem constructor

VarItem::VarItem (Preference *r) : MyItem(r)
{
	r->FindInt32(VARS, count, &orgOrder);
	Complex::Read(r, LabelStr(), varValue);
	count++;
}

// definition of VarItem methods

const char * VarItem::ContentStr() const
{
	Complex::ComplexToStr(varValue, tmp);
	return tmp.String();
}

bool VarItem::Write (Preference *w)
{
	MyItem::Write(w);    // write parent fields
	w->AddInt32(VARS, orgOrder);
	Complex::Write(w, LabelStr(), varValue);
	return true;
}

bool VarItem::IsHidden() const
{
	return (LabelStr()[0] == '.');
}

static BListItem * New (Preference *r)
{
	return (new VarItem(r));
}

// compare two BListItems and return -1 if first < second,
// 0 if first = second, and 1 if first > second
int Variables::Compare (const VarItem** first, const VarItem** second)
{
	BString s1((*first)->LabelStr());
	BString s2((*second)->LabelStr());
	if (IsSensitive()) return s1.Compare(s2);
	else return s1.ICompare(s2); 
}

int Variables::Original (const VarItem** first, const VarItem** second)
{
	int32 v1 = (*first)->orgOrder;
	int32 v2 = (*second)->orgOrder;
	return (v1-v2);
}

void Variables::InitSettings()
{
	Settings::SetBool(VARSORT, false);
	Settings::SetBool(VARCASE, true);
}

// variable methods
bool Variables::Read ()
{
	VarItem::count = 0;
	Vars = new MyListView("dummy", VarName, New);
	if (Vars->IsEmpty()) DefaultVariables();
	return (Vars != NULL);
}

bool Variables::Write ()
{
	VarItem::count = 0;
	bool ok = Vars->Write(VarName);
	delete Vars; Vars = NULL;
	return ok;
}

int32 Variables::Find(const char *var)
{
	int32 pos = 0;
	VarItem *item;
	BString v(var);
	BString is;
	
	while (true) {
		item = cast_as(Vars->ItemAt(pos), VarItem);
		if (item == NULL) return NotFound;
		is.SetTo(item->LabelStr());
		if (IsSensitive()) {
			if (v.Compare(is) == 0) return pos;
		} else {
			if (v.ICompare(is) == 0) return pos;			
		}
		pos++;
	}
}

int32 Variables::Defined(void)
{
	return Vars->CountItems();
}

void Variables::Set(const char *var, const Complex& value)
{
	VarItem *item;
	int32 pos;
	
	// attempt to find the current item
	if (Vars == NULL) return;
	pos = Find(var);
	
	if (pos == NotFound) {		
		// append variable to end of list
		item = new VarItem(var, value);
		Vars->AddItem(item);
		if (IsSorted()) Vars->SortItems((int (*)(const void*, const void*))Compare);
	} else {
		// update an existing variable's value
		item = cast_as(Vars->ItemAt(pos), VarItem);
		item->varValue = value;
		Vars->InvalidateItem(pos);  // refresh the item
	}
}

void Variables::SetSorted (bool sorted)
{
	Settings::SetBool(VARSORT, sorted);
	if (sorted) Vars->SortItems((int (*)(const void*, const void*))Compare);
	else Vars->SortItems((int (*)(const void*, const void*))Original);
}

void Variables::Get(const char *var, Complex& value, bool& ok)
{
	int32 pos;
	VarItem *item;
	
	if (Vars == NULL) {
		// vars are unloaded but don't produce an error
		value = Complex::Ex0;
		ok = true; 
		return;
	}
	pos = Find(var);
	if (pos == NotFound) {
		ok = false;
		value = Complex::Ex0;
	} else {
		ok = true;
		item = cast_as(Vars->ItemAt(pos), VarItem);
		value = item->varValue;
	}
}

void Variables::CopyVars(MyListView *list)
{
	BListItem *item;
	int32 cnt = 0;
	list->MakeEmpty();
	while ((item = Vars->ItemAt(cnt++)) != NULL) {
		VarItem *var = static_cast <VarItem *> (item);
		if (!var->IsHidden()) list->AddItem(item);
	}
}

void Variables::Delete(int32 index)
{
	// item was already deleted
	Vars->RemoveItem(index);
}

void Variables::DeleteAll()
{
	int32 pos = Vars->CountItems()-1;
	while (pos >= 0) {
		VarItem * var = static_cast <VarItem *>(Vars->ItemAt(pos));
		if (!var->IsHidden()) Vars->RemoveItem(pos);
		pos--;
	}
}


void Variables::DefaultVariables(void)
{
	Set("M0", Complex::Ex0);
	Set("x", Complex::Ex0);
}
