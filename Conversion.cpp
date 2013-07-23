
/*
     Conversion window for BeCalc.
*/

#include <stdio.h>
#include <KernelKit.h>
#include <StorageKit.h>
#include <InterfaceKit.h>
#include <AppKit.h>

#include "layout-all.h"

#include "Conversion.h"
#include "Colors.h"
#include "Buttons.h"
#include "ConvertIcons.h"
#include "Equations.h"
#include "ExComplex.h"
#include "Settings.h"
#include "Variables.h"

static const char* FROM	 		= "Convert-from";
static const char* TO 	 		=	"Convert-to";
static const char* TYPE 		= "Convert-type";
static const char* TOLIST		= "Convert-tolist";
static const char* FROMLIST	= "Convert-fromlist";

void ConvertWin::InitSettings()
{
	Settings::SetColor(TOLIST, White);
	Settings::SetColor(FROMLIST, White);
	Settings::SetPosition(CONVERTTITLE, BRect(100, 100, 300, 300));
	Settings::SetInt(FROM, 0);
	Settings::SetInt(TO, 0);
	Settings::SetInt(TYPE, 0);
}

static void AddMime (BMessage &msg, const char *pubname, const char *field)
{
	msg.AddString("attr:public_name", pubname); 
	msg.AddString("attr:name", field); 
	msg.AddInt32("attr:type", B_STRING_TYPE); 
	msg.AddBool("attr:viewable", true); 
	msg.AddBool("attr:editable", true); 
	msg.AddInt32("attr:width", 120); 
	msg.AddInt32("attr:alignment", B_ALIGN_LEFT); 
	msg.AddBool("attr:extra", false); 
}

// these methods read the external conversion mime type

void ConvertWin::ReadTypes()
{
	char			*offset;
	char			str[256];
	char			*text;
	char			*text1;
	int32			count = 0;
	int32			index;
	BEntry			entry;
	BFile			file;
	BMenuItem		*item;
	BQuery		query;
	BVolume		vol;
	BVolumeRoster	volume;
	attr_info		info;

	while ((item = types->ItemAt(0))) {
		types->RemoveItem(item);
		delete item;
	}

	volume.GetBootVolume(&vol);
	query.SetVolume(&vol);
	sprintf(str, "%s=*", P_TYPE);
	query.SetPredicate(str);
	query.Fetch();

	while (query.GetNextEntry(&entry) == B_NO_ERROR) {
		file.SetTo(&entry, O_RDONLY);
		if ((file.InitCheck() == B_NO_ERROR) &&
			(file.GetAttrInfo(P_TYPE, &info) == B_NO_ERROR) &&
			(info.size > 1)) {
			text = (char *)malloc(info.size);
			text1 = text;
			file.ReadAttr(P_TYPE, B_STRING_TYPE, 0, text, info.size);
			while (1) {
				if ((offset = strstr(text, ",")))
					*offset = 0;
				if (!types->FindItem(text)) {
					index = 0;
					while ((item = types->ItemAt(index))) {
						if (strcmp(text, item->Label()) < 0)
							break;
						index++;
					}
					BMessage *msg = new BMessage(M_POPUP_SELECTED);
					msg->AddInt32("index", count);
					msg->AddPointer("popup", ptypes);
					types->AddItem(new BMenuItem(text, msg), index);
					count++;
				}
				if (offset) {
					text = offset + 1;
					while (*text == ' ')
						text++;
				}
				else
					break;
			}
			free(text1);
		}
	}

	if (!count) {
		types->AddItem(item = new BMenuItem("none", NULL));
		item->SetEnabled(FALSE);
	}
}

int ConvertWin::Compare (const BStringItem **first, const BStringItem **second)
{
	return strcmp((*first)->Text(), (*second)->Text());
}

void ConvertWin::GetUnits (int32 tindex, MListView *v)
{	
	char			str[256];
	char			*text;
	BEntry			entry;
	BFile			file;
	BQuery		query;
	BVolume		vol;
	BVolumeRoster	volume;
	attr_info		info;
	
	// remove the existing list items
	BListItem *item;
	while (v->FirstItem() != NULL) {
		item = v->RemoveItem((int32)0);
		delete item;
	}
	
	// define the new list items
	volume.GetBootVolume(&vol);
	query.SetVolume(&vol);
	BMenuItem *mitem = types->ItemAt(tindex);
	sprintf(str, "%s=\"%s\"", P_TYPE, mitem->Label());
	query.SetPredicate(str);
	query.Fetch();

	while (query.GetNextEntry(&entry) == B_NO_ERROR) {
		file.SetTo(&entry, O_RDONLY);
		if ((file.InitCheck() == B_NO_ERROR) &&
			(file.GetAttrInfo(P_UNIT, &info) == B_NO_ERROR) &&
			(info.size > 1)) {
			text = (char *)malloc(info.size);
			file.ReadAttr(P_UNIT, B_STRING_TYPE, 0, text, info.size);
			v->AddItem(new BStringItem(text));
			free(text);
		}
	}
	
	// sort them alphabetically
	v->SortItems((int (*)(const void*, const void*))Compare);
}

void ConvertWin::GetFactorDef(int32 unit, char *from, char *to, char *con)
{
	char			str[256];
	BEntry		entry;
	BFile			file;
	BQuery		query;
	BVolume	vol;
	BVolumeRoster	volume;
	attr_info	info;
		
	// get additional attributes
	volume.GetBootVolume(&vol);
	query.SetVolume(&vol);
	BStringItem *sitem = (BStringItem *)fromlist->ItemAt(unit);
	sprintf(str, "%s=\"%s\"", P_UNIT, sitem->Text());
	query.SetPredicate(str);
	query.Fetch();

	if (query.GetNextEntry(&entry) == B_NO_ERROR) {
		file.SetTo(&entry, O_RDONLY);
		if (file.InitCheck() == B_NO_ERROR) {
			if ((file.GetAttrInfo(P_TOEQUATION, &info) == B_NO_ERROR) && (info.size > 1)) {
				file.ReadAttr(P_TOEQUATION, B_STRING_TYPE, 0, to, info.size);
			}
			if ((file.GetAttrInfo(P_FROMEQUATION, &info) == B_NO_ERROR) && (info.size > 1)) {
				file.ReadAttr(P_FROMEQUATION, B_STRING_TYPE, 0, from, info.size);
			}
			if ((file.GetAttrInfo(P_CONSTANT, &info) == B_NO_ERROR) && (info.size > 1)) {
				file.ReadAttr(P_CONSTANT, B_STRING_TYPE, 0, con, info.size);
			}
		}		
	}
}

// The MWindow receives font-change messages from the application.
// It also takes care of color-drops. 
ConvertWin::ConvertWin(char *name, window_type type) : PWindow(name, type)
{
	const char *str;
	bool valid = false;
	int32 index = 0;
	MView *topview;
	BVolumeRoster roster;
	BVolume vol;
	BMimeType mime;
	BMessage info, msg;
	BBitmap large_icon(BRect(0, 0, B_LARGE_ICON-1, B_LARGE_ICON-1), B_COLOR_8_BIT);
	BBitmap mini_icon(BRect(0, 0, B_MINI_ICON-1, B_MINI_ICON-1), B_COLOR_8_BIT);
	
	// create the conversion mime type if it doesn't exist
	roster.GetBootVolume(&vol);
	fs_create_index(vol.Device(), P_TYPE, B_STRING_TYPE, 0);
	fs_create_index(vol.Device(), P_UNIT, B_STRING_TYPE, 0);
	fs_create_index(vol.Device(), P_TOEQUATION, B_STRING_TYPE, 0);
	fs_create_index(vol.Device(), P_FROMEQUATION, B_STRING_TYPE, 0);
	fs_create_index(vol.Device(), P_CONSTANT, B_STRING_TYPE, 0);

	// install person mime type
	mime.SetType(B_CONVERT_MIMETYPE);
	if (mime.IsInstalled()) {
		if (mime.GetAttrInfo(&info) == B_NO_ERROR) {
			while (info.FindString("attr:name", index++, &str) == B_NO_ERROR) {
				if (!strcmp(str, P_TYPE)) {
					valid = TRUE;
					break;
				}
			}
			if (!valid)
				mime.Delete();
		}
	}	

	if (!valid) {
		mime.Install();
		large_icon.SetBits(kLargeConvertIcon, large_icon.BitsLength(), 0, B_COLOR_8_BIT);
		mini_icon.SetBits(kSmallConvertIcon, mini_icon.BitsLength(), 0, B_COLOR_8_BIT);
		mime.SetShortDescription("BeCalc Conversion");
		mime.SetLongDescription("Conversion data for a given unit.");
		mime.SetIcon(&large_icon, B_LARGE_ICON);
		mime.SetIcon(&mini_icon, B_MINI_ICON);
		mime.SetPreferredApp(APP_SIG);

		// add relevant person fields to meta-mime type
		AddMime(msg, "Type of Conversion", P_TYPE);
		AddMime(msg, "Conversion Unit", P_UNIT);
		AddMime(msg, "To Equation", P_TOEQUATION);
		AddMime(msg, "From Equation", P_FROMEQUATION);
		AddMime(msg, "Conversion Constant", P_CONSTANT);
		mime.SetAttrInfo(&msg);
	}	
	
	// define the top two scrolling lists
	HGroup *lists=
		new HGroup
		(
			new VGroup
			(
				new MStringView("From Units", B_ALIGN_CENTER),
				new MScrollView
				(
					fromlist=new PListView(FROMLIST),
					false, true
				),
			0),
			new VGroup
			(
				new MStringView("To Units", B_ALIGN_CENTER),
				new MScrollView
				(
					tolist=new PListView(TOLIST),
					false, true
				),
			0),
		0);
		
    
    // define bottom controls	    
	HGroup *controls=
		new HGroup
			(
				ptypes = new MPopup("Units:", 0),
				new MButton("Convert From -> To", CID_CONVERSIONS), 
			 	new MButton("Convert To -> From", CID_INVERSE),
			 0);
			 
    // define the units
    types = ptypes->Menu();
    ReadTypes();
			 
	// set up lists for the default unit
	GetUnits(Settings::GetInt(TYPE), fromlist);
	GetUnits(Settings::GetInt(TYPE), tolist);
		
	// Build up the final view with the display, equation entry, and buttons
	topview= new MBorder
				(
					M_RAISED_BORDER, 3, "outer",
					new VGroup
					(
						lists,
						controls,
					0)
				);

	// because topview is an MView, we need to cast it to a BView.
	AddChild(dynamic_cast<BView*>(topview));
	Show();
}

void ConvertWin::InitWindow ()
{  
	// set the defaults  
	ptypes->SetActive(Settings::GetInt(TYPE), false);
	
	// check if a selection already exists
	fromlist->Select(Settings::GetInt(FROM));
	tolist->Select(Settings::GetInt(TO));
}

bool ConvertWin::QuitRequested()
{
	// destroy all the list items
	BListItem *item;
	PWindow::QuitRequested();
	
	// save the active list items and the current type
	Settings::SetInt(TO, tolist->CurrentSelection());
	Settings::SetInt(FROM, fromlist->CurrentSelection());	
	
	while (fromlist->FirstItem() != NULL) {
		item = fromlist->RemoveItem((int32)0);
		delete item;
		item = tolist->RemoveItem((int32)0);
		delete item;
	}	
	
	// unghost the mortgage button
	if (target->Lock()) {
    	DispatchMessage(new BMessage(ID_CLOSE_CONVERSIONS), target);
    	target->Unlock();
	}
	
	// shut down this window
	Quit();  	 // call the parent
	return true;
}

// Conversion routine to convert `n'
// assumed to be in "from" units to the
// "to" units.
void ConvertWin::DoConvert(Real &n, bool inverse) 
{
	Real One(1.0);
	char fromeq[64], toeq[64], con[64];
	Complex result, tmp;
	int32 from, to;
	
	if (inverse) {
		from	= tolist->CurrentSelection();
		to		= fromlist->CurrentSelection();
	} else {	
		from	= fromlist->CurrentSelection();
		to		= tolist->CurrentSelection();
	}
	
	if (from == to) return;  // nothing to do
	
	// convert to standard units	
	GetFactorDef(from, fromeq, toeq, con);
	if (strlen(con)>0) Equations::Evaluate(con, tmp, false);  	// define the constant "k"
	Variables::Set("m", Complex(n));									// define the dependent variable "m"
	Equations::Evaluate(toeq, result, false);
	
	// convert to the desired units
	GetFactorDef(to, fromeq, toeq, con);
	if (strlen(con)>0) Equations::Evaluate(con, tmp, false);  	// define the constant "k"
	Variables::Set("m", result);									 		// define the dependent variable "m"
	Equations::Evaluate(fromeq, result, false);
	n = result.RealPart();
}

// Always call the inherited MessageReceived(), unless you don't
// want your windows to react to font-changes and color-drops.
void ConvertWin::MessageReceived(BMessage *message)
{
	int32 id;
	void * ptr;
	
	switch(message->what)
	{
		case M_BUTTON_SELECTED:
			// redirect the message to main window
    		DispatchMessage(message, target);			
			break;
		case M_POPUP_SELECTED:
			id = message->FindInt32("index");
			Settings::SetInt(TYPE, id);
			message->FindPointer("popup", &ptr);
			if (ptr == ptypes) {
			 	// set up appropriate lists
				GetUnits(id, fromlist);
				GetUnits(id, tolist);
				
				// select a default list item
				fromlist->Select(0);
				tolist->Select(1);		
			}	
			MWindow::MessageReceived(message);
			break;
		default:
			//printf("received by window:\n");
			//message->PrintToStream();
			//printf("----------\n");
			MWindow::MessageReceived(message);
			break;
	}
}
