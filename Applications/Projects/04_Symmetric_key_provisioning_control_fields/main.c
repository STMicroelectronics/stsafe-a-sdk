/**
 ******************************************************************************
 * @file    main.c
 * @author  CS application team
 * @brief   STSAFE-A120 Symmetric key slot control fields provisioning example
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
 * @brief  Main program entry point - STSAFE-A120 Symmetric key provisioning control fields
 * @details Demonstrates symmetric key slot control field configuration:
 *          - Queries and displays current control field settings for slot 0
 *          - Configures control fields to enable key provisioning methods:
 *            * Wrapped anonymous provisioning
 *            * ECDHE anonymous provisioning
 *          - Writes updated control fields to the device
 *          - Verifies the changes by querying again
 * @warning This is a permanent configuration that controls how symmetric keys
 *          can be provisioned to the slot. Once change_right is disabled,
 *          these settings cannot be modified again.
 * @note   Control fields determine security level for key provisioning:
 *         plaintext, wrapped, or ECDHE-based methods
 * @retval Not applicable (infinite loop on success or error)
 */
int main(void)
{
	stse_return_code_t stse_ret = STSE_API_INVALID_PARAMETER;
	stse_handle_t stse_handle;

	PLAT_UI16 symmetric_key_slot = 0;
	stsafea_symmetric_key_slot_provisioning_ctrl_fields_t ctrl_fields = {0};

	/* Configure symmetric key slot to enable the write key command */
	ctrl_fields.wrapped_anonymous = 1;
	ctrl_fields.ecdhe_anonymous = 1;
	ctrl_fields.wrapped_authentication_key = 0xFF;
	ctrl_fields.ecdhe_authentication_key = 0xFF;

	/* Initialize Terminal */
	apps_terminal_init(115200);

	/* - Print Example instruction on terminal */
	printf("-----------------------------------------------------------------------------------------------------");
	printf("\n\r-              STSAFE-A120 Configure Symmetric key slot provisioning control fields example          -");
	printf("\n\r-----------------------------------------------------------------------------------------------------");
	printf("\n\r-                                                                                                   -");
	printf("\n\r- description :                                                                                     -");
	printf("\n\r- This examples illustrates how to configure symmetric key slot provisioning control fields         -");
	printf("\n\r- in STSAFE-A120 secure element                                                                     -");
	printf("\n\r-                                                                                                   -");
	printf("\n\r- Note : Device with accessible symmetric key (i.e. STSAFE-A120 SPL05) is required to run this example -");
	printf("\n\r-                                                                                                   -");
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

	/* ## Display key table provisioning control fields information in serial terminal */
	printf("\n\n\r");
	apps_print_symmetric_key_table_provisioning_control_fields(&stse_handle);

	printf(PRINT_BOLD PRINT_ITALIC "\n\n\r==> Enter the slot number to write : ");
	apps_terminal_read_unsigned_integer(&symmetric_key_slot);

	/* - Write selected symmetric key slot provisioning control fields */
	stse_ret = stsafea_put_symmetric_key_slot_provisioning_ctrl_fields(
		&stse_handle,                    /* Pointer to STSE handle */
		(PLAT_UI8)symmetric_key_slot,    /* Slot number */
		&ctrl_fields                     /* Pointer to control fields structure */
	);
	printf(PRINT_RESET "\n\n\r - Put slot %u provisioning control fields : ", symmetric_key_slot);
	if (stse_ret == STSE_OK)
	{
		printf(PRINT_GREEN "OK");
	}
	else if (stse_ret == STSE_ACCESS_CONDITION_NOT_SATISFIED)
	{
		printf(PRINT_CYAN "Already done");
	}
	else
	{
		printf(PRINT_RED "Error 0x%04X", stse_ret);
		apps_process_error(stse_ret);
	}

	while (1)
	{
		// infinite loop
	}

	return 0;
}
