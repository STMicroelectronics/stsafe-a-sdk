/**
 ******************************************************************************
 * @file    main.c
 * @author  CS application team
 * @brief   STSAFE-A120 Secure data storage access example
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
#define READ_BUFFER_SIZE 100 /**< Size of read buffer for zone data */
#define RANDOM_SIZE 100      /**< Size of random data to write to zone */

/**
 * @brief  Main program entry point - STSAFE-A120 Secure data storage zone
 * access
 * @details Demonstrates data storage partition operations:
 *          - Queries total partition count
 *          - Retrieves and displays partition configuration table
 *          - Reads data from zone 1 and displays content
 *          - Generates random data and writes to zone 1
 *          - Reads back zone 1 to verify the update
 *          - Compares read data with written data
 * @note   Zone IDs are aligned with STSAFE-A120 SPL05 personalization.
 *         Access parameters must be adapted for other personalizations.
 * @retval Not applicable (infinite loop on success or error)
 */
int main(void) {
  stse_return_code_t stse_ret = STSE_API_INVALID_PARAMETER;
  stse_handle_t stse_handle;

  uint8_t readBuffer[READ_BUFFER_SIZE];
  uint8_t random[RANDOM_SIZE];

	/* Initialize Terminal */
	apps_terminal_init(115200);

	/* - Print Example instruction on terminal */
	printf(PRINT_CLEAR_SCREEN);
	printf(    "-----------------------------------------------------------------------------------------------------");
	printf("\n\r-                                       STSAFE-A120                                                 -");
	printf("\n\r-                      secure data storage zone access example                                      -");
	printf("\n\r-----------------------------------------------------------------------------------------------------");
	printf("\n\r- description :                                                                                     -");
	printf("\n\r- This examples illustrates how to makes use of the STSAFE-A data storage APIs by performing         -");
	printf("\n\r- accesses/commands to the target STSAFE device:                                                    -");
	printf("\n\r-          o Query STSAFE-A total partition count                                                    -");
	printf("\n\r-          o Query STSAFE-A partitions information                                                   -");
	printf("\n\r-          o Read STSAFE-A zone 1                                                                    -");
	printf("\n\r-          o Update STSAFE-A zone 1                                                                  -");
	printf("\n\r- Note : zone IDs used in this example are aligned with STSAFE-A120 SPL05 personalization           -");
	printf("\n\r-        Accesses parameters must be adapted for other device personalization                       -");
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

	/* ## Print User NVM data partitioning information */
	apps_print_data_partition_record_table(&stse_handle);

	/* ## Read Zone 1 */
	stse_ret = stse_data_storage_read_data_zone(
		&stse_handle,                               /* Pointer to STSE handle */
		1,                                          /* Zone index */
		0x0000,                                     /* Read offset */
		readBuffer,                                 /* Read buffer */
		sizeof(readBuffer),                         /* Read length */
		04,                                         /* Read chunk size */
		STSE_NO_PROT                                /* Protection mode */
	);

	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r ### stse_data_storage_read_data_zone : ERROR 0x%04X", stse_ret);
		apps_process_error(stse_ret);
	} else {
		printf("\n\n\r - stse_data_storage_read_data_zone ( zone : 01 - length : %d)", (int)sizeof(readBuffer));
		apps_print_hex_buffer(readBuffer, sizeof(readBuffer));
	}

	/* ## Update Zone 1 with Random (Random Number storage) */
	stse_ret = stsafea_generate_random(
		&stse_handle,                               /* Pointer to STSE handle */
		random,                                     /* Output random buffer */
		100                                         /* Requested random size */
	);
	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r ### stsafea_generate_random : ERROR 0x%04X", stse_ret);
		apps_process_error(stse_ret);
	}

	stse_ret = stse_data_storage_update_data_zone(
		&stse_handle,                               /* Pointer to STSE handle */
		1,                                          /* Zone index */
		0x0000,                                     /* Update offset */
		random,                                     /* Update input buffer */
		sizeof(random),                             /* Update length */
		STSE_NON_ATOMIC_ACCESS,                     /* Atomicity */
		STSE_NO_PROT                                /* Protection mode */
	);

	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r ### stse_data_storage_update_data_zone : ERROR 0x%04X", stse_ret);
		apps_process_error(stse_ret);
	} else {
		printf("\n\n\r - stse_data_storage_update_data_zone (zone = 01 - length = %d)", (int)sizeof(random));
		apps_print_hex_buffer(random, sizeof(random));
	}

	/* ## Read Zone 1 */
	stse_ret = stse_data_storage_read_data_zone(
		&stse_handle,                               /* Pointer to STSE handle */
		1,                                          /* Zone index */
		0x0000,                                     /* Read offset */
		readBuffer,                                 /* Read buffer */
		sizeof(readBuffer),                         /* Read length */
		04,                                         /* Read chunk size */
		STSE_NO_PROT                                /* Protection mode */
	);

	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r ### stse_data_storage_read_data_zone : ERROR 0x%04X", stse_ret);
		apps_process_error(stse_ret);
	} else {
		printf("\n\n\r - stse_data_storage_read_data_zone ( zone : 01 - length : %d)", (int)sizeof(readBuffer));
		apps_print_hex_buffer(readBuffer, sizeof(readBuffer));
	}

	printf(PRINT_RESET "\n\r\n\r*#*# STMICROELECTRONICS #*#*\n\r");

	while (1) {
		/* Infinite loop */
	}

	return 0;
}
