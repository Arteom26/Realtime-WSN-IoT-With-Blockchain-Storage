/*
 * Copyright (c) 2016-2018 . All Rights Reserved.
 */

#include <stdlib.h>
#include <string.h>
#include "RLP.h"

#define SIZE_THRESHOLD 56
#define OFFSET_SHORT_LIST 0xc0
#define OFFSET_LONG_LIST 0xf7
#define OFFSET_LONG_ITEM  0xb7
#define OFFSET_SHORT_ITEM 0x80

static int wallet_copy_rpl(uint8_t *source, uint8_t *destination, uint8_t size,
                           int copyPos) {
    int ret_val = copyPos;
    if (size != 0) {
        memcpy(source, destination, size);
        ret_val = ret_val + size;
    }
    return ret_val;
}

static int calculateLengOfV(uint32_t v) {
    // 1byte
    if ((v & 0xFF) == v) {
        return 1;
        // 2byte
    } else if ((v & 0xFFFF) == v) {
        return 2;
        // 3byte
    } else if ((v & 0xFFFFFF) == v) {
        return 3;
        // 4byte
    } else if ((v & 0xFFFFFFFF) == v) {
        return 4;
    } else {
        return -1;
    }
}

void wallet_encode_byte(pb_byte_t singleByte, pb_byte_t *new_bytes) {
    if ((singleByte & 0xFF) == 0) {
        new_bytes[0] = (pb_byte_t) OFFSET_SHORT_ITEM;
    } else if ((singleByte & 0xFF) <= 0x7F) {
        new_bytes[0] = (pb_byte_t) singleByte;
    } else {
        new_bytes[0] = (pb_byte_t) (OFFSET_SHORT_ITEM + 1);
        new_bytes[1] = singleByte;
    }
}

void wallet_encode_short(uint16_t singleShort, pb_byte_t *new_bytes) {
    if ((singleShort & 0xFF) == singleShort)
        wallet_encode_byte((pb_byte_t) singleShort, new_bytes);
    else {
        new_bytes[0] = (pb_byte_t) (OFFSET_SHORT_ITEM + 2);
        new_bytes[1] = (singleShort >> 8 & 0xFF);
        new_bytes[2] = (singleShort >> 0 & 0xFF);
    }
}

int setup_transaction_unsigned(char* nonce, char* gas_price, char* gas_limit, char *to, char *value, char *data, uint8_t *rawTx){
    EthereumSignTx tx;

    tx.nonce.size = size_of_bytes(strlen(nonce));
    hex2byte_arr(nonce, strlen(nonce), tx.nonce.bytes, tx.nonce.size);

    tx.gas_price.size = size_of_bytes(strlen(gas_price));
    hex2byte_arr(gas_price, strlen(gas_price), tx.gas_price.bytes, tx.gas_price.size);

    tx.gas_limit.size = size_of_bytes(strlen(gas_limit));
    hex2byte_arr(gas_limit, strlen(gas_limit), tx.gas_limit.bytes, tx.gas_limit.size);

    tx.to.size = size_of_bytes(strlen(to));
    hex2byte_arr(to, strlen(to), tx.to.bytes, tx.to.size);

    tx.value.size = size_of_bytes(strlen(value));
    hex2byte_arr(value, strlen(value), tx.value.bytes, tx.value.size);

    tx.data_initial_chunk.size = size_of_bytes(strlen(data));
    hex2byte_arr(data, strlen(data), tx.data_initial_chunk.bytes, tx.data_initial_chunk.size);

    int size = encode_unsigned_transaction(&tx, rawTx);
    return size;
}

int encode_unsigned_transaction(EthereumSignTx *tx, uint8_t *rawTx){
    EncodeEthereumSignTx new_msg;

    wallet_encode_element(tx->nonce.bytes, tx->nonce.size,
                          new_msg.nonce.bytes, &(new_msg.nonce.size), false);
    wallet_encode_element(tx->gas_price.bytes, tx->gas_price.size,
                            new_msg.gas_price.bytes, &(new_msg.gas_price.size), false);
    wallet_encode_element(tx->gas_limit.bytes, tx->gas_limit.size,
                            new_msg.gas_limit.bytes, &(new_msg.gas_limit.size), false);
    wallet_encode_element(tx->to.bytes, tx->to.size, new_msg.to.bytes,
                            &(new_msg.to.size), false);
    wallet_encode_element(tx->value.bytes, tx->value.size,
                            new_msg.value.bytes, &(new_msg.value.size), false);
    wallet_encode_element(tx->data_initial_chunk.bytes,
                            tx->data_initial_chunk.size, new_msg.data_initial_chunk.bytes,
                            &(new_msg.data_initial_chunk.size), false);
	
	int size = wallet_encode_unsigned_list(&new_msg, (uint64_t*)rawTx);
    return size;
}

// Encodes without v, r, s
int wallet_encode_unsigned_list(EncodeEthereumSignTx *new_msg, uint64_t *rawTx) {
    uint32_t totalLength = 0;
    uint8_t *data;

    totalLength += new_msg->nonce.size;
    totalLength += new_msg->gas_price.size;
    totalLength += new_msg->gas_limit.size;
    totalLength += new_msg->to.size;
    totalLength += new_msg->value.size;
    totalLength += new_msg->data_initial_chunk.size;

    int copyPos;
    if (totalLength < SIZE_THRESHOLD) {
        data = malloc(1 + totalLength);
        data[0] = (int8_t) (OFFSET_SHORT_LIST + totalLength);
        copyPos = 1;
    } else {
        int tmpLength = totalLength;
        uint8_t byteNum = 0;
        while (tmpLength != 0) {
            ++byteNum;
            tmpLength = tmpLength >> 8;
        }
        tmpLength = totalLength;
        uint8_t *lenBytes;
        lenBytes = malloc(byteNum);
        int i;
        for (i = 0; i < byteNum; ++i) {
            lenBytes[byteNum - 1 - i] =
                    (uint8_t) ((tmpLength >> (8 * i)) & 0xFF);
        }
        data = malloc(1 + byteNum + totalLength);
        data[0] = (uint8_t) (OFFSET_LONG_LIST + byteNum);
        memcpy(data + 1, lenBytes, byteNum);
        copyPos = byteNum + 1;
        free(lenBytes);
    }

    copyPos = wallet_copy_rpl(data + copyPos, new_msg->nonce.bytes,
                              new_msg->nonce.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->gas_price.bytes,
                              new_msg->gas_price.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->gas_limit.bytes,
                              new_msg->gas_limit.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->to.bytes,
                              new_msg->to.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->value.bytes,
                              new_msg->value.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->data_initial_chunk.bytes,
                              new_msg->data_initial_chunk.size, copyPos);

    memcpy(rawTx, data, copyPos);
    return copyPos;
}

int wallet_encode_list(EncodeEthereumSignTx *new_msg, EncodeEthereumTxRequest *new_tx,
                       uint64_t *rawTx) {
    uint32_t totalLength = 0;
    uint8_t *data;

    totalLength += new_msg->nonce.size;
    totalLength += new_msg->gas_price.size;
    totalLength += new_msg->gas_limit.size;
    totalLength += new_msg->to.size;
    totalLength += new_msg->value.size;
    totalLength += new_msg->data_initial_chunk.size;

    totalLength += calculateLengOfV(new_tx->signature_v); //tx->signature_v.size
    totalLength += new_tx->signature_r.size;
    totalLength += new_tx->signature_s.size;

    int copyPos;
    if (totalLength < SIZE_THRESHOLD) {
        data = malloc(1 + totalLength);
        data[0] = (int8_t) (OFFSET_SHORT_LIST + totalLength);
        copyPos = 1;
    } else {
        int tmpLength = totalLength;
        uint8_t byteNum = 0;
        while (tmpLength != 0) {
            ++byteNum;
            tmpLength = tmpLength >> 8;
        }
        tmpLength = totalLength;
        uint8_t *lenBytes;
        lenBytes = malloc(byteNum);
        int i;
        for (i = 0; i < byteNum; ++i) {
            lenBytes[byteNum - 1 - i] =
                    (uint8_t) ((tmpLength >> (8 * i)) & 0xFF);
        }
        data = malloc(1 + byteNum + totalLength);
        data[0] = (uint8_t) (OFFSET_LONG_LIST + byteNum);
        memcpy(data + 1, lenBytes, byteNum);
        copyPos = byteNum + 1;
        free(lenBytes);
    }

    copyPos = wallet_copy_rpl(data + copyPos, new_msg->nonce.bytes,
                              new_msg->nonce.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->gas_price.bytes,
                              new_msg->gas_price.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->gas_limit.bytes,
                              new_msg->gas_limit.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->to.bytes,
                              new_msg->to.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->value.bytes,
                              new_msg->value.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_msg->data_initial_chunk.bytes,
                              new_msg->data_initial_chunk.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, (uint8_t*)&new_tx->signature_v, calculateLengOfV(new_tx->signature_v), copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_tx->signature_r.bytes,
                              new_tx->signature_r.size, copyPos);
    copyPos = wallet_copy_rpl(data + copyPos, new_tx->signature_s.bytes,
                              new_tx->signature_s.size, copyPos);

    memcpy(rawTx, data, copyPos);
    return copyPos;
}

void wallet_encode_element(pb_byte_t *bytes, pb_size_t size,
                           pb_byte_t *new_bytes, pb_size_t *new_size, bool remove_leading_zeros) {

    pb_byte_t *pbytes;
    pb_size_t psize;


    if (remove_leading_zeros) {
        int leading_count = 0;
        for (int j = 0; j < size; ++j) {
            pb_byte_t singleByte = bytes[j];
            if ((singleByte | 0x00) == 0) {
                leading_count = leading_count + 1;
            } else {
                break;
            }
        }
        if (leading_count > 0) {
            pbytes = malloc(size - leading_count);
            memcpy(pbytes, bytes + 1, (size - leading_count));
            psize = (pb_size_t) (size - leading_count);
        } else {
            pbytes = malloc(size);
            memcpy(pbytes, bytes, (size));
            psize = size;
        }
    } else {
        pbytes = malloc(size);
        memcpy(pbytes, bytes, (size));
        psize = size;
    }

    if (psize == 0) {
        *new_size = 1;
        new_bytes[0] = (pb_byte_t) OFFSET_SHORT_ITEM;
    } else if (psize == 1 && pbytes[0] == 0x00) {
        *new_size = 1;
        new_bytes[0] = pbytes[0];
    } else if (psize == 1 && ((pbytes[0] & 0xFF) == 0)) {
        *new_size = 1;
        new_bytes[0] = pbytes[0];
    } else if (psize == 1 && (pbytes[0] & 0xFF) < 0x80) {
        *new_size = 1;
        new_bytes[0] = pbytes[0];
    } else if (psize < SIZE_THRESHOLD) {
        pb_byte_t length = (pb_byte_t) (OFFSET_SHORT_ITEM + psize);
        new_bytes[0] = length;
        memcpy(new_bytes + 1, pbytes, psize);
        *new_size = psize + 1;
    } else {
        int tmpLength = psize;
        pb_byte_t lengthOfLength = (pb_byte_t) 0;
        while (tmpLength != 0) {
            ++lengthOfLength;
            tmpLength = tmpLength >> 8;
        }
        pb_byte_t *data = malloc(1 + lengthOfLength + psize);
        data[0] = (pb_byte_t) (OFFSET_LONG_ITEM + lengthOfLength);
        tmpLength = psize;
        int i;
        for (int i = lengthOfLength; i > 0; --i) {
            data[i] = (pb_byte_t) (tmpLength & 0xFF);
            tmpLength = tmpLength >> 8;
        }
        memcpy(data + 1 + lengthOfLength, pbytes, psize);
        memcpy(new_bytes, data, ((1 + lengthOfLength + psize)));
        *new_size = (1 + lengthOfLength + psize);
        free(pbytes);
        free(data);
    }
}

void wallet_encode_int(uint32_t singleInt, pb_byte_t *new_bytes) {
    if ((singleInt & 0xFF) == singleInt) {
        wallet_encode_byte((pb_byte_t) singleInt, new_bytes);
    } else if ((singleInt & 0xFFFF) == singleInt) {
        wallet_encode_short((uint16_t) singleInt, new_bytes);
    } else if ((singleInt & 0xFFFFFF) == singleInt) {
        new_bytes[0] = (pb_byte_t) (OFFSET_SHORT_ITEM + 3);
        new_bytes[1] = (pb_byte_t) (singleInt >> 16);
        new_bytes[2] = (pb_byte_t) (singleInt >> 8);
        new_bytes[3] = (pb_byte_t) (singleInt);

    } else {
        new_bytes[0] = (pb_byte_t) (OFFSET_SHORT_ITEM + 4);
        new_bytes[1] = (pb_byte_t) (singleInt >> 24);
        new_bytes[2] = (pb_byte_t) (singleInt >> 16);
        new_bytes[3] = (pb_byte_t) (singleInt >> 8);
        new_bytes[4] = (pb_byte_t) (singleInt);
    }
}



