/**
 ******************************************************************************
 * @file    main.c
 * @author  CS application team
 * @brief   STSAFE-A120 Echo loop example
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
#include "stselib.h"

/* Main program --------------------------------------------------------------*/

/**
 * brief :   Main program entry point - STSAFE-A120 Echo loop example
 * details : Demonstrates the echo command functionality:
 *            - Generates random message with random length (1-500 bytes)
 *            - Sends message to STSAFE-A120 device via echo command
 *            - Receives and verifies echoed message
 *            - Repeats continuously with 1 second delay
 * retval :  Not applicable (infinite loop)
 */
int main(void) {

  stse_return_code_t stse_ret = STSE_API_INVALID_PARAMETER;
  stse_handle_t stse_handle;
  uint16_t message_length = 0;

  /* Initialize Terminal */
  apps_terminal_init(115200);

	/* Print Example instruction on terminal */
	printf(PRINT_CLEAR_SCREEN);
	printf(    "-----------------------------------------------------------------------------------------------------");
	printf("\n\r-                                       STSAFE-A120                                                 -");
	printf("\n\r-                                       echo example                                                -");
	printf("\n\r-----------------------------------------------------------------------------------------------------");
	printf("\n\r- This example illustrates how to perform an echo test.                                             -");
	printf("\n\r-----------------------------------------------------------------------------------------------------");

	/* Initialize device handle */
	stse_ret = stse_set_default_handler_value(&stse_handle);

	/* Verify correct handle initialization */
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

	/* Verify stselib initialization */
	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\r ## stse_init ERROR : 0x%04X\n\r", stse_ret);
		apps_process_error(stse_ret);
	}

  /* Generate random message length (1..500) */
  message_length = (uint16_t)(apps_generate_random_number() & 0x1FF);
  if ((message_length > 500) || (message_length == 0)) {
    message_length = 1;
  }

  /* Create message and echo buffers (max 500 bytes) */
  uint8_t message[500] = {0};
  uint8_t echoed_message[500] = {0};

  /* Fill message with random content */
  apps_randomize_buffer(message, message_length);

  /* Print message */
  printf("\n\n\r ## Echo message :\n\r");
  apps_print_hex_buffer(message, message_length);

  /* Perform echo operation */
  stse_ret = stsafea_echo(&stse_handle,   /* Target STSE handle */
                          message,        /* Pointer to Echo message buffer */
                          echoed_message, /* Pointer to Echoed message buffer */
                          message_length  /* Echo message length */
  );
  if (stse_ret != STSE_OK) {
    printf("\n\r ## stsafea_echo ERROR : 0x%04X\n\r", stse_ret);
    apps_process_error(stse_ret);
  }

  /* Print echo message */
  printf("\n\n\r ## Echoed message :\n\r");
  apps_print_hex_buffer(echoed_message, message_length);

  /* Compare message and echoed message */
  if (apps_compare_buffers(message, echoed_message, message_length)) {
    printf("\n\n\r ## ECHO MESSAGES COMPARE ERROR (%d)", message_length);
  }
  printf("\n\r\n\r*#*# STMICROELECTRONICS #*#*\n\r");

  while (1) {
    /* Infinite loop */
  }
}
