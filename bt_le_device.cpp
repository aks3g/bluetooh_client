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

#include <signal.h>
#include <pthread.h>

#include <bluetooth/bluetooth.h>
#include <bluetooth/l2cap.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>


#include "aks_error.h"
#include "bt_att.h"
#include "bt_gatt.h"
#include "bt_le_device.h"



#define BT_SEC_LEVEL_SDP						(0)
#define BT_SEC_LEVEL_LOW						(1)
#define BT_SEC_LEVEL_MEDIUM						(2)
#define BT_SEC_LEVEL_HIGH						(3)


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int _create_ble_socket(const char *btaddr);
static void *_ble_receive_thread_func(void *arg);

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int btLeDeviceCreate(BtGattDeviceContext *ctx, const char *btaddr)
{
	if (ctx == NULL) {
		return AKS_ERROR_NULL;
	}
	else if (btaddr == NULL) {
		return AKS_ERROR_NULL;
	}

	memset (ctx, 0x00, sizeof(BtGattDeviceContext));

	int ret = _create_ble_socket(btaddr);
	if (ret < AKS_OK) {
		return ret;
	}

	{
		ctx->btdevice = ret;
		ctx->client.mtu = BT_ATT_MIN_LE_MTU;
		ctx->server.mtu = BT_ATT_MIN_LE_MTU;
	}

	ret = pthread_create(&ctx->receiveThread, NULL, _ble_receive_thread_func, (void *)ctx);
	if (ret != 0) {
		return ret;
	}

	ret = pthread_cond_init(&ctx->blockWaitCv, NULL);
	if (ret != 0) {
		return ret;
	}

	ret = pthread_mutex_init(&ctx->blockWaitMutex, NULL);
	if (ret != 0) {
		return ret;
	}

	ctx->connected = true;

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btLeDeviceDestroy(BtGattDeviceContext *ctx)
{
	if (ctx == NULL) {
		return AKS_ERROR_NULL;
	}

	close (ctx->btdevice);

	//J Thread終了
	
	//J 同期オブジェクト破壊
	pthread_cond_destroy(&ctx->blockWaitCv);
	pthread_mutex_destroy(&ctx->blockWaitMutex);

	ctx->connected = false;

	return AKS_OK;
}

/*---------------------------------------------------------------------------*/
int btLeDeviceSendAttPdu(
								BtGattDeviceContext *ctx,
								const uint8_t *pdu,
								const size_t len)
{
	if (ctx == NULL) {
		return AKS_ERROR_NULL;
	}
	if (pdu == NULL) {
		return AKS_ERROR_NULL;
	}
	if (len == 0) {
		return AKS_ERROR_NOBUF;
	}

	size_t ret = write (ctx->btdevice, pdu, len);
	if (ret != len) {
		return AKS_ERROR_IO;
	}

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int btLeDeviceSendAttPduAndWaitForResponse(
								BtGattDeviceContext *ctx,
								const uint8_t *pdu,
								const size_t len,
								const uint8_t expectedResponse,
								const uint32_t timeout_ns)
{
	if (ctx == NULL) {
		return AKS_ERROR_NULL;
	}

	int ret = btLeDeviceSendAttPdu(ctx, pdu, len);
	if (ret != AKS_OK) {
		return ret;
	}

	BtAttPdu *_pdu = (BtAttPdu *)pdu;

	ctx->requestedOpcode        = _pdu->pdu.opcode;
	ctx->expectedResponseOpcode = expectedResponse;
	if (timeout_ns != 0) {
		struct timespec timeout;
		memset (&timeout, 0x00, sizeof(timeout));
		timeout.tv_nsec = (long)timeout_ns;

		int ret = pthread_cond_timedwait(&ctx->blockWaitCv, &ctx->blockWaitMutex, &timeout);
		if (ret != 0) {
			ctx->expectedResponseOpcode = 0;
			return ret;
		}
	}
	else {
		int ret = pthread_cond_wait(&ctx->blockWaitCv, &ctx->blockWaitMutex);
		if (ret != 0) {
			ctx->expectedResponseOpcode = 0;
			return ret;
		}
	}

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
int btLeDeviceRegistNotificationCallback(
								BtGattDeviceContext *ctx,
								BtAttHandle config_handle,
								BtAttHandle value_handle,
								BtGattNotificationCb cb)
{
	if (ctx == NULL) {
		return AKS_ERROR_NULL;
	}
	
	if (ctx->num_notification >= BT_LE_DEVICE_MAX_NOTIFICATION) {
		return AKS_ERROR_FULL;
	}

	uint16_t config = BtAttClientCharacteristicConfiguration::cNotification;
	int ret = BtGattCharacteristicValueWrite::btGattWriteWithoutResponse(*ctx, config_handle, &config, sizeof(config));
	if (ret != AKS_OK) {
		return ret;
	}

	ctx->notification_list[ctx->num_notification].value_handle = value_handle;
	ctx->notification_list[ctx->num_notification].cb = cb;

	ctx->num_notification++;

	return AKS_OK;
}


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
static int _create_ble_socket(const char *btaddr)
{
	int ret = 0;
	bdaddr_t host_bt_addr;
	bdaddr_t target_bt_addr;
	
	memset(&host_bt_addr, 0, sizeof(host_bt_addr));
	memset(&target_bt_addr, 0, sizeof(target_bt_addr));

	ret = hci_devba(0, &host_bt_addr);
	if (ret != 0) {
//		printf ("hci_devba(). ret = %d,  errno = %d\n", ret, errno);
		return ret;
	}

	ret = str2ba(btaddr, &target_bt_addr);
	if (ret != 0) {
//		printf ("str2ba(). ret = %d,  errno = %d\n", ret, errno);
		return ret;
	}

	int sock = socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP);
	if (sock < 0) {
//		printf ("socket(PF_BLUETOOTH, SOCK_SEQPACKET, BTPROTO_L2CAP) was failed. errno = %d\n", errno);
		return -errno;
	}

	//J Host 側の準備
	struct sockaddr_l2 host_addr;
	{
		memset(&host_addr, 0, sizeof(host_addr));
		host_addr.l2_family      = AF_BLUETOOTH;
		host_addr.l2_cid         = htobs(BT_ATT_L2CAP_CID);
		host_addr.l2_psm         = 0;
		host_addr.l2_bdaddr_type = BDADDR_LE_PUBLIC;
		bacpy(&host_addr.l2_bdaddr, &host_bt_addr);
	}
	ret = bind(sock, (struct sockaddr *)&host_addr, sizeof(host_addr));
	if (ret < 0) {
//		printf ("bind(sock, (struct sockaddr *)&addr, sizeof(addr)) was failed. errno = %d\n", errno);
		return -errno;
	}

	//J Socket にオプションを付与
	{
		struct bt_security security_opt;
		{
			memset(&security_opt, 0, sizeof(security_opt));
			security_opt.level = BT_SEC_LEVEL_LOW;
		}
		ret = setsockopt(sock, SOL_BLUETOOTH, BT_SECURITY, &security_opt, sizeof(security_opt));
		if (ret < 0) {
//			printf ("setsockopt(opt, SOL_BLUETOOTH, BT_SECURITY, &sec, sizeof(sec)) was failed. errno = %d\n", errno);
			return -errno;
		}
	}

	//J Target 側の準備
	struct sockaddr_l2 target_addr;
	{
		memset (&target_addr, 0x00, sizeof(target_addr));
		target_addr.l2_family      = AF_BLUETOOTH;
		target_addr.l2_cid         = htobs(BT_ATT_L2CAP_CID);
		target_addr.l2_psm         = 0;
		target_addr.l2_bdaddr_type = BDADDR_LE_RANDOM; //J Device に依存するので、可変にしたほうが良いかも
		bacpy(&target_addr.l2_bdaddr, &target_bt_addr);
	}
	ret = connect(sock, (struct sockaddr *) &target_addr, sizeof(target_addr));
	if ((ret < 0) && (ret != EINPROGRESS)) {
//		printf ("connect(sock, (struct sockaddr *) &target_addr, sizeof(target_addr)) was failed. errno = %d\n", errno);
		return -errno;
	}
	else if(ret == EINPROGRESS) {
		//J select()を使って待つ
		fd_set fds, fds_reserve;
		FD_SET(sock, &fds_reserve);

		memcpy(&fds, &fds_reserve, sizeof(fd_set));

		//J Write が出来るようになるまで待つ
		ret = select (sock+1, NULL, &fds, NULL, NULL);
		if (ret < 0) {
			return -errno;
		}
		else if (ret == 0) {
			return AKS_ERROR_IO;
		}

		int error = 0;
		socklen_t len = 0;
		ret = getsockopt(sock, SOL_SOCKET, SO_ERROR, (void *)&error, &len);
		if (ret < 0) {
			return -errno;
		}

		if (error != 0) {
			return error;
		}
	}

	return sock;
}


/*---------------------------------------------------------------------------*/
static void *_ble_receive_thread_func(void *arg)
{
	BtGattDeviceContext *ctx = (BtGattDeviceContext*)arg;
	uint8_t data[BT_ATT_MAX_PDU_SIZE];

	while (1) {
		ssize_t read_size = read (ctx->btdevice, data, sizeof(data));
		if (read_size > 0) {
			BtAttPdu *_pdu = (BtAttPdu *)data;

			//J if notification, check the list of notification callback
			if (BtAttPduOpcode::cAttOpcodeHandleValueNotification == data[0] ){
				for (int i=0 ; i<ctx->num_notification ; ++i) {
					if (ctx->notification_list[i].value_handle == 
						_pdu->pdu.args.handleValueNotification.handle) {
						ctx->notification_list[i].cb( _pdu->pdu.args.handleValueNotification.value, read_size - 3); //TODO
					}
				}
			}
			//J 現在待ちになっているOPコードを見つけたらCBする
			else if (ctx->expectedResponseOpcode == data[0]) {
				memcpy (ctx->read_buf, data, read_size);
				ctx->read_size = read_size;
				ctx->expectedResponseOpcode = 0x00;
				ctx->read_error = AKS_OK;

				int ret = pthread_cond_signal(&ctx->blockWaitCv);
				if (ret != 0) {
					continue;
				}
			}
			//J 待っているレスポンスがエラーで帰ってきた場合
			else if (BtAttPduOpcode::cAttOpcodeErrorResponse == data[0]) {
				BtAttPdu *pdu = (BtAttPdu *)data;
				if (pdu->pdu.args.errorResponse.request_opcode == ctx->requestedOpcode) {
					uint8_t  error_opcode = 0;
					uint16_t error_handle = 0;
					uint8_t  error_status = 0;
					(void)btAttParsePduErrorResponse(
								data,
								read_size,
								error_opcode,
								error_handle,
								error_status);
					ctx->expectedResponseOpcode = 0x00;
					ctx->read_error = AKS_ERROR_BT_ATT_ERROR | error_status;

					int ret = pthread_cond_signal(&ctx->blockWaitCv);
					if (ret != 0) {
						continue;
					}
				}
				else{
				}
			}
			//J それ以外は捨てる
			else {
			}
		}
		else if (read_size != 0) {
			break;
		}
	}

	pthread_exit(NULL);
	return NULL;
}

