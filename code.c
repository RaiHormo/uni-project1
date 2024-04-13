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

//Globals
char **Map, Msg[50], NextMsg[50];
int menu_index = 0, N = 0, M = 0, start_pressed = false, better_controls = true, ingame = false, is_injured = false, got_help = false;
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
int move_dir(int vector[2], int dir);
void reposition(int from[2], int to[2]);
void handle_turn(int leias_move);
char* position(int vector[2]);
void print_vector(int vector[2]);
void handle_stormtroopers();
int turn_around(int dir);
void memory_error();
void free_everything();
void write_slowly(char* str);
void leia_check(int vector[2]);
void help();
#ifdef __linux__
    char getch(void);
#endif

//Code
int main() {
    title_screen();
}

void setup_map() {
    system(clears);
    is_injured = false;
    got_help = false;
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
        if ((rand() % 10) == 1 && Map[i][j] != obstacle_c) {
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
    stormtrooper = (int**) malloc(sizeof(int*) * stormtrooper_am);
    printf("Allocating memory for stormtrooper's directions\n");
    stormtrooper_dir = (int*) malloc(sizeof(int) * stormtrooper_am);
    if (stormtrooper == NULL || stormtrooper_dir == NULL) memory_error();
    printf("Allocating memory for stormtrooper's vectors\n");
    for (i=0; i<stormtrooper_am; i++) {
        printf("%d", i);
        stormtrooper[i] = (int*) malloc(sizeof(int) * 2);
        if (stormtrooper[i] == NULL) memory_error();
        stormtrooper_dir[i] = rand() % 4;
    }

    printf("Placing stormtroopers\n");
    int w = 0;
    i = 0, j = 0;
    while (stormtrooper_am_tmp > 0) {
        if ((rand() % 10) == 1 && Map[i][j] == empty_c) {
            Map[i][j] = strooper_c;
            stormtrooper_am_tmp--;
            stormtrooper[w][0] = i;
            stormtrooper[w][1] = j;
            w++;
        }
        printf("(%d,%d) ", i, j);
        i++;
        if (i >= N) i = 0, j++;
        if (j >= M) j = 0;
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
    handle_stormtroopers();
    move_dir(leia, leias_move);

    render_map();
    
    if (leias_move == -1) strcpy(NextMsg, "You are Leia. Find R2D2 and give him the plans!");
    if (strcmp(NextMsg, Msg)) write_slowly(NextMsg);
    else printf("\n\n%s", Msg);

    char u = get_input();
    if (u == 'h') help();
    handle_turn(to_dir(u));
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
            if (j == leia[0] || i == leia[1] || Map[j][i] == vader_c || got_help) {
                printf(" %c", Map[j][i]);
            } else printf(" %c", obf_c);
        }
        printf("\n");
    }
    printf("Level %d\n", 1);
    printf("Move with %c%c%c%c ", toupper(up_c), toupper(left_c), toupper(down_c), toupper(right_c));
}

void print_vector(int vector[2]) {
    printf("\n(%d,%d)\n", vector[0], vector[1]);
}

int move_dir(int vector[2], int dir) {
    //print_vector(vector);
    //printf("%d", dir);
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
    if (vector[0] == leia[0] && vector[1] == leia[1]) {
        leia_check(new_pos);
    }
    if (check_collision(new_pos)) {
        reposition(vector, new_pos);
        vector[0] = new_pos[0];
        vector[1] = new_pos[1];
        return true;
    } else return false;
}

void leia_check(int vector[2]) {
    if (vector[0]>=0 && vector[0]<N && vector[1]>=0 && vector[1]<M) {
        if (*position(vector) == strooper_c) {
            int i;
            for (i=0; i<stormtrooper_am; i++)
                if (stormtrooper[i][0] == vector[0] && stormtrooper[i][1] == vector[1])
                    stormtrooper_dir[i] = -1;
            strcpy(NextMsg, "Leia fought the stormtrooper and got injured.");
            reposition(leia, vector);
            is_injured = true;
            leia[0] = vector[0];
            leia[1] = vector[1];
        }
    }
}

void reposition(int from[2], int to[2]) {
    //print_vector(from);
    //print_vector(to);
    char chara = *position(from);
    Map[from[0]][from[1]] = empty_c;
    Map[to[0]][to[1]] = chara;
}

int check_collision(int vector[2]) {
    if (vector[0]>=0 && vector[0]<N && vector[1]>=0 && vector[1]<M) {
        if (*position(vector) == empty_c) return true;
        else return false;
    } else return false;
}

void handle_stormtroopers() {
    int i;
    for (i=0; i<stormtrooper_am; i++) {
        //print_vector(stormtrooper[i]);
        if (stormtrooper_dir[i] != -1)
            if (!move_dir(stormtrooper[i], stormtrooper_dir[i])) {
                stormtrooper_dir[i] = turn_around(stormtrooper_dir[i]);
                move_dir(stormtrooper[i], stormtrooper_dir[i]);
            }
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

char *position(int vector[2]) {
    return &Map[vector[0]][vector[1]];
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
    free(Map);
    free(stormtrooper);
    free(stormtrooper_dir);
}

int maxi(int a, int b) {
    return (a > b);
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