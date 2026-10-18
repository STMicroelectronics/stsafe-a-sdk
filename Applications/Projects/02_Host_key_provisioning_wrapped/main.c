/**
 ******************************************************************************
 * @file    main.c
 * @author  CS application team
 * @brief   STSAFE-A120 Wrapped host key provisioning example
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

/**
 * @brief  Main program entry point - STSAFE-A120 Host key provisioning wrapped
 * @details Demonstrates secure host key provisioning using wrapped envelope:
 *          - Generates ephemeral ECC key pair on host
 *          - Uses STSAFE-A120's public key for ECDH key agreement
 *          - Derives shared secret using ECDH
 *          - Wraps host keys using the shared secret
 *          - Provisions wrapped host keys to STSAFE-A120
 *          - Verifies keys by establishing a host session
 * @note   Wrapped provisioning provides secure key transfer without exposing
 *         keys in plaintext on the communication bus. Recommended for production.
 * @warning This example permanently provisions host keys. Use appropriate
 *          key management procedures for production deployments.
 * @retval Not applicable (infinite loop on success or error)
 */
int main(void)
{
	stse_return_code_t stse_ret = STSE_API_INVALID_PARAMETER;
	stse_handle_t stse_handle;
	stsafea_aes_128_host_keys_t host_keys = {
		.host_mac_key = {0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77, 0x88, 0x99, 0xAA, 0xBB, 0xCC, 0xDD, 0xEE, 0xFF},
		.host_cipher_key = {0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF, 0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF}
	};
	stsafea_host_key_provisioning_ctrl_fields_t provisioning_ctrl_fields;

	/* Initialize Terminal */
	apps_terminal_init(115200);

	/* - Print Example instruction on terminal */
	printf(" -----------------------------------------------------------------------------------------------------");
	printf("\n\r -                              STSAFE-A120 Host key provisioning wrapped example                    -");
	printf("\n\r -----------------------------------------------------------------------------------------------------");
	printf("\n\r -                                                                                                   -");
	printf("\n\r - description :                                                                                     -");
	printf("\n\r - This examples illustrates host keys provisioning of a target STSAFE-A120 device                    -");
	printf("\n\r -                                                                                                   -");
	printf("\n\r -----------------------------------------------------------------------------------------------------");

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

	/* - Display host keys */
	printf("\n\n\r - Host mac key to write : \n\r");
	apps_print_hex_buffer(host_keys.host_mac_key, STSAFEA_HOST_AES_128_MAC_KEY_SIZE);
	printf("\n\n\r - Host cipher key to write : \n\r");
	apps_print_hex_buffer(host_keys.host_cipher_key, STSAFEA_HOST_AES_128_CIPHER_KEY_SIZE);

	/* - Query host key provisioning control fields */
	printf("\n\n\r - Query host key slot provisioning control fields :");
	stse_ret = stsafea_query_host_key_provisioning_ctrl_fields(
		&stse_handle,                /* Pointer to STSE handle */
		&provisioning_ctrl_fields    /* Pointer to host key provisioning control fields */
	);
	if (stse_ret != STSE_OK)
	{
		printf(PRINT_RED "\n\n\r - stsafea_query_host_key_provisioning_ctrl_fields ERROR : 0x%04X", stse_ret);
		apps_process_error(stse_ret);
	}
	apps_print_host_key_provisioning_control_fields(&stse_handle);

	printf(PRINT_RED "\n\n\r +---------------------------------------------------------------------------------------------------+");
	printf("\n\r |                                                                                                   |");
	printf("\n\r |                                           /!\\  W A R N I N G  /!\\                                 |");
	printf("\n\r |                                                                                                   |");
	printf("\n\r | The program will write test keys in the target STSAFE-A120 and enable re-provisioning             |");
	printf("\n\r | It will add two following constraints:                                                            |");
	printf("\n\r |    - The host keys slot will be opened to re-provisioning forever and this is not suitable for    |");
	printf("\n\r |      production                                                                                   |");
	printf("\n\r |    - The target STSAFE-A120 will no longer support stse_host_key_provisioning_wrapped_authenticated |");
	printf("\n\r |                                                                                                   |");
	printf("\n\r | " PRINT_RESET PRINT_BOLD PRINT_ITALIC "Press any key to continue and write the host keys :" PRINT_RED "                                               |");
	printf("\n\r |                                                                                                   |");
	printf("\n\r +---------------------------------------------------------------------------------------------------+\n\r" PRINT_RESET);
	apps_terminal_read_string(NULL, NULL);

	if (provisioning_ctrl_fields.change_right == 0)
	{
		if (provisioning_ctrl_fields.reprovision == 0)
		{
			printf(PRINT_RED "\n\r - ERROR : Host keys are already locked and re-provisioning disabled");
			apps_process_error(stse_ret);
		}
		else
		{
			printf(PRINT_CYAN "\n\r - Host key provisioning control fields value already set" PRINT_RESET);
		}
	}
	else
	{
		printf("\n\n\r - Open host key slot to re-provisioning");
		provisioning_ctrl_fields.filler = 0;
		provisioning_ctrl_fields.change_right = 0;
		provisioning_ctrl_fields.reprovision = 1;
		provisioning_ctrl_fields.plaintext = 1;
		provisioning_ctrl_fields.wrapped_anonymous = 1;
		provisioning_ctrl_fields.wrapped_or_dh_derived_authentication_key = 0xFF;
		stse_ret = stsafea_put_host_key_provisioning_ctrl_fields(
			&stse_handle,                /* Pointer to STSE handle */
			&provisioning_ctrl_fields    /* Pointer to host key provisioning control fields */
		);
		if (stse_ret != STSE_OK)
		{
			printf(PRINT_RED "\n\n\r - stsafea_put_host_key_provisioning_ctrl_fields ERROR : 0x%04X", stse_ret);
		}
	}

	/* Host key provisioning */
	stse_ret = stse_host_key_provisioning_wrapped(
		&stse_handle,                         /* Pointer to STSE handle */
		STSAFEA_AES_128_HOST_KEY,             /* Host key type */
		(stsafea_host_keys_t *)&host_keys,    /* Pointer to host keys */
		STSE_ECC_KT_NIST_P_256                /* Key table / curve type */
	);
	if (stse_ret != STSE_OK)
	{
		printf(PRINT_RED "\n\n\r - stse_host_key_provisioning_wrapped ERROR : 0x%04X", stse_ret);
		while (1);
	}
	else
	{
		printf(PRINT_GREEN "\n\n\r - stse_host_key_provisioning_wrapped : PASS\n\r");
	}

	while (1)
	{
		/* - Infinite loop */
	}
}
