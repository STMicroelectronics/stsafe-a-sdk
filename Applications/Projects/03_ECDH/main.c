/**
 ******************************************************************************
 * @file    main.c
 * @author  CS application team
 * @brief   STSAFE-A120 ECDH example
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

#define ENCRYPT_NO  0x00
#define ENCRYPT_RSP 0x01

/**
 * @brief  Main program entry point - STSAFE-A120 ECDH example
 * @details Demonstrates ECDH key exchange:
 *          - Generates ECC key pair on STSAFE-A120
 *          - Opens host session to protect communication
 *          - Establishes shared secret with host public key
 * @retval Not applicable (infinite loop)
 */
int main(void)
{
	stse_return_code_t stse_ret = STSE_API_INVALID_PARAMETER;
	stse_handle_t stse_handle;
	stse_session_t host_session_handler;
	stse_cmd_access_conditions_t establish_key_protection = STSE_CMD_AC_NEVER;
	PLAT_UI8 establish_key_rsp_encryption_flag = ENCRYPT_NO;

	stse_ecc_key_type_t key_type = STSE_ECC_KT_NIST_P_256;
	static uint8_t host_public_key[] = { // NIST P-256 host public key
		0xe2, 0xaa, 0x79, 0x20, 0x16, 0x90, 0xd8, 0x50, 0x3f, 0x79, 0x2b, 0xb8, 0xf5, 0x14, 0x67, 0x3d,
		0x8c, 0x88, 0x85, 0x8a, 0x82, 0xc2, 0xb8, 0xd1, 0xb7, 0xe8, 0x14, 0x53, 0x23, 0x0e, 0xac, 0xbb,
		0x7d, 0x25, 0xeb, 0x2c, 0x09, 0xf4, 0xb0, 0x67, 0x20, 0x35, 0xac, 0x0d, 0x14, 0x1e, 0xe6, 0xd1,
		0x42, 0x20, 0x1c, 0x21, 0xef, 0xee, 0xb7, 0x87, 0x98, 0xdf, 0xe8, 0x76, 0xae, 0x7d, 0x1a, 0x07
	};
	uint8_t slot_number = 0xFF; // 0xFF: ephemeral key slot(ECDHE), 0x01 to 0x04 : static key slot (ECDH)

	/* - allocate ECDHE buffer sizes according to ECC key type */
	PLAT_UI16 pub_key_size = stse_ecc_info_table[key_type].public_key_size;
	PLAT_UI8 stse_public_key[pub_key_size];
	PLAT_UI16 shared_secret_size = stse_ecc_info_table[key_type].shared_secret_size;
	PLAT_UI8 shared_secret[shared_secret_size];

	/* Initialize Terminal */
	apps_terminal_init(115200);

	/* Print Example instruction on terminal */
	printf(PRINT_CLEAR_SCREEN);
	printf("-----------------------------------------------------------------------------------------------------");
	printf("\n\r-                                       STSAFE-A ECDH example                                       -");
	printf("\n\r-----------------------------------------------------------------------------------------------------");
	printf("\n\r- This example illustrates STSAFE-A120 ECDH process.                                                -");
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

	/* ## Command config through perso */
	stsafea_perso_info_get_cmd_ac(&stse_handle.perso_info, STSAFEA_CMD_ESTABLISH_KEY, &establish_key_protection);
	stsafea_perso_info_get_rsp_encrypt_flag(&stse_handle.perso_info, STSAFEA_CMD_ESTABLISH_KEY, &establish_key_rsp_encryption_flag);

	if (establish_key_protection < STSE_CMD_AC_HOST || establish_key_rsp_encryption_flag == ENCRYPT_NO)
	{
		printf(PRINT_CYAN "\n\n\n\n\n\n\r-----------------------------------------------------------------------------------------------------");
		printf("\n\r-                                          !!!!! WARNING !!!!!                                      -");
		printf("\n\r-----------------------------------------------------------------------------------------------------");
		printf("\n\r-           To avoid any leakage of secret through communication bus during establish key process,   -");
		printf("\n\r-                        it's recommended to provision command access condition                     -");
		printf("\n\r-----------------------------------------------------------------------------------------------------\n\n\r" PRINT_RESET);
	}

	/* ## Generate ECC key pair on STSE */
	stse_ret = stsafea_generate_ecc_key_pair(
		&stse_handle,       /* Pointer to STSE handle */
		slot_number,        /* Slot number */
		key_type,           /* ECC key type */
		1,                  /* Generate private key */
		stse_public_key     /* Output buffer for public key */
	);
	if (stse_ret != STSE_OK)
	{
		/* Clear key pairs info on SE ECDHE key pair generation error  */
		memset(host_public_key, 0, sizeof(host_public_key));
		printf(PRINT_RED "\n\r ## stsafea_generate_ecc_key_pair ERROR : 0x%04X\n\r", stse_ret);
		apps_process_error(stse_ret);
	}

	/* ## Open Host Session (required for protecting sensitive exchanges over I2C between host and STSAFE-A) */
	stsafea_session_clear_context(&host_session_handler);

	stse_ret = apps_open_host_session(
		&stse_handle,             /* Pointer to STSE handle */
		&host_session_handler,    /* Pointer to host session */
		&host_mac_key,            /* Pointer to host MAC key */
		&host_cipher_key          /* Pointer to host cipher key */
	);
	if (stse_ret != STSE_OK)
	{
		printf(PRINT_RED "\n\r - apps_open_host_session ERROR : 0x%04X", stse_ret);
		if (stse_ret == STSE_SERVICE_SESSION_ERROR)
		{
			printf("\n\r - Host keys not populated through STSE");
		}
		apps_process_error(stse_ret);
	}

	/* ## Establish a shared secret on STSE */
	stse_ret = stsafea_ecc_establish_shared_secret(
		&stse_handle,       /* Pointer to STSE handle */
		slot_number,        /* Slot number */
		key_type,           /* ECC key type */
		host_public_key,    /* Host public key buffer */
		shared_secret       /* Output buffer for shared secret */
	);
	if (stse_ret != STSE_OK)
	{
		/* Clear key pairs info on SE ECDHE key pair generation error  */
		memset(host_public_key, 0, sizeof(host_public_key));
		printf(PRINT_RED "\n\r ## stsafea_ecc_establish_shared_secret ERROR : 0x%04X\n\r", stse_ret);
		apps_process_error(stse_ret);
	}
	else
	{
		printf(PRINT_GREEN "\n\n\r\t - Shared secret:");
		apps_print_hex_buffer(shared_secret, sizeof(shared_secret));
	}

	stsafea_close_host_session(&host_session_handler);

	while (1)
	{
		// infinite loop
	}

	return 0;
}
