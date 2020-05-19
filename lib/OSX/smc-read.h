#pragma once
/*
Portions of this code Copyright (c) 2020 Frank Stock under the MIT License.
For details see the original code in the GitHub repo at https://github.com/pcafstockf/smc-reader
*/

#ifndef SMC_READER_H
#define SMC_READER_H

#include <IOKit/IOKitLib.h>

#ifdef __cplusplus
extern "C" {
#endif

// SMC data types are described in human readable string format, even though the SMC itself considers types to be 32 bit unsigned integers.
// Comparing two numbers is a *lot* faster than converting to a string and doing calling strcmp, so this library works with integers.
#define DATATYPE_FP1F_KEY 0x66703166    // "fp1f"
#define DATATYPE_FP4C_KEY 0x66703463    // "fp4c"
#define DATATYPE_FP5B_KEY 0x66703562    // "fp5b"
#define DATATYPE_FP6A_KEY 0x66703661    // "fp6a"
#define DATATYPE_FP79_KEY 0x66703739    // "fp79"
#define DATATYPE_FP88_KEY 0x66703838    // "fp88"
#define DATATYPE_FPA6_KEY 0x66706136    // "fpa6"
#define DATATYPE_FPC4_KEY 0x66706334    // "fpc4"
#define DATATYPE_FPE2_KEY 0x66706532    // "fpe2"

#define DATATYPE_SP1E_KEY 0x73703165    // "sp1e"
#define DATATYPE_SP3C_KEY 0x73703363    // "sp3c"
#define DATATYPE_SP4B_KEY 0x73703462    // "sp4b"
#define DATATYPE_SP5A_KEY 0x73703561    // "sp5a"
#define DATATYPE_SP69_KEY 0x73703639    // "sp69"
#define DATATYPE_SP78_KEY 0x73703738    // "sp78"
#define DATATYPE_SP87_KEY 0x73703837    // "sp87"
#define DATATYPE_SP96_KEY 0x73703936    // "sp96"
#define DATATYPE_SPB4_KEY 0x73706234    // "spb4"
#define DATATYPE_SPF0_KEY 0x73706630    // "spf0"

#define DATATYPE_UINT8_KEY 0x75693820   // "ui8 "
#define DATATYPE_UINT16_KEY 0x75693136  // "ui16"
#define DATATYPE_UINT32_KEY 0x75693332  // "ui32"

#define DATATYPE_SI8_KEY 0x73693820     // "si8 "
#define DATATYPE_SI16_KEY 0x73693136    // "si16"

#define DATATYPE_PWM_KEY 0x7B70776D     // "{pwm"
#define DATATYPE_FLAG_KEY 0x666C6167    // "flag"
#define DATATYPE_HEX_KEY 0x6865785F     // "hex_"

uint32_t stringToKey(const char* str);

void keyToString(uint32_t key, char* str);

// Some useful constants for talking to the SMC
#define KERNEL_INDEX_SMC      2
#define SMC_CMD_READ_BYTES    5
#define SMC_CMD_READ_INDEX    8
#define SMC_CMD_READ_KEYINFO  9

// For purposes of this library, we have a 32 byte buffer for exchanging command specific data with the SMC.
typedef uint8_t SMCBytes_t[32];

// Declare the memory layout of data exchanged with the SMC
typedef struct {
	uint8_t major;
	uint8_t minor;
	uint8_t build;
	uint8_t reserved[1];
	uint16_t release;
} SMCKeyDataVers;

typedef struct {
	uint16_t version;
	uint16_t length;
	uint32_t cpuPLimit;
	uint32_t gpuPLimit;
	uint32_t memPLimit;
} SMCKeyDataLimits;

// This declares the meta info about a key.
typedef struct {
	uint32_t dataSize;
	uint32_t dataType;
	uint8_t dataAttributes;
} SMCKeyMetaData;

typedef struct {
	uint32_t key;
	SMCKeyDataVers vers;
	SMCKeyDataLimits limitData;
	SMCKeyMetaData keyInfo;
	uint8_t result;
	uint8_t status;
	uint8_t data8;
	uint32_t data32;
	SMCBytes_t bytes;
} SMCKeyData;

/**
 * Code using I/O Kit usually follows the same pattern:
 *  Find the service (usually via IOServiceGetMatchingServices)
 *  Open a connection to the service with IOServiceOpen
 *  Send some message to the service and get the result using one of the IOConnectCall*** functions.
 *  Close the service with IOServiceClose
 * This function performs the first two steps and if successful initializes the connection reference.
 *
 * @param conn  Reference to a connection handle used to communicate with the SMC.  This value is undefined if the connection was not opened successfully.
 * @return      kIOReturnSuccess if the connection was opened successfully, some other error if not.
 */
IOReturn AppleSMCOpen(io_connect_t* conn);

/**
 * Read the numeric value of a key from the SMC.
 *
 * @param conn      Reference to the connection handle obtained from @see AppleSMCOpen
 * @param key       A human readable string describing the key (such as "PC0C" for CPU Core power in watts).
 * @param value     The current value of 'key'.  This value is undefined if the function returns does not return kIOReturnSuccess.
 * @return          kIOReturnSuccess if the value was read successfully, some other error if not.
 */
IOReturn AppleSMCReadNumber(io_connect_t conn, const char* key, double* value);

/**
 * Closes the connection handle obtained previously from @see AppleSMCOpen
 *
 * @param conn      Reference to the connection handle obtained from @see AppleSMCOpen
 * @return          kIOReturnSuccess if the connection was closed successfully, some other error if not.
 */
IOReturn AppleSMCClose(io_connect_t conn);

#ifdef __cplusplus
}
#endif


#endif //SMC_READER_H
