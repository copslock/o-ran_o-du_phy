/******************************************************************************
*
*   Copyright (c) 2019 Intel.
*
*   Licensed under the Apache License, Version 2.0 (the "License");
*   you may not use this file except in compliance with the License.
*   You may obtain a copy of the License at
*
*       http://www.apache.org/licenses/LICENSE-2.0
*
*   Unless required by applicable law or agreed to in writing, software
*   distributed under the License is distributed on an "AS IS" BASIS,
*   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*   See the License for the specific language governing permissions and
*   limitations under the License.
*
*******************************************************************************/

/**
 * @file This file has Shared Memory interface functions between FAPI and PHY
 * @defgroup nr5g_fapi_source_framework_wls_fapi2phy_group
 **/

#include "nr5g_fapi_std.h"
#include "nr5g_fapi_common_types.h"
#include "nr5g_fapi_wls.h"
#include "nr5g_fapi_fapi2phy_wls.h"
#include "nr5g_fapi_log.h"

//------------------------------------------------------------------------------
/** @ingroup nr5g_fapi_source_framework_wls_fapi2phy_group
 *
 *  @param data Pointer to validate
 *
 *  @return  TRUE If pointer is within valid shared WLS memory region
 *           FALSE If pointer is out of valid shared WLS memory region
 *
 *  @description
 *  This function validates pointer's in WLS shared memory region
 *
**/
//------------------------------------------------------------------------------
uint8_t nr5g_fapi_fapi2phy_is_valid_wls_ptr(
    void *data)
{
    p_nr5g_fapi_wls_context_t p_wls_ctx = nr5g_fapi_wls_context();
    if ((unsigned long)data >= (unsigned long)p_wls_ctx->pPartitionMemBase &&
        (unsigned long)data < ((unsigned long)p_wls_ctx->pPartitionMemBase +
            p_wls_ctx->nPartitionMemSize)) {
        return TRUE;
    } else {
        return FALSE;
    }
}

//------------------------------------------------------------------------------
/** @ingroup nr5g_fapi_source_framework_wls_fapi2phy_group
 *
 *  @param void
 *
 *  @return  A pointer to WLS_HANDLE stucture
 *
 *  @description
 *  This function returns the WLS instance
 *
**/
//------------------------------------------------------------------------------
static inline WLS_HANDLE nr5g_fapi_fapi2phy_wls_instance(
    )
{
    p_nr5g_fapi_wls_context_t p_wls_ctx = nr5g_fapi_wls_context();
    return p_wls_ctx->h_wls[NR5G_FAPI2PHY_WLS_INST];
}

//----------------------------------------------------------------------------------
/** @ingroup nr5g_fapi_source_framework_wls_fapi2phy_group
 *
 *  @param[out]   data Location where First API from L1 is stored
 *
 *  @return  Size of Message sent from L1
 *
 *  @description
 *  This function queries the APIs sent from L1 to L2 and gets the first pointer
 *  to the linked list
 *
**/
//----------------------------------------------------------------------------------
inline uint64_t *nr5g_fapi_fapi2phy_wls_get(
    uint32_t * msg_size,
    uint16_t * msg_type,
    uint16_t * flags)
{
    uint64_t *data = NULL;
    WLS_HANDLE h_wls;
    uint32_t ms = 0;
    uint16_t mt = 0, f = 0;

    h_wls = nr5g_fapi_fapi2phy_wls_instance();
    data = (uint64_t *) WLS_Get(h_wls, &ms, &mt, &f);
    *msg_size = ms, *msg_type = mt, *flags = f;
    NR5G_FAPI_LOG(TRACE_LOG, ("[NR5G_FAPI][FAPI2PHY WLS][GET] %p size: %d "
            "type: %x flags: %x", data, *msg_size, *msg_type, *flags));

    return data;
}

//----------------------------------------------------------------------------------
/** @ingroup nr5g_fapi_source_framework_wls_fapi2phy_group
 *
 *  @param[in]   p_msg Pointer to API block that needs to be sent to L1
 *  @param[in]   msg_size Size of Message
 *  @param[in]   msg_id Message Id
 *  @param[in]   flags Special flags needed for WLS
 *
 *  @return  0 if SUCCESS
 *
 *  @description
 *  This function adds a block of API from L2 to L1 which will be sent later
 *
**/
//----------------------------------------------------------------------------------
inline uint8_t nr5g_fapi_fapi2phy_wls_put(
    uint64_t p_msg,
    uint32_t msg_size,
    uint16_t msg_type,
    uint16_t flags)
{
    int ret = SUCCESS;
    WLS_HANDLE h_phy_wls = nr5g_fapi_fapi2phy_wls_instance();
    uint64_t pa = nr5g_fapi_wls_va_to_pa(h_phy_wls, (void *)p_msg);
    NR5G_FAPI_LOG(TRACE_LOG, ("[NR5G_FAPI][FAPI2PHY WLS][PUT] %ld size: %d "
            "type: %x flags: %x", pa, msg_size, msg_type, flags));
    ret = WLS_Put(h_phy_wls, pa, msg_size, msg_type, flags);

    return ret;
}

//----------------------------------------------------------------------------------
/** @ingroup nr5g_fapi_source_framework_wls_fapi2phy_group
 *
 *  @param void
 *
 *  @return  0 if SUCCESS
 *
 *  @description
 *  This function is called at WLS init and waits in an infinite for L1 to respond back with some information
 *  needed by the L2
 *
**/
//----------------------------------------------------------------------------------
inline uint8_t nr5g_fapi_fapi2phy_wls_ready(
    )
{
    int ret = SUCCESS;
    //NR5G_FAPI_LOG(TRACE_LOG, ("Waiting for L1 to respond in WLS Ready"));
    ret = WLS_Ready(nr5g_fapi_fapi2phy_wls_instance());
    return ret;
}

//----------------------------------------------------------------------------------
/** @ingroup nr5g_fapi_source_framework_wls_fapi2phy_group
 *
 *  @param   void
 *
 *  @return  Number of blocks of APIs received
 *
 *  @description
 *  This functions waits in a infinite loop for L1 to send a list of APIs to MAC. This is called
 *  during runtime when L2 sends a API to L1 and then waits for response back.
 *
**/
//----------------------------------------------------------------------------------
inline uint8_t nr5g_fapi_fapi2phy_wls_wait(
    )
{
    int ret = SUCCESS;
//    NR5G_FAPI_LOG(TRACE_LOG, ("Waiting for L1 to respond in WLS Wait"));
    ret = WLS_Wait(nr5g_fapi_fapi2phy_wls_instance());
    return ret;
}

//------------------------------------------------------------------------------
/** @ingroup nr5g_fapi_source_framework_wls_fapi2phy_group
 *
 *  @param[out]   data Location where First API from L1 is stored
 *
 *  @return  Size of Message sent from L1
 *
 *  @description
 *  This function checks if this is the last message in this tti.
 *
**/
//------------------------------------------------------------------------------
static inline uint8_t is_msg_present(
    uint16_t flags)
{
    return (!((flags & WLS_TF_FIN) || (flags == 0)));
}

//----------------------------------------------------------------------------------
/** @ingroup nr5g_fapi_source_framework_wls_fapi2phy_group
 *
 *  @param[out]   data Location where First API from L1 is stored
 *
 *  @return  Size of Message sent from L1
 *
 *  @description
 *  This function queries the APIs sent from L1 to L2 and gets the first pointer
 *  to the linked list
 *
**/
//----------------------------------------------------------------------------------
PMAC2PHY_QUEUE_EL nr5g_fapi_fapi2phy_wls_recv(
    )
{
    uint16_t msg_type = 0;
    uint16_t flags = 0;
    uint32_t msg_size = 0;
    uint32_t num_elms = 0;
    uint64_t *p_msg = NULL;
    static uint32_t g_free_recv_idx = 0;
    PMAC2PHY_QUEUE_EL p_qelm_list = NULL;
    PMAC2PHY_QUEUE_EL p_qelm = NULL;
    PMAC2PHY_QUEUE_EL p_tail_qelm = NULL;

    num_elms = nr5g_fapi_fapi2phy_wls_wait();
    if (!num_elms)
        return p_qelm_list;

    do {
        p_msg = nr5g_fapi_fapi2phy_wls_get(&msg_size, &msg_type, &flags);
        if (p_msg) {
            WLS_HANDLE h_wls = nr5g_fapi_fapi2phy_wls_instance();
            p_qelm = (PMAC2PHY_QUEUE_EL)
                nr5g_fapi_wls_pa_to_va(h_wls, (uint64_t) p_msg);
            if (nr5g_fapi_fapi2phy_is_valid_wls_ptr(p_qelm) == FALSE) {
                printf("Error: Invalid Ptr\n");
                continue;
            }
            p_qelm->pNext = NULL;
            if (p_qelm_list) {
                p_tail_qelm->pNext = p_qelm;
                p_tail_qelm = p_qelm;
            } else {
                p_qelm_list = p_qelm;
                p_tail_qelm = p_qelm;
            }
        }
        num_elms--;
    } while (num_elms && is_msg_present(flags));

    if (p_qelm_list) {
        wls_fapi_add_recv_apis_to_free(p_qelm_list, g_free_recv_idx);
        g_free_recv_idx++;
        if (g_free_recv_idx >= TO_FREE_SIZE) {
            g_free_recv_idx = 0;
        }
        // Free 10 TTIs Later
        wls_fapi_free_recv_free_list(g_free_recv_idx);

        wls_fapi_add_blocks_to_ul();
    }

    return p_qelm_list;
}

//------------------------------------------------------------------------------
/** @ingroup nr5g_fapi_source_framework_wls_fapi2phy_group
 *
 *  @param[in]    p_msg_header Pointer to the TxSDuReq Message block
 *  @param[out]   n_zbc_blocks Number of ZBC blocks
 *
 *  @return  1 if this block is a TxSduReq message. 0 else.
 *
 *  @description
 *  This function checks if a block is a TxSduReq messages and counts the number
 *  of ZBC blocks in this API
 *
**/
//------------------------------------------------------------------------------
int nr5g_fapi_fapi2phy_is_sdu_zbc_block(
    void *p_msg,
    int *num_zbc_blocks)
{
    PL1L2MessageHdr p_msg_header = (PL1L2MessageHdr) p_msg;
    *num_zbc_blocks = 0;

    if (p_msg_header->nMessageType == MSG_TYPE_PHY_TX_REQ) {
        PTXRequestStruct p_dl_sdu_req = (PTXRequestStruct) p_msg_header;
        PDLPDUDataStruct p_dl_pdu_data = (PDLPDUDataStruct) (p_dl_sdu_req + 1);
        uint32_t i;
        for (i = 0; i < p_dl_sdu_req->nPDU; i++) {
            *num_zbc_blocks += (p_dl_pdu_data->nPduLen1 ? 1 : 0);
            *num_zbc_blocks += (p_dl_pdu_data->nPduLen2 ? 1 : 0);
            p_dl_pdu_data++;
        }

        if (*num_zbc_blocks) {
            return 1;
        }
    }

    return 0;
}

//----------------------------------------------------------------------------------
/** @ingroup nr5g_fapi_source_framework_wls_fapi2phy_group
 *
 *  @param[in]   p_msg_header Pointer to the TxSduReq Message block
 *  @param[in]   flags Special flags needed for WLS
 *  @param[in]   n_zbc_blocks Number of ZBC blocks in list
 *
 *  @return  0 if SUCCESS
 *
 *  @description
 *  This function adds all the ZBC blocks in a TXSDU Message and prepares them to
 *  be sent to the L1
 *
**/
//------------------------------------------------------------------------------
uint32_t nr5g_fapi_fapi2phy_send_zbc_blocks(
    void *p_msg,
    uint16_t flags)
{
    PL1L2MessageHdr p_msg_header = (PL1L2MessageHdr) p_msg;
    PTXRequestStruct p_dl_sdu_req = (PTXRequestStruct) p_msg_header;
    PDLPDUDataStruct p_dl_pdu_data = (PDLPDUDataStruct) (p_dl_sdu_req + 1);
    uint32_t i, j, is_last, is_last1, msg_type;
    uint16_t list_flags = flags;

    for (i = 0; i < p_dl_sdu_req->nPDU; i++) {
        is_last = (i == (p_dl_sdu_req->nPDU - 1));
        for (j = 0; j < MAX_DL_PER_UE_CODEWORD_NUM; j++) {
            uint32_t pdu_len;
            uint8_t *p_payload;
            p_payload = NULL;
            msg_type = 0;
            if (j == 0) {
                pdu_len = p_dl_pdu_data->nPduLen1;
                p_payload = p_dl_pdu_data->pPayload1;
                msg_type = MSG_PHY_ZBC_BLOCK0_REQ;
            } else {
                pdu_len = p_dl_pdu_data->nPduLen2;
                p_payload = p_dl_pdu_data->pPayload2;
                msg_type = MSG_PHY_ZBC_BLOCK1_REQ;
            }

            if (p_payload) {
                is_last1 = (((j == 0) && (p_dl_pdu_data->pPayload2 == 0)) ||
                    (j == (MAX_DL_PER_UE_CODEWORD_NUM - 1)));
                if ((list_flags & WLS_TF_FIN) && is_last && is_last1) {
                    flags = WLS_SG_LAST;
                } else {
                    flags = WLS_SG_NEXT;
                }

                WLS_HANDLE h_phy_wls = nr5g_fapi_fapi2phy_wls_instance();
                if (WLS_Put(h_phy_wls, (uint64_t) p_payload, pdu_len, msg_type,
                        flags) != SUCCESS) {
                    printf("Error ZBC block 0x%016lx\n", (uint64_t) p_payload);
                    return FAILURE;
                }
            }
        }
        p_dl_pdu_data++;
    }

    return SUCCESS;
}

//------------------------------------------------------------------------------
/** @ingroup nr5g_fapi_source_framework_wls_fapi2phy_group
 *
 *  @param[in]      A pointer to the phy instance
 *  @param[in]      A data Pointer to the Linked list header
 *
 *  @return         0 if SUCCESS
 *
 *  @description    This function sends a list of APIs to the L1 via WLS
 *
**/
//------------------------------------------------------------------------------
uint8_t nr5g_fapi_fapi2phy_wls_send(
    void *data)
{
    p_nr5g_fapi_wls_context_t p_wls_ctx = nr5g_fapi_wls_context();
    PMAC2PHY_QUEUE_EL p_curr_msg = NULL;
    PL1L2MessageHdr p_msg_header;
    uint16_t flags = 0;
    uint8_t ret = SUCCESS;
    int n_zbc_blocks = 0, is_zbc = 0, count = 0;
    static uint32_t g_free_send_idx = 0;

    p_curr_msg = (PMAC2PHY_QUEUE_EL) data;
    wls_fapi_add_send_apis_to_free(p_curr_msg, g_free_send_idx);

    if (pthread_mutex_lock((pthread_mutex_t *) & p_wls_ctx->fapi2phy_lock_send)) {
        NR5G_FAPI_LOG(ERROR_LOG, ("unable to lock send pthread mutex"));
        return FAILURE;
    }

    if (p_curr_msg->pNext) {
        flags = WLS_SG_FIRST;
        while (p_curr_msg) {
            // only batch mode
            count++;
            p_msg_header = (PL1L2MessageHdr) (p_curr_msg + 1);
            if (p_curr_msg->pNext) {    // FIRST/NEXT list element
                if (SUCCESS != nr5g_fapi_fapi2phy_wls_put((uint64_t) p_curr_msg,
                        p_curr_msg->nMessageLen + sizeof(MAC2PHY_QUEUE_EL),
                        p_msg_header->nMessageType, flags)) {
                    if (pthread_mutex_unlock((pthread_mutex_t *) & p_wls_ctx->
                            fapi2phy_lock_send)) {
                        NR5G_FAPI_LOG(ERROR_LOG,
                            ("unable to unlock send pthread mutex"));
                    }
                    return FAILURE;
                }

                if (nr5g_fapi_fapi2phy_is_sdu_zbc_block(p_msg_header, &n_zbc_blocks)) { // ZBC blocks
                    if (nr5g_fapi_fapi2phy_send_zbc_blocks(p_msg_header,
                            flags) != SUCCESS) {
                        if (pthread_mutex_unlock((pthread_mutex_t *) &
                                p_wls_ctx->fapi2phy_lock_send)) {
                            NR5G_FAPI_LOG(ERROR_LOG,
                                ("unable to unlock send pthread mutex"));
                        }
                        return FAILURE;
                    }
                }
                p_curr_msg = p_curr_msg->pNext;
            } else {            /* p_curr_msg->Next */
                // LAST
                flags = WLS_SG_LAST;
                is_zbc = 0;
                if (nr5g_fapi_fapi2phy_is_sdu_zbc_block(p_msg_header,
                        &n_zbc_blocks)) {
                    flags = WLS_SG_NEXT;
                    is_zbc = 1;
                }
                if (nr5g_fapi_fapi2phy_wls_put((uint64_t) p_curr_msg,
                        p_curr_msg->nMessageLen + sizeof(MAC2PHY_QUEUE_EL),
                        p_msg_header->nMessageType, flags) != SUCCESS) {
                    printf("Error\n");
                    if (pthread_mutex_unlock((pthread_mutex_t *) & p_wls_ctx->
                            fapi2phy_lock_send)) {
                        NR5G_FAPI_LOG(ERROR_LOG,
                            ("unable to unlock send pthread mutex"));
                    }
                    return FAILURE;
                }

                if (is_zbc) {   // ZBC blocks
                    if (nr5g_fapi_fapi2phy_send_zbc_blocks(p_msg_header,
                            WLS_SG_LAST) != SUCCESS) {
                        printf("Error\n");
                        if (pthread_mutex_unlock((pthread_mutex_t *) &
                                p_wls_ctx->fapi2phy_lock_send)) {
                            NR5G_FAPI_LOG(ERROR_LOG,
                                ("unable to unlock send pthread mutex"));
                        }
                        return FAILURE;
                    }
                }
                p_curr_msg = NULL;
            }                   /* p_curr_msg->Next */
            flags = WLS_SG_NEXT;
        }
    } else {                      // one block
        count++;
        if (nr5g_fapi_fapi2phy_is_sdu_zbc_block(p_curr_msg, &n_zbc_blocks)) {
            printf("Error ZBC block cannot be only one in the list\n");
            if (pthread_mutex_unlock((pthread_mutex_t *) & p_wls_ctx->
                    fapi2phy_lock_send)) {
                NR5G_FAPI_LOG(ERROR_LOG,
                    ("unable to unlock send pthread mutex"));
            }
            return FAILURE;
        }

        if (SUCCESS != nr5g_fapi_fapi2phy_wls_put((uint64_t) p_curr_msg,
                p_curr_msg->nMessageLen + sizeof(MAC2PHY_QUEUE_EL),
                p_curr_msg->nMessageType, flags)) {
            printf("Error\n");
            if (pthread_mutex_unlock((pthread_mutex_t *) & p_wls_ctx->
                    fapi2phy_lock_send)) {
                NR5G_FAPI_LOG(ERROR_LOG,
                    ("unable to unlock send pthread mutex"));
            }
            return FAILURE;
        }
    }

    if (count > 1) {
        g_free_send_idx++;
        if (g_free_send_idx >= TO_FREE_SIZE)
            g_free_send_idx = 0;

        // Free 10 TTIs Later
        wls_fapi_free_send_free_list(g_free_send_idx);
    }

    if (pthread_mutex_unlock((pthread_mutex_t *) & p_wls_ctx->
            fapi2phy_lock_send)) {
        NR5G_FAPI_LOG(ERROR_LOG, ("unable to unlock send pthread mutex"));
        return FAILURE;
    }
    return ret;
}
