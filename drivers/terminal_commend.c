#include "const.h"
#include "terminal_commend.h"

unsigned char cmd_buffer[SCREEN_COLS];
int cmd_index = 0;
void set_color(int color);
void lower(char* c);

char* commends[] = {"clean", "color", "help"};
int commends_size;
// מכין את הטרמינל לקבלת פקודות
void terminal_init(){
    clean_commend_buffer();
    commends_size = sizeof(commends) / sizeof(commends[0]);
}

void clean_commend_buffer() {
    for(int i = 0; i < SCREEN_COLS; i++) {
        cmd_buffer[i] = null;
    }
    cmd_index = 0;
}

void handle_commend() {
    // פירוק הפקודה
    char* parts[10];
    char* c = strip(cmd_buffer);
    int size = split(c , ' ' , parts);
    
    if (size == 0) return; // פקודה ריקה

    // ההדפסה של הפקודה
    printf("cmd got -->> ");
    for(int i = 0; i < size ; i++){
        lower(parts[i]);
        printf("$%s$",parts[i]);
    }
    int exists = 0;
    // בדיקת קיום הפקודה
    for(int i = 0; i < commends_size; i++){
        if(strcmp(parts[0], commends[i]) == 1){
            exists = 1;
            break;
        }
    }

    if(exists == 0){
        int old_color = cur_color;
        set_color(RED_ON_BLACK);
        printf("\nUnknown command: %s", parts[0]);
        set_color(old_color);
        clean_commend_buffer();
        return;
    }

    // הניתוח של הפקודה
    if(strcmp(parts[0], "clean") == 1){
        clear_screen();
    }

    else if(strcmp(parts[0] , "color")==1 ){
        change_color(parts, size);
    }
    else if(strcmp(parts[0] , "help") == 1){
        help_commend();
    }
    // איפוס הפקודה לאחר הלוגיקה שלה
    clean_commend_buffer();
}


void print_to_commend_buffer(char c) {
    if (c == '\n') {
        if (c == '\n') {
            cmd_buffer[cmd_index] = '\0';
            return; 
        }
    } 

    else if( c == '\b'){
        cmd_buffer[cmd_index] = null;
        cmd_index--;
        if(cmd_index < 0){
            cmd_index = 0;
        }
    }
    else{
        if (cmd_index < SCREEN_COLS - 1) {
            cmd_buffer[cmd_index] = c;
            cmd_index++;
        }
    }
}

void change_color(char* parts[], int size){
    if(size < 2){
        printf("\nUsage: color <color_name>");
    }
    else{
        if(strcmp(parts[1], "red") == 1){
            set_color(RED_ON_BLACK);
        }
        else if(strcmp(parts[1], "green") == 1){
            set_color(GREEN_ON_BLACK);
        }
        else if(strcmp(parts[1], "yellow") == 1){
            set_color(YELLOW_ON_BLACK);
        }
        else if(strcmp(parts[1], "white") == 1){
            set_color(WHITE_ON_BLACK);
        }
        else if(strcmp(parts[1], "blue") == 1){
            set_color(BLUE_ON_BLACK);
        }
        else{
            printf("\nUnknown color: %s", parts[1]);
        }
    }
}
void help_commend(){
    printf("\nAvailable commands:");
    for(int i = 0; i < commends_size; i++){
        printf("\n- %s", commends[i]);
    }
}