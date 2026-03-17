#include "const.h"
#include "terminal_commend.h"
#include "key_board.h"
unsigned char cmd_buffer[SCREEN_COLS];
int size_of_history = 0;
char commends_history[SCREEN_ROWS * MAX_ROWS][SCREEN_COLS*5]; // היסטוריית פקודות
int display_history_index = 0;

int cmd_index = 0;
void set_color(int color);
void lower(char* c);
void printf(char* msg, ...);
char* commends[] = {"clear", "color", "help", "history"};
int commends_size;



void refresh_cmd_buffer();
void update_screen();


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
    insert_to_history(cmd_buffer);
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

    // הניתוח של הפקודה
    if(strcmp(parts[0], "clear") == 1){
        clear_screen();
    }

    else if(strcmp(parts[0] , "color")==1 ){
        change_color(parts, size);
    }
    else if(strcmp(parts[0] , "help") == 1){
        help_commend();
    }
    else if (strcmp(parts[0] , "history") == 1){
        display_history();
    }
    else{
        int old_color = cur_color;
        set_color(RED_ON_BLACK);
        printf("\nUnknown command: %s", parts[0]);
        set_color(old_color);
    }
    // איפוס הפקודה לאחר הלוגיקה שלה
    clean_commend_buffer();
}


void print_to_commend_buffer(char c) {
    if (c == '\n') {
        cmd_buffer[cmd_index] = '\0';
        display_history_index = size_of_history;
        return; 
    }
    else if(c == '\t'){
        print_to_commend_buffer(' ');
        print_to_commend_buffer(' ');
        print_to_commend_buffer(' ');
        print_to_commend_buffer(' ');
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
        printf("\n--- %s", commends[i]);
    }
}

void clean_commend_history(){
    for(int i = 0; i < size_of_history; i++){
        commends_history[i][0] = null;
    }
    size_of_history = 0;
}

void insert_to_history(char* cmd_buffer){
    if(cmd_buffer[0] == null) return; // פקודה ריקה לא נכנסת להיסטוריה
    if(strcmp(cmd_buffer, commends_history[size_of_history - 1]) == 1) return; // פקודה זהה לפקודה האחרונה לא נכנסת להיסטוריה
    
    if(size_of_history < SCREEN_ROWS * MAX_ROWS){
        int i;
        for( i = 0; i < cmd_buffer[i] != null; i++){
            commends_history[size_of_history][i] = cmd_buffer[i]; 
        }
        commends_history[size_of_history][i] = null;
        size_of_history++;
        display_history_index = size_of_history;
    }
}

void display_history(){

    printf("\nCommand History:\n");
    for(int i = 0; i < size_of_history; i++){
        if(commends_history[i][0] != null){
            printf("\n--- %s", commends_history[i]);
        }
    }
}

void get_from_history(int index){
    index_of_shift = -1;
    clean_commend_buffer();
    clean_row_of_terminal();
    if(index != -1 && index > -1 && index < size_of_history){
        printf("%s",commends_history[index]);
    }
    update_cursor();
    refresh_cmd_buffer();
}

// יוצר אותו מחדש לאחר שינוי של הפקודה חיצוני 
void refresh_cmd_buffer(){
    clean_commend_buffer();
    for(int i = get_safe_index(); i < last_pos/2; i++){
        char c = (char)(terminal_buffer[i] & 0xFF);
         if (c != null) {
            print_to_commend_buffer(c);
         }
    }
}
