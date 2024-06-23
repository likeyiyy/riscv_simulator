#include <ncurses.h>

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);

int main() {
    initscr();            // Start curses mode
    cbreak();             // Line buffering disabled, Pass on every key press
    noecho();             // Don't echo() while we do getch
    keypad(stdscr, TRUE); // Enable special keys input
    // Clear the screen
    clear();
    refresh();

    int height = 10;
    int width = 30;
    int starty = 1;       // Starting position y of the first window
    int startx = 1;       // Starting position x of the first window

    // Create windows
    WINDOW *reg_win = create_newwin(height, width, starty, startx);
    WINDOW *stack_win = create_newwin(height, width, starty, startx + width + 1);
    WINDOW *source_win = create_newwin(height, width, starty + height + 1, startx);

    // Print something in the windows
    mvwprintw(reg_win, 1, 1, "This is reg_win");
    mvwprintw(stack_win, 1, 1, "This is stack_win");
    mvwprintw(source_win, 1, 1, "This is source_win");

    // Refresh each window
    wrefresh(reg_win);
    wrefresh(stack_win);
    wrefresh(source_win);

    // Wait for user input
    getch();

    // Clean up and exit
    destroy_win(reg_win);
    destroy_win(stack_win);
    destroy_win(source_win);
    endwin();

    return 0;
}

// Function to create a new window
WINDOW *create_newwin(int height, int width, int starty, int startx) {
    WINDOW *local_win;
    local_win = newwin(height, width, starty, startx);
    box(local_win, 0 , 0); // 0, 0 gives default characters for the vertical and horizontal lines
    wrefresh(local_win);   // Show that box

    return local_win;
}

// Function to destroy a window
void destroy_win(WINDOW *local_win) {
    wborder(local_win, ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ');
    wrefresh(local_win);
    delwin(local_win);
}
