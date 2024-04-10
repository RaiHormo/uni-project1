#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include <conio.h> // Windows specific
#include <windows.h> //Windows specific
//#include <curses.h> //Linux specific
//#include <unistd.h> //Linux specific


//Constants
#define false 0
#define true 1
#define up 0
#define down 1
#define left 2
#define right 3

const char *clear = "cls"; //This changes from one OS to the other, "cls" works on windows

const char obstacle_c = 'X';
const char empty_c = '.';
const char strooper_c = '@';
const char vader_c = 'D';
const char leia_c = 'L';
const char r2d2_c = 'R';
const char obf_c = '#';

const char *alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";

//Controls
char up_c = 'w';
char down_c = 's';
char left_c = 'a';
char right_c = 'd';

//Globals
char **Map;
int menu_index = 0, N = 0, M = 0, start_pressed = false, better_controls = true;
enum diffs {UNSET, EASY, MEDIUM, HARD, IMPOSSIBLE};
enum diffs difficulty = UNSET;

int leia[2], *stormtrooper_dir, **stormtrooper, stormtrooper_am;

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
int check_collision(int to[2]);
int move(int vector[2], int dir);
void reposition(int from[2], int to[2]);
void handle_turn(int leias_move);
char* position(int vector[2]);
void print_vector(int vector[2]);
void handle_stormtroopers();
int turn_around(int dir);

//Code
int main() {
    set_control_scheme();
    title_screen();
}

void setup_map() {
    system(clear);
    if (N == 0 || M == 0) {
        system(clear);
        set_board_size();
    }
    int i, j, stormtrooper_am_tmp, obstacle_am;

    Map = (char**) malloc(sizeof(char*) * N);
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
        if ((rand() % 10) == 1 && Map[i][j] != obstacle_c) {
            Map[i][j] = obstacle_c;
            obstacle_am--;
        }
        i++;
        if (i == N) i = 0, j++;
    }
    
    printf("Filling empty spaces\n");
        for (i=0; i<N; i++) for (j=0; j<M; j++) {
            if (Map[i][j] != obstacle_c && Map[i][j] != strooper_c) Map[i][j] = '.';
    }

    printf("Allocating memory for stormtroopers\n");
    stormtrooper_am_tmp = stormtrooper_am;
    stormtrooper = (int**) malloc(sizeof(int*) * stormtrooper_am);
    stormtrooper_dir = (int*) malloc(sizeof(int) * stormtrooper_am);
    for (i=0; i<stormtrooper_am; i++) {
        stormtrooper[i] = (int*) malloc(sizeof(int) * 2);
        stormtrooper_dir[i] = rand() % 4;
    }
    printf("Placing stormtroopers\n");
    int w = 0;
    while (stormtrooper_am_tmp > 0) {
        if ((rand() % 10) == 1 && Map[i][j] == empty_c) {
            Map[i][j] = strooper_c;
            stormtrooper_am_tmp--;
            stormtrooper[w][0] = i;
            stormtrooper[w][1] = j;
        }
        i++;
        if (i == N) i = 0, j++;
    }

    printf("Placing Leia, Vader and R2D2\n");
    do {
        leia[0] = rand() % N;
        leia[1] = rand() % M;
        print_vector(leia);
    } while (*position(leia) != empty_c);
    *position(leia) = leia_c;

    int vader_pos[2];
    do {
        vader_pos[0] = rand() % N;
        vader_pos[1] = rand() % M;
        print_vector(vader_pos);
    } while (*position(vader_pos) != empty_c);
    *position(vader_pos) = vader_c;

    int r2d2_pos[2];
    do {
        r2d2_pos[0] = rand() % N;
        r2d2_pos[1] = rand() % M;
        print_vector(r2d2_pos);
    } while (*position(r2d2_pos) != empty_c);
    *position(r2d2_pos) = r2d2_c;

    handle_turn(-1);
}

void handle_turn(int leias_move) {
    move(leia, leias_move);
    handle_stormtroopers();

    render_map();
    Sleep(50);

    char u = get_input();
    handle_turn(to_dir(u));
}

void render_map() {
    system(clear);
    int i, j;
    printf("      ");
    for (i=0; i<N; i++) printf("%c ", alphabet[i]);
    printf("\n");
    for (i=0; i<N*2+6; i++) printf("-");
    printf("\n");

    for (i=0; i<M; i++) {
        printf("%02d | ", i+1);
        for (j=0; j<N; j++) {
            printf(" %c", Map[j][i]);
        }
        printf("\n");
    }
    printf("Level %d\n", 1);
    printf("Move with %c%c%c%c ", toupper(up_c), toupper(left_c), toupper(down_c), toupper(right_c));
}

void print_vector(int vector[2]) {
    printf("\n(%d,%d)\n", vector[0], vector[1]);
}

int move(int vector[2], int dir) {
    int new_pos[2];
    new_pos[0] = vector[0];
    new_pos[1] = vector[1];
    switch (dir) {
        case right:
            new_pos[0]++;
            break;
        case left:
            new_pos[0]--;
            break;
        case up:
            new_pos[1]--;
            break;
        case down:
            new_pos[1]++;
            break;
    }
    if (check_collision(new_pos)) {
        reposition(vector, new_pos);
        if (vector[0] == leia[0] && vector[1] == leia[1]) {
            leia[0] = new_pos[0];
            leia[1] = new_pos[1];
        }
        return true;
    } else return false;
}

void reposition(int from[2], int to[2]) {
    //print_vector(from);
    //print_vector(to);
    char chara = *position(from);
    Map[from[0]][from[1]] = '.';
    Map[to[0]][to[1]] = chara;
}

int check_collision(int vector[2]) {
    if (vector[0]>=0 && vector[0]<N && vector[1]>=0 && vector[1]<M) {
        if (*position(vector) == '.') return true;
        else return false;
    } else return false;
}

void handle_stormtroopers() {
    int i;
    for (i=0; i<stormtrooper_am; i++) {
        if (!move(stormtrooper[i], stormtrooper_dir[i]))
            stormtrooper_dir[i] = turn_around(stormtrooper_dir[i]);
    }
}

int turn_around(int dir) {
    switch (dir) {
        case up: return down;
        case down: return up;
        case right: return left;
        case left: return right;
    }
    return -1;
}

void get_position(int vector[2], char chara) {
    int i, j;
    for (i=0; i<N; i++) for (j=0; j<N; j++) {
        if (Map[i][j] == chara) {
            vector[0] = i;
            vector[1] = j;
        }
    }
}

char* position(int vector[2]) {
    return &Map[vector[0]][vector[1]];
}

void title_screen() {
    start_pressed = false;
    system(clear);
    //Cool ASCII art for the title
    printf("Programming II:\n  _____           _           _     __ \n |  __ \\         (_)         | |   /_ |\n | |__) | __ ___  _  ___  ___| |_   | |\n |  ___/ '__/ _ \\| |/ _ \\/ __| __|  | |\n | |   | | | (_) | |  __/ (__| |_   | |\n |_|   |_|  \\___/| |\\___|\\___|\\__|  |_|\n                _/ |                   \n               |__/                    \n");
    const int menu_size = 4;
    char *options[4] = {"START", "SET BOARD SIZE", "SET DIFFICULTY", "QUIT"};
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
            system(clear);
            set_board_size();
            break;
        case 2:
            menu_index = 0;
            set_difficulty();
            break;
        case 3:
            system(clear);
            exit(2);
            break;
    }
}

void set_board_size() {
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
    system(clear);
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
    if (u == '\n') return get_input();
    u = tolower(u);
    if (u == 'x') exit(1);
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
    system(clear);
    int scheme;
    do {
        printf("Select a control scheme\n 1 - WSAD, no enter\n 2 - ULDR, then enter\n\n");
        printf("1 is obviously the better option, 2 was just inlcuded out of obligation.");
        scheme = get_input();
    } while (scheme != '1' && scheme != '2');
    if (scheme == '2') {
        better_controls = false;
        up_c = 'u';
        down_c = 'd';
        left_c = 'l';
        right_c = 'r';
    }
}

int maxi(int a, int b) {
    if (a > b) return a;
    else return b;
}