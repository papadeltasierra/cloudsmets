// ZigBee constants.
//
// // TODO: Are header files correct in C#?
// // TODO: Add Paul D.Smith copyright.
//
// Portions extracted from the following Telink files:
// - zigbee\zcl\zcl_const.h
// - zbhci\zbhci.h
//
// Telink Copyright provided below.
//
//          Copyright (c) 2021, Telink Semiconductor (Shanghai) Co., Ltd. ("TELINK")
//          All rights reserved.
//
//          Licensed under the Apache License, Version 2.0 (the "License");
//          you may not use this file except in compliance with the License.
//          You may obtain a copy of the License at
//
//              http://www.apache.org/licenses/LICENSE-2.0
//
//          Unless required by applicable law or agreed to in writing, software
//          distributed under the License is distributed on an "AS IS" BASIS,
//          WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
//          See the License for the specific language governing permissions and
//          limitations under the License.

/** @addtogroup zcl_data_types ZCL Attribute Data Types
 * @{
 */
#define ZCL_DATA_TYPE_NO_DATA                            0x00
#define ZCL_DATA_TYPE_DATA8                              0x08
#define ZCL_DATA_TYPE_DATA16                             0x09
#define ZCL_DATA_TYPE_DATA24                             0x0a
#define ZCL_DATA_TYPE_DATA32                             0x0b
#define ZCL_DATA_TYPE_DATA40                             0x0c
#define ZCL_DATA_TYPE_DATA48                             0x0d
#define ZCL_DATA_TYPE_DATA56                             0x0e
#define ZCL_DATA_TYPE_DATA64                             0x0f
#define ZCL_DATA_TYPE_BOOLEAN                            0x10
#define ZCL_DATA_TYPE_BITMAP8                            0x18
#define ZCL_DATA_TYPE_BITMAP16                           0x19
#define ZCL_DATA_TYPE_BITMAP24                           0x1a
#define ZCL_DATA_TYPE_BITMAP32                           0x1b
#define ZCL_DATA_TYPE_BITMAP40                           0x1c
#define ZCL_DATA_TYPE_BITMAP48                           0x1d
#define ZCL_DATA_TYPE_BITMAP56                           0x1e
#define ZCL_DATA_TYPE_BITMAP64                           0x1f
#define ZCL_DATA_TYPE_UINT8                              0x20
#define ZCL_DATA_TYPE_UINT16                             0x21
#define ZCL_DATA_TYPE_UINT24                             0x22
#define ZCL_DATA_TYPE_UINT32                             0x23
#define ZCL_DATA_TYPE_UINT40                             0x24
#define ZCL_DATA_TYPE_UINT48                             0x25
#define ZCL_DATA_TYPE_UINT56                             0x26
#define ZCL_DATA_TYPE_UINT64                             0x27
#define ZCL_DATA_TYPE_INT8                               0x28
#define ZCL_DATA_TYPE_INT16                              0x29
#define ZCL_DATA_TYPE_INT24                              0x2a
#define ZCL_DATA_TYPE_INT32                              0x2b
#define ZCL_DATA_TYPE_INT40                              0x2c
#define ZCL_DATA_TYPE_INT48                              0x2d
#define ZCL_DATA_TYPE_INT56                              0x2e
#define ZCL_DATA_TYPE_INT64                              0x2f
#define ZCL_DATA_TYPE_ENUM8                              0x30
#define ZCL_DATA_TYPE_ENUM16                             0x31
#define ZCL_DATA_TYPE_SEMI_PREC                          0x38
#define ZCL_DATA_TYPE_SINGLE_PREC                        0x39
#define ZCL_DATA_TYPE_DOUBLE_PREC                        0x3a
#define ZCL_DATA_TYPE_OCTET_STR                          0x41
#define ZCL_DATA_TYPE_CHAR_STR                           0x42
#define ZCL_DATA_TYPE_LONG_OCTET_STR                     0x43
#define ZCL_DATA_TYPE_LONG_CHAR_STR                      0x44
#define ZCL_DATA_TYPE_ARRAY                              0x48
#define ZCL_DATA_TYPE_STRUCT                             0x4c
#define ZCL_DATA_TYPE_SET                                0x50
#define ZCL_DATA_TYPE_BAG                                0x51
#define ZCL_DATA_TYPE_TOD                                0xe0
#define ZCL_DATA_TYPE_DATE                               0xe1
#define ZCL_DATA_TYPE_UTC                                0xe2
#define ZCL_DATA_TYPE_CLUSTER_ID                         0xe8
#define ZCL_DATA_TYPE_ATTR_ID                            0xe9
#define ZCL_DATA_TYPE_BAC_OID                            0xea
#define ZCL_DATA_TYPE_IEEE_ADDR                          0xf0
#define ZCL_DATA_TYPE_128_BIT_SEC_KEY                    0xf1
#define ZCL_DATA_TYPE_UNKNOWN                            0xff
/** @} end of group zcl_data_types */
