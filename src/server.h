#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "lwip/pbuf.h"
#include "lwip/tcp.h"

#ifndef SERVER
#define SERVER

#define TCP_PORT 22
#define BUF_SIZE 4096
#define TEST_ITERATIONS 10
#define POLL_TIME_S 5

typedef struct TCP_SERVER_T_ {
    struct tcp_pcb *server_pcb;
    struct tcp_pcb *client_pcb;
    bool complete;
    uint8_t buffer_sent[BUF_SIZE];
    uint8_t buffer_recv[BUF_SIZE];
    int sent_len;
    int recv_len;
    int run_count;
} TCP_SERVER_T;

TCP_SERVER_T* tcp_server_init(void);
err_t tcp_server_close(void *arg);
err_t tcp_server_sent(void *arg, struct tcp_pcb *tpcb, u16_t len);
err_t tcp_server_poll(void *arg, struct tcp_pcb *tpcb);
void tcp_server_err(void *arg, err_t err);
err_t tcp_server_accept(void *arg, struct tcp_pcb *client_pcb, err_t err);
bool tcp_server_open(void *arg);
err_t tcp_server_handle_response(void *arg, struct tcp_pcb *tpcb, char *buffer, int length);
err_t tcp_server_recv(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
err_t tcp_server_write_string(struct tcp_pcb *tpcb, char *buffer);

// Command Handlers
err_t tcp_server_handle_exit(void *arg, struct tcp_pcb *tpcb);
err_t tcp_server_handle_flash_read(struct tcp_pcb *tpcb);
err_t tcp_server_handle_flash_clear(struct tcp_pcb *tpcb);
err_t tcp_server_handle_flash_write(struct tcp_pcb *tpcb, char *buffer, int length);
err_t tcp_server_handle_unrecognized(struct tcp_pcb *tpcb, char *buffer, int length);

// Util
void buffer_to_hex(const char* src, char* dest, size_t length);

#endif