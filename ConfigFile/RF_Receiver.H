//---------------------------------------------------------
//
//---------------------------------------------------------
#define RF_ADDRLEARNTIME	20




//---------------------------------------------------------
extern xdata unsigned char RF_LearnAddrssTime;
extern xdata unsigned char OutHoldTime;// output timer
extern xdata unsigned char RF_KEY_Hold[2];//按键值备份

void RF_Receiver_Initial(void);
void RF_SampleReceive(void);

void WirelessDecode(void);
