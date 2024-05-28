#ifdef __DSHOW_INCLUDED__
#define __WIN_UUIDS_H__
#define __VMF_VFWMSGS_H__
#define __VMF_EVCODE_H__
#define __VMF_DVDMEDIA_H__
#define __VMF_MMREG_H__
#endif

#ifndef __VMF_DSHOW_H__
#define __VMF_DSHOW_H__
#include <VMF_PlatForm.h>
#include <VMFError.h>
#include <VMFVfwmsgs.h>
#include <VMFRegister.h>
#include <VMFStrmif.h>     // Generated IDL header file for streams interfaces
#include <vmfstrmifex.h>   // Generated IDL header file for streams interfaces
#include <VMFUuids.h>      // declaration of type GUIDs and well-known clsids
#include <VMFKsuuids.h>
#include <VMFAMVideo.h>    // ActiveMovie video interfaces and definitions
#include <VMFControl.h>    // generated from control
#include <VMFEvcode.h>     // event code definitions
#include <VMFDVDmedia.h>
#include <VMFMMReg.h>
#endif