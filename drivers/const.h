// הכרזות לפעולות
void clear_screen();

void printf(char* format, ...);

void print_char(char c);

void print_int(int n);

void clear_screen();

void print_hex(unsigned int pointer);

void scrall_screen();
// שימוש במאקרואים פנימיים של GCC
typedef __builtin_va_list va_list;
#define va_start(v,l) __builtin_va_start(v,l)
#define va_arg(v,l)   __builtin_va_arg(v,l)
#define va_end(v)     __builtin_va_end(v)

// קבועי מערכת
#define true 1
#define false 0
#define null 0x00

// הגדרות צבעים (VGA Color)
#define VIDEO_ADDRESS 0xB8000 // VGA pointer (לשם כותבים את ההדפסה)
#define RED_ON_BLACK 0x04
#define WHITE_ON_BLACK 0x0F
#define GREEN_ON_BLACK 0x0A
#define YELLOW_ON_BLACK 0x0E

