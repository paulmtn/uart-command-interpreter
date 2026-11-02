#include <avr/io.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <avr/interrupt.h>

/* Defines */
/* MCU Configs*/
#define F_CPU 16000000UL
#define BAUD 9600UL
#define UBRR ((F_CPU / (16 * BAUD)) - 1)
#define RING_SIZE 64

#define MAX_COMMAND_LENGTH 100

/* MESSAGES */
static const char HELP_MSG[] = 
    "Available commands:\r\n"
    "  help\r\n"
    "    - show this message\r\n"
    "  led  <on | off | toggle>\r\n" 
    "    - turns built-in LED on or off\r\n"
    "  uptime\r\n" 
    "    - prints uptime of the program\r\n";

/* Prototypes */
void cmd_help(const char *args);
void cmd_led(const char *args);
void cmd_uptime(const char *args);
void process_keypress(void);
void uart_send_string(const char *str);

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

static volatile uint8_t head;
static volatile uint8_t tail;
static volatile char ring_buf[RING_SIZE];
static volatile uint32_t ms_ticks;

static command_buffer cmd_buf;

static const cmd_entry cmd_table[] = {
    {"help", cmd_help},
    {"led", cmd_led},
    {"uptime", cmd_uptime},
};

/* init */

void timer_init(void) {
    // enable CTC mode
    TCCR0A |= (1 << WGM01);

    // set the value where the clock will raise interrupt and reset at (16000000/64/1000 = 250 timer ticks a millisecond)
    OCR0A = 250 - 1;

    TIMSK0 |= (1 << OCIE0A);

    // enable timer with prescaler 64
    TCCR0B |= (1 << CS01) | (1 << CS00);
}

void uart_init(void) {
    UBRR0H = (uint8_t)(UBRR >> 8); // upper 4 bits
    UBRR0L = (uint8_t)(UBRR);

    // Enable RX, TX, and interrupt driven
    UCSR0B = (1 << RXEN0) | (1 << TXEN0) | (1 << RXCIE0);

    // Set to 8 bit data frames
    UCSR0C = (1<< UCSZ01) | (1 << UCSZ00);
}

void main_init(void) {
    timer_init();
    uart_init();
    uart_send_string("Ready\r\n$ ");
    sei();
}

/* Command Handling */
int process_command(command_buffer *cmd_buf) {
    // Gets command and its args. Adds null terminators. Handles both cases where
    // there are arguments and when not
    char *space_ptr = strchr(cmd_buf->input, ' ');
    char *command = cmd_buf->input;
    char *args = "";

    // Set the space separating the command and args to the null terminator
    if (space_ptr != NULL) {
        *space_ptr = '\0';
        args = space_ptr + 1;
    }
    
    size_t cmd_table_size = sizeof(cmd_table) / sizeof(cmd_entry);
    
    // Loop through each command in the table and compare to whats in the command buffer
    for (size_t i = 0; i <  cmd_table_size; i++) {
        if (strcmp(cmd_table[i].name, command) == 0) {
            cmd_table[i].handler(args);
            return 0;
        }
    }
    return -1;
}

/* UART Input Handling */
void uart_send(const char c) {
    // Wait until transmit buffer is empty
    while (!(UCSR0A & (1 << UDRE0)));

    UDR0 = c;
}

void uart_send_string(const char *str) {
    while (*str != '\0') {
        uart_send(*str);
        str++;
    }
}

int cmd_buf_append(const char c) {
    // -1 so we dont index error when putting null terminators in process_command()
    if (cmd_buf.len >= MAX_COMMAND_LENGTH - 1) {
        return -1;
    }

    cmd_buf.input[cmd_buf.len++] = c;
    cmd_buf.input[cmd_buf.len] = '\0';
    return 0;
}

void reset_cmd_buf(void) {
    cmd_buf.len = 0;
    cmd_buf.input[0] = '\0';
}

/* Interrupt & Buffer stuff */
int data_available(void) {
    return head != tail;
}

char data_read(void) {
    char c = ring_buf[tail];
    tail = (tail + 1) & (RING_SIZE - 1);
    return c;
}

ISR(USART_RX_vect) {
    char c = UDR0;
    // When head + 1 reaches 64, it zeros out since RING_SIZE - 1 is 0011 1111 and head + 1 is 0100 0000
    uint8_t next = (head + 1) & (RING_SIZE - 1);
    if (next == tail) {
        return;
    }
    ring_buf[head] = c;
    head = next;
}

uint32_t get_ticks(void) {
    // the interrupt is processed late which doesn't cause any timing errors
    uint8_t saved = SREG; // copies the status register into a local variable
    cli(); // clears the interrupt bit so ISR cant run
    uint32_t t = ms_ticks;
    SREG = saved;
    return t;
}

/* Input Handling */
void backspace() {
    if (cmd_buf.len == 0) return;
    cmd_buf.len --;
    cmd_buf.input[cmd_buf.len] = '\0'; // seal the input with the null terminator
    uart_send_string("\b");
    uart_send_string(" ");
    uart_send_string("\x1B[1D");
}

void process_keypress(void) {
    char c = data_read();

    switch (c) {
        case '\r': // Hitting enter returns \r only and we must send \r\n
            uart_send_string("\r\n");
            if (cmd_buf.len > 0){
                if (process_command(&cmd_buf) != 0){
                    uart_send_string("Unrecognized Command\r\n");
                }
            }
            uart_send_string("$ ");
            reset_cmd_buf();
            break;
        case '\b':
        case 0x7F:
            backspace();
            break;
        default:
            if (cmd_buf_append(c) != -1) uart_send(c);
            break;
    }
}

/* Timer */
ISR(TIMER0_COMPA_vect){
    ms_ticks++;
}



// Command handlers
void cmd_help(const char *args) {
    uart_send_string(HELP_MSG);
}

void cmd_led(const char *args) {
    // Set PB5 as an output pin
    DDRB |= (1 << PB5);

    if (strcmp("on", args) == 0) {
        PORTB |= (1 << PB5);  // LED on
    } else if (strcmp("off", args) == 0) {
        PORTB &= ~(1 << PB5);  // LED off
    } else if (strcmp("toggle", args) == 0) {
        PORTB ^= (1 << PB5);
    }
}

void cmd_uptime(const char *args) {
    char buf[40];
    uint32_t seconds = get_ticks() / 1000;

    snprintf(buf, sizeof(buf), "Uptime (s): %lu\r\n", (unsigned long) seconds);
    uart_send_string(buf);
}
