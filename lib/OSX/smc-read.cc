/*
Portions of this code Copyright (c) 2020 Frank Stock under the MIT License.
For details see the original code in the GitHub repo at https://github.com/pcafstockf/smc-reader
*/

#ifndef BUILDING_NODE_EXTENSION
    #define BUILDING_NODE_EXTENSION
#endif
#include <node.h>
#include <v8.h>

#include "smc-read.h"
#include <string.h>
#include <math.h>
#include <arpa/inet.h>

/**
 * Most documentation on SMC keys are described in string format, even though the SMC itself considers keys to be 32 bit unsigned integers.
 * This function translates from a null terminated "C" string to a 32 bit unsigned integer that the SMC will recognize.
 */
uint32_t stringToKey(const char* str) {
	uint32_t tmp[2];
	strncpy((char*)&tmp, str, 4);	// This is necessary because a char* may not be 4 byte aligned.
	return ntohl(tmp[0]);
}

/**
 * Convert a SMC key to a human readable string.
 * @see stringToKey
 */
void keyToString(uint32_t key, char* str) {
	*((uint32_t*) str) = htonl(key);
	str[4] = 0;
}

// See header for documentation
IOReturn AppleSMCOpen(io_connect_t* conn) {
	if (conn == NULL)
		return kIOReturnInvalid;
	io_iterator_t existing;
	*conn = 0;
	// Create a matching dictionary that specifies an IOService class match.
	CFMutableDictionaryRef matching = IOServiceMatching("AppleSMC");
	// Look up registered IOService objects that match a matching dictionary.
	IOReturn result = IOServiceGetMatchingServices(kIOMasterPortDefault, matching, &existing);
	if (result != kIOReturnSuccess)
		return result;
	// Returns the next object in an iteration.
	io_object_t service = IOIteratorNext(existing);
	// Releases an object handle previously returned by IOKitLib.
	IOObjectRelease(existing);
	if (service == 0)
		return kIOReturnNotFound;
	// A request to create a connection to an IOService.
	result = IOServiceOpen(service, mach_task_self(), 0, conn);
	// Releases an object handle previously returned by IOKitLib.
	IOObjectRelease(service);
	return result;
}

// See header for documentation
IOReturn AppleSMCClose(io_connect_t conn) {
	// Close a connection to an IOService and destroy the connect handle.
	return IOServiceClose(conn);
}

// See header for documentation
IOReturn AppleSMCReadBuffer(io_connect_t conn, const char* key, uint32_t* dataType, SMCBytes_t buff, uint8_t* buffLen) {
	SMCKeyData inputStructure;
	SMCKeyData outputStructure;

	// We *probably* don't care that there is stale data on our stack, but I've found that the SMC_CMD_READ_KEYINFO command is looking at more than just 'key' and 'data8'.
	memset(&inputStructure, 0, sizeof(SMCKeyData));
	memset(&outputStructure, 0, sizeof(SMCKeyData));

	// Send a command to retrieve information about the key (specifically it's dataType and size).
	inputStructure.key = stringToKey(key);
	inputStructure.data8 = SMC_CMD_READ_KEYINFO;
	size_t structureOutputSize = sizeof(SMCKeyData);
	IOReturn result = IOConnectCallStructMethod(conn, KERNEL_INDEX_SMC, &inputStructure, sizeof(SMCKeyData), &outputStructure, &structureOutputSize);
	if (result != kIOReturnSuccess)
		return result;
	if (dataType != NULL)
		*dataType = outputStructure.keyInfo.dataType;
	if (buffLen != NULL)
		*buffLen = outputStructure.keyInfo.dataSize;

	// Send another command to retrieve the actual key value.
	inputStructure.keyInfo.dataSize = outputStructure.keyInfo.dataSize;
	inputStructure.data8 = SMC_CMD_READ_BYTES;
	result = IOConnectCallStructMethod(conn, KERNEL_INDEX_SMC, &inputStructure, sizeof(SMCKeyData), &outputStructure, &structureOutputSize);
	if (result != kIOReturnSuccess)
		return result;

	// Copy the key value into the buffer supplied by our caller.
	memcpy(buff, outputStructure.bytes, sizeof(outputStructure.bytes));

	return kIOReturnSuccess;
}

// See header for documentation
float ToSMCFloat(uint32_t dataType, uint16_t value) {
	float result = value;
	switch (dataType) {
		case DATATYPE_FP1F_KEY:
			result /= 32768.0f;
			break;
		case DATATYPE_FP4C_KEY:
			result /= 4096.0f;
			break;
		case DATATYPE_FP5B_KEY:
			result /= 2048.0f;
			break;
		case DATATYPE_FP6A_KEY:
			result /= 1024.0f;
			break;
		case DATATYPE_FP79_KEY:
			result /= 512.0f;
			break;
		case DATATYPE_FP88_KEY:
			result /= 256.0f;
			break;
		case DATATYPE_FPA6_KEY:
			result /= 64.0f;
			break;
		case DATATYPE_FPC4_KEY:
			result /= 16.0f;
			break;
		case DATATYPE_FPE2_KEY:
			result /= 4.0f;
			break;

		case DATATYPE_SP1E_KEY:
			result /= 16384.0f;
			break;
		case DATATYPE_SP3C_KEY:
			result /= 4096.0f;
			break;
		case DATATYPE_SP4B_KEY:
			result /= 2048.0f;
			break;
		case DATATYPE_SP5A_KEY:
			result /= 1024.0f;
			break;
		case DATATYPE_SP69_KEY:
			result /= 512.0f;
			break;
		case DATATYPE_SP78_KEY:
			result /= 256.0f;
			break;
		case DATATYPE_SP87_KEY:
			result /= 128.0f;
			break;
		case DATATYPE_SP96_KEY:
			result /= 64.0f;
			break;
		case DATATYPE_SPB4_KEY:
			result /= 16.0f;
			break;
		case DATATYPE_SPF0_KEY:
			// is / 1.0;
			break;

		case DATATYPE_PWM_KEY:
			result /= 655.36f;
			break;

		default:
			// Oops!
			result = NAN;
	}
	return result;
}

// See header for documentation
double ToSMCNumber(uint32_t dataType, const SMCBytes_t buf, uint8_t bufLen) {
	switch (dataType) {
		case DATATYPE_HEX_KEY:
			// Hex keys vary in length, but 1,2,4,8 are just numbers.
			switch (bufLen) {
				case 1:
					return *((uint8_t*) buf);
				case 2:
					return ntohs(*((uint16_t*) buf));
				case 4:
					return ntohl(*((uint32_t*) buf));
				default:
					return NAN;
			}
		case DATATYPE_UINT8_KEY:
		case DATATYPE_FLAG_KEY:
			if (bufLen != 1)
				return NAN;
			return *((uint8_t*) buf);
		case DATATYPE_SI8_KEY:
			if (bufLen != 1)
				return NAN;
			return *((int8_t*) buf);
		case DATATYPE_UINT16_KEY:
			if (bufLen != 2)
				return NAN;
			return ntohs(*((uint16_t*) buf));
		case DATATYPE_SI16_KEY:
			if (bufLen != 2)
				return NAN;
			return ntohs(*((int16_t*) buf));
		case DATATYPE_UINT32_KEY:
			if (bufLen != 4)
				return NAN;
			return ntohl(*((uint32_t*) buf));
		default:
			return ToSMCFloat(dataType, ntohs(*((uint16_t*) buf)));
	}
}

// See header for documentation
IOReturn AppleSMCReadNumber(io_connect_t conn, const char* key, double* value) {
	SMCBytes_t buf;
	uint8_t bufLen;
	uint32_t dataType;
	// Read the key into a buffer
	IOReturn result = AppleSMCReadBuffer(conn, key, &dataType, buf, &bufLen);
	if (result != kIOReturnSuccess)
		return result;
	if (value != NULL)
		*value = ToSMCNumber(dataType, buf, bufLen);
	return kIOReturnSuccess;
}

using namespace v8;

void Get(const FunctionCallbackInfo<Value>& args) {
    Isolate* isolate = Isolate::GetCurrent();
    Local<Context> context = isolate->GetCurrentContext();
    HandleScope scope(isolate);
    io_connect_t conn;

    if (args.Length() < 1) {
        args.GetReturnValue().Set(Undefined(isolate));
        return;
    }
    if (!args[0]->IsString()) {
        isolate->ThrowException(Exception::TypeError(
          String::NewFromUtf8(isolate, "Expected string", NewStringType::kNormal)
          .ToLocalChecked())
        );
        return;
    }
    v8::String::Utf8Value k(isolate, args[0]->ToString(context).ToLocalChecked());
    char *key = *k;

    IOReturn ioResult = AppleSMCOpen(&conn);
    if (ioResult != kIOReturnSuccess) {
        isolate->ThrowException(Exception::Error(
          String::NewFromUtf8(isolate, "Unavailable", NewStringType::kNormal)
          .ToLocalChecked())
        );
        return;
    }

    double value;
    ioResult = AppleSMCReadNumber(conn, key, &value);
    if (ioResult != kIOReturnSuccess)
      value = 0.0;
    AppleSMCClose(conn);

    args.GetReturnValue().Set(Number::New(isolate, value));
}

void Init(v8::Local<Object> exports) {
    NODE_SET_METHOD(exports, "get", Get);
}

NODE_MODULE(smc, Init)
