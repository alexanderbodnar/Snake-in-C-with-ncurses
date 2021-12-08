#include <stdio.h>
#include <stdlib.h>
#include <curses.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define SNAKE_CHAR "O"
#define FOOD_CHAR "+"
#define SNAKE_START_LENGHT 3
#define GAME_SPEED 0.1

void main_menu(int *pick_state);
int get_max_score();
void game(int *game_state);
bool is_out_of_field(int last_x, int last_y);
void update_terminal(int height, int width);
void start_terminal(int height, int width);
void highest_score(int max_score);
void create_file();
void options();
//hlavicka pre nanosleep funkciu
int nanosleep(const struct timespec *req, struct timespec *rem);

//tutorial z bomberu
//refresh nastaveny na kazdych 0.1s
struct timespec ts = {
    .tv_sec = 0,
    .tv_nsec = GAME_SPEED * 1000000000L
};

//struct pre hada s x,y
typedef struct {
    int x;
    int y;
}position;


int main(){

    int height = 0;
    int width = 0;
    int game_state = 0;

    //inicializacia okna
    start_terminal(height, width);
    srand(time(NULL));

    //vytvori novy suobor pre max_score v pripade ak subor este neexistuje
    FILE *fp = NULL;
    fp = fopen("highest_score", "r");
    //ak subor neexistuje
    if(fp == NULL){
        create_file();
    }

    while(true){

        int main_menu_pick = 0;
        main_menu(&main_menu_pick);
        int x,y;

        getmaxyx(stdscr, y, x);

        if(main_menu_pick == 0){
            //Main game loop
            game(&game_state);
            //pokial main game loop skonci, vypise ci hrac prehral
            if(game_state == 2){
                mvwprintw(stdscr, y/2, x/2 - 10, "-----GAME OVER!-----");
                attron(COLOR_PAIR(3));
                mvwprintw(stdscr, y/2 + 2, x/2 - 13, "Press ENTER to play again.");
                attroff(COLOR_PAIR(3));
            }
        }
        else if(main_menu_pick == 1){
            //Options
            options();
        }
        else{
            //Exit
            break;
        }

        update_terminal(height, width);
    }

    //vypne okno
    endwin();

    return 0;
}
//hlavna funkcia pre game loop
void game(int *game_state){

    //cely snake
    position snake[300] = {};   
    
    int x;
    int y;

    bool is_food_spawned = false;
    bool collided = false;
    bool is_debug_mode = false;
    bool is_debug_window_active = false;
    bool is_game_over = false;
    
    //zikanie inputu od hraca
    keypad(stdscr, true);

    //premaze obrazovku
    erase();

    //random position na hracej ploche
    getmaxyx(stdscr, y, x);

    int last_direction = 0;
    int food_x;
    int food_y;
    int food_count = 0;
    int snake_lenght = SNAKE_START_LENGHT;
    int index_color = 0;
    int random_color_index = 0;

    //vytvor noveho snakea (hlavu) v danej pozicii
    position p;
    p.x = rand() % rand() % ((y+1)-1) + 1;
    p.y = rand() % rand() % ((y+1)-1) + 1;
    snake[0] = p;

    //pokial hrac nevyjde z hracej plochy tak sa hraje
    while(true){

        int last_food_count = food_count;

        bool is_snake_collided = false;

        //border game fieldu
        box(stdscr, 0, 0);
        
        int direction = 0;

        //ak hrac nezada nic, necaka na vstup, skipne ho
        nodelay(stdscr, true);

        //vstup od hraca
        int key = getch();

        //ak hrac nezadal nic nestlacil
        if(key == ERR){
            direction = 0;
        }
        
        //zapinane/vypinanie debugger okna
        //key 330 je DELETE
        if(is_debug_window_active == false){
            if(key == 330){
                is_debug_mode = true;
            }
        }
        else{
            if(key == 330){
                is_debug_mode = false;
                is_debug_window_active = false;
            }
        }

        //directions su podla premennej nastavene, simuluju svetove strany
        if(key == KEY_DOWN){
            direction = 3;
        }
        else if(key == KEY_UP){
            direction = 2;
        }
        else if(key == KEY_LEFT){
            direction = -1;
        }
        else if(key == KEY_RIGHT){
            direction = 1;
        }
        
        int X = snake[0].x;
        int Y = snake[0].y;

        //movement handler input
        if(direction == 3){
            X++;
            last_direction = 3;
        }
        else if(direction == 2){
            X--;
            last_direction = 2;
        }
        else if(direction == -1){
            Y--;
            last_direction = -1;
        }
        else if(direction == 1){
            Y++;
            last_direction = 1;
        }
        else if(direction == 0){
            if(last_direction == 3){
                X++;
            }
            else if(last_direction == 2){
                X--;
            }
            else if(last_direction == -1){
                Y--;
            }
            else if(last_direction == 1){
                Y++;
            }
        }

        //ak hrac vysiel z hracieho pola tak nastavi bool is_game_over = true
        if(is_out_of_field(snake[0].x, snake[0].y) == true){
            is_game_over = true;
            break;
        }

        //zavolanie funkcie pre sleep
        nanosleep(&ts, NULL);
    
        //pomocne premenne pre debug_mode
        char last_x_char[20];
        char last_y_char[20];
        char curent_key[20];
        char last_direction_char[10];
        char food_x_char[10];
        char food_y_char[10];
        char collided_char[10];
        char snake_lenght_char[10];
        char border_x_char[10];
        char border_y_char[10];
        char food_count_char[100];
    
        //premaze obrazovku a ju refreshne            
        refresh();
        erase();
        
        //DEBUG INFORMACIE
        if(is_debug_mode){

            is_debug_window_active = true;
            
            sprintf(border_x_char, "%d", x);
            mvwprintw(stdscr, 1, 2, "Border X: ");
            mvwprintw(stdscr, 1, 13, border_x_char);

            sprintf(border_y_char, "%d", y);
            mvwprintw(stdscr, 2, 2, "Border Y: ");
            mvwprintw(stdscr, 2, 13, border_y_char);

            sprintf(last_direction_char, "%d", last_direction);
            mvwprintw(stdscr, 3, 2, "Last Direction: ");
            mvwprintw(stdscr, 3, 18, last_direction_char);

            sprintf(curent_key, "%d", direction);
            mvwprintw(stdscr, 4, 2, "Current key: ");
            mvwprintw(stdscr, 4, 15, curent_key);

            sprintf(last_x_char, "%d", snake[0].x);
            mvwprintw(stdscr, 5, 2, "Y: ");
            mvwprintw(stdscr, 5, 5, last_x_char);

            sprintf(last_y_char, "%d", snake[0].y);
            mvwprintw(stdscr, 6, 2, "X: ");
            mvwprintw(stdscr, 6, 5, last_y_char);

            sprintf(food_x_char, "%d", food_x);
            mvwprintw(stdscr, 7, 2, "Food X: ");
            mvwprintw(stdscr, 7, 10, food_x_char);

            sprintf(food_y_char, "%d", food_y);
            mvwprintw(stdscr, 8, 2, "Food Y: ");
            mvwprintw(stdscr, 8, 10, food_y_char);

            sprintf(collided_char, "%d", collided);
            mvwprintw(stdscr, 9, 2, "Collided: ");
            mvwprintw(stdscr, 9, 12, collided_char);

            sprintf(snake_lenght_char, "%d", snake_lenght);
            mvwprintw(stdscr, 10, 2, "Snake Lenght:");
            mvwprintw(stdscr, 10, 16, snake_lenght_char);

            sprintf(food_count_char, "%d", food_count);
            mvwprintw(stdscr, 11, 2, "Food Counter:");
            mvwprintw(stdscr, 11, 16, food_count_char);
        }
        
        //vykreslenie hada s farbami
        for(int i = 0; i <= snake_lenght; i++){
            if(index_color >= 5){
                index_color = 0;
            }
            attron(COLOR_PAIR(index_color++));
            mvaddstr(snake[i].x, snake[i].y, SNAKE_CHAR);
            attroff(COLOR_PAIR(index_color));
        }

        snake[snake_lenght - 1].x = X;
        snake[snake_lenght - 1].y = Y;

        position p = snake[snake_lenght - 1];

        //premazavanie posledneho znaku z hada
        for(int i = snake_lenght; i> 0; i--){
            snake[i]=snake[i - 1];
        }

        //nastavenie hlavy hada na danu aktualnu polohu
        snake[0]= p;

        //checkovanie collision hada s jedlom
        if(snake[0].x == food_y && snake[0].y == food_x){
            collided = true;
            is_food_spawned = false;
            food_count++;
        }

        //ak snake narazil na znak, tak spawne druhy znak
        if(!collided){
            if(!is_food_spawned){
                //vygeneruj random polohu a spawni znak s random farbou
                do{
                    food_x = rand() % ((x-1)-1) + 1;
                    food_y = rand() % ((y-2)-1) + 1;
                    random_color_index = rand() % 5; // 6 je pocet farieb inicializovanych
                }
                while(food_x >= x-3 && food_y >= y-3 && food_x < 3 && food_y < 3);

                is_food_spawned = true;
            }
            else{
                //vypisuj znak (jedlo) kazdy looop
                attron(COLOR_PAIR(random_color_index));
                mvwprintw(stdscr, food_y, food_x, FOOD_CHAR);
                attroff(COLOR_PAIR(random_color_index));
            }
        }
        else{
            //vygeneruj random polohu a spawni znak s random farbou
            do{
                food_x = rand() % ((x-1)-1) + 1;
                food_y = rand( )% ((y-2)-2) + 1;
                random_color_index = rand() % 5; // 6 je pocet farieb inicializovanych
            }
            while(food_x >= x-3 && food_y >= y-3 && food_x < 3 && food_y < 3);

            is_food_spawned = true;
            collided = false;

            //pridaj chvost pre hada
            position tail;
            tail.x = X;
            tail.y = Y;
            snake_lenght++;
            snake[snake_lenght]=tail;
        }

        //pokial had narazi do vlastneho tela ukonci hru
        for(int i = 1; i < snake_lenght; i++){
            if(last_direction != 0){
                if(snake[i].x == snake[0].x && snake[i].y == snake[0].y){
                    //bugovalo tak pridany primitivy podmienkovy fix
                    if(last_food_count == food_count){
                        is_snake_collided = true;
                        is_game_over = true;
                        break;
                    }
                }
            }
            
        }

        //ak had do seba narazil vypne hru
        if(is_snake_collided){
            *game_state = 2;
            break;
        }

                
        //vypise aktualne ziskane skore
        mvwprintw(stdscr, y-2, 1, "Current Score: ");
        char current_food_char[100];
        sprintf(current_food_char, "%d", food_count);
        mvwprintw(stdscr, y-2, 16, current_food_char);
    }

    //premaz obrazovku
    erase();
    refresh();

    //ak hrac vysiel z hracej plochy, tak nastav game_state == 2
    if(is_game_over){
        *game_state = 2;
        highest_score(food_count);
    }

}

void main_menu(int *pick_state){
    
    int key;
    int counter = 0;
    int y = 0;
    int x = 0;
    int current_max_score = 0;

    char current_max_score_char[100];

    //z funkcie ulozi max score do premennej zo suboru a premeni ho na char
    current_max_score = get_max_score();
    sprintf(current_max_score_char, "%d", current_max_score);

    //povoluje hracovi zadavat key inputy
    keypad(stdscr, true);

    while(true){

        //caka od uzivatela vstup, nepreskoci dalej
        nodelay(stdscr, false);

        getmaxyx(stdscr, y, x);

        //vstup z klavesnice od uzivatela
        key = wgetch(stdscr);

        //premazanie terminalu
        refresh();
        clear();

        //made by
        mvwprintw(stdscr, y-1, x-25, "Author: Alexander Bodnár");

        //vypise najvacsie dosiahnute max score
        mvwprintw(stdscr, y-1, 1, "Max Score: ");
        mvwprintw(stdscr, y-1, 12, current_max_score_char);

        mvwprintw(stdscr, (y/3) , (x/2 - 10), "-----Start Game-----");
        mvwprintw(stdscr, (y/2.5) , (x/2 - 9), "-----Options-----");
        mvwprintw(stdscr, (y/2) , (x/2 - 10), "-----Exit Game-----");

        //pohybovanie sa v MENU (hore,dole)
        switch(key){
            case KEY_UP:
                counter--;
                if(counter == -1){
                    counter = 0;
                }
                if(counter == 0){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/3) , (x/2 - 10), "-----Start Game-----");
                    attroff(COLOR_PAIR(3));
                }
                else if( counter == 1){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/2.5) , (x/2 - 9), "-----Options-----");
                    attroff(COLOR_PAIR(3));
                }
                else if( counter == 2){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/2) , (x/2 - 10), "-----Exit Game-----");
                    attroff(COLOR_PAIR(3));
                }
                break;
            case KEY_DOWN:
                counter++;
                if(counter == 1){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/2.5) , (x/2 - 9), "-----Options-----");
                    attroff(COLOR_PAIR(3));

                }
                else if(counter >= 2){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/2) , (x/2 - 10), "-----Exit Game-----");
                    attroff(COLOR_PAIR(3));
                    counter = 2;
                }
                break;
            default:
                break;
        }
        //ak uzivatel stlaci ENTER vyhod z cyklu
        if(key == 10){
            if(counter == 2){
                *pick_state = 2;
            }
            else if(counter == 1){
                *pick_state = 1;
            }
            else{
                *pick_state = 0;
            }
            break;
        }
    }
}

void options(){

    FILE *fp = NULL;
    fp = fopen("highest_score", "r");

    int key;
    int counter = 0;
    int y = 0;
    int x = 0;
    int food_pick = 0;
    int snake_pick = 0;
    int game_speed = 0;

    const char *food_chars[5] = {"&", "@", "?", "!", "#"};
    const char *snake_chars[5] = {"I", "|", "H", "M", "A"};

    //povoluje hracovi zadavat key inputy
    keypad(stdscr, true);

    while(true){

        //caka od uzivatela vstup, nepreskoci dalej
        nodelay(stdscr, false);

        getmaxyx(stdscr, y, x);

        //vstup z klavesnice od uzivatela
        key = wgetch(stdscr);

        //premazanie terminalu
        refresh();
        clear();
        
        mvwprintw(stdscr, 0 , 1, "<--BACKSPACE");

        mvwprintw(stdscr, (y/3) , (x/2 - 10), "-----Snake Symbol: ");
        mvwprintw(stdscr, (y/3) , (x/2 + 10), snake_chars[snake_pick]);
        mvwprintw(stdscr, (y/2.5) , (x/2 - 10), "------Food Symbol: ");
        mvwprintw(stdscr, (y/2.5) , (x/2 + 10), food_chars[food_pick]);
        mvwprintw(stdscr, (y/2) , (x/2 - 10), "-------Game Speed: ");
        char game_speed_char[10];
        sprintf(game_speed_char, "%d", game_speed);
        mvwprintw(stdscr, (y/2) , (x/2 + 10), game_speed_char);

        //pohybovanie sa v MENU (hore,dole)
        switch(key){
            case KEY_UP:
                counter--;
                if(counter == -1){
                    counter = 0;
                }
                if(counter == 0){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/3) , (x/2 - 10), "-----Snake Symbol: ");
                    attroff(COLOR_PAIR(3));
                }
                else if( counter == 1){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/2.5) , (x/2 - 10), "------Food Symbol: ");
                    attroff(COLOR_PAIR(3));
                }
                else if( counter == 2){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/2) , (x/2 - 10), "-------Game Speed: ");
                    attroff(COLOR_PAIR(3));
                }
                break;
            case KEY_DOWN:
                counter++;
                if(counter == 1){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/2.5) , (x/2 - 10), "------Food Symbol: ");
                    attroff(COLOR_PAIR(3));

                }
                else if(counter >= 2){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/2) , (x/2 - 10), "-------Game Speed: ");
                    attroff(COLOR_PAIR(3));
                    counter = 2;
                }
                break;
            case KEY_RIGHT:
                if(counter == 1){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/2.5) , (x/2 - 10), "------Food Symbol: ");
                    attroff(COLOR_PAIR(3));
                    food_pick++;
                    if(food_pick > 4){
                        food_pick = 4;
                    }
                }
                else if(counter == 0){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/3) , (x/2 - 10), "-----Snake Symbol: ");
                    attroff(COLOR_PAIR(3));
                    snake_pick++;
                    if(snake_pick > 4){
                        snake_pick = 4;
                    }
                }
                else if(counter == 2){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/2) , (x/2 - 10), "-------Game Speed: ");
                    attroff(COLOR_PAIR(3));
                    game_speed++;
                }
                break;
            case KEY_LEFT:
                if(counter == 1){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/2.5) , (x/2 - 10), "------Food Symbol: ");
                    attroff(COLOR_PAIR(3));
                    food_pick--;
                    if(food_pick < 0){
                        food_pick = 0;
                    }
                }
                else if(counter == 0){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/3) , (x/2 - 10), "-----Snake Symbol: ");
                    attroff(COLOR_PAIR(3));
                    snake_pick--;
                    if(snake_pick < 0){
                        snake_pick = 0;
                    }
                }
                else if(counter == 2){
                    attron(COLOR_PAIR(3));
                    mvwprintw(stdscr, (y/2) , (x/2 - 10), "-------Game Speed: ");
                    attroff(COLOR_PAIR(3));
                    game_speed--;
                }
                break;
            default:
                break;
        }
        //ak uzivatel stlaci BACKSPACE vyhod z cyklu a chod do main_menu
        if(key == KEY_BACKSPACE){

            fp = fopen("highest_score", "w");
            //ak subor neexistuje
            if(fp == NULL){
                perror("Error opening file.\n");
            }
            else{

                int score = 0;

                fscanf(fp, "%d", &score);
                char max_score_char[10];              
                const char *current_food_char = food_chars[food_pick];
                const char *current_snake_char = snake_chars[snake_pick];
                //uloz nastavene hodnoty z options sekcie
                sprintf(max_score_char, "%d", score);
                fputs(max_score_char, fp);
                fputs(current_food_char, fp);
                fputs(current_snake_char, fp);
                fputs(game_speed_char, fp);
                
            }
            fclose(fp);
            break;
        }
    }
}

//updatuje terminal kazdu chvilu pri resizovani terminalu
void update_terminal(int height, int width){

    getmaxyx(stdscr, height, width);
    refresh();

    if(is_term_resized(height, width)){  
        clear();
    } 
}
//zapne terminal s aktualnou pozadovanou velkostou okna a inicializuje farby
void start_terminal(int height, int width){

    initscr();
    start_color();
    //vypne kurzor
    curs_set(FALSE);
    getmaxyx(stdscr, height, width);
    //inicializacia dostupnych farieb
    init_pair(1, COLOR_YELLOW, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_CYAN, COLOR_BLACK);
    init_pair(4, COLOR_RED, COLOR_BLACK);
    init_pair(5, COLOR_BLUE, COLOR_BLACK);
    init_pair(6, COLOR_WHITE, COLOR_BLACK);
    refresh();
} 
//vytvori subor v pripade ake este nebolo vytvorene
void create_file(){
    FILE *fp = NULL;
    //vytvori novy subor a ak sa tam nenachadza ziadne score pridaj 0
    if(fp == NULL){
        fp = fopen("highest_score", "w");
        if (ftell(fp) == 0){
            fputs("0", fp);
            fputs("#", fp);
            fputs("O", fp);
            fputs("5", fp);
            fclose(fp);
        }
    }
}
//uklada najvyssie ziskane hracie skore
void highest_score(int max_score){

    FILE *fp = NULL;

    fp = fopen("highest_score", "r");
    //ak subor neexistuje
    if(fp == NULL){
        perror("Error opening file.\n");
    }
    else{

        int score;
        char max_score_char[100];

        //precita najvacsie dosiahnute skore a ulozi ho do premennej
        fscanf(fp,"%d", &score); 
        fseek(fp, 2, SEEK_SET);

        fclose(fp);
        //ak je max_score ziskane v danej chvili vacsia ako score ulozene v subore, prepise aktualne score
        if(max_score > score){
            fp = fopen("highest_score", "w");
            sprintf(max_score_char, "%d", max_score);
            fputs(max_score_char, fp);
            fclose(fp);
        }
    }
}
//checkuje ci snake nevysiel z hracej plochy
bool is_out_of_field(int last_x, int last_y){

    if (last_x >= LINES || last_x <= 0 || last_y >= COLS || last_y <= 0){
        return true;
    }
    else{
        return false;
    }

}
//vrati najvacsie score zo suboru ktore hrac ziskal
int get_max_score(){

    FILE *fp = NULL;

    int score;

    fp = fopen("highest_score", "r");
    //ak subor neexistuje
    if(fp == NULL){
        perror("Error opening file.\n");
    }
    else{
        
        //precita najvacsie dosiahnute skore a ulozi ho do premennej
        fscanf(fp,"%d", &score); 
        fclose(fp);
    }
    return score;
}