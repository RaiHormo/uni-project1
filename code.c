#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <unistd.h>
#ifdef __linux__
    #include <termios.h>
#elif __WIN32__
    #include <conio.h>
#endif
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

//Constants
#define false 0
#define true 1
#define up 0
#define down 1
#define left 2
#define right 3

//This changes from one OS to the other, "cls" works on windows
#ifdef __linux__
    const char *clears = "clear";
#elif __WIN32__
    const char *clears = "cls";
#endif
const char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

const char obstacle_c = 'X';
const char empty_c = '.';
const char strooper_c = '@';
const char vader_c = 'D';
const char leia_c = 'L';
const char r2d2_c = 'R';
const char obf_c = '#';

//Controls
char up_c = 'w';
char down_c = 's';
char left_c = 'a';
char right_c = 'd';

//Structs
typedef struct {
    int x;
    int y;
} vec2;

//Globals
char **Map, Msg[100], NextMsg[100];
int menu_index = 0, N = 0, M = 0, start_pressed = false, better_controls = true, ingame = false, is_injured = false, got_help = false, game_ended = 0, vader_moved = false;
enum diffs {UNSET, EASY, MEDIUM, HARD, IMPOSSIBLE};
enum diffs difficulty = UNSET;
int stormtrooper_am, *stormtrooper_dir, level = 1;
vec2 leia, *stormtrooper, vader;


//Funcion definitions
void set_control_scheme();
void title_screen();
void print_menu(char *options[], int length);
char get_input();
void set_board_size();
void setup_map();
void set_difficulty();
void render_map();
int maxi(int a, int b);
int to_dir(char c);
int check_collision(vec2 to, char self);
int move_dir(vec2* vector_src, int dir, char self);
char reposition(vec2 from, vec2 to);
void handle_turn(int leias_move);
char* position(vec2 vector);
void print_vector(vec2 vector);
void handle_stormtroopers();
int turn_around(int dir);
void memory_error();
void free_everything();
void write_slowly(char* str);
void leia_check(vec2 vector, char self, char tar);
void help();
void hit(vec2 vector);
void victory();
void handle_vader();
vec2 get_dir_towards(vec2 from, vec2 to);
int rand_range(int from, int to);
#ifdef __linux__
    char getch(void);
#endif

//Code
int main() {
    title_screen();
}

void setup_map() {
    system(clears);
    free_everything();
    is_injured = false;
    got_help = false;
    game_ended = 0;
    ingame = true;
    if (N == 0 || M == 0) {
        system(clears);
        set_board_size();
    }
    int i, j, stormtrooper_am_tmp, obstacle_am;

    Map = (char**) malloc(sizeof(char*) * N);
    if (Map == NULL) memory_error();
    for (i=0; i<N; i++) 
        Map[i] = (char*) malloc(sizeof(char) * M);
    printf("Memory allocated\n");

    switch (difficulty) {
        case UNSET:
            set_difficulty();
            return;
        case EASY:
            stormtrooper_am = round((N*M) * 0.02);
            obstacle_am = round((N*M) * 0.13);
            break;
        case MEDIUM:
            stormtrooper_am = round((N*M) * 0.05);
            obstacle_am = round((N*M) * 0.10);
            break;
        case HARD:
            stormtrooper_am = round((N*M) * 0.1);
            obstacle_am = round((N*M) * 0.05);
            break;
        case IMPOSSIBLE:
            stormtrooper_am = round((N*M) * 0.15);
            obstacle_am = 0;
            break;
    }
    stormtrooper_am = maxi(2, stormtrooper_am);
    printf("%d stormtroopers, %d obstacles\n", stormtrooper_am, obstacle_am);

    printf("Placing obstacles\n");
    srand(time(NULL));
    i = 0, j = 0;
    while (obstacle_am > 0) {
        if (rand_range(0, 10) == 1 && Map[i][j] != obstacle_c) {
            Map[i][j] = obstacle_c;
            obstacle_am--;
        }
        i++;
        if (i == N) i = 0, j++;
    }
    
    printf("Filling empty spaces\n");
        for (i=0; i<N; i++) for (j=0; j<M; j++) {
            if (Map[i][j] != obstacle_c) Map[i][j] = empty_c;
    }

    printf("Allocating memory for stormtroopers\n");
    stormtrooper_am_tmp = stormtrooper_am;
    stormtrooper = (vec2*) malloc(sizeof(vec2) * stormtrooper_am);
    //if (stormtrooper == NULL) memory_error();
    printf("Allocating memory for stormtrooper's directions\n");
    stormtrooper_dir = (int*) malloc(sizeof(int) * stormtrooper_am);
    //if (stormtrooper_dir == NULL) memory_error();

    printf("Placing stormtroopers\n");
    int w = 0;
    i = 0, j = 0;
    while (stormtrooper_am_tmp > 0) {
        if (rand_range(0, 10) == 1 && Map[i][j] == empty_c) {
            Map[i][j] = strooper_c;
            stormtrooper_am_tmp--;
            stormtrooper[w].x = i;
            stormtrooper[w].y = j;
            stormtrooper_dir[w] = rand_range(0, 4);
            w++;
        }
        printf("(%d,%d) ", i, j);
        i++;
        if (i >= N) i = 0, j++;
        if (j >= M) j = 0;
    }

    printf("Placing Leia, Vader and R2D2\n");
    do {
        leia.x = rand_range(0, N);
        leia.y = rand_range(0, M);
        print_vector(leia);
    } while (*position(leia) != empty_c);
    *position(leia) = leia_c;

    do {
        vader.x = rand_range(0, N);
        vader.y = rand_range(0, M);
        print_vector(vader);
    } while (*position(vader) != empty_c);
    *position(vader) = vader_c;

    vec2 r2d2_pos;
    do {
        r2d2_pos.x = rand_range(0, N);
        r2d2_pos.y = rand_range(0, M);
        print_vector(r2d2_pos);
    } while (*position(r2d2_pos) != empty_c);
    *position(r2d2_pos) = r2d2_c;
    handle_turn(-1);
}

void handle_turn(int leias_move) {
    move_dir(&leia, leias_move, leia_c);
    handle_stormtroopers();
    if (vader_moved) vader_moved = false;
    else handle_vader();

    render_map();
    
    if (leias_move == -1) strcpy(NextMsg, "You are Leia. Find R2D2 and give him the plans!");
    if (strcmp(NextMsg, Msg)) write_slowly(NextMsg);
    else printf("\n\n%s", Msg);

    if (!game_ended) {
        char u = get_input();
        if (u == 'h') help();
        handle_turn(to_dir(u));
    } else if (game_ended == 1) {
        getch();
        level++;
        N--;
        M--;
        setup_map();
    }
}

void render_map() {
    system(clears);
    int i, j;
    printf("      ");
    for (i=0; i<N; i++) printf("%c ", alphabet[i]);
    printf("\n");
    for (i=0; i<N*2+6; i++) printf("-");
    printf("\n");

    for (i=0; i<M; i++) {
        printf("%02d | ", i+1);
        for (j=0; j<N; j++) {
            if (j == leia.x || i == leia.y || Map[j][i] == vader_c || got_help) {
                printf(" %c", Map[j][i]);
            } else printf(" %c", obf_c);
        }
        printf("\n");
    }
    printf("Level %d\n", level);
    printf("Move with %c%c%c%c ", toupper(up_c), toupper(left_c), toupper(down_c), toupper(right_c));
}

void print_vector(vec2 vector) {
    printf("\n(%d,%d)\n", vector.x, vector.y);
}

int move_dir(vec2* vector_src, int dir, char self) {
    vec2 new_pos, vector = *vector_src;
    if (dir < 0) {
        if (self == strooper_c) return true;
        else return false;
    }
    putc(self, stdout);
    if (dir > 3) printf("Invalid direction!");
    print_vector(vector);
    new_pos.x = vector.x;
    new_pos.y = vector.y;
    switch (dir) {
        case right:
            new_pos.x++;
            break;
        case left:
            new_pos.x--;
            break;
        case up:
            new_pos.y--;
            break;
        case down:
            new_pos.y++;
            break;
    }
    if (check_collision(new_pos, self)) {
        int tar = reposition(vector, new_pos);
        (*vector_src).x = new_pos.x;
        (*vector_src).y = new_pos.y;
        leia_check(new_pos, self, tar);
        return true;
    } else return false;
}

void leia_check(vec2 vector, char self, char tar) {
    printf("self: %c, target: %c\n", self, tar);
    if (self == strooper_c && tar == leia_c)
        hit(vector);
    if (self == leia_c && tar == r2d2_c) victory();
}

void hit(vec2 vector) {
    if (is_injured) {
        reposition(vector, leia);
        strcpy(NextMsg, "Leia was caught by a stormtrooper!\nGame over.\n\nPress X to return.");
        got_help = true;
        handle_turn(-2);
        game_ended = 2;
    } else {
        int i;
        for (i=0; i<stormtrooper_am; i++)
            if (stormtrooper[i].x == vector.x && stormtrooper[i].y == vector.y)
                stormtrooper_dir[i] = -1;
        strcpy(NextMsg, "Leia fought the stormtrooper and got injured.");
        reposition(leia, vector);
        is_injured = true;
        leia.x = vector.x;
        leia.y = vector.y;
        *position(leia) = leia_c;
    }
}

void victory() {
    strcpy(NextMsg, "Leia successfully gave the rebel plans to R2D2!\n\nPress any button to move to the next level.");
    game_ended = 1;
}

char reposition(vec2 from, vec2 to) {
    //print_vector(from);
    //print_vector(to);
    char chara = *position(from), rtn = *position(to);
    Map[from.x][from.y] = empty_c;
    Map[to.x][to.y] = chara;
    return rtn;
}

int check_collision(vec2 vector, char self) {
    if (vector.x>=0 && vector.x<N && vector.y>=0 && vector.y<M) {
        if (*position(vector) == empty_c || *position(vector) == leia_c ) return true;
        if (*position(vector) == r2d2_c && self == leia_c) return true;
        else return false;
    } else return false;
}

void handle_stormtroopers() {
    int i;
    for (i=0; i<stormtrooper_am; i++) {
        printf("%d", stormtrooper_dir[i]);
        if (stormtrooper_dir[i] != -1)
            if (!move_dir(&stormtrooper[i], stormtrooper_dir[i], strooper_c)) {
                stormtrooper_dir[i] = turn_around(stormtrooper_dir[i]);
                move_dir(&stormtrooper[i], stormtrooper_dir[i], strooper_c);
            }
    }
}

void handle_vader() {
    vec2 towards = get_dir_towards(vader, leia);
    int i = 0;
    while (!vader_moved) {
        printf("vader direction:");
        print_vector(towards);
        vader_moved = move_dir(&vader, towards.x, vader_c);
        vader_moved = move_dir(&vader, towards.y, vader_c) || vader_moved;
        if (!vader_moved) {
            if (leia.y == vader.y) towards.y = rand_range(0, 2);
            if (leia.x == vader.x) towards.x = rand_range(2, 4);
        }
        i++;
        if (i >= 8) {
            printf("Vader is stuck");
            return;
        }
    }
}

vec2 get_dir_towards(vec2 from, vec2 to) {
    int x_dist = from.x - to.x;
    int y_dist = from.y - to.y;
    vec2 rtn;

    if (x_dist > 0) rtn.x = left;
    else if (x_dist < 0) rtn.x = right;
    else rtn.x = -1;

    if (y_dist > 0) rtn.y = up;
    else if (y_dist < 0) rtn.y = down;
    else rtn.y = -1;
    
    return rtn;
}

int turn_around(int dir) {
    switch (dir) {
        case up: return down;
        case down: return up;
        case right: return left;
        case left: return right;
    }
    return dir;
}

char *position(vec2 vector) {
    return &Map[vector.x][vector.y];
}

void help() {
    strcpy(NextMsg, "Leia calls for help!");
    got_help = true;
}

void write_slowly(char *str) {
    int i;
    strcpy(Msg, str);
    printf("\n\n");
    for (i=0; i<(int)strlen(str); i++) {
        printf("%c", str[i]);
        usleep(500);
    }
    //sleep(1);
}

void title_screen() {
    ingame = false;
    start_pressed = false;
    system(clears);
    //Cool ASCII art for the title
    printf("Programming II:\n  _____           _           _     __ \n |  __ \\         (_)         | |   /_ |\n | |__) | __ ___  _  ___  ___| |_   | |\n |  ___/ '__/ _ \\| |/ _ \\/ __| __|  | |\n | |   | | | (_) | |  __/ (__| |_   | |\n |_|   |_|  \\___/| |\\___|\\___|\\__|  |_|\n                _/ |                   \n               |__/  A New Hope\n");
    const int menu_size = 5;
    char *options[5] = {"START", "BOARD SIZE", "DIFFICULTY", "CONTROL SCHEME", "QUIT"};
    print_menu(options, menu_size);
    printf("\nUse %c and %c to navigate, %c to select ", toupper(up_c), toupper(down_c), toupper(right_c));
    char u = to_dir(get_input());
    if (u == down) {
        if (menu_index < menu_size-1) menu_index++;
        title_screen();
        return;
    } else if (u == up) {
        if (menu_index > 0) menu_index--;
        title_screen();
        return;
    } else if (u != right) {
        title_screen();
        return;
    }
    //It will only get to this point if u == right
    switch (menu_index) {
        case 0:
            start_pressed = true;
            setup_map();
            break;
        case 1:
            system(clears);
            set_board_size();
            break;
        case 2:
            menu_index = 0;
            set_difficulty();
            break;
        case 3:
            set_control_scheme();
            break;
        case 4:
            system(clears);
            exit(2);
            break;
    }
}

void set_board_size() {
    level = 1;
    printf("\nSetup the maximum size of the board\n\nEnter the number of colummns: ");
    scanf("%d", &N);
    printf("Enter the number of rows: ");
    scanf("%d", &M);
    if (N >= 5 && M >= 5) {
            printf("\n%dx%d, is that correct? (Y/N)", N, M);
            if (getch() == 'y') {
                if (start_pressed) setup_map();
                else title_screen();
            }
            else set_board_size();
        } else {
            printf("\nThat's too small, the minimum size is 5x5.\n");
            set_board_size();
    }
}

void set_difficulty() {
    system(clears);
    printf("Select difficulty\n");
    const int menu_size = 4;
    char *options[4] = {"EASY", "MEDIUM", "HARD", "IMPOSSIBLE"};
    print_menu(options, menu_size);
    printf("\nUse %c and %c to navigate, %c to select ", toupper(up_c), toupper(down_c), toupper(right_c));
    char u = to_dir(get_input());
    if (u == down) {
        if (menu_index < menu_size-1) menu_index++;
        set_difficulty();
        return;
    } else if (u == up) {
        if (menu_index > 0) menu_index--;
        set_difficulty();
        return;
    } else if (u == left) {
        menu_index = 0;
        title_screen();
        return;
    } else if (u != right) {
        set_difficulty();
        return;
    }
    //It will only get to this point if u == right
    switch (menu_index) {
        case 0:
            difficulty = EASY;
            break;
        case 1:
            difficulty = MEDIUM;
            break;
        case 2:
            difficulty = HARD;
            break;
        case 3:
            difficulty = IMPOSSIBLE;
            break;
    }
    menu_index = 0;
    if (start_pressed) setup_map();
    else title_screen();
}

void print_menu(char *options[], int length) {
    int i;
    for (i=0; i<length; i++) {
        if (i == menu_index) printf("> ");
        else printf("  ");
        printf("%s\n", options[i]);
    }
}

char get_input() {
    char u;
    if (better_controls)
        u = getch();
    else {
        printf("\nYour input: ");
        u = getc(stdin);
    }
    u = tolower(u);
    if (u == 'x') {
        if (ingame) {
            free_everything();
            title_screen();
        } else {
            system(clears);
            exit(1);
        }
    }
    return u;
}

int to_dir(char c) {
    if (c == right_c) {
        return right;
    } else if (c == left_c) {
        return left;
    } else if (c == up_c) {
        return up;
    } else if (c == down_c) {
        return down;
    } else return c;
}

void set_control_scheme() {
    int scheme;
    do {
        system(clears);
        printf("Select a control scheme\n 1 - WSAD, no enter\n 2 - ULDR, then enter\n\n");
        printf("1 is obviously the better option, 2 was just inlcuded out of obligation.");
        scheme = getch();
    } while (scheme != '1' && scheme != '2');
    if (scheme == '2') {
        better_controls = false;
        up_c = 'u';
        down_c = 'd';
        left_c = 'l';
        right_c = 'r';
    } else {
        better_controls = true;
        up_c = 'w';
        down_c = 's';
        left_c = 'a';
        right_c = 'd';
    }
    title_screen();
}

void free_everything() {
    if (Map == NULL) return;
    free(Map);
    free(stormtrooper);
    free(stormtrooper_dir);
    Map = NULL;
    stormtrooper = NULL;
    stormtrooper_dir = NULL;
}

int maxi(int a, int b) {
    if (a > b) return a;
    return b;
}

int rand_range(int from, int to) { 
    return (rand() % (to - from)) + from;
} 

void memory_error() {
    printf("you don't have enough memory in the memory card");
    exit(9);
}

//I did not write this function, I took it from https://stackoverflow.com/questions/7469139/what-is-the-equivalent-to-getch-getche-in-linux
//it's an equvlent of getch() from <conio.h> on windows. It gets one char with no need for enter.
#ifdef __linux__
char getch(void) {
    char buf = 0;
    struct termios old = {0};
    fflush(stdout);
    if(tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if(tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if(read(0, &buf, 1) < 0)
        perror("read()"); 
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if(tcsetattr(0, TCSADRAIN, &old) < 0)
        perror("tcsetattr ~ICANON");
    printf("%c\n", buf);
    return buf;
}
#endif