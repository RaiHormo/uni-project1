#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
//#include <terminos.h>

//Constants
const char *clear = "cls"; //This changes from one OS to the other, "cls" works on windows

//Controls
char up = 'u';
char down = 'd';
char left = 'l';
char right = 'r';

//Globals
int *Map;
int menu_index = 0, N, M;

//Funcion definitions
void set_control_scheme();
void title_screen();
void print_menu(char *options[], int length);
char get_input();
void set_board_size();

//Code
int main() {
    system(clear);
    set_control_scheme();
    title_screen();
}

void title_screen() {
    system(clear);
    //Cool ASCII art for the title
    printf("Programming II:\n  _____           _           _     __ \n |  __ \\         (_)         | |   /_ |\n | |__) | __ ___  _  ___  ___| |_   | |\n |  ___/ '__/ _ \\| |/ _ \\/ __| __|  | |\n | |   | | | (_) | |  __/ (__| |_   | |\n |_|   |_|  \\___/| |\\___|\\___|\\__|  |_|\n                _/ |                   \n               |__/                    \n");
    const int menu_size = 3;
    char *options[3] = {"START", "SET BOARD SIZE", "QUIT"};
    print_menu(options, menu_size);
    printf("\nUse %c and %c to navigate, %c to select ", toupper(up), toupper(down), toupper(right));
    char u = get_input();
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
        case 1:
            system(clear);
            set_board_size();
            break;
        case 2:
            system(clear);
            exit(2);
            break;
    }
}

void set_board_size() {
    printf("Setup the maximum size of the board\n\nEnter the number of collumns: ");
    N = get_input() - '0';
    printf("\nEnter the number of rows: ");
    M = get_input() - '0';
    if (N > 8 && M > 8) {
        printf("\n%dx%d, is that correct? (Y/N)", N, M);
        if (get_input() == 'y') title_screen();
        else set_board_size();
    } else {
        printf("\nThat's too small, the minimum size is 8x8.");
        set_board_size();
    }
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
    char u = getc(stdin);
    u = tolower(u);
    if (u == 'x') exit(1);
    return u;
}

void set_control_scheme() {
    int scheme;
    do {
        printf("Select a control scheme\n 1 - UDRL\n 2 - WSAD\n");
        scheme = get_input();
    } while (scheme != '1' && scheme != '2');
    if (scheme == '2') {
        up = 'w';
        down = 's';
        left = 'a';
        right = 'd';
    }
}