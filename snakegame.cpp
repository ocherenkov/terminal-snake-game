/* 
///////////////////////////////////////////////////////////
   SNAKE GAME FOR MACOS

   "Snake Game" only operated on the macOS
   If you want to compile this file, you need to use Command
                        -> gcc -o snakegame snakegame.cpp -lncurses

   * Need to revise : show prize and penalty
///////////////////////////////////////////////////////////
*/

#include <iostream>
#include <cstdlib>
#include <ncurses.h> // input output
#include <unistd.h> // sleep function

//Global Variable
bool gameOver;
int modeGame;
int typeGame;
enum GameMode { WITHWALL = 1, WITHOUTWALL = 2 };
enum GameType { ENDLESS = 1, FIVEMINUTES = 2, THOUSANDSCORE = 3 };
int side; //field
int x, y; //snake head
int fruitX, fruitY;
int score;
time_t startTime;

int snakeColor = 1;
int fruitColor = 2;
int wallColor = 3;
int scoreColor = 4;
int timeColor = 5;

char fruit = '@';
char snakeHead = 'O';
char snakeTail = 'o';
char wallVertical = '*';
char wallHorizontal = '*';

enum eDirection {
    STOP = 0, LEFT, RIGHT, UP, DOWN
};

eDirection dir;

// snake tail. Change this for max size of map.
int tailX[40 * 40], tailY[40 * 40];
int nTail = 0;

int randomFruitPos() {
    return (rand() % side) + 1;
}

bool IsOccupied(int posX, int posY) {
    // Check if the given position overlaps with the snake's body
    for (int i = 0; i < nTail; i++) {
        if (tailX[i] == posX && tailY[i] == posY) {
            return true;
        }
    }
    if (posX == 0 || posY == 0) {
        return true;
    }
    return false;
}

bool IsSnakeFilled() {
    // Check if the snake has filled all cells in the game field
    return nTail >= (side * side) - 1;
}

int getPlayingTime() {
    time_t now = time(nullptr);
    double duration = difftime(now, startTime);
    return (int)duration;
}

int getGameMapSize() {
    initscr(); // Initialize ncurses
    keypad(stdscr, TRUE);
    echo(); // Echo input
    curs_set(true); // Show the cursor

    int size;
    printw("Enter map size.\n");
    printw("The number must be between 20 and 40 and a multiple of 2.\n");
    printw("Enter number: ");

    scanw((char *)"%d", &size);

    endwin(); // End ncurses

    if (size >= 20 && size <= 40 && size % 2 == 0) {
        return size;
    } else {
        return 20;
    }
}

GameMode SelectGameMode()
{
    initscr(); // Initialize ncurses
    keypad(stdscr, TRUE);
    echo(); // Echo input
    curs_set(true); // Show the cursor

    int choice;
    printw("Select Game Mode:\n");
    printw("1. With wall\n");
    printw("2. Without wall\n");
    printw("Mode with wall: Snake will die when collide the wall.\n");
    printw("Mode without wall: Snake will not die when collide the wall.\n");
    printw("Enter number: ");

    scanw((char *)"%d", &choice);

    endwin(); // End ncurses

    switch (choice)
    {
        case 1:
            return WITHWALL;
        case 2:
            return WITHOUTWALL;
        default:
            printw("Invalid choice. Defaulting to With wall mode.");
            return WITHWALL;
    }
}

GameType SelectGameType()
{
    initscr(); // Initialize ncurses
    keypad(stdscr, TRUE);
    echo(); // Echo input
    curs_set(true); // Show the cursor

    int choice;
    printw("Select Game Type:\n");
    printw("1. Endless\n");
    printw("2. 5 minutes\n");
    printw("3. 1000 scores\n");
    printw("First - until the field ends.\n");
    printw("Second - up to 5 minutes.\n");
    printw("Third - up to 1000 scores.\n");
    printw("Enter number: ");

    scanw((char *)"%d", &choice);

    endwin(); // End ncurses

    switch (choice)
    {
        case 1:
            return ENDLESS;
        case 2:
            return FIVEMINUTES;
        case 3:
            return THOUSANDSCORE;
        default:
            printw("Invalid choice. Defaulting to Endless type.");
            return ENDLESS;
    }
}

void gameMode() {
    // if snake goes outside from the map, game will be over
    if (modeGame == WITHWALL) {
        if (x > side || x < 1 || y < 1 || y > side) {
            gameOver = true;
        }
    }
    // if snake goes outside from the map, game will be continued
    if (modeGame == WITHOUTWALL) {
        if (x >= side)
            x = 0;
        else if (x <= 0)
            x = side - 1;

        if (y >= side)
            y = 0;
        else if (y <= 0)
            y = side - 1;
    }
}

void gameType()
{
    if (typeGame == FIVEMINUTES && getPlayingTime() >= 300) {
        gameOver = true;
    }
    if (typeGame == THOUSANDSCORE && score == 1000) {
        gameOver = true;
    }
}

void Setup() {
    //Shell Screen Init, functions from ncurses.h
    initscr();
    clear();
    noecho();
    cbreak();
    curs_set(false);

    typeGame = SelectGameType();
    refresh();
    modeGame = SelectGameMode();
    refresh();
    side = getGameMapSize();
    startTime = time(nullptr);
    gameOver = false;
    dir = STOP;
    x = side / 2; // start xpos
    y = side / 2; // start ypos
    fruitX = randomFruitPos();
    fruitY = randomFruitPos();  // Initialize fruit coordinates
    score = 0;

    start_color();
    init_pair(snakeColor, COLOR_GREEN, COLOR_BLACK);
    init_pair(fruitColor, COLOR_RED, COLOR_BLACK);
    init_pair(wallColor, COLOR_YELLOW, COLOR_BLACK);
    init_pair(scoreColor, COLOR_MAGENTA, COLOR_BLACK);
    init_pair(timeColor, COLOR_MAGENTA, COLOR_BLACK);
}


void Render() {
    clear();
    //Draw map, snake head, fruit, and tail
    for (int i = 0; i < side + 1; i++) {
        for (int j = 0; j < side + 1; j++) {
            //Draw map
            if (i == 0) {
                attron(COLOR_PAIR(wallColor));
                mvprintw(i, j, "%c", wallHorizontal); // Top
                attroff(COLOR_PAIR(wallColor));
            } else if (j == 0 || j == side) {
                attron(COLOR_PAIR(wallColor));
                mvprintw(i, j, "%c", wallVertical); // Left & right
                attroff(COLOR_PAIR(wallColor));
            } else if (i == side) {
                attron(COLOR_PAIR(wallColor));
                mvprintw(i, j, "%c", wallHorizontal); // Bottom
                attroff(COLOR_PAIR(wallColor));
            }
            //Draw snake head
            else if (i == y && j == x) {
                attron(COLOR_PAIR(snakeColor));
                mvprintw(i, j, "%c", snakeHead);
                attroff(COLOR_PAIR(snakeColor));
            }
            //Draw fruit
            else if (i == fruitY && j == fruitX) {
                attron(COLOR_PAIR(fruitColor));
                mvprintw(i, j, "%c", fruit);
                attroff(COLOR_PAIR(fruitColor));
            } else {
                //Draw Tail
                for (int k = 0; k < nTail; k++) {
                    if (tailX[k] == j && tailY[k] == i) {
                        attron(COLOR_PAIR(snakeColor));
                        mvprintw(i, j, "%c", snakeTail);
                        attroff(COLOR_PAIR(snakeColor));
                    }
                }
            }
        }
    }
    //Draw Information
    mvprintw(1, 49, "------------------------------------");
    mvprintw(2, 49, "|             Snake Game           |");
    mvprintw(3, 49, "------------------------------------");
    mvprintw(4, 49, "|");
    attron(COLOR_PAIR(scoreColor));
    mvprintw(4, 52, "Score: %d", score);
    attroff(COLOR_PAIR(scoreColor));
    mvprintw(4, 69, "|");
    attron(COLOR_PAIR(timeColor));
    mvprintw(4, 72, "Time: %d", getPlayingTime());
    attroff(COLOR_PAIR(timeColor));
    mvprintw(4, 84, "|");
    mvprintw(5, 49, "------------------------------------");
    mvprintw(6, 49, "Type: %s", (typeGame == ENDLESS) ? "endless." : (typeGame == FIVEMINUTES ? "5 minutes." : "1000 scores"));
    mvprintw(7, 49, "Mode: %s", (modeGame == WITHWALL) ? "with wall." : "without wall.");
    mvprintw(8, 49, "%s", (modeGame == WITHWALL) ? "(Snake will die when collide the wall)" : "(Snake will not die when collide the wall)");
    mvprintw(11, 49, "Use direction keys to eat fruit");
    mvprintw(13, 63, "   ^ ");
    mvprintw(14, 63, "<  v  >");

    refresh();
}

void Input() {
    keypad(stdscr, TRUE); // Check whether there is input, function from ncurses
    halfdelay(1);

    int c = getch(); // Save the typed key by gamer

    switch (c) {
        case KEY_LEFT:
            dir = LEFT;
            break;
        case KEY_RIGHT:
            dir = RIGHT;
            break;
        case KEY_DOWN: // definition is opposite between up and down
            dir = UP;
            break;
        case KEY_UP:
            dir = DOWN;
            break;
        case KEY_EXIT:
            gameOver = true;
            break;
    }

}

void Logic() {
    int prevX = tailX[0];
    int prevY = tailY[0];
    int prev2X, prev2Y;
    tailX[0] = x;
    tailY[0] = y;
    // add new position and other one copy
    for (int i = 1; i < nTail; i++) {
        prev2X = tailX[i];
        prev2Y = tailY[i];
        tailX[i] = prevX;
        tailY[i] = prevY;
        prevX = prev2X;
        prevY = prev2Y;
    }


    switch (dir) {
        case LEFT:
            x--;
            break;
        case RIGHT:
            x++;
            break;
        case UP:
            y++;
            break;
        case DOWN:
            y--;
            break;
        default:
            break;
    }

    gameMode();
    gameType();
    // if snake eats the fruit on the map, score will increase
    if (x == fruitX && y == fruitY) {
        score += 10;
        do {
            fruitX = randomFruitPos() - 1;
            fruitY = randomFruitPos() - 1;
        } while (IsOccupied(fruitX, fruitY));
        nTail++;
    }

    if (IsSnakeFilled()) {
        gameOver = true;
    }

    for (int i = 0; i < nTail; i++) {
        if (tailX[i] == x && tailY[i] == y) {
            gameOver = true;
        }
    }
}

int main(void) {
    system("osascript -e 'tell application \"Terminal\" to set bounds of window 1 to {100, 100, 800, 600}'");
    Setup();
    curs_set(false);
    while (!gameOver) {
        Render();
        Input();
        Logic();
    }
    getch();
    sleep(1);
    endwin();

    return 0;
}
