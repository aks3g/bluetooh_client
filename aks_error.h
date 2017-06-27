/*
 * Copyright 2016 Kiyotaka Akasaka
 *
 * Released under the MIT license
 * http://opensource.org/licenses/mit-license.php
 */
#ifndef AKS_ERROR_H_
#define AKS_ERROR_H_

#ifndef AKS_OK
#define AKS_OK								(0)
#endif

#define AKS_ERROR_NULL						(0xC0000001)
#define AKS_ERROR_INVALID					(0xC0000002)
#define AKS_ERROR_NOBUF						(0xC0000003)
#define AKS_ERROR_IO						(0xC0000004)
#define AKS_ERROR_NOT_IMPLEMENTED			(0xC0000005)
#define AKS_ERROR_FULL						(0xC0000006)

#define AKS_ERROR_BT_INVALID_UUID			(0xC0010001)
#define AKS_ERROR_BT_INCORRECT_PDU_SIZE		(0xC0010002)
#define AKS_ERROR_BT_INVALID_OPCODE			(0xC0010003)
#define AKS_ERROR_BT_INCLUDE_FRAGMENTS		(0xC0010004)
#define AKS_ERROR_BT_INVALUD_FORMAT			(0xC0010005)
#define AKS_ERROR_BT_ERROR_RESPONSE			(0xC0010006)
#define AKS_ERROR_BT_IMCOMPATIBLE_UUID		(0xC0010007)
#define AKS_ERROR_BT_UNEXPECTED_RESPONSE	(0xC0010008)
#define AKS_ERROR_BT_IMCOMPLETED_WRITE		(0xC0010009)

#define AKS_ERROR_BT_ATT_ERROR				(0xC0010100)


#endif/*AKS_ERROR_H_*/
