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
#include "bt_gatt.h"

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int BtGattServerConfiguration::btGattExchangeMtu(
								BtGattDeviceContext &ctx)
{
	uint8_t pdu[BT_ATT_MAX_LE_MTU];

	int ret = btAttBuildPduExchangeMtuRequest(pdu, sizeof(pdu), ctx.client.mtu);
	if (ret < 0) {
		return ret;
	}
	size_t pdu_len = (size_t)ret;

	ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_len,
								BtAttPduOpcode::cAttOpcodeExchangeMtuResponse,
								0);
	if (ret != AKS_OK) {
		return ret;
	}

	uint16_t mtu = 0;
	ret = btAttParsePduExchangeMtuResponse(
								ctx.read_buf,
								(size_t)ctx.read_size,
								mtu);
	if( ret == AKS_OK) {
		ctx.server.mtu = mtu;
	}

	return ret;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int BtGattPrimaryServiceDiscovery::btGattDiscoverAllPrimaryServices(
								BtGattDeviceContext &ctx, 
								BtAttHandleRangeUuid16Pair *handleUuids,
								uint32_t pair_size,
								uint32_t &pair_cnt)
{
	uint8_t pdu[BT_ATT_MAX_LE_MTU];

	BtUuid uuid;
	uuid.format = BtUuid::cBtUuid16;
	uuid.value.uuid16 = GattAttributeTypeUuid::cPrimaryService;

	BtAttHandleRange range;
	range.start = 0x0001;
	range.end   = 0xffff;

	do {
		int ret = btAttBuildPduReadByGroupTypeRequest(
								pdu,
								sizeof(pdu),
								range, uuid);
		if (ret < 0) {
			return ret;
		}
		size_t pdu_len = (size_t)ret;

		ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_len,
								BtAttPduOpcode::cAttOpcodeReadByGroupTypeResponse,
								0);
		if (ret != AKS_OK) {
			return ret;
		}

		if (ctx.read_error == AKS_OK) {
			uint16_t item_len = 0;
			uint16_t item_cnt = 0;
			uint8_t  buf[BT_ATT_MAX_LE_MTU];

			ret = btAttParsePduReadByGroupTypeResponse(
								ctx.read_buf,
								(size_t)ctx.read_size,
								item_len,
								(void *)buf,
								sizeof(buf),
								item_cnt);
			if (ret != AKS_OK) {
				return ret;
			}

			BtAttAttributeData *attributeData = (BtAttAttributeData *)(buf);
			for (int i=0 ; i<item_cnt ; ++i) {
				if ((handleUuids != NULL) && (pair_cnt < pair_size) ) {
					handleUuids[pair_cnt].handles.start
										= attributeData->attributeHandle;
					handleUuids[pair_cnt].handles.end
										= attributeData->endGroupHandle;
					uint16_t value16=0;
					(void)btAttReadAttributeData16(attributeData, value16);
					handleUuids[pair_cnt].uuid = value16;
				}
				pair_cnt++;
				attributeData
						= btAttNextAttributeData(attributeData, item_len);
			}

			range.start = attributeData->endGroupHandle + 1;
		}
		else {
		}
	} while (ctx.read_error == AKS_OK);

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int BtGattPrimaryServiceDiscovery::btGattDiscoverPrimaryServicesByServiceUuid(
								BtGattDeviceContext &ctx,
								BtUuid uuid,
								BtAttHandleRange &handle)
{
	uint8_t pdu[BT_ATT_MAX_LE_MTU];

	BtAttHandleRange range;
	range.start = 0x0001;
	range.end   = 0xffff;

	int ret = btAttBuildPduFindByTypeValueRequest(
								pdu,
								sizeof(pdu),
								range,
								GattAttributeTypeUuid::cPrimaryService,
								(uint8_t *)&uuid.value.uuid16,
								sizeof(uuid.value.uuid16));
	if (ret < AKS_OK) {
		return ret;
	}
	size_t pdu_len = (size_t)ret;

	ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_len,
								BtAttPduOpcode::cAttOpcodeFindByTypeValueResponse,
								0);
	if (ret != AKS_OK) {
		return ret;
	}

	if (ctx.read_error == AKS_OK) {
		BtAttHandleRange handles;
		uint16_t cnt = 0;
		ret = btAttParsePduFindByTypeValueResponse(
								ctx.read_buf,
								(size_t)ctx.read_size,
								&handles,
								sizeof(handles),
								cnt);
		if (ret != AKS_OK) {
			return ret;
		}

		handle = handles;
	}
	else {
		return ctx.read_error;
	}

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int BtGattRelationshipDiscovery::btGattFindIncludedService(
								BtGattDeviceContext &ctx,
								BtAttHandleRange range)
{
	uint8_t pdu[BT_ATT_MAX_LE_MTU];

	BtUuid uuid;
	uuid.format = BtUuid::cBtUuid16;
	uuid.value.uuid16 = GattAttributeTypeUuid::cInclude;

	int ret = btAttBuildPduReadByTypeRequest(pdu, sizeof(pdu), range, uuid);
	if (ret < AKS_OK) {
		return ret;
	}
	size_t pdu_len = (size_t)ret;

	ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_len,
								BtAttPduOpcode::cAttOpcodeReadByTypeResponse,
								0);
	if (ret != AKS_OK) {
		return ret;
	}

	if (ctx.read_error != AKS_OK) {
		return ctx.read_error;
	}

	uint8_t item_len = 0;
	uint8_t item_cnt = 0;
	ret = btAttParsePduReadByTypeResponse(
								ctx.read_buf,
								(size_t)ctx.read_size,
								item_len,
								item_cnt,
								NULL,
								0);
	if (ret != AKS_OK) {
		return ret;
	}

	//J TODO 今のところこれに該当するデバイスが無いので検証できず
	//J 必要になったら実装する（たぶんしない）

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int BtGattCharacteristicDiscovery::btGattDiscoverAllCharactaristicOfAService(
								BtGattDeviceContext &ctx,
								BtAttHandleRange range,
								BtGattCharacteristic *chars,
								uint32_t char_len,
								uint32_t &char_cnt)

{
	uint8_t pdu[BT_ATT_MAX_LE_MTU];

	BtUuid uuid;
	uuid.format = BtUuid::cBtUuid16;
	uuid.value.uuid16 = GattAttributeTypeUuid::cCharacteristic;

	uint32_t list_count = 0;

	do {
		int ret = btAttBuildPduReadByTypeRequest(pdu, sizeof(pdu), range, uuid);
		if (ret < AKS_OK) {
			return ret;
		}
		size_t pdu_len = (size_t)ret;

		ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_len,
								BtAttPduOpcode::cAttOpcodeReadByTypeResponse,
								0);
		if (ret != AKS_OK) {
			return ret;
		}

		if (ctx.read_error != AKS_OK) {
			if (list_count != 0) {
				break;
			}
			return ctx.read_error;
		}

		uint8_t item_len = 0;
		uint8_t item_cnt = 0;
		uint8_t  buf[BT_ATT_MAX_LE_MTU];
		ret = btAttParsePduReadByTypeResponse(
								ctx.read_buf,
								(size_t)ctx.read_size,
								item_len,
								item_cnt,
								buf,
								sizeof(buf));
		if (ret != AKS_OK) {
			return ret;
		}

		BtAttAttributeDataReadByTypeResponse *readResponse
						= (BtAttAttributeDataReadByTypeResponse *)(buf);
		for (uint8_t i=0 ; i<item_cnt ; ++i) {

			if ((chars != NULL) && (list_count < char_len)) {
				chars[list_count].handle      = readResponse->handle;
				chars[list_count].properties  = readResponse->attributeValue.characteristic.properties;
				chars[list_count].valueHandle = readResponse->attributeValue.characteristic.valueHandle;

				//J UUID16
				if (item_len == 7) {
					chars[list_count].uuid.format = BtUuid::cBtUuid16;
					chars[list_count].uuid.value.uuid16 = readResponse->attributeValue.characteristic.uuid.uuid16;
				}
				//J UUID128
				else if (item_len == 21) {
					chars[list_count].uuid.format = BtUuid::cBtUuid128;
					chars[list_count].uuid.value.uuid128 = readResponse->attributeValue.characteristic.uuid.uuid128;
				}
			}

			list_count++;
			range.start = readResponse->handle + 1;
			readResponse = btAttNextAttributeDataReadByResponse(readResponse, item_len);
		}
	} while (1);

	char_cnt = list_count;

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int BtGattCharacteristicDiscovery::btGattDiscoverCharacteristicByUuid(
								BtGattDeviceContext	&ctx,
								BtAttHandleRange	range,
								BtUuid 				charUuid,
								BtGattCharacteristic &characteristic)

{
	uint8_t pdu[BT_ATT_MAX_LE_MTU];

	BtUuid uuid;
	uuid.format = BtUuid::cBtUuid16;
	uuid.value.uuid16 = GattAttributeTypeUuid::cCharacteristic;

	do {
		int ret = btAttBuildPduReadByTypeRequest(pdu, sizeof(pdu), range, uuid);
		if (ret < AKS_OK) {
			return ret;
		}
		size_t pdu_len = (size_t)ret;

		ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_len,
								BtAttPduOpcode::cAttOpcodeReadByTypeResponse,
								0);
		if (ret != AKS_OK) {
			return ret;
		}

		if (ctx.read_error != AKS_OK) {
			return ctx.read_error;
		}

		uint8_t item_len = 0;
		uint8_t item_cnt = 0;
		uint8_t  buf[BT_ATT_MAX_LE_MTU];
		ret = btAttParsePduReadByTypeResponse(
								ctx.read_buf,
								(size_t)ctx.read_size,
								item_len,
								item_cnt,
								buf,
								sizeof(buf));
		if (ret != AKS_OK) {
			return ret;
		}

		//J UUIDのサイズが違う場合、捜索できない
		if ((item_len == 7) && (charUuid.format != BtUuid::cBtUuid16)) {
			return AKS_ERROR_BT_IMCOMPATIBLE_UUID;
		}
		else if ((item_len == 21) && (charUuid.format != BtUuid::cBtUuid128)){
			return AKS_ERROR_BT_IMCOMPATIBLE_UUID;
		}

		BtAttAttributeDataReadByTypeResponse *readResponse = (BtAttAttributeDataReadByTypeResponse *)(buf);
		for (uint8_t i=0 ; i<item_cnt ; ++i) {
			//J UUID16
			if ((item_len == 7) && (charUuid.value.uuid16 == readResponse->attributeValue.characteristic.uuid.uuid16)) {
				characteristic.handle      = readResponse->handle;
				characteristic.properties  = readResponse->attributeValue.characteristic.properties;
				characteristic.valueHandle = readResponse->attributeValue.characteristic.valueHandle;
				characteristic.uuid.format = BtUuid::cBtUuid16;
				characteristic.uuid.value.uuid16 = readResponse->attributeValue.characteristic.uuid.uuid16;

				return AKS_OK;
			}
			//J UUID128
			else if ((item_len == 21) &&
					 (0 == memcmp(
					 		&charUuid.value.uuid128,
					 		&readResponse->attributeValue.characteristic.uuid.uuid128,
					 		sizeof(charUuid.value.uuid128))) )
			{
				characteristic.handle      = readResponse->handle;
				characteristic.properties  = readResponse->attributeValue.characteristic.properties;
				characteristic.valueHandle = readResponse->attributeValue.characteristic.valueHandle;
				characteristic.uuid.format = BtUuid::cBtUuid128;
				characteristic.uuid.value.uuid128 = readResponse->attributeValue.characteristic.uuid.uuid128;

				return AKS_OK;
			}

			range.start = readResponse->handle + 1;
			readResponse = btAttNextAttributeDataReadByResponse(readResponse, item_len);
		}
	} while (1);

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int BtGattCharacteristicDescriptorDiscovery::btGattDiscoverAllCharacteristicDescriptors(
								BtGattDeviceContext	&ctx,
								BtAttHandleRange	range,
								BtAttHandleUuidPair *pairs,
								uint32_t            pair_size,
								uint32_t            &pair_count)
{
	uint8_t pdu[BT_ATT_MAX_LE_MTU];
	int ret = AKS_OK;

	pair_count = 0;
	while (1) {
		ret = btAttBuildPduFindInformationRequest(pdu, sizeof(pdu), range);
		if (ret < AKS_OK) {
			return ret;
		}
		size_t pdu_size = (size_t)ret;

		ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_size,
								BtAttPduOpcode::cAttOpcodeFindInformationResponse,
								0);
		if (ret != AKS_OK) {
			return ret;
		}

		if (ctx.read_error != AKS_OK) {
			ret = ctx.read_error;
			break;
		}

		uint8_t format    = 0;
		uint16_t item_cnt = 0;
		uint8_t handle_uuid_pair[BT_ATT_MAX_LE_MTU];
		ret = btAttParsePduFindInformationResponse(
								ctx.read_buf,
								(size_t)ctx.read_size,
								format,
								item_cnt,
								handle_uuid_pair,
								sizeof(handle_uuid_pair));
		if (ret != AKS_OK) {
			return ret;
		}

		if (pairs == NULL) {
			continue;
		}
		else if (pair_size < (pair_count + item_cnt)) {
			return AKS_ERROR_NOBUF;
		}

		if (format == 0x01) {
			BtAttHandleUuid16Pair *tmp_pairs = (BtAttHandleUuid16Pair *)handle_uuid_pair;
			for (uint16_t i= 0 ; i<item_cnt ; ++i) {
				pairs[pair_count].handle = tmp_pairs->handle;
				pairs[pair_count].uuid.format = BtUuid::cBtUuid16;
				pairs[pair_count].uuid.value.uuid16 = tmp_pairs->uuid;

				pair_count++;
				tmp_pairs++;
			}
		}
		else if (format == 0x02) {
			BtAttHandleUuid128Pair *tmp_pairs = (BtAttHandleUuid128Pair *)handle_uuid_pair;
			for (uint16_t i= 0 ; i<item_cnt ; ++i) {
				pairs[pair_count].handle = tmp_pairs->handle;
				pairs[pair_count].uuid.format = BtUuid::cBtUuid128;
				pairs[pair_count].uuid.value.uuid128 = tmp_pairs->uuid;

				pair_count++;
				tmp_pairs++;
			}
		}
		else {
			return AKS_ERROR_BT_INVALUD_FORMAT;
		}

		range.start = pairs[pair_count-1].handle + 1;
	}

	if (pair_count != 0) {
		return AKS_OK;
	}
	else {
		return ret;
	}
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int BtGattCharacteristicValueRead::btGattReadCharacteristicValue(
								BtGattDeviceContext	&ctx,
								BtAttHandle			handle,
								void				*buf,
								size_t				buf_size,
								size_t				&read_size)
{
	if ((buf == NULL) || (buf_size == 0)) {
		return AKS_ERROR_NOBUF;
	}

	uint8_t pdu[BT_ATT_MAX_LE_MTU];

	int ret = btAttBuildPduReadRequest(pdu, sizeof(pdu), handle);
	if (ret < AKS_OK) {
		return ret;
	}
	size_t pdu_size = (size_t)ret;

	ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_size,
								BtAttPduOpcode::cAttOpcodeReadResponse,
								0);
	if (ret != AKS_OK) {
		return ret;
	}

	if (ctx.read_error != AKS_OK) {
		return ctx.read_error;
	}

	uint16_t read_size16 = 0;
	ret = btAttParsePduReadResponse(
								ctx.read_buf,
								(size_t)ctx.read_size,
								buf,
								buf_size,
								read_size16);

	read_size = (size_t)read_size16;

	return ret;
}


/*---------------------------------------------------------------------------*/
int BtGattCharacteristicValueRead::btGattReadUsingCharacteristicUuid(
								BtGattDeviceContext	&ctx,
								BtUuid				uuid,
								BtAttHandle			&handle,
								void				*buf,
								size_t				buf_size,
								size_t				&read_size)
{
	if ((buf == NULL) || (buf_size == 0)) {
		return AKS_ERROR_NOBUF;
	}

	uint8_t pdu[BT_ATT_MAX_LE_MTU];

	BtAttHandleRange range;
	range.start = 0x0001;
	range.end   = 0xffff;

	int ret = btAttBuildPduReadByTypeRequest(pdu, sizeof(pdu), range, uuid);
	if (ret < AKS_OK) {
		return ret;
	}
	size_t pdu_size = (size_t)ret;

	ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_size,
								BtAttPduOpcode::cAttOpcodeReadByTypeResponse,
								0);
	if (ret != AKS_OK) {
		return ret;
	}

	if (ctx.read_error != AKS_OK) {
		return ctx.read_error;
	}

	uint8_t item_len;
	uint8_t item_cnt;
	uint8_t tmp_buf[BT_ATT_MAX_LE_MTU];
	ret = btAttParsePduReadByTypeResponse(
								ctx.read_buf,
								(size_t)ctx.read_size,
								item_len,
								item_cnt,
								tmp_buf,
								sizeof(tmp_buf));
	if (ret != AKS_OK) {
		return ret;
	}
	if (item_cnt > 1) {
		return AKS_ERROR_BT_UNEXPECTED_RESPONSE;
	}

	memcpy(&handle, &tmp_buf[0], sizeof(handle));

	if ((buf != NULL) && (buf_size > (size_t)(item_len-2))) {
		memcpy(buf, &(tmp_buf[2]), item_len);
	}

	read_size = item_len - 2; //J Handle + value で帰ってくるのでHandle分引く

	return ret;
}


/*---------------------------------------------------------------------------*/
int BtGattCharacteristicValueRead::btGattReadLongCharacteristicValues (
								BtGattDeviceContext	&ctx,
								BtAttHandle			handle,
								void				*buf,
								size_t				buf_size,
								size_t				&read_size)
{
	if ((buf == NULL) || (buf_size == 0)) {
		return AKS_ERROR_NOBUF;
	}

	uint8_t pdu[BT_ATT_MAX_LE_MTU];

	int ret = btAttBuildPduReadRequest(pdu, sizeof(pdu), handle);
	if (ret < AKS_OK) {
		return ret;
	}
	size_t pdu_size = (size_t)ret;

	ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_size,
								BtAttPduOpcode::cAttOpcodeReadResponse,
								0);
	if (ret != AKS_OK) {
		return ret;
	}

	if (ctx.read_error != AKS_OK) {
		return ctx.read_error;
	}

	uint8_t *ptr = (uint8_t *)buf;
	uint16_t read_size16 = 0;
	ret = btAttParsePduReadResponse(
								ctx.read_buf,
								(size_t)ctx.read_size,
								ptr,
								buf_size,
								read_size16);
	if (ret != AKS_OK) {
		return ret;
	}

	ptr += read_size16;
	read_size = (size_t)read_size16;

	while ((ret == AKS_OK) && (read_size16 != 0)) {
		ret = btAttBuildPduReadBlobRequest (
								pdu,
								sizeof(pdu),
								handle,
								(uint16_t)read_size);
		if (ret < AKS_OK) {
			return ret;
		}

		pdu_size = (size_t)ret;
		ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_size,
								BtAttPduOpcode::cAttOpcodeReadBlobResponse,
									0);
		if (ret != AKS_OK) {
			return ret;
		}

		if (ctx.read_error != AKS_OK) {
			return ctx.read_error;
		}

		ret = btAttParsePduReadBlobResponse(
								ctx.read_buf,
								(size_t)ctx.read_size,
								ptr,
								buf_size - read_size,
								read_size16);
		if (ret != AKS_OK) {
			return ret;;
		}

		ptr       += read_size16;
		read_size += (size_t)read_size16;
	}

	return AKS_OK;
}



/*---------------------------------------------------------------------------*/
int BtGattCharacteristicValueRead::btGattMultipleCharacteristicValues(
								BtGattDeviceContext	&ctx,
								BtAttHandle			*handles,
								size_t				num_handles,
								void				*buf,
								size_t				buf_size,
								size_t				&read_size)
{
	if (handles == NULL) {
		return AKS_ERROR_NULL;
	}
	if ((buf == NULL) || (buf_size == 0)) {
		return AKS_ERROR_NOBUF;
	}

	uint8_t pdu[BT_ATT_MAX_LE_MTU];

	int ret = btAttBuildPduReadMultipleRequest(pdu, sizeof(pdu), handles, num_handles);
	if (ret < AKS_OK) {
		return ret;
	}
	size_t pdu_size = (size_t)ret;

	ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_size,
								BtAttPduOpcode::cAttOpcodeReadMultipleResponse,
								0);
	if (ret != AKS_OK) {
		return ret;
	}

	if (ctx.read_error != AKS_OK) {
		return ctx.read_error;
	}

	ret = btAttParsePduReadMultipleResponse(
								ctx.read_buf,
								(size_t)ctx.read_size,
								buf,
								buf_size,
								&read_size);
	if (ret != AKS_OK) {
		return ret;
	}

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int BtGattCharacteristicValueWrite::btGattWriteWithoutResponse(
								BtGattDeviceContext	&ctx,
								const BtAttHandle	handle,
								const void			*buf,
								const size_t		buf_size)
{
	if ((buf == NULL) || (buf_size == 0)) {
		return AKS_ERROR_NOBUF;
	}

	uint8_t pdu[BT_ATT_MAX_LE_MTU];

	int ret = btAttBuildPduWriteCommand(
								pdu,
								sizeof(pdu),
								handle,
								(uint16_t)buf_size,
								(const uint8_t *)buf);
	if (ret < AKS_OK) {
		return ret;
	}
	size_t pdu_size = (size_t)ret;

	ret = btLeDeviceSendAttPdu(
								&ctx,
								pdu,
								pdu_size);
	if (ret != AKS_OK) {
		return ret;
	}

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int BtGattCharacteristicValueWrite:: btGattSignedWriteWithoutResponse(
								BtGattDeviceContext	&ctx,
								const BtAttHandle	handle,
								const void			*buf,
								const size_t		buf_size)
{
	(void)ctx;
	(void)handle;
	(void)buf;
	(void)buf_size;

	return AKS_ERROR_NOT_IMPLEMENTED;
}


/*---------------------------------------------------------------------------*/
int BtGattCharacteristicValueWrite::btGattWriteCharacteristicValue(
								BtGattDeviceContext	&ctx,
								const BtAttHandle	handle,
								const void			*buf,
								const size_t		buf_size)
{
	if ((buf == NULL) || (buf_size == 0)) {
		return AKS_ERROR_NOBUF;
	}

	uint8_t pdu[BT_ATT_MAX_LE_MTU];
	int ret = btAttBuildPduWriteRequest(
								pdu,
								sizeof(pdu),
								handle,
								(uint16_t)buf_size,
								(const uint8_t *)buf);
	if (ret < AKS_OK) {
		return ret;
	}
	size_t pdu_size = (size_t)ret;

	ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_size,
								BtAttPduOpcode::cAttOpcodeWriteResponse,
								0);
	if (ret != AKS_OK) {
		return ret;
	}

	if (ctx.read_error != AKS_OK) {
		return ctx.read_error;
	}

	ret = btAttParsePduWriteResponse(
								ctx.read_buf,
								(size_t)ctx.read_size);
	if (ret != AKS_OK) {
		return ret;
	}

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int BtGattCharacteristicValueWrite::btGattWriteLongCharacteristicValues(
								BtGattDeviceContext	&ctx,
								const BtAttHandle	handle,
								const void			*buf,
								const size_t		buf_size)
{
	uint8_t pdu[BT_ATT_MAX_LE_MTU];
	uint16_t offset = 0;
	uint16_t write_size = 0;
	size_t remaining_size = buf_size;

	if ((buf == NULL) || (buf_size == 0)) {
		return AKS_ERROR_NOBUF;
	}

	while (remaining_size) {
		write_size = ((size_t)(ctx.server.mtu-5) < remaining_size) ? (ctx.server.mtu-5) : remaining_size;
		int ret = btAttBuildPduPrepareWriteRequest(
								pdu,
								sizeof(pdu),
								handle, offset,
								write_size,
								&(((const uint8_t *)buf)[offset]));
		if (ret < AKS_OK) {
			return ret;
		}
		size_t pdu_size = (size_t)ret;

		ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_size,
								BtAttPduOpcode::cAttOpcodePrepareWriteResponse,
								0);
		if (ctx.read_error != AKS_OK) {
			return ctx.read_error;
		}

		BtAttHandle response_handle = 0;
		uint16_t response_value_len = 0;
		uint16_t response_offset = 0;
		uint8_t response_value[BT_ATT_MAX_LE_MTU];
		ret = btAttParsePduPrepareWriteResponse(
								ctx.read_buf,
								(size_t)ctx.read_size,
								response_handle,
								response_offset,
								response_value_len,
								response_value,
								sizeof(response_value));
		if (ret != AKS_OK) {
			return ret;
		}

		if ((response_handle != handle) ||
			(response_offset != offset) ||
			(response_value_len != write_size))
		{
			return AKS_ERROR_BT_IMCOMPLETED_WRITE;
		}
		else if (0 != memcmp(
								response_value,
								&(((const uint8_t *)buf)[offset]),
								write_size) )
		{
			return AKS_ERROR_BT_IMCOMPLETED_WRITE;
		}

		remaining_size -= write_size;
		offset += write_size;
	}

	int ret = btAttBuildPduExecuteWriteRequest(
								pdu,
								sizeof(pdu),
								BtAttExecuteWriteFlag::cImmediatelyWriteAllPendingPreparedValues);
	if (ret < AKS_OK) {
		return ret;
	}
	size_t pdu_size = (size_t)ret;

	ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_size,
								BtAttPduOpcode::cAttOpcodeExecuteWriteResponse,
								0);
	if (ret != AKS_OK) {
		return ret;
	}

	if (ctx.read_error != AKS_OK) {
		return ctx.read_error;
	}

	ret = btAttParsePduExecuteWriteResponse(
								ctx.read_buf,
								(size_t)ctx.read_size);
	if (ret != AKS_OK) {
		return ret;
	}

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int BtGattCharacteristicValueWrite::btGattWriteCharacteristicValueReliableWrites(
								BtGattDeviceContext		&ctx,
								BtGattHandleValueSet	*handleValueSet,
								size_t					set_len)
{
	if (handleValueSet == NULL) {
		return AKS_ERROR_NULL;
	}

	int ret = 0;
	uint8_t pdu[BT_ATT_MAX_LE_MTU];
	for (size_t i=0 ; i<set_len ; ++i) {
		ret = btAttBuildPduPrepareWriteRequest(
								pdu,
								sizeof(pdu),
								handleValueSet[i].handle,
								0x0000,
								handleValueSet[i].size,
								handleValueSet[i].value);
		if (ret < AKS_OK) {
			return ret;
		}
		size_t pdu_size = (size_t)ret;

		ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_size,
								BtAttPduOpcode::cAttOpcodePrepareWriteResponse,
								0);

		if (ctx.read_error != AKS_OK) {
			return ctx.read_error;
		}

		BtAttHandle response_handle = 0;
		uint16_t response_value_len = 0;
		uint16_t response_offset = 0;
		uint8_t response_value[BT_ATT_MAX_LE_MTU];
		ret = btAttParsePduPrepareWriteResponse(
								ctx.read_buf,
								(size_t)ctx.read_size,
								response_handle,
								response_offset,
								response_value_len,
								response_value,
								sizeof(response_value));
		if (ret != AKS_OK) {
			return ret;
		}

		if ((response_handle != handleValueSet[i].handle) || 
			(response_value_len != handleValueSet[i].size)) {
			return AKS_ERROR_BT_IMCOMPLETED_WRITE;
		}
		else if (0 != memcmp(
								response_value,
								handleValueSet[i].value,
								handleValueSet[i].size) )
		{
			return AKS_ERROR_BT_IMCOMPLETED_WRITE;
		}
	}
	ret = btAttBuildPduExecuteWriteRequest(
								pdu,
								sizeof(pdu),
								BtAttExecuteWriteFlag::cImmediatelyWriteAllPendingPreparedValues);
	if (ret < AKS_OK) {
		return ret;
	}
	size_t pdu_size = (size_t)ret;

	ret = btLeDeviceSendAttPduAndWaitForResponse(
								&ctx,
								pdu,
								pdu_size,
								BtAttPduOpcode::cAttOpcodeExecuteWriteResponse,
								0);
	if (ret != AKS_OK) {
		return ret;
	}

	if (ctx.read_error != AKS_OK) {
		return ctx.read_error;
	}

	ret = btAttParsePduExecuteWriteResponse(
								ctx.read_buf,
								(size_t)ctx.read_size);
	if (ret != AKS_OK) {
		return ret;
	}

	return AKS_OK;
}
