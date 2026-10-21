#include <Windows.h>
#include <stdio.h>
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
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    int columns;

    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        columns = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    } else {
        columns = 80; // returns default width if screen buffer info is unavailable...
    }
    return columns;
}

void Indent(int indentation){
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
