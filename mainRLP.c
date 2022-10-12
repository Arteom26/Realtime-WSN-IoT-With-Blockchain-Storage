/*
 * Copyright (c) 2016-2018 . All Rights Reserved.
 */

#include <string.h>
#include "utils.h"
#include "RLP.h"

#define CHAIN_ID 1337

// Converts transaction to RLP
int wallet_ethereum_assemble_tx(EthereumSignTx *msg, EthereumSig *tx, uint64_t *rawTx) {
    EncodeEthereumSignTx new_msg;
    EncodeEthereumTxRequest new_tx;
    memset(&new_msg, 0, sizeof(new_msg));
    memset(&new_tx, 0, sizeof(new_tx));
    wallet_encode_element(msg->nonce.bytes, msg->nonce.size,
                          new_msg.nonce.bytes, &(new_msg.nonce.size), false);
    wallet_encode_element(msg->gas_price.bytes, msg->gas_price.size,
                          new_msg.gas_price.bytes, &(new_msg.gas_price.size), false);
    wallet_encode_element(msg->gas_limit.bytes, msg->gas_limit.size,
                          new_msg.gas_limit.bytes, &(new_msg.gas_limit.size), false);
    wallet_encode_element(msg->to.bytes, msg->to.size, new_msg.to.bytes,
                          &(new_msg.to.size), false);
    wallet_encode_element(msg->value.bytes, msg->value.size,
                          new_msg.value.bytes, &(new_msg.value.size), false);
    wallet_encode_element(msg->data_initial_chunk.bytes,
                          msg->data_initial_chunk.size, new_msg.data_initial_chunk.bytes,
                          &(new_msg.data_initial_chunk.size), false);

    wallet_encode_int(tx->signature_v, (pb_byte_t*)&(new_tx.signature_v));
    wallet_encode_element(tx->signature_r.bytes, tx->signature_r.size,
                          new_tx.signature_r.bytes, &(new_tx.signature_r.size), true);
    wallet_encode_element(tx->signature_s.bytes, tx->signature_s.size,
                          new_tx.signature_s.bytes, &(new_tx.signature_s.size), true);

    int length = wallet_encode_list(&new_msg, &new_tx, rawTx);
    return length;
}

int assembleTx(uint8_t *rawTx, EthereumSignTx *tx) {
    EthereumSig signature;
    uint64_t raw_tx_bytes[24];

    // Blank for unsigned transaction
    const char *r = "";
    const char *s = "";
    
    signature.signature_v = CHAIN_ID;

    signature.signature_r.size = size_of_bytes(strlen(r));
    hex2byte_arr((char*)r, strlen(r), signature.signature_r.bytes, signature.signature_r.size);

    signature.signature_s.size = size_of_bytes(strlen(s));
    hex2byte_arr((char*)s, strlen(s), signature.signature_s.bytes, signature.signature_s.size);
    
    int length = wallet_ethereum_assemble_tx(tx, &signature, (uint64_t*)rawTx);
    return length;
}

int assembleFinalTx(uint8_t *rawTx, EthereumSig *sig) {
    EthereumSignTx tx;
    uint64_t raw_tx_bytes[24];

    char *nonce = "";
    char *gas_price = "2540be400";
    char *gas_limit = "5208";
    char *to = "FD0156A25E9f4b95a99eaC5252189A806B018D8a";
    char *value = "64";// In hex
    char *data = "";

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
    hex2byte_arr(data, strlen(data), tx.data_initial_chunk.bytes,
                 tx.data_initial_chunk.size);

    int length = wallet_ethereum_assemble_tx(&tx, sig, (uint64_t*)rawTx);
    return length;
}

EthereumSignTx setupTransactionfromChar(char* nonce, char* gas_price, char* gas_limit, char* to, 
                            char* value, char* data){
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
    hex2byte_arr(data, strlen(data), tx.data_initial_chunk.bytes,
                 tx.data_initial_chunk.size);
    return tx;
}
