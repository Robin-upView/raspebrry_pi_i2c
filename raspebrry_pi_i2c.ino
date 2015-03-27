#include <WSWire.h> 

 

#define  SLAVE_ADDRESS         0x29  //slave address, any number from 0x01 to 0x7F

#define  REG_MAP_SIZE             14

#define  MAX_SENT_BYTES       10

#define I2C_ADDR                 0x29             
#define TWI_FREQ_SETTING         400000L       // 400KHz for I2C
#define CPU_FREQ                 16000000L     // 16MHz

//RC Receiver
//A variable must be declared volatile whenever its value can be changed by an interrupt service routine.
volatile unsigned long startPeriod; // set in the interrupt
volatile int16_t rc[7]; //nb of RC channels
volatile bool dataReady;
bool dataReceive;

/********* Global  Variables  ***********/

byte registerMap[REG_MAP_SIZE];

byte registerMapTemp[REG_MAP_SIZE - 1];

byte receivedCommands[MAX_SENT_BYTES];

byte  mot0_0,mot0_1,mot1_0,mot1_1,mot2_0,mot2_1,mot3_0,mot3_1;

int mot0, mot1, mot2, mot3;

void setup()

{
    //function calcInput is called everytime there in a falling edge on pin 2. 
  //Pin 2 is hard-wired to interrupt 0
  
  TWBR = ((CPU_FREQ / TWI_FREQ_SETTING) - 16) / 2;
  
     Wire.begin(SLAVE_ADDRESS); 

     attachInterrupt(0,calcInput,FALLING); 
     
     Serial.begin(115200);

    registerMap[0]=100;
    registerMap[1]=50;
    registerMap[2]=20;
    registerMap[3]=40;
    registerMap[4]=10;
    registerMap[5]=0;
    registerMap[6]=0;
    registerMap[7]=0;
    
     Wire.onRequest(requestEvent);

     Wire.onReceive(receiveEvent);
   
    
}



void loop()

{
  /*
  Serial.print(zeroBData);
  Serial.print(" ");
  Serial.print(rc[0]);
  Serial.print(" ");
  Serial.print(rc[1]);
  Serial.print(" ");
  Serial.print(rc[2]);
  Serial.print(" ");
  Serial.print(rc[3]);
  Serial.print(" ");
  Serial.println(rc[4]);
  */
  /*
  Serial.print(registerMapTemp[0]<<8 | registerMapTemp[1]);
  Serial.print(" ");
  //Serial.print(registerMapTemp[1]);
  //Serial.print(" ");
  Serial.print(registerMapTemp[2]<<8 | registerMapTemp[3]);
  Serial.print(" ");
  //Serial.print(registerMapTemp[3]);
  Serial.print(" ");
  Serial.print(registerMapTemp[4]<<8 | registerMapTemp[5]);
  Serial.print(" ");
  //Serial.print(registerMapTemp[5]);
  Serial.print(" ");
  Serial.print(registerMapTemp[6]<<8 | registerMapTemp[7]);
  Serial.println(" ");
  //Serial.println(registerMapTemp[7]);
  */
  
  /*
  Serial.print(mot0_0);
  Serial.print(" ");
  Serial.print(mot0_1);
  Serial.print(" ");
  Serial.print(mot1_0);
  Serial.print(" ");
  Serial.print(mot1_1);
  Serial.print(" ");
  Serial.print(mot2_0);
  Serial.print(" ");
  Serial.print(mot2_1);
  Serial.print(" ");
  Serial.print(mot3_0);
  Serial.print(" ");
  Serial.println(mot3_1);
  */
  
  if(dataReceive == true)
  {
    dataReceive =false;
    mot0 = mot0_0<<8 | mot0_1;
    mot1 = mot1_0<<8 | mot1_1;
    mot2 = mot2_0<<8 | mot2_1;
    mot3 = mot3_0<<8 | mot3_1;
    /*
    Serial.print(mot0);
    Serial.print(" ");
    Serial.print(mot1);
    Serial.print(" ");
    Serial.print(mot2);
    Serial.print(" ");
    Serial.print(mot3);
    Serial.println(" ");
    */
  }
  
  if(dataReady==true)
  {
    dataReady=false;
    storeData(); 
  }
   
   
  //delay(10);
}

 

void requestEvent()

{
  
   Wire.write(registerMap+receivedCommands[0], REG_MAP_SIZE);  //Set the buffer up to send all 14 bytes of data

}

 

void receiveEvent(int bytesReceived)

{
 

     for (int a = 0; a < bytesReceived; a++)

     {

          if ( a < MAX_SENT_BYTES)

          {

               receivedCommands[a] = Wire.read();

          }

          else

          {

               Wire.read();  // if we receive more data then allowed just throw it away

          }

     }

     if(bytesReceived == 1 && (receivedCommands[0] < REG_MAP_SIZE))

     {

          return; 

     }

     if(bytesReceived == 1 && (receivedCommands[0] >= REG_MAP_SIZE))

     {

          receivedCommands[0] = 0x00;

          return;

     }

    switch(receivedCommands[0]){

          case 0x0B:

                mot0_0 = receivedCommands[1]; //save the data to a separate variable
                mot0_1 = receivedCommands[2];
                mot1_0=receivedCommands[3];
                mot1_1=receivedCommands[4];
                mot2_0=receivedCommands[5];
                mot2_1=receivedCommands[6];
                mot3_0=receivedCommands[7];
                mot3_1=receivedCommands[8];
                dataReceive = true;

               return; //we simply return here because the most bytes we can receive is three anyway

               break;


          default:

               return; // ignore the commands and return

     }
     

}

 //Interrupt service routine called everytime the digital PPM signal from the RC receiver is falling on pin 2 
void calcInput()
{
  noInterrupts();
  //static variables are not reset when we exit the function
  static unsigned int pulseIn;
  static int channel;
  
      //length of current pulse
      pulseIn = (int)(micros() - startPeriod);
      
      //remember the time for next loop
      startPeriod = micros();

      //channel detector
      if(pulseIn >2000){
        dataReady=true;
        channel = 0;
      }
      //store value
      else
      {
        rc[channel]=pulseIn;
        channel++; //increment channel for next time
      }
      interrupts();
}

void storeData()

{

     byte * bytePointer;  //we declare a pointer as type byte

     byte arrayIndex = 0; //we need to keep track of where we are storing data in the array


     bytePointer = (byte*)&rc[0]; //latitude is 4 bytes
     
     
     
     for (int i = 1; i > -1; i--)

     {

          registerMapTemp[arrayIndex] = bytePointer[i];  //increment pointer to store each byte

          arrayIndex++;

     }

     bytePointer = (byte*)&rc[1]; //latitude is 4 bytes

     for (int i = 1; i > -1; i--)

     {

          registerMapTemp[arrayIndex] = bytePointer[i];  //increment pointer to store each byte

          arrayIndex++;

     }
     
      bytePointer = (byte*)&rc[2]; //latitude is 4 bytes

     for (int i = 1; i > -1; i--)

     {

          registerMapTemp[arrayIndex] = bytePointer[i];  //increment pointer to store each byte

          arrayIndex++;

     }
     
          bytePointer = (byte*)&rc[3]; //latitude is 4 bytes

     for (int i = 1; i > -1; i--)

     {

          registerMapTemp[arrayIndex] = bytePointer[i];  //increment pointer to store each byte

          arrayIndex++;

     }
/*
  Serial.print(registerMapTemp[0]);
  Serial.print(" ");
  Serial.print(registerMapTemp[1]<<1);
  Serial.print(" ");
  Serial.print(registerMapTemp[2]);
  Serial.print(" ");
  Serial.print(registerMapTemp[3]);
  Serial.print(" ");
  Serial.print(registerMapTemp[4]);
  Serial.print(" ");
  Serial.print(registerMapTemp[5]);
  Serial.print(" ");
  Serial.print(registerMapTemp[6]);
  Serial.print(" ");
  Serial.println(registerMapTemp[7]);
 */
 
  noInterrupts();
  for(int c=0; c<(REG_MAP_SIZE-1);c++)
  {
    registerMap[c]=registerMapTemp[c];
  }
  interrupts();
  

}
