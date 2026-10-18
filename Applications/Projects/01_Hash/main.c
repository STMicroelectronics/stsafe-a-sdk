/**
 ******************************************************************************
 * @file    main.c
 * @author  CS application team
 * @brief   STSAFE-A120 Hash example
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
 * @brief  Main program entry point - STSAFE-A120 Hash example
 * @details Demonstrates hash computation functionality:
 *          - Generates a random 128-byte message
 *          - Computes SHA-256 hash using platform crypto library
 *          - Computes SHA-256 hash using STSAFE-A120 device
 *          - Compares both hash values to verify correctness
 * @retval Not applicable (infinite loop on success or error)
 */
int main(void) {
  stse_return_code_t stse_ret = STSE_API_INVALID_PARAMETER;
  stse_handle_t stse_handle;

  PLAT_UI16 message_length = 128;
  PLAT_UI8 message_to_hash[message_length];

  stse_hash_algorithm_t hash_algo = STSE_SHA_256;
  PLAT_UI16 hash_length = stsafea_hash_info_table[hash_algo].length;
  PLAT_UI8 hash_platform[hash_length];
  PLAT_UI8 hash_stsafea[hash_length];

	/* Initialize Terminal */
	apps_terminal_init(115200);

	/* - Print Example instruction on terminal */
	printf(PRINT_CLEAR_SCREEN);
	printf(    "-----------------------------------------------------------------------------------------------------");
	printf("\n\r-                                       STSAFE-A120                                                 -");
	printf("\n\r-                                  hash commands example                                            -");
	printf("\n\r-----------------------------------------------------------------------------------------------------");
	printf("\n\r- description :                                                                                     -");
	printf("\n\r- This examples illustrates how to use hash commands of a target STSAFE-A120 device                 -");
	printf("\n\r-----------------------------------------------------------------------------------------------------");

	/* ## Initialize STSAFE-A1xx device handler */
	stse_ret = stse_set_default_handler_value(&stse_handle);
	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\r ## stse_set_default_handler_value ERROR : 0x%04X\n\r", stse_ret);
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
	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\r ## stse_init ERROR : 0x%04X\n\r", stse_ret);
		apps_process_error(stse_ret);
	}

	/* - Generate & Display random message */
	printf("\n\n\r - Message buffer to hash : \n\r");
	apps_randomize_buffer(message_to_hash, message_length);
	apps_print_hex_buffer(message_to_hash, message_length);

	/* - Hash message using platform */
	stse_ret = stse_platform_hash_compute(
		hash_algo,                                  /* Hash algorithm */
		message_to_hash,                            /* Input message */
		message_length,                             /* Message length */
		hash_platform,                              /* Output hash buffer */
		&hash_length                                /* Output hash length */
	);

	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r - stse_platform_hash_compute ERROR : 0x%04X", stse_ret);
	} else {
		printf("\n\n\r - stse_platform_hash_compute  : \n\r");
		apps_print_hex_buffer(hash_platform, hash_length);
	}

	/* - Hash message using STSAFE-A120 */
	stse_ret = stse_compute_hash(
		&stse_handle,                               /* Pointer to STSE handle */
		hash_algo,                                  /* Hash algorithm */
		message_to_hash,                            /* Input message */
		message_length,                             /* Message length */
		hash_stsafea,                               /* Output hash buffer */
		&hash_length                                /* Output hash length */
	);

	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r - stse_compute_hash ERROR : 0x%04X", stse_ret);
	} else {
		printf("\n\n\r - stse_compute_hash  : \n\r");
		apps_print_hex_buffer(hash_stsafea, hash_length);
	}

	/* - Check STSAFE-A hash against platform hash */
	if (memcmp(hash_platform, hash_stsafea, hash_length)) {
		printf(PRINT_RED "\n\n\r - memcmp ERROR");
	} else {
		printf(PRINT_GREEN "\n\n\r - HASH SUCCESS\n\r");
	}

	printf(PRINT_RESET "\n\r\n\r*#*# STMICROELECTRONICS #*#*\n\r");

	while (1) {
		/* Infinite loop */
	}

	return 0;
}
