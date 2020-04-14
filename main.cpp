#include "mbed.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#define UINT14_MAX        16383
// FXOS8700CQ I2C address
#define FXOS8700CQ_SLAVE_ADDR0 (0x1E<<1) // with pins SA0=0, SA1=0
#define FXOS8700CQ_SLAVE_ADDR1 (0x1D<<1) // with pins SA0=1, SA1=0
#define FXOS8700CQ_SLAVE_ADDR2 (0x1C<<1) // with pins SA0=0, SA1=1
#define FXOS8700CQ_SLAVE_ADDR3 (0x1F<<1) // with pins SA0=1, SA1=1
// FXOS8700CQ internal register addresses
#define FXOS8700Q_STATUS 0x00
#define FXOS8700Q_OUT_X_MSB 0x01
#define FXOS8700Q_OUT_Y_MSB 0x03
#define FXOS8700Q_OUT_Z_MSB 0x05
#define FXOS8700Q_M_OUT_X_MSB 0x33
#define FXOS8700Q_M_OUT_Y_MSB 0x35
#define FXOS8700Q_M_OUT_Z_MSB 0x37
#define FXOS8700Q_WHOAMI 0x0D
#define FXOS8700Q_XYZ_DATA_CFG 0x0E
#define FXOS8700Q_CTRL_REG1 0x2A
#define FXOS8700Q_M_CTRL_REG1 0x5B
#define FXOS8700Q_M_CTRL_REG2 0x5C
#define FXOS8700Q_WHOAMI_VAL 0xC7

InterruptIn btn(SW2); // btn input for interrupt
I2C i2c( PTD9,PTD8); 
Serial pc(USBTX, USBRX);
int m_addr = FXOS8700CQ_SLAVE_ADDR1;
DigitalOut led2(LED2); // GREEN LED output

Thread thread1(osPriorityLow);
Thread thread2(osPriorityNormal);
EventQueue queue1(32 * EVENTS_EVENT_SIZE);
EventQueue queue2(32 * EVENTS_EVENT_SIZE);

/*-------------------log acc vector function-------------------*/

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len);
void FXOS8700CQ_writeRegs(uint8_t * data, int len);
void acc_measure() {

   pc.baud(115200);

   uint8_t who_am_i, data[2], res[6];
   int16_t acc16;
   float t[3];
   int tilt;

   // Enable the FXOS8700Q

   FXOS8700CQ_readRegs( FXOS8700Q_CTRL_REG1, &data[1], 1);
   data[1] |= 0x01;
   data[0] = FXOS8700Q_CTRL_REG1;
   FXOS8700CQ_writeRegs(data, 2);

   // Get the slave address
   FXOS8700CQ_readRegs(FXOS8700Q_WHOAMI, &who_am_i, 1);

   int j;
   for(j=0;j<100;j++){ // for sampling 100 times ( 10 seconds / 0.1second )

      FXOS8700CQ_readRegs(FXOS8700Q_OUT_X_MSB, res, 6);

      acc16 = (res[0] << 6) | (res[1] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      t[0] = ((float)acc16) / 4096.0f;

      acc16 = (res[2] << 6) | (res[3] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      t[1] = ((float)acc16) / 4096.0f;

      acc16 = (res[4] << 6) | (res[5] >> 2);
      if (acc16 > UINT14_MAX/2)
         acc16 -= UINT14_MAX;
      t[2] = ((float)acc16) / 4096.0f;
      if(t[0]>0.5||t[0]<-0.5||t[1]>0.5||t[1]<-0.5) // if x or y tilt > 45 degree
      {
         tilt = 1;
      }
      else // x & y tilt < 45 degree
      {
         tilt = 0;
      }
      
      printf("%1.4f\r\n%1.4f\r\n%1.4f\r\n%d\r\n",t[0],t[1],t[2],tilt);
      // t[0] == X
      // t[1] == Y
      // t[2] == Z

      wait(0.1); // sample once in 0.1 second
   }
   
}

void FXOS8700CQ_readRegs(int addr, uint8_t * data, int len) {
   char t = addr;
   i2c.write(m_addr, &t, 1, true);
   i2c.read(m_addr, (char *)data, len);
}

void FXOS8700CQ_writeRegs(uint8_t * data, int len) {
   i2c.write(m_addr, (char *)data, len);
}

/*-------------------blink the LED function-------------------*/

void blink_led(){
   int i;
   led2 = 1;
   queue2.call(&acc_measure); // call() APIs of queue for acc vector logging
   for(i=0;i<20;i++){ // blinking 10 seconds
      led2 = !led2;
      wait(0.5); // blink once in one second
   }
}

int main()
{
   led2 = 1;
   thread1.start(callback(&queue1, &EventQueue::dispatch_forever));
   thread2.start(callback(&queue2, &EventQueue::dispatch_forever));
   
   btn.rise(queue1.event(blink_led));   
   while (1) {wait(1);}
}