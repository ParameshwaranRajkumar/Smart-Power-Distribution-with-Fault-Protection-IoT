// PIC16F887 Configuration Bit Settings

// 'C' source line config statements

// CONFIG1
#pragma config FOSC = INTRC_NOCLKOUT// Oscillator Selection bits (INTOSCIO oscillator: I/O function on RA6/OSC2/CLKOUT pin, I/O function on RA7/OSC1/CLKIN)
#pragma config WDTE = OFF       // Watchdog Timer Enable bit (WDT disabled and can be enabled by SWDTEN bit of the WDTCON register)
#pragma config PWRTE = ON       // Power-up Timer Enable bit (PWRT enabled)
#pragma config MCLRE = ON       // RE3/MCLR pin function select bit (RE3/MCLR pin function is MCLR)
#pragma config CP = OFF         // Code Protection bit (Program memory code protection is disabled)
#pragma config CPD = OFF        // Data Code Protection bit (Data memory code protection is disabled)
#pragma config BOREN = OFF      // Brown Out Reset Selection bits (BOR disabled)
#pragma config IESO = ON        // Internal External Switchover bit (Internal/External Switchover mode is enabled)
#pragma config FCMEN = ON       // Fail-Safe Clock Monitor Enabled bit (Fail-Safe Clock Monitor is enabled)
#pragma config LVP = OFF        // Low Voltage Programming Enable bit (RB3 pin has digital I/O, HV on MCLR must be used for programming)

// CONFIG2
#pragma config BOR4V = BOR40V   // Brown-out Reset Selection bit (Brown-out Reset set to 4.0V)
#pragma config WRT = OFF        // Flash Program Memory Self Write Enable bits (Write protection off)

// #pragma config statements should precede project file includes.
// Use project enums instead of #define for ON and OFF.
#define _XTAL_FREQ 4000000
#include<xc.h>
long int total_1=0,total_2=0,vol=0,cur=0,power=0;
int fault=0,load3_status=0,ov_flage=0;
void enable()
{
    RE1=1;
    __delay_ms(10);
    RE1=0;
    __delay_ms(10);
}
void lcd(char ins, char data)
{
    RE0=ins;
    PORTD=data;
    enable();
}
void tx(unsigned char tx_data)
{
    TXREG=tx_data;
    while(TXIF==0);
    TXIF=0;
}
void seperate (int raw)
{
    tx(((raw/100)%10)+48);//0
    tx(((raw/10)%10)+48);//1
    tx((raw%10)+48);//9
}
void voltage()
{
    ADCON0=0X81;
    GO=1;
    while(GO);
    total_1=(ADRESH<<8)|(ADRESL);
}
void current()
{
    ADCON0=0X85;
    GO=1;
    while(GO);
    total_2=(ADRESH<<8)|(ADRESL);
}
void string(char *ptr)
{
    while(*ptr!='\0')
    {
        lcd(1,*ptr++);
    }
}
void send_current(int raw)
{
    tx((raw/10)+48);//1
    tx('.');//.
    tx((raw%10)+48);//9
}
void protection()
{
    //protection
        if(cur>14)
        {
            RB2=0;
        }
        if(vol>235)
        {
            fault=1;
            RB0=0;RB1=0;RB2=0;
            lcd(0,0xC8);
            string("OV");
        }
        else if(vol<=232)
        {
            fault=0;
        }
        if(vol<=222)
        {
            lcd(0,0xC8);
            string("UV");
            RA5=1;
            __delay_ms(100);
            RA5=0;
            __delay_ms(100);
        }
        if(vol>222 && vol<=232)
        {
            lcd(0,0xC8);
            string(" N");
        }
        
        seperate(vol);
        tx(',');
        send_current(cur);
        tx(',');
        seperate(power);
        tx(',');
        tx(fault+48);
        tx('\r');
        tx('\n');
        
}
void main()
{
    PORTA=PORTB=PORTD=PORTE=0X00;
    TRISA=0X0B;
    TRISB=0XF8;
    TRISD=TRISE=0X00;
    ANSEL=0X03;
    ANSELH=0X00;
    ADCON1=0X90;
    TXSTA=0X26;
    RCSTA=0X90;
    SPBRG=25;
    lcd(0,0x38);
    lcd(0,0x0E);
    lcd(0,0x01);
    lcd(0,0x80);
    while(1)
    {
        voltage();
        current();
        vol=(int)((float)total_1*0.244379276);
        
        
        //lcd
        lcd(0,0x80);
        string("vol");
        lcd(1,'=');
        lcd(1,((vol/100)%10)+48);
        lcd(1,((vol/10)%10)+48);
        lcd(1,(vol%10)+48);
        
        //current
        cur=(int)(((float)total_2*0.001953125)*10);
        
        //lcd
        lcd(0,0x88);
        string("cur");
        lcd(1,'=');
        lcd(1,(cur/10)+48);
        lcd(1,46);
        lcd(1,(cur%10)+48);
        
        //power
        power=(int)((float)vol*((float)cur/10.0));
        lcd(0,0xC0);
        string("pow");
        lcd(1,'=');
        lcd(1,((power/100)%10)+48);
        lcd(1,((power/10)%10)+48);
        lcd(1,(power%10)+48);
        
        protection();  
        
        

        //load control
        
        if(RB3==0 && RB4==0)
        {
            lcd(0,0xCB);
            string("Mode");
            RB0=0;RB1=0;RB2=0;
        }
        
        //auto mode
        if(RB3==1 && fault==0 && RB4==0)
        {
            lcd(0,0xCB);
            string("A    ");
            
            RB0=1;RB1=1;
            if((cur<=7) && (load3_status==0))
            {
                RB2=1;
                load3_status=1;
            }
            else if((cur>=11) && (load3_status==1))
            {
                RB2=0;
                load3_status=0;
            }
        }
        
        //manual mode
        if(RB4==1 && fault==0 && RB3==0)
        {
            lcd(0,0xCB);
            string("M    ");

            if((cur<=7) && (load3_status==0) && (RB7==1))
            {
                RB2=1;
                load3_status=1;
            }
            else if((cur>=11) && (load3_status==1))
            {
                RB2=0;
                load3_status=0;
            }
            else if(RB7==0)
            {
                RB2=0;
                load3_status=0;
            }
            if(RB5==1)
            {
                RB0=1;
            }
            else
            {
                RB0=0;
            }
            if(RB6==1)
            {
                RB1=1;
            }
            else
            {
                RB1=0;
            }
            
        }
        
        
    }
}