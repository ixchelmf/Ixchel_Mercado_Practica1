/*
 * module_crcaes_ixchel.c
 *
 *  Created on: 19 feb 2022
 *      Author: Mercado
 */
#include "aes.h"
#include "fsl_crc.h"
#include "module_crcaes_ixchel.h"
#include "fsl_debug_console.h"
#include <string.h>

//prototypes
static void InitCrc32(CRC_Type *base, uint32_t seed);


uint8_t validation_module(void *dataptr, uint16_t len)
{
	/* CRC data */
	CRC_Type *base = CRC0;
	uint8_t  checksumlen=4, message_len = (uint8_t)len-checksumlen;
	uint32_t received_checksum, calculated_checksum;
	uint8_t message[message_len], messages_checkhsum[checksumlen];
	uint8_t *ptr_initchecksum = (uint8_t*)dataptr + message_len;
	uint8_t crcok,aesdone;

	//AES DATA
	uint8_t key[] = { 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
	uint8_t iv[]  = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
	struct AES_ctx ctx;


	PRINTF("\nTesting CRC32\r\n\n");

	 char *d = (char*)dataptr;
	 PRINTF("\nPointer:%c",(char*)dataptr);
	 for(int i= 0; i<(uint8_t)len;i++)
	 {
		 PRINTF("\nData:%c",*d);
		 *d++;
	 }
	//el primero memcpy es para tener sólo el mensaje, y hacerle el sumcheck
	memcpy(&message[0], (uint8_t*)dataptr, message_len);
	PRINTF("\nafter memcpy cutting first part of the message\r\n");
	for(int i= 0; i<message_len;i++)
	{
		PRINTF("\nData:%c",message[i]);
	}

	//el segundo memcpy va a ser para sacar el sumcheck enviado

	memcpy(&messages_checkhsum[0], ptr_initchecksum, (size_t)checksumlen);
	PRINTF("\nafter memcpy cutting second part of the message\r\n");
	received_checksum = messages_checkhsum[0] | (messages_checkhsum[1] << 8) | (messages_checkhsum[2] << 16) | (messages_checkhsum[3] << 24);
	PRINTF("\nProvided checksum: %d",received_checksum);
//	for(int i= 0; i<checksumlen;i++)
//	{
//		PRINTF("\nData:%c",messages_checkhsum[i]);
//	}

//
	InitCrc32(base, 0xFFFFFFFFU);
	PRINTF("\nCRC32 initiated...\r\n");
	CRC_WriteData(base, (uint8_t *)&message[0], (size_t)message_len); ///
	calculated_checksum = CRC_Get32bitResult(base);
	PRINTF("\nProvided checksum: %d",received_checksum);

	PRINTF("\n");
	PRINTF("\nCalculated checksum: %d\r\n",calculated_checksum);
	if(received_checksum==calculated_checksum)
	{
		crcok=1;
		PRINTF("\nChecksum is the same, there was no error on the transmission");
	}
	else
	{
		crcok=0;
		PRINTF("\nChecksum is not the same, there was an error on the transmission");
		return crcok;
	}
	////////////////AES//////////////////////////////


	if(crcok==1)
	{
		PRINTF("\nInitializing AES...\r\n");
		AES_init_ctx_iv(&ctx, key, iv);
		PRINTF("\nDecrypting...\r\n");
		AES_CBC_decrypt_buffer(&ctx,&message[0], message_len);
		PRINTF("\nDecrypted message:\r\n");
		for(int i= 0; i<message_len;i++)
			{
				PRINTF("%c",message[i]);
			}

		return crcok;
	}




}

static void InitCrc32(CRC_Type *base, uint32_t seed)
{
    crc_config_t config;

    config.polynomial         = 0x04C11DB7U;
    config.seed               = seed;
    config.reflectIn          = true;
    config.reflectOut         = true;
    config.complementChecksum = true;
    config.crcBits            = kCrcBits32;
    config.crcResult          = kCrcFinalChecksum;

    CRC_Init(base, &config);
}
