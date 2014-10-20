/*
Copyright (C) 1994-1999 The XFree86 Project, Inc.  All Rights Reserved.
Copyright (C) 2000 Silicon Motion, Inc.  All Rights Reserved. 
Copyright (c) 2012 by Silicon Motion, Inc. (SMI)

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the "Software"), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to do
so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FIT-
NESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
XFREE86 PROJECT BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Except as contained in this notice, the names of The XFree86 Project and
Silicon Motion shall not be used in advertising or otherwise to promote the
sale, use or other dealings in this Software without prior written
authorization from The XFree86 Project or Silicon Motion.
*/

#include	"smi_common.h"
#include	"smi_accel.h"
#include	"smi_video.h"
#include	"smi_ver.h"
#include	"smi_dbg.h"
#include 	"smi_driver.h"
#include 	"version.h"
#if !(XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,7,0,0,0) && XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(6,8,0,0,0))
	#include 	"xf86Resources.h"
	#include 	"xf86RAC.h"
#endif
#include 	"xf86DDC.h"
#include 	"xf86int10.h"
#include 	"vbe.h"
#include 	"shadowfb.h"
#include    "xf86Crtc.h"
#include	"drv750/smi_750_driver.h"

const OptionInfoRec *SMI_AvailableOptions (int chipid, int busid);
void SMI_Identify (int flags);
static Bool SMI_Probe (DriverPtr drv, int flags);
static Bool SMI_PreInit(ScrnInfoPtr pScrn,int flags);
static Bool SMI_ScreenInit(ScreenPtr pScreen,int argc,char ** argv);
void SMI_EntityInit(int,pointer);
void SMI_EntityEnter(int,pointer);
void SMI_EntityLeave(int,pointer);
Bool SMI_SwitchMode(int,DisplayModePtr,int);
void SMI_AdjustFrame(int,int,int,int);
static void SMI_FreeScreen(ScrnInfoPtr);
static ModeStatus SMI_ValidMode(ScrnInfoPtr,DisplayModePtr,Bool,int);
static Bool SMI_EntityMap(int,SMIHWPtr);
static void SMI_EntityUnmap(int,SMIHWPtr);
static Bool SMI_SaveScreen(ScreenPtr pScreen,int mode);
static Bool SMI_CloseScreen(ScreenPtr);
static Bool SMI_EnterVT(ScrnInfoPtr);
static void SMI_LeaveVT(ScrnInfoPtr);

int	entity_priv_index[MAX_ENTITIES] = {-1, -1, -1, -1, -1, -1, -1, -1,	-1, -1, -1, -1, -1, -1, -1, -1};
#ifdef SMI_DBG
	int smi_indent = 1;
#endif

#if XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(6,9,0,0,0)
	_X_EXPORT DriverRec SILICONMOTION =
#elif XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0)
	_X_EXPORT DriverRec SILICONMOTION =
#else
	DriverRec SILICONMOTION =
#endif
{
	SILICONMOTION_DRIVER_VERSION,
	SILICONMOTION_DRIVER_NAME,
	SMI_Identify,
	SMI_Probe,
	SMI_AvailableOptions,
	NULL,
	0
};

static const SymTabRec SMIChipsets[] = {
	{PCI_CHIP_SMI712, "LynxEM+"},
	{PCI_CHIP_SMI720, "Lynx3DM"},
	{PCI_CHIP_SMI718, "Lynx718"},
	{PCI_CHIP_SMI750, "Lynx750"},
	{PCI_CHIP_SMI501, "Voyager502"},
	{-1, NULL}
};

static PciChipsets SMIPciChipsets[] = {
#if (XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)) || \
	(XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(4,3,0,0,0))
	{PCI_CHIP_SMI712, PCI_CHIP_SMI712, resVgaIo},
	{PCI_CHIP_SMI501, PCI_CHIP_SMI501, resVgaIo},
	{PCI_CHIP_SMI750, PCI_CHIP_SMI750, resVgaIo},
	{PCI_CHIP_SMI718, PCI_CHIP_SMI718, resVgaIo},
	{ PCI_CHIP_SMI720,PCI_CHIP_SMI720, resVgaIo},
#else
	{PCI_CHIP_SMI712, PCI_CHIP_SMI712, RES_SHARED_VGA},
	{PCI_CHIP_SMI501, PCI_CHIP_SMI501, RES_SHARED_VGA},
	{PCI_CHIP_SMI750, PCI_CHIP_SMI750, RES_SHARED_VGA},
	{PCI_CHIP_SMI718, PCI_CHIP_SMI718, RES_SHARED_VGA},
	{PCI_CHIP_SMI720, PCI_CHIP_SMI720, RES_SHARED_VGA},
#endif
	{-1, -1, RES_UNDEFINED}
};


static const OptionInfoRec smiOptions[] = 
{
	/*-----------------------------------------------------------------------------
	 *  basic options ,used to compose specific options and common options
	 *-----------------------------------------------------------------------------*/
	{OPTION_PCI_BURST, 	"pci_burst", OPTV_BOOLEAN, {0}, TRUE},
	{OPTION_PCI_RETRY, 	"pci_retry", OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_NOACCEL, 	"NoAccel", 	 OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_VIDEOKEY, 	"VideoKey",  OPTV_INTEGER, {0}, FALSE},
	{OPTION_INTERLACED, "Interlaced",OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_PANELHEIGHT,"LCDHeight", OPTV_INTEGER, {0}, FALSE},
	{OPTION_PANELWIDTH, "LCDWidth",  OPTV_INTEGER, {0}, FALSE},
	{OPTION_CSCVIDEO, 	"CSCVideo",	 OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_EDID,		"EDID_EN",	 OPTV_BOOLEAN, {0}, FALSE},
	{OPTION_DUALVIEW,	"DualView",	 OPTV_BOOLEAN, {0}, TRUE},
 	{OPTION_XLCD,		"XLCD",		 OPTV_INTEGER, {0}, FALSE},
    {OPTION_YLCD,		"YLCD", 	 OPTV_INTEGER, {0},	FALSE},
	/* 502 options*/
	/* 718,750 options*/
	/* 712,722 options*/
	{-1, NULL, OPTV_NONE, {0}, FALSE}
};

/*static */ const char *vgahwSymbols[] = {
	"vgaHWCopyReg",
	"vgaHWGetHWRec",
	"vgaHWGetIOBase",
	"vgaHWGetIndex",
	"vgaHWInit",
	"vgaHWLock",
	"vgaHWUnLock",
	"vgaHWMapMem",
	"vgaHWProtect",
	"vgaHWRestore",
	"vgaHWSave",
	"vgaHWSaveScreen",
	"vgaHWSetMmioFuncs",
	"vgaHWSetStdFuncs",
	"vgaHWUnmapMem",

#if (XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(4,3,0,0,0))|| \
	(XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(1,3,0,0,0))
	"vgaHWddc1SetSpeedWeak",
#endif
	NULL
};

/*static*/ const char *xaaSymbols[] = {
	"XAAGetPatternROP",
	"XAAGetCopyROP",
	"XAAGetFallbackOps",

	"XAAScreenIndex",

	"XAAPatternROP",
	"XAACopyROP",
	"XAAFallbackOps",

	"XAACreateInfoRec",
	"XAADestroyInfoRec",
	"XAAInit",
	NULL
};

/*static*/ const char *ddcSymbols[] = {
	"xf86PrintEDID",
	"xf86DoEDID_DDC1",
	"xf86InterpretEDID",
	"xf86DoEDID_DDC2",
	"xf86SetDDCproperties",
	NULL
};

/*static*/ const char *i2cSymbols[] = {
	"xf86CreateI2CBusRec",
	"xf86CreateI2CDevRec",
	"xf86DestroyI2CBusRec",
	"xf86DestroyI2CDevRec",
	"xf86I2CBusInit",
	"xf86I2CDevInit",
	"xf86I2CReadBytes",
	"xf86I2CWriteByte",
	NULL
};

/*static*/ const char *int10Symbols[] = {
	"xf86ExecX86int10",
	"xf86FreeInt10",
	"xf86InitInt10",
	NULL
};

/*static*/ const char *vbeSymbols[] = {
	"VBEInit",
	"vbeDoEDID",
	"vbeFree",
	NULL
};

/*static*/ const char *fbSymbols[] = {
	"fbPictureInit",
	"fbScreenInit",
	NULL
};

/*static*/ const char *ramdacSymbols[] = {
	"xf86CreateCursorInfoRec",
	"xf86DestroyCursorInfoRec",
	"xf86InitCursor",
	NULL
};

#ifdef XFree86LOADER

static MODULESETUPPROTO (siliconmotionSetup);
static XF86ModuleVersionInfo SMIVersRec = {
	"siliconmotion",
	MODULEVENDORSTRING,
	MODINFOSTRING1,
	MODINFOSTRING2,
	XORG_VERSION_CURRENT,
	SMI_VER_MAJOR,
	SMI_VER_MINOR,
	SMI_VER_PATCH,
	ABI_CLASS_VIDEODRV,
	ABI_VIDEODRV_VERSION,
	MOD_CLASS_VIDEODRV,
	{0, 0, 0, 0}
};
	
_X_EXPORT XF86ModuleData siliconmotionModuleData =
{
	&SMIVersRec,
	siliconmotionSetup,
	NULL
};

pointer siliconmotionSetup (pointer module, pointer opts, int *errmaj, 
		int *errmin)
{
	static Bool setupDone = FALSE;
	if (!setupDone)
	{
		setupDone = TRUE;
		xf86AddDriver (&SILICONMOTION, module, 0);
		/*
		 * Modules that this driver always requires can be loaded here
		 * by calling LoadSubModule().
		 */

		/*
		 * Tell the loader about symbols from other modules that this module
		 * might refer to.
		 */
#if (XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)) || \
		(XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(4,3,0,0,0))
		LoaderRefSymLists (vgahwSymbols, fbSymbols, xaaSymbols, ramdacSymbols,
				ddcSymbols, i2cSymbols, int10Symbols, vbeSymbols, NULL);
#endif
		/*
		 * The return value must be non-NULL on success even though there
		 * is no TearDownProc.
		 */
		return (pointer) 1;
	}
	else
	{
		if (errmaj)
		{
			*errmaj = LDR_ONCEONLY;
		}
		return (NULL);
	}
}

#endif

typedef struct {
	int entityIndex;
	pointer private;
}tmpRec;


const OptionInfoRec * SMI_AvailableOptions(int chipid,int busid){
	return smiOptions;
}


void SMI_Identify (int flags)
{
	xf86PrintChipsets (SILICONMOTION_NAME, 
		"driver (version " SILICONMOTION_VERSION_NAME ") for Silicon Motion Graphic chipsets", 
		SMIChipsets);
}


static void claim(void){
	xf86Msg(X_INFO,"+-------------SMI Driver Information------------+\n");
	xf86Msg(X_INFO,"Release type : " RELEASE_TYPE "\n");
	xf86Msg(X_INFO,"Driver version: v" _version_ "\n");
	xf86Msg(X_INFO,"Support products:\n"
			SUPPORT_CHIP);	
	xf86Msg(X_INFO,"Support OS:\n"
			SUPPORT_OS);
	xf86Msg(X_INFO,"Support ARCH: " SUPPORT_ARCH "\n");
	xf86Msg(X_INFO,"+-----------------------------------------------+\n");
}


/* Free SMI_REC */
static void SMI_FreeRec (ScrnInfoPtr pScrn)//as equal as close screen.
{
	if (pScrn->driverPrivate != NULL)
	{
		xfree (pScrn->driverPrivate);
		pScrn->driverPrivate = NULL;
	}
}
/*********************************************************************
SMI_Probe
Description:
	Find a hardware device to drive
	1. Set up entity and screen and the private data for a special video card
**********************************************************************/
static Bool SMI_Probe (DriverPtr drv, int flags)
{
	int i;
	int allocPriv;
	tmpRec * pTmp;
	/*-----------------------------------------------------------------------------
	 *  if X use libpciaccess,struct pci_device is provided from libpciaccess
	 *  if X use libpci,there is also a struct named pciVideoPtr 
	 *-----------------------------------------------------------------------------*/
#ifdef XSERVER_LIBPCIACCESS
	struct pci_device * pPci;
#else
	pciVideoPtr pPci;
#endif
	GDevPtr *devSections;
	int *usedChips;
	int numDevSections;
	int numUsed;
	Bool foundScreen = FALSE;
	ScrnInfoPtr pScrn;
	int chipType = 0;

	ENTER();
	claim();
	
	xf86Msg(X_INFO,"SMI Alcyone Driver Probe HW!\n");
	/*This function takes the name of the driver and returns via driversectlist a list of device sections that match the driver name in 'xorg.conf'. 
	The function return value is the number of matches found. */
	numDevSections = xf86MatchDevice (SILICONMOTION_DRIVER_NAME, &devSections);
	if (numDevSections <= 0)
		LEAVE (FALSE);
#ifndef XSERVER_LIBPCIACCESS
	/* who can tell me what the hell below line mean?? */
	if (xf86GetPciVideoInfo () == NULL)
	{
		LEAVE (FALSE);
	}
#endif

	/*This function used to find the PCI devices that match the given vendor ID.
	The function return value is the number of 'screen' matches found.
	'usedChips' is a  entities index array for every 'screen' */
	numUsed = xf86MatchPciInstances(SILICONMOTION_NAME, PCI_SMI_VENDOR_ID,
			SMIChipsets, SMIPciChipsets, devSections,
			numDevSections, drv, &usedChips);
	DEBUG("numUsed = %d\n",numUsed);
	
	/* Free it since we don't need that list after this */
	xfree(devSections);
	/*By using xnfcalloc() to do the allocation it is zeroed
	, and if the allocation fails the server exits.*/
	pTmp = XNFcalloc(sizeof(*pTmp)*numUsed);
	if(!pTmp)
		LEAVE (FALSE);

	if(numUsed < 1){
		LEAVE (FALSE);
	}

	if(flags & PROBE_DETECT){
		foundScreen = TRUE;
	}else{
		/*Traversal the whole Entity Index Table (usedChips[]) and assign resource for every 'screen'*/
		for(i = 0;i < numUsed;i++)
		{
			SMIPtr pSmi = NULL;
			pointer priv = NULL;
			tmpRec * pTmp2;
			pTmp[i].entityIndex = usedChips[i];
			/*Assume each entity is different */
			allocPriv = 1;
			/*Make 'pTmp2' point to the start of entity index table*/
			pTmp2 = pTmp;
			/*Traversal the entities before the current to check if the current 'screen' is belong to a previous entity*/
			while(pTmp2 != &pTmp[i]){
				if(pTmp2->entityIndex == usedChips[i]){
					/* seems previously,already got the same entity as current entity */
					allocPriv = 0;
					/* get previously allocated private for this time usage */
					priv = pTmp2->private;
					break;
				}
				pTmp2 ++;
				/*Maybe it is unavailable to assign 3 or above 'screen' to 1 entity*/
			}

			/*This function returns a pointer to the 'pciVideoRec' for the specified entity. 
			If the entity is not a PCI device, NULL is returned.*/
			pPci = xf86GetPciInfoForEntity(usedChips[i]);
			pScrn = NULL;
			
/*==================================================================================
		In the common layer, only here to judge the chip type
		Here to generate the chip special structure including date and function
		supplied to common layer to handle the chip special layer.
==================================================================================*/	  
		#ifdef XSERVER_LIBPCIACCESS
			switch (pPci->device_id)
		#else
			switch (pPci->device)
		#endif
			{
				case SMI_502:
//                      pSmi = sm502_genSMI(priv, usedChips[i]);
					chipType |= SM5XX;
					pSmi->LoadModules = 0;
					break;
				case SMI_750:
				case SMI_718:
				#ifdef XSERVER_LIBPCIACCESS
						if (pPci->revision == 0xFE) 
				#else
						if (pPci->chipRev == 0xFE) 
				#endif
                                                       {
							pSmi = sm750le_genSMI(priv,usedChips[i]);
                    		chipType |= SM7XX;
                    		pSmi->LoadModules = 0;
                            break;
						}
					pSmi = sm750_genSMI(priv,usedChips[i]);
					chipType |= SM7XX;
					pSmi->LoadModules = 0;
					break;
				case SMI_712:
				case SMI_722:
//                    pSmi = sm712_genSMI(priv,usedChips[i]);
					chipType |= SM7X2;
					pSmi->LoadModules = MODULE_INT10|MODULE_VBE;
					break;
				default:
					break;
			}

			if( pSmi == NULL )
			{		
				XERR("Couldn't generate SMI REC!\n");		
				LEAVE(FALSE);
			}

			if(allocPriv){
				pTmp[i].private = pSmi->pHardware;
				priv = pTmp[i].private;
			}
			
			/*This function register all non-relocatable resources
			and allocate A 'ScrnInfoRec' for active entities.*/
			if((pScrn = xf86ConfigPciEntity(pScrn,0,usedChips[i],
						SMIPciChipsets,NULL,
						SMI_EntityInit,
						SMI_EntityEnter,
						SMI_EntityLeave,
						priv)))
			{

				/* SetEntitySharable must be called between two xf86ConfigPciEntity's calling for dual view situation */
				xf86SetEntitySharable(usedChips[i]);
				/* set screen instance id for pScrn */
				xf86SetEntityInstanceForScreen(pScrn, usedChips[i],pSmi->screen);
				
				pScrn->driverVersion = SILICONMOTION_DRIVER_VERSION;
				pScrn->driverName = SILICONMOTION_DRIVER_NAME;
				pScrn->name = SILICONMOTION_NAME;
				pScrn->Probe = SMI_Probe;
				pScrn->PreInit = SMI_PreInit;
				pScrn->ScreenInit = SMI_ScreenInit;
				pScrn->SwitchMode = SMI_SwitchMode;
				pScrn->AdjustFrame = SMI_AdjustFrame;
				pScrn->EnterVT = SMI_EnterVT;
				pScrn->LeaveVT = SMI_LeaveVT;
				pScrn->FreeScreen = SMI_FreeScreen;
			#if (SMI_RANDR==0)
				pScrn->ValidMode = SMI_ValidMode;
			#endif
				pScrn->driverPrivate = pSmi;
				pSmi->screen_info = pScrn;
				foundScreen = TRUE;
			}
			
			/*This blank line is for 'smi_driver.o: file not recognized: File truncated'*/
		}
	}

	if( chipType&SM7XX )
	{		
		if(!xf86LoadSubModule(pScrn,"smiddk750")){
			XERR("couldn't load ddk750!\n");		
			LEAVE(FALSE);}
	}
	if( chipType&SM7X2 )	
	{		
		if(!xf86LoadSubModule(pScrn,"smiddk712")){
			XERR("couldn't load ddk712!\n");		
			LEAVE(FALSE);}
	}
	if( chipType&SM5XX )	
	{		
		if(!xf86LoadSubModule(pScrn,"smiddk502")){
		XERR("couldn't load ddk502!\n");		
		LEAVE(FALSE);}
	}
	xfree(pTmp);
	xfree(usedChips);
	LEAVE (foundScreen);
}
/*********************************************************************
SMI_PreInit
Description:
	The purpose of this function is to find out all the information required to determine if the configuration is usable
	, and to initialise those parts of the ScrnInfoRec that can be set once at the beginning of the first server generation. 
*********************************************************************/
static Bool SMI_PreInit(ScrnInfoPtr pScrn,int flags)
{
	EntityInfoPtr pEntInfo;
	SMIPtr pSmi;
	MessageType from;
	int i;
	int entityIndex = pScrn->entityList[0];
	ENTER();

	pSmi = SMIPTR(pScrn);
	xf86Msg(X_INFO,"SMI Alcyone Driver Running!\n");
	if(flags & PROBE_DETECT){
		LEAVE(TRUE);
	}
	
	/*The number of entities registered for the screen should be checked against the expected number (most drivers expect only one)
	The item of 'Screen' is a virtual concept, sometimes, several entities support a big screen together.
	But our driver only support only one entity hold the entire screen.
	*/
	if(pScrn->numEntities > 1){
		LEAVE(FALSE);
	}

	/*what's the difference between 'pScrn->entityList[0]' and 'pEntInfo->index'??*/
	pEntInfo = xf86GetEntityInfo(entityIndex);	
	pSmi->pHardware->pEnt_info = pEntInfo;
	
	/* Get the entity, and make sure it is PCI. */
	if ((pEntInfo->location.type != BUS_PCI) 
#if (XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)) || \
    (XORG_VERSION_CURRENT > XORG_VERSION_NUMERIC(6,8,0,0,0))
                    || (pEntInfo->resources)
#endif
                     )
        {
		SMI_FreeRec (pScrn);
		LEAVE (FALSE);
	}	
	/*-----------------------------------------------------------------------------
	 *  load Sub modules
	 *	Load a module that a driver depends on. 
	 *	This function loads the module name (such as vgahw)as a sub module of the driver. 
	 *-----------------------------------------------------------------------------*/

	/* Load the submodule vgahw for get the PIOBASE */
	if(xf86LoadSubModule(pScrn,"vgahw") == FALSE)
	{
		xf86Msg(X_ERROR,"Load SubModule:vgahw failed\n");	
		LEAVE();
	}
	/*This function allocates a vgaHWRec structure, and hooks it into the ScrnInfoRec's privates. */
	if(!vgaHWGetHWRec(pScrn))
	{
		xf86Msg(X_ERROR,"Create vgaHWRec failed\n");	
		LEAVE();
	}

    vgaHWPtr hwp = VGAHWPTR(pScrn);
    /* Initialize the PIOBase for hw init*/
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,12,0,0,0)    
    pSmi->pHardware->PIOBase = hwp->PIOOffset;
#else
    pSmi->pHardware->PIOBase = 0;//X donot use the PIOOffset anymore.
#endif

#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0) || \
    (XORG_VERSION_CURRENT >= XORG_VERSION_NUMERIC(4,3,0,0,0))
        xf86LoaderReqSymLists (vgahwSymbols, NULL);
#endif

#if 0     // for 750LE
        if (!vgaHWSetRegCounts(pScrn, 0x27, 0x5, 0x9, 0x15)) {
            LEAVE(FALSE);
        }	
#endif   

	if(pSmi->LoadModules & MODULE_INT10){
		if(xf86LoadSubModule(pScrn,"int10") == FALSE)
			LEAVE(FALSE);
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
	    xf86LoaderReqSymLists (int10Symbols, NULL);
#endif
		/*The function will soft-boot any non-primary device and return a pointer to a xf86Int10InfoRec on success.
		If anything fails or if int10 execution is disabled by an foption in the device section NULL will be returned.
		The driver should store this pointer for later calls to other int10 module functions.*/
		pSmi->pInt10 = xf86InitInt10(pEntInfo->index);
		if(!pSmi->pInt10)
			LEAVE(FALSE);
	}

	if((pSmi->LoadModules & MODULE_VBE)&&pSmi->pInt10){
		if(xf86LoadSubModule(pScrn,"vbe") == FALSE)
			LEAVE(FALSE);
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
	    xf86LoaderReqSymLists (vbeSymbols, NULL);
#endif
		pSmi->pVbe = VBEInit(pSmi->pInt10,pEntInfo->index);
   		if(!pSmi->pVbe)
			LEAVE(FALSE);
	}
	
	if(xf86LoadSubModule(pScrn,"fb") == NULL)
		LEAVE(FALSE);
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
	xf86LoaderReqSymLists (fbSymbols, NULL);
#endif

    pScrn->monitor = pScrn->confScreen->monitor;

	/*-----------------------------------------------------------------------------
	 *  set bpp stuffs
	 *	This function sets the depth, pixmapBPP and bitsPerPixel fields of the ScrnInfoRec. 
	 *	It also determines the defaults for display-wide attributes and pixmap formats the screen will support
	 *	, and finds the Display subsection that matches the depth/bpp.
	 *-----------------------------------------------------------------------------*//*
	 * The first thing we should figure out is the depth, bpp, etc.  Our
	 * default depth is 8, so pass it to the helper function.  We support
	 * only 24bpp layouts, so indicate that. */
	if(!xf86SetDepthBpp(pScrn,8,8,8,
						Support32bppFb |
						SupportConvert24to32 |
						PreferConvert24to32)){
		LEAVE (FALSE);		
	}
	
    /* Check that the returned depth is one we support */
    if(pScrn->depth != 16 && pScrn->depth != 24)
    {
    	XERR("Given depth (%d) is not supported by this driver\n",pScrn->depth);
        LEAVE(FALSE);
    }

    if(pScrn->bitsPerPixel != 16 && pScrn->bitsPerPixel != 32)
    {
        XERR("Given bpp (%d) is not supported by this driver\n", pScrn->bitsPerPixel);
        LEAVE(FALSE);
    }

	/* byfar,only 8/16/24 bpp supported */
	switch(pScrn->depth){
		case 8:
		case 16:
		case 24:
			break;
		case 32:
            pScrn->depth = 24;
			break;
			
		default:
			XERR("Given bpp %d not supported \n",pScrn->depth);
			LEAVE (FALSE);
	}
	/*This function can be used to print out the depth and bpp settings. 
	It should be called after the final call to xf86SetDepthBpp(). */
	xf86PrintDepthBpp(pScrn);

	/*-----------------------------------------------------------------------------
	 *  check some parameter valid or not
	 * This function sets the weight, mask, offset and rgbBits fields of the ScrnInfoRec. 
	 * It would normally be called fairly early in the XXXPreInit() function for depths > 8bpp.
	 * What is 'Weight' in X?
	 *-----------------------------------------------------------------------------*/
	if ((pScrn->depth > 8))
	{
		/* 'BitsPerComponent' is the driver's preferred default weight if no other is given. 
		If zero, use the overall server default. 
		'BitMask',Same, but for mask.
		The defaults are OK for us */
		rgb BitsPerComponent = { 0, 0, 0 };
		rgb BitMask = { 0, 0, 0 };
		if (!xf86SetWeight (pScrn, BitsPerComponent, BitMask))
		{
			LEAVE(FALSE);
		}
	}
	/*This function sets the defaultVisual field of the ScrnInfoRec. 
	 It would normally be called fairly early from the XXXPreInit() function. 
	 It requires that the depth and display fields of the ScrnInfoRec be initialised prior to calling it. 
	 The parameters passed are: 
	 * 	ScrnInfoPtr
	 *	int visual:driver's preferred default visual if no other is given.
	 *** x server maybe used the visual type from command line or config file
	 *** Everything take the server source code as the standard.
	 */
	if(!xf86SetDefaultVisual(pScrn,-1)){
		LEAVE(FALSE);
	}
	
	/* We don't currently support DirectColor at > 8bpp */
	if ((pScrn->depth > 8) && (pScrn->defaultVisual != TrueColor))
	{
		XERR("bpp > 8 and visual is not Truecolor,cannot support!\n");
		LEAVE(FALSE);
	}
	
	/* We use a programmable clock */
	pScrn->progClock = TRUE;

	/* Collect all of the relevant option flags from xorg.conf(fill in pScrn->options) */ 
	/* Collect the options from each of the config file sections used by the screen (pScrn) and return the merged list as pScrn->options. 
	 This function requires that pScrn->confScreen, pScrn->display, pScrn->monitor, pScrn->numEntities, and pScrn->entityList are initialised. 
	 The second parametre 'extraOpts' may optionally be set to an additional list of options to be combined with the others. 
	 The order of precedence for options is extraOpts, display, confScreen, monitor, device. */
	xf86CollectOptions(pScrn,NULL);

	if(!(pSmi->Options = xalloc(sizeof(smiOptions))))
		LEAVE(FALSE);
	memcpy(pSmi->Options,smiOptions,sizeof(smiOptions));

	/* Process the options */
	/* Processes a list of options according to the information in the array of OptionInfoRecs (pSmi->Options). 
	 The resulting information is stored in the value fields of the appropriate (pSmi->Options) entries. 
	 The found fields are set to TRUE when an option with a value of the correct type if found, and FALSE otherwise. 
	 The type field is used to determine the expected value type for each option. 
	 Each option in the list of options for which there is a name match (but not necessarily a value type match) is marked as used. 
	 Warning messages are printed when option values don't match the types specified in the optinfo data.*/
	xf86ProcessOptions(pScrn->scrnIndex,pScrn->options,pSmi->Options);
	/*-----------------------------------------------------------------------------
	 *  common options handle
	 * This function is used to check boolean options (OPTV_BOOLEAN). 
	 * If the option is set, its value is returned. 
	 * If the options is not set, the default value specified by 'def' is returned. 
	 *-----------------------------------------------------------------------------*/
	if (xf86ReturnOptValBool (pSmi->Options, OPTION_PCI_BURST, TRUE))
		pSmi->pci_burst = TRUE;
	else
		pSmi->pci_burst = FALSE;
	XINF("pci burst %s\n",(pSmi->pci_burst==TRUE)?"on":"off");

	i = -1;
	pSmi->TFTColor = i;
	/* if option TFT_TYPE not defined in xorg.conf, below xf86Getxxxx will return 0 */
	if(xf86GetOptValInteger(pSmi->Options,OPTION_TFT_TYPE,&i))
		xf86DrvMsg(pScrn->scrnIndex, X_CONFIG, "xorg.conf Set TFT to %d bit.\n",i);        

	switch (i)
	{
		case 9:
			pSmi->TFTColor = SMI_TFT_9BIT;
			break;                                
		case 12:
			pSmi->TFTColor = SMI_TFT_12BIT;
			break;
		case 18:
			pSmi->TFTColor = SMI_TFT_18BIT;                
			break;
		case 24:
			pSmi->TFTColor = SMI_TFT_24BIT;
			break;
		default:
			pSmi->TFTColor = SMI_TFT_24BIT;
			break;
	} 

	/*For CSC video*/
	if (xf86ReturnOptValBool (pSmi->Options, OPTION_CSCVIDEO, FALSE))
	{
		pSmi->IsCSCVideo = TRUE;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option:  csc video "
				"read enabled\n");
	}
	else
	{
		pSmi->IsCSCVideo = FALSE;
	}	

        pSmi->NoPCIRetry = TRUE;
        if (xf86ReturnOptValBool (pSmi->Options, OPTION_PCI_RETRY, FALSE))
        {
            if (xf86ReturnOptValBool (pSmi->Options, OPTION_PCI_BURST, FALSE))
            {
                pSmi->NoPCIRetry = FALSE;
                xf86DrvMsg (pScrn->scrnIndex, 
                        X_CONFIG, "Option: pci_retry enabled\n");
            }
            else
            {
                xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "\"pci_retry\" option "
                        "requires \"pci_burst\".\n");
            }
        }
	
	if (xf86ReturnOptValBool (pSmi->Options, OPTION_NOACCEL, FALSE))
//		pSmi->NoAccel = TRUE;
//	else
		pSmi->NoAccel = FALSE;
	//pSmi->NoAccel = TRUE;
	XINF("no acceleration %s\n",(pSmi->NoAccel==TRUE)?"enable":"disable");


	if (xf86GetOptValInteger (pSmi->Options, OPTION_VIDEOKEY, &pSmi->videoKey))
	{
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "Option: Video key set to "
				"0x%08X\n", pSmi->videoKey);
	}
	else
	{
		/* should not use pScrn->offset now,because screenInit will 
		 * modify their value,the videoKey will be set in ScreenInit after
		 * the pScrn->offset modified 
		 * */
	}

	if (xf86ReturnOptValBool (pSmi->Options, OPTION_INTERLACED, FALSE))
	{
		pSmi->interlaced = TRUE;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG,
				"Option: Interlaced enabled.\n");
	}
	else
	{
		pSmi->interlaced = FALSE;
	}	
#if SMI_RANDR
	/*Enable xLCD and yLCD*/
	pSmi->lcdHeight = pSmi->lcdWidth = 0;
	from = X_DEFAULT;
	if( xf86GetOptValInteger(pSmi->Options,OPTION_XLCD,&pSmi->lcdWidth)
	&& xf86GetOptValInteger(pSmi->Options,OPTION_YLCD,&pSmi->lcdHeight))
		from = X_CONFIG;
	xf86DrvMsg(pScrn->scrnIndex, from, "Set panel size to %dx%d. 0x0 means no limitation\n",
	pSmi->lcdWidth,pSmi->lcdHeight);
	from = X_DEFAULT;
	if(xf86GetOptValBool(pSmi->Options,OPTION_DUALVIEW,&pSmi->DualView))
		from = X_CONFIG;
	xf86DrvMsg(pScrn->scrnIndex, from, "Dual View %sabled\n",pSmi->DualView ? "en" : "dis");
#endif
	/*-----------------------------------------------------------------------------
	 *  chip specific options;
	 *-----------------------------------------------------------------------------*/
	pSmi->psHandleOptions(pScrn);

//#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
#ifndef XSERVER_LIBPCIACCESS
	/*This function tries to register the entity in list. 
	If list is NULL it tries to determine the resources automatically. 
	This only works for entities that provide a generic way to read out the resource*/
    xf86RegisterResources (pScrn->entityList[0], NULL, ResExclusive);
#endif

	/* * Set the Chipset and ChipRev, allowing config file entries to * override.  */
	if (pEntInfo->device->chipset && *pEntInfo->device->chipset)
	{
		pScrn->chipset = pEntInfo->device->chipset;
		pSmi->pHardware->Chipset = xf86StringToToken (SMIChipsets, pScrn->chipset);
		from = X_CONFIG;
	}
	else if (pEntInfo->device->chipID >= 0)
	{
#ifndef XSERVER_LIBPCIACCESS    
		pSmi->pHardware->Chipset = pEntInfo->device->chipID;
#else
		pSmi->pHardware->Chipset = (pSmi->pHardware->pPci->device_id);	//caesar modified
#endif
		pScrn->chipset =
			(char *) xf86TokenToString (SMIChipsets, pSmi->pHardware->Chipset);
		from = X_CONFIG;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "ChipID override: 0x%04X\n",
				pSmi->pHardware->Chipset);
	}
	else
	{
		from = X_PROBED;
#ifndef XSERVER_LIBPCIACCESS  	  
		pSmi->pHardware->Chipset = pSmi->pHardware->pPci->chipType;
#else
		pSmi->pHardware->Chipset = (pSmi->pHardware->pPci->device_id); //caesar modified
#endif
		pScrn->chipset =
			(char *) xf86TokenToString (SMIChipsets, pSmi->pHardware->Chipset);
	}

	if (pEntInfo->device->chipRev >= 0)
	{
		pSmi->pHardware->ChipRev = pEntInfo->device->chipRev;
		xf86DrvMsg (pScrn->scrnIndex, X_CONFIG, "ChipRev override: %d\n",
				pSmi->pHardware->ChipRev);
	}
	else
	{
#ifndef XSERVER_LIBPCIACCESS    
		pSmi->pHardware->ChipRev = pSmi->pHardware->pPci->chipRev;
#else
		pSmi->pHardware->ChipRev = (pSmi->pHardware->pPci->revision);	//caesar modified
#endif
	}

	xfree(pEntInfo);

	/*
	 * This shouldn't happen because such problems should be caught in
	 * SMI_Probe(), but check it just in case.
	 */
	if (pScrn->chipset == NULL)
	{
		xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "ChipID 0x%04X is not "
				"recognised\n", pSmi->pHardware->Chipset);
		LEAVE (FALSE);
	}
	if (pSmi->pHardware->Chipset < 0)
	{
		xf86DrvMsg (pScrn->scrnIndex, X_ERROR, "Chipset \"%s\" is not "
				"recognised\n", pScrn->chipset);
		LEAVE (FALSE);
	}

	xf86DrvMsg (pScrn->scrnIndex, from, "Chipset: \"%s\"\n", pScrn->chipset);

	/*assign the video ram in KB*/
	pScrn->videoRam = ((pSmi->pHardware->physize_mem) / (pSmi->pHardware->dual))>>10;

#if 0// by ilena: maybe we need this???
#ifndef XSERVER_LIBPCIACCESS
    //ilena: PciInfo->>>hdware->pPci
    pSmi->PciTag = pciTag (pSmi->PciInfo->bus, 
            pSmi->PciInfo->device,pSmi->PciInfo->func);
    pci_tag = pSmi->PciTag;
    config = pciReadByte(pSmi->PciTag, PCI_CMD_STAT_REG);
    pciWriteByte(pSmi->PciTag, 
            PCI_CMD_STAT_REG, config | PCI_CMD_MEM_ENABLE);
#endif
#endif
/// set i2c init
	pSmi->IsSecondary = FALSE;
	pSmi->IsPrimary = TRUE;
	pSmi->IsLCD = TRUE;
	pSmi->IsCRT = FALSE;

	/* Set dual head */
    if (xf86IsEntityShared (pScrn->entityList[0])) 
	{
        if (!xf86IsPrimInitDone(pScrn->entityList[0])) 
		{
            xf86SetPrimInitDone(pScrn->entityList[0]);
            pSmi->IsPrimary = TRUE;
            pSmi->IsSecondary = FALSE;
            pSmi->IsLCD = TRUE;
            pSmi->IsCRT = FALSE;
        } 
		else if (pSmi->pHardware->dual > 1) 
        {
            pSmi->IsSecondary = TRUE;
            pSmi->IsCRT = TRUE;
            pSmi->IsPrimary = FALSE;
            pSmi->IsLCD = FALSE;
        }
		else 
        {
            LEAVE(FALSE);
        }
    }

	/**/
	pScrn->virtualX = pScrn->display->virtualX;

	/*
	 * If the driver can do gamma correction, it should call xf86SetGamma()
	 * here. (from MGA, no ViRGE gamma support yet, but needed for
	 * xf86HandleColormaps support.)
	 */
	{
		Gamma zeros = { 0.0, 0.0, 0.0 };

		if (!xf86SetGamma (pScrn, zeros))
		{
            //SMI_EnableVideo (pScrn);
            //SMI750LE_UnmapMem (pScrn); 
			LEAVE(FALSE);
		}
	}
	
	/* Lynx built-in ramdac speeds */ //135000, 135000, 135000, 135000 for 712?
	pScrn->numClocks = 4;
	if ((pScrn->clock[3] <= 0) && (pScrn->clock[2] > 0))
		pScrn->clock[3] = pScrn->clock[2];
	if (pScrn->clock[0] <= 0)
		pScrn->clock[0] = 200000;
	if (pScrn->clock[1] <= 0)
		pScrn->clock[1] = 200000;
	if (pScrn->clock[2] <= 0)
		pScrn->clock[2] = 200000;
	if (pScrn->clock[3] <= 0)
		pScrn->clock[3] = 200000;

	/*-----------------------------------------------------------------------------
	 * Setup the ClockRanges, which describe what clock ranges are available,
	 * and what sort of modes they can be used for.
	 *----------------------------------------------------------------------------*/
	pSmi->clockRanges = xnfcalloc (sizeof (ClockRange), 1);
	pSmi->clockRanges->next = NULL;
	pSmi->clockRanges->minClock = 12000;//25000 for LE
	pSmi->clockRanges->maxClock = 270000;// 110000 for LE
	pSmi->clockRanges->clockIndex = -1;
	pSmi->clockRanges->interlaceAllowed = FALSE;
	pSmi->clockRanges->doubleScanAllowed = FALSE;
	/**/
	pSmi->linePitches = NULL;
	pSmi->minPitch = 128;
	pSmi->maxPitch = 4096;
	pSmi->pitchInc = 128;
	pSmi->minHeight = 128;
	pSmi->maxHeight = 4096;
#if SMI_RANDR
	if (xf86LoadSubModule(pScrn, "i2c")){
		pSmi->psI2CInit(pScrn);
	}
	if(xf86LoadSubModule(pScrn, "ddc")){
		//xf86LoaderReqSymLists(ddcSymbols, NULL);            
	}
	if(!pSmi->psCrtcPreInit(pScrn))
		LEAVE(FALSE);
	if(!pSmi->psOutputPreInit(pScrn))
		LEAVE(FALSE);
//	xf86InitialConfiguration (pScrn, !pSmi->NoAccel);
    xf86InitialConfiguration (pScrn, 0);//ilena: set 1 while using EXA only.
#endif	

#if (SMI_RANDR==0)
	/*-----------------------------------------------------------------------------
	 *  chip private PreInit 
	 *-----------------------------------------------------------------------------*/
	/* * This function takes a set of mode names, modes and limiting conditions,
	and selects a set of modes and parameters based on those conditions.*/
	i = xf86ValidateModes(pScrn,
						pScrn->monitor->Modes,/* Available monitor modes  */
						pScrn->display->modes,/* req mode names for screen */
						pSmi->clockRanges,
						pSmi->linePitches,
						pSmi->minPitch,
						pSmi->maxPitch,
						pSmi->pitchInc,
						pSmi->minHeight,
						pSmi->maxHeight,
						pScrn->display->virtualX,
						pScrn->display->virtualY,
						(pSmi->pHardware->physize_mem) / (pSmi->pHardware->dual),
						LOOKUP_BEST_REFRESH);
	/*The function's return value is the number of matching modes found, or -1
	if an unrecoverable error was encountered.*/
	if(i == -1)
	{
        SMI_FreeRec(pScrn);
		LEAVE(FALSE);
	}
#endif

	/*-----------------------------------------------------------------------------
	 *This function deletes modes in the modes field of the ScrnInfoRec that have been marked as invalid. 
	 *This is normally run after having run xf86ValidateModes() for the last time. 
	 *For each mode that is deleted, a warning message is printed out indicating the reason for it being deleted.
	 *-----------------------------------------------------------------------------*/
	xf86PruneDriverModes(pScrn);
	if( i== 0 || pScrn->modes == NULL){
		XERR("No valid modes found!\n");
		SMI_FreeRec(pScrn);
		LEAVE(FALSE);
	}
#if (SMI_RANDR==1)

	/*Goes through the screen's mode list, and initialises the Crtc parameters for each mode.  
	The initialisation includes adjustments for interlaced and double scan modes.*/
	xf86SetCrtcForModes(pScrn,0);
#endif	
	/* Set the current mode to the first in the list */
	pScrn->currentMode = pScrn->modes;

	/* Print the list of modes being used */
	xf86PrintModes(pScrn);

	/*This function sets the xDpi and yDpi fields of the ScrnInfoRec (display resolution)*/
	xf86SetDpi(pScrn,0,0);
#if 0 //ilena: load them while we need .
    /* Load ramdac if needed */
    if (pSmi->hwcursor)
    {
        if (!xf86LoadSubModule (pScrn, "ramdac"))
        {
            SMI_FreeRec (pScrn);
            LEAVE (FALSE);
        }
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
        xf86LoaderReqSymLists (ramdacSymbols, NULL);
#endif
    }
    if (pSmi->shadowFB)
    {
        if (!xf86LoadSubModule (pScrn, "shadowfb"))
        {
            SMI_FreeRec (pScrn);
            LEAVE (FALSE);
        }
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,7,0,0,0)
        xf86LoaderReqSymLists (shadowSymbols, NULL);
#endif
    }
#endif
        if(pSmi->pHardware->pcInitHardware!= NULL)
			pSmi->pHardware->pcInitHardware(pSmi->pHardware);

	LEAVE(TRUE);
fail:
	
	LEAVE(FALSE);
}

static Bool SMI_ScreenInit(ScreenPtr pScreen,int argc,char ** argv)
{
      int scrnIndex;
    scrnIndex = pScreen->myNum;
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	xf86Msg(X_INFO, "Initialize the %d screen!\n",scrnIndex);
	SMIPtr pSmi;
	BoxRec AvailFBArea;

	ENTER();
	pSmi = SMIPTR(pScrn);
        xf86CheckMTRR(scrnIndex);	
    pScrn->displayWidth = pScrn->virtualX;
	
	/* entityInit already map the fb and registers */
	pSmi->psGetMapResult(pScrn);

#if (SMI_RANDR==0)
	/* Write registers to set mode */
	pSmi->psSetMode(pScrn,pScrn->currentMode);
	pSmi->psSetDisplay(pScrn,pScrn->currentMode);
#endif

#if 1 //not for 750LE
	/* This function fills in the vgaHWRec's ModeReg field with the values appropriate 
	for programming the given video mode. 
	It requires that the ScrnInfoRec's depth field is initialised, 
	which determines how the registers are programmed. 
	Before this function called, save the vga status first for LeaveVT*/
	if (!vgaHWInit (pScrn, pScrn->currentMode))
	{
		LEAVE (FALSE);
	}  
#endif
	
	/* clear on-screen */
	memset(pSmi->pFB, 0, pScrn->virtualX * pScrn->virtualY * (pScrn->bitsPerPixel>>3));
#if (SMI_RANDR==0)
	xf86DisableRandR();
	XINF("RandR Disabled.");
#endif
	/*
	 * The next step is to setup the screen's visuals, and initialise the framebuffer code.
	 * In cases where the framebuffer's default choises for things like visual layouts and bits per RGB are OK
	 * , this may be as simple as calling the framebuffer's ScreenInit() function.  
	 * If not, the visuals will need to be setup before calling a fbScreenInit() function and fixed up after.
	 *
	 * For most PC hardware at depths >= 8, the defaults that cfb uses are not appropriate.
	 * In this driver, we fixup the visuals after.
	 */

	/* Reset the visual list. */
	miClearVisualTypes();
	
	/* The default set of visuals isn't acceptable.
	 * To deal with this, call miSetVisualTypes with the appropriate visual mask.
	 */
#if (SMI_RANDR==0)
	if(!miSetVisualTypes(pScrn->depth,TrueColorMask,pScrn->rgbBits, pScrn->defaultVisual))
	{
		XERR("Set Visual Types Failed!\n");
		LEAVE(FALSE);
	}
#else
    if (!miSetVisualTypes(pScrn->depth, miGetDefaultVisualMask(pScrn->depth),
                pScrn->rgbBits, pScrn->defaultVisual))
        LEAVE(FALSE);
#endif
	if(!miSetPixmapDepths())
	{
		XERR("Set Pixmap Depths Failed!\n");
		LEAVE(FALSE);
	}
#if 1
	/* fbScreenInit() is used to tell the fb layer where the video card framebuffer is*/
	if(!fbScreenInit(pScreen,pSmi->pFB,pScrn->virtualX,pScrn->virtualY,
				pScrn->xDpi,pScrn->yDpi,pScrn->displayWidth,
				pScrn->bitsPerPixel))
#else // by ilena: I think 502 should be seperated
	if (!SMI502_InternalScreenInit (scrnIndex, pScreen))
#endif
	{
		XERR("FB Screen Init Failed!\n");
		LEAVE (FALSE);
	}

#if X_BYTE_ORDER == BIG_ENDIAN
		if(pScrn->bitsPerPixel == 32){
			pScrn->offset.red = 8;
			pScrn->offset.green = 16;
			pScrn->offset.blue = 24;
	
			pScrn->mask.red = 0x0000ff00;
			pScrn->mask.green = 0x00ff0000;
			pScrn->mask.blue = 0xff000000;
		}
#endif
		pSmi->videoKey = (1 << pScrn->offset.red)|(1 << pScrn->offset.green)|
			(((pScrn->mask.blue >> pScrn->offset.blue) - 1)<< pScrn->offset.blue);

	/* Set initial black & white colourmap indices. */
	xf86SetBlackWhitePixels(pScreen);

	if(pScrn->bitsPerPixel > 8){
		VisualPtr visual;
		/* Fixup RGB ordering */
		visual = pScreen->visuals + pScreen->numVisuals;
		while (--visual >= pScreen->visuals)
		{
			if ((visual->class | DynamicClass) == DirectColor)
			{
				visual->offsetRed = pScrn->offset.red;
				visual->offsetGreen = pScrn->offset.green;
				visual->offsetBlue = pScrn->offset.blue;
				visual->redMask = pScrn->mask.red;
				visual->greenMask = pScrn->mask.green;
				visual->blueMask = pScrn->mask.blue;
			}
		}
	}

	/* must be after RGB ordering fixed */
	fbPictureInit(pScreen,0,0);

	/*-----------------------------------------------------------------------------
	 *  init FB manager
	 *-----------------------------------------------------------------------------*/

//	pSmi->psHwInit(pScrn);
	xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
			"BDEBUG: pSmi->FBReserved is %d, pSmi->width is %d, \
			pSmi->Bpp is %d\n", pSmi->FBReserved, pSmi->width, pSmi->Bpp);

  /* Do the CRTC independent initialization */
   // if(!SMI750_HWInit(pScrn))
   //     LEAVE(FALSE);
	
#if 0 //for 750LE
                                 AvailFBArea.x2 = (pSmi->width + 15) & ~15;
#if (XORG_VERSION_CURRENT <= XORG_VERSION_NUMERIC(4,4,0,0,0)) 
/*ilena, SLES 9 work around: set memory for only 1 screen.*/	
		AvailFBArea.y2 = numLines = pScrn->virtualY;
#endif
#endif
	AvailFBArea.x1 = 0;
	AvailFBArea.y1 = 0;
	AvailFBArea.x2 = (pScrn->virtualX);
	AvailFBArea.y2 = pSmi->FBReserved / (pScrn->displayWidth * pScrn->bitsPerPixel / 8);
	/*XINF("FrameBuffer Box:(%d,%d) ==> (%d,%d)",
			AvailFBArea.x1,
			AvailFBArea.y1,
			AvailFBArea.x2,
			AvailFBArea.y2,);*/
	/*Initialization of the XFree86 framebuffer manager is done via Bool xf86InitFBManager(ScreenPtr pScreen, BoxPtr FullBox);
      FullBox represents the area of the framebuffer that the manager is allowed to manage. 
      This is typically a box with a width of pScrn->displayWidth and a height of as many lines as can be fit within the total video memory, 
      however, the driver can reserve areas at the extremities by passing a smaller area to the manager. */	

	/* xf86InitFBManager() must be called before XAA is initialized since XAA uses the manager for it's pixmap cache. */		
	xf86InitFBManager(pScreen,&AvailFBArea); 

	
#if SMI_RANDR
	if (!xf86SetDesiredModes(pScrn))
		LEAVE(FALSE);
#endif
	/*If backing store is to be supported (as is usually the case), initialise it.*/
	//miInitializeBackingStore(pScreen);//ilena: donot use since Xorg 1.14
	
	/* Initialise cursor functions */
	miDCInitialize(pScreen,xf86GetPointerScreenFuncs());

#if 0 /*for 502*/
    /* Initialize HW cursor layer.  Must follow software cursor
     * initialization.
     */
    /*
       monk @ 10/14/2010:
       for non-xrandr arch driver running on X server 1.6+ 
       (maybe 1.51+ not sure) different depth of two screen will make system 
       hang after move cursor from primary screen to secondary screen because
       of bugs of X server but if hardware cursor enabled, the bug will 
       disappare 
       Let's enable hardware cursor if screens with different color depth
     */
    if (pSmi->shadowFB)
    {
        Bool bRetCode;
        RefreshAreaFuncPtr refreshArea;
        refreshArea = SMI502_RefreshArea;
        if (pSmi->rotate || pSmi->RandRRotation) /*caesar modify*/
        {
            if (pSmi->PointerMoved == NULL)
            {
                pSmi->PointerMoved = pScrn->PointerMoved;
                pScrn->PointerMoved = SMI502_PointerMoved;
            }
        }
        bRetCode = ShadowFBInit (pScreen, refreshArea);
    }
#endif

	/* Initialise default colormap */
	if(!miCreateDefColormap(pScreen))
		LEAVE(FALSE);
	
	/* Initialize colormap layer. 
	 * Must follow initialization of the default colormap.
	 * And SetGamma call, else it will load palette with solid white.*/

	if (!xf86HandleColormaps
                        (pScreen, 256, pScrn->rgbBits, pSmi->psLoadPalette, NULL,
                         CMAP_RELOAD_ON_MODE_SWITCH | CMAP_PALETTED_TRUECOLOR))
                                LEAVE(FALSE);

	pScreen->SaveScreen = SMI_SaveScreen;
	pSmi->CloseScreen = pScreen->CloseScreen;
	pScreen->CloseScreen = SMI_CloseScreen; /* ilena: 502 does not use like this. please note!*/
#if SMI_RANDR
	/*This function completes the screen initialization process for the crtc and
	output objects. Call it near the end of the ScreenInit function, after the
	frame buffer and acceleration layers have been added.
	*/
	if(!xf86CrtcScreenInit(pScreen)) 
		LEAVE(FALSE);	
 #endif
	/*-----------------------------------------------------------------------------
	 *  register XV extension
	 *-----------------------------------------------------------------------------*/
	SMI_Videoinit(pScreen);

	/* Report any unused options (only for the first generation) */
	if(serverGeneration == 1)
		/*Prints out warning messages for each option in the list of options that isn't marked as used. 
		This is intended to show options that the driver hasn't recognised. */
		xf86ShowUnusedOptions(pScrn->scrnIndex,pScrn->options);

#if 0 // for 502
    if(pSmi->edid_enable)
    {
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
                "david print virtualX=%d, virtualY = %d\n", 
                pScrn->virtualX, pScrn->virtualY);
        pScrn->virtualX = pScrn->currentMode ->HDisplay;
        pScrn->virtualY = pScrn->currentMode->VDisplay;
        xf86DrvMsg(pScrn->scrnIndex, X_INFO, 
                "david print virtualX=%d, virtualY = %d\n", 
                pScrn->virtualX, pScrn->virtualY);
    }
#endif
	LEAVE(TRUE);
}


/*-----------------------------------------------------------------------------
 *  each adpator/entity should only invoke EntityInit one time
 *  I strongly suggest driver put register and videomemory mapping
 *  in EntityInit,because we always need only map once for dualview or SIMUL case
 *  and each screen's mapping routine could just take the mapped address
 *  and do some operateing like dulaview onscreen need an offset addon on
 *  mapped videomemory,while mmio is just a copy from mapped register address
 *-----------------------------------------------------------------------------*/
void SMI_EntityInit(int entIndex,pointer private)
{
	ScrnInfoPtr pScrn;
	SMIHWPtr pHw = (SMIHWPtr)private;
	SMIPtr pSmi = HWPSMI(pHw);
	vgaHWPtr hwp;

	ENTER();	

	/*entityIndex should better not lager than 16*/
	if(entIndex > MAX_ENT_IDX){
		xf86ErrorF("entityIndex:%d is too large\n",entIndex);
		LEAVE();
	}

	if(entity_priv_index[entIndex] == -1)
	{
		entity_priv_index[entIndex] = xf86AllocateEntityPrivateIndex();
		xf86Msg(X_INFO,"entity_prv_index[%d] = %d\n",entIndex,entity_priv_index[entIndex]);	
	}

	pScrn = SMIPSI(pSmi);

	
	if(pHw != NULL){
		
		/* Load the submodule vgahw for get the PIOBASE */
		if(xf86LoadSubModule(pScrn,"vgahw") == FALSE)
		{
			xf86Msg(X_ERROR,"Load SubModule:vgahw failed\n");	
			LEAVE();
		}
		/*This function allocates a vgaHWRec structure, and hooks it into the ScrnInfoRec's privates. */
		if(!vgaHWGetHWRec(pScrn))
		{
			xf86Msg(X_ERROR,"Create vgaHWRec failed\n");	
			LEAVE();
		}
		pSmi->psVgaPciInit(pScrn, entIndex);		

	    hwp = VGAHWPTR(pScrn);
		/* Initialize the PIOBase for hw init*/
#if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,12,0,0,0)    
        pHw->PIOBase = hwp->PIOOffset;
#else
        pHw->PIOBase = 0;//X donot use the PIOOffset anymore.

#endif
		
		SMI_EntityMap(entIndex, pHw);
			
		/* Save the registers and font for leaving VT before init HW*/
		pSmi->psSaveText(pScrn);

		/* The ScrnInfoRec's vtSema field should be set to TRUE 
		just prior to changing the video hardware's state. */
		pScrn->vtSema = TRUE;
		
		/* chip specific routine */
		if(pHw->pcEntityInit != NULL)
			pHw->pcEntityInit(entIndex,pHw);
		else
			ERROR("no pcEntityInit,hareware may not work!!!\n");		
	}else{
		/* map register and memory first */
		ERROR("private passed to Entity is null\n");
	}
	LEAVE();
}

void SMI_EntityEnter(int entIndex,pointer private)
{
	/*-----------------------------------------------------------------------------
	 *  entityEnter will be invoked before EnterVT
	 *  entityEnter called only once each adaptor,while EnterVT get called
	 *  twice in dualview mode
	 *-----------------------------------------------------------------------------*/
	SMIHWPtr pHw = (SMIHWPtr)private;
	ENTER();
	/*put anything regular and common here*/

	/* chip specific routine*/
	pHw->pcEntityEnter(entIndex,pHw);
	LEAVE();
}

void SMI_EntityLeave(int entIndex,pointer private)
{
	/*-----------------------------------------------------------------------------
	 *  entityLeave will be invoked after LeaveVT
	 *  entityLeave get called only onece each adaptor,while LeaveVT
	 *  get called twice in dualview mode
	 *-----------------------------------------------------------------------------*/
	SMIHWPtr pHw = (SMIHWPtr)private;
	ENTER();

	/*put anything regular and common here*/

	/* chip specific routine*/
	pHw->pcEntityLeave(entIndex,private);
	LEAVE();
}

Bool SMI_SwitchMode(int scrnIndex,DisplayModePtr mode,int flags)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	SMIPtr pSmi = SMIPTR(pScrn);
	
	ENTER();
	
	pSmi->psSetMode(pScrn,mode);

	
	LEAVE(TRUE);
}

void SMI_AdjustFrame(int scrnIndex,int x,int y,int flags)
{
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	/*SMIPtr pSmi = SMIPTR(pScrn);
	uint32_t base; raw data */
	ENTER();
	/*base = pSmi->FBOffset + (x+y*pScrn->displayWidth)*pScrn->bitsPerPixe/8;
	pSmi->psAdjustFrame(pScrn,base);*/
	LEAVE();
}


/*-----------------------------------------------------------------------------
 * This function is optional. 
 * It should be defined if the ScrnInfoRec driverPrivate field is used 
 * so that it can be freed when a screen is deleted by the common layer for reasons 
 * possibly beyond the driver's control. 
 * This function is not used in during normal (error free) operation. 
 * The per-generation data is freed by the CloseScreen() function. 
 * FreeScreen is invoked after CloseScreen
 *-----------------------------------------------------------------------------*/
static void SMI_FreeScreen(ScrnInfoPtr pScrn)
{
	//ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	ENTER();
	
	SMI_FreeRec(pScrn);

	if(xf86LoaderCheckSymbol("vgaHWFreeHWRec")){
		vgaHWFreeHWRec(pScrn);
	}
	
	LEAVE();
}

static ModeStatus SMI_ValidMode(ScrnInfoPtr pScrn,DisplayModePtr mode,Bool verbose,int flags)
{
	//ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	SMIPtr pSmi = SMIPTR(pScrn);
	ENTER();
	/* no interlace mode supported by all smi card byfar 
	if(mode->Flags & V_INTERLACE){
		return MODE_BAD;
	}*/
	if(pSmi->psValidMode)
		LEAVE(pSmi->psValidMode(pScrn,mode));
	else
	            LEAVE(MODE_OK);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SMI_EntityMap
 *  Description:  map chip depentent mmio,video memory and vgahw ram if in need
 * =====================================================================================
 */
static Bool SMI_EntityMap(int entIndex,SMIHWPtr pHw)
{
	int error;
	ENTER();
        ScrnInfoPtr pScrn;
        SMIPtr pSmi = HWPSMI(pHw);
        pScrn = SMIPSI(pSmi);
	
	/* map regular register */
#ifdef XSERVER_LIBPCIACCESS
	/*Map the specified memory range so that it can be accessed by the CPU.*/
void **result = (void**)&pHw->pReg;
	error = pci_device_map_range(
			pHw->pPci,
			pHw->phyaddr_reg,
			pHw->physize_reg,
			PCI_DEV_MAP_FLAG_WRITABLE,
			result);
	if(error){
		ERROR("Map mmio failed\n");
		LEAVE(FALSE);
	}
	else
	{
		xf86Msg(X_NOTICE,"Map mmio %x\n",pHw->pReg);
	}
#else
	pHw->pReg = xf86MapPciMem(entIndex,
			VIDMEM_MMIO|VIDMEM_MMIO_32BIT,
			pHw->pPci,
			pHw->phyaddr_reg,
			pHw->physize_reg);
	if(!pHw->pReg){
		ERROR("Map mmio failed\n");
		LEAVE(FALSE);
	}
	else
	{
		xf86Msg(X_NOTICE,"Map mmio %x\n",pHw->pReg);
	}
#endif

	/* map vgahw stuffs or chip-private stuffs */
	if(pHw->pcDeepMap != NULL)
		pHw->pcDeepMap(pHw);
	
	pHw->physize_mem = pHw->pcFBSize(pHw);
	
	/* map regular video memory */
#ifdef	XSERVER_LIBPCIACCESS
	error = pci_device_map_range(
		pHw->pPci,
		pHw->phyaddr_mem,
		pHw->physize_mem,
		PCI_DEV_MAP_FLAG_WRITABLE| PCI_DEV_MAP_FLAG_WRITE_COMBINE,
		&pHw->pMem);
	if (error){
		ERROR("MAP framebuffer failed.\n");
		LEAVE(FALSE); 
	}
#else
	pHw->pMem = xf86MapPciMem(entIndex,
		   	VIDMEM_FRAMEBUFFER, 
		   	pHw->pPci,
			pHw->phyaddr_mem,
			pHw->physize_mem);
	if (!pHw->pMem){
		ERROR("MAP framebuffer failed.\n");
		LEAVE(FALSE);
	}
#endif
	if (!pSmi->lcdWidth)
		pSmi->lcdWidth = pScrn->virtualX;
	if (!pSmi->lcdHeight)
		pSmi->lcdHeight = pScrn->virtualY;
	LEAVE(TRUE);
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SMI_EntityUnmap
 *  Description:  unmap per-chip dependent vmem or regs
 * =====================================================================================
 */
static void SMI_EntityUnmap(int entIndex,SMIHWPtr pHw)
{

	ENTER();

	if(pHw->pReg)
	{
#ifndef XSERVER_LIBPCIACCESS
		xf86UnMapVidMem (entIndex,
				(pointer) pHw->phyaddr_reg, pHw->physize_reg);
#else
		pci_device_unmap_range(pHw->pPci,
				pHw->pReg, pHw->physize_reg);
#endif
		pHw->pReg = NULL;
	}

	if(pHw->pMem)
	{
#ifndef XSERVER_LIBPCIACCESS
		xf86UnMapVidMem (entIndex, 
				(pointer) pHw->phyaddr_mem, pHw->physize_mem);
#else
		pci_device_unmap_range(pHw->pPci,
				pHw->pMem,pHw->physize_mem);
#endif
		pHw->pMem = NULL;
	}
	
	LEAVE();
}
/*  */
static Bool SMI_SaveScreen(ScreenPtr pScreen,int mode)
{
	
/*	SMIPtr pSmi = SMIPTR(pScrn);
	Bool ret = TRUE;*/
	ENTER();
#if SMI_RANDR
#if 0
	ScrnInfoPtr pScrn = xf86Screens[pScreen->myNum];
	if(xf86IsUnblank(mode))
		pScrn->DPMSSet(pScrn, DPMSModeOn, 0);
   	 else
		pScrn->DPMSSet(pScrn, DPMSModeOff, 0);
#endif
#endif
#if 0
#if defined(__i386__) || defined(__amd64__)
	if(pSmi->chipId != SMI_502){
		ret = vgaHWSaveScreen(pScreen,mode);
	}
#endif
#endif
	LEAVE();
}

/* 
 * ===  FUNCTION  ======================================================================
 *         Name:  SMI_CloseScreen
 *  Description:  This is called at the end of each server generation
 *  It restores the original(text)mode.
 *  unmap the video memory and mapped registers
 *	free the data allocated in pSmi and pSmi itself and modules needed by X
 *  And call saved CloseScreen function
 *  It is not necessary to unload sub modules
 *  They will be unloaded automatically.
 * =====================================================================================
 */
static Bool SMI_CloseScreen(ScreenPtr pScreen)
{
    int scrnIndex;
    scrnIndex = pScreen->myNum;
	ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	vgaHWPtr hwp = VGAHWPTR(pScrn);
	SMIPtr pSmi = SMIPTR(pScrn);

	ENTER();
	xf86Msg(X_INFO,"Close Screen %d\n",scrnIndex);

	if(pScrn->vtSema)
	{
		SMI_LeaveVT(pScrn);
	}

#if 0
    if (pSmi->AccelInfoRec != NULL)
    {
		/* The operation of driver for 2D
		1. The hardware part is enabled when hardware init and never disabled
		2. The driver will create xaa when screen init and destroy it when close screen
		3. The 2d state will remain when switch VT and these is no any opertion */
        xf86DrvMsg ("", X_INFO, "Destroy Accel on line %d\n", __LINE__);
        XAADestroyInfoRec (pSmi->AccelInfoRec);
    }
#endif    
	
#if 0   //ilena: we need them later
    if (pSmi->CursorInfoRec != NULL)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        xf86DestroyCursorInfoRec (pSmi->CursorInfoRec);
    }
#endif	
    if (pSmi->ptrAdaptor != NULL)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        xfree (pSmi->ptrAdaptor);
    }
    if (pSmi->BlockHandler != NULL)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        pScreen->BlockHandler = pSmi->BlockHandler;
    }
#if 0	   //ilena: we need them later
    if (pSmi->I2C != NULL)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        xf86DestroyI2CBusRec (pSmi->I2C, FALSE, TRUE);
        xfree (pSmi->I2C);
        pSmi->I2C = NULL;
    }
    if (pSmi->I2C_secondary != NULL)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        xf86DestroyI2CBusRec (pSmi->I2C_secondary, FALSE, TRUE);
        xfree (pSmi->I2C_secondary);
        pSmi->I2C_secondary = NULL;
    }
    if (pSmi->ediddata!= NULL)
    {
        xf86DrvMsg ("", X_INFO, "free edidbuffer line %d\n", __LINE__);
        /*xf86DestroyI2CBusRec (pSmi->I2C_secondary, FALSE, TRUE);*/
        xfree (pSmi->ediddata);
        pSmi->ediddata = NULL;
    }	
    /* #670 */
    if (pSmi->pSaveBuffer)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        xfree (pSmi->pSaveBuffer);
    }
    /* #920 */
    if (pSmi->paletteBuffer)
    {
        xf86DrvMsg ("", X_INFO, "line %d\n", __LINE__);
        xfree (pSmi->paletteBuffer);
    }

#endif

	if(pSmi->pVbe){
	/* vbe is short for VESA BIOS Extensions ?
	a VESA standard, currently at version 3, 
	that defines the interface that can be used by software 
	to access compliant video boards at high resolutions and bit depths.
	*/
		xf86Msg(X_INFO,"Close Screen Free vbe\n");
		vbeFree(pSmi->pVbe);
		pSmi->pVbe = NULL;
	}

	if(pSmi->pInt10 != NULL){
	/* Int10 module, a module that uses the int10 call to the BIOS of the graphics card to initialize it. 
	Normally, only secondary cards are soft-booted using the Int10 module, 
	as the primary card has already been initialized by the BIOS at boot time.*/
		xf86Msg(X_INFO,"Close Screen Free int10\n");
		xf86FreeInt10(pSmi->pInt10);
		pSmi->pInt10 = NULL;
	}

	if(pSmi->clockRanges != NULL){
		xf86Msg(X_INFO,"Close Screen Free clock range\n");
		xfree(pSmi->clockRanges);
		pSmi->clockRanges = NULL;
	}

	if(pSmi->linePitches != NULL){
		xf86Msg(X_INFO,"Close Screen Free line pitches\n");
		xfree(pSmi->linePitches);
		pSmi->linePitches = NULL;
	}

	if(pSmi->Options !=NULL){
		xf86Msg(X_INFO,"Close Screen Free smi options\n");
		xfree(pSmi->Options);
		pSmi->Options = NULL;
	}

    /* 	Is it necessary to ummap entity when closing a screen 
		In the entity arch, there is no a function for closing the entity
		So, putting the corresponding operation here and adding a 'if' to judge
		it is the last screen. */
	if(pSmi->pHardware->dual == 1)
	{
		xf86Msg(X_INFO,"Close Screen Free entity and hw rec\n");
		if(pScrn->vtSema)
		    SMI_EntityLeave(pScrn->entityList[0],pSmi->pHardware);
		/* 'Leave' first and then 'Unmap',
		because 'Leave' will use pHw->mem which assign NULL in 'Unmap' */
		SMI_EntityUnmap(pScrn->entityList[0],pSmi->pHardware);

		pSmi->pHardware->pcCloseAllScreen(pSmi->pHardware);
	}
	if(hwp != NULL){
		xf86Msg(X_INFO,"Close Screen Free vga\n");
		vgaHWUnmapMem (pScrn);
		vgaHWFreeHWRec(xf86Screens[scrnIndex]);
	}
	pSmi->pHardware->dual -= 1;
	pScrn->vtSema = FALSE;
	pScreen->CloseScreen = pSmi->CloseScreen;
	xfree(pSmi);
	LEAVE((*pScreen->CloseScreen)(pScreen));
}

static Bool SMI_EnterVT(ScrnInfoPtr pScrn)
{
	//ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	SMIPtr pSmi = SMIPTR(pScrn);
	ENTER();
	
    if(!pSmi){
		xf86Msg(X_INFO,"ilena: enter VT, pSmi = NULL \n");
		LEAVE(FALSE);
    }
    if(!pSmi->pHardware){
		xf86Msg(X_INFO,"ilena: enter VT, pHardware = NULL \n");
		LEAVE(FALSE);
    }
	else if(pSmi->pHardware->Chipset){
		xf86Msg(X_INFO,"ilena: enter VT, Chipset=[%x]\n", pSmi->pHardware->Chipset);
		xf86Msg(X_INFO,"ilena: enter VT, PIOBase=[%x]\n", pSmi->pHardware->PIOBase);
		if(!pSmi->pHardware->pPci){
			xf86Msg(X_INFO,"ilena: enter VT, pPci=err\n");
			LEAVE(FALSE);
		}
	}
	else{
		xf86Msg(X_INFO,"ilena: enter VT, Chipset=err\n");
		LEAVE(FALSE);
	}
#if 0
/*
 * if you unmapped and remapped , accessing video memory 
 * will need to modify the framebuffer layers of new
 * location in virtual memory. But we don't need this.
 * if adding it ,make sure you give the correct address in dualview
 */
    pScrn->pScreen->ModifyPixmapHeader(pScrn->pScreen->GetScreenPixmap(pScrn->pScreen),

            -1,-1,-1,-1,-1, pSmi->pFB + pSmi->FBOffset);
if XORG_VERSION_CURRENT < XORG_VERSION_NUMERIC(1,10,0,0,0)
    pScrn->pixmapPrivate.ptr = pSmi->pFB;
#endif		
//	pSmi->psGetMapResult(pScrn);

	pSmi->psSetMode(pScrn,pScrn->currentMode);
	pSmi->psSetDisplay(pScrn,pScrn->currentMode);
#if SMI_RANDR  
	if (!xf86SetDesiredModes(pScrn))
        LEAVE(FALSE);
#endif

#if  SMI_USE_VIDEO
    if(SMI_NEWLYNX(pSmi->pHardware->devId))
    {
        pSmi->pHardware->pcVideoReset(pScrn);
    }
#endif    
	/* Video reset stuffs here*/
	/* To do */
	LEAVE(TRUE);	
}

static void SMI_LeaveVT(ScrnInfoPtr pScrn )
{
	//ScrnInfoPtr pScrn = xf86Screens[scrnIndex];
	SMIPtr pSmi = SMIPTR(pScrn);

	ENTER();

	/* Clear the current frame buffer */
	memset(pSmi->pFB, 0, pSmi->videoRAMBytes);

	LEAVE();	
}
