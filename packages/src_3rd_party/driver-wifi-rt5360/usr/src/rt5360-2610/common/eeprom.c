/*
 *************************************************************************
 * Ralink Tech Inc.
 * 5F., No.36, Taiyuan St., Jhubei City,
 * Hsinchu County 302,
 * Taiwan, R.O.C.
 *
 * (c) Copyright 2002-2010, Ralink Technology, Inc.
 *
 * This program is free software; you can redistribute it and/or modify  *
 * it under the terms of the GNU General Public License as published by  *
 * the Free Software Foundation; either version 2 of the License, or     *
 * (at your option) any later version.                                   *
 *                                                                       *
 * This program is distributed in the hope that it will be useful,       *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 * GNU General Public License for more details.                          *
 *                                                                       *
 * You should have received a copy of the GNU General Public License     *
 * along with this program; if not, write to the                         *
 * Free Software Foundation, Inc.,                                       *
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 *                                                                       *
 *************************************************************************/


#include "rt_config.h"

static struct {
	UINT32 	ChipVersion;
	PSTRING name;
} RTMP_CHIP_E2P_FILE_TABLE[] = {
	{0x3071,	"RT3092_PCIe_LNA_2T2R_ALC_V1_2.bin"},
	{0x3090,	"RT3092_PCIe_LNA_2T2R_ALC_V1_2.bin"},
	{0x3593,	"HMC_RT3593_PCIe_3T3R_V1_3.bin"},
	{0x5392,	"RT5392_PCIe_2T2R_ALC_V1_4.bin"},
	{0x5592,	"RT5592_PCIe_2T2R_V1_7.bin"},
	{0,}
};

INT RtmpChipOpsEepromHook(
	IN RTMP_ADAPTER *pAd,
	IN INT			infType)
{
	RTMP_CHIP_OP	*pChipOps = &pAd->chipOps;
#ifdef RT30xx
#ifdef RTMP_EFUSE_SUPPORT
	UINT32			eFuseCtrl, mac_val;
	int index;
#endif
#endif

#ifdef RTMP_FLASH_SUPPORT
	pChipOps->eeinit = rtmp_nv_init;
	pChipOps->eeread = rtmp_ee_flash_read;
	pChipOps->eewrite = rtmp_ee_flash_write;
	return 0;
#endif /* RTMP_FLASH_SUPPORT */

#ifdef RT30xx
#ifdef RTMP_EFUSE_SUPPORT
	index = 0;
	do
	{
		if (RTMP_TEST_FLAG(pAd, fRTMP_ADAPTER_NIC_NOT_EXIST))			
			return -1;
		
		RTMP_IO_READ32(pAd, MAC_CSR0, &mac_val);
		pAd->MACVersion = mac_val;

		if ((pAd->MACVersion != 0x00) && (pAd->MACVersion != 0xFFFFFFFF))
			break;

		RTMPusecDelay(10);
	} while (index++ < 100);
	
	pAd->bUseEfuse=FALSE;
#ifdef RT3290
	if (IS_RT3290(pAd))
	{
		RTMP_IO_READ32(pAd, EFUSE_CTRL_3290, &eFuseCtrl);
	}
	else
#endif /* RT3290 */
	{
		RTMP_IO_READ32(pAd, EFUSE_CTRL, &eFuseCtrl);
	}

	pAd->bUseEfuse = ( (eFuseCtrl & 0x80000000) == 0x80000000) ? 1 : 0;	
	if(pAd->bUseEfuse)
	{
		pChipOps->eeinit = eFuse_init;
		pChipOps->eeread = rtmp_ee_efuse_read16;
		pChipOps->eewrite = rtmp_ee_efuse_write16;
		DBGPRINT(RT_DEBUG_OFF, ("NVM is EFUSE\n"));
		return 0 ;	
	}
	else
	{
		pAd->bFroceEEPROMBuffer = FALSE;
		DBGPRINT(RT_DEBUG_OFF, ("NVM is EEPROM\n"));
	}
#endif /* RTMP_EFUSE_SUPPORT */
#endif /* RT30xx */

	switch(infType) 
	{
#ifdef RTMP_PCI_SUPPORT
		case RTMP_DEV_INF_PCI:
		case RTMP_DEV_INF_PCIE:
#ifdef RT3290
			if (IS_RT3290(pAd))
				pAd->EEPROMAddressNum = 8;     /* 93C66 */
			else
#endif /* RT3290 */
			{
				UINT32 val;
				RTMP_IO_READ32(pAd, E2PROM_CSR, &val);
				if ((val & 0x30) == 0)
					pAd->EEPROMAddressNum = 6;	/* 93C46*/
				else if ((val & 0x30) == 0x10)
					pAd->EEPROMAddressNum = 8;	/* 93C66*/
				else
					pAd->EEPROMAddressNum = 8;	/* 93C86*/
			}
			pChipOps->eeinit = NULL;
			pChipOps->eeread = rtmp_ee_prom_read16;
			pChipOps->eewrite = rtmp_ee_prom_write16;
			break;
#endif /* RTMP_PCI_SUPPORT */


		default:
			DBGPRINT(RT_DEBUG_ERROR, ("RtmpChipOpsEepromHook() failed!\n"));
			break;
	}

	return 0;
}

BOOLEAN rtmp_get_default_bin_file_by_chip(
	IN PRTMP_ADAPTER pAd,
	IN UINT32 	ChipVersion,
	OUT PSTRING *pBinFileName)
{
	BOOLEAN RetVal = FALSE;
	INT i;
	for (i = 0; RTMP_CHIP_E2P_FILE_TABLE[i].ChipVersion != 0; i++ )
	{
		if (RTMP_CHIP_E2P_FILE_TABLE[i].ChipVersion == ChipVersion)
		{
			*pBinFileName = RTMP_CHIP_E2P_FILE_TABLE[i].name;
			DBGPRINT(RT_DEBUG_OFF, 
						("%s(): Found E2P bin file name:%s\n",
						__FUNCTION__, *pBinFileName));
			RetVal = TRUE;
			break;
		}
	}

	if (RetVal != TRUE)
		DBGPRINT(RT_DEBUG_ERROR, 
				("%s(): E2P bin file name not found.\n", __FUNCTION__));
	
	return RetVal;
}
