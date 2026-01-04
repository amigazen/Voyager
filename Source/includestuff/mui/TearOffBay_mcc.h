#ifdef __GNUC__
#pragma pack(2)
#pragma pack(2)
#endif /* __GNUC__ */
#ifndef TEAROFFBAY_MCC_H
#define TEAROFFBAY_MCC_H

#ifndef LIBRARIES_MUI_H
#ifdef __GNUC__
#pragma pack()
#pragma pack()
#endif /* __GNUC__ */
#include "libraries/mui.h"
#ifdef __GNUC__
#pragma pack(2)
#pragma pack(2)
#endif /* __GNUC__ */
#endif

#define MUIC_TearOffBay "TearOffBay.mcc"
#define TearOffBayObject MUI_NewObject(MUIC_TearOffBay

#define MUIA_TearOffBay_LinkedBay  0xfa34ffd0
#define MUIA_TearOffBay_PrimaryBay 0xfa34ffd1
#define MUIA_TearOffBay_Horiz      0xfa34ffd2

#define MUIA_TearOffBay_Default    0xfa34ffd4

#endif

#ifdef __GNUC__
#pragma pack()
#pragma pack()
#endif /* __GNUC__ */
