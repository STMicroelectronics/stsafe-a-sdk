/**
 ******************************************************************************
 * @file    main.c
 * @author  CS application team
 * @brief   STSAFE-A120 Random number generation example
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
 * @brief  Main program entry point - STSAFE-A120 Random number generation
 * example
 * @details Demonstrates random number generation:
 *          - Initializes STSAFE-A120 device
 *          - Generates 64 bytes of random data using STSAFE-A120 TRNG
 *          - Displays the generated random bytes in hexadecimal format
 * @retval Not applicable (infinite loop on success or error)
 */
int main(void) {
  stse_return_code_t stse_ret = STSE_API_INVALID_PARAMETER;
  stse_handle_t stse_handle;

  PLAT_UI16 random_size = 64;
  PLAT_UI8 p_random[random_size];

	/* Initialize Terminal */
	apps_terminal_init(115200);

	/* - Print Example instruction on terminal */
	printf(PRINT_CLEAR_SCREEN);
	printf(    "-----------------------------------------------------------------------------------------------------");
	printf("\n\r-                                       STSAFE-A120                                                 -");
	printf("\n\r-                             Random number generation example                                      -");
	printf("\n\r-----------------------------------------------------------------------------------------------------");
	printf("\n\r- This examples illustrates how to use generate random commands of a target STSAFE-A120 device      -");
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

	/* Call STSE "Generate random" API */
	stse_ret = stsafea_generate_random(
		&stse_handle,                               /* Pointer to STSE handle */
		p_random,                                   /* Output random buffer */
		random_size                                 /* Requested random size */
	);

	if (stse_ret != STSE_OK) {
		printf("\n\n\r - stsafea_generate_random ERROR : 0x%04X", stse_ret);
	} else {
		printf("\n\n\r - stsafea_generate_random (%d bytes) : \n\r", random_size);
		apps_print_hex_buffer(p_random, random_size);
	}

	printf(PRINT_RESET "\n\r\n\r*#*# STMICROELECTRONICS #*#*\n\r");

	while (1) {
		/* - Infinite loop */
	}

	return 0;
}
