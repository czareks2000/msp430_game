#include <msp430x14x.h>
#include "lcd.h"
#include "portyLcd.h"

#define         B1                 BIT4&P4IN         // Klawisz B1 - P4.4
#define         B2                 BIT5&P4IN         // Klawisz B2 - P4.4
#define         B3                 BIT6&P4IN         // Klawisz B3 - P4.4
#define         B4                 BIT7&P4IN         // Klawisz B4 - P4.4

unsigned char i;
unsigned char znak;

void gameover()
{
  clearDisplay();
  SEND_CHAR('G');
  SEND_CHAR('A');
  SEND_CHAR('M');
  SEND_CHAR('E');
  SEND_CHAR(' ');
  SEND_CHAR('O');
  SEND_CHAR('V');
  SEND_CHAR('E');
  SEND_CHAR('R');
  while(1)
  {
    
     if(!((P4IN & BIT7)==BIT7))             //detekcja guzika B4
     {  
         WDTCTL=0x00; //przerwanie, powoduj¹ce powrót do main
     }
  }
}

int pow2(int a)
{
  int res = 2;
  for(int i=2; i<=a; ++i)
  {
    res*=i;
  }
  return res;
}


void main( void )
{
znak='A';
WDTCTL=WDTPW+ WDTHOLD;                        // zatrzymanie WDT

P1DIR = 0x00; 
P4DIR = 0x00; 

P1SEL = 0x00; 
P1DIR = 0xff; 

P2OUT |= BIT1;  // wylaczanie status leda

P1OUT |= BIT5;  //REL1 ON

InitPortsLcd();                               // inicjalizacja portów  
InitLCD();                                    // inicjalizacja LCD
clearDisplay();                               // czyszczenie LCD  


P2OUT |= BIT1;  // wylaczanie status leda

unsigned char ship_pos = 0x00;
unsigned short draw_obst = 0;
char line_a[14] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char line_b[14] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
//unsigned short line_b = 0x0000f;

while (1)                                    // nieskoñczona pêtla 
{
  Delayx100us(200);
  Delayx100us(200);
  Delayx100us(200);
  
  ++draw_obst;
  if(draw_obst > 9)
    draw_obst = 0;
  
  //line_a *=2;
  //line_b *=2;
  if(draw_obst == 1)
    line_a[13] = 1;
   if(draw_obst == 6)
    line_b[13] = 1;
  
   
  if((line_a[0] == 1) && (ship_pos == 0x00))
     gameover();
   if((line_b[0] == 1) && (ship_pos == 0x01))
      gameover();
  
  SEND_CMD(DD_RAM_ADDR);
  P2OUT |= BIT1;
  clearDisplay();
  if(ship_pos == BIT0)
    SEND_CMD(DD_RAM_ADDR2);
  SEND_CHAR('O');
  SEND_CHAR('>');
  SEND_CMD(DD_RAM_ADDR+2);
  
  for(int i=0; i<=13; ++i)
      {
        line_a[i] = line_a[i+1];
        line_b[i] = line_b[i+1];
        if(line_a[i] == 1)
        {
          SEND_CMD(DD_RAM_ADDR+2+i);
            SEND_CHAR('X');
         // SEND_CMD(DD_RAM_ADDR2); SEND_CHAR('*'); //info ze jest przeszkoda
        }
       //   else SEND_CHAR('.');

        if(line_b[i] == 1)
        {
          SEND_CMD(DD_RAM_ADDR2+2+i);
            SEND_CHAR('X');
          //  SEND_CMD(DD_RAM_ADDR2); SEND_CHAR('*'); //info ze jest przeszkoda
          
        }
       //   else SEND_CHAR('.');
      }
  line_a[13] = 0;
      
  /*
  for(int i=13; i>=1; --i)
  {
    
    power = pow2(i);
    if((power & line_a) == power)
    {
      
      
    }
   
  }
  */


 if(!((P4IN & BIT4)==BIT4))             //detekcja guzika B1
 {  
   
   ship_pos ^= BIT0;
   P2OUT &= ~BIT1;  
   Delayx100us(50);
 }

 if(!((P4IN & BIT7)==BIT7))             //detekcja guzika B3
 {  
   while(1){};
   
 }
 /*
 if(!((P4IN & BIT7)==BIT7))             //detekcja guzika B4
 {  
   WDTCTL=0x00; 
   
 }*/
 
 /*if ((P4IN & BIT5)==BIT5)             //detekcja guzika B2
 {
   
   P2OUT &= ~BIT1;  
  
   SEND_CMD(DATA_ROL_RIGHT);
   Delayx100us(20);
 }
}*/
}
}
/*
 if ((P4IN & BIT6)==BIT6)             //detekcja guzika B3
 {
   Delayx100us(2500);  
   P2OUT &= ~BIT1;  
  
   ch_cnt = ch_cnt + SEND_CHAR(znak);
   SEND_CHAR(znak);
   //P2OUT ^=BIT1;
 }
 
 if ((P4IN & BIT7)==BIT7)             //detekcja guzika B4
 {
   Delayx100us(2500);  
   P2OUT &= ~BIT1;  
  
   ch_cnt = ch_cnt + SEND_CHAR(znak);
   SEND_CHAR(znak);
   //P2OUT ^=BIT1;
 } */

/*


for (i=0;i<16;i++)
 {
   //SEND_CMD(DATA_ROL_LEFT);                  // przesuwanie napisu w lewo    
    ch_cnt += SEND_CHAR(znak);
    Delayx100us(2500);                        // opóŸnienie
  }
 }*/
