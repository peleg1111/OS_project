#include "const.h"
#include "idt.h"



void main() {
    // ניקוי המסך מכל מה שה-BIOS השאיר
    clear_screen();

    // הדפסת הודעה ראשונה דרך הדרייבר שבנינו
    printf("Driver System: Loaded Successfully.\n");
    printf("Memory Layout: 32-bit Protected Mode Active.\n");
    printf("Kernel Status: Stable.\n");
    printf("============== %s  ==========\n","1234");
    printf("=============  %d  ==============\n", -1234);
    printf("=============  %c  ==============\n", '1');
    printf("=============  %4f  ==============\n", 3.1234);
    char* c = "123145\0";
    replace(c,'1','2');
    printf("==============\t=== %s ==============\n",c);
    printf("\n\n=====================================\n\n");

}