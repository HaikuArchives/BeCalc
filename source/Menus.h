#ifndef MENUS_H
#define MENUS_H

#include <InterfaceKit.h>

#include "Buttons.h"
#include "MMenuBar.h"

MMenuBar *MakeMenu();
void SetEditEnabled(bool enabled, BTextView *v);
BMenuItem *AddPlotItem(const char *item, int32 code);
void RemovePlotItem(BMenuItem * item);
void SavePlotMenu(BList &menu);
void ReadPlotMenu(BList &menu);
void SetMortgageEnabled(bool enabled);

#endif
