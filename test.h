#define STOP (unsigned int)0
#define CHARGE (unsigned int)1
#define DISCHARGE (unsigned int)2
#define REPEAT (unsigned int)3






void putch(char TX);
void PRINTF(char *data, ...);
void take_over(char recived_data);
SIGNAL(SIG_USART_RECV);
SIGNAL(SIG_OUTPUT_COMPARE1A);





