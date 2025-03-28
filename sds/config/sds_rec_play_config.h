/*
 * Copyright (c) 2025 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Name:    sds_rec_play_config.h
 * Purpose: SDS Recorder and Player configuration options
 * Rev.:    V1.0.0
 */

//-------- <<< Use Configuration Wizard in Context Menu >>> --------------------

// <h>SDS Recorder and Player

//   <o>Maximum number of streams <1-31>
//   <i>Default: 16
#define SDS_REC_PLAY_MAX_STREAMS     16U

//   <o>Size of a temporary recorder buffer
//   <i>Default: 8192
//   <i>Default: At least the size of the largest record
#define SDS_REC_PLAY_BUF_SIZE        8192U

// </h>

//------------- <<< end of configuration section >>> ---------------------------

// Timeout value for opening the player stream in milliseconds
#define SDS_REC_PLAY_OPEN_TOUT     3000U

// Timeout value for closing the recorder/player stream in milliseconds
#define SDS_REC_PLAY_CLOSE_TOUT    3000U
