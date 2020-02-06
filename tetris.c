#include <stdlib.h>
#include <curses.h>
#include <signal.h>
#include <locale.h>
#include <time.h>
#include <unistd.h>

int pieces[7][16] = {{
0,0,1,0,
0,0,1,0,
0,0,1,0,
0,0,1,0
},{
0,0,0,0,
0,1,1,0,
0,1,1,0,
0,0,0,0
},{
0,0,0,0,
0,1,1,0,
0,0,1,1,
0,0,0,0
},{
0,0,0,0,
0,1,1,0,
1,1,0,0,
0,0,0,0
},{
0,0,0,0,
1,1,1,0,
0,1,0,0,
0,0,0,0
},{
0,1,0,0,
0,1,0,0,
1,1,0,0,
0,0,0,0
},{
0,1,0,0,
0,1,0,0,
0,1,1,0,
0,0,0,0
}};
typedef struct activePiece activePiece;
struct activePiece
{
    int x;
    int y;
    int *piece;
    int id;
};

int left = 20;
int n = 1;
int height = 20;
int width = 12;
int field[240] = { 0 };

static void finish(int sig);

void generatePlayField() {
    //Rows
    
    for(int i = 0; i <= height; i++) {
        for(int j = 0; j <= width; j++) {
            if (i == 20) {
                field[i * width + j] = 1;
            } else {
                field[i * width + j] = (j == 0 || j == width) ? 1 : 0;
            }
        }
    }
}



void displayField() {
    for(int i = 0; i <= height; i++) {
        for(int j = 0; j <= width; j++) {
            move(i, j * 2 + left);
            int p = field[i * width + j] == 1 ? 0 : field[i * width + j] % 6;
            if(field[i * width + j] == INT16_MAX) {
                p = 6;
            }
            attron(COLOR_PAIR(p));
            
            char *str = field[i * width + j] >= 1 ? "\u2588\u2588" : "  ";
            
            addstr(str); //Print out the unicode character
            attroff(COLOR_PAIR(p));
        }
    }
}

int inArray(int val, int arr[])
{
    int i;
    for(i = 0; i < 16; i++)
    {
        if(arr[i] == val)
            return 1;
    }
    return 0;
}

// rotation can be 0, 1, 2 or 3
void displayPiece(activePiece active) {
    for(int i = 0; i <= height; i++) {
        for(int j = 0; j <= width; j++) {
            if (i >= active.y && i < active.y+4 && j >= active.x + 1 && j < active.x+5) {
                // printf("%i - %i\n", (i-active.y) * 4 + (j-1-active.x), active.piece[(i-active.y) * 4 + (j - 1 - active.x)] * (active.id + 1));
                int k = (i-active.y) * 4 + (j - 1 - active.x);
                field[i * width + j] = field[i * width + j] >= 1 && field[i * width + j] < active.id + 1 ? field[i * width + j] : active.piece[k] * (active.id + 1);
            } else if (i < height && j > 0 && j < width && field[i * width + j] >= (active.id + 1) && field[i * width + j] < INT16_MAX) {
                field[i * width + j] = 0;
            }
        }
    }
}
// Return 1 or 0 depending on whether the piece could be moved to it's new position or not
int canMovePiece(int x, int y, activePiece active) {
    for (int i = 0; i < 4;i++) {
        for (int j = 0; j < 4;j++) {
            int below = field[(active.y + i + 1) * width + active.x + j + 1];
            int left = field[(active.y + i) * width + active.x + j ];
            int right = field[(active.y + i) * width + active.x + j + 2];

            if (active.piece[i * 4 + j] == 1 
            && ((below <= active.id && below >= 1 && y == 1) 
            || (right <= active.id && right >= 1 && x == 1) 
            || (left <= active.id && left >= 1 && x == -1))) {
                return 0;
            }
        }
    }
    
    return 1;
}

void rotatePiece(int data[16], int *newData) {
    // TODO Do not rotate if it would create a collision
    for(int i = 0; i < 4; i++) {
        for(int j = 0; j < 4; j++) {
            newData[j * 4 + 3 - i] = data[i * 4 + j];
        }
    }
}
int gameOver() {
    return 0;
    for (int i = 1; i < width; i++) {
        if (field[i] >= 1 && field[i] < n) {
            return 1;
        }
    }
    return 0;
}

int checkLine() {
    int cnt = 0;
    int lines = 0;
    for (int i = 0; i < height; i++) {
        cnt = 0;
        for (int j = 1; j < width; j++) {
            if (field[i * width + j] >= 1) {
                cnt++;
            }
        }
        
        if (cnt >= width - 1) {
            for (int j = 1; j < width; j++) {
                field[i * width + j] = INT16_MAX;
            }
            lines += 1;
        }
    }
    return lines;
}

void removeLines(int linesToRemove) {
    for (int k = 0; k < linesToRemove; k++) {
        for (int i = 0; i < height; i++) {
            if (field[i * width + 2] == INT16_MAX) {
                
                // We are on a removed line, move everything down 1
                for (int l = i; l >= 0; l--) {
                    for (int j = 1; j < width; j++) {
                        if (l - 1 >= 0) {
                            field[l * width + j] = field[(l-1) * width + j];
                        } else {
                            field[l * width + j] = 0;
                        }
                        
                    }
                }
                i=height;
                break;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    (void) signal(SIGINT, finish);      /* arrange interrupts to terminate */
    setlocale(LC_ALL, "");
    (void) initscr();      /* initialize the curses library */
    keypad(stdscr, FALSE);  /* enable keyboard mapping */
    (void) nonl();         /* tell curses not to do NL->CR/NL on output */
    (void) cbreak();       /* take input chars one at a time, no wait for \n */
    (void) noecho();
    (void) timeout(0);
    (void) nodelay(stdscr, TRUE);
    curs_set(0); //remove cursor

    srand(time(0));

    int c;
    int y = 0;
    int x = 0;
    
    // Create a thread for the loop.
    generatePlayField();
    
    activePiece active = {5, 0, pieces[rand() % 7], n};

    int level = 1;

    int pressedKeys[16];
    int linesToRemove = 0;
    int score = 0;
    unsigned long cnt = 0;
    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_BLACK);
        init_pair(2, COLOR_BLUE, COLOR_BLACK);
        init_pair(3, COLOR_CYAN, COLOR_BLACK);
        init_pair(4, COLOR_GREEN, COLOR_BLACK);
        init_pair(5, COLOR_MAGENTA, COLOR_BLACK);
        init_pair(6, COLOR_YELLOW, COLOR_BLACK);
        init_pair(7, COLOR_BLACK, COLOR_BLACK);
    }

    while(!gameOver())
    {
        displayPiece(active);
        displayField();
        char p[32];

        move(0, 0);
        sprintf(p, "LEVEL - %i", level);
        addstr(p);
        move(1, 0);
        sprintf(p, "SCORE - %i", score);
        addstr(p);
        if (cnt % (100 / level) == 0) {
            score += 10;
            if (canMovePiece(0, 1, active)) {
                active.y += 1;
                if (linesToRemove) {
                    removeLines(linesToRemove);
                    linesToRemove = 0;

                }
            } else {
                // this piece is stuck, create a new active piece
                linesToRemove = checkLine();
                if (linesToRemove) {
                    score += linesToRemove * 100;
                    if (linesToRemove >= 2) {
                        score += linesToRemove * 100;
                    }
                }
                activePiece old = active;
                n++;
                active.x = 4; 
                active.y = 0;
                active.piece = pieces[rand() % 7];
                active.id = n;

                //TODO check if we have a line
                
            }
        }

        refresh();
        usleep(10000);
        
        for (int k=0; k < 16; k++) {
            pressedKeys[k] = 0;
        }
        
        int gcnt = 0;
        while ((c = getch()) != ERR) {
            pressedKeys[gcnt] = c;
            gcnt++;
        }

        move(1, 0);
        //addstr("     ");

        if (inArray(65, pressedKeys)) {
            //UP
            int data[16] = {0};
            for (int l = 0; l < 16; l++) {
                data[l] = active.piece[l];
            }
            int newData[16] = {0};
            rotatePiece(data, newData);
            active.piece = newData;
        }

        if (inArray(68, pressedKeys)) {
            //LEFT
            move(1, 0);
            //addstr("LEFT");
            if (canMovePiece(-1, 0, active)) {
                active.x -= 1;
            }
            
        }
        if (inArray(67, pressedKeys)) {
            //RIGHT
            move(1, 0);
            //addstr("RIGHT");
            if (canMovePiece(1, 0, active)) {
                active.x += 1;
            }
        }
        if (inArray(66, pressedKeys)) {
            //DOWN
            move(1, 0);
            //addstr("DOWN");
            if (canMovePiece(0, 1, active)) {
                active.y += 1;
            }
        }
        
        

        cnt += 1;
        if (cnt % 10000 == 0 && level < 100) {
            level++;
        }
        /* process the command keystroke */
    }


    //Game over

    finish(0);               /* we are done */

}

static void finish(int sig)
{
    endwin();

    /* do your non-curses wrapup here */

    exit(0);
}