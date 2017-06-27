/*
 * Copyright 2016 Kiyotaka Akasaka
 *
 * Released under the MIT license
 * http://opensource.org/licenses/mit-license.php
 */
#ifndef BT_GATT_H_
#define BT_GATT_H_

#include "bt_le_device.h"

/* Gatt profile attribute types */
struct GattAttributeTypeUuid{
	static const uint16_t cPrimaryService							= 0x2800;
	static const uint16_t cSecondaryService							= 0x2801;
	static const uint16_t cInclude									= 0x2802;
	static const uint16_t cCharacteristic							= 0x2803;
	static const uint16_t cCharacteristicExtendedPropertis			= 0x2900;
	static const uint16_t cCharacteristicUserDescription			= 0x2901;
	static const uint16_t cClientCharacteristicConfiguration		= 0x2902;
	static const uint16_t cServerCharacteristicConfiguration		= 0x2903;
	static const uint16_t cCharacteristicFormat						= 0x2904;
	static const uint16_t cCharacteristicAgreegateFormat			= 0x2905;
};

struct BtGattCharacteristic
{
	BtAttHandle	handle;
	uint8_t		properties;
	BtAttHandle	valueHandle;
	BtUuid		uuid;
};


struct BtGattHandleValueSet
{
	BtAttHandle handle;
	uint8_t		*value;
	size_t		size;
};

namespace BtGattServerConfiguration
{
	int btGattExchangeMtu(BtGattDeviceContext &ctx);
}

namespace BtGattPrimaryServiceDiscovery
{
	int btGattDiscoverAllPrimaryServices(	BtGattDeviceContext &ctx,
											BtAttHandleRangeUuid16Pair *handleUuids,
											uint32_t pair_size,
											uint32_t &pair_cnt);
	int btGattDiscoverPrimaryServicesByServiceUuid(
											BtGattDeviceContext &ctx,
											BtUuid uuid,
											BtAttHandleRange &handle);
}

namespace BtGattRelationshipDiscovery
{
	int btGattFindIncludedService(			BtGattDeviceContext &ctx,
											BtAttHandleRange range);
}

namespace BtGattCharacteristicDiscovery
{
	int btGattDiscoverAllCharactaristicOfAService(
											BtGattDeviceContext &ctx,
											BtAttHandleRange range,
											BtGattCharacteristic *chars,
											uint32_t char_len,
											uint32_t &char_cnt);
	int btGattDiscoverCharacteristicByUuid(
											BtGattDeviceContext	&ctx,
											BtAttHandleRange	range,
											BtUuid 				charUuid,
											BtGattCharacteristic &characteristic);
			}

namespace BtGattCharacteristicDescriptorDiscovery
{
	int btGattDiscoverAllCharacteristicDescriptors(
								BtGattDeviceContext	&ctx,
								BtAttHandleRange	range,
								BtAttHandleUuidPair *pairs,
								uint32_t			pair_size,
								uint32_t			&pair_count);
}

namespace BtGattCharacteristicValueRead
{
	int btGattReadCharacteristicValue(
								BtGattDeviceContext	&ctx,
								BtAttHandle			handle,
								void				*buf,
								size_t				buf_size,
								size_t				&read_size);
	int btGattReadUsingCharacteristicUuid(
								BtGattDeviceContext	&ctx,
								BtUuid				uuid,
								BtAttHandle			&handle,
								void				*buf,
								size_t				buf_size,
								size_t				&read_size);
	int btGattReadLongCharacteristicValues(
								BtGattDeviceContext	&ctx,
								BtAttHandle			handle,
								void				*buf,
								size_t				buf_size,
								size_t				&read_size);
	int btGattMultipleCharacteristicValues(
								BtGattDeviceContext	&ctx,
								BtAttHandle			*handles,
								size_t				num_handles,
								void				*buf,
								size_t				buf_size,
								size_t				&read_size);
}

namespace BtGattCharacteristicValueWrite
{
	int btGattWriteWithoutResponse(
								BtGattDeviceContext	&ctx,
								const BtAttHandle	handle,
								const void			*buf,
								const size_t		buf_size);
	int btGattSignedWriteWithoutResponse(
								BtGattDeviceContext	&ctx,
								const BtAttHandle	handle,
								const void			*buf,
								const size_t		buf_size);
	int btGattWriteCharacteristicValue(
								BtGattDeviceContext	&ctx,
								const BtAttHandle	handle,
								const void			*buf,
								const size_t		buf_size);
	int btGattWriteLongCharacteristicValues(
								BtGattDeviceContext	&ctx,
								const BtAttHandle	handle,
								const void			*buf,
								const size_t		buf_size);
	int btGattWriteCharacteristicValueReliableWrites(
								BtGattDeviceContext		&ctx,
								BtGattHandleValueSet	*handleValueSet,
								size_t					set_len);
}

namespace BtGattCharacteristicDescriptorValueRead
{
	int btGattReadCharacteristicDecriptors();
	int btGattReadLongCharacteristicDescriptors();
}

namespace BtGattCharacteristicDescriptorValueWrite
{
	int btGattWriteCharacteristicDescriptors();
	int btGattWriteLongCharacteristicDescriptors();
}



#endif/*BT_GATT_H_*/
