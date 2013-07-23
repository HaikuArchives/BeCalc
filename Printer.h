#ifndef PRINTER_H
#define PRINTER_H

#include <Rect.h>
#include <Message.h>
#include <PrintJob.h>

#include "MyListView.h"

class Printer
{
public:
  	// constructor and destructor
	Printer(void) {};
	~Printer() {};
	
	// methods
	static status_t Setup(const char *name);
	static void Print(const char *name, MyListView *view1, MyListView *view2 = NULL, MyListView *view3 = NULL);
	
private:
	static BMessage *setup;
	static BRect paper, printable;
	static int32 pageCount;
};

#endif
