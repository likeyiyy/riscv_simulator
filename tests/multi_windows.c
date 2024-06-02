#include <ncurses.h>
#include <string.h>
#include <unistd.h>

WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);
void scroll_window(WINDOW *win, const char *text, int *offset);

int main() {
    initscr();            // Start curses mode
    cbreak();             // Line buffering disabled, Pass on every key press
    noecho();             // Don't echo() while we do getch
    keypad(stdscr, TRUE); // Enable special keys input

    int height = 10;
    int width = 30;
    int starty = 1;       // Starting position y of the first window
    int startx = 1;       // Starting position x of the first window

    // Clear the screen
    clear();
    refresh();

    // Create windows
    WINDOW *reg_win = create_newwin(height, width, starty, startx);
    WINDOW *stack_win = create_newwin(height, width, starty, startx + width + 1);
    WINDOW *source_win = create_newwin(height, width, starty + height + 1, startx);

    const char *reg_text = "This is reg_win scrolling text. ";
    const char *stack_text = "This is stack_win scrolling text. ";
    const char *source_text = "This is source_win scrolling text. ";
    int reg_offset = 0, stack_offset = 0, source_offset = 0;

    while (1) {
        // Scroll text in each window
        scroll_window(reg_win, reg_text, &reg_offset);
        scroll_window(stack_win, stack_text, &stack_offset);
        scroll_window(source_win, source_text, &source_offset);

        // Refresh each window
        wrefresh(reg_win);
        wrefresh(stack_win);
        wrefresh(source_win);

        // Wait for 1 second
        napms(1000);
    }

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

// Function to scroll text in a window
void scroll_window(WINDOW *win, const char *text, int *offset) {
    int win_height, win_width;
    getmaxyx(win, win_height, win_width);
    werase(win); // Clear the window
    box(win, 0, 0); // Redraw the box

    // Print the text starting from the current offset
    for (int i = 1; i < win_height - 1; ++i) {
        for (int j = 1; j < win_width - 1; ++j) {
            int text_pos = (*offset + (i - 1) * (win_width - 2) + (j - 1)) % strlen(text);
            mvwaddch(win, i, j, text[text_pos]);
        }
    }

    // Update the offset for the next scroll
    *offset = (*offset + 1) % strlen(text);
}
