#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

/* Defines */
/* MCU Configs*/
#define F_CPU 16000000UL
#define BAUD 9600UL
#define UBRR ((F_CPU / (16 * BAUD)) - 1)

#define MAX_COMMAND_LENGTH 100


/* Prototypes */
void cmd_help(const char *args);
void process_keypress(void);

// Declares the cmd_handler_t type as a pointer to a handler function
typedef void (*cmd_handler_t)(const char *args);

typedef struct {
    const char *name;
    cmd_handler_t handler;
} cmd_entry;

typedef struct {
    char input[MAX_COMMAND_LENGTH];
    int len;
} command_buffer;

static command_buffer cmd_buf;

static const cmd_entry cmd_table[] = {
    {"help", cmd_help},
};

void uart_init(void) {
    UBRR0H = (uint8_t)(UBRR >> 8); // upper 4 bits
    UBRR0L = (uint8_t)(UBRR);

    // Enable RX and TX
    UCSR0B = (1 << RXEN0) | (1 << TXEN0);

    // Set to 8 bit data frames
    UCSR0C = (1<< UCSZ01) | (1 << UCSZ00);
}

/* Command Handling */
void process_command(command_buffer *cmd_buf) {
    cmd_handler_t handler_function;

    // Gets command and its args. Adds null terminators. Handles both cases where
    // there are arguments and when not
    char *space_ptr = memchr(cmd_buf->input, ' ', cmd_buf->len);
    char *command = cmd_buf->input;
    int space_idx;
    char *args;
    if (space_ptr == NULL) {
        command[cmd_buf->len] = '\0';
        args = "";
    } else {
        space_idx = space_ptr - cmd_buf->input;
        command[space_idx] = '\0';
        args = space_ptr + 1;
        command[cmd_buf->len] = '\0';
    }

    int cmd_table_size = sizeof(cmd_table) / sizeof(cmd_entry);
    
    // Loop through each command in the table and compare to whats in the command buffer
    for (size_t i = 0; i < (size_t) cmd_table_size; i++) {
        if (!strcmp(cmd_table[i].name, command)) {
            handler_function = cmd_table[i].handler;
            handler_function(args);
            return;
        }
    }


}

/* UART Input Handling */
char uart_receive(void) {
    // RXC0 is set when there is unread data in the receive buffer
    while (!(UCSR0A & (1 << RXC0)));

    return UDR0;
}

void uart_send(char c) {
    // Wait until transmit buffer is empty
    while (!(UCSR0A & (1 << UDRE0)));

    UDR0 = c;
}

void uart_send_string(char *str, int len) {
    for (int i = 0; i < len; i++) {
        uart_send(str[i]);
    }
}

void cmd_buf_append(char c) {
    // -1 so we dont index error when putting null terminators in process_command()
    if (cmd_buf.len >= MAX_COMMAND_LENGTH - 1) {
        return;
    }

    cmd_buf.input[cmd_buf.len++] = c;
}

void reset_cmd_buf(void) {
    cmd_buf.len = 0;
}


/* Input Handling */
void process_keypress(void) {
    char c = uart_receive();

    switch (c) {
        case '\r': // Hitting enter returns \r only and we must send \r\n
            uart_send_string("\r\n", 2);
            process_command(&cmd_buf);
            reset_cmd_buf();
            break;
        case 'q':
            uart_send_string("pressed q", 9);
            break;
        default:
            uart_send(c);
            cmd_buf_append(c);
            break;
    }
}

// Command handlers
void cmd_help(const char *args) {
    uart_send_string("helped", 6);
}

