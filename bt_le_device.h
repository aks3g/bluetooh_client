/*
 * Copyright 2016 Kiyotaka Akasaka
 *
 * Released under the MIT license
 * http://opensource.org/licenses/mit-license.php
 */
#ifndef BT_LE_DEVICE_H_
#define BT_LE_DEVICE_H_


#define BT_LE_DEVICE_MAX_NOTIFICATION				(16)

typedef int (*BtGattNotificationCb)(uint8_t *value, size_t value_len);

struct BtGattNotificationContext{
	BtAttHandle value_handle;
	BtGattNotificationCb cb;
};

struct BtGattDeviceContext
{
	bool connected;

	int btdevice;
	struct {
		uint16_t mtu;
	} client;
	
	struct {
		uint16_t mtu;
	} server;

	pthread_t receiveThread;
	pthread_mutex_t blockWaitMutex;
	pthread_cond_t  blockWaitCv;

	uint8_t requestedOpcode;
	uint8_t expectedResponseOpcode;
	uint8_t read_buf[BT_ATT_MAX_LE_MTU];
	ssize_t read_size;
	int     read_error;

	int num_notification;
	BtGattNotificationContext notification_list[BT_LE_DEVICE_MAX_NOTIFICATION];
};

int btLeDeviceCreate(BtGattDeviceContext *ctx, const char *btaddr);
int btLeDeviceDestroy(BtGattDeviceContext *ctx);

int btLeDeviceSendAttPdu(BtGattDeviceContext *ctx, const uint8_t *pdu, const size_t len);
int btLeDeviceSendAttPduAndWaitForResponse(
								BtGattDeviceContext *ctx,
								const uint8_t *pdu,
								const size_t len,
								const uint8_t expectedResponse,
								const uint32_t timeout_ns);

int btLeDeviceRegistNotificationCallback(BtGattDeviceContext *ctx, BtAttHandle config_handle, BtAttHandle value_handle, BtGattNotificationCb cb);
// int btDeviceSetClientMtu(BtGattDeviceContext &ctx, uint16_t mtu);


#endif/*BT_LE_DEVICE_H_*/