/**
 ******************************************************************************
 * @file    main.c
 * @author  CS application team
 * @brief   STSAFE-A120 Symmetric key establishment compute AES-128 CMAC example
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

/* Host keys used to open the host session (stored in platform secure storage at runtime) */
const stse_aes_key_t host_mac_key = {
	.type = STSE_AES_128_KT,
	.usage = STSE_AES_KEY_USAGE_MAC,
	.key = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF}
};

const stse_aes_key_t host_cipher_key = {
	.type = STSE_AES_128_KT,
	.usage = STSE_AES_KEY_USAGE_CIPHER,
	.key = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}
};

/**
 * @brief  Main program entry point - STSAFE-A120 AES-128 CMAC key establishment
 * @details Demonstrates symmetric key establishment and CMAC operations:
 *          - Establishes host secure session for protected communication
 *          - Generates ephemeral ECC key pair on host
 *          - Performs ECDH key agreement with STSAFE-A120
 *          - Derives AES-128 key and provisions to symmetric key slot
 *          - Computes CMAC (Cipher-based MAC) on test message
 *          - Verifies CMAC using platform crypto library
 * @note   Key establishment via ECDH provides secure key derivation without
 *         transmitting the key. CMAC is used for message authentication.
 * @retval Not applicable (infinite loop on success or error)
 */
int main(void)
{
	stse_return_code_t stse_ret = STSE_API_INVALID_PARAMETER;
	stse_handle_t stse_handle;

	PLAT_UI16 symmetric_key_slot = 0;
	PLAT_UI8 message[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	PLAT_UI8 MAC[4] = {0};
	uint8_t mac_verification = 0;

	/* Initialize Terminal */
	apps_terminal_init(115200);

	/* - Print Example instruction on terminal */
	printf("-----------------------------------------------------------------------------------------------------");
	printf("\n\r-                   STSAFE-A120 Symmetric key cryptography and key establishment example             -");
	printf("\n\r-                                              AES 128 CMAC                                         -");
	printf("\n\r-----------------------------------------------------------------------------------------------------");
	printf("\n\r-                                                                                                   -");
	printf("\n\r- description :                                                                                     -");
	printf("\n\r- This examples illustrates how to provision symmetric keys in STSAFE-A120 secure element            -");
	printf("\n\r- and perform AES cryptographic operations                                                          -");
	printf("\n\r-                                                                                                   -");
	printf("\n\r- Note : Device with accessible symmetric key (i.e. STSAFE-A120 SPL05) is required to run this example -");
	printf("\n\r-        Please run \"STSAFE-A120 Configure Symmetric key slot provisioning control fields example\"   -");
	printf("\n\r-        to activate symmetric key slot provisioning on SPL05 devices                                -");
	printf("\n\r-----------------------------------------------------------------------------------------------------");

	/* ## Initialize STSAFE-A1xx device handler */
	stse_ret = stse_set_default_handler_value(&stse_handle);
	if (stse_ret != STSE_OK)
	{
		printf(PRINT_RED "\n\r ## stse_set_default_handler_value ERROR : 0x%04X\n\r", stse_ret);
		apps_process_error(stse_ret);
	}

	stse_handle.device_type = STSAFE_A120;
	stse_handle.io.bus_type = STSE_BUS_TYPE_I2C;
	stse_handle.io.devaddr = 0x20;
	stse_handle.io.bus_speed = 400;

#ifdef STSE_CONF_USE_STATIC_PERSONALIZATION_INFORMATIONS
	stse_handle.perso_info = stsafe_a120_spl05_perso_info;
#endif

	printf("\n\r - Initialize target STSAFE-A120");
	stse_ret = stse_init(&stse_handle, NULL);
	if (stse_ret != STSE_OK)
	{
		printf(PRINT_RED "\n\r ## stse_init ERROR : 0x%04X\n\r", stse_ret);
		apps_process_error(stse_ret);
	}

	printf("\n\n\r");
	/* - Display key table information in serial terminal */
	apps_print_symmetric_key_table_info(&stse_handle);

	printf(PRINT_BOLD PRINT_ITALIC "\n\n\r==> Enter the slot number to write : ");
	apps_terminal_read_unsigned_integer(&symmetric_key_slot);

	/* - Establish test symmetric key in slot SYMMETRIC_KEY_SLOT */
	PLAT_UI8 p_aes_cmac_key[STSE_AES_128_KEY_SIZE];

	stsafea_generic_key_information_t aes_cmac_128_key_info = {0};
	aes_cmac_128_key_info.info_length = STSAFEA_KEY_INFO_LENGTH_CMAC;
	aes_cmac_128_key_info.lock_indicator = 0;
	aes_cmac_128_key_info.slot_number = (PLAT_UI8)symmetric_key_slot;
	aes_cmac_128_key_info.type = STSAFEA_SYMMETRIC_KEY_TYPE_AES_128;
	aes_cmac_128_key_info.mode_of_operation = STSAFEA_KEY_OPERATION_MODE_CMAC;
	aes_cmac_128_key_info.usage.mac_generation = 1;
	aes_cmac_128_key_info.usage.mac_verification = 1;
	aes_cmac_128_key_info.cmac.min_mac_length = 2;

	printf(PRINT_RESET "\n\n\r - Establish symmetric key in slot %d", symmetric_key_slot);
	stse_ret = stse_establish_symmetric_key(
		&stse_handle,                 /* Pointer to STSE handle */
		STSE_ECC_KT_NIST_P_256,       /* ECC key type */
		1,                            /* Slot number */
		&aes_cmac_128_key_info,       /* Pointer to key information */
		p_aes_cmac_key                /* Pointer to output established key */
	);

	if (stse_ret != STSE_OK)
	{
		printf(PRINT_RED "\n\n\r - stse_establish_symmetric_key : ERROR 0x%04x", stse_ret);
		if (stse_ret == STSE_COMMAND_NOT_AUTHORIZED)
		{
			printf("\n\n\r - Control fields for selected symmetric key slot might not be configured");
		}
		apps_process_error(stse_ret);
	}
	else
	{
		printf(PRINT_GREEN "\n\n\r - Established key : \n\r");
		apps_print_hex_buffer(p_aes_cmac_key, STSE_AES_128_KEY_SIZE);
	}

	stse_session_t host_session_handler;
	stsafea_session_clear_context(&host_session_handler);

	printf(PRINT_RESET "\n\n\r - stse_open_host_session");
	stse_ret = apps_open_host_session(
		&stse_handle,             /* Pointer to STSE handle */
		&host_session_handler,    /* Pointer to host session */
		&host_mac_key,            /* Pointer to host MAC key */
		&host_cipher_key          /* Pointer to host cipher key */
	);

	if (stse_ret != STSE_OK)
	{
		printf(PRINT_RED " ERROR : 0x%04X", stse_ret);
		apps_process_error(stse_ret);
	}

	printf("\n\n\r - Plaintext Message : \n\r");
	apps_print_hex_buffer(message, 16);

	/* - Compute MAC on message */
	stse_ret = stsafea_cmac_hmac_compute(
		&stse_handle,        /* Pointer to target STSAFE device */
		symmetric_key_slot,  /* Symmetric key slot number for aes-cmac */
		message,             /* Pointer to MAC input message buffer */
		16,                  /* MAC input message buffer length */
		MAC,                 /* Pointer to MAC buffer */
		4                    /* Expected MAC length */
	);

	if (stse_ret != STSE_OK)
	{
		printf(PRINT_RED "\n\r - stse_aes_cmac_compute : ERROR 0x%04x", stse_ret);
		apps_process_error(stse_ret);
	}
	else
	{
		printf("\n\n\r - Encrypted Message MAC : \n\r");
		apps_print_hex_buffer(MAC, 4);
	}

	/* - Verify encrypted message MAC */
	stse_ret = stsafea_cmac_hmac_verify(
		&stse_handle,        /* Pointer to target STSAFE device */
		symmetric_key_slot,  /* Symmetric key slot number for aes-cmac */
		MAC,                 /* Pointer to MAC buffer */
		4,                   /* Expected MAC length */
		message,             /* Pointer to MAC input message buffer */
		16,                  /* MAC input message buffer length */
		&mac_verification    /* Verification byte */
	);

	if (stse_ret != STSE_OK)
	{
		printf(PRINT_RED "\n\r - stsafea_cmac_hmac_verify : ERROR 0x%04x", stse_ret);
	}
	else if (mac_verification == 0)
	{
		printf(PRINT_RED "\n\n\r - Encrypted Message MAC verification : FAIL");
	}
	else
	{
		printf(PRINT_GREEN "\n\n\r - Encrypted Message MAC verification : PASS");
	}

	stsafea_close_host_session(&host_session_handler);

	while (1)
	{
		// infinite loop
	}

	return 0;
}
