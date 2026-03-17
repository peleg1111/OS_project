#ifndef MOUSE_H
#define MOUSE_H

// PS/2 פורטים של בקר ה 
#define MOUSE_PORT   0x60
#define MOUSE_STATUS 0x64
#define MOUSE_WRITE  0xD4

// ביטים בסטטוס
#define MOUSE_ABIT   0x02 // Input buffer full (בקר מוכן לקבל)
#define MOUSE_BBIT   0x01 // Output buffer full (נתונים מחכים לקריאה)

#endif