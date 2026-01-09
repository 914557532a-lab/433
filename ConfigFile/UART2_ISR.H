//-----------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
#define RXD_LENGTH	40 //接收缓冲区长度
#define TXD_LENGTH	35 //发送缓冲区长度

#define UART2_Direction_SET		P6 |= 0x04
#define UART2_Direction_CLR		P6 &= ~(0x04)

//-----------------------------------------------------------------------------
extern xdata unsigned char ReceiveIndex;
extern xdata unsigned char UART2_OverTime;//接收超时

extern xdata unsigned char UART2_SendHoldTime;
extern xdata unsigned char TakeRunStateTime;
extern xdata unsigned char UART_ERROR_Time;

extern xdata unsigned char UART_2_RXD[RXD_LENGTH];
extern xdata union{//Array[1]=MSB,Array[0]=LSB, STM8的编译器中 Array[0]=MSB,Array[1]=LSB
    unsigned char CRC_Array[2];
    unsigned int CRC_REG;//CRC值
 }CRC_DATA;//----存储CRC 计算结果
//-----------------------------------------------------------------------------
void Uart2_Initial(unsigned long int baudrate);
void CRC16_CHECK(unsigned char *Message, unsigned int Length);
void UART2_DataProcess(void);
void UART2_DataTransmit (void);
