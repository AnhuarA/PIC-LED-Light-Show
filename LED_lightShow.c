#include <p18f4321.h>
#include <usart.h>
#include <stdio.h>
#include <math.h>
#include <usart.h>
#pragma config OSC = INTIO2  
#pragma config WDT=OFF
#pragma config LVP=OFF
#pragma config BOR =OFF
#define SEC_LED PORTEbits.RE0																							// Define LED timer bit 
 						   																								// Function prototypes
void chk_isr(void);       
void do_init(void);   		        
void init_UART(void); 
void Serial_RX_ISR(void); 
void do_print_menu(void); 
void T0ISR(void);
void wait_one_sec();
void wait_half_sec(); 
unsigned int get_full_ADC(void);
void LED_output (char);
void Do_Pattern(void);

char rx_char;																											// Variable to store RCREG 
char rx_flag;																											// Software serial interrrupt flag
char sw_flag;																											// Software hardware interrupt flag
char tmr_flag;																											// Software timer flag
char sequence=1; 																										// Sequence variable, initialized to 1

char count = 0;																											// Variable for counter
char T0H;																												// Char value for TMR0H
char T0L;																												// Char value for TMR0L

	char array2[4] = {0x00,0x00,0xFF,0xFF};																				// Array for sequence 2 led outputs
	char array3[10] = {0x00,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};												// Array for sequence 3 led outputs
	char array4[16] = {0x00,0xFF,0x00,0xFF,0x00,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};				// Array for sequence 4 led outputs
	char array5[4] = {0x0F,0xFF,0xF0,0xFF};																				// Array for sequence 5 led outputs
	char array6[16] = {0xFE,0xFD,0xFB, 0xF7, 0xEF, 0xDF,0xBF, 0x7F, 0xFE,0xFD,0xFB, 0xF7, 0xEF, 0xDF,0xBF,0x7F};		// Array for sequence 6 led outputs
 
void init_UART()         																								// Initialized UART function 
{  
    OpenUSART (USART_TX_INT_OFF & USART_RX_INT_OFF &  
  USART_ASYNCH_MODE & USART_EIGHT_BIT & USART_CONT_RX &  
  USART_BRGH_HIGH, 25);  
    OSCCON = 0x60;  
} 
 
 

#pragma code My_HiPriority_Int=0x08 																					// High Priority Interrupt 
void My_HiPrio_Int() 
{ 
  _asm 																													// Use assembly language and jump to chk_isr
 GOTO chk_isr          
   
  _endasm 
} 
#pragma code 


void Serial_RX_ISR(void) 																								// Serial Interrupt service routine
{
	rx_char = RCREG; 																									// Assign RCREG value to rx_char
	rx_flag = 1; 																										// Set rx_flag to 1
} 
 
void INT0_ISR(void)																										// Hardware Interrupt service routine
{
	INTCONbits.INT0IF = 0; 																								// Set INT0 interrupt flag to 0
	sw_flag = 1;																										// Set sw_flag to 1
}

void T0ISR(void)																										// Timer0 Interrupt service routine
{
	INTCONbits.TMR0IF = 0;																								// Clear hardware timer0 interrupt flag
	tmr_flag=1;	
	TMR0H = T0H;																										// Load value into TMR0H
	TMR0L = T0L;																										// Load value into TMR0L

	SEC_LED = ~SEC_LED;																									// Alternate LED value everytime timer interrupt occurs

}
#pragma interrupt chk_isr 																								//Check ISR routine 
void chk_isr() 
{ 
	if (INTCONbits.TMR0IF==1)																							// If TMR0IF is set, 
		T0ISR();																										// go to T0ISR()
	if (PIR1bits.RCIF == 1) 
		Serial_RX_ISR();      																							// If RCIF is set it goes to Serial_RX_ISR 
	if (INTCONbits.INT0IF==1)																							// When switch is pushed (0) interrput flag occurs/ switch is open (1) no interrupt
		INT0_ISR(); 																									// go to INT0_ISR()
} 

 
void do_init(void) 																										// Intialization function          
{ 
	init_UART();																										// Initialize UART
	ADCON0=0x01;																										// Set ADCON0 register
	ADCON1=0x0E;																										// Set ADCON1 register
	ADCON2=0xA9;																										// Set ADCON2 register
	TRISA=0x01; 																										// Set PORT A as all outputs except bit 0 as input
	TRISB=0x01;         																								// Set PORT B as all outputs except bit 0 as input		
	TRISC=0x80;																											// Set PORT C as all outputs except bit 7 as input  
	TRISD=0x00; 																										// Set PORT D as all outputs      
	TRISE=0x00;																											// Set PORT E as all outputs
	OSCCON=0x60; 																										// Program oscillator to be at 4Mhz
	T0CON=0x06;																											// Initialize timer0 with 128 prescale value  
	TMR0H= 0xFF;																										// Initialeze TMR0H value
	TMR0L=0x6C;																											// Initialeze TMR0L value
	rx_char=RCREG;																										// Assign RCREG to rx_char
	rx_flag=0;																											// Initialize rx_flag to 0
	sw_flag=0;																											// Initialize sw_flag to 0
	
  
	PIR1bits.RCIF = 0;     																								// Clear the RX Receive flag 
	PIE1bits.RCIE = 1;      																							// Enables Rx Receive interrupt 
	INTCONbits.PEIE = 1;     																							// Enables PE interrupt 
	INTCONbits.GIE = 1;      																							// Enables Global interrupt 

	INTCONbits.TMR0IE = 1;																								// sets TMR0IE bit to 1
	INTCONbits.TMR0IF = 0;																								// Clear TMR0IF
		
	INTCONbits.INT0IF=0;																								// Clear INTO Receive flag 
	INTCONbits.INT0IE=1;																								// Enablles INTO recieve interupt 
	INTCON2bits.INTEDG0=0; 																								// Interrupt on rising edge 
 	T0CONbits.TMR0ON=1; 																								// Turn on Timer 0
 
} 

void wait_half_sec()                     																				//Delay function  
{
	int I;                              																				//Declares counter variable
    {
         for ( I=0; I <8500; I++);      																				//Increments counter until 8,500 for a delay of one half second
    }
}
void wait_one_sec()                    	 																				//Delay function
{
    wait_half_sec();																									// Wait half sec
 
    wait_half_sec();																									// Wait another half sec
 
}

unsigned int get_full_ADC(void)																							// Function for ADC conversion
{
	int result;																											// Result variable
    ADCON0bits.GO=1;                            																		// Start Conversion
    while(ADCON0bits.DONE==1);                  																		// wait for conversion to be completed
    result = (ADRESH * 0x100) + ADRESL;         																		// combine result of upper byte and lower byte into result
    return result;                              																		// return the result.
}
	
 
 
void gen_short_beep (void)																								// Short beep function 
{ 
	PR2 = 0x18;    																										// values for short beep 
	CCPR1L = 0x0C; 
	CCP1CON = 0x1C;
	T2CON = 0x07;    																									// Turn T2 on here 
	wait_half_sec();    																								// Wait half-second 
	T2CON = 0x03;             																							// Turn T2 off here 
	wait_half_sec();																									// wait half second
}

void gen_long_beep (void)																								// Long beep function 
{ 
	PR2 = 0xA6;																											// values for long beep 
	CCPR1L = 0x37; 
	CCP1CON = 0x0C; 
	T2CON = 0x05;   			 																						// Turn T2 on here 
	wait_one_sec();   			 																						// Wait one-second 
	T2CON = 0x01;            	 																						// Turn T2 off here 
	wait_half_sec(); 																									// Wait half second
}

void play_series_beep(char pattern_number)																				// Function to generate alternating short and long beeps
{	int i;
	for (i=0;i<pattern_number;i++)																						// Loops for amount of times as sequence value
	{
		if (i%2==1)																										// If an odd value
			gen_long_beep();																							// Long beep
		else 																											// Else if even
			gen_short_beep();																							// Short beep
	}
}


void LED_output (char value)																							// Function to output sequence to proper ports
{
	PORTA = value << 2; 																								//shifts # of bits where the least significant LED starts in PORTA
	PORTB = value >> 2; 																								//shifts # of bits where the least significant LED starts in PORTB
	PORTD = value; 	 																									// no need to shift because all the LEDs are used in this port
}

void Do_Pattern(void)																									// Create sequence function
{
	switch (sequence)																									// Switch cases for each sequence
	{
		case 1: 			   																							// sequence 1
			LED_output(0x00);																							// LEDs always on
			break;
		case 2:				   																							//sequence 2
			LED_output(array2[count]); 																					//output the specific array array for sequence 2
			count++;
			if(count==4) 																								// When end of count, restart count
				count = 0;
			break;
		case 3:
			LED_output(array3[count]); 																					//output the specific array array for sequence 3
			count++;
			if(count==10)
			 count = 0;
			break;
		case 4:
			LED_output(array4[count]); 																					//output the specific array for sequence 4
			count++;
			if(count==16)
				count = 0;
			break;
		case 5:
			LED_output(array5[count]); 																					//output the specific array for sequence 5
			count++;
			if(count==4) count = 0;
			break;
				
		case 6:	
																														// Output the array for sequence 6
			if(count == 32) count =0;
			if (count %2) LED_output(0xFF);
				else {if (count <8) PORTA = array6[count/2]<<2;
					else if (count < 16) PORTB = array6[count/2]>>2;
						else if (count <32) PORTD = array6[count/2];
					}
			count++;
			break;

			
		case 7:
																														// Output the array for sequence 6 but modified for sequence 7
			if(count%2) LED_output (0xFF);
			
			else 
				{
					if(count<7)
						PORTA=array6[count/2]<<1;
					else if (count<16) PORTB= array6[count/2]>>2;
					else if (count<32) PORTD= array6[count/2];
					else if (count< 45) PORTD= array6[((count/2)-(count-30))];
					else if (count< 53) PORTB= array6[((count/2)-(count-30))]>>2;
					else if (count<62) PORTA= array6[((count/2)-(count-30))]<<2;
				}
			count++;
			if (count==61)
				{
					PORTD=0xFF; 
					count=0; 
				}
			
			break;
	}
}


 

void main() 												// Main function
{ 
  int i;													// i Variable
  double vin;												// Variable for Vin
  int n;													// n variable for ADC conversion
  do_init();												// execute initialization funciton
  do_print_menu();											// Print teraterm menu
	
  sequence = 1;												// set sequence to 1
  count = 0;												// start count at 0
	
  
	  while (1)       									 	// Infinite While loop 
	 {  
		n=get_full_ADC()+3;									// get adc value plus 3 for adjustments
		vin = n * 4.88/1000.0;								// Conversion of ADC value to volts
		
		if (vin < 2.0)										// Timer0 values if vin is less than 2
		{
    		T0H=0xFD;
    		T0L=0xF5;
		}
		else if (vin >= 2.0 && vin <= 3.0)					// Timer0 values if vin is greater than 2 and less than 3 
		{
    		T0H=0xF7;
    		T0L=0x5F;			
		}
		else if (vin > 3.0)									// Timer0 values if vin is greater than 3
		{
    		T0H=0xF5;
    		T0L=0x52;			
		}
		

	    if (rx_flag == 1)   								// wait until rx_flag is set to 1 indicating rx char has a value
	 	 { 
	   		printf ("%c\r\n\n", rx_char); 					// Print inputted value
	   		rx_flag = 0; 									// Clear rx_flag
	 	
		   if ((rx_char >= '1') && (rx_char <= '7')) 		// If input is within the sequence bounds
		   	{ 
		   	 	sequence = rx_char - '0'; 					// Set sequence to adjusted rx_char value
				play_series_beep(sequence);					// Play corresponding amount of alternating beeps
				count=0;									// Restart count
			}         
		   
		   else 											// If input is not within sequence bounds
		   { 
				printf ("\n\n*** INVALID ENTRY ***\n\n"); 	// Output error message
		   }  
	
	      do_print_menu(); 									// Print menu again
	  	}   
		


	    if(sw_flag == 1)									// If sw_flag is set
		{
			sw_flag = 0;									// Clear sw_flag
			sequence++;										// Increment sequence by one
			if(sequence>7)									// If sequence value is over 7
				sequence = 1;								// Reset sequence to 1
			play_series_beep(sequence);						// Play corresponding amount of alternating beeps				
			count = 0;										// Restart count	
		}


		if(tmr_flag==1)										// If tmr_flag is set
		{
			tmr_flag=0;										// Clear tmr_flag
	 		Do_Pattern();									// Play corresponding led pattern
	 	}  	
	}   
}
 
void do_print_menu(void)									// Teraterm menu function 
{ 															// Prints out menu and sequence to teraterm
  printf ("\r\n\n     Menu\r\n\n"); 
	printf ("Sequence= %d\r\n\n",sequence); 
  printf ("Choose pattern from 1 to 7\r\n"); 
  printf ("\r\nEnter choice : "); 
}

