#include "stdio.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include "main.h"

static int firmware_fd = 0;
static uint8_t* pu8Firmware = NULL;

int main(int argc, char const *argv[])
{
	tsFW_Info sFW_Info;

	printf("cmd file name is: %s\n",argv[0] );
	printf("cmd file name is: %s\n",argv[1] );

    /* Have file to program */

    if (iFW_Open(&sFW_Info, (char*)argv[1]))
    {
        /* Error with file. FW module has displayed error so just exit. */
        return -1;
    }

    /*if (BL_eReprogram(&sFW_Info) != E_STATUS_OK)
    {
        return -1;
    }*/

    printf("Success\n");
	getchar();
	/* code */
	return 0;
}

/****************************************************************************
 *
 * NAME: iFW_Open
 *
 * DESCRIPTION:
 *  Reads required information from the binary image
 *
 * PARAMETERS:  Name            RW  Usage
 *              pu8Firmware     R   Pointer to firmware image to analyse
 *              psFW_Info       W   Pointer to Info structure to be filled in
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occured
 *
 ****************************************************************************/
teFWStatus iFW_Open(tsFW_Info *psFW_Info, char *pcFirmwareFile)
{
    struct stat sb;
    
    firmware_fd = open(pcFirmwareFile, O_RDONLY);
    if (firmware_fd < 0)
    {
        printf("Could not open firmware file\n");
        return E_FW_INVALID_FILE;
    }
    else
    {
    	printf("Succeed to open firmware file\n");
    }
    
    if (fstat(firmware_fd, &sb) == -1)           /* To obtain file size */
    {
        printf("Could not stat file");
        return -1;
    }
    else
    {
    	printf("Succeed to open firmware file\n");
    }
    
    psFW_Info->u32ImageLength = (uint32_t)sb.st_size;

    printf("psFW_Info->u32ImageLength:%08x\n",psFW_Info->u32ImageLength);
    
    /* Copy-on-write, changes are not written to the underlying file. */
    pu8Firmware = mmap(NULL, sb.st_size, PROT_READ | PROT_WRITE, MAP_PRIVATE, firmware_fd, 0);
    
    if (pu8Firmware == MAP_FAILED)
    {
        printf("Could not mmap file");
        return E_FW_ERROR;
    }
    else
    {
    	printf("Succeed to open firmware file\n");
    }
    
    return iFW_GetInfo(pu8Firmware, psFW_Info);
}

/****************************************************************************
 *
 * NAME: iFW_GetInfo
 *
 * DESCRIPTION:
 *  Reads required information from the binary image
 *
 * PARAMETERS:  Name            RW  Usage
 *              pu8Firmware     R   Pointer to firmware image to analyse
 *              psFW_Info       W   Pointer to Info structure to be filled in
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occured
 *
 ****************************************************************************/
teFWStatus iFW_GetInfo(uint8_t *pu8Firmware, tsFW_Info *psFW_Info)
{
    tsBL_BinHeaderV2 *psHeader              = (tsBL_BinHeaderV2 *)pu8Firmware;

    // JN5148-J01 onwards uses multiimage bootloader - check for it's magic number.   
    if ((ntohl(psHeader->au32BootImageRecord[0]) == 0x12345678) &&
        (ntohl(psHeader->au32BootImageRecord[1]) == 0x11223344) &&
        (ntohl(psHeader->au32BootImageRecord[2]) == 0x55667788))
    {
        printf("tsBL_BinHeaderV2 used!\n");

        printf("tsBL_BinHeaderV2 header length:%d!\n",sizeof(tsBL_BinHeaderV2));

        psFW_Info->u32ROMVersion                = ntohl(psHeader->u32ROMVersion);
        
        psFW_Info->u32TextSectionLoadAddress    = 0x04000000 + (((ntohl(psHeader->u32DataSectionInfo)) >> 16) * 4);
        psFW_Info->u32TextSectionLength         = (((ntohl(psHeader->u32DataSectionInfo)) & 0x0000FFFF) * 4);
        psFW_Info->u32BssSectionLoadAddress     = 0x04000000 + (((ntohl(psHeader->u32BssSectionInfo)) >> 16) * 4);
        psFW_Info->u32BssSectionLength          = (((ntohl(psHeader->u32BssSectionInfo)) & 0x0000FFFF) * 4);

        psFW_Info->u32ResetEntryPoint           = psHeader->u32ResetEntryPoint;
        psFW_Info->u32WakeUpEntryPoint          = psHeader->u32WakeUpEntryPoint;
        
        /* Pointer to and length of image for flash */
        psFW_Info->pu8ImageData                 = (uint8_t*)&(psHeader->au32BootImageRecord[0]);
        
        psFW_Info->u32MacAddressLocation        = 0x10;
        
        /* Pointer to text section in image for RAM */
        psFW_Info->pu8TextData                  = &(psHeader->u8TextDataStart);
    }
    else
    {
        printf("tsBL_BinHeaderV1 used!\n");

        printf("tsBL_BinHeaderV1 header length:%d!\n",sizeof(tsBL_BinHeaderV1));

        tsBL_BinHeaderV1 *psHeader              = (tsBL_BinHeaderV1 *)pu8Firmware;

        psFW_Info->u32ROMVersion                = ntohl(psHeader->u32ROMVersion);
        
        psFW_Info->u32TextSectionLoadAddress    = ntohl(psHeader->u32TextStartAddress);
        psFW_Info->u32TextSectionLength         = ntohl(psHeader->u32TextLength);
        psFW_Info->u32BssSectionLoadAddress     = ntohl(psHeader->u32BssStartAddress);
        psFW_Info->u32BssSectionLength          = ntohl(psHeader->u32BssLength);
        
        psFW_Info->u32ResetEntryPoint           = psHeader->u32ResetEntryPoint;
        psFW_Info->u32WakeUpEntryPoint          = psHeader->u32WakeUpEntryPoint;
        
        /* Pointer to and length of image for flash */
        psFW_Info->pu8ImageData                 = &(psHeader->u8ConfigByte0);
        //psFW_Info->u32ImageLength               = sizeof(tsBL_BinHeaderV1) + psFW_Info->u32TextSectionLength;
        
        psFW_Info->u32MacAddressLocation        = 0x30;
        
        /* Pointer to text section in image for RAM */
        psFW_Info->pu8TextData                  = &(psHeader->u8TextDataStart);
        
    }
    
    printf("u32ROMVersion:         0x%08x\r\n", psFW_Info->u32ROMVersion);
    printf("au32BootImageRecord[0]:  0x%08x\r\n",ntohl(psHeader->au32BootImageRecord[0]));
    printf("au32BootImageRecord[1]:  0x%08x\r\n",ntohl(psHeader->au32BootImageRecord[1]));
    printf("au32BootImageRecord[2]:  0x%08x\r\n",ntohl(psHeader->au32BootImageRecord[2]));
    printf("au32BootImageRecord[3]:  0x%08x\r\n",ntohl(psHeader->au32BootImageRecord[3]));
    printf("u64MacAddress:         0x%08x\r\n",ntohl(psHeader->u64MacAddress));
    printf("au32EncryptionInitialisationVector[0]:    0x%08x\r\n", ntohl(psHeader->au32EncryptionInitialisationVector[0]));
    printf("au32EncryptionInitialisationVector[1]:    0x%08x\r\n", ntohl(psHeader->au32EncryptionInitialisationVector[1]));
    printf("au32EncryptionInitialisationVector[2]:    0x%08x\r\n", ntohl(psHeader->au32EncryptionInitialisationVector[2]));
    printf("au32EncryptionInitialisationVector[3]:    0x%08x\r\n", ntohl(psHeader->au32EncryptionInitialisationVector[3]));
    printf("u32DataSectionInfo:    0x%08x\r\n", ntohl(psHeader->u32DataSectionInfo));
    printf("u32TextSectionLength:  0x%08x\r\n", (((ntohl(psHeader->u32DataSectionInfo)) & 0x0000FFFF) * 4));
    printf("u32BssSectionInfo:  0x%08x\r\n", (((ntohl(psHeader->u32BssSectionInfo)) & 0x0000FFFF) * 4));
    printf("u32WakeUpEntryPoint:  0x%08x\r\n", (ntohl(psHeader->u32WakeUpEntryPoint)));
    printf("u32ResetEntryPoint:  0x%08x\r\n", (ntohl(psHeader->u32ResetEntryPoint)));
    printf("u8TextDataStart:  0x%08x\r\n", (ntohl(psHeader->u8TextDataStart)));

    return E_FW_OK;
}

/****************************************************************************
 *
 * NAME: BL_iReprogram
 *
 * DESCRIPTION:
 *  Reprograms the device
 *
 * PARAMETERS:  Name            RW  Usage
 *              pu8Firmware     R   Pointer to firmware image to download
 *              pu64Address     R   Pointer to MAC Address. If NULL, read from flash.
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
teStatus BL_eReprogram(tsFW_Info *psFWImage)
{

    int n;
    uint8_t u8ChunkSize;

    printf("psFWImage->u32ImageLength:%08x!\n",psFWImage->u32ImageLength);

    for(n=0;n<128;n++)
       printf("psFWImage->pu8ImageData[%d]:%02x\n",n,psFWImage->pu8ImageData[n]);

    //for(n = 0; n < psFWImage->u32ImageLength;)
    for(n = 0; n < 257;)
    {
        if((psFWImage->u32ImageLength - n) > 128)
        {        
            //printf("(psFWImage->u32ImageLength - n) > 128!\n");
            u8ChunkSize = 128;
        }
        else
        {
            u8ChunkSize = psFWImage->u32ImageLength - n;
            printf("u8ChunkSize = psFWImage->u32ImageLength - n,u8ChunkSize is:%d\n",u8ChunkSize);
        }

        if(iBL_WriteFlash(n, u8ChunkSize, psFWImage->pu8ImageData + n) == -1)
        {
            printf("Failed to write at address 0x%08x\r\n", n);
            return E_STATUS_ERROR;
        }

        n += u8ChunkSize;

        //printf("%c[AWriting:   %3d%%\n", 0x1B, (n * 100) / psFWImage->u32ImageLength);
    }

    return E_STATUS_OK;
}

/****************************************************************************
 *
 * NAME: iBL_WriteFlash
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occurred
 *
 ****************************************************************************/
static int iBL_WriteFlash(uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer)
{
    uint8_t u8RxDataLen = 0;
    uint8_t au8CmdBuffer[6];
    teBL_MessageType eRxType = 0;

    if(u8Length > 0xfc || pu8Buffer == NULL)
    {
        return -1;
    }

    au8CmdBuffer[0] = (uint8_t)(u32Address >> 0) & 0xff;
    au8CmdBuffer[1] = (uint8_t)(u32Address >> 8) & 0xff;
    au8CmdBuffer[2] = (uint8_t)(u32Address >> 16) & 0xff;
    au8CmdBuffer[3] = (uint8_t)(u32Address >> 24) & 0xff;
    au8CmdBuffer[4] = u8Length;
    au8CmdBuffer[5] = 0;

    eBL_Request(4, au8CmdBuffer, u8Length, pu8Buffer, &eRxType, &u8RxDataLen, pu8Buffer);

    return u8Length;
}

/****************************************************************************/
/***        Local Functions                                               ***/
/****************************************************************************/
/****************************************************************************
 *
 * NAME: eBL_Request
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * void
 ****************************************************************************/
static teBL_Response eBL_Request(uint8_t u8HeaderLen, uint8_t *pu8Header, uint8_t u8TxLength, uint8_t *pu8TxData,
                          teBL_MessageType *peRxType, uint8_t *pu8RxLength, uint8_t *pu8RxData)
{
    /* Check data is not too long */
    if(u8TxLength > 0xfd)
    {
        printf("Data too long\r\n");
        return -1;
    }

    /* Send message */
    if(iBL_WriteMessage(u8HeaderLen, pu8Header, u8TxLength, pu8TxData) == -1)
    {
        return -1;
    }

    /* Get the response to the request */
    return 0;
}

/****************************************************************************
 *
 * NAME: iBL_WriteMessage
 *
 * DESCRIPTION:
 *
 * PARAMETERS: Name        RW  Usage
 *
 * RETURNS:
 * int          0 if success
 *              -1 if an error occured
 *
 ****************************************************************************/
static int iBL_WriteMessage(uint8_t u8HeaderLength, uint8_t *pu8Header, uint8_t u8Length, uint8_t *pu8Data)
{
    uint8_t u8CheckSum = 0;
    int n;

    uint8_t au8Msg[256];

    /* total message length cannot be > 255 bytes */
    if(u8HeaderLength + u8Length >= 0xfe)
    {
        printf("Length too big\r\n");
        return(-1);
    }

    /* Message length */
    au8Msg[0] = u8HeaderLength + u8Length + 2;

    /* Message type */
    au8Msg[1] = 0x09;

    /* Message header */
    memcpy(&au8Msg[2], pu8Header, u8HeaderLength);

    /* Message payload */
    memcpy(&au8Msg[2 + u8HeaderLength], pu8Data, u8Length);

    for(n = 0; n < u8HeaderLength + u8Length + 2; n++)
    {
        u8CheckSum ^= au8Msg[n];
    }

    /* Message checksum */
    au8Msg[u8HeaderLength + u8Length + 2] = u8CheckSum;

    printf("Tx buffer length:%d\r\n",u8HeaderLength + u8Length + 3);
    printf("u8HeaderLength length:%d\r\n",u8HeaderLength);
    printf("u8Length:%d\r\n",u8Length);

    printf("Tx data:");

    for(n=0;n<u8HeaderLength + u8Length + 3;n++)
    {
        printf("0x%02x ",au8Msg[n]);
    }

    printf("\r\n");

    /* Write whole message to UART */
    //UART_eWrite(dev, au8Msg, u8HeaderLength + u8Length + 3);

    printf("Bytes write:%d\r\n",n);

    return(0);

}