/*
 * Copyright 2016 Kiyotaka Akasaka
 *
 * Released under the MIT license
 * http://opensource.org/licenses/mit-license.php
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#include <error.h>
#include <errno.h>


#include "aks_error.h"
#include "bt_att.h"
#include "bt_util.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*
 * Attribute関係のUtils
 */
BtAttAttributeData *btAttNextAttributeData(const void *ptr, size_t item_len)
{
	if (ptr == NULL) {
		return NULL;
	}

	return (BtAttAttributeData *)((uint64_t)ptr + item_len);
}

/*---------------------------------------------------------------------------*/
int btAttReadAttributeData16(BtAttAttributeData *attrData, uint16_t &value)
{
	if (attrData == NULL) {
		return AKS_ERROR_NULL;
	}

	value = ((uint16_t)(attrData->attributeData[1]) << 8) |
			((uint16_t)(attrData->attributeData[0]) << 0);

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
BtAttAttributeDataReadByTypeResponse *btAttNextAttributeDataReadByResponse(const void *ptr, size_t item_len)
{
	if (ptr == NULL) {
		return NULL;
	}

	return (BtAttAttributeDataReadByTypeResponse *)((uint64_t)ptr + item_len);
}



/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int btAttBuildPduErrorResponse(
								uint8_t *pdu, const size_t len, 
								const uint8_t opcode,
								const BtAttHandle handle,
								const uint8_t status)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ErrorResponse);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeErrorResponse;
	_pdu->pdu.args.errorResponse.request_opcode = opcode;
	_pdu->pdu.args.errorResponse.handle = handle;
	_pdu->pdu.args.errorResponse.status = status;

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduExchangeMtuRequest(
								uint8_t *pdu, const size_t len,
								const uint16_t mtu)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ExchangeMtuRequest);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeExchangeMtuRequest;
	_pdu->pdu.args.exchangeMtuRequest.mtu = mtu;

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduExchangeMtuResponse(
								uint8_t *pdu, const size_t len,
								const uint16_t mtu)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ExchangeMtuResponse);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeExchangeMtuResponse;
	_pdu->pdu.args.exchangeMtuResponse.mtu = mtu;

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduFindInformationRequest(
								uint8_t *pdu, const size_t len,
								const BtAttHandleRange range)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::FindInformationRequest);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeFindInformationRequest;
	_pdu->pdu.args.findInformationRequest.range = range;

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduFindInformationResponse(
								uint8_t *pdu, const size_t len,
								const uint8_t format,
								const uint16_t num,
								const void *handle_uuid_pair)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((handle_uuid_pair == NULL) && (num != 0)) {
		return AKS_ERROR_NULL;
	}

	int item_size = 0;
	if(format == 0x01) {
		item_size = sizeof(BtAttHandleUuid16Pair);
	}
	else if (format == 0x02) {
		item_size = sizeof(BtAttHandleUuid128Pair);
	}
	else {
		return AKS_ERROR_BT_INVALUD_FORMAT;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::FindInformationResponse)
				 + (num * item_size);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeFindInformationResponse;
	_pdu->pdu.args.findInformationResponse.format = format;

	uint8_t *list_ptr = _pdu->pdu.args.findInformationResponse.list_buf;
	memcpy(list_ptr, handle_uuid_pair, item_size * num);

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduFindByTypeValueRequest(
								uint8_t *pdu, const size_t len,
								const BtAttHandleRange range,
								const uint16_t uuid16,
								const uint8_t *value,
								const uint16_t value_len)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((value == NULL) && (value_len != 0)) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::FindByTypeValueRequest)
				 + (value_len);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeFindByTypeValueRequest;
	_pdu->pdu.args.findByTypeValueRequest.range = range;
	_pdu->pdu.args.findByTypeValueRequest.uuid = uuid16;
	if (value_len) {
		memcpy(_pdu->pdu.args.findByTypeValueRequest.value, value, value_len);
	}

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduFindByTypeValueResponse(
								uint8_t *pdu, const size_t len,
								const BtAttHandleRange *range,
								const uint16_t range_cnt)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((range == NULL) && (range_cnt != 0)) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::FindByTypeValueResponse)
				 + (sizeof(BtAttHandleRange) * range_cnt);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeFindByTypeValueResponse;
	for (int i=0 ; i<range_cnt ; ++i) {
		_pdu->pdu.args.findByTypeValueResponse.range[i] = range[i];
	}

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduReadByTypeRequest(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandleRange range,
								const BtUuid uuid)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	int uuid_len = 0;
	if (uuid.format == BtUuid::cBtUuid16) {
		uuid_len = 2;
	} else if (uuid.format == BtUuid::cBtUuid128) {
		uuid_len = 16;
	} else {
		return AKS_ERROR_BT_INVALID_UUID;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::ReadByTypeRequest)
				 + uuid_len;
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeReadByTypeRequest;
	_pdu->pdu.args.readByTypeRequest.range = range;

	//J TODO 本当に動くか確認が必要
	memcpy(_pdu->pdu.args.readByTypeRequest.uuid, &uuid.value.uuid16, uuid_len);

	return pdu_size;
}

/*---------------------------------------------------------------------------*/
int btAttBuildPduReadByTypeResponse(
								uint8_t *pdu,
								const size_t len,
								const uint8_t num_item,
								const uint8_t item_len,
								const uint8_t *list_data)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((list_data == NULL) && (item_len != 0)) {
		return AKS_ERROR_NULL;
	}


	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::ReadByTypeResponse)
				 + item_len * num_item;
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeReadByTypeResponse;
	_pdu->pdu.args.readByTypeResponse.len = item_len;

	uint8_t *ptr = _pdu->pdu.args.readByTypeResponse.list;
	const uint8_t *list = list_data;
	for (uint8_t i=0 ; i<num_item ; ++i) {
		memcpy(ptr, list, item_len);
		
		ptr  += item_len;
		list += item_len;
	}

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduReadRequest(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::ReadRequest);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeReadRequest;
	_pdu->pdu.args.readRequest.handle = handle;

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduReadResponse(
								uint8_t *pdu,
								const size_t len,
								const uint8_t *value,
								const uint16_t value_len)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((value == NULL) || (value_len == 0)){
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::ReadResponse)
				 + value_len;
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeReadResponse;
	memcpy(_pdu->pdu.args.readResponse.value, value, value_len);

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduReadBlobRequest(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t offset)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::ReadBlobRequest);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeReadBlobRequest;
	_pdu->pdu.args.readBlobRequest.handle = handle;
	_pdu->pdu.args.readBlobRequest.offset = offset;

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduReadBlobResponse(
								uint8_t *pdu,
								const size_t len,
								const uint8_t *value,
								const uint16_t value_len)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((value == NULL) || (value_len == 0)){
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::ReadBlobResponse)
				 + value_len;
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeReadBlobResponse;
	memcpy(_pdu->pdu.args.readResponse.value, value, value_len);

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduReadMultipleRequest(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle *handles,
								const size_t num_handles)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((handles == NULL) || (num_handles < 2)){
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::ReadMultipleRequest)
				 + sizeof(BtAttHandle) * num_handles;
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeReadMultipleRequest;
	memcpy(_pdu->pdu.args.readMultipleRequest.handles, handles, sizeof(BtAttHandle)*num_handles);

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduReadMultipleResponse(
								uint8_t *pdu,
								const size_t len,
								const void *values,
								const size_t values_len)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((values == NULL) || (values_len == 0)){
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::ReadMultipleResponse)
				 + values_len;
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeReadMultipleResponse;
	memcpy(_pdu->pdu.args.readMultipleResponse.values, values, values_len);

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduReadByGroupTypeRequest(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandleRange range,
								const BtUuid uuid)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	int uuid_len = 0;
	if (uuid.format == BtUuid::cBtUuid16) {
		uuid_len = 2;
	} else if (uuid.format == BtUuid::cBtUuid128) {
		uuid_len = 16;
	} else {
		return AKS_ERROR_BT_INVALID_UUID;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::ReadByGroupTypeRequest)
				 + uuid_len;
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeReadByGroupTypeRequest;
	_pdu->pdu.args.readByGroupTypeRequest.range = range;
	//J TODO 本当に動くか確認が必要
	memcpy(_pdu->pdu.args.readByGroupTypeRequest.uuid, &uuid.value.uuid16, uuid_len);

	return pdu_size;
}

/*---------------------------------------------------------------------------*/
int btAttBuildPduReadByGroupTypeResponse(
								uint8_t *pdu,
								const size_t len,
								const uint16_t num_items,
								const uint16_t item_len,
								const uint8_t *item_list)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((item_list == NULL) && ((num_items * item_len) != 0)){
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::ReadByGroupTypeResponse)
				 + (num_items * item_len);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeReadByGroupTypeResponse;
	_pdu->pdu.args.readByGroupTypeResponse.len = item_len;

	if (num_items * item_len) {
		memcpy(_pdu->pdu.args.readByGroupTypeResponse.list, item_list, num_items * item_len);
	}

	return pdu_size;
}

/*---------------------------------------------------------------------------*/
int btAttBuildPduWriteRequest(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t value_len,
								const uint8_t *value)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((value == NULL) && (value_len != 0)){
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::WriteRequest)
				 + value_len;
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeWriteRequest;
	_pdu->pdu.args.writeRequest.handle = handle;
	if (value_len != 0) {
		memcpy(_pdu->pdu.args.writeRequest.value, value, value_len);
	}

	return pdu_size;
}

/*---------------------------------------------------------------------------*/
int btAttBuildPduWriteResponse(
								uint8_t *pdu,
								const size_t len)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::WriteResponse);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeWriteResponse;

	return pdu_size;
}

/*---------------------------------------------------------------------------*/
int btAttBuildPduWriteCommand(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t value_len,
								const uint8_t *value)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((value == NULL) && (value_len != 0)){
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::WriteCommand)
				 + value_len;
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeWriteCommand;
	_pdu->pdu.args.writeCommand.handle = handle;
	if (value_len != 0) {
		memcpy(_pdu->pdu.args.writeCommand.value, value, value_len);
	}

	return pdu_size;
}

/*---------------------------------------------------------------------------*/
int btAttBuildPduPrepareWriteRequest(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t offset,
								const uint16_t value_len,
								const uint8_t *value)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((value == NULL) && (value_len != 0)){
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::PrepareWriteRequest)
				 + value_len;
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodePrepareWriteRequest;
	_pdu->pdu.args.prepareWriteRequest.handle = handle;
	_pdu->pdu.args.prepareWriteRequest.offset = offset;
	if (value_len != 0) {
		memcpy(_pdu->pdu.args.prepareWriteRequest.value, value, value_len);
	}

	return pdu_size;
}

/*---------------------------------------------------------------------------*/
int btAttBuildPduPrepareWriteResponse(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t offset,
								const uint16_t value_len,
								const uint8_t *value)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((value == NULL) && (value_len != 0)){
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::PrepareWriteResponse)
				 + value_len;
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodePrepareWriteResponse;
	_pdu->pdu.args.prepareWriteResponse.handle = handle;
	_pdu->pdu.args.prepareWriteResponse.offset = offset;
	if (value_len != 0) {
		memcpy(_pdu->pdu.args.prepareWriteResponse.value, value, value_len);
	}

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduExecuteWriteRequest(
								uint8_t *pdu,
								const size_t len,
								const uint8_t flags)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::ExecuteWriteRequest);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeExecuteWriteRequest;
	_pdu->pdu.args.executeWriteRequest.flags = flags;

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduExecuteWriteResponse(
								uint8_t *pdu,
								const size_t len)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::ExecuteWriteResponse);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeExecuteWriteResponse;

	return pdu_size;
}


/*---------------------------------------------------------------------------*/
int btAttBuildPduHandleValueNotification(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t value_len,
								const uint8_t *value)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((value == NULL) && (value_len != 0)){
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::HandleValueNotification)
				 + value_len;
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeHandleValueNotification;
	_pdu->pdu.args.handleValueNotification.handle = handle;
	if (value_len != 0) {
		memcpy(_pdu->pdu.args.handleValueNotification.value, value, value_len);
	}

	return pdu_size;
}

/*---------------------------------------------------------------------------*/
int btAttBuildPduHandleValueIndication(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t value_len,
								const uint8_t *value)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((value == NULL) && (value_len != 0)){
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::HandleValueIndication)
				 + value_len;
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeHandleValueIndication;
	_pdu->pdu.args.handleValueIndication.handle = handle;
	if (value_len != 0) {
		memcpy(_pdu->pdu.args.handleValueIndication.value, value, value_len);
	}

	return pdu_size;
}

/*---------------------------------------------------------------------------*/
int btAttBuildPduHandleValueConfirmation(
								uint8_t *pdu,
								const size_t len)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::HandleValueConfirmation);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeHandleValueConfirmation;

	return pdu_size;
}

/*---------------------------------------------------------------------------*/
int btAttBuildPduSignedWriteCommand(
								uint8_t *pdu,
								const size_t len,
								const BtAttHandle handle,
								const uint16_t value_len,
								const uint8_t *value,
								struct bt_crypto *crypto,
								uint8_t csrk[16],
								const uint32_t sign_count)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if (crypto == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((value == NULL) && (value_len != 0)){
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::SignedWriteCommand)
				 + value_len
				 + BT_ATT_SIGNATURE_SIZE;
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	_pdu->pdu.opcode = BtAttPduOpcode::cAttOpcodeSignedWriteCommand;
	_pdu->pdu.args.signedWriteCommand.handle = handle;
	if (value_len != 0) {
		memcpy(_pdu->pdu.args.signedWriteCommand.value, value, value_len);
	}

	//J 真面目に作らないとダメな感じ
	uint8_t signature[BT_ATT_SIGNATURE_SIZE];
	pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				+ sizeof(BtAttPdu::Pdu::Args::SignedWriteCommand)
				+ value_len;
	int ret = btUtilCryptoSignAtt(crypto, csrk, pdu, pdu_size, sign_count, signature);
	if (ret != AKS_OK) {
		return ret;
	}

	memcpy(&(_pdu->pdu.args.signedWriteCommand.value[value_len]), signature, sizeof(signature));

	return pdu_size;
}



/*
 *J ATT PDU の解析系
 */
/*---------------------------------------------------------------------------*/
int btAttParsePduErrorResponse(
								const uint8_t *pdu,
								const size_t len,
								uint8_t &opcode,
								uint16_t &handle,
								uint8_t &status)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ErrorResponse);
	if (len != pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeErrorResponse) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	opcode = _pdu->pdu.args.errorResponse.request_opcode;
	handle = _pdu->pdu.args.errorResponse.handle;
	status = _pdu->pdu.args.errorResponse.status;

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduExchangeMtuRequest(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &mtu)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ExchangeMtuRequest);
	if (len != pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeExchangeMtuRequest) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	mtu = _pdu->pdu.args.exchangeMtuRequest.mtu;

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int btAttParsePduExchangeMtuResponse(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &mtu)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ExchangeMtuResponse);
	if (len != pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeExchangeMtuResponse) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	mtu = _pdu->pdu.args.exchangeMtuResponse.mtu;

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int btAttParsePduFindInformationRequest(
								const uint8_t *pdu,
								const size_t len,
								BtAttHandleRange &range)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::FindInformationRequest);
	if (len != pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeFindInformationRequest) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	range = _pdu->pdu.args.findInformationRequest.range;

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduFindInformationResponse(
								const uint8_t *pdu,
								const size_t len,
								uint8_t &format,
								uint16_t &num,
								void *handle_uuid_pair,
								const uint16_t buf_size)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::FindInformationResponse);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeFindInformationResponse) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	//J フォーマットに応じたリストの型を選ぶ
	format = _pdu->pdu.args.findInformationResponse.format;
	int item_len = 0;
	if (format == 0x01) {
		item_len = sizeof(BtAttHandleUuid16Pair);
	}
	else if (format == 0x02) {
		item_len = sizeof(BtAttHandleUuid128Pair);
	}
	else {
		return AKS_ERROR_BT_INVALUD_FORMAT;
	}

	int ret = AKS_OK;
	int list_size = (len - pdu_size);
	num = list_size / item_len;
	if ((list_size % item_len) != 0) {
		ret = AKS_ERROR_BT_INCLUDE_FRAGMENTS;
	}
	//J バッファサイズゼロで呼ばれた時には、Itemの数だけを返す
	if (buf_size == 0) {
		return ret;
	}
	else if (handle_uuid_pair == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (list_size > buf_size) {
		return AKS_ERROR_NOBUF;
	}
	memcpy (handle_uuid_pair, _pdu->pdu.args.findInformationResponse.list_buf, num * item_len);

	return ret;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduFindByTypeValueRequest(
								const uint8_t *pdu,
								const size_t len,
								BtAttHandleRange &range,
								uint16_t &uuid16,
								void *value,
								const uint16_t value_buf_size,
								uint16_t &value_len)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::FindByTypeValueRequest);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeFindByTypeValueRequest) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	range  = _pdu->pdu.args.findByTypeValueRequest.range;
	uuid16 = _pdu->pdu.args.findByTypeValueRequest.uuid;
	value_len = len - pdu_size;
	if (value_buf_size == 0) {
		return AKS_OK;
	}
	else if (value == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (value_buf_size < value_len) {
		return AKS_ERROR_NOBUF;
	}

	memcpy(value, _pdu->pdu.args.findByTypeValueRequest.value, value_len);

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduFindByTypeValueResponse(
								const uint8_t *pdu,
								const size_t len,
								BtAttHandleRange *range,
								const uint16_t range_buf_size,
								uint16_t &range_cnt)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::FindByTypeValueResponse);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeFindByTypeValueResponse) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	int ret = AKS_OK;
	if (((len - pdu_size) % sizeof(BtAttHandleRange)) != 0) {
		ret = AKS_ERROR_BT_INCLUDE_FRAGMENTS;
	}

	range_cnt = (len - pdu_size) / sizeof(BtAttHandleRange);
	if (range_buf_size == 0) {
		return ret;
	}
	else if (range == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (range_buf_size < (range_cnt * sizeof(BtAttHandleRange))) {
		return AKS_ERROR_NOBUF;
	}

	memcpy(range, _pdu->pdu.args.findByTypeValueResponse.range, range_cnt * sizeof(BtAttHandleRange));

	return ret;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduReadByTypeRequest(
								const uint8_t *pdu,
								const size_t len,
								BtAttHandleRange &range,
								BtUuid &uuid)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ReadByTypeRequest);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeReadByTypeRequest) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	range = _pdu->pdu.args.readByTypeRequest.range;

	int uuid_size = len - pdu_size;
	if (uuid_size == 2) {
		uuid.format = BtUuid::cBtUuid16;
	}
	else if (uuid_size == 16) {
		uuid.format = BtUuid::cBtUuid128;
	}
	else {
		return AKS_ERROR_BT_INVALID_UUID;
	}

	memcpy(&uuid.value.uuid16, _pdu->pdu.args.readByTypeRequest.uuid, uuid_size);

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduReadByTypeResponse(
								const uint8_t *pdu,
								const size_t len,
								uint8_t &item_len,
								uint8_t &item_cnt,
								void *list_data,
								const uint16_t list_buf_size)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ReadByTypeResponse);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeReadByTypeResponse) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	item_len = _pdu->pdu.args.readByTypeResponse.len;

	int item_list_size = len - pdu_size;
	item_cnt = item_list_size / item_len;

	int ret = 0;
	if ((item_list_size % item_len) != 0) {
		ret = AKS_ERROR_BT_INCLUDE_FRAGMENTS;
	}

	if (list_buf_size == 0) {
		return ret;
	}
	else if (list_data == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (list_buf_size < item_list_size) {
		return AKS_ERROR_NOBUF;
	}

	memcpy(list_data, _pdu->pdu.args.readByTypeResponse.list, item_cnt * item_len);

	return ret;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduReadRequest(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ReadRequest);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeReadRequest) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	handle = _pdu->pdu.args.readRequest.handle;

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduReadResponse(
								const uint8_t *pdu,
								const size_t len,
								void *value,
								const uint16_t value_buf_size,
								uint16_t &value_len)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ReadResponse);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeReadResponse) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	value_len = len - pdu_size;
	if (value_buf_size == 0) {
		return AKS_OK;
	}
	else if (value == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (value_buf_size < value_len) {
		return AKS_ERROR_NOBUF;
	}

	memcpy(value, _pdu->pdu.args.readResponse.value, value_len);

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduReadBlobRequest(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle,
								uint16_t &offset)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ReadBlobRequest);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeReadBlobRequest) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	handle = _pdu->pdu.args.readBlobRequest.handle;
	offset = _pdu->pdu.args.readBlobRequest.offset;

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int btAttParsePduReadBlobResponse(
								const uint8_t *pdu,
								const size_t len,
								void *value,
								const uint16_t value_buf_size,
								uint16_t &value_len)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ReadBlobResponse);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeReadBlobResponse) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	value_len = len - pdu_size;
	if (value_buf_size == 0) {
		return AKS_OK;
	}
	else if (value == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (value_buf_size < value_len) {
		return AKS_ERROR_NOBUF;
	}

	memcpy(value, _pdu->pdu.args.readBlobResponse.value, value_len);

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int btAttParsePduReadMultipleRequest(
								uint8_t *pdu,
								const size_t len,
								BtAttHandle *handles,
								const size_t handles_size,
								size_t *num_handles)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if (num_handles == NULL){
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::ReadMultipleRequest);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeReadMultipleRequest) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	int ret = AKS_OK;
	*num_handles = (len - pdu_size) / sizeof(BtAttHandle);
	if ( ((len - pdu_size) % sizeof(BtAttHandle)) != 0) {
		ret = AKS_ERROR_BT_INCLUDE_FRAGMENTS;
	}
	if (handles == NULL) {
		return ret;
	}
	else if (handles_size < (len - pdu_size)) {
		return AKS_ERROR_NOBUF;
	}

	memcpy(handles, _pdu->pdu.args.readMultipleRequest.handles, *num_handles * sizeof(BtAttHandle));

	return ret;
}


/*---------------------------------------------------------------------------*/
int btAttParsePduReadMultipleResponse(
								uint8_t *pdu,
								const size_t len,
								void *values,
								const size_t values_buf_size,
								size_t *values_len)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if (values_len == NULL){
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode) 
				 + sizeof(BtAttPdu::Pdu::Args::ReadMultipleResponse);
	if (pdu_size > len) {
		return AKS_ERROR_NOBUF;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeReadMultipleResponse) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	*values_len = (len - pdu_size);
	if (values == NULL) {
		return AKS_OK;
	}
	else if (values_buf_size < (len - pdu_size)) {
		return AKS_ERROR_NOBUF;
	}

	memcpy(values, _pdu->pdu.args.readMultipleResponse.values, *values_len);

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int btAttParsePduReadByGroupTypeRequest(
								const uint8_t *pdu,
								const size_t len,
								BtAttHandleRange &range,
								BtUuid &uuid)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ReadByGroupTypeRequest);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeReadByGroupTypeRequest) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	int uuid_size = (len - pdu_size);
	if (uuid_size == 2) {
		uuid.format = BtUuid::cBtUuid16;
	}
	else if (uuid_size == 16) {
		uuid.format = BtUuid::cBtUuid128;
	}
	else {
		return AKS_ERROR_BT_INVALID_UUID;
	}

	range = _pdu->pdu.args.readByGroupTypeRequest.range;
	memcpy(&uuid.value.uuid16, _pdu->pdu.args.readByGroupTypeRequest.uuid, uuid_size);

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int btAttParsePduReadByGroupTypeResponse(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &item_len,
								void *item_list,
								const uint16_t item_list_buf_size,
								uint16_t &item_cnt)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ReadByGroupTypeResponse);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeReadByGroupTypeResponse) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	item_len = _pdu->pdu.args.readByGroupTypeResponse.len;

	int item_list_len = len - pdu_size;

	int ret = 0;
	item_cnt = item_list_len / item_len;
	if ((item_list_len % item_len) != 0) {
		ret = AKS_ERROR_BT_INCLUDE_FRAGMENTS;
	}

	if (item_list_buf_size == 0) {
		return ret;
	}
	else if (item_list == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (item_list_buf_size < item_list_len) {
		return AKS_ERROR_NOBUF;
	}

	memcpy(item_list, _pdu->pdu.args.readByGroupTypeResponse.list, item_cnt * item_len);

	return ret;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduWriteRequest(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle,
								uint16_t &value_len,
								void *value,
								const uint16_t value_buf_size)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::WriteRequest);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeWriteRequest) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	handle = _pdu->pdu.args.writeRequest.handle;
	value_len = len - pdu_size;

	if (value_buf_size == 0) {
		return AKS_OK;
	}
	else if (value == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (value_buf_size < value_len) {
		return AKS_ERROR_NOBUF;
	}

	memcpy (value, _pdu->pdu.args.writeRequest.value, value_len);

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduWriteResponse(
								const uint8_t *pdu,
								const size_t len)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeWriteResponse) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduWriteCommand(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle,
								uint16_t &value_len,
								void *value,
								const uint16_t value_buf_size)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::WriteCommand);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeWriteCommand) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	handle = _pdu->pdu.args.writeCommand.handle;
	value_len = len - pdu_size;

	if (value_buf_size == 0) {
		return AKS_OK;
	}
	else if (value == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (value_buf_size < value_len) {
		return AKS_ERROR_NOBUF;
	}

	memcpy (value, _pdu->pdu.args.writeCommand.value, value_len);

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduPrepareWriteRequest(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle,
								uint16_t &offset,
								uint16_t &value_len,
								void *value,
								const uint16_t value_buf_size)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::PrepareWriteRequest);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodePrepareWriteRequest) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	handle = _pdu->pdu.args.prepareWriteRequest.handle;
	offset = _pdu->pdu.args.prepareWriteRequest.offset;
	value_len = len - pdu_size;

	if (value_buf_size == 0) {
		return AKS_OK;
	}
	else if (value == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (value_buf_size < value_len) {
		return AKS_ERROR_NOBUF;
	}

	memcpy (value, _pdu->pdu.args.prepareWriteRequest.value, value_len);

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduPrepareWriteResponse(
								const uint8_t *pdu,
								const size_t len,
								BtAttHandle &handle,
								uint16_t &offset,
								uint16_t &value_len,
								void *value,
								const uint16_t value_buf_size)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::PrepareWriteResponse);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodePrepareWriteResponse) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	handle = _pdu->pdu.args.prepareWriteResponse.handle;
	offset = _pdu->pdu.args.prepareWriteResponse.offset;

	value_len = len - pdu_size;

	if (value_buf_size == 0) {
		return AKS_OK;
	}
	else if (value == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (value_buf_size < value_len) {
		return AKS_ERROR_NOBUF;
	}

	memcpy (value, _pdu->pdu.args.prepareWriteResponse.value, value_len);

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int btAttParsePduExecuteWriteRequest(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &flags)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::ExecuteWriteRequest);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeExecuteWriteRequest) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	flags = _pdu->pdu.args.executeWriteRequest.flags;


	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int btAttParsePduExecuteWriteResponse(
								const uint8_t *pdu,
								const size_t len)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeExecuteWriteResponse) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int btAttParsePduHandleValueNotification(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle,
								uint16_t &value_len,
								void *value,
								const uint16_t value_buf_size)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::HandleValueNotification);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeHandleValueNotification) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	handle = _pdu->pdu.args.handleValueNotification.handle;

	value_len = len - pdu_size;

	if (value_buf_size == 0) {
		return AKS_OK;
	}
	else if (value == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (value_buf_size < value_len) {
		return AKS_ERROR_NOBUF;
	}

	memcpy (value, _pdu->pdu.args.handleValueNotification.value, value_len);

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduHandleValueIndication(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle,
								uint16_t &value_len,
								void *value,
								const uint16_t value_buf_size)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::HandleValueIndication);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeHandleValueIndication) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	handle = _pdu->pdu.args.handleValueIndication.handle;

	value_len = len - pdu_size;

	if (value_buf_size == 0) {
		return AKS_OK;
	}
	else if (value == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (value_buf_size < value_len) {
		return AKS_ERROR_NOBUF;
	}

	memcpy (value, _pdu->pdu.args.handleValueIndication.value, value_len);

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduHandleValueConfirmation(
							const uint8_t *pdu,
							const size_t len)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeHandleValueConfirmation) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btAttParsePduSignedWriteCommand(
								const uint8_t *pdu,
								const size_t len,
								uint16_t &handle,
								uint16_t &value_len,
								uint8_t *value,
								const uint16_t value_buf_size)
{
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}

	size_t pdu_size = sizeof(BtAttPdu::Pdu::opcode)
				 + sizeof(BtAttPdu::Pdu::Args::SignedWriteCommand);
	if (len < pdu_size) {
		return AKS_ERROR_BT_INCORRECT_PDU_SIZE;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;
	if (_pdu->pdu.opcode != BtAttPduOpcode::cAttOpcodeSignedWriteCommand) {
		return AKS_ERROR_BT_INVALID_OPCODE;
	}

	handle = _pdu->pdu.args.signedWriteCommand.handle;

	value_len = len - pdu_size - BT_ATT_SIGNATURE_SIZE;

	if (value_buf_size == 0) {
		return AKS_OK;
	}
	else if (value == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (value_buf_size < value_len) {
		return AKS_ERROR_NOBUF;
	}

	memcpy (value, _pdu->pdu.args.signedWriteCommand.value, value_len);

	//J Signの検証をどうするか？

	return AKS_OK;
}


