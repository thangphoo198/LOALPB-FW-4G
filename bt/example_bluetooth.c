/*================================================================
  Copyright (c) 2020 Quectel Wireless Solution, Co., Ltd.  All Rights Reserved.
  Quectel Wireless Solution Proprietary and Confidential.
=================================================================*/
#include <stdio.h>
#include <string.h>
#include "ql_bt.h"
#include "ql_rtos.h"
#include "ql_log.h"
#include "ql_application.h"
#include "ql_audio.h"
#include "ql_gpio.h"



/***************************************BLE*************************************************************/
#define QL_BT_USE_UART     1 //3--uart4

//#define QL_GPIO_BT_POWER   77
#define QL_GPIO_BT_RESET   25
#define QL_GPIO_BT_LDO_EN  26
#define QL_GPIO_BT_WAKEUP_HOST 22
#define QL_GPIO_HOST_WAKEUP_BT 21


bt_uart_cfg ql_bt_uart_port =
{
    QL_BT_USE_UART,     //you can change it, if you use other UART port
    1,                  //do not change it
    0,                  //do not change it
    0,                  //do not change it
    bt_rx_int,          //do not change it
    0,                  //do not change it
    0,                  //do not change it
    BAUDRATE_115200,    //do not change it. it is a default baudrate. stack will change baudrate accroding bt_user_cfg
    BT_DATA_8,          //do not change it
    BT_PARITY_NONE,     //do not change it
    NULL,               //do not change it. host stack will use it
    NULL,               //do not change it. host stack will use it
};

struct bt_user_cfg ql_default_bt_cfg = {
    .bt_mode = BT_MODE_BR_LE,
    .h4_h5 = BT_HCI_MODE_H5,
    .firmware_where = BT_FIRMWARE_AT_FLASH,
    .hw.baudrate = BAUDRATE_921600,
    .hw.uart_cfg = &ql_bt_uart_port,
    .hw.reset_pin = QL_GPIO_BT_RESET,
    .hw.pwdn_pin = QL_GPIO_BT_LDO_EN,
    .hw.host_wakeup_bt_pin = QL_GPIO_HOST_WAKEUP_BT,
    .hw.bt_wakeup_host_pin = QL_GPIO_BT_WAKEUP_HOST,
    .profile = {
        .hfp = 1,
        .a2dp = 1,
        .avrcp = 1,
        .spp = 1,
        .obex = 1,
        .hfp_hf = 0,
        .hid = 0,
        .a2dp_sink = 0,
    },
    .bt_chip_id = 5801,
    .xip = 0,
};

struct bt_user_init_cfg ql_init_cfg = {
	.name = "QUECTEL_BT",
	.inquiry_scan_interval = 0x1000,
	.inquiry_scan_window = 0x12,
	.io_capability = 3,
};

#if 0
void bt_usr_handle_le(struct bt_task_event * msg)
{
	char buffer[256];
	switch (msg->event_id) {
		case BTTASK_IND_LE_SCAN_EVENT: {
			struct bt_event_le_scan_event *scan = (struct bt_event_le_scan_event *)msg->payload;
			unsigned char index = 0;
			int total_size = (int)scan->length;

			app_ble_dbg("[DEMO]handle LE scan event from: "
						"%02x:%02x:%02x:%02x:%02x:%02x. length %d. type %d, rssi %d, device type %d\r\n",
						scan->address.bytes[0], scan->address.bytes[1],
						scan->address.bytes[2], scan->address.bytes[3],
						scan->address.bytes[4], scan->address.bytes[5],
						scan->length, scan->event_type,
						scan->rssi,
						appbt_le_parse_device_type(scan->data, total_size));
			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] le scan event from: "
						 "%02x:%02x:%02x:%02x:%02x:%02x. length %d. type %d, rssi %d, device type %d\r\n",
						 scan->address.bytes[0], scan->address.bytes[1],
						 scan->address.bytes[2], scan->address.bytes[3],
						 scan->address.bytes[4], scan->address.bytes[5],
						 scan->length, scan->event_type,
						 scan->rssi,
						 appbt_le_parse_device_type(scan->data, total_size));
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
			}

			while (total_size > 0 && scan->event_type == LE_ADV_TYPE_IND) {
				int length = scan->data[index];
				int type = scan->data[index + 1];
				app_ble_dbg("[DEMO]handle type %02x, length %d, index %d\r\n",
							type, length, index);
				switch (type) {
					case LE_GAP_TYPE_COMPLETE_NAME: {
						char temp[31];
						memset(temp, 0, 31);
						memcpy(temp, &scan->data[index + 2], length - 1);
						app_ble_dbg("[DEMO]handle name %s\r\n", temp);
						break;
					}

					default:
						break;
				}
				index += (length + 1);
				total_size -= (length + 1);
			}
			break;
		}

		case BTTASK_IND_LE_GATT_CONNECTED: {
			struct bt_event_le_att_connected *connect =
				(struct bt_event_le_att_connected *)msg->payload;

			app_ble_dbg("[DEMO]handle GATT connected(%d) handle %d "
						"%02x%02x%02x%02x%02x%02x type %d\r\n",
						connect->role,
						connect->acl_handle,
						connect->addr.bytes[0], connect->addr.bytes[1],
						connect->addr.bytes[2], connect->addr.bytes[3],
						connect->addr.bytes[4], connect->addr.bytes[5],
						connect->addr_type);
			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] le GATT connected(%d) handle %d,"
						 "addr: %02x%02x%02x%02x%02x%02x, addr_type: %d\r\n",
						 connect->role,
						 connect->acl_handle,
						 connect->addr.bytes[0], connect->addr.bytes[1],
						 connect->addr.bytes[2], connect->addr.bytes[3],
						 connect->addr.bytes[4], connect->addr.bytes[5],
						 connect->addr_type);
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
			}

			m_con_handle = connect->acl_handle;

#if BLE_USE_EXTEND_ADV
			unsigned char handle[1] = {0x01};
			unsigned short duration[1] = {0x00};
			unsigned char max_event[1] = {0x00};
			appbt_le_set_ext_adv_enable(0, 1, handle, duration, max_event);
#else
			appbt_le_set_adv_enable(0);
#endif			

			// test security req
			// appbt_le_security_request(m_con_handle);
			break;
		}

		case BTTASK_IND_LE_GATT_DISCONNECTED: {
			int *conn_handle = (int *)msg->payload;

			app_ble_dbg("[DEMO]handle GATT disconnected handler %d \r\n", *conn_handle);
			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] le GATT disconnected handle %d\r\n", *conn_handle);
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
			}

			m_con_handle = BLE_DEMO_HCI_CON_HANDLE_INVALID;
#if BLE_USE_EXTEND_ADV
			unsigned char handle[1] = {0x01};
			unsigned short duration[1] = {0x00};
			unsigned char max_event[1] = {0x00};
			appbt_le_set_ext_adv_enable(1, 1, handle, duration, max_event);
#else
			appbt_le_set_adv_enable(1);
#endif
 			break;
		}

		case BTTASK_IND_LE_MTU_EXCHANGED: {
			struct bt_event_le_mtu_exchange *mtu = (struct bt_event_le_mtu_exchange *)msg->payload;

			app_ble_dbg("[DEMO]handle GATT MTU exchaneg message %d %d\r\n",
						mtu->mut, mtu->acl_handle);
			current_att_payload_len = mtu->mut;
			app_ble_dbg("[DEMO] current_att_payload_len: %d\n", current_att_payload_len);
			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] le mtu exchanged, connection_handle: 0x%x, mtu: 0x%x\r\n",
						 mtu->acl_handle, mtu->mut);
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
			}
			break;
		}

		case BTTASK_IND_LE_CLIENT_MTU_EXCHANGED: {
			struct bt_event_le_mtu_exchange *mtu = (struct bt_event_le_mtu_exchange *)msg->payload;

			app_ble_dbg("[DEMO]GATT client MTU exchaneg done %d \r\n", mtu->mut);

			break;
		}

		case BTTASK_IND_SMP_PASSKEY: {
			struct bt_event_le_smp_passkey *passkey =
				(struct bt_event_le_smp_passkey *)msg->payload;

			app_ble_dbg("[DEMO]handle SMP PASSKEY %d %02X%02X%02X%02X%02X%02X\r\n",
						passkey->passkey_value,
						passkey->addr.bytes[0], passkey->addr.bytes[1],
						passkey->addr.bytes[2], passkey->addr.bytes[3],
						passkey->addr.bytes[4], passkey->addr.bytes[5]);

			// RTI_LOG("SMP PASSKEY %d\r\n", passkey->passkey_value);
			break;
		}
		case BTTASK_IND_LE_ATT_EVENT_CAN_SEND_NOW: {
			app_ble_dbg("[DEMO] BTTASK_IND_LE_ATT_EVENT_CAN_SEND_NOW\n");
#ifdef BLE_DEMO_LOOP_SEND_TEST
			uint16_t send_len = ble_demo_min(current_att_payload_len, sizeof(ble_demo_loop_test_buf));
			app_ble_dbg("[DEMO] current_att_payload_len: %d, sizeof(ble_demo_loop_test_buf): %d\n", current_att_payload_len, sizeof(ble_demo_loop_test_buf));
			ble_demo_loop_test_buf[0]++;
			if (ble_demo_loop_test_ccc != 0) {
				appbt_le_notify(ble_demo_loop_test_handle, (uint8_t *)ble_demo_loop_test_buf, send_len);
			}
#endif
			break;
		}
		case BTTASK_IND_LE_ATT_EVENT_CAN_WRITE_NOW: {
			struct bt_event_le_att_can_write_now *event =
				(struct bt_event_le_att_can_write_now *)msg->payload;
			app_ble_dbg("[DEMO] BTTASK_IND_LE_ATT_EVENT_CAN_WRITE_NOW, conn: 0x%x, write_type: %d\n", event->connection_handle, event->write_type);
			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] handle can write now, connection_handle: 0x%x, write_type: %d\r\n",
						 event->connection_handle, event->write_type);
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
			}
			break;
		}

		case BTTASK_IND_LE_ATT_EVENT_READ_DATA_RESULT: {
			struct bt_event_le_att_read_data_result *event =
				(struct bt_event_le_att_read_data_result *)msg->payload;
			app_ble_dbg("[DEMO] BTTASK_IND_LE_ATT_EVENT_READ_DATA_RESULT, connection_handle: 0x%x, value_handle: 0x%x, value_offset: %d, value_length: %d\n",
						event->connection_handle, event->value_handle, event->value_offset, event->value_len);
			app_bt_dump_data("[DEMO]", event->value, event->value_len);
			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] handle read data result, connection_handle: 0x%x, value_handle: 0x%x, value_offset: %d, value_length: %d\r\n",
						 event->connection_handle, event->value_handle, event->value_offset, event->value_len);
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);

				int i = 0;
				int offset = 0;
				memset(buffer, 0, sizeof(buffer) - 1);
				for (i = 0; i < event->value_len; i++) {
					snprintf(buffer + offset * 3, 4, "%02x,", event->value[i]);
					offset++;
					if (offset == APP_BT_DUMP_DATA_MAX_BYTES_OF_EACH_LINE) {
						ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
						offset = 0;
						memset(buffer, 0, APP_BT_DUMP_DATA_MAX_SIZE_OF_EACH_LINE);
					}
				}

				if (offset > 0) {
					ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
					offset = 0;
					memset(buffer, 0, APP_BT_DUMP_DATA_MAX_SIZE_OF_EACH_LINE);
				}
			}
			break;
		}
		case BTTASK_IND_LE_ATT_EVENT_READ_OVER_RESULT: {
			struct bt_event_le_att_read_over_result *event =
				(struct bt_event_le_att_read_over_result *)msg->payload;
			app_ble_dbg("[DEMO] BTTASK_IND_LE_ATT_EVENT_READ_OVER_RESULT, connection_handle: 0x%x, att_status: %d\n",
						event->connection_handle, event->att_status);
			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] handle read over result, connection_handle: 0x%x, att_status: %d\r\n",
						 event->connection_handle, event->att_status);
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
			}

			break;
		}

		case BTTASK_IND_SLAVE_LE_BOND_COMPLETE: {
			struct bt_event_le_bond_complete *event =
				(struct bt_event_le_bond_complete *)msg->payload;
			app_ble_dbg("[DEMO] BTTASK_IND_SLAVE_LE_BOND_COMPLETE, type: %d, address: %02x%02x%02x%02x%02x%02x\r\n",
						event->address_type,
						event->address.bytes[0], event->address.bytes[1], event->address.bytes[2],
						event->address.bytes[3], event->address.bytes[4], event->address.bytes[5]);

			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				snprintf(buffer, sizeof(buffer) - 1,
						 "[BTTEST] slave le bond complete, type: %d, address: %02x%02x%02x%02x%02x%02x\r\n",
						 event->address_type,
						 event->address.bytes[0], event->address.bytes[1], event->address.bytes[2],
						 event->address.bytes[3], event->address.bytes[4], event->address.bytes[5]);
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
			}
			break;
		}

		case BTTASK_IND_LE_IDENTITY_INFO: {
			app_ble_dbg("[DEMO] BTTASK_IND_LE_IDENTITY_INFO\n");
			struct bt_event_le_identity_info_event *info =
				(struct bt_event_le_identity_info_event *)msg->payload;

			if (info->success_flag == 1) {
				if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
					snprintf(buffer, sizeof(buffer) - 1,
							 "[BTTEST] identity success, index: %d, connection_handle: 0x%x\r\n""addr: %02x%02x%02x%02x%02x%02x, addr_type: %d\r\n""identity_addr: %02x%02x%02x%02x%02x%02x, identity_addr_type: %d\r\n",
							 info->index, info->connection_handle,
							 info->address[0], info->address[1],
							 info->address[2], info->address[3],
							 info->address[4], info->address[5],
							 info->address_type,
							 info->identity_address[0], info->identity_address[1],
							 info->identity_address[2], info->identity_address[3],
							 info->identity_address[4], info->identity_address[5],
							 info->identity_address_type
							);
					ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
				}
			} else {
				if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
					snprintf(buffer, sizeof(buffer) - 1,
							 "[BTTEST] identity fail, index: %d, connection_handle: 0x%x\r\n""addr: %02x%02x%02x%02x%02x%02x, addr_type: %d\r\n",
							 info->index, info->connection_handle,
							 info->address[0], info->address[1],
							 info->address[2], info->address[3],
							 info->address[4], info->address[5],
							 info->address_type
							);
					ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
				}
			}
			break;
		}

		case BTTASK_IND_LE_DB_CONTROL_GET_INFO: {
			app_ble_dbg("[DEMO] BTTASK_IND_LE_DB_CONTROL_GET_INFO\n");
			struct bt_event_le_get_pair_info_event *info =
				(struct bt_event_le_get_pair_info_event *)msg->payload;

			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] index: %d, seq: %d, valid count: %d, max count: %d, "
						 "addr: %02x%02x%02x%02x%02x%02x, addr_type: %d\r\n", info->index, info->seq, info->valid_count, info->max_count,
						 info->identity_address[0], info->identity_address[1],
						 info->identity_address[2], info->identity_address[3],
						 info->identity_address[4], info->identity_address[5],
						 info->identity_address_type);
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
			}
			break;
		}

		case BTTASK_IND_LE_CLIENT_HANDLE_NOTIFY: {
			struct bt_event_le_client_handle_notify *notify =
				(struct bt_event_le_client_handle_notify *)msg->payload;
			app_ble_dbg("[DEMO] BTTASK_IND_LE_CLIENT_HANDLE_NOTIFY, connection_handle: 0x%x, value_handle: 0x%x, value_length: %d\n",
						notify->connection_handle, notify->value_handle, notify->value_length);

			app_bt_dump_data("[DEMO]", notify->value, notify->value_length);

			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] handle notify, connection_handle: 0x%x, value_handle: 0x%x, value_length: %d\r\n",
						 notify->connection_handle, notify->value_handle, notify->value_length);
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);

				int i = 0;
				int offset = 0;
				memset(buffer, 0, sizeof(buffer) - 1);
				for (i = 0; i < notify->value_length; i++) {
					snprintf(buffer + offset * 3, 4, "%02x,", notify->value[i]);
					offset++;
					if (offset == APP_BT_DUMP_DATA_MAX_BYTES_OF_EACH_LINE) {
						ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
						offset = 0;
						memset(buffer, 0, APP_BT_DUMP_DATA_MAX_SIZE_OF_EACH_LINE);
					}
				}

				if (offset > 0) {
					ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
					offset = 0;
					memset(buffer, 0, APP_BT_DUMP_DATA_MAX_SIZE_OF_EACH_LINE);
				}
			}
			break;
		}

		case BTTASK_IND_LE_CLIENT_HANDLE_INDIATION: {
			struct bt_event_le_client_handle_indication *indication =
				(struct bt_event_le_client_handle_indication *)msg->payload;
			app_ble_dbg("[DEMO] BTTASK_IND_LE_CLIENT_HANDLE_INDIATION, connection_handle: 0x%x, value_handle: 0x%x, value_length: %d\n",
						indication->connection_handle, indication->value_handle, indication->value_length);

			app_bt_dump_data("[DEMO]", indication->value, indication->value_length);

			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] handle indication, connection_handle: 0x%x, value_handle: 0x%x, value_length: %d\r\n",
						 indication->connection_handle, indication->value_handle, indication->value_length);
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);

				int i = 0;
				int offset = 0;
				for (i = 0; i < indication->value_length; i++) {
					snprintf(buffer + offset * 3, 4, "%02x,", indication->value[i]);
					offset++;
					if (offset == APP_BT_DUMP_DATA_MAX_BYTES_OF_EACH_LINE) {
						ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
						offset = 0;
						memset(buffer, 0, APP_BT_DUMP_DATA_MAX_SIZE_OF_EACH_LINE);
					}
				}

				if (offset > 0) {
					ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
					offset = 0;
					memset(buffer, 0, APP_BT_DUMP_DATA_MAX_SIZE_OF_EACH_LINE);
				}
			}
			break;
		}
		case BTTASK_IND_LE_GATT_SCAN_RESULT: {
			struct bt_event_le_gatt_scan_result *result =
				(struct bt_event_le_gatt_scan_result *)msg->payload;
			app_ble_dbg("[DEMO] BTTASK_IND_LE_GATT_SCAN_RESULT, connection_handle: 0x%x, status: %d\n", result->connection_handle, result->status);

			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] gatt scan result, connection_handle: 0x%x, status: %d\r\n", result->connection_handle, result->status);
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
			}
			break;
		}
		case BTTASK_IND_LE_GATT_SCAN_DUMP_SERVICE: {
			struct bt_event_le_gatt_scan_dump_service *result =
				(struct bt_event_le_gatt_scan_dump_service *)msg->payload;
			app_ble_dbg("[DEMO] BTTASK_IND_LE_GATT_SCAN_RESULT, connection_handle: 0x%x\n", result->connection_handle);
			gatt_client_asr_service_t *service = &result->service;
			if (service->uuid16) {
				app_ble_dbg("[DEMO]* service: [0x%x-0x%x], uuid 0x%x",
							service->start_group_handle, service->end_group_handle, service->uuid16);
			} else {
				app_ble_dbg("[DEMO]* service: [0x%x-0x%x], uuid  %s",
							service->start_group_handle, service->end_group_handle, ble_demo_uuid128_to_str(service->uuid128));
			}

			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				// snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] gatt scan dump service, connection_handle: 0x%x\r\n", result->connection_handle);
				// ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
				if (service->uuid16) {
					snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] service: [0x%x-0x%x], uuid 0x%x",
							 service->start_group_handle, service->end_group_handle, service->uuid16);
				} else {
					snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] service: [0x%x-0x%x], uuid  %s",
							 service->start_group_handle, service->end_group_handle, ble_demo_uuid128_to_str(service->uuid128));
				}
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
			}
			break;
		}

		case BTTASK_IND_LE_GATT_SCAN_DUMP_CHARACTERISTIC: {
			struct bt_event_le_gatt_scan_dump_characteristic *result =
				(struct bt_event_le_gatt_scan_dump_characteristic *)msg->payload;
			app_ble_dbg("[DEMO] BTTASK_IND_LE_GATT_SCAN_DUMP_CHARACTERISTIC, connection_handle: 0x%x\n", result->connection_handle);
			gatt_client_asr_characteristic_t *characteristic = &result->characteristic;
			if (characteristic->uuid16) {
				app_ble_dbg("[DEMO]*   characteristic: [0x%x-0x%x-0x%x], properties 0x%x(%s), uuid 0x%x",
							characteristic->start_handle, characteristic->value_handle, characteristic->end_handle, characteristic->properties,
							ble_demo_print_properties(characteristic->properties), characteristic->uuid16);
			} else {
				app_ble_dbg("[DEMO]*   characteristic: [0x%x-0x%x-0x%x], properties 0x%x(%s), uuid %s",
							characteristic->start_handle, characteristic->value_handle, characteristic->end_handle, characteristic->properties,
							ble_demo_print_properties(characteristic->properties), ble_demo_uuid128_to_str(characteristic->uuid128));
			}

			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				// snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] gatt scan dump characteristic, connection_handle: 0x%x\r\n", result->connection_handle);
				// ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
				if (characteristic->uuid16) {
					snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] characteristic: [0x%x-0x%x-0x%x], properties 0x%x(%s), uuid 0x%x",
							 characteristic->start_handle, characteristic->value_handle, characteristic->end_handle, characteristic->properties,
							 ble_demo_print_properties(characteristic->properties), characteristic->uuid16);
				} else {
					snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] characteristic: [0x%x-0x%x-0x%x], properties 0x%x(%s), uuid %s",
							 characteristic->start_handle, characteristic->value_handle, characteristic->end_handle, characteristic->properties,
							 ble_demo_print_properties(characteristic->properties), ble_demo_uuid128_to_str(characteristic->uuid128));
				}
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
			}
			break;
		}

		case BTTASK_IND_LE_GATT_SCAN_DUMP_DESCRIPTOR: {
			struct bt_event_le_gatt_scan_dump_descriptor *result =
				(struct bt_event_le_gatt_scan_dump_descriptor *)msg->payload;
			app_ble_dbg("[DEMO] BTTASK_IND_LE_GATT_SCAN_DUMP_DESCRIPTOR, connection_handle: 0x%x\n", result->connection_handle);
			gatt_client_asr_characteristic_descriptor_t *descriptor = &result->descriptor;
			if (descriptor->uuid16) {
				app_ble_dbg("[DEMO]* descriptor, handle: 0x%x, uuid 0x%x", descriptor->handle, descriptor->uuid16);
			} else {
				app_ble_dbg("[DEMO]* descriptor, handle: 0x%x, uuid %s", descriptor->handle, ble_demo_uuid128_to_str(descriptor->uuid128));
			}

			if (IsChipCraneLS() || IsChipCraneG() || IsChipCrane()) {
				// snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] gatt scan dump descriptor, connection_handle: 0x%x\r\n", result->connection_handle);
				// ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
				if (descriptor->uuid16) {
					snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] descriptor, handle: 0x%x, uuid 0x%x", descriptor->handle, descriptor->uuid16);
				} else {
					snprintf(buffer, sizeof(buffer) - 1, "[BTTEST] descriptor, handle: 0x%x, uuid %s", descriptor->handle, ble_demo_uuid128_to_str(descriptor->uuid128));
				}
				ATRESP(1, ATCI_RESULT_CODE_OK, 0, buffer);
			}
			break;
		}

		default:
			app_ble_dbg("[DEMO]handle unknow LE event %d\r\n", msg->event_id);
			break;
	}
}


#endif

#define HID_SERVICE 1





#if HID_SERVICE
uint32_t ble_demo_min(uint32_t a, uint32_t b)
{
    return (a < b) ? a : b;
}

uint16_t ble_demo_little_endian_read_16(const uint8_t *buffer, int position)
{
    return (uint16_t)(((uint16_t) buffer[position]) | (((uint16_t)buffer[position + 1]) << 8));
}

// att_read_callback helpers
uint16_t ble_demo_att_read_callback_handle_blob(const uint8_t *blob, uint16_t blob_size, uint16_t offset, uint8_t *buffer,
                                                       uint16_t buffer_size)
{

    if (buffer != NULL) {
        uint16_t bytes_to_copy = 0;
        if (blob_size >= offset) {
            bytes_to_copy = ble_demo_min(blob_size - offset, buffer_size);
            (void)memcpy(buffer, &blob[offset], bytes_to_copy);
        }
        return bytes_to_copy;
    } else {
        return blob_size;
    }
}

static uint8_t profile_data[] = {
    // ATT DB Version
    1,

    // [0] add GATT Service
    // 0x0001 PRIMARY_SERVICE-GATT_SERVICE
    0x0a, 0x00, 0x02, 0x00, 0x01, 0x00, 0x00, 0x28, 0x01, 0x18, 
    // 0x0002 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_GATT_SERVICE_CHANGED-INDICATE
    0x0d, 0x00, 0x02, 0x00, 0x02, 0x00, 0x03, 0x28, 0x20, 0x03, 0x00, 0x05, 0x2a, 
    // 0x0003 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_GATT_SERVICE_CHANGED-INDICATE-''
    // 
    0x08, 0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x2a, 
    // 0x0004 CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0x01, 0x04, 0x00, 0x02, 0x29, 0x00, 0x00, 
    // [1] add GAP Service

    // 0x0005 PRIMARY_SERVICE-GAP_SERVICE
    0x0a, 0x00, 0x02, 0x00, 0x05, 0x00, 0x00, 0x28, 0x00, 0x18, 
    // 0x0006 CHARACTERISTIC-GAP_DEVICE_NAME-READ |
    0x0d, 0x00, 0x02, 0x00, 0x06, 0x00, 0x03, 0x28, 0x02, 0x07, 0x00, 0x00, 0x2a, 
    // 0x0007 VALUE-GAP_DEVICE_NAME-READ |-'BLE HID DEMO'
    // READ_ANYBODY
    0x14, 0x00, 0x02, 0x00, 0x07, 0x00, 0x00, 0x2a, 0x42, 0x4c, 0x45, 0x20, 0x48, 0x49, 0x44, 0x20, 0x44, 0x45, 0x4d, 0x4f, 
    // 0x0008 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_GAP_APPEARANCE-READ
    0x0d, 0x00, 0x02, 0x00, 0x08, 0x00, 0x03, 0x28, 0x02, 0x09, 0x00, 0x01, 0x2a, 
    // 0x0009 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_GAP_APPEARANCE-READ-'C0 03'
    // READ_ANYBODY
    0x0a, 0x00, 0x02, 0x00, 0x09, 0x00, 0x01, 0x2a, 0xC0, 0x03, 
    // [2] add Customed Service
    // Counter Service

    // 0x000a PRIMARY_SERVICE-A2345678-B234-C234-D234-E23456789ABC
    0x18, 0x00, 0x02, 0x00, 0x0a, 0x00, 0x00, 0x28, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0xe2, 0x34, 0xd2, 0x34, 0xc2, 0x34, 0xb2, 0x78, 0x56, 0x34, 0xa2, 
    // Counter Characteristic, with read and write and notify
    // 0x000b CHARACTERISTIC-AFAEADAC-ABAA-A9A8-A7A6-A5A4A3A2A1A0-READ | WRITE | NOTIFY | DYNAMIC
    0x1b, 0x00, 0x02, 0x00, 0x0b, 0x00, 0x03, 0x28, 0x1a, 0x0c, 0x00, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 
    // 0x000c VALUE-AFAEADAC-ABAA-A9A8-A7A6-A5A4A3A2A1A0-READ | WRITE | NOTIFY | DYNAMIC-''
    // READ_ANYBODY, WRITE_ANYBODY
    0x16, 0x00, 0x0a, 0x03, 0x0c, 0x00, 0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9, 0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf, 
    // 0x000d CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0x01, 0x0d, 0x00, 0x02, 0x29, 0x00, 0x00, 
    // 0x000e CHARACTERISTIC-12345678-1234-5678-ABCD-BA9876543210-READ | WRITE | NOTIFY | DYNAMIC
    0x1b, 0x00, 0x02, 0x00, 0x0e, 0x00, 0x03, 0x28, 0x1a, 0x0f, 0x00, 0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xcd, 0xab, 0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12, 
    // 0x000f VALUE-12345678-1234-5678-ABCD-BA9876543210-READ | WRITE | NOTIFY | DYNAMIC-''
    // READ_ANYBODY, WRITE_ANYBODY
    0x16, 0x00, 0x0a, 0x03, 0x0f, 0x00, 0x10, 0x32, 0x54, 0x76, 0x98, 0xba, 0xcd, 0xab, 0x78, 0x56, 0x34, 0x12, 0x78, 0x56, 0x34, 0x12, 
    // 0x0010 CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0x01, 0x10, 0x00, 0x02, 0x29, 0x00, 0x00, 
    // [3] add HID Service
    // Specification Type org.bluetooth.service.human_interface_device
    // https://www.bluetooth.com/api/gatt/xmlfile?xmlFileName=org.bluetooth.service.human_interface_device.xml
    // Human Interface Device 1812

    // 0x0011 PRIMARY_SERVICE-ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE
    0x0a, 0x00, 0x02, 0x00, 0x11, 0x00, 0x00, 0x28, 0x12, 0x18, 
    // 0x0012 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_PROTOCOL_MODE-DYNAMIC | READ | WRITE_WITHOUT_RESPONSE
    0x0d, 0x00, 0x02, 0x00, 0x12, 0x00, 0x03, 0x28, 0x06, 0x13, 0x00, 0x4e, 0x2a, 
    // 0x0013 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_PROTOCOL_MODE-DYNAMIC | READ | WRITE_WITHOUT_RESPONSE-''
    // READ_ANYBODY, WRITE_ANYBODY
    0x08, 0x00, 0x06, 0x01, 0x13, 0x00, 0x4e, 0x2a, 
    // 0x0014 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT-DYNAMIC | READ | WRITE | NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x14, 0x00, 0x03, 0x28, 0x1a, 0x15, 0x00, 0x4d, 0x2a, 
    // 0x0015 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_REPORT-DYNAMIC | READ | WRITE | NOTIFY-''
    // READ_ANYBODY, WRITE_ANYBODY
    0x08, 0x00, 0x0a, 0x01, 0x15, 0x00, 0x4d, 0x2a, 
    // 0x0016 CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0x01, 0x16, 0x00, 0x02, 0x29, 0x00, 0x00, 
    // fixed report id = 1, type = Input (1)
    // 0x0017 REPORT_REFERENCE-READ-1-1
    0x0a, 0x00, 0x02, 0x00, 0x17, 0x00, 0x08, 0x29, 0x1, 0x1, 
    // 0x0018 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT-DYNAMIC | READ | WRITE | NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x18, 0x00, 0x03, 0x28, 0x1a, 0x19, 0x00, 0x4d, 0x2a, 
    // 0x0019 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_REPORT-DYNAMIC | READ | WRITE | NOTIFY-''
    // READ_ANYBODY, WRITE_ANYBODY
    0x08, 0x00, 0x0a, 0x01, 0x19, 0x00, 0x4d, 0x2a, 
    // 0x001a CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0x01, 0x1a, 0x00, 0x02, 0x29, 0x00, 0x00, 
    // fixed report id = 2, type = Output (2)
    // 0x001b REPORT_REFERENCE-READ-2-2
    0x0a, 0x00, 0x02, 0x00, 0x1b, 0x00, 0x08, 0x29, 0x2, 0x2, 
    // 0x001c CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT-DYNAMIC | READ | WRITE | NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x1c, 0x00, 0x03, 0x28, 0x1a, 0x1d, 0x00, 0x4d, 0x2a, 
    // 0x001d VALUE-ORG_BLUETOOTH_CHARACTERISTIC_REPORT-DYNAMIC | READ | WRITE | NOTIFY-''
    // READ_ANYBODY, WRITE_ANYBODY
    0x08, 0x00, 0x0a, 0x01, 0x1d, 0x00, 0x4d, 0x2a, 
    // 0x001e CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0x01, 0x1e, 0x00, 0x02, 0x29, 0x00, 0x00, 
    // fixed report id = 3, type = Feature (3)
    // 0x001f REPORT_REFERENCE-READ-3-3
    0x0a, 0x00, 0x02, 0x00, 0x1f, 0x00, 0x08, 0x29, 0x3, 0x3, 
    // 0x0020 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_REPORT_MAP-DYNAMIC | READ
    0x0d, 0x00, 0x02, 0x00, 0x20, 0x00, 0x03, 0x28, 0x02, 0x21, 0x00, 0x4b, 0x2a, 
    // 0x0021 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_REPORT_MAP-DYNAMIC | READ-''
    // READ_ANYBODY
    0x08, 0x00, 0x02, 0x01, 0x21, 0x00, 0x4b, 0x2a, 
    // 0x0022 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_INPUT_REPORT-DYNAMIC | READ | WRITE | NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x22, 0x00, 0x03, 0x28, 0x1a, 0x23, 0x00, 0x22, 0x2a, 
    // 0x0023 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_INPUT_REPORT-DYNAMIC | READ | WRITE | NOTIFY-''
    // READ_ANYBODY, WRITE_ANYBODY
    0x08, 0x00, 0x0a, 0x01, 0x23, 0x00, 0x22, 0x2a, 
    // 0x0024 CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0x01, 0x24, 0x00, 0x02, 0x29, 0x00, 0x00, 
    // 0x0025 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_OUTPUT_REPORT-DYNAMIC | READ | WRITE | WRITE_WITHOUT_RESPONSE
    0x0d, 0x00, 0x02, 0x00, 0x25, 0x00, 0x03, 0x28, 0x0e, 0x26, 0x00, 0x32, 0x2a, 
    // 0x0026 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_OUTPUT_REPORT-DYNAMIC | READ | WRITE | WRITE_WITHOUT_RESPONSE-''
    // READ_ANYBODY, WRITE_ANYBODY
    0x08, 0x00, 0x0e, 0x01, 0x26, 0x00, 0x32, 0x2a, 
    // 0x0027 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_BOOT_MOUSE_INPUT_REPORT-DYNAMIC | READ | WRITE | NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x27, 0x00, 0x03, 0x28, 0x1a, 0x28, 0x00, 0x33, 0x2a, 
    // 0x0028 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_BOOT_MOUSE_INPUT_REPORT-DYNAMIC | READ | WRITE | NOTIFY-''
    // READ_ANYBODY, WRITE_ANYBODY
    0x08, 0x00, 0x0a, 0x01, 0x28, 0x00, 0x33, 0x2a, 
    // 0x0029 CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0x01, 0x29, 0x00, 0x02, 0x29, 0x00, 0x00, 
    // bcdHID = 0x101 (v1.0.1), bCountryCode 0, remote wakeable = 0 | normally connectable 2
    // 0x002a CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_HID_INFORMATION-READ
    0x0d, 0x00, 0x02, 0x00, 0x2a, 0x00, 0x03, 0x28, 0x02, 0x2b, 0x00, 0x4a, 0x2a, 
    // 0x002b VALUE-ORG_BLUETOOTH_CHARACTERISTIC_HID_INFORMATION-READ-'01 01 00 02'
    // READ_ANYBODY
    0x0c, 0x00, 0x02, 0x00, 0x2b, 0x00, 0x4a, 0x2a, 0x01, 0x01, 0x00, 0x02, 
    // 0x002c CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_HID_CONTROL_POINT-DYNAMIC | WRITE_WITHOUT_RESPONSE
    0x0d, 0x00, 0x02, 0x00, 0x2c, 0x00, 0x03, 0x28, 0x04, 0x2d, 0x00, 0x4c, 0x2a, 
    // 0x002d VALUE-ORG_BLUETOOTH_CHARACTERISTIC_HID_CONTROL_POINT-DYNAMIC | WRITE_WITHOUT_RESPONSE-''
    // WRITE_ANYBODY
    0x08, 0x00, 0x04, 0x01, 0x2d, 0x00, 0x4c, 0x2a, 
    // [4] add Battery Service
    // Specification Type org.bluetooth.service.battery_service
    // https://www.bluetooth.com/api/gatt/xmlfile?xmlFileName=org.bluetooth.service.battery_service.xml
    // Battery Service 180F

    // 0x002e PRIMARY_SERVICE-ORG_BLUETOOTH_SERVICE_BATTERY_SERVICE
    0x0a, 0x00, 0x02, 0x00, 0x2e, 0x00, 0x00, 0x28, 0x0f, 0x18, 
    // 0x002f CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL-DYNAMIC | READ | NOTIFY
    0x0d, 0x00, 0x02, 0x00, 0x2f, 0x00, 0x03, 0x28, 0x12, 0x30, 0x00, 0x19, 0x2a, 
    // 0x0030 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL-DYNAMIC | READ | NOTIFY-''
    // READ_ANYBODY
    0x08, 0x00, 0x02, 0x01, 0x30, 0x00, 0x19, 0x2a, 
    // 0x0031 CLIENT_CHARACTERISTIC_CONFIGURATION
    // READ_ANYBODY, WRITE_ANYBODY
    0x0a, 0x00, 0x0e, 0x01, 0x31, 0x00, 0x02, 0x29, 0x00, 0x00, 
    // [5] add Device ID Service
    // Specification Type org.bluetooth.service.device_information
    // https://www.bluetooth.com/api/gatt/xmlfile?xmlFileName=org.bluetooth.service.device_information.xml
    // Device Information 180A

    // 0x0032 PRIMARY_SERVICE-ORG_BLUETOOTH_SERVICE_DEVICE_INFORMATION
    0x0a, 0x00, 0x02, 0x00, 0x32, 0x00, 0x00, 0x28, 0x0a, 0x18, 
    // 0x0033 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_MANUFACTURER_NAME_STRING-READ | ENCRYPTION_KEY_SIZE_16
    0x0d, 0x00, 0x02, 0x00, 0x33, 0x00, 0x03, 0x28, 0x02, 0x34, 0x00, 0x29, 0x2a, 
    // 0x0034 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_MANUFACTURER_NAME_STRING-READ | ENCRYPTION_KEY_SIZE_16-'QUEC manufacturer_name_string'
    // READ_ENCRYPTED, ENCRYPTION_KEY_SIZE=16
    0x25, 0x00, 0x03, 0xf4, 0x34, 0x00, 0x29, 0x2a, 0x51, 0x55, 0x45, 0x43, 0x20, 0x6d, 0x61, 0x6e, 0x75, 0x66, 0x61, 0x63, 0x74, 0x75, 0x72, 0x65, 0x72, 0x5f, 0x6e, 0x61, 0x6d, 0x65, 0x5f, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 
    // 0x0035 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_MODEL_NUMBER_STRING-READ
    0x0d, 0x00, 0x02, 0x00, 0x35, 0x00, 0x03, 0x28, 0x02, 0x36, 0x00, 0x24, 0x2a, 
    // 0x0036 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_MODEL_NUMBER_STRING-READ-'QUEC model_number_string'
    // READ_ANYBODY
    0x20, 0x00, 0x02, 0x00, 0x36, 0x00, 0x24, 0x2a, 0x51, 0x55, 0x45, 0x43, 0x20, 0x6d, 0x6f, 0x64, 0x65, 0x6c, 0x5f, 0x6e, 0x75, 0x6d, 0x62, 0x65, 0x72, 0x5f, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 
    // 0x0037 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_SERIAL_NUMBER_STRING-READ
    0x0d, 0x00, 0x02, 0x00, 0x37, 0x00, 0x03, 0x28, 0x02, 0x38, 0x00, 0x25, 0x2a, 
    // 0x0038 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_SERIAL_NUMBER_STRING-READ-'QUEC serial_number_string'
    // READ_ANYBODY
    0x21, 0x00, 0x02, 0x00, 0x38, 0x00, 0x25, 0x2a, 0x51, 0x55, 0x45, 0x43, 0x20, 0x73, 0x65, 0x72, 0x69, 0x61, 0x6c, 0x5f, 0x6e, 0x75, 0x6d, 0x62, 0x65, 0x72, 0x5f, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 
    // 0x0039 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_HARDWARE_REVISION_STRING-READ
    0x0d, 0x00, 0x02, 0x00, 0x39, 0x00, 0x03, 0x28, 0x02, 0x3a, 0x00, 0x27, 0x2a, 
    // 0x003a VALUE-ORG_BLUETOOTH_CHARACTERISTIC_HARDWARE_REVISION_STRING-READ-'QUEC hardware_revision_string'
    // READ_ANYBODY
    0x25, 0x00, 0x02, 0x00, 0x3a, 0x00, 0x27, 0x2a, 0x51, 0x55, 0x45, 0x43, 0x20, 0x68, 0x61, 0x72, 0x64, 0x77, 0x61, 0x72, 0x65, 0x5f, 0x72, 0x65, 0x76, 0x69, 0x73, 0x69, 0x6f, 0x6e, 0x5f, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 
    // 0x003b CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_FIRMWARE_REVISION_STRING-READ
    0x0d, 0x00, 0x02, 0x00, 0x3b, 0x00, 0x03, 0x28, 0x02, 0x3c, 0x00, 0x26, 0x2a, 
    // 0x003c VALUE-ORG_BLUETOOTH_CHARACTERISTIC_FIRMWARE_REVISION_STRING-READ-'QUEC firmware_revision_string'
    // READ_ANYBODY
    0x25, 0x00, 0x02, 0x00, 0x3c, 0x00, 0x26, 0x2a, 0x51, 0x55, 0x45, 0x43, 0x20, 0x66, 0x69, 0x72, 0x6d, 0x77, 0x61, 0x72, 0x65, 0x5f, 0x72, 0x65, 0x76, 0x69, 0x73, 0x69, 0x6f, 0x6e, 0x5f, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 
    // 0x003d CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_SOFTWARE_REVISION_STRING-READ
    0x0d, 0x00, 0x02, 0x00, 0x3d, 0x00, 0x03, 0x28, 0x02, 0x3e, 0x00, 0x28, 0x2a, 
    // 0x003e VALUE-ORG_BLUETOOTH_CHARACTERISTIC_SOFTWARE_REVISION_STRING-READ-'QUEC software_revision_string'
    // READ_ANYBODY
    0x25, 0x00, 0x02, 0x00, 0x3e, 0x00, 0x28, 0x2a, 0x51, 0x55, 0x45, 0x43, 0x20, 0x73, 0x6f, 0x66, 0x74, 0x77, 0x61, 0x72, 0x65, 0x5f, 0x72, 0x65, 0x76, 0x69, 0x73, 0x69, 0x6f, 0x6e, 0x5f, 0x73, 0x74, 0x72, 0x69, 0x6e, 0x67, 
    // 0x003f CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_SYSTEM_ID-READ
    0x0d, 0x00, 0x02, 0x00, 0x3f, 0x00, 0x03, 0x28, 0x02, 0x40, 0x00, 0x23, 0x2a, 
    // 0x0040 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_SYSTEM_ID-READ-'00 01 02 03 04 05 06'
    // READ_ANYBODY
    0x0f, 0x00, 0x02, 0x00, 0x40, 0x00, 0x23, 0x2a, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 
    // 0x0041 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_IEEE_11073_20601_REGULATORY_CERTIFICATION_DATA_LIST-READ
    0x0d, 0x00, 0x02, 0x00, 0x41, 0x00, 0x03, 0x28, 0x02, 0x42, 0x00, 0x2a, 0x2a, 
    // 0x0042 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_IEEE_11073_20601_REGULATORY_CERTIFICATION_DATA_LIST-READ-'00 01 02 03'
    // READ_ANYBODY
    0x0c, 0x00, 0x02, 0x00, 0x42, 0x00, 0x2a, 0x2a, 0x00, 0x01, 0x02, 0x03, 
    // 0x0043 CHARACTERISTIC-ORG_BLUETOOTH_CHARACTERISTIC_PNP_ID-READ
    0x0d, 0x00, 0x02, 0x00, 0x43, 0x00, 0x03, 0x28, 0x02, 0x44, 0x00, 0x50, 0x2a, 
    // 0x0044 VALUE-ORG_BLUETOOTH_CHARACTERISTIC_PNP_ID-READ-'00 01 02 03 04 05 06'
    // READ_ANYBODY
    0x0f, 0x00, 0x02, 0x00, 0x44, 0x00, 0x50, 0x2a, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 

    // END
    0x00, 0x00, 
};
	 // total size 429 bytes 
	
	
	//
	// list service handle ranges
	//
#define ATT_SERVICE_GATT_SERVICE_START_HANDLE 0x0001
#define ATT_SERVICE_GATT_SERVICE_END_HANDLE 0x0004
#define ATT_SERVICE_GATT_SERVICE_01_START_HANDLE 0x0001
#define ATT_SERVICE_GATT_SERVICE_01_END_HANDLE 0x0004
#define ATT_SERVICE_GAP_SERVICE_START_HANDLE 0x0005
#define ATT_SERVICE_GAP_SERVICE_END_HANDLE 0x0009
#define ATT_SERVICE_GAP_SERVICE_01_START_HANDLE 0x0005
#define ATT_SERVICE_GAP_SERVICE_01_END_HANDLE 0x0009
#define ATT_SERVICE_A2345678_B234_C234_D234_E23456789ABC_START_HANDLE 0x000a
#define ATT_SERVICE_A2345678_B234_C234_D234_E23456789ABC_END_HANDLE 0x0010
#define ATT_SERVICE_A2345678_B234_C234_D234_E23456789ABC_01_START_HANDLE 0x000a
#define ATT_SERVICE_A2345678_B234_C234_D234_E23456789ABC_01_END_HANDLE 0x0010
#define ATT_SERVICE_ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE_START_HANDLE 0x0011
#define ATT_SERVICE_ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE_END_HANDLE 0x002d
#define ATT_SERVICE_ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE_01_START_HANDLE 0x0011
#define ATT_SERVICE_ORG_BLUETOOTH_SERVICE_HUMAN_INTERFACE_DEVICE_01_END_HANDLE 0x002d
#define ATT_SERVICE_ORG_BLUETOOTH_SERVICE_BATTERY_SERVICE_START_HANDLE 0x002e
#define ATT_SERVICE_ORG_BLUETOOTH_SERVICE_BATTERY_SERVICE_END_HANDLE 0x0031
#define ATT_SERVICE_ORG_BLUETOOTH_SERVICE_BATTERY_SERVICE_01_START_HANDLE 0x002e
#define ATT_SERVICE_ORG_BLUETOOTH_SERVICE_BATTERY_SERVICE_01_END_HANDLE 0x0031
#define ATT_SERVICE_ORG_BLUETOOTH_SERVICE_DEVICE_INFORMATION_START_HANDLE 0x0032
#define ATT_SERVICE_ORG_BLUETOOTH_SERVICE_DEVICE_INFORMATION_END_HANDLE 0x0044
#define ATT_SERVICE_ORG_BLUETOOTH_SERVICE_DEVICE_INFORMATION_01_START_HANDLE 0x0032
#define ATT_SERVICE_ORG_BLUETOOTH_SERVICE_DEVICE_INFORMATION_01_END_HANDLE 0x0044
	
	//
	// list mapping between characteristics and handles
	//
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_GATT_SERVICE_CHANGED_01_VALUE_HANDLE 0x0003
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_GATT_SERVICE_CHANGED_01_CLIENT_CONFIGURATION_HANDLE 0x0004
#define ATT_CHARACTERISTIC_GAP_DEVICE_NAME_01_VALUE_HANDLE 0x0007
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_GAP_APPEARANCE_01_VALUE_HANDLE 0x0009
#define ATT_CHARACTERISTIC_AFAEADAC_ABAA_A9A8_A7A6_A5A4A3A2A1A0_01_VALUE_HANDLE 0x000c
#define ATT_CHARACTERISTIC_AFAEADAC_ABAA_A9A8_A7A6_A5A4A3A2A1A0_01_CLIENT_CONFIGURATION_HANDLE 0x000d
#define ATT_CHARACTERISTIC_12345678_1234_5678_ABCD_BA9876543210_01_VALUE_HANDLE 0x000f
#define ATT_CHARACTERISTIC_12345678_1234_5678_ABCD_BA9876543210_01_CLIENT_CONFIGURATION_HANDLE 0x0010
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_PROTOCOL_MODE_01_VALUE_HANDLE 0x0013
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_01_VALUE_HANDLE 0x0015
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_01_CLIENT_CONFIGURATION_HANDLE 0x0016
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_02_VALUE_HANDLE 0x0019
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_02_CLIENT_CONFIGURATION_HANDLE 0x001a
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_03_VALUE_HANDLE 0x001d
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_03_CLIENT_CONFIGURATION_HANDLE 0x001e
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_MAP_01_VALUE_HANDLE 0x0021
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_INPUT_REPORT_01_VALUE_HANDLE 0x0023
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_INPUT_REPORT_01_CLIENT_CONFIGURATION_HANDLE 0x0024
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_OUTPUT_REPORT_01_VALUE_HANDLE 0x0026
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_MOUSE_INPUT_REPORT_01_VALUE_HANDLE 0x0028
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_MOUSE_INPUT_REPORT_01_CLIENT_CONFIGURATION_HANDLE 0x0029
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_HID_INFORMATION_01_VALUE_HANDLE 0x002b
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_HID_CONTROL_POINT_01_VALUE_HANDLE 0x002d
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL_01_VALUE_HANDLE 0x0030
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL_01_CLIENT_CONFIGURATION_HANDLE 0x0031
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_MANUFACTURER_NAME_STRING_01_VALUE_HANDLE 0x0034
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_MODEL_NUMBER_STRING_01_VALUE_HANDLE 0x0036
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_SERIAL_NUMBER_STRING_01_VALUE_HANDLE 0x0038
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_HARDWARE_REVISION_STRING_01_VALUE_HANDLE 0x003a
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_FIRMWARE_REVISION_STRING_01_VALUE_HANDLE 0x003c
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_SOFTWARE_REVISION_STRING_01_VALUE_HANDLE 0x003e
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_SYSTEM_ID_01_VALUE_HANDLE 0x0040
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_IEEE_11073_20601_REGULATORY_CERTIFICATION_DATA_LIST_01_VALUE_HANDLE 0x0042
#define ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_PNP_ID_01_VALUE_HANDLE 0x0044

#define BAS_SUPPORT    1
#define CUSTOM_SUPPORT    1
#define HID_SUPPORT    1

static uint16_t att_read_callback(uint16_t con_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer, uint16_t buffer_size);
static int att_write_callback(uint16_t con_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer,
                              uint16_t buffer_size);
// test param
static uint8_t test_value = 0;
static uint8_t test_buf[256];

static uint8_t  service_changed;
static uint16_t service_changed_client_configuration;

#ifdef BAS_SUPPORT
static uint8_t  battery_value;
static uint16_t battery_value_client_configuration;
#endif
#ifdef CUSTOM_SUPPORT
static uint8_t  custom_value1[256];
static uint16_t custom_value_client_configuration1;
static uint8_t  custom_value2[20];
static uint16_t custom_value_client_configuration2;
#endif
#ifdef HID_SUPPORT
static uint8_t  hid_protocol_mode = 1; // 0: boot mode, 1: report mode
static uint8_t  hid_control_point = 1; // 0: suspend, 1: running
static uint16_t hid_report_input_input_value = 0;
static uint16_t hid_report_input_input_value_client_configuration = 0;
static uint16_t hid_report_output_input_value = 0;
static uint16_t hid_report_output_input_value_client_configuration = 0;
static uint16_t hid_report_feature_input_value = 0;
static uint16_t hid_report_feature_input_value_client_configuration = 0;
static uint16_t hid_boot_mouse_input_value = 0;
static uint16_t hid_boot_mouse_input_value_client_configuration = 0;
static uint16_t hid_boot_keyboard_input_value = 0;
static uint16_t hid_boot_keyboard_input_value_client_configuration = 0;

// from USB HID Specification 1.1, Appendix B.1
static const uint8_t hid_report_map[] = {

    0x05, 0x01,                    // Usage Page (Generic Desktop)
    0x09, 0x06,                    // Usage (Keyboard)
    0xa1, 0x01,                    // Collection (Application)

    0x85,  0x01,                   // Report ID 1

    // Modifier byte

    0x75, 0x01,                    //   Report Size (1)
    0x95, 0x08,                    //   Report Count (8)
    0x05, 0x07,                    //   Usage Page (Key codes)
    0x19, 0xe0,                    //   Usage Minimum (Keyboard LeftControl)
    0x29, 0xe7,                    //   Usage Maxium (Keyboard Right GUI)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x25, 0x01,                    //   Logical Maximum (1)
    0x81, 0x02,                    //   Input (Data, Variable, Absolute)

    // Reserved byte

    0x75, 0x01,                    //   Report Size (1)
    0x95, 0x08,                    //   Report Count (8)
    0x81, 0x03,                    //   Input (Constant, Variable, Absolute)

    // LED report + padding

    0x95, 0x05,                    //   Report Count (5)
    0x75, 0x01,                    //   Report Size (1)
    0x05, 0x08,                    //   Usage Page (LEDs)
    0x19, 0x01,                    //   Usage Minimum (Num Lock)
    0x29, 0x05,                    //   Usage Maxium (Kana)
    0x91, 0x02,                    //   Output (Data, Variable, Absolute)

    0x95, 0x01,                    //   Report Count (1)
    0x75, 0x03,                    //   Report Size (3)
    0x91, 0x03,                    //   Output (Constant, Variable, Absolute)

    // Keycodes

    0x95, 0x06,                    //   Report Count (6)
    0x75, 0x08,                    //   Report Size (8)
    0x15, 0x00,                    //   Logical Minimum (0)
    0x25, 0xff,                    //   Logical Maximum (1)
    0x05, 0x07,                    //   Usage Page (Key codes)
    0x19, 0x00,                    //   Usage Minimum (Reserved (no event indicated))
    0x29, 0xff,                    //   Usage Maxium (Reserved)
    0x81, 0x00,                    //   Input (Data, Array)

    0xc0,                          // End collection
};
#endif

static uint16_t att_read_callback(uint16_t connection_handle, uint16_t att_handle, uint16_t offset, uint8_t *buffer,
                                  uint16_t buffer_size)
{

    ql_app_log("[DEMO][%s] connection_handle: %d, att_handle: 0x%04x, offset: %d, buffer: 0x%x, buffer_size: %d", __func__, connection_handle,
                att_handle, offset,
                buffer, buffer_size);

    if (buffer == NULL) {
        ql_app_log("[DEMO][%s] used for check value len", __func__);
    }

    switch (att_handle) {
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_GATT_SERVICE_CHANGED_01_CLIENT_CONFIGURATION_HANDLE:
            if (buffer != NULL) {
                service_changed++;
                ql_app_log("[DEMO][%s] service_changed_client_configuration: %d, service_changed: %d", __func__, service_changed_client_configuration, service_changed);
            }
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)&service_changed_client_configuration, sizeof(service_changed_client_configuration), offset,
                                                          buffer, buffer_size);
#ifdef BAS_SUPPORT
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL_01_VALUE_HANDLE:
            if (buffer != NULL) {
                battery_value++;
                ql_app_log("[DEMO][%s] battery_value: %d", __func__, battery_value);
            }
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)&battery_value, sizeof(battery_value), offset, buffer, buffer_size);
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL_01_CLIENT_CONFIGURATION_HANDLE:
            if (buffer != NULL) {
                battery_value++;
                ql_app_log("[DEMO][%s] battery_value_client_configuration: %d, battery_value: %d", __func__, battery_value_client_configuration, battery_value);
            }
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)&battery_value_client_configuration, sizeof(battery_value_client_configuration), offset, buffer,
                                                          buffer_size);
#endif
#ifdef CUSTOM_SUPPORT
        case ATT_CHARACTERISTIC_AFAEADAC_ABAA_A9A8_A7A6_A5A4A3A2A1A0_01_VALUE_HANDLE:
            if (buffer != NULL) {
                memcpy(custom_value1, test_buf, sizeof(custom_value1));
                ql_app_log("[DEMO][%s] custom_value1: %d", __func__, custom_value1);
            }
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)custom_value1, sizeof(custom_value1), offset, buffer, buffer_size);
        case ATT_CHARACTERISTIC_AFAEADAC_ABAA_A9A8_A7A6_A5A4A3A2A1A0_01_CLIENT_CONFIGURATION_HANDLE:
            if (buffer != NULL) {
                custom_value1[0]++;
                ql_app_log("[DEMO][%s] custom_value_client_configuration1: %d, custom_value1: %d", __func__, custom_value_client_configuration1, custom_value1[0]);
            }
            // att_server_request_can_send_now_event(connection_handle);
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)&custom_value_client_configuration1, sizeof(custom_value_client_configuration1), offset, buffer,
                                                          buffer_size);
        case ATT_CHARACTERISTIC_12345678_1234_5678_ABCD_BA9876543210_01_VALUE_HANDLE:
            if (buffer != NULL) {
                memcpy(custom_value2, test_buf, sizeof(custom_value2));
                ql_app_log("[DEMO][%s] custom_value2: %d", __func__, custom_value2);
            }
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)custom_value2, sizeof(custom_value2), offset, buffer, buffer_size);
        case ATT_CHARACTERISTIC_12345678_1234_5678_ABCD_BA9876543210_01_CLIENT_CONFIGURATION_HANDLE:
            if (buffer != NULL) {
                custom_value2[0]++;
                ql_app_log("[DEMO][%s] custom_value_client_configuration2: %d, custom_value2: %d", __func__, custom_value_client_configuration2, custom_value2[0]);
            }
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)&custom_value_client_configuration2, sizeof(custom_value_client_configuration2), offset, buffer,
                                                          buffer_size);
#endif
#ifdef HID_SUPPORT
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_PROTOCOL_MODE_01_VALUE_HANDLE:
            if (buffer != NULL) {
                ql_app_log("[DEMO][%s] hid_protocol_mode: %d", __func__, hid_protocol_mode);
            }
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_protocol_mode, sizeof(hid_protocol_mode), offset, buffer, buffer_size);
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_MAP_01_VALUE_HANDLE:
            if (buffer != NULL) {
                ql_app_log("[DEMO][%s] hid_report_map: %d", __func__, hid_report_map);
            }
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_report_map, sizeof(hid_report_map), offset, buffer, buffer_size);
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_HID_CONTROL_POINT_01_VALUE_HANDLE:
            if (buffer != NULL) {
                ql_app_log("[DEMO][%s] hid_control_point: %d", __func__, hid_control_point);
            }
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_control_point, sizeof(hid_control_point), offset, buffer, buffer_size);
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_01_CLIENT_CONFIGURATION_HANDLE:
            if (buffer != NULL) {
                ql_app_log("[DEMO][%s] hid_report_input_input_value_client_configuration: %d", __func__, hid_report_input_input_value_client_configuration);
            }
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_report_input_input_value_client_configuration,
                                                          sizeof(hid_report_input_input_value_client_configuration), offset, buffer, buffer_size);
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_02_CLIENT_CONFIGURATION_HANDLE:
            if (buffer != NULL) {
                ql_app_log("[DEMO][%s] hid_report_output_input_value_client_configuration: %d", __func__, hid_report_output_input_value_client_configuration);
            }
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_report_output_input_value_client_configuration,
                                                          sizeof(hid_report_output_input_value_client_configuration), offset, buffer, buffer_size);
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_03_CLIENT_CONFIGURATION_HANDLE:
            if (buffer != NULL) {
                ql_app_log("[DEMO][%s] hid_report_feature_input_value_client_configuration: %d", __func__, hid_report_feature_input_value_client_configuration);
            }
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_report_feature_input_value_client_configuration,
                                                          sizeof(hid_report_feature_input_value_client_configuration), offset, buffer, buffer_size);
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_INPUT_REPORT_01_CLIENT_CONFIGURATION_HANDLE:
            if (buffer != NULL) {
                ql_app_log("[DEMO][%s] hid_boot_mouse_input_value_client_configuration: %d", __func__, hid_boot_mouse_input_value_client_configuration);
            }
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_boot_mouse_input_value_client_configuration,
                                                          sizeof(hid_boot_mouse_input_value_client_configuration),
                                                          offset, buffer, buffer_size);
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_MOUSE_INPUT_REPORT_01_CLIENT_CONFIGURATION_HANDLE:
            if (buffer != NULL) {
                ql_app_log("[DEMO][%s] hid_boot_keyboard_input_value_client_configuration: %d", __func__, hid_boot_keyboard_input_value_client_configuration);
            }
            return ble_demo_att_read_callback_handle_blob((const uint8_t *)&hid_boot_keyboard_input_value_client_configuration,
                                                          sizeof(hid_boot_keyboard_input_value_client_configuration), offset, buffer, buffer_size);
#endif
        default:
            break;
    }

    return 0;
}

static int att_write_callback(uint16_t connection_handle, uint16_t att_handle, uint16_t transaction_mode, uint16_t offset, uint8_t *buffer,
                              uint16_t buffer_size)
{

    ql_app_log("[DEMO][%s] connection_handle: %d, att_handle: 0x%04x, offset: %d, buffer_size: %d", __func__, connection_handle, att_handle, offset, buffer_size);
    ql_app_log("att_write_callback:", buffer, buffer_size);

    switch (att_handle) {
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_GATT_SERVICE_CHANGED_01_CLIENT_CONFIGURATION_HANDLE:
            service_changed_client_configuration = ble_demo_little_endian_read_16(buffer, 0);
            ql_app_log("[DEMO][%s] service_changed_client_configuration: %d", __func__, service_changed_client_configuration);
            return 0;
#ifdef BAS_SUPPORT
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BATTERY_LEVEL_01_CLIENT_CONFIGURATION_HANDLE:
            battery_value_client_configuration = ble_demo_little_endian_read_16(buffer, 0);
            ql_app_log("[DEMO][%s] battery_value_client_configuration: %d", __func__, battery_value_client_configuration);
            return 0;
#endif
#ifdef CUSTOM_SUPPORT
        case ATT_CHARACTERISTIC_AFAEADAC_ABAA_A9A8_A7A6_A5A4A3A2A1A0_01_CLIENT_CONFIGURATION_HANDLE:
            custom_value_client_configuration1 = ble_demo_little_endian_read_16(buffer, 0);
#ifdef BLE_DEMO_LOOP_SEND_TEST
            ble_demo_loop_test_ccc = custom_value_client_configuration1;
#endif
            ql_app_log("[DEMO][%s] custom_value_client_configuration1: %d", __func__, custom_value_client_configuration1);
            return 0;
        case ATT_CHARACTERISTIC_12345678_1234_5678_ABCD_BA9876543210_01_CLIENT_CONFIGURATION_HANDLE:
            custom_value_client_configuration2 = ble_demo_little_endian_read_16(buffer, 0);
            ql_app_log("[DEMO][%s] custom_value_client_configuration2: %d", __func__, custom_value_client_configuration2);
            return 0;
#endif
#ifdef HID_SUPPORT
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_PROTOCOL_MODE_01_VALUE_HANDLE:
            hid_protocol_mode = buffer[0];
            ql_app_log("[DEMO][%s] hid_protocol_mode: %d(0: boot mode, 1: report mode)", __func__, hid_protocol_mode);
            return 0;
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_HID_CONTROL_POINT_01_VALUE_HANDLE:
            hid_control_point = buffer[0];
            ql_app_log("[DEMO][%s] hid_control_point: %d(0: suspend, 1: running)", __func__, hid_control_point);
            return 0;
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_01_CLIENT_CONFIGURATION_HANDLE:
            hid_report_input_input_value_client_configuration = ble_demo_little_endian_read_16(buffer, 0);
            ql_app_log("[DEMO][%s] hid_report_input_input_value_client_configuration: %d", __func__, hid_report_input_input_value_client_configuration);
#ifdef TEST_LOOP_SEND_HID
            test_hid_demo_input();
#endif
            return 0;
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_02_CLIENT_CONFIGURATION_HANDLE:
            hid_report_output_input_value_client_configuration = ble_demo_little_endian_read_16(buffer, 0);
            ql_app_log("[DEMO][%s] hid_report_output_input_value_client_configuration: %d", __func__, hid_report_output_input_value_client_configuration);
            return 0;
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_REPORT_03_CLIENT_CONFIGURATION_HANDLE:
            hid_report_feature_input_value_client_configuration = ble_demo_little_endian_read_16(buffer, 0);
            ql_app_log("[DEMO][%s] hid_report_feature_input_value_client_configuration: %d", __func__, hid_report_feature_input_value_client_configuration);
            return 0;
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_KEYBOARD_INPUT_REPORT_01_CLIENT_CONFIGURATION_HANDLE:
            hid_boot_keyboard_input_value_client_configuration = ble_demo_little_endian_read_16(buffer, 0);
            return 0;
        case ATT_CHARACTERISTIC_ORG_BLUETOOTH_CHARACTERISTIC_BOOT_MOUSE_INPUT_REPORT_01_CLIENT_CONFIGURATION_HANDLE:
            hid_boot_mouse_input_value_client_configuration = ble_demo_little_endian_read_16(buffer, 0);
            ql_app_log("[DEMO][%s] hid_boot_mouse_input_value_client_configuration: %d", __func__, hid_boot_mouse_input_value_client_configuration);
            return 0;
#endif
        default:
            break;
    }
    return 0;
}

void ble_hid_demo_register(void)
{
    int i;
    for (i = 10; i < sizeof(test_buf); i++) {
        test_buf[i] = i;
    }

#ifdef BLE_DEMO_LOOP_SEND_TEST
    memcpy((uint8_t*)ble_demo_loop_test_buf, test_buf, sizeof(ble_demo_loop_test_buf));
    ble_demo_loop_test_handle = ATT_CHARACTERISTIC_AFAEADAC_ABAA_A9A8_A7A6_A5A4A3A2A1A0_01_VALUE_HANDLE;
#endif

    ql_app_log("[DEMO] %s", __func__);
    ql_bt_le_register_att_service(profile_data, sizeof(profile_data), att_read_callback, att_write_callback);
}

#define LE_HID_NAME "QUEC HID"
#define LE_HID_APPEARANCE  962

void ql_bt_le_service_process(void)
{
	struct ql_bt_task_le_adv_parameters adv;
    unsigned char scan_response_data[31];
    int scan_response_size = 0;
	char name[31];
	int total = 0;
	unsigned char flags = 0;
	unsigned char txpower = 0;
	unsigned char service_uuid = 0;
	unsigned short uuid = 0;
	unsigned short appearance = 0;
	int adv_size = 0;
	unsigned char data[31];
	struct bt_addr addr = {0x00,0x11,0x22,0x33,0x44,0x55};

	ql_bt_le_set_random_address(addr);

	memset(name, 0, 31);
    memset(scan_response_data, 0, 31);
    snprintf(name, 31, LE_HID_NAME);

	//ql_bt_le_set_gap_name(name, strlen(name));
	memset(&adv, 0, sizeof(adv));
	//adv.interval_min = 1364; // 852.5ms
	//adv.interval_max = 1364; // 852.5ms
	adv.interval_min = 0x30; // 30ms
	adv.interval_max = 0x30; // 30ms
	adv.advertising_type = LE_ADV_TYPE_IND;
	adv.own_address_type = LE_ADDRESS_TYPE_RANDOM;
	adv.filter = 0;


	flags = (1 << 1);
    adv_size = ql_bt_le_create_adv_data(LE_GAP_TYPE_FLAGS,
		&data[total], 31 - total,
		&flags, sizeof(flags));
	total += adv_size;
	
	txpower = 2; // 2dbm
	adv_size = ql_bt_le_create_adv_data(LE_GAP_TYPE_TX_POWER,
										&data[total], 31 - total,
										&txpower, sizeof(txpower));
	total += adv_size;
	
	uuid = LE_ATT_UUID_HID;
	adv_size = ql_bt_le_create_adv_data(LE_GAP_TYPE_COMPLETE_SERVICE_LIST,
						&data[total], 31 - total,
						(unsigned char *)&uuid, sizeof(uuid));
	total += adv_size;
	
	appearance = LE_HID_APPEARANCE; 
	adv_size = ql_bt_le_create_adv_data(LE_GAP_TYPE_APPEARANCE,
						&data[total], 31 - total,
						(unsigned char *)&appearance, sizeof(appearance));
	total += adv_size;
	
	scan_response_size += ql_bt_le_create_adv_data(LE_GAP_TYPE_COMPLETE_NAME,
										&scan_response_data[0], 31 - 0,
										(unsigned char *)name, strlen(name));

	ql_bt_le_set_adv_data(data, total);
	ql_le_set_scan_response_data(scan_response_data, scan_response_size);
	ql_bt_le_set_adv_parameters(&adv);
	ql_bt_le_set_adv_enable(1);

	ble_hid_demo_register();

}
#endif
/****************************************************************************************************/



/*******************************************Classic Bluetooth*********************************************************/
void ql_get_bt_connect_result(int result)
{
	if(result == BT_CONNECT) {
		ql_app_log("Classic Bluetooth connect sucess\r\n");
	} else if(result == BT_DISCONNECT) {
		ql_app_log("Classic Bluetooth disconnect\r\n");
	} else if(result == BLE_CONNECT) {
		ql_app_log("BLE connect\r\n");
	}else if(result == BLE_DISCONNECT) {
		ql_app_log("BLE disconnect\r\n");
	}
}

void ql_get_bt_scan_result(struct bt_event_inquiry *inquiry,int result)
{
	if(result == 0){
		ql_app_log("inquiry complete\r\n");
	} else {
		if(inquiry->name[0] > 0x7f)
		{
			int i = 0;
			ql_app_log("[QEUC_BT]handle inquiry result(chinese name): %02x%02x%02x%02x%02x%02x\r\n",
						inquiry->addr[0], inquiry->addr[1], inquiry->addr[2], 
						inquiry->addr[3], inquiry->addr[4], inquiry->addr[5]);
			for(i = 0; i < inquiry->length; i++)
			{
				ql_app_log("%02x", inquiry->name[i]);
			}
			ql_app_log("\r\n");
		}
		else
		{
			ql_app_log("[QEUC_BT]handle inquiry result: %s, %02x%02x%02x%02x%02x%02x\r\n",
						inquiry->name,
						inquiry->addr[0], inquiry->addr[1], inquiry->addr[2], 
						inquiry->addr[3], inquiry->addr[4], inquiry->addr[5]);
		}

		//find test device address
		if((inquiry->addr[0] == 0x04 && inquiry->addr[5] == 0xfd) || ((inquiry->addr[0] == 0x2c && inquiry->addr[5] == 0x4c)))
		{
			struct bt_addr bt_addr_get;
			ql_bt_inquiry_cancel();
			ql_get_bt_connect_status(ql_get_bt_connect_result);
			memcpy(&bt_addr_get,inquiry->addr,sizeof(struct bt_addr));
			ql_bt_connect_headset(bt_addr_get, 0);
		}
	}
}
/****************************************************************************************************/

void ql_get_bt_start_result(int result)
{
	if(result == 0){
		ql_app_log("OPEN BT sucess can do next step\r\n");
		struct bt_addr addr = {0};
		ql_bt_get_bt_address(&addr);
        ql_app_log("[QEUC_BT]get local bluetooth address: %02x%02x%02x%02x%02x%02x\r\n",
            addr.bytes[0], addr.bytes[1], addr.bytes[2], addr.bytes[3], addr.bytes[4], addr.bytes[5]);
#if 1  // 1-传统蓝牙配置   0-ble service配置
		ql_get_scan_bt_status(ql_get_bt_scan_result);
		ql_bt_inquiry(0x30, 10);
#else
		ql_bt_le_service_process();
#endif
	} else {
		ql_app_log("OPEN BT failed please check SW config\r\n");
	}
}


void ql_bt_open_gpio_set()   //需要使用的pin都先设置成GPIO模式
{
	//ql_gpio_init(QL_GPIO_BT_POWER, PIN_DIRECTION_OUT, PIN_PULL_DISABLE,PIN_LEVEL_LOW);
	ql_gpio_init(QL_GPIO_BT_RESET, PIN_DIRECTION_OUT, PIN_PULL_DISABLE,PIN_LEVEL_LOW);
	ql_gpio_init(QL_GPIO_BT_LDO_EN, PIN_DIRECTION_OUT, PIN_PULL_DISABLE,PIN_LEVEL_LOW);
	ql_gpio_init(QL_GPIO_BT_WAKEUP_HOST, PIN_DIRECTION_OUT, PIN_PULL_DISABLE,PIN_LEVEL_LOW);
	ql_gpio_init(QL_GPIO_HOST_WAKEUP_BT, PIN_DIRECTION_OUT, PIN_PULL_DISABLE,PIN_LEVEL_LOW);
}

QuecOSStatus ql_bt_task_init(void)
{
    QuecOSStatus err = kNoErr;    

	ql_log_mask_set(QL_LOG_APP_MASK, QL_LOG_PORT_UART|QL_LOG_PORT_USB);
	ql_app_log("ql_bt_task_init start\r\n");
	ql_rtos_task_sleep_s(10);
	ql_get_bt_open_status(ql_get_bt_start_result);

	ql_bt_open_gpio_set();
	err = ql_bt_open(NULL, &ql_init_cfg, &ql_default_bt_cfg);
	if(err != kNoErr){
		ql_app_log("OPEN BT failed please check HW config\r\n");
	}

}
//application_init(ql_bt_task_init, "ql_bt_task_init", 4, 0);

