/**
 ******************************************************************************
 * @file    Apps_utils.c
 * @author  CS application team
 * @brief   Application utilities implementation file providing common helper
 *          functions for STSAFE-A120 examples
 ******************************************************************************
 *                      COPYRIGHT 2022 STMicroelectronics
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */

#include "Apps_utils.h"

/**
 * @brief  Hard fault exception handler
 * @note   This function is called when a hard fault exception occurs and enters
 *         an infinite loop to halt the system
 */
void HardFault_Handler(void) {
    printf("\n\r ## SYSTEM : HARDFAULT ## ");
    while (1) {
        /* Infinite loop */
    }
}

/**
 * @brief  STDIO redirection definitions for different compilers
 * @note   Redirects standard I/O operations (putchar/getchar) to UART functions
 */
#if defined(__GNUC__) && !defined(__ARMCC_VERSION)
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#define GETCHAR_PROTOTYPE int __io_getchar()
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#define GETCHAR_PROTOTYPE int fgetc(FILE *f)
#endif /* __GNUC__ */

/**
 * @brief  Redirect putchar to UART output
 * @param  ch Character to output
 * @return The character written
 */
PUTCHAR_PROTOTYPE {
    uart_putc(ch);
    return ch;
}

/**
 * @brief  Redirect getchar to UART input
 * @return The character read from UART
 */
GETCHAR_PROTOTYPE {
    return uart_getc();
}

/**
 * @brief  Get string representation of ECC key type
 * @param  key_type The ECC key type enumeration value
 * @return Pointer to string containing the key type name
 * @note   Returns "bad curve ID" string if key type is not recognized
 */
char *get_key_type_str(stse_ecc_key_type_t key_type) {
    switch (key_type) {
#ifdef STSE_CONF_ECC_NIST_P_256
    case STSE_ECC_KT_NIST_P_256:
        return ("NIST_P_256");
#endif
#ifdef STSE_CONF_ECC_NIST_P_384
    case STSE_ECC_KT_NIST_P_384:
        return ("NIST_P_384");
#endif
#ifdef STSE_CONF_ECC_NIST_P_521
    case STSE_ECC_KT_NIST_P_521:
        return ("NIST_P_521");
#endif
#ifdef STSE_CONF_ECC_BRAINPOOL_P_256
    case STSE_ECC_KT_BP_P_256:
        return (" BP_P_256 ");
#endif
#ifdef STSE_CONF_ECC_BRAINPOOL_P_384
    case STSE_ECC_KT_BP_P_384:
        return (" BP_P_384 ");
#endif
#ifdef STSE_CONF_ECC_BRAINPOOL_P_512
    case STSE_ECC_KT_BP_P_512:
        return (" BP_P_512 ");
#endif
#ifdef STSE_CONF_ECC_CURVE_25519
    case STSE_ECC_KT_CURVE25519:
        return ("CURVE25519");
#endif
#ifdef STSE_CONF_ECC_EDWARD_25519
    case STSE_ECC_KT_ED25519:
        return (" ED25519  ");
#endif
    default:
        return (PRINT_YELLOW "bad curve ID" PRINT_RESET);
    }
}

/**
 * @brief  Convert curve ID to key type enumeration
 * @param  curve_id The ECC curve identifier to convert
 * @param  p_key_type Pointer to store the corresponding key type
 * @return STSE_OK on success, STSE_UNEXPECTED_ERROR if curve ID not found
 * @note   Iterates through known curve IDs to find a match
 */
stse_return_code_t get_curve_id_key_type(stsafea_ecc_curve_id_t curve_id, stse_ecc_key_type_t *p_key_type) {
    stse_ecc_key_type_t curve_id_index;
    int diff;
    *p_key_type = STSE_ECC_KT_INVALID;

    /* Compare slot curve ID against each known curve ID to set the key type */
    for (curve_id_index = (stse_ecc_key_type_t)0; curve_id_index < STSE_ECC_KT_INVALID; curve_id_index++) {
        diff = memcmp((PLAT_UI8 *)&stse_ecc_info_table[curve_id_index].curve_id,
                      (PLAT_UI8 *)&curve_id,
                      stse_ecc_info_table[curve_id_index].curve_id_total_length);
        if (diff == 0) {
            *p_key_type = curve_id_index;
            break;
        }
    }
    /* If the comparison loop reach the end and p_key_type is always as initialized return error */
    if ((curve_id_index + 1) >= STSE_ECC_KT_INVALID || *p_key_type == STSE_ECC_KT_INVALID) {
        return STSE_UNEXPECTED_ERROR;
    }

    return STSE_OK;
}

/**
 * @brief  Print formatted symmetric key table information
 * @param  p_stse Pointer to STSE handler structure
 * @note   Retrieves and displays all symmetric key slot information including:
 *         - Key type (AES-128/AES-256)
 *         - Mode of operation (CCM*;CMAC;ECB;GCM;HKDF;HMAC)
 *         - Key usage flags (derive, MAC generation/verification, encryption/decryption)
 *         - Parameters (MAC length, auth tag length, counter values, HKDF settings)
 */

void apps_print_symmetric_key_table_info(stse_handle_t *p_stse) {
    stse_return_code_t ret;
    PLAT_UI8 i;
    PLAT_UI8 slot_count;

    ret = stsafea_query_symmetric_key_slots_count(p_stse, &slot_count);
    if (ret != STSE_OK) {
        printf("\n\n\r - stsafea_query_symmetric_key_slots_count : ERROR 0x%04x", ret);
        while (1)
            ;
    }

    stsafea_symmetric_key_slot_information_t symmetric_key_table[slot_count];

    ret = stsafea_query_symmetric_key_table(p_stse, slot_count, symmetric_key_table);
    if (ret != STSE_OK) {
        printf("\n\n\r - stse_get_symmetric_key_slot_info : ERROR 0x%04x", ret);
        while (1)
            ;
    }

    printf("\n\n\r");
    printf("\n\r -------------------------------------+--------------------------------------------------------------------------+------------------------------------------------------------------");
    printf("\n\r                                      |                                  KEY USAGE                               |                            PARAMETERS                            ");
    printf("\n\r ------+------+----------+------------+--------+---------+-----------+---------+---------+-----------+-----------+-------------+--------------+------------+-------------+----------");
    printf("\n\r  SLOT | LOCK | KEY TYPE | MODE OF OP | DERIVE | MAC GEN | MAC VERIF | ENCRYPT | DECRYPT | ENC_CHUNK | DEC CHUNK | MIN MAC LEN | AUTH TAG LEN | CNT OFFSET |   CNT VAL   | HKDF RET ");
    printf("\n\r ------+------+----------+------------+--------+---------+-----------+---------+---------+-----------+-----------+-------------+--------------+------------+-------------+----------");

    for (i = 0; i < slot_count; i++) {
        printf("\r\n   %02d  |", i);
        printf("   %s  |", (symmetric_key_table[i].lock_indicator == 0) ? "." : "x");
        if (symmetric_key_table[i].key_presence == 1) {
            printf(" %s  |", (symmetric_key_table[i].key_type == 0) ? "AES 128" : "AES 256");
            /* Mode of operation */
            printf("    %s    |", (symmetric_key_table[i].mode_of_operation == 0x00) ? "CCM*" : (symmetric_key_table[i].mode_of_operation == 0x01) ? "CMAC"
                                                                                            : (symmetric_key_table[i].mode_of_operation == 0x02)   ? "ECB "
                                                                                            : (symmetric_key_table[i].mode_of_operation == 0x03)   ? "GCM "
                                                                                            : (symmetric_key_table[i].mode_of_operation == 0x04)   ? "HKDF"
                                                                                                                                                   : "HMAC");
            /* Key usage */
            printf("    %s   |", (symmetric_key_table[i].key_usage.derive == 0) ? "." : "x");
            printf("    %s    |", (symmetric_key_table[i].key_usage.mac_generation == 0) ? "." : "x");
            printf("     %s     |", (symmetric_key_table[i].key_usage.mac_verification == 0) ? "." : "x");
            printf("    %s    |", (symmetric_key_table[i].key_usage.encryption == 0) ? "." : "x");
            printf("    %s    |", (symmetric_key_table[i].key_usage.decryption == 0) ? "." : "x");
            printf("     %s     |", (symmetric_key_table[i].key_usage.chunks_encryption == 0) ? "." : "x");
            printf("     %s     |", (symmetric_key_table[i].key_usage.chunks_decryption == 0) ? "." : "x");
            /* Parameters for MAC */
            if (symmetric_key_table[i].mode_of_operation == STSAFEA_KEY_OPERATION_MODE_CMAC) {
                printf("    0x%02X     |", symmetric_key_table[i].parameters.cmac.minimum_mac_length);
            } else if (symmetric_key_table[i].mode_of_operation == STSAFEA_KEY_OPERATION_MODE_HMAC) {
                printf("    0x%02X     |", symmetric_key_table[i].parameters.hmac.minimum_mac_length);
            } else {
                printf("      .      |");
            }
            /* Parameters for AEAD */
            if (symmetric_key_table[i].mode_of_operation == STSAFEA_KEY_OPERATION_MODE_CCM) {
                printf("    0x%02X      |", symmetric_key_table[i].parameters.ccm.auth_tag_length);
                if (symmetric_key_table[i].parameters.ccm.counter_presence == 1) {
                    printf("    %03d     |", symmetric_key_table[i].parameters.ccm.counter_offset_in_nonce);
                    printf(" 0x%02X%02X%02X%02X |",
                           symmetric_key_table[i].parameters.ccm.counter_value[3],
                           symmetric_key_table[i].parameters.ccm.counter_value[2],
                           symmetric_key_table[i].parameters.ccm.counter_value[1],
                           symmetric_key_table[i].parameters.ccm.counter_value[0]);
                } else {
                    printf("     .      |      .      |");
                }
            } else if (symmetric_key_table[i].mode_of_operation == STSAFEA_KEY_OPERATION_MODE_GCM) {
                printf("    0x%02X      |     .      |      .      |", symmetric_key_table[i].parameters.gcm.auth_tag_length);
            } else {
                printf("      .       |     .      |      .      |");
            }
            /* Parameters for Derivation */
            printf("    %s     ", (symmetric_key_table[i].mode_of_operation == STSAFEA_KEY_OPERATION_MODE_HKDF && symmetric_key_table[i].parameters.hkdf.derivation_output_in_response_allowed == 1) ? "x" : ".");
        } else {
            printf("    .     |     .      |    .   |    .    |     .     |    .    |    .    |     .     |     .     |      .      |      .       |     .      |      .      |    .");
        }
    }
}

/**
 * @brief  Print symmetric key table provisioning control fields
 * @param  p_stse Pointer to STSE handler structure
 * @note   Displays provisioning control field information for each symmetric key slot:
 *         - Change right
 *         - Derived key flag
 *         - Plaintext provisioning
 *         - Put key access
 *         - Wrapped provisioning with authentication key
 *         - ECDHE provisioning with authentication key
 */
void apps_print_symmetric_key_table_provisioning_control_fields(stse_handle_t *p_stse) {
    stse_return_code_t ret;
    PLAT_UI8 i;
    PLAT_UI8 slot_count;
    stsafea_symmetric_key_slot_provisioning_ctrl_fields_t ctrl_fields;

    ret = stsafea_query_symmetric_key_slots_count(p_stse, &slot_count);
    if (ret != STSE_OK) {
        printf("\n\n\r - stsafea_query_symmetric_key_slots_count : ERROR 0x%04x", ret);
        while (1)
            ;
    }

    printf("\n\n\r");
    printf("\n\r ------+----------------------------------------------------------------");
    printf("\n\r       |                    PROVISIONNING CTRL FIELDS                   ");
    printf("\n\r ------+--------------+---------+-----------+---------+---------+-------");
    printf("\n\r  SLOT | CHANGE RIGHT | DERIVED | PLAINTEXT | PUT KEY | WRAPPED | ECDHE ");
    printf("\n\r ------+--------------+---------+-----------+---------+---------+-------");

    for (i = 0; i < slot_count; i++) {
        printf("\r\n   %02d  |", i);

        ret = stsafea_query_symmetric_key_slot_provisioning_ctrl_fields(p_stse, i, &ctrl_fields);
        if (ret != STSE_OK) {
            printf("\n\n\r - stse_query_symmetric_key_slot_provisioning_ctrl_fields : ERROR 0x%04x", ret);
            while (1)
                ;
        }

        printf("       %s      |", (ctrl_fields.change_right == 0) ? "." : "x");
        printf("    %s    |", (ctrl_fields.derived == 0) ? "." : "x");
        printf("     %s     |", (ctrl_fields.plaintext == 0) ? "." : "x");
        printf("    %s    |", (ctrl_fields.put_key == 0) ? "." : "x");
        if (ctrl_fields.wrapped_authentication_key != 0xFF) {
            printf("  0x%02X   |", ctrl_fields.wrapped_authentication_key);
        } else {
            printf("    %s    |", (ctrl_fields.wrapped_anonymous == 0) ? "." : "x");
        }

        if (ctrl_fields.ecdhe_authentication_key != 0xFF) {
            printf(" 0x%02X ", ctrl_fields.ecdhe_authentication_key);
        } else {
            printf("   %s   ", (ctrl_fields.ecdhe_anonymous == 0) ? "." : "x");
        }
    }
}

/**
 * @brief  Print formatted asymmetric (ECC) key table information
 * @param  p_stse Pointer to STSE handler structure
 * @note   Retrieves and displays ECC key slot information including:
 *         - Global change right and usage limit
 *         - Per-slot: presence, key type (NIST, Brainpool, Curve25519, Ed25519)
 *         - EdDSA variant (Pure/PreHashed)
 *         - Generate key access control
 *         - Signature generation and key establishment capabilities
 */
void apps_print_asymmetric_key_table_info(stse_handle_t *p_stse) {
    stse_return_code_t ret;
    PLAT_UI8 i;
    PLAT_UI8 slot_count;
    PLAT_UI16 global_usage_limit;
    stse_ecc_key_type_t key_type;

    ret = stse_get_ecc_key_slots_count(p_stse, &slot_count);
    if (ret != STSE_OK) {
        printf("\n\n\r - stse_get_ecc_key_slots_count : ERROR 0x%04x", ret);
        while (1)
            ;
    }

    stsafea_private_key_slot_information_t p_private_key_table_info[slot_count];
    ret = stsafea_query_private_key_table(p_stse, slot_count, &global_usage_limit, p_private_key_table_info);
    if (ret != STSE_OK) {
        printf("\n\n\r - stsafea_query_private_key_table : ERROR 0x%04x", ret);
        while (1)
            ;
    }

    printf("\n\n\r");
    printf("\n\r -----------------------------------------------------------------------------------------");
    printf("\n\r       |                                 ASYMMETRIC KEY INFO                              ");
    printf("\n\r ------+----------+------------+---------------+------------+---------+-------------------");
    printf("\n\r  SLOT | PRESENCE |    TYPE    | EdDSA Variant | Gen Key AC | Sig Gen | Key establishment ");
    printf("\n\r ------+----------+------------+---------------+------------+---------+-------------------");

    for (i = 0; i < slot_count; i++) {
        get_curve_id_key_type(p_private_key_table_info[i].curve_id, &key_type);
        printf("\r\n  0x%02X |", p_private_key_table_info[i].slot_number);
        printf("    %s     |", (p_private_key_table_info[i].presence_flag == 0) ? "." : "x");
        printf(" %s |", (p_private_key_table_info[i].presence_flag == 0) ? "     .    " : get_key_type_str(key_type));
#if defined(STSE_CONF_ECC_CURVE_25519) && defined(STSE_CONF_ECC_EDWARD_25519)
        printf("   %s   |", (key_type != STSE_ECC_KT_CURVE25519 && key_type != STSE_ECC_KT_ED25519) ? "  None   " : (p_private_key_table_info[i].mode_of_operation.eddsa_variant == 0) ? "  Pure   "
                                                                                                                : (p_private_key_table_info[i].mode_of_operation.eddsa_variant == 1)   ? "PreHashed"
                                                                                                                                                                                      : "  ERROR  ");
#elif defined(STSE_CONF_ECC_CURVE_25519)
        printf("   %s   |", (key_type != STSE_ECC_KT_CURVE25519) ? "  None   " : (p_private_key_table_info[i].mode_of_operation.eddsa_variant == 0) ? "  Pure   "
                                                                             : (p_private_key_table_info[i].mode_of_operation.eddsa_variant == 1)   ? "PreHashed"
                                                                                                                                                   : "  ERROR  ");
#elif defined(STSE_CONF_ECC_EDWARD_25519)
        printf("   %s   |", (key_type != STSE_ECC_KT_ED25519) ? "  None   " : (p_private_key_table_info[i].mode_of_operation.eddsa_variant == 0) ? "  Pure   "
                                                                          : (p_private_key_table_info[i].mode_of_operation.eddsa_variant == 1)   ? "PreHashed"
                                                                                                                                                : "  ERROR  ");
#else
        printf("   %s   |", "  None   ");
#endif
        printf("  %s |", (p_private_key_table_info[i].mode_of_operation.generate_key_ac == 0) ? "  Free   " : (p_private_key_table_info[i].mode_of_operation.generate_key_ac == 1) ? "  Host   "
                                                                                                                                                                                 : "Forbidden");
        printf("    %s    |", (p_private_key_table_info[i].mode_of_operation.sig_gen_over_external_data == 0 &&
                               p_private_key_table_info[i].mode_of_operation.sig_gen_over_internal_external_data == 0)
                                  ? "."
                                  : "x");
        printf("         %s", (p_private_key_table_info[i].mode_of_operation.key_establishment == 0) ? "." : "x");
    }
}

#ifdef STSE_CONF_STSAFE_A_SUPPORT

/**
 * @brief  Print host key provisioning control fields
 * @param  p_stse Pointer to STSE handler structure
 * @note   Displays host key provisioning control information:
 *         - Change right
 *         - Re-provision capability
 *         - Plaintext provisioning
 *         - Wrapped anonymous provisioning
 *         - Wrapped/DH derived authentication key slot number
 */
void apps_print_host_key_provisioning_control_fields(stse_handle_t *p_stse) {
    stsafea_host_key_provisioning_ctrl_fields_t provisioning_ctrl_fields;
    stse_return_code_t ret;

    ret = stsafea_query_host_key_provisioning_ctrl_fields(p_stse, &provisioning_ctrl_fields);

    if (ret != STSE_OK) {
        printf("\n\n\r - " PRINT_RED "stsafea_query_host_key_provisioning_ctrl_fields ERROR : 0x%04X" PRINT_RESET, ret);
        while (1)
            ;
    }

    printf("\n\r --------------+--------------+-----------+-------------------+----------------------------");
    printf("\n\r  Change right | Re-provision | Plaintext | Wrapped anonymous | Wrapped authentication key ");
    printf("\n\r --------------+--------------+-----------+-------------------+----------------------------");
    printf("\n\r      %s      |", (provisioning_ctrl_fields.change_right == 0) ? "NO " : "YES");
    printf("     %s      |", (provisioning_ctrl_fields.reprovision == 0) ? "NO " : "YES");
    printf("    %s    |", (provisioning_ctrl_fields.plaintext == 0) ? "NO " : "YES");
    printf("        %s        |", (provisioning_ctrl_fields.wrapped_anonymous == 0) ? "NO " : "YES");
    printf("            0x%02X            ", provisioning_ctrl_fields.wrapped_or_dh_derived_authentication_key);
    printf("\n\r --------------+--------------+-----------+-------------------+----------------------------");
}

/**
 * @brief  Print generic public key slot configuration flags
 * @param  p_stse Pointer to STSE handler structure
 * @param  slot_number Slot number to query and display
 * @note   Displays configuration information for the specified generic public key slot:
 *         - Change right
 *         - Establish symmetric key capability
 *         - Start volatile KEK session capability
 *         - Key presence status
 */
void apps_print_generic_public_key_slot_configuration_flags(stse_handle_t *p_stse, PLAT_UI8 slot_number) {
    stse_return_code_t ret;
    PLAT_UI8 generic_public_key_presence;
    stse_ecc_key_type_t generic_public_key_type;
    stsafea_generic_public_key_configuration_flags_t generic_public_key_config;

    ret = stsafea_query_generic_public_key_slot_info(
        p_stse,
        slot_number,
        &generic_public_key_presence,
        &generic_public_key_config,
        &generic_public_key_type);
    if (ret != STSE_OK) {
        printf("\n\n\r - " PRINT_RED "stsafea_query_generic_public_key_slot_info ERROR : 0x%04X" PRINT_RESET, ret);
        while (1)
            ;
    }

    printf("\n\r ------+--------------+--------------------------+----------------------------+--------------");
    printf("\n\r  Slot | Change right | Estabblish symmetric key | Start volatile KEK session | Key presence ");
    printf("\n\r ------+--------------+--------------------------+----------------------------+--------------");
    printf("\n\r  0x%02X |", slot_number);
    printf("      %s     |", (generic_public_key_config.change_right == 0) ? "NO " : "YES");
    printf("        %s         |", (generic_public_key_config.establish_symmetric_key == 0) ? "FORBIDDEN" : "ALLOWED  ");
    printf("          %s         |", (generic_public_key_config.start_volatile_kek_session == 0) ? "FORBIDDEN" : "ALLOWED  ");
    printf("      %s     ", (generic_public_key_presence == 0) ? "NO " : "YES");
    printf("\n\r ------+--------------+--------------------------+----------------------------+--------------");
}

#endif /* STSE_CONF_STSAFE_A_SUPPORT */

/**
 * @brief  Print buffer content in hexadecimal format
 * @param  buffer Pointer to buffer to display
 * @param  buffer_size Size of the buffer in bytes
 * @note   Formats output with 16 bytes per line, each byte as "0xXX"
 */
void apps_print_hex_buffer(uint8_t *buffer, uint16_t buffer_size) {
    uint16_t i;
    for (i = 0; i < buffer_size; i++) {
        if (i % 16 == 0) {
            printf(" \n\r ");
        }
        printf(" 0x%02X", buffer[i]);
    }
}

/**
 * @brief  Print data partition record table information
 * @param  p_stse Pointer to STSE handler structure
 * @note   Retrieves and displays data storage partition information for each zone:
 *         - Zone ID
 *         - Counter presence flag
 *         - Data segment size
 *         - Read access control and change right
 *         - Update access control and change right
 *         - Current counter value
 */
void apps_print_data_partition_record_table(stse_handle_t *p_stse) {
    uint8_t i;
    stse_return_code_t ret;
    uint16_t data_partition_record_table_length;
    uint8_t total_zone_count;

    /*- Get total partition count */
    ret = stsafea_get_total_partition_count(
        p_stse,
        &total_zone_count);
    if (ret != STSE_OK) {
        printf("\n\n\r ### stsafe_get_total_partition_count : ERROR 0x%04X", ret);
        while (1)
            ;
    }

    /*- Allocate data partition table  */
    if (total_zone_count > STSAFEA_MAX_DATA_PARTITIONS) {
        printf("\n\n\r ### total_zone_count exceeds maximum (%u > %u)", total_zone_count, STSAFEA_MAX_DATA_PARTITIONS);
        while (1)
            ;
    }
    stsafea_data_partition_record_t data_partition_record_table[STSAFEA_MAX_DATA_PARTITIONS];

    /*- Get data partition table  */
    data_partition_record_table_length = (uint16_t)(total_zone_count * sizeof(stsafea_data_partition_record_t));
    ret = stsafea_get_data_partitions_configuration(
        p_stse,
        total_zone_count,
        data_partition_record_table,
        data_partition_record_table_length);
    if (ret != STSE_OK) {
        printf("\n ### stse_get_data_partitions_configuration : ERROR 0x%04X", ret);
        while (1)
            ;
    } else {
        printf("\n\n\r - stse_get_data_partitions_configuration");
    }

    printf("\n  ID | COUNTER | DATA SEGMENT SIZE | READ AC CR |  READ AC | UPDATE AC CR | UPDATE AC |  COUNTER VAL \r\n");
    for (i = 0; i < total_zone_count; i++) {
        /*- print id (col 1)*/
        printf(" %03d | ", data_partition_record_table[i].index);

        /*- print counter presence (col 2)*/
        printf("   %c    |", (data_partition_record_table[i].zone_type) == 0 ? '.' : 'x');

        /*- print data segment size (col 3) */
        printf("       %04u        | ", data_partition_record_table[i].data_segment_length);

        /*- read ac change right (col 4) */
        printf(" %s | ", (data_partition_record_table[i].read_ac_cr) == 1 ? " ALLOWED " : " DENIED  ");

        /*- read ac right (col 5) */
        switch (data_partition_record_table[i].read_ac) {
        case STSE_AC_ALWAYS:
            printf(" ALWAYS  |");
            break;

        case STSE_AC_HOST:
            printf("   HOST  |");
            break;

        case STSE_AC_AUTH_AND_HOST:
            printf("   AUTH + HOST  |");
            break;

        default:
            printf("  NEVER  |");
            break;
        }

        /*- update ac change right (col 4) */
        printf(" %s | ", (data_partition_record_table[i].update_ac_cr) == 1 ? "   ALLOWED  " : "   DENIED   ");

        /*- update ac right (col 5) */
        switch (data_partition_record_table[i].update_ac) {
        case STSE_AC_ALWAYS:
            printf("  ALWAYS  |");
            break;

        case STSE_AC_HOST:
            printf("   HOST   |");
            break;

        case STSE_AC_AUTH_AND_HOST:
            printf("  AUTH + HOST  |");
            break;

        default:
            printf("  NEVER   |");
            break;
        }

        printf(" %06" PRIu32 "\r\n", data_partition_record_table[i].counter_value);
    }
}

/**
 * @brief  Print command authorization record table
 * @param  command_ac_record_table Pointer to array of command authorization records
 * @param  total_command_count Number of commands in the table
 * @note   Displays command access control information:
 *         - Command header and extended header
 *         - Access conditions (NEVER/FREE/ADMIN/HOST/ADMIN_OR_PWD/ADMIN_OR_HOST)
 *         - Command encryption flag
 *         - Response encryption flag
 */
void apps_print_command_ac_record_table(stse_cmd_authorization_record_t *command_ac_record_table,
                                        uint8_t total_command_count) {
    uint8_t i;

    printf("\n\r  HEADER | EXT-HEADER |    AC   | CMDEnc | RSPEnc  ");
    for (i = 0; i < total_command_count; i++) {
        printf("\n\r");
        /*- print HEADER (col 1)*/
        printf("   0x%02X  | ", command_ac_record_table[i].header);

        /*- print EXT-HEADER (col 2)*/
        if (command_ac_record_table[i].extended_header != 0) {
            printf("    0x%02X   | ", command_ac_record_table[i].extended_header);
        } else {
            printf("     -     | ");
        }

        /*- Access conditions (col 3) */
        switch (command_ac_record_table[i].command_ac) {
        case STSE_CMD_AC_NEVER:
            printf(" NEVER  |");
            break;
        case STSE_CMD_AC_FREE:
            printf("  FREE  |");
            break;
        case STSE_CMD_AC_ADMIN:
            printf(" ADMIN  |");
            break;
        case STSE_CMD_AC_HOST:
            printf("  HOST  |");
            break;
        case STSE_CMD_AC_ADMIN_OR_PWD:
            printf("ADM/PWD |");
            break;
        case STSE_CMD_AC_ADMIN_OR_HOST:
            printf(" ADM/HST |");
            break;
        default:
            printf("  --?--  |");
            break;
        }

        /*- cmd encryption (col 4) */
        printf(" %s | ", (command_ac_record_table[i].host_encryption_flags.cmd) == 1 ? "  YES " : "  NO  ");
        /*- rsp encryption (col 5) */
        printf(" %s | ", (command_ac_record_table[i].host_encryption_flags.rsp) == 1 ? "  YES " : "  NO  ");
    }
}

/**
 * @brief  Print human-readable life cycle state
 * @param  life_cycle_state The life cycle state enumeration value
 * @note   Prints one of: BORN, PATCHING, OPERATIONAL, TERMINATED,
 *         BORN_AND_LOCKED, OPERATIONAL_AND_LOCKED, or UNKNOWN
 */
void apps_print_life_cycle_state(stsafea_life_cycle_state_t life_cycle_state) {
    switch (life_cycle_state) {
    case STSAFEA_LCS_BORN:
        printf("BORN");
        break;
    case STSAFEA_LCS_PATCHING:
        printf("PATCHING");
        break;
    case STSAFEA_LCS_OPERATIONAL:
        printf("OPERATIONAL");
        break;
    case STSAFEA_LCS_TERMINATED:
        printf("TERMINATED");
        break;
    case STSAFEA_LCS_BORN_AND_LOCKED:
        printf("BORN_AND_LOCKED");
        break;
    case STSAFEA_LCS_OPERATIONAL_AND_LOCKED:
        printf("OPERATIONAL_AND_LOCKED");
        break;
    default:
        printf("UNKNOWN");
        break;
    }
};

/**
 * @brief  Initialize terminal for application output
 * @param  baudrate Baudrate for UART communication (currently hardcoded to 115200)
 * @note   Initializes UART, disables I/O buffering for stdout/stdin,
 *         and clears the terminal screen
 */
void apps_terminal_init(uint32_t baudrate) {
    (void)baudrate;
    /* - Initialize UART for example output log (baud 115200)  */
    uart_init(115200);
    /* Disable I/O buffering for STDOUT stream*/
    setvbuf(stdout, NULL, _IONBF, 0);
    /* Disable I/O buffering for STDIN stream*/
    setvbuf(stdin, NULL, _IONBF, 0);
    /* - Clear terminal */
    printf(PRINT_RESET PRINT_CLEAR_SCREEN);
}

/**
 * @brief  Read string from terminal
 * @param  string Pointer to buffer to store the read string (can be NULL)
 * @param  length Pointer to maximum length, updated with actual read length (can be NULL)
 * @return 0 on success, 1 if maximum length exceeded
 * @note   Reads characters until carriage return ('\r') is received
 *         Echoes each character back to the terminal
 */
uint8_t apps_terminal_read_string(char *string, uint8_t *length) {
    char c = 1;
    uint16_t i = 0;
    while (c != '\r') {
        c = uart_getc();
        uart_putc(c);
        if (string != NULL) {
            string[i] = c;
        }
        i++;
        if (((length != NULL) && (i > *length)) || ((length == NULL) && (i > 0))) {
            return 1;
        }
    }
    if (length != NULL) {
        *length = i;
    }
    return 0;
}

/**
 * @brief  Read unsigned integer from terminal
 * @param  integer Pointer to store the read integer value
 * @return 0 on success, 1 on error
 * @note   Reads a string of up to 6 characters and converts to integer using atoi()
 */
uint8_t apps_terminal_read_unsigned_integer(uint16_t *integer) {
    uint8_t length = 6;
    char string[length];
    if (apps_terminal_read_string(string, &length)) {
        return 1;
    }

    *integer = (uint8_t)atoi(string);

    return 0;
}

/**
 * @brief  Generate a random 32-bit number
 * @return Random 32-bit unsigned integer
 * @note   Uses the platform's random number generator
 */
uint32_t apps_generate_random_number(void) {
    return rng_generate_random_number();
}

/**
 * @brief  Fill buffer with random data
 * @param  p_buffer Pointer to buffer to fill
 * @param  buffer_length Length of the buffer in bytes
 * @note   Each byte is filled with a random value using the platform RNG
 */
void apps_randomize_buffer(uint8_t *p_buffer, uint16_t buffer_length) {
    for (uint16_t i = 0; i < buffer_length; i++) {
        *(p_buffer + i) = (rng_generate_random_number() & 0xFF);
    }
}

/**
 * @brief  Compare two buffers byte by byte
 * @param  p_buffer1 Pointer to first buffer
 * @param  p_buffer2 Pointer to second buffer
 * @param  buffers_length Length of buffers to compare in bytes
 * @return 0 if buffers are identical, 1 if any byte differs
 * @note   Compares buffers element by element
 */
uint8_t apps_compare_buffers(uint8_t *p_buffer1, uint8_t *p_buffer2, uint16_t buffers_length) {

    for (uint16_t i = 0; i < buffers_length; i++) {
        if (*p_buffer1 != *p_buffer2) {
            return 1;
        }
    }

    return 0;
}

/**
 * @brief  Delay execution for specified milliseconds
 * @param  ms Number of milliseconds to delay
 * @note   Wrapper around platform delay function
 */
void apps_delay_ms(uint16_t ms) {
    delay_ms(ms);
}

/**
 * @brief  Process error by entering infinite loop
 * @param  err Error code (currently unused)
 * @note   This function never returns - used for error handling in examples
 */
void apps_process_error(uint32_t err) {
    while (1) {
        /* Infinite loop */
    }
}

#ifdef STSE_CONF_USE_HOST_SESSION
/**
 * @brief  Store host keys in platform secure storage and open a host session
 * @param  p_stse            Pointer to STSE handler structure
 * @param  p_session         Pointer to the session handler to open
 * @param  p_host_mac_key    Pointer to host MAC key
 * @param  p_host_cipher_key Pointer to host cipher key
 * @return STSE_OK on success ; stse_return_code_t error code otherwise
 */
stse_return_code_t apps_open_host_session(stse_handle_t *p_stse,
                                          stse_session_t *p_session,
                                          const stse_aes_key_t *p_host_mac_key,
                                          const stse_aes_key_t *p_host_cipher_key) {
    stse_return_code_t ret;
    PLAT_UI32 host_mac_key_idx;
    PLAT_UI32 host_cipher_key_idx;

    /* - Store host MAC key in platform secure storage */
    ret = stse_platform_store_aes_key((PLAT_UI8 *)p_host_mac_key->key,
                                      (p_host_mac_key->type == STSE_AES_256_KT) ? STSE_AES_256_KEY_SIZE : STSE_AES_128_KEY_SIZE,
                                      STSE_AES_KEY_USAGE_MAC,
                                      &host_mac_key_idx);
    if (ret != STSE_OK) {
        return ret;
    }

    /* - Store host cipher key in platform secure storage */
    ret = stse_platform_store_aes_key((PLAT_UI8 *)p_host_cipher_key->key,
                                      (p_host_cipher_key->type == STSE_AES_256_KT) ? STSE_AES_256_KEY_SIZE : STSE_AES_128_KEY_SIZE,
                                      STSE_AES_KEY_USAGE_CIPHER,
                                      &host_cipher_key_idx);
    if (ret != STSE_OK) {
        stse_platform_delete_aes_key(host_mac_key_idx);
        return ret;
    }

    /* - Open host session using stored key indexes */
    ret = stsafea_open_host_session_from_idx(p_stse, p_session, host_mac_key_idx, host_cipher_key_idx);
    if (ret != STSE_OK) {
        stse_platform_delete_aes_key(host_mac_key_idx);
        stse_platform_delete_aes_key(host_cipher_key_idx);
    }

    return ret;
}
#endif /* STSE_CONF_USE_HOST_SESSION */

