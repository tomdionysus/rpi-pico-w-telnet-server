/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#include "server.h"

TCP_SERVER_T* tcp_server_init(void) {
    TCP_SERVER_T *state = calloc(1, sizeof(TCP_SERVER_T));
    if (!state) return NULL;
    return state;
}

err_t tcp_server_close(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    err_t err = ERR_OK;
    if (state->client_pcb != NULL) {
        tcp_arg(state->client_pcb, NULL);
        tcp_poll(state->client_pcb, NULL, 0);
        tcp_sent(state->client_pcb, NULL);
        tcp_recv(state->client_pcb, NULL);
        tcp_err(state->client_pcb, NULL);
        err = tcp_close(state->client_pcb);
        if (err != ERR_OK) {
            tcp_abort(state->client_pcb);
            err = ERR_ABRT;
        }
        state->client_pcb = NULL;
    }
    if (state->server_pcb) {
        tcp_arg(state->server_pcb, NULL);
        tcp_close(state->server_pcb);
        state->server_pcb = NULL;
    }
    return err;
}

err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    // DEBUG_printf("tcp_server_sent %u\n", len);
    state->sent_len += len;

    if (state->sent_len >= BUF_SIZE) {

        // We should get the data back from the client
        state->recv_len = 0;
        // DEBUG_printf("Waiting for buffer from client\n");
    }

    return ERR_OK;
}

err_t tcp_server_handle_response(void *arg, struct tcp_pcb *tpcb, char *buffer, int length) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;

    // Buffer finishes with CRLF, truncate it
    if(length<2) return ERR_ABRT;
    length = length-2;
    buffer[length] = 0;
        
    char txbuffer[1024], hexbuffer[1024];

    if(strcmp(buffer, "exit") == 0) {
        tcp_server_close(arg);
    } else {
        buffer_to_hex(buffer, hexbuffer, length);
        sprintf(&txbuffer, "Unrecognised Command: %s\r\nLength: %d\r\nHex: %s\r\n\0", buffer, length, hexbuffer);
        tcp_server_write_string(tpcb, txbuffer);
    }

    tcp_server_write_string(tpcb, ">\0");

    return ERR_OK;
}

err_t tcp_server_write_string(struct tcp_pcb *tpcb, char *buffer) {
    cyw43_arch_lwip_check();
    return tcp_write(tpcb, buffer, strlen(buffer), TCP_WRITE_FLAG_COPY);
}

err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (!p) {
        return ERR_ABRT;
    }
    cyw43_arch_lwip_check();
    if (p->tot_len > 0) {
        // DEBUG_printf("tcp_server_recv %d/%d err %d\n", p->tot_len, state->recv_len, err);

        const uint16_t buffer_left = BUF_SIZE - state->recv_len;
        state->recv_len += pbuf_copy_partial(p, state->buffer_recv + state->recv_len,
                                             p->tot_len > buffer_left ? buffer_left : p->tot_len, 0);
        tcp_recved(tpcb, p->tot_len);
    }
    pbuf_free(p);

    // Look for CRLF in the buffer
    for (int i = 0; i < state->recv_len - 1; ++i) {
        if (state->buffer_recv[i] == '\r' && state->buffer_recv[i + 1] == '\n') {
            // Call the external handler with the data up to and including CRLF
            tcp_server_handle_response(arg, tpcb, state->buffer_recv, i + 2);

            // Move the rest of the buffer to the beginning
            int remaining = state->recv_len - (i + 2);
            memmove(state->buffer_recv, state->buffer_recv + i + 2, remaining);
            state->recv_len = remaining;

            // Zero out the rest of the buffer
            memset(state->buffer_recv + state->recv_len, 0, BUF_SIZE - state->recv_len);
            return ERR_OK;
        }
    }

    // Check if the buffer is full
    if (state->recv_len == BUF_SIZE) {
        // Handle the situation when buffer is full but CRLF was not found
        // DEBUG_printf("buffer full without CRLF\n");
        return ERR_ABRT;
    }
    return ERR_OK;
}

err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb) {
    // DEBUG_printf("tcp_server_poll_fn\n");
    return ERR_OK;
}

void tcp_server_err(void *arg, err_t err) {
    // DEBUG_printf("tcp_client_err_fn %d\n", err);
}

err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    if (err != ERR_OK || client_pcb == NULL) {
        // DEBUG_printf("Failure in accept\n");
        return ERR_VAL;
    }
    // DEBUG_printf("Client connected\n");

    state->client_pcb = client_pcb;
    tcp_arg(client_pcb, state);

    // Set up Callbacks
    tcp_sent(client_pcb, tcp_server_sent);
    tcp_recv(client_pcb, tcp_server_recv);
    tcp_poll(client_pcb, tcp_server_poll, POLL_TIME_S * 2);
    tcp_err(client_pcb, tcp_server_err);

    // Say Hello
    return tcp_server_write_string(client_pcb, "RPi Pico W Telnet\r\n>\0");
}

bool tcp_server_open(void *arg) {
    TCP_SERVER_T *state = (TCP_SERVER_T*)arg;
    // DEBUG_printf("Starting server at %s on port %u\n", ip4addr_ntoa(netif_ip4_addr(netif_list)), TCP_PORT);

    struct tcp_pcb *pcb = tcp_new_ip_type(IPADDR_TYPE_ANY);
    if (!pcb) {
        // DEBUG_printf("failed to create pcb\n");
        return false;
    }

    err_t err = tcp_bind(pcb, NULL, TCP_PORT);
    if (err) {
        // DEBUG_printf("failed to bind to port %u\n", TCP_PORT);
        return false;
    }

    state->server_pcb = tcp_listen_with_backlog(pcb, 1);
    if (!state->server_pcb) {
        // DEBUG_printf("failed to listen\n");
        if (pcb) {
            tcp_close(pcb);
        }
        return false;
    }

    tcp_arg(state->server_pcb, state);
    tcp_accept(state->server_pcb, tcp_server_accept);

    return true;
}

void buffer_to_hex(const char* src, char* dest, size_t length) {
    for (size_t i = 0; i < length; ++i) {
        sprintf(dest + i * 2, "%02x", (unsigned char)src[i]);
    }
}