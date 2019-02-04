#include "msp.h"

volatile uint8_t state = 0x00;
//volatile uint8_t rgb_state = 0x00;

void configSwitches(void)
{
    P1DIR &= ~0x12;
    P1REN |= 0x12;
    P1OUT |= 0x12;
}

void configSwitchInterrupts(void)
{
    P1IES |= 0x12;
    P1IFG &= ~0x12;
    P1IE |= 0x12;
}

void configNVIC(void)
{
   NVIC_SetPriority(PORT1_IRQn, 2);
   NVIC_ClearPendingIRQ(PORT1_IRQn);
   NVIC_EnableIRQ(PORT1_IRQn);
}

void configGlobalInterrupts(void)
{
    __ASM("CPSIE I");  
}

void configLED(void)
{
    P1DIR |= 0x01;
    P2DIR |= 0x07;

    P1DS &= ~0x01;
    P2DS &= ~0x07;

    P1OUT &= ~0x01;
    P2OUT &= ~0x07;
}

void configTimer(void)
{
	TA0CTL |= TACLR; // Clear Timer A
	TA0CTL &= ~TAIFG; // Clear pending Timer A interrupt flag
	TA0CTL =  MC_1 + ID_0 + TASSEL_1 + TAIE;
//	TA0CTL |= MC_1; // Up Mode
//	TA0CTL |= ID_0; // Divided by 1
//	TA0CTL |= TASSEL_1; // Auxillery Clock
//	TA0CTL |= TAIE; // Timer A Interrupt Enable
	
	TA0CCTL0 &= ~CCIFG;  // Clear pending Capture/Compare interrupt flag
	TA0CCTL0 |= CCIE; // Enable Capture/Compare enable

	TA0CCR0 |= 0x0f; // Upper limit
}

int main(void)
{
    WDT_A->CTL = WDT_A_CTL_PW | WDT_A_CTL_HOLD; // Disable watchdog timer
	
	// Port 1 GPIO
    P1SEL0 &= ~0x13;
    P1SEL1 &= ~0x12;
	
	// Port 2 GPIO
	P2SEL0 &= ~0x07;
	P2SEL1 &= ~0x07;
	
	configSwitches();
	configSwitchInterrupts();
	configNVIC();
	configGlobalInterrupts();
	configLED();
	configTimer();
	
    while(1)
    {
        __ASM("WFI"); // Wait for interrupt, else sleep
    }
}

void PORT1_IRQHandler(void)
{
	static uint8_t pr = 0x01;
	
	if ((P1IFG & 0x02) != 0) 
	{
		P1IFG &= ~0x02;
		state ^= 0x01;
	}
	else if ((P1IFG & 0x10) != 0) 
	{
		P1IFG &= ~0x10;
		pr ^= 0x01;
		if (pr == 1) 
		{
			TA0CTL |= MC_1;
		}
		else 
		{
			TA0CTL |= MC_0;
		}
		
	}
    NVIC_ClearPendingIRQ(PORT1_IRQn);
}

void TA0_N_IRQHandler(void)
{
	static uint8_t rgb_state = 0x0;
	if (TAIFG == 1)
	{
		TA0CTL &= ~TAIFG;
		if (state == 0x00)
		{
			P1OUT ^= 0x01;
		}
		else
		{
			if (rgb_state > 0x07) // state rollover when rgb value is greater than 7
			{
				rgb_state = 0x00; // reset to 0
			}
			P2OUT &= ~0x07;
			P2OUT |= rgb_state; // setting rgb value (between 0 and 7)
			rgb_state++; // change colours
		}
	}	
	TA0CTL &= ~TAIFG; // Clear pending Timer A interrupt flag
}
