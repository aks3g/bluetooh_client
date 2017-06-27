/*
 * Copyright 2016 Kiyotaka Akasaka
 *
 * Released under the MIT license
 * http://opensource.org/licenses/mit-license.php
 */
#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>


#ifndef BT_ATT_H_
#define BT_ATT_H_

// 3.4 ATTRIBUTE PROTOCOL PDUS
class BtAttPduOpcode{
public:
	static const uint8_t cAttOpcodeErrorResponse			= 0x01;
	static const uint8_t cAttOpcodeExchangeMtuRequest		= 0x02;
	static const uint8_t cAttOpcodeExchangeMtuResponse		= 0x03;
	static const uint8_t cAttOpcodeFindInformationRequest	= 0x04;
	static const uint8_t cAttOpcodeFindInformationResponse	= 0x05;
	static const uint8_t cAttOpcodeFindByTypeValueRequest	= 0x06;
	static const uint8_t cAttOpcodeFindByTypeValueResponse	= 0x07;
	static const uint8_t cAttOpcodeReadByTypeRequest		= 0x08;
	static const uint8_t cAttOpcodeReadByTypeResponse		= 0x09;
	static const uint8_t cAttOpcodeReadRequest				= 0x0A;
	static const uint8_t cAttOpcodeReadResponse				= 0x0B;
	static const uint8_t cAttOpcodeReadBlobRequest			= 0x0C;
	static const uint8_t cAttOpcodeReadBlobResponse			= 0x0D;
	static const uint8_t cAttOpcodeReadMultipleRequest		= 0x0E;
	static const uint8_t cAttOpcodeReadMultipleResponse		= 0x0F;
	static const uint8_t cAttOpcodeReadByGroupTypeRequest	= 0x10;
	static const uint8_t cAttOpcodeReadByGroupTypeResponse	= 0x11;
	static const uint8_t cAttOpcodeWriteRequest				= 0x12;
	static const uint8_t cAttOpcodeWriteResponse			= 0x13;
	static const uint8_t cAttOpcodeWriteCommand				= 0x52;
	static const uint8_t cAttOpcodePrepareWriteRequest		= 0x16;
	static const uint8_t cAttOpcodePrepareWriteResponse		= 0x17;
	static const uint8_t cAttOpcodeExecuteWriteRequest		= 0x18;
	static const uint8_t cAttOpcodeExecuteWriteResponse		= 0x19;
	static const uint8_t cAttOpcodeHandleValueNotification	= 0x1B;
	static const uint8_t cAttOpcodeHandleValueIndication	= 0x1D;
	static const uint8_t cAttOpcodeHandleValueConfirmation	= 0x1E;
	static const uint8_t cAttOpcodeSignedWriteCommand		= 0xD2;
};

// 3.4.1.1 Error Response
class BtAttErrorCode {
public:
	static const uint8_t cAttErrorCodeInvalidHandle			= 0x01;
	static const uint8_t cAttErrorCodeReadNotPermitted		= 0x02;
	static const uint8_t cAttErrorCodeWriteNotPermitted		= 0x03;
	static const uint8_t cAttErrorCodeInvalidPdu			= 0x04;
	static const uint8_t cAttErrorCodeInsufficientAuthentication	= 0x05;
	static const uint8_t cAttErrorCodeRequestNotSupported	= 0x06;
	static const uint8_t cAttErrorCodeInvalidOffset			= 0x07;
	static const uint8_t cAttErrorCodeInsufficientAuthorization		= 0x08;
	static const uint8_t cAttErrorCodePrepareQueueFull		= 0x09;
	static const uint8_t cAttErrorCodeAttributeNotFound		= 0x0A;
	static const uint8_t cAttErrorCodeAttributeNotLong		= 0x0B;
	static const uint8_t cAttErrorCodeInsufficientEncryptionKeySize	= 0x0C;
	static const uint8_t cAttErrorCodeINvalidAttributeValueLength	= 0x0D;
	static const uint8_t cAttErrorCodeUnlikelyError			= 0x0E;
	static const uint8_t cAttErrorCodeInsufficientEncryption		= 0x0F;
	static const uint8_t cAttErrorCodeUnsupportedGroupType	= 0x10;
	static const uint8_t cAttErrorCodeInsufficientResources	= 0x11;
	static const uint8_t cAttErrorCodeApplicationErrorBase	= 0x80;
	static const uint8_t cAttErrorCodeApplicationErrorMax	= 0x9F;
	static const uint8_t cAttErrorCodeCommonProfileAndServiceErrorCodesBase = 0xE0;
	static const uint8_t cAttErrorCodeCommonProfileAndServiceErrorCodesMax  = 0xFF;
};

// 3.3.1.1 Characteristic Properties
struct BtAttCharacteristicProperties
{
	static const uint8_t cBroadcast							= 0x01;
	static const uint8_t cRead								= 0x02;
	static const uint8_t cWriteWithoutResponse				= 0x04;
	static const uint8_t cWrite								= 0x08;
	static const uint8_t cNotify							= 0x10;
	static const uint8_t cIndicate							= 0x20;
	static const uint8_t cAuthenticatedSignedWrites			= 0x40;
	static const uint8_t cExtendedProperies					= 0x80;
};

// 3.3.3.3 Client Characteristic Configuration
struct BtAttClientCharacteristicConfiguration
{
	static const uint16_t cNotification						= 0x0001;
	static const uint16_t cIndication						= 0x0002;
};


// 3.4.6.3 Execute Write Request
struct BtAttExecuteWriteFlag {
	static const uint8_t cCancelAllPreparedWrites = 0;
	static const uint8_t cImmediatelyWriteAllPendingPreparedValues = 1;
};




//J Signed Write 系
#define BT_ATT_SIGNATURE_SIZE							(12)

//J MTU のレンジ
#define BT_ATT_MIN_LE_MTU								(23)
#define BT_ATT_MAX_LE_MTU								(512)


#define BT_ATT_L2CAP_CID								(0x0004)	// 5.2.2 LE Channel Requirements
#define BT_ATT_L2CAP_PSM								(0x001F)	// 5.1 BR/EDR L2CAP INTEROPERABILITY REQUIREMENTS



#pragma pack(1)
typedef uint16_t	BtAttHandle;
typedef uint16_t 	BtAttUuid16;
typedef uint128_t	BtAttUuid128;

struct BtAttHandleRange
{
	BtAttHandle start;
	BtAttHandle end;
};

struct BtAttHandleUuid16Pair
{
	BtAttHandle handle;
	BtAttUuid16 uuid;
};

struct BtAttHandleUuid128Pair
{
	BtAttHandle  handle;
	BtAttUuid128 uuid;
};

struct BtAttHandleRangeUuid16Pair
{
	BtAttHandleRange handles;
	BtAttUuid16 uuid;
};

struct BtUuid
{
	enum {
		cBtUuidUnknown = 0,
		cBtUuid16		= 1,
		cBtUuid128		= 2,
	} format;
	union {
		BtAttUuid16  uuid16;
		BtAttUuid128 uuid128;
	} value;
};

struct BtAttHandleUuidPair
{
	BtAttHandle handle;
	BtUuid      uuid;
};

union BtAttPdu
{
	uint8_t raw[1024];
	struct Pdu{
		uint8_t opcode;
		union Args{
			struct ErrorResponse{
				uint8_t  request_opcode;
				BtAttHandle handle;
				uint8_t  status;
			} errorResponse;
			struct ExchangeMtuRequest{
				uint16_t mtu;
			} exchangeMtuRequest;
			struct ExchangeMtuResponse{
				uint16_t mtu;
			} exchangeMtuResponse;
			struct FindInformationRequest{
				BtAttHandleRange range;
			} findInformationRequest;
			struct FindInformationResponse{
				uint8_t format;
				uint8_t list_buf[0];
			} findInformationResponse;
			struct FindByTypeValueRequest{
				BtAttHandleRange range;
				BtAttUuid16 uuid;
				uint8_t  value[0];
			} findByTypeValueRequest;
			struct FindByTypeValueResponse{
				BtAttHandleRange range[0];
			} findByTypeValueResponse;
			struct ReadByTypeRequest{
				BtAttHandleRange range;
				uint8_t uuid[0];			//J 16bit / 128bit
			} readByTypeRequest;
			struct ReadByTypeResponse{
				uint8_t len;
				uint8_t list[0];
			} readByTypeResponse;
			struct ReadRequest{
				BtAttHandle handle;
			} readRequest;
			struct ReadResponse{
				uint8_t value[0];
			} readResponse;
			struct ReadBlobRequest{
				BtAttHandle handle;
				uint16_t offset;
			} readBlobRequest;
			struct ReadBlobResponse{
				uint8_t value[0];
			} readBlobResponse;
			struct ReadMultipleRequest{
				BtAttHandle handles[0];
			} readMultipleRequest;
			struct ReadMultipleResponse{
				uint8_t values[0];
			} readMultipleResponse;
			struct ReadByGroupTypeRequest{
				BtAttHandleRange range;
				uint8_t uuid[0];
			} readByGroupTypeRequest;
			struct ReadByGroupTypeResponse{
				uint8_t len;
				uint8_t list[0];
			} readByGroupTypeResponse;
			struct WriteRequest{
				BtAttHandle handle;
				uint8_t  value[0];
			} writeRequest;
			struct WriteResponse{
				;
			} writeResponse;
			struct WriteCommand{
				BtAttHandle handle;
				uint8_t  value[0];
			} writeCommand;
			struct PrepareWriteRequest{
				BtAttHandle handle;
				uint16_t offset;
				uint8_t  value[0];
			} prepareWriteRequest;
			struct PrepareWriteResponse{
				BtAttHandle handle;
				uint16_t offset;
				uint8_t  value[0];
			} prepareWriteResponse;
			struct ExecuteWriteRequest{
				uint8_t flags;
			} executeWriteRequest;
			struct ExecuteWriteResponse{
				;
			} executeWriteResponse;
			struct HandleValueNotification{
				BtAttHandle handle;
				uint8_t  value[0];
			} handleValueNotification;
			struct HandleValueIndication{
				BtAttHandle handle;
				uint8_t  value[0];
			} handleValueIndication;
			struct HandleValueConfirmation{
				;
			} handleValueConfirmation;
			struct SignedWriteCommand{
				BtAttHandle handle;
				uint8_t  value[0];
			} signedWriteCommand;
		} args;
	} pdu;
};


struct BtAttAttributeData
{
	uint16_t attributeHandle;
	uint16_t endGroupHandle;
	uint8_t  attributeData[0];
};



struct BtAttAttributeDataReadByTypeResponse
{
	uint16_t handle;
	union AttributeValue {
		uint8_t  data[0];
		// 3.2 INCLUDE DEFINITION
		struct {
			uint16_t includedServiceAttributeHandle;
			uint16_t endGroupHandle;
			uint16_t uuid16;
		} include;

		//J 3.3.1 Characteristic Declaration
		struct {
			uint8_t properties;
			uint16_t valueHandle;
			union {
				BtAttUuid16  uuid16;
				BtAttUuid128 uuid128;
			} uuid;
		} characteristic;
	} attributeValue;
};

#pragma pack()

/*
 * Attribute関係のUtils
 */
BtAttAttributeData *btAttNextAttributeData(const void *ptr, size_t item_len);
int btAttReadAttributeData16(BtAttAttributeData *attrData, uint16_t &value);

BtAttAttributeDataReadByTypeResponse *btAttNextAttributeDataReadByResponse(const void *ptr, size_t item_len);

/*
 *J ATT PDU の組み立て系
 */
int btAttBuildPduErrorResponse(
								uint8_t *pdu, const size_t len, 
								const uint8_t opcode,
								const BtAttHandle handle,
								const uint8_t status);
int btAttBuildPduExchangeMtuRequest(
								uint8_t *pdu, const size_t len,
								const uint16_t mtu);
int btAttBuildPduExchangeMtuResponse(
								uint8_t *pdu, const size_t len,
								const uint16_t mtu);
int btAttBuildPduFindInformationRequest(
								uint8_t *pdu, const size_t len,
								const BtAttHandleRange range);
int btAttBuildPduFindInformationResponse(
								uint8_t *pdu, const size_t len,
								const uint8_t format,
								const uint16_t num,
								const void *handle_uuid_pair);
int btAttBuildPduFindByTypeValueRequest(
								uint8_t *pdu, const size_t len,
								const BtAttHandleRange range,
								const uint16_t uuid16,
								const uint8_t *value,
								const uint16_t value_len);
int btAttBuildPduFindByTypeValueResponse(
								uint8_t *pdu, const size_t len,
								const BtAttHandleRange *range,
								const uint16_t range_cnt);
int btAttBuildPduReadByTypeRequest(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandleRange range,
								const BtUuid uuid);
int btAttBuildPduReadByTypeResponse(
								uint8_t *pdu,
								const size_t len,
								const uint8_t num_item,
								const uint8_t item_len,
								const uint8_t *list_data);
int btAttBuildPduReadRequest(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle);
int btAttBuildPduReadResponse(
								uint8_t *pdu,
								const size_t len,
								const uint8_t *value,
								const uint16_t value_len);
int btAttBuildPduReadBlobRequest(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t offset);
int btAttBuildPduReadBlobResponse(
								uint8_t *pdu,
								const size_t len,
								const uint8_t *value,
								const uint16_t value_len);
int btAttBuildPduReadMultipleRequest(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle *handles,
								const size_t num_handles);
int btAttBuildPduReadMultipleResponse(
								uint8_t *pdu,
								const size_t len,
								const void *values,
								const size_t values_len);
int btAttBuildPduReadByGroupTypeRequest(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandleRange range,
								const BtUuid uuid);
int btAttBuildPduReadByGroupTypeResponse(
								uint8_t *pdu,
								const size_t len,
								const uint16_t num_items,
								const uint16_t item_len,
								const uint8_t *item_list);
int btAttBuildPduWriteRequest(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t value_len,
								const uint8_t *value);
int btAttBuildPduWriteResponse(
								uint8_t *pdu,
								const size_t len);
int btAttBuildPduWriteCommand(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t value_len,
								const uint8_t *value);
int btAttBuildPduPrepareWriteRequest(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t offset,
								const uint16_t value_len,
								const uint8_t *value);
int btAttBuildPduPrepareWriteResponse(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t offset,
								const uint16_t value_len,
								const uint8_t *value);
int btAttBuildPduExecuteWriteRequest(
								uint8_t *pdu,
								const size_t len,
								const uint8_t flags);
int btAttBuildPduExecuteWriteResponse(
								uint8_t *pdu,
								const size_t len);
int btAttBuildPduHandleValueNotification(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t value_len,
								const uint8_t *value);
int btAttBuildPduHandleValueIndication(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t value_len,
								const uint8_t *value);
int btAttBuildPduHandleValueConfirmation(
								uint8_t *pdu,
								const size_t len);
int btAttBuildPduSignedWriteCommand(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t value_len,
								const uint8_t *value,
								const struct bt_crypto *crypto,
								const uint8_t csrk[16],
								const uint32_t sign_count);

/*
 *J ATT PDU の解析系
 */
int btAttParsePduErrorResponse(
								const uint8_t *pdu,
								const size_t len,
								uint8_t &opcode,
								uint16_t &handle,
								uint8_t &status);
int btAttParsePduExchangeMtuRequest(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &mtu);
int btAttParsePduExchangeMtuResponse(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &mtu);
int btAttParsePduFindInformationRequest(
								const uint8_t *pdu,
								const size_t len,
								BtAttHandleRange &range);
int btAttParsePduFindInformationResponse(
								const uint8_t *pdu,
								const size_t len,
								uint8_t &format,
								uint16_t &num,
								void *handle_uuid_pair,
								const uint16_t buf_size);
int btAttParsePduFindByTypeValueRequest(
								const uint8_t *pdu,
								const size_t len,
								BtAttHandleRange &range,
								uint16_t &uuid16,
								void *value,
								const uint16_t value_buf_size,
								uint16_t &value_len);
int btAttParsePduFindByTypeValueResponse(
								const uint8_t *pdu,
								const size_t len,
								BtAttHandleRange *range,
								const uint16_t range_buf_size,
								uint16_t &range_cnt);
int btAttParsePduReadByTypeRequest(
								const uint8_t *pdu,
								const size_t len,
								BtAttHandleRange &range,
								BtUuid &uuid);
int btAttParsePduReadByTypeResponse(
								const uint8_t *pdu,
								const size_t len,
								uint8_t &item_len,
								uint8_t &item_cnt,
								void *list_data,
								const uint16_t list_buf_size);
int btAttParsePduReadRequest(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle);
int btAttParsePduReadResponse(
								const uint8_t *pdu,
								const size_t len,
								void *value,
								const uint16_t value_buf_size,
								uint16_t &value_len);
int btAttParsePduReadBlobReques(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle,
								uint16_t &offset);
int btAttParsePduReadBlobResponse(
								const uint8_t *pdu,
								const size_t len,
								void *value,
								const uint16_t value_buf_size,
								uint16_t &value_len);
int btAttParsePduReadMultipleRequest(
								uint8_t *pdu,
								const size_t len,
								BtAttHandle *handles,
								const size_t handles_size,
								size_t *num_handles);
int btAttParsePduReadMultipleResponse(
								uint8_t *pdu,
								const size_t len,
								void *values,
								const size_t values_buf_size,
								size_t *values_len);
int btAttParsePduReadByGroupTypeRequest(
								const uint8_t *pdu,
								const size_t len,
								BtAttHandleRange &range,
								BtUuid &uuid);
int btAttParsePduReadByGroupTypeResponse(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &item_len,
								void *item_list,
								const uint16_t item_list_buf_size,
								uint16_t &item_cnt);
int btAttParsePduWriteRequest(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle,
								uint16_t &value_len,
								void *value,
								const uint16_t value_buf_size);
int btAttParsePduWriteResponse(
								const uint8_t *pdu,
								const size_t len);
int btAttParsePduWriteCommand(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle,
								uint16_t &value_len,
								void *value,
								const uint16_t value_buf_size);
int btAttParsePduPrepareWriteRequest(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle,
								uint16_t &offset,
								uint16_t &value_len,
								void *value,
								const uint16_t value_buf_size);
int btAttParsePduPrepareWriteResponse(
								const uint8_t *pdu,
								const size_t len,
								BtAttHandle &handle,
								uint16_t &offset,
								uint16_t &value_len,
								void *value,
								const uint16_t value_buf_size);
int btAttParsePduExecuteWriteRequest(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &flags);
int btAttParsePduExecuteWriteResponse(
								const uint8_t *pdu,
								const size_t len);
int btAttParsePduHandleValueNotification(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle,
								uint16_t &value_len,
								void *value,
								const uint16_t value_buf_size);
int btAttParsePduHandleValueIndication(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle,
								uint16_t &value_len,
								void *value,
								const uint16_t value_buf_size);
int btAttParsePduHandleValueConfirmation(
							const uint8_t *pdu,
							const size_t len);
int btAttParsePduSignedWriteCommand(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle,
								uint16_t &value_len,
								uint8_t *value,
								const uint16_t value_buf_size);

#endif/*BT_ATT_H_*/
