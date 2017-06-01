#include "io430.h"
#include <stdio.h>

typedef int bool;
enum { false, true };
 
#define full_speed 512
#define half_speed 450
#define stop_speed 0

//Save the data from the HC-SR04
unsigned int distance_in_cm=0;
unsigned int distance_past=0;

//Save the data from the accelerometer, a device that is part of the msp board
int accelerometer_value=0;
int accelerometer_past=0;

unsigned int timer_counter_HCSR04;
int speed_state=0;

char movment_pre_state=' ';
char movment_corrent_state=' ';

int DTMF_value1=0;
int DTMF_value2=0;

int delay = 0;

void config_motor(void)
{
  ////Settings GPIO For Motor
  P4SEL |= 0x7E;                    // P4 option select
  P4DIR |= 0x7E;                    // P4 outputs
  P4OUT = 0x00;

  TBCCR0 = 512-1;                   // PWM Period    
  //Right Motor Port 4.1->A-1B CCR1 4.2->A-1A CCR2
  TBCCR1 = half_speed;              // CCR1 Duty cycle
  TBCCTL1 &= ~OUTMOD_7;             // CCR1 reset, For Set "= OUTMOD_7" 
  TBCCR2 = half_speed;              // CCR2 Duty cycle
  TBCCTL2 &= ~OUTMOD_7;             // CCR2 reset, For Set "= OUTMOD_7"     


  //Left Motor  Port 4.3->B-1B CCR3 4.5->B-1A CCR5
  TBCCR3 = half_speed;              // CCR3 Duty cycle
  TBCCTL3 &= ~OUTMOD_7;             // CCR3 reset, For Set "= OUTMOD_7" 
  TBCCR5 = half_speed;              // CCR5 Duty cycle
  TBCCTL5 &= ~OUTMOD_7;             // CCR5 reset, For Set "= OUTMOD_7" 

  TBCTL = TBSSEL_2 + MC_1 + TBCLR;  // SMCLK, contmode, clear TBR  
}

void config_serial(void)
{
  //This Part is for communicate with the ESP8266
  P3SEL |= BIT4+BIT5;                       // P3.4,p3.5 UART option select  
  UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
  UCA0CTL1 |= UCSSEL_2;                     // SMCLK 
  UCA0BR0 = 9;                              // 1MHz 115200 (see User's Guide)
  UCA0BR1 = 0;                              // 1MHz 115200
  UCA0MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  UCA0IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt   
}

void config_accelerometer(void)
{

  P6DIR = 0x01;							//Set 6.0 as output
  P6OUT = 0x01; 						//Turn accelerometer ON

  /* ADC12 Sample Hold 0 Select Bit: 2 , ADC12 On/enable */
  ADC12CTL0 = ADC12SHT0_2 + ADC12ON;    // Set sampling time, turn on ADC12
  ADC12CTL1 =  ADC12SHP;                /* ADC12 Sample/Hold Pulse Mode */
  ADC12IE = 0x01;                           // Enable interrupt
  ADC12MCTL0 = ADC12INCH_1;             /* ADC12 Input Channel 1 */
  ADC12CTL0 |= ADC12ENC ;               // Conversion enabled 
  P6SEL |= 0x02 ;                       // P6.1 ADC option select  
}

void config_DTMF(void)
{
  P1DIR = 0x00;
  P8DIR &= ~BIT2;  
  P1IE |= 0x80;                                         // P1.7 interrupt enabled
  P1IFG &= ~0x80;                                       // P1.7 IFG cleared
}

////Control On the Motors

void RM_MF () { //Right Motor Move Forward
  TBCCTL1 = OUTMOD_7;             // CCR1 reset, For Set "= OUTMOD_7" 
  TBCCTL2 &= ~OUTMOD_7;             // CCR2 reset, For Set "= OUTMOD_7"    
}
void RM_MB () { //Right Motor Move Backward
  TBCCTL1 &= ~OUTMOD_7;             // CCR1 reset, For Set "= OUTMOD_7" 
  TBCCTL2 = OUTMOD_7;             // CCR2 reset, For Set "= OUTMOD_7"    
}
void RM_ST () { //Right Motor Stop
  TBCCTL1 &= ~OUTMOD_7;             // CCR1 reset, For Set "= OUTMOD_7" 
  TBCCTL2 &= ~OUTMOD_7;             // CCR2 reset, For Set "= OUTMOD_7"    
}

void LM_MF () { //Left Motor Move Forward
  TBCCTL5 = OUTMOD_7;             // CCR1 reset, For Set "= OUTMOD_7" 
  TBCCTL3 &= ~OUTMOD_7;             // CCR2 reset, For Set "= OUTMOD_7"    
}
void LM_MB () { //Left Motor Move Backward
  TBCCTL5 &= ~OUTMOD_7;             // CCR1 reset, For Set "= OUTMOD_7" 
  TBCCTL3 = OUTMOD_7;             // CCR2 reset, For Set "= OUTMOD_7"    
}
void LM_ST () { //Left Motor Stop  
  TBCCTL5 &= ~OUTMOD_7;             // CCR1 reset, For Set "= OUTMOD_7" 
  TBCCTL3 &= ~OUTMOD_7;             // CCR2 reset, For Set "= OUTMOD_7"    
}

void config_HCSR04(){
  //Distance Sensor
  //The Sensor Work with interrupt, it send an echo and when the signal has return it pop an interrupt 

  //echo   
  P1DIR &= ~BIT2;
  P1SEL = BIT2;   // Select P1.2 as timer trigger.
  /* Timer A0 configure to read echo signal: 
  CM_3 = Both edges, SCS = sychronize, CAP = Capture Mode, CCIE = Interrupt Enable
  */
  TA0CCTL1 |= CM_3 + SCS + CCIS_0 + CAP + CCIE; 
  /* Timer A Control configuration =>
  Timer A clock source select: 1 - SMClock + 
  Timer A mode control: 2 - Continous up + 
  Timer A clock input divider 0 - No divider */
  TA0CTL |= TASSEL_2 + MC_2 +ID_0;  

  //TrigGer
  /* set P8.6 to output direction (trigger) */  
  P8DIR |= BIT6;
  P8OUT &= ~BIT6;
}

void take_Measure(){
  //This Function take measure
  P8OUT |= BIT6; 				// Start Sending the signal
  __delay_cycles(10);			// 10us wide
  P8OUT &= ~BIT6;  				// Stop Sending the signal
  __delay_cycles(10);			 
  ADC12CTL0 |= ADC12SC;        // Sampling open
}

int main( void ){
  // Stop watchdog timer to prevent time out reset

  WDTCTL = WDTPW + WDTHOLD;

  ////Serial and motor Settings
  config_serial();
  config_DTMF();  
  config_motor();
  config_HCSR04();
  config_accelerometer();

  __bis_SR_register(GIE);       // Enter LPM0, enable interrupts LPM0_bits + 
  //__no_operation();                         // For debugger
  while(true){
    take_Masure();
    for (delay=0;delay<30000;delay++){
      __no_operation();
    }
  }
}



// Echo back RXed character, confirm TX buffer is ready first
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
  switch(__even_in_range(UCA0IV,4)){
    case 0:break;                             // Vector 0 - no interrupt
    case 2:                                   // Vector 2 - RXIFG
      while (!(UCA0IFG&UCTXIFG));             // USCI_A1 TX buffer ready?
      movment_corrent_state = UCA0RXBUF;
      //Move Forward
      if (movment_corrent_state == 'F'){
        RM_MF();
        LM_MF();
      }
      //Stop
      else if (movment_corrent_state == 'S'){
        RM_ST();
        LM_ST();  
      }    
      //Move Back
      else if (movment_corrent_state == 'B'){
        RM_MB();
        LM_MB();  
      }
      //Move Left
      else if (movment_corrent_state == 'L'){
        LM_MF();
        RM_ST(); //Stop the motor
        RM_MB();
      }
      //Move Right
      else if (movment_corrent_state == 'R'){
        LM_MB();
        LM_ST(); //Stop the motor
        RM_MF();
      }
      
      if (movment_pre_state == movment_corrent_state || movment_pre_state==' '){
        TBCCR1 = full_speed;
        TBCCR2 = full_speed;
        TBCCR3 = full_speed;  
        TBCCR5 = full_speed;      
      }else{
        //speed_state=0;
        TBCCR1 = half_speed;
        TBCCR2 = half_speed;
        TBCCR3 = half_speed;  
        TBCCR5 = half_speed;      
      }
      movment_pre_state = movment_corrent_state; //Save the last movment state;
      break;
    case 4:break;                             // Vector 4 - TXIFG
    default: break;
  }
  UCA0IE |= UCRXIE;                         // Enable USCI_A1 RX interrupt  
}

////HC-SR04 
#pragma vector=TIMER0_A1_VECTOR   /* 0xFFEA Timer0_A5 CC1-4, TA */
__interrupt void TimerA_0(void)
{
  char string[7]="";
  unsigned int len,i;          
  if (TA0CCTL1 & CCI){	// Raising edge - The signal is out
    timer_counter_HCSR04 = TA0CCR1;
  }else{					// End of signal
    distance_in_cm = (TA0CCR1 - timer_counter_HCSR04)/58;	//Convert Value TO CM
    if (distance_in_cm != distance_past){	//For not sending the same value again
      distance_past = distance_in_cm;
      sprintf(string, "%lu", distance_in_cm);		//Conver integer to string
      len=strlen(string);							//Check the length of the string
      while (!(UCA0IFG&UCTXIFG));
      UCA0TXBUF='<';		//This Character will tell the WebSite that the is new distance value
      for(i=0;i<len;i++){
        while (!(UCA0IFG&UCTXIFG)); // Only If (UCTXIFG)-Finish Sending the character
        UCA0TXBUF=string[i];
      }
      while (!(UCA0IFG&UCTXIFG)); // Only If (UCTXIFG)-Finish Sending the character
      __delay_cycles(100);
      UCA0TXBUF='>';				//Tell the WebSite that all the data has been sended
    }
  }
  TA0CTL &= ~TAIFG;			// Clear interrupt flag - handled
  TA0CCTL1 &=~CCIFG;
}

// ADC12 interrupt service routine
#pragma vector=ADC12_VECTOR
__interrupt void ADC12_ISR (void)
{
  char string[7]="";
  unsigned int len,i;  	
  accelerometer_value = (ADC12MEM0*0.2048-423.86); //Conver the data to Degree
  if (accelerometer_value != accelerometer_past){	//For not sending the same value
    accelerometer_past = accelerometer_value;	//Saveing the last value
    sprintf(string, "%d", accelerometer_value);
    len=strlen(string);
    while (!(UCA0IFG&UCTXIFG));

    UCA0TXBUF='[';					//This Character will tell the WebSite that the is new accelerometer value
    for(i=0;i<len;i++){
      while (!(UCA0IFG&UCTXIFG)); // Only If (UCTXIFG)-Finish Sending the character
      UCA0TXBUF=string[i];           
    }
    while (!(UCA0IFG&UCTXIFG)); // Only If (UCTXIFG)-Finish Sending the character
    __delay_cycles(100);
    UCA0TXBUF=']';				//Tell the WebSite that all the data has been sended
  }
}

#pragma vector=PORT1_VECTOR 
__interrupt void PORT1_ISR(void){
  DTMF_value1 = (P8IN & 0x04);
  DTMF_value2 = (P1IN & 0x38);

  if (DTMF_value1 == 0){ 
    if(DTMF_value2 == 32){ //2
      RM_MF();
      LM_MF();
    }
  }else if(DTMF_value2 == 48){ //6
    RM_MF();
    LM_MB();  
  }else if(DTMF_value2 == 16){ //4
    RM_MB();
    LM_MF();  
  }else if(DTMF_value2 == 8){//8
    RM_MB();
    LM_MB();      
  }else if(DTMF_value1 == 4){ 
    if(DTMF_value2 == 16){ //5
      RM_ST();
      LM_ST();     
    }
  }
  P1IFG &= ~0x80;                            // P1.4 IFG cleared
}
