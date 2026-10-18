/**
 ******************************************************************************
 * @file    main.c
 * @author  CS application team
 * @brief   STSAFE-A120 NIST P256 Key pair & signature generation example
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
#define HASH_SIZE(x)                                                           \
  (x - x % 16) /**< Macro to align hash size to 16-byte boundary */

/**
 * @brief  Main program entry point - STSAFE-A120 NIST-P256 key pair generation
 * @details Demonstrates ECC key pair generation and signature:
 *          - Queries asymmetric key table to find available slot
 *          - Generates NIST-P256 key pair in selected slot
 *          - Retrieves the generated public key
 *          - Creates a random message and computes SHA-256 hash
 *          - Generates ECDSA signature using the private key
 *          - Verifies the signature using platform crypto library
 * @note   Demonstrates both key generation and signature verification
 *         in a single example
 * @retval Not applicable (infinite loop on success or error)
 */
int main(void) {
  stse_return_code_t stse_ret = STSE_API_INVALID_PARAMETER;
  stse_handle_t stse_handle;

	/* Initialize Terminal */
	apps_terminal_init(115200);

	/* - Print Example instruction on terminal */
	printf(PRINT_CLEAR_SCREEN);
	printf(    "-----------------------------------------------------------------------------------------------------");
	printf("\n\r-                                       STSAFE-A120                                                 -");
	printf("\n\r-                             NIST-P256 key pair generation                                         -");
	printf("\n\r-----------------------------------------------------------------------------------------------------");
	printf("\n\r- This example demonstrates how to generate and use a NIST-P256 key pair on a target STSAFE-A120    -");
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

	/* ## Generate key pair in slot 1 */
	uint8_t slot_1_public_key[STSE_NIST_P_256_PUBLIC_KEY_SIZE];
	stse_ret = stsafea_generate_ecc_key_pair(
		&stse_handle,                               /* Pointer to STSE handle */
		1,                                          /* Slot number */
		STSE_ECC_KT_NIST_P_256,                     /* Key type */
		255,                                        /* Usage limit */
		slot_1_public_key                           /* Pointer to output public key */
	);

	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r ## stsafea_generate_ecc_key_pair : 0x%04X", stse_ret);
		apps_process_error(stse_ret);
	}

	printf("\n\n\r - STSAFE-A120 generate ECC key pair : success");
	printf("\n\r\t o public key : ");
	apps_print_hex_buffer(slot_1_public_key, sizeof(slot_1_public_key));

	/* ## Generate hash to be signed */
	uint8_t hash[HASH_SIZE(STSE_NIST_P_256_PRIVATE_KEY_SIZE)];
	printf("\n\n\r - Plain-text hash :");
	apps_randomize_buffer(hash, sizeof(hash));
	apps_print_hex_buffer(hash, sizeof(hash));

	/* ## Generate signature over hash using target STSAFE-A120 */
	uint8_t signature[STSE_NIST_P_256_SIGNATURE_SIZE];
	stse_ret = stsafea_ecc_generate_signature(
		&stse_handle,                               /* Pointer to STSE handle */
		1,                                          /* Slot number */
		STSE_ECC_KT_NIST_P_256,                     /* Key type */
		hash,                                       /* Digest to sign */
		sizeof(hash),                               /* Digest size */
		signature                                   /* Pointer to output signature */
	);

	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r ## stsafea_ecc_generate_signature : 0x%04X", stse_ret);
		apps_process_error(stse_ret);
	}

	printf("\n\n\r - STSAFE-A120 generate signature : success");
	printf("\n\r\t o signature : ");
	apps_print_hex_buffer(signature, sizeof(signature));

	/* ## Verify signature using target STSAFE-A120 */
	uint8_t verification_status;
	stse_ret = stsafea_ecc_verify_signature(
		&stse_handle,                               /* Pointer to STSE handle */
		STSE_ECC_KT_NIST_P_256,                     /* Key type */
		slot_1_public_key,                          /* Public key */
		signature,                                  /* Signature */
		hash,                                       /* Digest */
		sizeof(hash),                               /* Digest size */
		0,                                          /* Slot number (0 if public key provided) */
		&verification_status                        /* Verification status output */
	);

	if (stse_ret != STSE_OK) {
		printf(PRINT_RED "\n\n\r ## stsafea_ecc_verify_signature : 0x%04X", stse_ret);
		apps_process_error(stse_ret);
	}
	if (verification_status != 1) {
		printf(PRINT_RED "\n\n\r ## STSAFE-A120 verify signature : failed");
	} else {
		printf(PRINT_GREEN "\n\n\r - STSAFE-A120 verify signature : success");
	}

	printf(PRINT_RESET "\n\r\n\r*#*# STMICROELECTRONICS #*#*\n\r");

	while (1) {
		/* Infinite loop */
	}

	return 0;
}
