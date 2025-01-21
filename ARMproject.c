//Data Acquisition using ARM LPC1768

#include<lpc17xx.h>
#include<stdio.h>
#include<stdlib.h>
   //P0.6- P0.11 ,P0.15,P0.24  //P1.20,23,24		 //P2.10 //P1.29
#define pi LPC_PINCON
#define u LPC_UART0
#define p LPC_PWM1
#define s LPC_SC

char new[]="\r\n";

void uart_init()
{
   pi->PINSEL0|=(1<<4) | (1<<6);
   u->LCR=(1<<0) | (1<<1) | (1<<7);

   u->DLL=98;
   u->DLM=0;
   u->LCR&=~(1<<7);
}

void uart_tx(char *d)
{
  int i;
  for(i=0; d[i]!='\0'; i++)
  {
   while((u->LSR & (1<<5))==0);
    u->THR=d[i];
  } 
}

char uart_rx()
{
   char r;
   while((u->LSR & (1<<0))==0);
   r=u->RBR;
   return r;
}

void dc_pwm(int x)
{
  pi->PINSEL4 = (1<<0);			   //pwm1.1		 //T2->22 T3->23 ADC->12 :PCONP ->LPC_SC	LPC_PINCON->PINSEL
  pi->PINSEL4 = (1<<11);		   //pmw1.3
  p->TCR = (1<<0);// | (1<<1);			 //Timer Control register -> Start the Timer
  p->PR = 0x0;		 //15M /1khz	   one sec at 2khz	   //PR+1=timer fre/required fr
  p->MCR = (1<<1);		  // S, R I.F.	   //IR=   MR0:2:0 MR1:5:3 MR2:8:6 MR3:11:9
  p->MR0 = 100;		   //MRn=required delay/Time for 1 pulse	 //t=1/f  =2000 
  p->MR1 = x;
  p->LER = (1<<0) | (1<<1);
  p->PCR = (1<<9);  //(1<<10);
}

void pll_prg()
{
   s->SCS=(1<<5);

   while((s->SCS &(1<<6))==0);

    s->CLKSRCSEL=(1<<0);   //00  01 10
	s->PLL0CON=(1<<0);	  //Enable PLL0	  0 1
	s->PLL0CFG=14;		//	 360MHz    //12Mhz	 //CPU 100MHz	  //275MHz	to 550MHz
	//Pllout=2xpllinx(M+1)/(N+1)	//increasing the frequency	  //32Khz to 50Mhz //Controller frequency

	s->PLL0FEED=0xAA;		//Feed Sequence
    s->PLL0FEED=0x55;
	
	s->CCLKCFG=5;	  //  CClk=Pllout/(N+1)		 //100Mhz		60

	while((s->PLL0STAT &(1<<26))==0);  //LOCK

	s->PLL0CON|=(1<<1);	 //Select PLLout as CCLk input
	s->PLL0FEED=0xAA;
    s->PLL0FEED=0x55;
}

void delay()
{
   int i,j;
   for(i=0;i<=200;i++)
   for(j=0;j<=300;j++);
}

void dc_motor()
{
   char a[]="DC motor is selected.\r\n";  char e[]="Enter the duty cycle for dc motor:\r\n";
   int v,j;  char t[3];
   uart_tx(a);
   uart_tx(e);  
   for(j=0;j<2;j++)
     t[j]=uart_rx();  
   uart_tx(t);
   v=atoi(t);  
   dc_pwm(v);   
}

void stepper_clock(int n)
{
       int i;
	for(i=0;i<(n/7.2);i++)
   {
     LPC_GPIO0->FIOSET=(1<<6);
	  delay();
	 LPC_GPIO0->FIOCLR=(1<<6);

	 LPC_GPIO0->FIOSET=(1<<7);
	  delay();
	 LPC_GPIO0->FIOCLR=(1<<7);

	 LPC_GPIO0->FIOSET=(1<<8);
	  delay();
	 LPC_GPIO0->FIOCLR=(1<<8);

	 LPC_GPIO0->FIOSET=(1<<9);
	  delay();
	 LPC_GPIO0->FIOCLR=(1<<9);
   }
}

void stepper_anticlock(int n)
{
      int i;
   for(i=0;i<(n/7.2);i++)
   {
     LPC_GPIO0->FIOSET=(1<<9);
	  delay();
	 LPC_GPIO0->FIOCLR=(1<<9);

	 LPC_GPIO0->FIOSET=(1<<8);
	  delay();
	 LPC_GPIO0->FIOCLR=(1<<8);

	 LPC_GPIO0->FIOSET=(1<<7);
	  delay();
	 LPC_GPIO0->FIOCLR=(1<<7);

	 LPC_GPIO0->FIOSET=(1<<6);
	  delay();
	 LPC_GPIO0->FIOCLR=(1<<6);
   }
}

void stepper_motor()
{
    char v[]="Enter ,c for clock wise ,a for anti_clockwise rotation :\r\n "	,ch;
    char a[]="Enter the angle for stepper motor:\r\n";  int r,j;  char q[4];
	uart_tx(v);
	ch=uart_rx();
    uart_tx(new);
	if(ch=='c')
    {
	  uart_tx(a);
      for(j=0;j<=2;j++)
      q[j]=uart_rx();
      uart_tx(q);
   	  uart_tx(new);
      r=atoi(q); 
      stepper_clock(r);
   }
    else if(ch=='a')  
	{
	   uart_tx(a);
       for(j=0;j<=2;j++)
       q[j]=uart_rx();
       uart_tx(q);
   	   uart_tx(new);
       r=atoi(q);
	   stepper_anticlock(r);
	}
}

void Manual_mode()
{
  char d[]= "It is manual mode.\r\n";  char a[]="Enter the motors ,s  for stepper motor, d for dc motor :\r\n";
  char b[]="no motors\r\n";  char c[]="\r\nDo you want to continue any motor?\r\n",ch,ch1;  
  uart_tx(d);
l1: uart_tx(a);
    ch=uart_rx();
	uart_tx(new);
    if(ch=='s')       stepper_motor();
    else if(ch=='d')  dc_motor(); 
    else
	   uart_tx(b); 
	   uart_tx(c);
    ch1=uart_rx();
	while((u->LSR & (1<<5))==0);
    u->THR=ch1;
	uart_tx(new);
    if(ch1=='y')   goto l1; 
	else
	  uart_tx(b);
}

void temp_sensor()	//<=13M
{
   int res;  char a[]="Threshold Temperature is = 26 C\r\n";  char b[]="temperature = "; char c[3];
  // float res1;  	//AD0.n	  //01 //15:14
   LPC_PINCON->PINSEL1=(1<<14); //ad0.0    	  :p0.23	
   LPC_ADC->ADCR=(1<<0) | (5<<8) | (1<<21) | (1<<24);                          
   while((LPC_ADC->ADDR0 & (1<<31))==0);                  
   res=(LPC_ADC->ADDR0 & (0xfff<<4));  //15:4                 
   res=(res>>4);
   res=((3.3*res*100)/4096); 
   //res=20;   
   sprintf(c,"%d",res); 
    uart_tx(a);
   uart_tx(b);
   uart_tx(c); 
   	uart_tx(new); 
   if(res>15)  
   dc_pwm(90);
   else      
   dc_pwm(60);   
}

void Gas_sensor()
{
   int res; char a[]="Threshold of gas = 4%\r\n";  char b[]="Gas= ";   char d[3];   
   LPC_GPIO1->FIODIR=(1<<29);
   LPC_PINCON->PINSEL1|=(1<<16); //ad0.1   , p0.24 
   LPC_ADC->ADCR=(1<<1) | (5<<8) | (1<<21) | (1<<24);                          
   while((LPC_ADC->ADDR1 & (1<<31))==0);                  
   res=(LPC_ADC->ADDR1 & (0xfff<<4));                      
   res=(res>>4);
   res=((res*100)/4096);
   sprintf(d,"%d",res); 
       uart_tx(a);
   uart_tx(b);  
   uart_tx(d);
   	uart_tx(new); 
   if(res>4) LPC_GPIO1->FIOSET=(1<<29); 
}

void IR_sensor()
{
  char a[]="Object is detected\r\n";
  char b[]="Object is not detected\r\n";
//  LPC_GPIO0->FIODIR=(0<<20); 
  if((LPC_GPIO0->FIOPIN & (1<<20))!=0)
  {           
       uart_tx(a);
      stepper_clock(90);
   }
   else
   { 
        uart_tx(b);
     stepper_anticlock(90);
   }
}

void Auto_mode()
{
   char a[]= "It is Auto mode.\r\n";
   uart_tx(a); 
  temp_sensor();
   Gas_sensor();
   IR_sensor();
}

int main()
{  
  char data[]="Enter the mode - m for manual, a for auto :\r\n",ch,ch1,ch2; char b[]="No mode\r\n"; 
  char a[]="Do you want to continue any mode: y or n\r\n"; char d[]="THANK YOU!";  char r[]="Stop the Actuators Enter q\r\n"; 
  pll_prg();
  uart_init();
  p->MR1 = 0;
  LPC_SC->PCONP|=(1<<12);
 LPC_GPIO0->FIODIR=(0xf<<6)|(1<<10);    //P0.10: one pin dc motor
 LPC_GPIO0->FIOCLR=(0xf<<6)|(1<<10);
  while(1)
  {
   l2:uart_tx(data);
	ch=uart_rx();
	while((u->LSR & (1<<5))==0);
    u->THR=ch; 
	uart_tx(new);
   if(ch=='m')
      Manual_mode();
   else if(ch=='a')
     Auto_mode();
   else
     uart_tx(b);

	 uart_tx(r);
	 ch1=uart_rx();
	while((u->LSR & (1<<5))==0);
    u->THR=ch1;
	uart_tx(new);
	if(ch1=='q')
	 {
	   p->MR1 = 0;
	   LPC_GPIO1->FIOCLR=(1<<29);
	 }
	 uart_tx(a);
	 ch2=uart_rx();
	while((u->LSR & (1<<5))==0);
    u->THR=ch2;
	uart_tx(new);
	if(ch2=='y')    goto l2;
    if(ch2=='n')   
	 {
	   uart_tx(d); 
	   exit(0);
	 }
   }
}
