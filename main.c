#include <msp.h>
#include <stdint.h>

int ChosenLed = 0;
void ConfigInputs(){
	//setting input as Gpio
	P1SEL0 &= ~(0x12);
	P1SEL1 &= ~(Ox12);

	//Setting pins as input	
	P1DIR &= ~(Ox12);
	
	//Enabling pull up resistors
	P1REN |= 0x12;
	P1OUT |= Ox12;

	//turning on interupt at pin level
	P1IFG &= 0x12;
	P1IES |= 0x12;
	P1IE |= 0x12;
}

void ConfigNVIC(){
	//Setting NVIC Priority
	NVIC_SetPriority(PORT1_IRQn,2);
	//clearing pending requests
	NVIC_ClearPendingIRQ(PORT1_IRQn);
	//Enabling Interrupts
	NVIC_EnableIRQ(PORT1_IRQn);
    //Setting NVIC Priority for timer
    NVIC_SetPriority(TA0_N_IRQn,2);
    //clear pending requests
    NVIC_ClearPendingIRQ(TA0_N_IRQn);
    //Enable Interrutps
    NVIC_EnableIRQ(TA0_N_IRQn);
}

void EnableGlobalInterrupts(){__ASM("CPSIE I");}

int ConfigLED(){
	//Configuring LED's as outputs
		//Port 1
	P1SEL0 &= ~0x01;
	P1SEL1 &= ~0x01;
		//Port 2
	P2SEL0 &= ~0x07;
	P2SEL1 &= ~0x07;

	//Configuring direction
	P1DIR |= 0x01;
	P2DIR |= 0x07;

	//Configuring drive strength
	P1DS &= ~0x01;
	P2DS &= ~0x07;	
	
	//Setting Initial state;
	P1OUT &= ~0x01;
	P2OUT &= ~0x07;
}

void PORT1_IRQHandler(void){
	//Checks to which of the 2 buttons caused the interrrupt, if it it was
	//P1.1, causes the selected led to switch, other resumes/pauses
	if((P1IFG & 0x02) !=0){
		P1IFG &= ~0x02;
		if(ChosenLed ==0){
			ChosenLed = 1;
		}else{
			ChosenLed = 0;
		}
	} else if((P1IFG & 0x10) !=0){
		P1IFG &= ~0x10;
		TA0CTL ^= 0x0002;
	}

}

void TA0_N_IRQHandler(void){
	static int i = 0;
	static int LedStates[8] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07};
	if(ChosenLed == 0){
		P1OUT ^= 0x01;
	}else{
		i = (i+1)%8;
		P2OUT = ((P2OUT & ~(0x07))|LedStates[i]);
	}
}

Void ConfigTimerA(){
	TA0CTL |= 0x0162;
	TA0CTL &= ~ 0x0293;
    //TA0CTL = 0xFFFF;
}



int main(){
	WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD;
	ConfigInputs();
	ConfigTimerA();
	ConfigNVIC();
	EnableGlobalInterrupts();
	while(1){
		__ASM("WFI");
	}
}
