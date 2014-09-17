typedef signed char     int8_t;
typedef short int       int16_t;
typedef int         int32_t;
typedef long long int       int64_t;

typedef unsigned char       uint8_t;
typedef unsigned short int  uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long int  uint64_t; 

#define O_RDONLY    0
#define O_WRONLY    1
#define O_RDWR      2
#define O_BINARY    4

/** Abstracted firmware information common across bootloaders. */
typedef struct
{
    uint32_t    u32ROMVersion;                  /**< ROM Version that the FW was built for */
    uint32_t    u32TextSectionLoadAddress;      /**< Address to load .text section */
    uint32_t    u32TextSectionLength;           /**< Length (bytes) of .text section */
    uint32_t    u32BssSectionLoadAddress;       /**< Address of start of .bss section */
    uint32_t    u32BssSectionLength;            /**< Length (bytes) of .bss section */
    uint32_t    u32WakeUpEntryPoint;            /**< Address of wake up (warm start) entry point */
    uint32_t    u32ResetEntryPoint;             /**< Address of rest (cold start) entry point */
    
    uint8_t*    pu8ImageData;                   /**< Pointer to loaded image data for Flash */
    uint32_t    u32ImageLength;                 /**< Length (bytes) of image for Flash */
    
    uint32_t    u32MacAddressLocation;          /**< Offset in file of MAC address */
    uint8_t*    pu8TextData;                    /**< Pointer to loaded .text section for RAM */
} tsFW_Info;

typedef struct
{
    uint8_t     u8ConfigByte0;
    uint8_t     u8ConfigByte1;
    uint16_t    u16SpiScrambleIndex;
    uint32_t    u32TextStartAddress;
    uint32_t    u32TextLength;
    uint32_t    u32ROMVersion;
    uint32_t    u32Unused1;
    uint32_t    u32BssStartAddress;
    uint32_t    u32BssLength;
    uint32_t    u32WakeUpEntryPoint;
    uint32_t    u32ResetEntryPoint;
    uint8_t     au8OadData[12];
    uint8_t     u8TextDataStart;
} __attribute__ ((packed)) tsBL_BinHeaderV1;

typedef struct
{
    uint32_t    u32ROMVersion;
    uint32_t    au32BootImageRecord[4];
    uint64_t    u64MacAddress;
    uint32_t    au32EncryptionInitialisationVector[4];
    uint32_t    u32DataSectionInfo;
    uint32_t    u32BssSectionInfo;
    uint32_t    u32WakeUpEntryPoint;
    uint32_t    u32ResetEntryPoint;
    uint8_t     u8TextDataStart;
} __attribute__ ((packed)) tsBL_BinHeaderV2;

typedef enum
{
    E_STATUS_OK,
    E_STATUS_ERROR,
    E_STATUS_ERROR_WRITING,
    E_STATUS_ERROR_READING,
    E_STATUS_FAILED_TO_OPEN_FILE,
    E_STATUS_BAD_PARAMETER,
    E_STATUS_NULL_PARAMETER,
    E_STATUS_INCOMPATIBLE,
} teStatus;

typedef enum
{
    E_FW_OK,
    E_FW_ERROR,
    E_FW_INVALID_FILE,
} teFWStatus;

typedef enum
{
    E_BL_MSG_TYPE_FLASH_ERASE_REQUEST                   = 0x07,
    E_BL_MSG_TYPE_FLASH_ERASE_RESPONSE                  = 0x08,
    E_BL_MSG_TYPE_FLASH_PROGRAM_REQUEST                 = 0x09,
    E_BL_MSG_TYPE_FLASH_PROGRAM_RESPONSE                = 0x0a,
    E_BL_MSG_TYPE_FLASH_READ_REQUEST                    = 0x0b,
    E_BL_MSG_TYPE_FLASH_READ_RESPONSE                   = 0x0c,
    E_BL_MSG_TYPE_FLASH_SECTOR_ERASE_REQUEST            = 0x0d,
    E_BL_MSG_TYPE_FLASH_SECTOR_ERASE_RESPONSE           = 0x0e,
    E_BL_MSG_TYPE_FLASH_WRITE_STATUS_REGISTER_REQUEST   = 0x0f,
    E_BL_MSG_TYPE_FLASH_WRITE_STATUS_REGISTER_RESPONSE  = 0x10,
    E_BL_MSG_TYPE_RAM_WRITE_REQUEST                     = 0x1d,
    E_BL_MSG_TYPE_RAM_WRITE_RESPONSE                    = 0x1e,
    E_BL_MSG_TYPE_RAM_READ_REQUEST                      = 0x1f,
    E_BL_MSG_TYPE_RAM_READ_RESPONSE                     = 0x20,
    E_BL_MSG_TYPE_RAM_RUN_REQUEST                       = 0x21,
    E_BL_MSG_TYPE_RAM_RUN_RESPONSE                      = 0x22,
    E_BL_MSG_TYPE_FLASH_READ_ID_REQUEST                 = 0x25,
    E_BL_MSG_TYPE_FLASH_READ_ID_RESPONSE                = 0x26,
    E_BL_MSG_TYPE_SET_BAUD_REQUEST                      = 0x27,
    E_BL_MSG_TYPE_SET_BAUD_RESPONSE                     = 0x28,
    E_BL_MSG_TYPE_FLASH_SELECT_TYPE_REQUEST             = 0x2c,
    E_BL_MSG_TYPE_FLASH_SELECT_TYPE_RESPONSE            = 0x2d,
    
    E_BL_MSG_TYPE_GET_CHIPID_REQUEST                    = 0x32,
    E_BL_MSG_TYPE_GET_CHIPID_RESPONSE                   = 0x33,
} __attribute ((packed)) teBL_MessageType;

typedef enum
{
    E_BL_RESPONSE_OK                                    = 0x00,
    E_BL_RESPONSE_NOT_SUPPORTED                         = 0xff,
    E_BL_RESPONSE_WRITE_FAIL                            = 0xfe,
    E_BL_RESPONSE_INVALID_RESPONSE                      = 0xfd,
    E_BL_RESPONSE_CRC_ERROR                             = 0xfc,
    E_BL_RESPONSE_ASSERT_FAIL                           = 0xfb,
    E_BL_RESPONSE_USER_INTERRUPT                        = 0xfa,
    E_BL_RESPONSE_READ_FAIL                             = 0xf9,
    E_BL_RESPONSE_TST_ERROR                             = 0xf8,
    E_BL_RESPONSE_AUTH_ERROR                            = 0xf7,
    E_BL_RESPONSE_NO_RESPONSE                           = 0xf6,
    E_BL_RESPONSE_ERROR                                 = 0xf0,
} __attribute__ ((packed)) teBL_Response;

teFWStatus iFW_Open(tsFW_Info *psFW_Info, char *pcFirmwareFile);
teFWStatus iFW_GetInfo(uint8_t *pu8Firmware, tsFW_Info *psFW_Info);
teStatus BL_eReprogram(tsFW_Info *psFWImage);
static int iBL_WriteFlash(uint32_t u32Address, uint8_t u8Length, uint8_t *pu8Buffer);
static teBL_Response eBL_Request(uint8_t u8HeaderLen, uint8_t *pu8Header, uint8_t u8TxLength, uint8_t *pu8TxData,
                          teBL_MessageType *peRxType, uint8_t *pu8RxLength, uint8_t *pu8RxData);
static int iBL_WriteMessage(uint8_t u8HeaderLength, uint8_t *pu8Header, uint8_t u8Length, uint8_t *pu8Data);