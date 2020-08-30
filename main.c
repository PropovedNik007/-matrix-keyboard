#include <msp430.h>
#include <stdint.h>
#include <lcd.h>

uint8_t p2_data[6] = {0x01, 0x00, 0x02, 0x00, 0x04, 0x00};
uint8_t char_data[4][3] = {0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C};
uint16_t disp_pos = 1;

int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;
	
	//Конфигурируем Р1.0 - Р1.4 для обработки прерываний
	P1OUT = 0x00;           //Сброс вывода в 0
	P1IN = 0x00;            //Очищаем входной регистр
	P1DIR = 0x00;           //Конфигурируем все порты как входы
	P1IFG = 0x00;           //Сброс регистра флагов прерываний
	P1IES = 0x00;           //Прерывание по переднему фронту
	P1IE |= 0x0F;           //Разрешаем прерывания от Р1.0 - Р1.4

	//Конфигурируем порты Р3.0 - Р3.2 как выходы
	P3DIR = 0x07;
	P3OUT = 0;

	//Инициализация дисплея
	init_lcd();

	//Конфигурируем таймер А для отсчета временных интервалов. По прерыванию от таймера А
	//запускается канал ПДП, который переносит данные из массива p2_data в регистр вывода Р3OUT
	//Таким образом, в порте Р3 циклически выставляются в 1 ножки Р3.0, Р3.1 и Р3.2, что позволяет
	//микроконтроллеру опрашивать 3 столбца клавиатуры

    TACCR0 = 1000;                      //Максимальное значение счета
    TACCR1 = 500;                       //Половинное значение счета
    TACCTL1 |= OUTMOD_2;                //Режим вывода "установка-сброс"
    TACTL |= TASSEL_1 + MC_1;           //Тактовый сигнал - ACLK, счет вверх до TACCR0

    //Конфигурация контроллера ПДП

	DMA0SA = p2_data;           //Источник данных - p2_data
	DMA0DA = &P3OUT;            //Приемник данных - регистр P3OUT
	DMA0SZ = 6;                 //Размер передачи - 6
	DMACTL0 |= DMA0TSEL_7;      //Запускающий сигнал -  TACCR0.IFG

	/*
	 * Повторно-одиночный перенос, инкремент адреса источника, побайтовый режим для источника
	 * и приемника, запуск канала, разрешение прерываний
	 */
	DMA0CTL |= DMADT_4 + DMASRCINCR_3 + DMASRCBYTE + DMADSTBYTE + DMAEN + DMAIE;

    __enable_interrupt();
    __bis_SR_register(LPM0 + GIE);

	return 0;
}

//Обработчик прерывания от порта 1
#pragma vector = PORT1_VECTOR
__interrupt void PORT2_ISR(void) {

    uint8_t ifg_mask, p3;
    uint16_t disp_ch;

    P1IE = 0x00;            //Запрещаем прерывания для предотвращения дребезга
    ifg_mask = P1IFG;       //Копируем регистр флагов прерываний
    P1IFG = 0x00;           //Очищаем регистр флагов прерываний
    p3 = P3OUT;             //Копируем текущее состояние Р3OUT

    //По полученным значениям ifg_mask и р3 находим в массиве char_data соответствующий символ
    switch(ifg_mask)
    {
        case 0x01: ifg_mask = 0x00; break;
        case 0x02: ifg_mask = 0x01; break;
        case 0x04: ifg_mask = 0x02; break;
        case 0x08: ifg_mask = 0x03; break;
    }
    p3 = p3 >> 1;

    disp_ch = char_data[ifg_mask][p3];

    //Отображаем цифру на дисплее
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
