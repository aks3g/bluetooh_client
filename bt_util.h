/*
 * Copyright 2016 Kiyotaka Akasaka
 *
 * Released under the MIT license
 * http://opensource.org/licenses/mit-license.php
 */
#ifndef BT_UTIL_H_
#define BT_UTIL_H_


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/



/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
struct bt_crypto
{
};

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int btUtilCryptoSignAtt(
					struct bt_crypto *ctx,
					uint8_t *csrk,
					const uint8_t *data,
					const size_t size,
					const int sign_count,
					uint8_t *signature);

const char *btUtilGattUuidToString(BtAttUuid16 uuid);
const char *btUtilAttCharacteristicProperiesToString(uint8_t properties);

bool btUtilAttCharacteristicProperiesHasBroadcast(uint8_t properties);
bool btUtilAttCharacteristicProperiesHasRead(uint8_t properties);
bool btUtilAttCharacteristicProperiesHasWriteWithoutResponse(uint8_t properties);
bool btUtilAttCharacteristicProperiesHasWrite(uint8_t properties);
bool btUtilAttCharacteristicProperiesHasNotify(uint8_t properties);
bool btUtilAttCharacteristicProperiesHasIndicate(uint8_t properties);
bool btUtilAttCharacteristicProperiesHasAuthenticatedSignedWrites(uint8_t properties);
bool btUtilAttCharacteristicProperiesHasExtendedProperies(uint8_t properties);

#endif/*BT_UTIL_H*/