#include <msp430.h>
#include <stdint.h>
#include <lcd.h>

uint8_t p2_data[6] = {0x01, 0x00, 0x02, 0x00, 0x04, 0x00};
uint8_t char_data[4][3] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C};
uint16_t disp_pos = 1;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;
	
	//������������� �1.0 - �1.4 ��� ��������� ����������
	P1OUT = 0x00;           //����� ������ � 0
	P1IN = 0x00;            //������� ������� �������
	P1DIR = 0x00;           //������������� ��� ����� ��� �����
	P1IFG = 0x00;           //����� �������� ������ ����������
	P1IES = 0x00;           //���������� �� ��������� ������
	P1IE |= 0x0F;           //��������� ���������� �� �1.0 - �1.4

	//������������� ����� �3.0 - �3.2 ��� ������
	P3DIR = 0x07;
	P3OUT = 0;

	//������������� �������
	init_lcd();

	//������������� ������ � ��� ������� ��������� ����������. �� ���������� �� ������� �
	//����������� ����� ���, ������� ��������� ������ �� ������� p2_data � ������� ������ �3OUT
	//����� �������, � ����� �3 ���������� ������������ � 1 ����� �3.0, �3.1 � �3.2, ��� ���������
	//���������������� ���������� 3 ������� ����������

    TACCR0 = 1000;                      //������������ �������� �����
    TACCR1 = 500;                       //���������� �������� �����
    TACCTL1 |= OUTMOD_2;                //����� ������ "���������-�����"
    TACTL |= TASSEL_1 + MC_1;           //�������� ������ - ACLK, ���� ����� �� TACCR0

    //������������ ����������� ���

	DMA0SA = p2_data;           //�������� ������ - p2_data
	DMA0DA = &P3OUT;            //�������� ������ - ������� P3OUT
	DMA0SZ = 6;                 //������ �������� - 6
	DMACTL0 |= DMA0TSEL_7;      //����������� ������ -  TACCR0.IFG

	/*
	 * ��������-��������� �������, ��������� ������ ���������, ���������� ����� ��� ���������
	 * � ���������, ������ ������, ���������� ����������
	 */
	DMA0CTL |= DMADT_4 + DMASRCINCR_3 + DMASRCBYTE + DMADSTBYTE + DMAEN + DMAIE;

    __enable_interrupt();
    __bis_SR_register(LPM0 + GIE);

	return 0;
}

//���������� ���������� �� ����� 1
#pragma vector = PORT1_VECTOR
__interrupt void PORT2_ISR(void) {

    uint8_t ifg_mask, p3;
    uint16_t disp_ch;

    P1IE = 0x00;            //��������� ���������� ��� �������������� ��������
    ifg_mask = P1IFG;       //�������� ������� ������ ����������
    P1IFG = 0x00;           //������� ������� ������ ����������
    p3 = P3OUT;             //�������� ������� ��������� �3OUT

    //�� ���������� ��������� ifg_mask � �3 ������� � ������� char_data ��������������� ������
    switch(ifg_mask)
    {
        case 0x01: ifg_mask = 0x00; break;
        case 0x02: ifg_mask = 0x01; break;
        case 0x04: ifg_mask = 0x02; break;
        case 0x08: ifg_mask = 0x03; break;
    }
    p3 = p3 >> 1;

    disp_ch = char_data[ifg_mask][p3];

    //���������� ����� �� �������
    LCDdigit(disp_ch, disp_pos);

    if(disp_pos == 4)
        disp_pos = 1;
    else disp_pos++;

    P1IE |= 0x0F;
}

#pragma vector = DMA_VECTOR
__interrupt void DMA_ISR(void) {

    DMA0CTL |= DMAEN;
}
