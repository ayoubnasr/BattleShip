#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include "../include/UITools.h"

/**
 * A function that prints a string in color. Developer could pick from the defined colors in UITools.h.
 */
void PrintClr(char* text, char* color){
    if (text == NULL) return;

    printf("%s%s%s", color, text, RESET);

}

/**
 * A function that prints a string, then a new line, but in color. Developer could pick from the defined colors in UITools.h.
 */
void PrintlnClr(char* text, char* color){
    if (text == NULL) return;

    printf("%s%s%s\n", color, text, RESET);

}

void SetColor(char* color){

    printf("%s", color);

}

void ResetFormat(){
    printf(RESET);
}







///UI positioning functions:

/**
 * A function that returns the width of the console window.
 */
int getConsoleWidth() {
    int columns = 80; // default width if the console size is unavailable (eg. output is piped)...

#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;

    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
#else
    struct winsize ws;

    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) == 0 && ws.ws_col > 0) {
        columns = ws.ws_col;
    }
#endif
    return columns;
}

void Indent(int indentation){
    if (indentation < 0) indentation = 0; //Text wider than the console would give a negative width
    printf("%*s", indentation, "");
}


void Print_Centered(char* text, int textLen, char* color){
    if (text == NULL) return;

    Indent((getConsoleWidth() - textLen) / 2);
    PrintClr(text, color);
}

void Println_Centered(char* text, int textLen, char* color){
    if (text == NULL) return;

    Indent((getConsoleWidth() - textLen) / 2);
    PrintlnClr(text, color);
}

void SetBold(){
    printf("%s", BOLD);
}

void ClearScreen(){

    #ifdef _WIN32
        system("cls");
    #else
        system("clear");  // Unix-based systems (Linux, macOS)
    #endif

}
