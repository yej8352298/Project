#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>

#define LCD_Dir  DDRA
#define LCD_Port PORTA
#define RS PA0
#define RW PA1
#define EN PA2

void LCD_Command(unsigned char cmnd) {
	LCD_Port = (LCD_Port & 0x0F) | (cmnd & 0xF0);
	LCD_Port &= ~(1 << RS);
	LCD_Port &= ~(1 << RW);
	LCD_Port |= (1 << EN);
	_delay_us(1);
	LCD_Port &= ~(1 << EN);
	_delay_us(200);
	LCD_Port = (LCD_Port & 0x0F) | (cmnd << 4);
	LCD_Port |= (1 << EN);
	_delay_us(1);
	LCD_Port &= ~(1 << EN);
	_delay_ms(2);
}

void LCD_Char(unsigned char data) {
	LCD_Port = (LCD_Port & 0x0F) | (data & 0xF0);
	LCD_Port |= (1 << RS);
	LCD_Port &= ~(1 << RW);
	LCD_Port |= (1 << EN);
	_delay_us(1);
	LCD_Port &= ~(1 << EN);
	_delay_us(200);
	LCD_Port = (LCD_Port & 0x0F) | (data << 4);
	LCD_Port |= (1 << EN);
	_delay_us(1);
	LCD_Port &= ~(1 << EN);
	_delay_ms(2);
}

void LCD_Init() {
	LCD_Dir = 0xFF;
	_delay_ms(20);
	LCD_Command(0x02);
	LCD_Command(0x28);
	LCD_Command(0x0C);
	LCD_Command(0x06);
	LCD_Command(0x01);
	_delay_ms(2);
}

void LCD_String(char *str) {
	while (*str) LCD_Char(*str++);
}

void LCD_Clear() {
	LCD_Command(0x01);
	_delay_ms(2);
}

void LCD_SetCursor(unsigned char row, unsigned char pos) {
	if(row == 1)
	LCD_Command(0x80 + pos);
	else if(row == 2)
	LCD_Command(0xC0 + pos);
}

char keypad[4][4] = {
	{'7','8','9','/'},
	{'4','5','6','*'},
	{'1','2','3','-'},
	{'C','0','=','+'}
};

char get_key() {
	DDRC = 0xF0;
	PORTC = 0xFF;
	while (1) {
		for (int c = 0; c < 4; c++) {
			PORTC = ~(1 << (c + 4));
			_delay_ms(5);
			for (int r = 0; r < 4; r++) {
				if (!(PINC & (1 << r))) {
					while (!(PINC & (1 << r)));
					_delay_ms(20);
					return keypad[r][c];
				}
			}
		}
	}
}

int main() {
	char key, str[16];
	long num1 = 0, num2 = 0, result = 0;
	char op = 0;
	uint8_t step = 0;
	
	LCD_Init();
	LCD_Clear();

	while (1) {
		key = get_key();
		if (key >= '0' && key <= '9') {
			LCD_Char(key);
			if (step == 0)
			num1 = num1 * 10 + (key - '0');
			else
			num2 = num2 * 10 + (key - '0');
			} else if (key == '+' || key == '-' || key == '*' || key == '/') {
			LCD_Char(key);
			op = key;
			step = 1;
			} else if (key == '=') {
			LCD_Char(key);
			if (op == '+') result = num1 + num2;
			else if (op == '-') result = num1 - num2;
			else if (op == '*') result = num1 * num2;
			else if (op == '/' && num2 != 0) result = num1 / num2;
			itoa(result, str, 10);
			LCD_SetCursor(2, 0);
			LCD_String(str);
			num1 = result;
			num2 = 0;
			step = 1;
			} else if (key == 'C') {
			num1 = 0;
			num2 = 0;
			result = 0;
			step = 0;
			op = 0;
			LCD_Clear();
		}
	}
}
