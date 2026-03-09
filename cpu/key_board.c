#include "const.h"
#include "terminal_commend.h"


unsigned char port_byte_in(unsigned short port);
void port_byte_out(unsigned short port, unsigned char data);

// לתווים בעת לחיצה על המקלדת hex תרגום מ
// 0 = מקש שאיןן לו טיפול
unsigned char keyboard_map[] = {
    0,  27, '1', '2', '3', '4', '5', '6', '7', '8',	/* 9 */
    '9', '0', '-', '=', '\b',	/* Backspace */
    '\t',			/* Tab */
    'q', 'w', 'e', 'r',	/* 19 */
    't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',	/* Enter key */
    0,			/* 29   - Control */
    'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',	/* 39 */
    '\'', '`',   0,		/* Left shift */
    '\\', 'z', 'x', 'c', 'v', 'b', 'n',			/* 49 */
    'm', ',', '.', '/',   0,				/* Right shift */
    '*',
    0,	/* Alt */
    ' '	/* Space bar */
};

void pic_remap(){
    port_byte_out(0x20, 0x11);
    port_byte_out(0x21, 0x20);
    port_byte_out(0x21,0x04);
    port_byte_out(0x21,0x01);

    port_byte_out(0xA0, 0x11);
    port_byte_out(0xA1, 0x28);  
    port_byte_out(0xA1,0x02);
    port_byte_out(0xA1,0x01);

    // 0xFD = 11111101 (רק ביט 1, המקלדת, הוא 0 - כלומר פתוח)
    port_byte_out(0x21, 0xFD); 
    port_byte_out(0xA1, 0xFF); // חוסם את כל השאר ב-Slav
    
    clean_commend_buffer();//מכין את המקום לכתיבת הפקודות
}

void isr_handler() {
    unsigned char scan_code = port_byte_in(0x60);
    if (scan_code < 0x80) { //  (ללא העזיבה של המקש)רק לחיצות 
        char letter = keyboard_map[scan_code];
        
        if (letter != 0) {

            print_char(letter);
            print_to_commend_buffer(letter);

            if(letter == '\n'){
                handle_commend();        // מעבד את הפקודה
                header_msg();            // מדפיס שורת פקודה חדשה
            }
        }
    }
    port_byte_out(0x20, 0x20); // EOI --> משחרר את ההאזנה על המקלדת כדי להמשיך לקלוט מקשים
    
}
