#include <stdio.h>
//#include <View.h>
#include <Message.h>

#include "Errors.h"
#include "Printer.h"

BMessage *Printer::setup = NULL;
BRect Printer::paper, Printer::printable;
int32 Printer::pageCount;
	
status_t Printer::Setup(const char *name)
{
	status_t result;
	BPrintJob job(name);
	result = job.ConfigPage();
	setup = job.Settings();
	return result;
}

void Printer::Print(const char *name, MyListView *view1, MyListView *view2, MyListView *view3)
{
	BPrintJob job(name);
	MyListView *view[3] = {view1, view2, view3};
	
	// call ConfigPage if there is no setup message
	if (setup == NULL)
		if (Setup(name) != B_NO_ERROR) {
			Error("Print cancelled!");
			return;
		}	
	Error("Printing not currently supported");	
	// set up the driver with user settings
	//job.SetSettings(new BMessage(setup));
	
	// prompt user for pages to print
	if (job.ConfigJob() != B_OK) {
		Error("Print job cancelled!");
		return;
	}
	
	// get the paper size
	paper = job.PaperRect();
	printable = job.PrintableRect();
		
	// determine number of pages to print
	int32 firstPage = job.FirstPage();
	int32 lastPage = job.LastPage();
	
	// verify the range is ok
	int32 maxPages = view1->GetMaxPages(printable);
	if (view2 != NULL) maxPages += view2->GetMaxPages(printable);
	if (view3 != NULL) maxPages += view3->GetMaxPages(printable);
	if (lastPage > maxPages) lastPage = maxPages;
	int32 nbPages = lastPage - firstPage + 1;
	
	// verify the range is correct
	if ((nbPages <= 0) || (firstPage < 1)) {
		Error("Incorrect range of pages!");
		return;
	}
	
	// now we can finally print the pages
	job.BeginJob();
		
	// iterate through all three views and print them
	int32 pages = 1;
	int32 max = 0;
	int32 current;
	BRect curPrintable;
	for (int32 j = 0; j < 3; j++) {
		curPrintable = printable;
		if (view[j] != NULL) {
			max = view[j]->GetMaxPages(printable);
			current = view[j]->CurrentSelection();			// don't print selection bar
			view[j]->Deselect(current);
		} else {
			max = 0;
			current = -1;
		}
	
		// print all the pages
		for (int32 i = 1; i<=max; i++) {
			if ((pages >= firstPage) && (pages <= lastPage)) {
				curPrintable.OffsetTo(0, (i-1)*printable.Height());
				job.DrawView(view[j], curPrintable, BPoint(0, 0));
				job.SpoolPage();
			}
			if (!job.CanContinue()) {
				Error("Print job cancelled!");
				return;
			}
			pages++;
		}
		
		// reselect the active list item
		if (current != -1) view[j]->Select(current);
	}
	
	job.CommitJob();
}
