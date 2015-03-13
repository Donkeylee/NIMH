typedef	unsigned char		UINT8;
typedef	unsigned int		UINT16;
typedef	unsigned long		UINT32;


#define STOP (UINT16)0
#define CHARGE (UINT16)1
#define DISCHARGE (UINT16)2
#define REPEAT (UINT16)3

#define	INVALID8	((UINT8) -1)
#define	INVALID16	((UINT16) -1)
#define	INVALID32	((UINT32) -1)







void putch(char TX);
void PRINTF(char *data, ...);
void take_over(char recived_data);
SIGNAL(SIG_USART_RECV);
SIGNAL(SIG_OUTPUT_COMPARE1A);





