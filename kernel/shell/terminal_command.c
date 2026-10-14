#include "stdio.h"
#include "terminal_command.h"
#include "key_board.h"
#include "memory_test.h"
#include "exception_test.h"

void update_screen();
void set_color(int color);
void lower(char* c);
void printf(char* msg, ...);

Terminal_command terminal_command = {
    .cmd_buffer = {0},
    .size_of_history = 0,
    .commands_history = {{0}},
    .display_history_index = 0,
    .cmd_index = 0,
    .handle_command = null,
    .clean_command_buffer = null,
    .change_color = null,
    .clean_command_history = null,
    .insert_to_history = null,
    .display_history = null
};



void terminal_init(){

    terminal_command.handle_command = handle_command;
    terminal_command.clean_command_buffer = clean_command_buffer;
    terminal_command.change_color = change_color;
    terminal_command.clean_command_history = clean_command_history;
    terminal_command.insert_to_history = insert_to_history;
    terminal_command.display_history = display_history;

}

void clean_command_buffer() {
    for(int i = 0; i < COMMAND_BUFFER_SIZE; i++) {
        terminal_command.cmd_buffer[i] = null;
    }
    terminal_command.cmd_index = 0;
}


void handle_command() {
    insert_to_history(terminal_command.cmd_buffer);
    terminal_command.display_history_index = terminal_command.size_of_history;

    // פירוק הפקודה
    char* parts[10];
    char* c = strip(terminal_command.cmd_buffer);
    int size = split(c , ' ' , parts);

    if (size == 0) return; // פקודה ריקה

    // הניתוח של הפקודה
    if(strcmp(parts[0], "clear") == 1){
        clear_screen();
    }

    else if(strcmp(parts[0] , "color")==1 ){
        change_color(parts, size);
    }


    else if (strcmp(parts[0] , "history") == 1){
        display_history();
    }

    else if(strcmp(parts[0] , "help") == 1){

        printf(
            "\n\n========================= TERMINAL HELP ========================="
            "\n[ USAGE ]"
            "\n  Type a command name followed by its arguments and press ENTER."
            "\n"
            "\n[ AVAILABLE COMMANDS ]"
            "\n  add <num1> <num2>         - return => num1 + num2"
            "\n  sub <num1> <num2>         - return => num1 - num2"
            "\n  mod <num1> <num2>         - return => num1 % num2"
            "\n  div <num1> <num2>         - return => num1 / num2"
            "\n  mult <num1> <num2>        - return => num1 * num2"
            "\n  test_memory               - test the memory manager"
            "\n  memory_map                - print ranges of free and occupied memory address"
            "\n  trigger_invalid_opcode    - trigger an error"
            "\n  trigger_divide_by_zero    - trigger an error"
            "\n  trigger_general_protection_fault - trigger an error"
            "\n  trigger_page_fault        - trigger an error"
            "\n  clear                     - Clears the terminal screen."
            "\n  color <txt color_name> <bg color_name (optional)>    - Changes text color."
            "\n  history                   - Shows the list of previously used commands."
            "\n  help                      - Displays this help menu."
            "\n  kill_thread <thread_id>   - Terminates a specific thread by its ID."
            "\n  kill_process <process_id> - Terminates a specific process by its ID."
            "\n  show_thread <thread_id>   - Displays information about a specific thread."
            "\n  show_process <process_id> - Displays information about a specific process."
            "\n  show_all_threads          - Displays information about all threads."
            "\n  show_all_processes        - Displays information about all processes."
            "\n"
            "\n[ KEYBOARD SHORTCUTS ]"
            "\n  UP / DOWN             - Navigate command history."
            "\n  LEFT / RIGHT          - Move cursor back and forth."
            "\n  Shift + UP / DOWN     - Scroll up / down through buffer."
            "\n  Shift + LEFT / RIGHT  - Select / highlight text."
            "\n  Ctrl + A              - Select all text on current row." 
            "\n  Ctrl + C              - Copy selected text."
            "\n  Ctrl + V              - Paste copied text."
            "\n================================================================="
            "\n");
    }
    else if(strcmp(parts[0] , "kill_thread") && size > 1){
        char* str = parts[1];
        int thread_id = intStr_to_int(str + null);
        int result = kill_thread(get_thread_by_id(thread_id));
        printf("\nKill thread %d: %s", thread_id, result ? "Success" : "Failed");
    }
    else if (strcmp(parts[0], "kill_process") && size > 1)
    {
        char *str = parts[1];
        int process_id = intStr_to_int(str + null);
        int result = kill_process(get_process_by_id(process_id));
        printf("\nKill process %d: %s", process_id, result ? "Success" : "Failed");
    }
    
    else if(strcmp(parts[0],"show_thread")){
        if(size != 2){
            printf("\nshow_process gets one parameter\nshow_process <thread id>");
            return;
        }
        char* str = parts[1];
        int thread_id = intStr_to_int(str + null);
        Thread* th = get_thread_by_id(thread_id);
        if(th != null){
            print_thread_info(th);
        }
        else{
            printf("\nThread with ID %d not found.", thread_id);
        }
    }
    else if(strcmp(parts[0],"show_process")){
        if(size != 2){
            printf("\nshow_process gets one parameter\nshow_process <pid>");
            return;
        }
        char* str = parts[1];
        int process_id = intStr_to_int(str + null);
        Process* p = get_process_by_id(process_id);
        if(p != null){
            print_process_info(p);
        }
        else{
            printf("\nProcess with ID %d not found.", process_id);
        }
    }
    else if(strcmp(parts[0],"show_all_threads") == 1){
        print_all_threads_info();
    }
    else if(strcmp(parts[0],"show_all_processes") == 1){
        print_all_processes_info();
    }
    else if(strcmp(parts[0],"memory_map")){
        printf("\n");
        print_memory_map();
    }

    else if(strcmp(parts[0],"test_memory")){
        run_memory_tests();
    }
    else if(strcmp(parts[0],"trigger_page_fault")){
        trigger_page_fault();
    }

    else if(strcmp(parts[0],"trigger_general_protection_fault")){
        trigger_general_protection_fault();
    }
    else if(strcmp(parts[0],"trigger_invalid_opcode")){
        trigger_invalid_opcode();
    }
    else if(strcmp(parts[0],"trigger_divide_by_zero")){
        trigger_divide_by_zero();
    }

    else if(strcmp(parts[0],"mult")){
        if(size != 3){
            printf("\nerror => mult gets 2 parameters\nmult <num1> <num2>");
            return;
        }
        printf("result = %d",intStr_to_int(parts[1]) * intStr_to_int(parts[2]));
    }
    else if(strcmp(parts[0],"div")){
        if(size != 3){
            printf("\nerror => div gets 2 parameters\ndiv <num1> <num2>");
            return;
        }
        printf("result = %5f",(float)intStr_to_int(parts[1]) / intStr_to_int(parts[2]));
    }

    else if(strcmp(parts[0],"mod")){
        if(size != 3){
            printf("\nerror => mod gets 2 parameters\nmod <num1> <num2>");
            return;
        }
        printf("result = %d",intStr_to_int(parts[1]) % intStr_to_int(parts[2]));
    }

    else if(strcmp(parts[0],"sub")){
        if(size != 3){
            printf("\nerror => sub gets 2 parameters\nsub <num1> <num2>");
            return;
        }
        printf("result = %d",intStr_to_int(parts[1]) - intStr_to_int(parts[2]));
    }

    else if(strcmp(parts[0],"add")){
        if(size != 3){
            printf("\nerror => add gets 2 parameters\nadd <num1> <num2>");
            return;
        }
        printf("result = %d",intStr_to_int(parts[1]) + intStr_to_int(parts[2]));
    }
    
    else{
        int old_color = print_info.cur_color;
        set_color(RED_ON_BLACK);
        printf("\nUnknown command: %s", parts[0]);
        set_color(old_color);
    }
    // איפוס הפקודה לאחר הלוגיקה שלה
    clean_command_buffer();
}


void print_to_command_buffer(char c) {

    if (terminal_command.cmd_index >= COMMAND_BUFFER_SIZE - 1){
        clean_command_buffer();
        clean_row_of_terminal();
        int old_color = print_info.cur_color;
        set_color(RED_ON_BLACK);
        printf("\nerror --> too long command\n");
        set_color(old_color);
        header_msg();
        print_info.buffer_row_offset = 0;
        update_cursor();
        update_screen();
        return;
    }

    if (c == '\n') {
        terminal_command.cmd_buffer[terminal_command.cmd_index] = null;
        terminal_command.display_history_index = terminal_command.size_of_history;
        return; 
    }

    else if(c == '\t'){
        print_to_command_buffer(' ');
        print_to_command_buffer(' ');
        print_to_command_buffer(' ');
        print_to_command_buffer(' ');
    }
    else if( c == '\b'){
        terminal_command.cmd_buffer[terminal_command.cmd_index] = null;
        terminal_command.cmd_index--;
        if(terminal_command.cmd_index < 0){
            terminal_command.cmd_index = 0;
        }
    }
    else{
        if (terminal_command.cmd_index < COMMAND_BUFFER_SIZE - 1) {
            terminal_command.cmd_buffer[terminal_command.cmd_index] = c;
            terminal_command.cmd_index++;
        }
    }
}

int get_color_by_string(char* str){

    if (str == null)
        return -1;

    if (strcmp(str, "black") == 1)
        return BLACK_ON_BLACK;
    if (strcmp(str, "blue") == 1)
        return BLUE_ON_BLACK;
    if (strcmp(str, "green") == 1)
        return GREEN_ON_BLACK;
    if (strcmp(str, "cyan") == 1)
        return CYAN_ON_BLACK;
    if (strcmp(str, "red") == 1)
        return RED_ON_BLACK;
    if (strcmp(str, "magenta") == 1)
        return MAGENTA_ON_BLACK;
    if (strcmp(str, "brown") == 1)
        return BROWN_ON_BLACK;
    if (strcmp(str, "light_gray") == 1)
        return LIGHT_GRAY_ON_BLACK;
    if (strcmp(str, "dark_gray") == 1)
        return DARK_GRAY_ON_BLACK;
    if (strcmp(str, "light_blue") == 1)
        return LIGHT_BLUE_ON_BLACK;
    if (strcmp(str, "light_green") == 1)
        return LIGHT_GREEN_ON_BLACK;
    if (strcmp(str, "light_cyan") == 1)
        return LIGHT_CYAN_ON_BLACK;
    if (strcmp(str, "light_red") == 1)
        return ORANGE_ON_BLACK;
    if (strcmp(str, "light_magenta") == 1)
        return LIGHT_MAGENTA_ON_BLACK;
    if (strcmp(str, "yellow") == 1)
        return YELLOW_ON_BLACK;
    if (strcmp(str, "white") == 1)
        return WHITE_ON_BLACK;

    return -1; // צבע לא נמצא
}

void change_color(char* parts[], int size){

    if(size < 3){
        if(strcmp(parts[1], "default") == 1){
            set_color(DEFAULT_COLOR);
            return;
        }
        printf("\nUsage: color <txt color_name> <bg color_name>");
        return;
    }
    int color = get_color_by_string(parts[1]);
    if(color == -1){

        int old_color = print_info.cur_color;
        set_color(RED_ON_BLACK);
        printf("\nUnknown color: %s", parts[1]);
        set_color(old_color);
        return;
    }

    int bg = get_color_by_string(parts[2]);
    if(bg == -1){
        int old_color = print_info.cur_color;
        set_color(RED_ON_BLACK);
        printf("\nUnknown bg color: %s", parts[2]);
        set_color(old_color);
        return;
    }
    color += (bg << 4);
    set_color(color);
    
}

void clean_command_history(){

    for(int i = 0; i < terminal_command.size_of_history; i++){
        terminal_command.commands_history[i][0] = null;
    }
    terminal_command.size_of_history = 0;
}

void insert_to_history(char* cmd_buffer){
    if(cmd_buffer[0] == null) return; // פקודה ריקה לא נכנסת להיסטוריה
    if (terminal_command.size_of_history > 0 && strcmp(cmd_buffer, terminal_command.commands_history[terminal_command.size_of_history - 1]) == 1)
        return; // פקודה זהה לפקודה האחרונה לא נכנסת להיסטוריה

    if(terminal_command.size_of_history < SCREEN_ROWS * MAX_ROWS && terminal_command.size_of_history >= 0 ){
        
    int i = 0;
    while (cmd_buffer[i] != null && i < SCREEN_ROWS * MAX_ROWS -1) {
        terminal_command.commands_history[terminal_command.size_of_history][i] = cmd_buffer[i];
        i++;
    }

    terminal_command.commands_history[terminal_command.size_of_history][i] = null;
        terminal_command.size_of_history++;
        terminal_command.display_history_index = terminal_command.size_of_history;
    }
}

void display_history(){

    printf("\nCommand History:\n");
    for(int i = 0; i < terminal_command.size_of_history; i++){
        if(terminal_command.commands_history[i][0] != null){
            printf("\n--- %s", terminal_command.commands_history[i]);
        }
    }
}

void get_from_history(int index){
    key_board.index_of_shift = -1;

    if (index != -1 && index > -1 && index < terminal_command.size_of_history)
    {
        clean_row_of_terminal();
        printf("%s", terminal_command.commands_history[index]);
        refresh_cmd_buffer();
    }
    update_cursor();
}

// יוצר אותו מחדש לאחר שינוי של הפקודה חיצונית 
void refresh_cmd_buffer(){
    clean_command_buffer();
    for(int i = get_safe_index(); i < print_info.last_pos/2; i++){
        char c = (char)(print_info.terminal_buffer[i] & 0xFF);
         if (c != null) {
            print_to_command_buffer(c);
         }
    }
}
