#include <desktop.h>

unsigned char shift = 0;

void put_key(char c)
{
    key_press(c);
}

void handle_keyboard(unsigned char scancode)
{
    switch(scancode) {
        case 0x02: if (shift) put_key('!'); else put_key('1'); break;
        case 0x03: if (shift) put_key('@'); else put_key('2'); break;
        case 0x04: if (shift) put_key('#'); else put_key('3'); break;
        case 0x05: if (shift) put_key('$'); else put_key('4'); break;
        case 0x06: if (shift) put_key('%'); else put_key('5'); break;
        case 0x07: if (shift) put_key('^'); else put_key('6'); break;
        case 0x08: if (shift) put_key('&'); else put_key('7'); break;
        case 0x09: if (shift) put_key('*'); else put_key('8'); break;
        case 0x0A: if (shift) put_key('('); else put_key('9'); break;
        case 0x0B: if (shift) put_key(')'); else put_key('0'); break;
        case 0x0C: if (shift) put_key('_'); else put_key('-'); break;
        case 0x0D: if (shift) put_key('='); else put_key('+'); break;

        case 0x10: if (shift) put_key('Q'); else put_key('q'); break;
        case 0x11: if (shift) put_key('W'); else put_key('w'); break;
        case 0x12: if (shift) put_key('E'); else put_key('e'); break;
        case 0x13: if (shift) put_key('R'); else put_key('r'); break;
        case 0x14: if (shift) put_key('T'); else put_key('t'); break;
        case 0x15: if (shift) put_key('Y'); else put_key('y'); break;
        case 0x16: if (shift) put_key('U'); else put_key('u'); break;
        case 0x17: if (shift) put_key('I'); else put_key('i'); break;
        case 0x18: if (shift) put_key('O'); else put_key('o'); break;
        case 0x19: if (shift) put_key('P'); else put_key('p'); break;
        case 0x1A: if (shift) put_key('{'); else put_key('['); break;
        case 0x1B: if (shift) put_key('}'); else put_key(']'); break;
        case 0x2B: if (shift) put_key('|'); else put_key('\\'); break;

        case 0x1E: if (shift) put_key('A'); else put_key('a'); break;
        case 0x1F: if (shift) put_key('S'); else put_key('s'); break;
        case 0x20: if (shift) put_key('D'); else put_key('d'); break;
        case 0x21: if (shift) put_key('F'); else put_key('f'); break;
        case 0x22: if (shift) put_key('G'); else put_key('g'); break;
        case 0x23: if (shift) put_key('H'); else put_key('h'); break;
        case 0x24: if (shift) put_key('J'); else put_key('j'); break;
        case 0x25: if (shift) put_key('K'); else put_key('k'); break;
        case 0x26: if (shift) put_key('L'); else put_key('l'); break;
        case 0x27: if (shift) put_key(':'); else put_key(';'); break;
        case 0x28: if (shift) put_key('\"'); else put_key('\''); break;

        case 0x2C: if (shift) put_key('Z'); else put_key('z'); break;
        case 0x2D: if (shift) put_key('X'); else put_key('x'); break;
        case 0x2E: if (shift) put_key('C'); else put_key('c'); break;
        case 0x2F: if (shift) put_key('V'); else put_key('v'); break;
        case 0x30: if (shift) put_key('B'); else put_key('b'); break;
        case 0x31: if (shift) put_key('N'); else put_key('n'); break;
        case 0x32: if (shift) put_key('M'); else put_key('m'); break;
        case 0x33: if (shift) put_key('<'); else put_key(','); break;
        case 0x34: if (shift) put_key('>'); else put_key('.'); break;
        case 0x35: if (shift) put_key('?'); else put_key('/'); break;

        case 0x1C: put_key('\n'); break;
        case 0x0E: put_key('\b'); break;
        case 0x0F: put_key('\t'); break;
        case 0x39: put_key(' '); break;
        case 0x2A: case 0x36: shift = 1; break;
        case 0xAA: case 0xB6: shift = 0; break;

        case 0x48: put_key(' '); break; //TODO: valahogy kezelni a nyilakat es elkuldeni az appoknak stb
        case 0x4B: put_key(' '); break;
        case 0x4D: put_key(' '); break;
        case 0x50: put_key(' '); break;
    }
}
