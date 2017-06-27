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


#include "bt_att.h"
#include "bt_gatt.h"
#include "aks_error.h"
#include "bt_util.h"

/*---------------------------------------------------------------------------*/
int btUtilCryptoSignAtt(
					struct bt_crypto *ctx,
					uint8_t *csrk,
					const uint8_t *data,
					const size_t size,
					const int sign_count,
					uint8_t *signature)
{
	(void)ctx;
	(void)csrk;
	(void)data;
	(void)size;
	(void)sign_count;
	(void)signature;

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
const char *btUtilGattUuidToString(BtAttUuid16 uuid)
{
	static const char *cGattAttributeTypeUuidString[]
	=
	{
		"Primary Service",
		"Secondary Service",
		"Include",
		"Characteristic",
		"Characteristic Extended Properties",
		"Client Characteristic Configuration",
		"Server Characteristic Configuration",
		"Characteristic Format",
		"Characteristic Aggregate Format"
	};
	
	static const BtAttUuid16 cGattAttributeTypeUuidIndex[]
	=
	{
		0x2800,
		0x2801,
		0x2802,
		0x2803,
		0x2900,
		0x2901,
		0x2902,
		0x2903,
		0x2904,
		0x2905
	};
	
	size_t num_types = sizeof(cGattAttributeTypeUuidIndex) / sizeof(cGattAttributeTypeUuidIndex[0]);
	for (size_t i=0 ; i<num_types ; ++i) {
		if (cGattAttributeTypeUuidIndex[i] == uuid) {
			return cGattAttributeTypeUuidString[i];
		}
	}

	return NULL;
}

/*---------------------------------------------------------------------------*/
const char *btUtilAttCharacteristicProperiesToString(uint8_t properties)
{
	static char properies_string[32];
	memset(properies_string, 0x00, sizeof(properies_string));
	
	char *str = properies_string;
	if (BtAttCharacteristicProperties::cBroadcast & properties) {
		str += sprintf (str, "|B");
	}
	else {
		str += sprintf (str, "| ");
	}
	
	if (BtAttCharacteristicProperties::cRead & properties) {
		str += sprintf (str, "|R");
	}
	else {
		str += sprintf (str, "| ");
	}
	
	if (BtAttCharacteristicProperties::cWriteWithoutResponse & properties) {
		str += sprintf (str, "|WwoR");
	}
	else {
		str += sprintf (str, "| ");
	}
	
	if (BtAttCharacteristicProperties::cWrite & properties) {
		str += sprintf (str, "|W");
	}
	else {
		str += sprintf (str, "| ");
	}
	
	if (BtAttCharacteristicProperties::cNotify & properties) {
		str += sprintf (str, "|N");
	}
	else {
		str += sprintf (str, "| ");
	}
	
	if (BtAttCharacteristicProperties::cIndicate & properties) {
		str += sprintf (str, "|I");
	}
	else {
		str += sprintf (str, "| ");
	}
	
	if (BtAttCharacteristicProperties::cAuthenticatedSignedWrites & properties) {
		str += sprintf (str, "|S");
	}
	else {
		str += sprintf (str, "| ");
	}
	
	if (BtAttCharacteristicProperties::cExtendedProperies & properties) {
		str += sprintf (str, "|E");
	}
	else {
		str += sprintf (str, "| ");
	}

	str += sprintf (str, "|");

	return properies_string;
	
}

/*---------------------------------------------------------------------------*/
bool btUtilAttCharacteristicProperiesHasBroadcast(uint8_t properties)
{
	return (BtAttCharacteristicProperties::cBroadcast & properties) ? true : false;
}

/*---------------------------------------------------------------------------*/
bool btUtilAttCharacteristicProperiesHasRead(uint8_t properties)
{
	return (BtAttCharacteristicProperties::cRead & properties) ? true : false;
}

/*---------------------------------------------------------------------------*/
bool btUtilAttCharacteristicProperiesHasWriteWithoutResponse(uint8_t properties)
{
	return (BtAttCharacteristicProperties::cWriteWithoutResponse & properties) ? true : false;
}

/*---------------------------------------------------------------------------*/
bool btUtilAttCharacteristicProperiesHasWrite(uint8_t properties)
{
	return (BtAttCharacteristicProperties::cWrite & properties) ? true : false;
}

/*---------------------------------------------------------------------------*/
bool btUtilAttCharacteristicProperiesHasNotify(uint8_t properties)
{
	return (BtAttCharacteristicProperties::cNotify & properties) ? true : false;
}

/*---------------------------------------------------------------------------*/
bool btUtilAttCharacteristicProperiesHasIndicate(uint8_t properties)
{
	return (BtAttCharacteristicProperties::cIndicate & properties) ? true : false;
}

/*---------------------------------------------------------------------------*/
bool btUtilAttCharacteristicProperiesHasAuthenticatedSignedWrites(uint8_t properties)
{
	return (BtAttCharacteristicProperties::cAuthenticatedSignedWrites & properties) ? true : false;
}

/*---------------------------------------------------------------------------*/
bool btUtilAttCharacteristicProperiesHasExtendedProperies(uint8_t properties)
{
	return (BtAttCharacteristicProperties::cExtendedProperies & properties) ? true : false;
}