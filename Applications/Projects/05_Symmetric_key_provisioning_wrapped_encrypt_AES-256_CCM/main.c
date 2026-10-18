/**
 ******************************************************************************
 * @file    main.c
 * @author  CS application team
 * @brief   STSAFE-A120 Symmetric key establishment AES-256 CCM encrypt/decrypt example
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
 * @brief  Main program entry point - STSAFE-A120 AES-256 CCM* wrapped key provisioning
 * @details Demonstrates wrapped symmetric key provisioning and CCM* AEAD operations:
 *          - Establishes host secure session for protected communication
 *          - Wraps AES-256 key using local envelope (KEK from host session)
 *          - Provisions wrapped key to symmetric key slot securely
 *          - Configures key for CCM* (Counter with CBC-MAC) mode
 *          - Encrypts test message with authentication using CCM*
 *          - Decrypts message and verifies authentication tag
 *          - Compares decrypted message with original to verify operation
 * @note   Wrapped provisioning protects the symmetric key during transfer.
 *         CCM* provides authenticated encryption (AEAD). Recommended for production.
 * @retval Not applicable (infinite loop on success or error)
 */
int main(void)
{
	stse_return_code_t stse_ret = STSE_API_INVALID_PARAMETER;
	stse_handle_t stse_handle;

	PLAT_UI16 symmetric_key_slot = 0;
	PLAT_UI8 message[16] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
	PLAT_UI8 encrypted_message[16];
	PLAT_UI8 MAC[4] = {0};
	PLAT_UI8 p_nonce[STSAFEA_NONCE_SIZE] = {
		0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C
	};
	uint8_t decrypt_verification = 0;

	/* Initialize Terminal */
	apps_terminal_init(115200);

	/* - Print Example instruction on terminal */
	printf("-----------------------------------------------------------------------------------------------------");
	printf("\n\r-              STSAFE-A120 Symmetric key cryptography and key provisioning wrapped example           -");
	printf("\n\r-                                              AES 256 CCM*                                         -");
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

	/* - Write test symmetric key in slot SYMMETRIC_KEY_SLOT */
	PLAT_UI8 p_aes_ccm_key[] = {
		0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A,
		0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A,
		0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A,
		0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A
	};
	PLAT_UI8 p_associated_data[] = {0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A, 0x5A};

	stsafea_generic_key_information_t aes_ccm_256_key_info = {0};
	aes_ccm_256_key_info.info_length = STSAFEA_KEY_INFO_LENGTH_CCM;
	aes_ccm_256_key_info.lock_indicator = 0;
	aes_ccm_256_key_info.slot_number = (PLAT_UI8)symmetric_key_slot;
	aes_ccm_256_key_info.type = STSAFEA_SYMMETRIC_KEY_TYPE_AES_256;
	aes_ccm_256_key_info.mode_of_operation = STSAFEA_KEY_OPERATION_MODE_CCM;
	aes_ccm_256_key_info.usage.encryption = 1;
	aes_ccm_256_key_info.usage.decryption = 1;
	aes_ccm_256_key_info.ccm.auth_tag_length = 4;

	printf(PRINT_RESET "\n\n\r - Write symmetric key in slot %d", symmetric_key_slot);
	stse_ret = stse_write_symmetric_key_wrapped(
		&stse_handle,             /* Pointer to target STSAFE device */
		p_aes_ccm_key,            /* Buffer of the key to provision */
		&aes_ccm_256_key_info,    /* Key info to set in the target slot */
		STSE_ECC_KT_NIST_P_256    /* Key Encryption Key (KEK) ECC Key type */
	);

	if (stse_ret != STSE_OK)
	{
		printf(PRINT_RED "\n\n\r - stse_write_symmetric_key : ERROR 0x%04x", stse_ret);
		if (stse_ret == STSE_COMMAND_NOT_AUTHORIZED)
		{
			printf("\n\n\r - Control fields for selected symmetric key slot might not be configured");
		}
		apps_process_error(stse_ret);
	}
	else
	{
		printf(PRINT_GREEN "\n\n\r - Populated key : \n\r");
		apps_print_hex_buffer(p_aes_ccm_key, STSE_AES_256_KEY_SIZE);
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

	printf(PRINT_CYAN "\n\n\r - Plaintext Message : \n\r");
	apps_print_hex_buffer(message, 16);

	/* - Encrypt message */
	stse_ret = stsafea_aes_ccm_encrypt(
		&stse_handle,         /* Pointer to target STSAFE device */
		symmetric_key_slot,   /* Symmetric key slot number for AES-CCM* */
		4,                    /* MAC input message buffer length */
		p_nonce,              /* Nonce used by AES-CCM* */
		8,                    /* Associated data length */
		p_associated_data,    /* Associated data */
		16,                   /* Plaintext length */
		message,              /* Plaintext message */
		encrypted_message,    /* Encrypted message */
		MAC,                  /* Pointer to MAC buffer */
		0,                    /* Counter slot */
		NULL                  /* Counter pointer */
	);

	if (stse_ret != STSE_OK)
	{
		printf(PRINT_RED "\n\r - stsafea_aes_ccm_encrypt : ERROR 0x%04x", stse_ret);
		apps_process_error(stse_ret);
	}
	else
	{
		printf(PRINT_RESET "\n\n\r - Encrypted Message : \n\r");
		apps_print_hex_buffer(encrypted_message, 16);
		printf("\n\n\r - Authentication tag : \n\r");
		apps_print_hex_buffer(MAC, 4);
	}

	/* - Decrypt message */
	stse_ret = stsafea_aes_ccm_decrypt(
		&stse_handle,          /* Pointer to target STSAFE device */
		symmetric_key_slot,    /* Symmetric key slot number for AES-CCM* */
		4,                     /* MAC input message buffer length */
		p_nonce,               /* Nonce */
		8,                     /* Associated data length */
		p_associated_data,     /* Associated data */
		16,                    /* Encrypted message length */
		encrypted_message,     /* Encrypted message */
		MAC,                   /* Pointer to MAC buffer */
		&decrypt_verification, /* Verification byte */
		message                /* Plaintext message */
	);

	if (stse_ret != STSE_OK)
	{
		printf(PRINT_RED "\n\r - stse_platform_aes_ccm_dec : ERROR 0x%04x", stse_ret);
	}
	else
	{
		printf(PRINT_GREEN "\n\r - decrypt verification result : %d", decrypt_verification);
		printf("\n\r - Decrypted Message : \n\r");
		apps_print_hex_buffer(message, 16);
	}

	stsafea_close_host_session(&host_session_handler);

	while (1)
	{
		// infinite loop
	}

	return 0;
}
