/**
 ******************************************************************************
 * @file    main.c
 * @author  CS application team
 * @brief   STSAFE-A120 Device authentication multi steps example
 ******************************************************************************
 *           			COPYRIGHT 2022 STMicroelectronics
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component. If no LICENSE file comes
 * with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/

#include "Apps_utils.h"

/* Defines -------------------------------------------------------------------*/

/* Root CA key used for STSE-A SPL02/SPL03 */
#define CA_SELF_SIGNED_CERTIFICATE_01                                          \
  0x30, 0x82, 0x01, 0xA0, 0x30, 0x82, 0x01, 0x46, 0xA0, 0x03, 0x02, 0x01,      \
      0x02, 0x02, 0x01, 0x01, 0x30, 0x0A, 0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE,  \
      0x3D, 0x04, 0x03, 0x02, 0x30, 0x4F, 0x31, 0x0B, 0x30, 0x09, 0x06, 0x03,  \
      0x55, 0x04, 0x06, 0x13, 0x02, 0x4E, 0x4C, 0x31, 0x1E, 0x30, 0x1C, 0x06,  \
      0x03, 0x55, 0x04, 0x0A, 0x0C, 0x15, 0x53, 0x54, 0x4D, 0x69, 0x63, 0x72,  \
      0x6F, 0x65, 0x6C, 0x65, 0x63, 0x74, 0x72, 0x6F, 0x6E, 0x69, 0x63, 0x73,  \
      0x20, 0x6E, 0x76, 0x31, 0x20, 0x30, 0x1E, 0x06, 0x03, 0x55, 0x04, 0x03,  \
      0x0C, 0x17, 0x53, 0x54, 0x4D, 0x20, 0x53, 0x54, 0x53, 0x41, 0x46, 0x45,  \
      0x2D, 0x41, 0x20, 0x50, 0x52, 0x4F, 0x44, 0x20, 0x43, 0x41, 0x20, 0x30,  \
      0x31, 0x30, 0x1E, 0x17, 0x0D, 0x31, 0x38, 0x30, 0x37, 0x32, 0x37, 0x30,  \
      0x30, 0x30, 0x30, 0x30, 0x30, 0x5A, 0x17, 0x0D, 0x34, 0x38, 0x30, 0x37,  \
      0x32, 0x37, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x5A, 0x30, 0x4F, 0x31,  \
      0x0B, 0x30, 0x09, 0x06, 0x03, 0x55, 0x04, 0x06, 0x13, 0x02, 0x4E, 0x4C,  \
      0x31, 0x1E, 0x30, 0x1C, 0x06, 0x03, 0x55, 0x04, 0x0A, 0x0C, 0x15, 0x53,  \
      0x54, 0x4D, 0x69, 0x63, 0x72, 0x6F, 0x65, 0x6C, 0x65, 0x63, 0x74, 0x72,  \
      0x6F, 0x6E, 0x69, 0x63, 0x73, 0x20, 0x6E, 0x76, 0x31, 0x20, 0x30, 0x1E,  \
      0x06, 0x03, 0x55, 0x04, 0x03, 0x0C, 0x17, 0x53, 0x54, 0x4D, 0x20, 0x53,  \
      0x54, 0x53, 0x41, 0x46, 0x45, 0x2D, 0x41, 0x20, 0x50, 0x52, 0x4F, 0x44,  \
      0x20, 0x43, 0x41, 0x20, 0x30, 0x31, 0x30, 0x59, 0x30, 0x13, 0x06, 0x07,  \
      0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x02, 0x01, 0x06, 0x08, 0x2A, 0x86, 0x48,  \
      0xCE, 0x3D, 0x03, 0x01, 0x07, 0x03, 0x42, 0x00, 0x04, 0x82, 0x19, 0x4F,  \
      0x26, 0xCC, 0xA3, 0x6E, 0x0E, 0x82, 0x19, 0x5C, 0xE6, 0x66, 0x58, 0xEC,  \
      0x64, 0xA4, 0x66, 0x92, 0x2F, 0x58, 0xC9, 0xE6, 0x4B, 0x5D, 0xE1, 0xA2,  \
      0x9E, 0x7F, 0x39, 0x86, 0x3D, 0x04, 0x26, 0x92, 0xE4, 0xC8, 0xAC, 0x79,  \
      0xF9, 0x6D, 0x2F, 0xED, 0x52, 0x77, 0x4D, 0x52, 0x81, 0x95, 0x39, 0xF2,  \
      0x1F, 0x3E, 0xCD, 0x19, 0x38, 0xF8, 0x3D, 0x70, 0xAE, 0xE0, 0x9C, 0xCD,  \
      0x8D, 0xA3, 0x13, 0x30, 0x11, 0x30, 0x0F, 0x06, 0x03, 0x55, 0x1D, 0x13,  \
      0x01, 0x01, 0xFF, 0x04, 0x05, 0x30, 0x03, 0x01, 0x01, 0xFF, 0x30, 0x0A,  \
      0x06, 0x08, 0x2A, 0x86, 0x48, 0xCE, 0x3D, 0x04, 0x03, 0x02, 0x03, 0x48,  \
      0x00, 0x30, 0x45, 0x02, 0x20, 0x6E, 0xE5, 0x43, 0x32, 0x47, 0xAC, 0x72,  \
      0x34, 0xFC, 0x9D, 0x17, 0x5A, 0xA5, 0x1E, 0x83, 0x27, 0x69, 0x01, 0xAD,  \
      0xEC, 0x1F, 0x00, 0x5E, 0x37, 0x1F, 0x40, 0x73, 0x4D, 0xE3, 0x8C, 0xC5,  \
      0x2E, 0x02, 0x21, 0x00, 0xB1, 0xD9, 0x51, 0x6A, 0xAD, 0x9A, 0x3E, 0x86,  \
      0xD2, 0x2B, 0x8E, 0x3B, 0x3B, 0xD0, 0x14, 0x6F, 0xAB, 0xB9, 0xB9, 0x22,  \
      0xF0, 0x45, 0x26, 0x34, 0xFE, 0x92, 0x7F, 0xF5, 0xD6, 0x36, 0xCD, 0x90

/* Target device leaf-certificate zone*/
#define STSAFE_CERTIFICATE_ZONE_0 0U

/* Target device private key slot associated to leaf-certificate*/
#define STSE_STATIC_PRIVATE_KEY_SLOT_0 0U


/**
 * brief  Main program entry point - STSAFE-A120 Multi-step device authentication
 * details Demonstrates detailed device authentication process with individual
 * steps:
 *          - Reads device certificate from STSAFE-A120 zone 0
 *          - Parses both CA and device certificates into structured format
 *          - Extracts and displays certificate information (subject, issuer, keys)
 *          - Verifies device certificate signature using CA public key
 *          - Generates random challenge for device
 *          - Requests device signature on challenge using private key
 *          - Verifies signature using device's public key from certificate
 * note   This multi-step approach shows each authentication phase separately,
 *         useful for understanding the authentication protocol or implementing
 *         custom authentication flows for distant server scenarios
 * retval : 0 on success (never reached due to infinite loop)
 */
int main(void) {
	stse_return_code_t stse_ret = STSE_API_INVALID_PARAMETER;
	stse_handle_t stse_handle;
	PLAT_UI16 certificate_size;
	stse_certificate_t parsed_stse_certificate;
	stse_certificate_t parsed_ca_selfsigned_cert;
	static const uint8_t ca_selfsigned_cert[] = {CA_SELF_SIGNED_CERTIFICATE_01};


	/* Initialize Terminal */
  	apps_terminal_init(115200);

  	/* Print Example instruction on terminal */
  	printf(PRINT_CLEAR_SCREEN);
  	printf(    "-----------------------------------------------------------------------------------------------------");
  	printf("\n\r-                                       STSAFE-A120                                                 -");
  	printf("\n\r-                             multi-step device authentication                                      -");
  	printf("\n\r-----------------------------------------------------------------------------------------------------");
    printf("\n\r- This example illustrates STSAFE-A120 device authentication process using Multi-Step approach.     -");
    printf("\n\r- it can be taken as reference for building distant server authentication use cases.                -");
  	printf("\n\r-----------------------------------------------------------------------------------------------------");

  	/* Initialize device handle */
  	stse_ret = stse_set_default_handler_value(&stse_handle);

  	/* Verify correct handle initialization */
  	if (stse_ret != STSE_OK) {
  		printf(PRINT_RED "\n\r ## stse_set_default_handler_value ERROR : 0x%04X\n\r",stse_ret);
  		apps_process_error(stse_ret);
  	}

  	/* Configure handle for target STSE device */
  	stse_handle.device_type = STSAFE_A120; 			/* Tagret device type : STSAFE-A120 */
  	stse_handle.io.bus_type = STSE_BUS_TYPE_I2C;	/* Target device bus type : I2C */
  	stse_handle.io.devaddr = 0x20;					/* Target device address : 0x20 */
  	stse_handle.io.bus_speed = 400;					/* Communication speed : 400kHz */
  	#ifdef STSE_CONF_USE_STATIC_PERSONALIZATION_INFORMATIONS
  	stse_handle.perso_info = stsafe_a120_spl05_perso_info; /* use static device perso info */
  	#endif

  	/* Initialize stselib for target STSE device */
  	printf("\n\n\r ## Initialize library for target STSAFE-A120");
  	stse_ret = stse_init(&stse_handle, NULL);

  	/* Verify stselib initialization */
  	if (stse_ret != STSE_OK) {
  		printf(PRINT_RED "\n\r ## stse_init ERROR : 0x%04X\n\r", stse_ret);
  		apps_process_error(stse_ret);
  	}

	/* Parse root CA certificate */
	stse_ret = stse_certificate_parse(
		  ca_selfsigned_cert,			/* Pointer to root CA DER format buffer */
		  &parsed_ca_selfsigned_cert,	/* Pointer to root CA parsed structure */
		  NULL							/* No chained certificate*/
	);

	/* Verify certificate parsing status */
	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r## ST CA stse_certificate_parse ERROR : 0x%04X",stse_ret);
		apps_process_error(stse_ret);
	}

	/* Print parsed root CA certificate */
	printf("\n\n\r## CA self-signed certificate : \n\r");
	stse_certificate_print_parsed_cert(&parsed_ca_selfsigned_cert);

	/* Get target STSE device leaf-certificate size */
	stse_ret = stse_get_device_certificate_size(
			&stse_handle,				/* Pointer to STSE handle */
			STSAFE_CERTIFICATE_ZONE_0,	/* Target leaf-certificate storage zone*/
			&certificate_size			/* Pointer to certificate size variable*/
	);

	/* Verify get certificate size execution status */
	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r## stse_get_device_certificate_size ERROR : 0x%04X",stse_ret);
		apps_process_error(stse_ret);
	}

	if (certificate_size == 0U || certificate_size > STSE_MAX_DEVICE_CERTIFICATE_SIZE) {
		printf(PRINT_RED "\n\n\r## certificate_size out of bounds : %u", certificate_size);
		apps_process_error(STSE_API_INVALID_PARAMETER);
	}

	/* Allocate leaf-certificate buffer in memory */
	PLAT_UI8 stse_certificate[STSE_MAX_DEVICE_CERTIFICATE_SIZE];

	/* Get target STSE device leaf-certificate size */
	stse_ret = stse_get_device_certificate(
			  &stse_handle,					/* Pointer to STSE handle */
			  STSAFE_CERTIFICATE_ZONE_0,	/* Target leaf-certificate storage zone */
			  certificate_size,				/* leaf-certificate size */
			  stse_certificate				/* pointer to leaf-certificate buffer */
	);

	/* Verify get STSE leaf-certificate execution status */
	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r## stse_get_device_certificate ERROR : 0x%04X",stse_ret);
		apps_process_error(stse_ret);
	}

	/* Parse target STSE leaf-certificate */
	stse_ret = stse_certificate_parse(
		stse_certificate,
		&parsed_stse_certificate,
		NULL
	);

	/* Verify */
	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r## STSE stse_certificate_parse ERROR : 0x%04X",stse_ret);
		apps_process_error(stse_ret);
	}

	/* Print parsed leaf-device certificate */
	printf("\n\n\r## Target STSAFE-Axxx certificate : \n\r");
	stse_certificate_print_parsed_cert(&parsed_stse_certificate);

	/* Verify STSE leaf-certificate signature with CA certificate public key */
	stse_ret = stse_certificate_is_parent(
			&parsed_ca_selfsigned_cert,
			&parsed_stse_certificate,
			NULL
	);


	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r## stse_certificate_is_parent ERROR : 0x%04X",stse_ret);
		apps_process_error(stse_ret);
	} else {
		printf("\n\n\r## Device Certificate Verified\n\r");
	}

	/* ## Extract certificate key type */
	stse_ecc_key_type_t key_type = stse_certificate_get_key_type(&parsed_stse_certificate);
	PLAT_UI16 signature_size = stse_ecc_info_table[key_type].signature_size;
	PLAT_UI8 signature[signature_size];

	/* ## Generate a challenge ( aligned with key type size requirement) */
	printf("\n\n\r## Host random challenge : \n\r");
	PLAT_UI16 challenge_size = stse_ecc_info_table[key_type].private_key_size;
	PLAT_UI8 challenge[challenge_size];
	apps_randomize_buffer(challenge, challenge_size);
	apps_print_hex_buffer(challenge, challenge_size);

	/* ##  Signature over challenge */
	stse_ret = stsafea_ecc_generate_signature(
		&stse_handle,                   /* Pointer to STSE handle */
		STSE_STATIC_PRIVATE_KEY_SLOT_0, /* Slot number */
		key_type,                       /* Certificate key type */
		challenge,                      /* Challenge */
		challenge_size,                 /* Challenge size */
		signature                       /* Challenge signature */
	);

	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\r## stsafea_ecc_generate_signature ERROR : 0x%04X", stse_ret);
		apps_process_error(stse_ret);
	}
	printf("\n\n\r## Device signature over Host challenge: ");
	apps_print_hex_buffer(signature, signature_size);

	/* ## Verify The Signature of the Random Number */
	stse_ret = stse_certificate_verify_signature(
		&parsed_stse_certificate,
		challenge,
		challenge_size,
		signature,
		(signature_size >> 1),
		&signature[signature_size >> 1],
		(signature_size >> 1)
	);
	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r## stse_certificate_verify_signature ERROR : 0x%04X", stse_ret);
	} else {
		printf(PRINT_GREEN "\n\n\r## Device Authenticated (Challenge signature verified successfully)");
	}

	printf(PRINT_RESET "\n\r\n\r*#*# STMICROELECTRONICS #*#*\n\r");

	while (1) {
		/* Infinite loop */
	}

	return 0;
}
