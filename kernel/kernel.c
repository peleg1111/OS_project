// kernel.c - הליבה של מערכת ההפעלה
#include "const.h"




// הפונקציה הראשונה בקובץ - חייבת להיות main!
void main() {
    // ניקוי המסך מכל מה שה-BIOS השאיר
    clear_screen();

    // הדפסת הודעה ראשונה דרך הדרייבר שבנינו
    printf("Driver System: Loaded Successfully.\n");
    printf("Memory Layout: 32-bit Protected Mode Active.\n\n\n");
    printf("Kernel Status: Stable.\n\n");
    printf("============== %s  ==========\n\n","1234");
    printf("=============  %d  ==============\n\n", -1234);
    printf("=============  %c  ==============\n\n", '1');
    char* c = "12345";
    printf("================= %p ==============\n\n",c);
    for (int i = 1; i<30;i++){
        printf("----------%d--------------\n",i);
    }

    
    while(true); // עצירה אינסופית
}
