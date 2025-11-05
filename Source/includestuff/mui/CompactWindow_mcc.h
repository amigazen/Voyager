#pragma pack(2)
#pragma pack(2)
#ifndef COMPACTWINDOW_MCC_H
#define COMPACTWINDOW_MCC_H

#ifndef LIBRARIES_MUI_H
#pragma pack()
#pragma pack()
#include "libraries/mui.h"
#pragma pack(2)
#pragma pack(2)
#endif

#define MUIC_CompactWindow "CompactWindow.mcc"
#define CompactWindowObject MUI_NewObject(MUIC_CompactWindow

#define MUIA_CompactWindow_Contents			0xfa34ffd0
#define MUIA_CompactWindow_Title				0xfa34ffd1
#define MUIA_CompactWindow_SizeGadget		0xfa34ffd2
#define MUIA_CompactWindow_HVPreference	0xfa34ffd3
#define MUIA_CompactWindow_CloseGadget		0xfa34ffd4
#define MUIA_CompactWindow_NoGadgets		0xfa34ffd5
#define MUIA_CompactWindow_IntuiDrag		0xfa34ffd6

#define MUIM_CompactWindow_IsInDragBar		0xfa34ffd0

#endif

#pragma pack()
#pragma pack()
