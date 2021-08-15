#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
using namespace std;
// Please use GB 2312 to encode.
// Please use GB 2312 to encode.
// Please use GB 2312 to encode.
// 开始开发时间：2021年8月13日23：13
// 开发结束时间：2021年8月14日23：09
// 控制台-----俄罗斯方块
// 写好必要注释
// 开发日志：
// 8.14 0:35 写好了开始菜单的动效
// 8.14白天有事没写一点东西
// 8.14 23:06：能玩了，还有很多体验需要优化
// 8.15 20:29：修复了bug，并增加了Esc暂停菜单

#define max(a, b) a > b ? a : b

enum screenData{
    SCREENWIDTH = 16,
    SCREENHEIGHT = 26,
};

// 游戏状态
enum gameState{
    CLEANING = 114,
    DROPPING = 514,
    NORMAL = 233
};

enum renderBufferWard{
    DRAWING = 0,
    SHOWING
};

// 掉落的图案
struct Drop{
    int id;
    int width, height;
    char *texture;
    int state;
};

Drop *dropList;
int dropListSiz;

int score;                              // 分数
int bestScore;                          // 最佳分数
char bestPlayer[100];                   // 最秀玩家
int dropType;                           // 下落类型
int dropX, dropY;                       // 下落物坐标
int timeDelay;                          // 延迟
enum gameState state;                   // 游戏状态
const int NORMALTIMEDELAY = 180;        // 延迟的默认
const char BLOCK = '#';                 // 方块贴图
const char AIR = ' ';                   // 空白
const int UP = 0;
const int RIGHT = 1;
const int DOWN = 2;
const int LEFT = 3;
BOOL blocks[SCREENWIDTH][SCREENHEIGHT];
BOOL gameOver, gameExit;
char renderBuffer[2][SCREENWIDTH][SCREENHEIGHT];        // 双缓冲

void init();
void dropInit();
void begin();                           // 启动（进入开始菜单）
void gameStart();                       // 游戏启动
void render();                          // 渲染器
void renderFresh();                     // 刷新渲染器的两个缓冲
void renderDraw();                      // 绘制
void keyEvent();                        // 处理按键事件
void update();                          // 更新游戏数据
void rotate();                          // 旋转掉落物
bool judgeMovable(int x, int y);        // 判断是否可以移动到某个地方

void unblockedSleep(unsigned int);      // 伪非阻塞响应输入
void gotoxy(short, short);
void slowlyPrint(char*);

int main(){
    begin();
    return 0;
}

/////////////////////////////////////////////////////

void gotoxy(short x, short y){
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleCursorPosition(h, {x, y});
}

void unblockedSleep(unsigned int _time){
    int timeCount = _time;
    while (_kbhit() == 0 && timeCount >= 0){
        Sleep(10);
        timeCount -= 10;
    }
}

void slowlyPrint(char *text){
    for (int i = 0; i < strlen(text); ++i){
        putchar(text[i]);
        Sleep(70);
    }
}

/////////////////////////////////////////////////////

void init(){
    srand((unsigned)time(NULL));
    dropInit();
    for (int i = 0; i < SCREENWIDTH; ++i)
        for (int j = 0; j < SCREENHEIGHT; ++j)
            blocks[i][j] = FALSE;
    score = 0;
    dropType = -1;
    timeDelay = NORMALTIMEDELAY;
    dropY = 0;
    dropX = SCREENWIDTH >> 1;
    gameOver = FALSE;
    gameExit = FALSE;
}

// push into dropList
void pdl(int _id, int _width, int _height, char *_texture, int _state){
    dropList[++dropListSiz].width = _width;
    dropList[dropListSiz].height = _height;
    dropList[dropListSiz].id = _id;
    dropList[dropListSiz].texture = _texture;
    dropList[dropListSiz].state = _state;
}

void dropInit(){
    dropListSiz = -1;
    dropList = (Drop*)malloc(sizeof(Drop) * 5 * 4);
    //  # 
    // ###
    pdl(1, 3, 2, " # ###", UP);
    //  #
    // ##
    //  #
    pdl(1, 2, 3, " ### #", RIGHT);
    // ###
    //  # 
    pdl(1, 3, 2, "### # ", DOWN);
    // # 
    // ##
    // #
    pdl(1, 2, 3, "# ### ", LEFT);
/////////////////////////////////////
    // ##
    // #
    // #
    pdl(2, 2, 3, "### # ", UP);
    // ###
    //   #
    pdl(2, 3, 2, "###  #", RIGHT);
    //  #
    //  #
    // ##
    pdl(2, 2, 3, " # ###", DOWN);
    // #
    // ###
    pdl(2, 3, 2, "#  ###", LEFT);
/////////////////////////////////////
    // ##
    //  #
    //  #
    pdl(3, 2, 3, "## # #", UP);
    //   #
    // ###
    pdl(3, 3, 2, "  ####", RIGHT);
    // #
    // #
    // ##
    pdl(3, 2, 3, "# # ##", DOWN);
    // ###
    // #
    pdl(3, 3, 2, "####  ", LEFT);
/////////////////////////////////////
    // ##
    // ##
    pdl(4, 2, 2, "####", UP);
    pdl(4, 2, 2, "####", RIGHT);
    pdl(4, 2, 2, "####", DOWN);
    pdl(4, 2, 2, "####", LEFT);
/////////////////////////////////////
    // ####
    pdl(5, 4, 1, "####", UP);
    pdl(5, 1, 4, "####", RIGHT);
    pdl(5, 4, 1, "####", DOWN);
    pdl(5, 1, 4, "####", LEFT);
}

// 本来想做像
//  >Start<
// 这样的动画的，但是鸽了
int makeChoice(char* _choice[], int _amo, char *title = NULL){
    if (title != NULL){
        slowlyPrint(title);
        putchar('\n');
    }
    for (int i = 0; i < _amo; ++i)
        printf("   %s\n", _choice[i]);
    int choice = 0;
    unsigned long long ani = 0;
    char key;
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO pos;
    GetConsoleScreenBufferInfo(h, &pos);
    pos.dwCursorPosition.Y;
    while (1){
        if (_kbhit() == 1){
            gotoxy(0, choice + pos.dwCursorPosition.Y - _amo);
            printf("   ");
            switch(tolower(_getch())){
            case 's':
                //choice = ++choice > _amo ? _amo - 1 : choice;
                if (++choice >= _amo)
                    choice = _amo - 1;
                break;
            case 'w':
                //choice = --choice < 0 ? 0 : choice;
                if (--choice < 0)
                    choice = 0;
                break;
            case '\r':
                // clean test of options and the title
                if (title != NULL){
                    gotoxy(0, pos.dwCursorPosition.Y - _amo - 1);
                    for (int j = 0; j < strlen(title); ++j)
                        putchar(' ');
                }
                for (int i = 0; i < _amo; ++i){
                    gotoxy(0, pos.dwCursorPosition.Y - _amo + i);
                    printf("   ");
                    for (int j = 0; j < strlen(_choice[i]); ++j)
                        putchar(' ');
                }
                gotoxy(0, pos.dwCursorPosition.Y - _amo);
                return choice;
                break;
            }
            ani = 0;
        }
        ani = ++ani % 2;
        gotoxy(0, choice + pos.dwCursorPosition.Y - _amo);
        if (ani == 0)
            printf(" ->");
        if (ani == 1)
            printf("-> ");
        gotoxy(0, pos.dwCursorPosition.Y + 1);
        unblockedSleep(160);
    }
    return choice;
}

void begin(){
    system("title Console Tetris");
    while (1){
        printf("<------------Console Tetris------------>\n");
        if (bestScore != 0)
            printf("Best %s with %d!\n", bestPlayer, bestScore);
        printf("Press W or S to change yr choice!\n");
        printf("Press ENTER to make yr choice!\n");
        
        char *choice[4];
        choice[0] = "Start!";
        choice[1] = "About";
        choice[2] = "Rules";
        choice[3] = "Exit";
        switch(makeChoice(choice, 4)){
            case 0:
                gameStart();
                system("cls");
                break;
            case 1:
                slowlyPrint("This bug is made by JensenTsTs\n");
                slowlyPrint("Just for fun.\n\n");
                break;
            case 2:
                slowlyPrint("You know the rules, and so do I~~~\n");
                slowlyPrint("Press W to rotate.\n");
                slowlyPrint("Press S to fall faster.\n");
                slowlyPrint("Press A or D to move horizontally.\n");
                slowlyPrint("Press ESC to suspend the game.\n");
                break;
            default:
                printf("\nSee you next time.\n(Press any key to continue...)\n");
                _getch();
                exit(0);
        }
    }
}

void gameStart(){
    system("cls");
    init();
    while(gameOver != TRUE && gameExit != TRUE){
        keyEvent();
        update();
        unblockedSleep(timeDelay);
        renderDraw();
        render();
    }
    if (gameOver == TRUE)
        slowlyPrint("W A S T E D!\n");
    if (gameExit == TRUE)
        slowlyPrint("Exited.\n");
    slowlyPrint("Your score: ");
    printf("%d\n", score);
    if (score > bestScore){
        slowlyPrint("It\'s the latest best score!!!\n");
        slowlyPrint("Write down your name: ");
        scanf("%s", bestPlayer);
        bestScore = score;
        printf("\n");
    }
    else if (bestScore != 0){
        slowlyPrint("Once the best score is ");
        printf("%d", bestScore);
        slowlyPrint(", made by ");
        slowlyPrint(bestPlayer);
        slowlyPrint(".\n");
    }
    slowlyPrint("Press any key to continue...");
    _getch();
}

void render(){
    for (int i = 0; i < SCREENHEIGHT; ++i)
        for (int j = 0; j < SCREENWIDTH; ++j)
            if (renderBuffer[DRAWING][j][i] != renderBuffer[SHOWING][j][i]){
                gotoxy(j, i);
                putchar(renderBuffer[DRAWING][j][i]);
            }
    gotoxy(0, SCREENHEIGHT);
    printf("score: %d             \n", score);
    renderFresh();
}

void renderFresh(){
    for (int i = 0; i < SCREENHEIGHT; ++i)
        for (int j = 0; j < SCREENWIDTH; ++j)
            renderBuffer[SHOWING][j][i] = renderBuffer[DRAWING][j][i];
}

void renderDraw(){
    for (int i = 0; i < SCREENHEIGHT; ++i)
        for (int j = 0; j < SCREENWIDTH; ++j)
            renderBuffer[DRAWING][j][i] = blocks[j][i] == TRUE ? BLOCK : AIR;
    if (dropType != -1)
        for (int i = 0; i < strlen(dropList[dropType].texture); ++i)
            renderBuffer[DRAWING][dropX + (i % dropList[dropType].width)][dropY + (i / dropList[dropType].width)] = dropList[dropType].texture[i] == BLOCK ? BLOCK : blocks[dropX + (i % dropList[dropType].width)][dropY + i / dropList[dropType].width] == TRUE ? BLOCK : AIR;
}

void keyEvent(){
    timeDelay = NORMALTIMEDELAY;
    if (dropType == -1)
        return;
    if (_kbhit()){
        switch(tolower(_getch())){
            case 'w':
                rotate();
                break;
            case 's':
                timeDelay >>= 1;
                break;
            case 'a':
                if (judgeMovable(dropX - 1, dropY))
                    dropX--;
                break;
            case 'd':
                if (judgeMovable(dropX + 1, dropY))
                    dropX++;
                break;
            case 27:
                char *opt[2];
                opt[0] = "Back to game.";
                opt[1] = "Exit.";
                if (makeChoice(opt, 2, "<--Game Suspended-->") == 1)
                    gameExit = TRUE;
                break;
        }
    }
}

// 就bool了，懒得改
bool judgeMovable(int x, int y){
    /*printf("%s ensuring...\n", dropList[dropType].texture);*/
    if (x + dropList[dropType].width - 1 >= SCREENWIDTH || x < 0 || y + dropList[dropType].height - 1 >= SCREENHEIGHT || y < 0)
        return false;
    size_t siz = strlen(dropList[dropType].texture);
    for (int i = 0; i < siz; ++i){
        int _x = i % dropList[dropType].width, _y = i / dropList[dropType].width;
        if (blocks[_x + x][_y + y] == TRUE && dropList[dropType].texture[i] == BLOCK)
            return false;
    }
    return true;
}

void update(){
    static BOOL cleanInNeed[SCREENHEIGHT] = {FALSE};
    if (state == CLEANING){
        // clean
        for (int i = 0; i < SCREENHEIGHT; ++i){
            BOOL needCleaned = TRUE;
            for (int j = 0; j < SCREENWIDTH; ++j){
                if (blocks[j][i] == FALSE){
                    needCleaned = FALSE;
                    break;
                }
            }
            if (needCleaned == TRUE){
                for (int j = 0; j < SCREENWIDTH; ++j)
                    blocks[j][i] = FALSE;
                score += SCREENWIDTH * 10;      // 乘10才有得分感（滑稽
                cleanInNeed[i] = TRUE;
            }
        }
        // upd state
        state = DROPPING;
        return;
    }
    if (state == DROPPING){
        // move
        int movLoc = SCREENHEIGHT - 1;                         // movLoc: 下次移动应该移动到哪一行
        for (int i = SCREENHEIGHT - 1; i >= 0; --i){
            // If cleaed, skip this line.
            if (cleanInNeed[i] == TRUE){
                cleanInNeed[i] = FALSE;
                continue;
            }
            if (movLoc != i){
                for (int j = 0; j < SCREENWIDTH; ++j){
                    blocks[j][movLoc] = blocks[j][i];
                    blocks[j][i] = FALSE;
                }
            }
            --movLoc;
        }
        // upd state
        state = NORMAL;
        return;
    }
    if (dropType == -1){
        //dropType = rand() % dropListSiz;
        dropType = 16;       // For debug
        dropY = 0;
        dropX = SCREENWIDTH >> 1;
        if (!judgeMovable(dropX, dropY))
            gameOver = TRUE;
        return;
    }
    // 如果可以下落
    if (judgeMovable(dropX, dropY + 1)){
        dropY++;
    }
    else{
        for (int i = 0; i < strlen(dropList[dropType].texture); ++i)
            blocks[dropX + (i % dropList[dropType].width)][dropY + (i / dropList[dropType].width)] = dropList[dropType].texture[i] == BLOCK ? TRUE : blocks[dropX + (i % dropList[dropType].width)][dropY + (i / dropList[dropType].width)];
        dropType = -1;
        for (int i = 0; i < SCREENHEIGHT; ++i){
            int account = 0;
            for (int j = 0; j < SCREENWIDTH; ++j)
                if (blocks[j][i] == FALSE)
                    break;
                else
                    account++;
            if (account == SCREENWIDTH){
                state = CLEANING;
                return;
            }
        }
    }
}

void rotate(){
    int _dropType = dropType;
    int id = dropList[dropType].id;
    int dropState = (dropList[dropType].state + 1) % 4;
    for (int i = 0; i <= dropListSiz; ++i){
        if (dropList[i].id == id && dropState == dropList[i].state){
            dropType = i;
            break;
        }
    }
    if (!judgeMovable(dropX, dropY))
        dropType = _dropType;
}
