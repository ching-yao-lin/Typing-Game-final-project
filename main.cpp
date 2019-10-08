#include <iostream>
#include <ctime>
#include <conio.h>
#include <windows.h>
#include "utilwin32.h"
#include <cstdlib>
#include <fstream>
#include <stdlib.h>
#include <iomanip>
#include <omp.h> // #pragma omp parallel for

#include <stdio.h>
#include <thread>
#include <tchar.h>

using namespace std;

struct Vocab
{
    int * color; // 該letter沒有變色，等於0；有變色，等於1。
    char* letter;
};

/// NEW ///
//顯示時間
class Clock
{
public:
    Clock(){};
    void SetTime(int newH = 0, int newM = 0, int newS = 0);
    void ShowTime();
    void Run();          //Run()控制计时
private:
    int hour, minute, second;
};


/// ================== DECLARATION OF GLOBAL VARIABLES ================== ///
const int MAX_WORDS = 1001;  // 從單字庫抽出的單字數
const int MAX_WORD_LEN = 20; // 單字的最長長杜
const int WORDS_DROPPING = 99; // 一輪總共要掉幾個字
int width = 50, height = 25, length = 10; // 單字的活動範圍
int randPosition = 0, orgX = 0, orgY = 0;
int mode = 0; //選擇單字庫
int score = 0; //分數
int healthPoints = 3; //HP
int restart = 0; //是否重新開始
int highest = 0; //歷史最高分
/// ================== DECLARATION OF GLOBAL FUNCTIONS ================== ///
void drop(int x, int y, int indexOfWordPrinted, double velocity, Vocab* vocabs, int numOfVocabs);
void printSpace(int numOfSpace);
void printEndl(int num);
void colorChange(Vocab* vocabs, int numOfVocabs, char ch); // 根據輸入，將第一個未變色字母變色
void printInColor(Vocab droppingVocabs); // 印出一個單字
void printInColor(Vocab* droppingVocabs, int numOfVocabs); // 根據 "color不為0的字母要變色" 的規則，印出該單字
void eraseVocabsIfNeeded(Vocab droppingVocabs, bool& wordDisappear); // erase一個單字
void eraseVocabsIfNeeded(Vocab* droppingVocabs, int numOfVocabs, bool& wordDisappear); // 當單字全部變色，清空該單字
void colorPlate(); // 可以用來顯示調色盤("數字" 對應 "字體顏色")

/// NEW ///
int DisplayConfirmSaveAsMessageBox() //顯示訊息
{
     int msgboxID = MessageBox(
         NULL,
         "Game Over!!!!!\nDo you want to replay?",
         "Replay",
        MB_ICONEXCLAMATION | MB_YESNO
     );

     if (msgboxID == IDYES)
     {
         restart = 1;
     }
     else
     {
         system("pause");
         cin.get();
     }

    return msgboxID;
}

/// NEW ///
void Clock::SetTime(int newH, int newM, int newS) //设置时间
{
    hour = newH;
    minute = newM;
    second = newS;
}
void Clock::ShowTime()        //显示时间，在显示时间前进行判断，如果时间设置不合适，则提示错误
{
    if (hour > 24 || hour<0 || minute>60 || minute<0 || second>60 || second < 0)
    {
        cout << "输入有误！" << endl;
        exit(0);
    }
    else
    {
        cout << setw(2) << setfill('0') << hour << ":" << setw(2) << setfill('0')   //<<setw(2)<<setfill('0')设置域宽为2 不够的话用字符‘0’填充
            << minute << ":" << setw(2) << setfill('0') << second << endl;
    }
}
/// NEW ///
void Clock::Run()  //实现计时功能
{

    while (1)
    {
        second += 1;
        if (second >= 60)
        {
            second -= 60;
            minute += 1;

        }
        if (minute >= 60)
        {
            minute -= 60;
            hour += 1;
        }
        if (hour >= 24)
        {
            hour -= 24;
        }
        system("cls");
        ShowTime();
        Sleep(1000);
    }
}

void runClock()
{
    Clock myClock;
    myClock.SetTime(00,00,00);
    myClock.ShowTime();
    myClock.Run();
}

void gotoOrixy (int x, int y)
{

    COORD coord;
    coord.X = x;
    coord.Y = y;

    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE),coord);
}


int RandX(int ran) // 隨機選出一個水平位置
{
	int randPosition = 0;
	srand( time(NULL) );//固定亂數種子
	randPosition = rand() % (width - length + 1);
	return randPosition;
}
void SetColor(int color = 7) // 預設：黑底白字
{
	HANDLE hConsole;
	hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleTextAttribute(hConsole,color);
}
/*
void SetColor(int f = 7, int b = 0)
{
    unsigned short ForeColor = f + 16 * b;
    HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hCon,ForeColor);
}
*/

/// ======================== MAIN FUNCTION ======================== ///
int main(){


    // 基本設定初始化
	begin:
    restart = 0;
    healthPoints = 3;
    score = 0;

    system("cls");

    system("color F0"); // 白底黑字

	//colorPlate(); // 查看調色盤

    /// NEW ///
    //讀取最高分
    ifstream file("highest.txt");
    if(file.is_open())
    {
        file >> highest;
        file.close();

    }
	//選擇介面
    string title = _T("TYPING GAME");
    SetConsoleTitle(title.c_str());

	cout << endl << endl << endl << endl << endl << endl;
	cout << "                   TYPING GAME" << endl << endl;
    cout << "                   1 Easy" <<endl;
    cout << "                   2 Intermediate" <<endl;
    cout << "                   3 Hard" <<endl;
	cout << "                   Please choose the difficulty level:";
	cin >> mode;
	cout << endl << endl;
	cout << "                   Game Start!!!" << endl;

	Sleep(1000);
    system("CLS");

    //計時器
    thread runClock();
    //runClock.join();

    //選擇第一個單字庫
	if(mode == 1 || mode == 2 || mode == 3)
    {
        // 動態記憶體配置 & 初始化
        Vocab* vocabs = new Vocab[MAX_WORDS];
        for(int i = 0; i < MAX_WORDS; i++)
            vocabs[i].letter = new char[MAX_WORD_LEN];
        for(int i = 0; i < MAX_WORDS; i++)
        {
            vocabs[i].color = new int[MAX_WORD_LEN];
            for(int j = 0; j < MAX_WORD_LEN; j++)
                vocabs[i].color[j] = 0;
        }
        // 動態記憶體配置 & 初始化
        Vocab* droppingVocabs = new Vocab[MAX_WORDS];
        for(int i = 0; i < MAX_WORDS; i++)
            droppingVocabs[i].letter = new char[MAX_WORD_LEN];
        for(int i = 0; i < MAX_WORDS; i++)
        {
            droppingVocabs[i].color = new int[MAX_WORD_LEN];
            for(int j = 0; j < MAX_WORD_LEN; j++)
                droppingVocabs[i].color[j] = 0;
        }
        // 動態記憶體配置 & 初始化
        char** c = new char*[MAX_WORDS];
        for(int i = 0; i < MAX_WORDS; i++)
            c[i] = new char[MAX_WORD_LEN];

        /*
        cout << "< test >: " << endl;
        for(int i = 0; i < MAX_WORDS; i++)
            cout << "vocabs[" << i << "].letter = " << vocabs[i].letter << endl;
        for(int i = 0; i < MAX_WORDS; i++)
        {
            cout << "vocabs[" << i << "].color = ";
            for(int j = 0; j < MAX_WORD_LEN; j++)
                 cout << vocabs[i].color[j] << " ";
            cout << endl;
        }
        */

        // 從檔案匯入單字庫
        ifstream file("file.txt");
        const int TOTAL_VOCAB_NUM = MAX_WORDS - 1;
        int i = 0;
        if(file.is_open())
        {
            for(i = 1; i <= TOTAL_VOCAB_NUM; ++i)
            {
                file >> vocabs[i].letter; // 物件vocabs被依序填滿單字
            }
        }
        else
            cout << "Error in reading files" << endl;
        /*
        for (i = 1 ; i <= TOTAL_VOCAB_NUM ; i++)
            cout << vocabs[i].letter << "\n" ;
        */

        // 隨機從單字庫抽出單字
        int n = 1;
        int x = 0, y = 0;
        double velocity;
        if(mode==1) velocity = 0;
        else if(mode == 2) velocity = 160;
        else velocity = 240;
        int randomNum = 0;
        int numOfVocabs = WORDS_DROPPING;
        srand( unsigned(time(0)) );
        //cin >> n;



        while(1)
        {

            for (i = 0 ; i < MAX_WORDS; i++)
            {
                /// NEW ///
                SetColor();
                gotoOrixy(64,0);
                cout << std::right << setw(5) << "Score:" << " " << score <<"\n";
                gotoOrixy(64,1);
                cout << std::right << setw(7) << "Health Points:" << " " << healthPoints <<endl;
                gotoOrixy(64,2);
                cout << std::right << setw(7) << "Highest Score:" << " " << highest <<endl;

                printEndl(height - 3);
                SetColor(64);
                cout << "----------------------------------------------------------------";
                SetColor();

                if(restart == 1)
                {
                    goto begin;
                }
                randomNum = (rand() % 1000); // generate random number from 0 ~ 999
                //cout << randomNum << " ";
                strcpy(droppingVocabs[i].letter, vocabs[randomNum].letter); // 從物件vocabs隨機挑出一個單字，共挑MAX_WORDS個
                //cout << vocabs[randomNum].letter << "\n";
                //cout << "c[" << i << "] = " << c[i] << "\n";

                if (velocity < 400) velocity = velocity + 40; //漸漸加速
                else if (velocity == 400) velocity = 400;

                drop(x, y, i, velocity, droppingVocabs, numOfVocabs);

                /// NEW ///
                SetColor();
                gotoOrixy(64,0);
                cout << std::right << setw(5) << "Score:" << " " << score <<"\n";
                gotoOrixy(64,1);
                cout << std::right << setw(7) << "Health Points:" << " " << healthPoints <<endl;
                gotoOrixy(64,2);
                cout << std::right << setw(7) << "Highest Score:" << " " << highest <<endl;

                fstream in;
                in.open("highest.txt", ios::out);
                if(score > highest)
                {
                    //cout << "highest";
                    in << score;
                    in.close();
                }
            }

        }

        /** 林璟耀 - 不知道該放哪裡的CODE
        cout << "Type any letter to change colors: " << endl;
        for(;1;)
        {
            char ch;
            ch = getch();
            cout << "Entering >> " << ch << " : ";
            colorChange(vocabs, numOfVocabs, ch);
            cout << endl;
            eraseVocabsIfNeeded(vocabs, numOfVocabs);
        }
        **/

    }


	system("PAUSE");
	return 0 ;
}
/// ======================== GLOBAL FUNCTIONS ======================== ///
void drop(int x, int y, int indexOfWordPrinted, double velocity, Vocab* droppingVocabs, int numOfVocabs)
{
    /*
    double velocity = 0;
    if (velocity < 400) velocity = velocity + 40; //漸漸加速
    else if (velocity == 400) velocity = 400;
    */

	int ran = 0;
	x = RandX(ran) + 1;
	gotoxy(x,orgY); // 游標移到隨機的x座標

	int freq = numOfVocabs;
    bool wordDisappear = false;

    for(y = orgY; y <= height; y++) // 讓一個單字從(y座標=0)掉到(y座標=height)
    {
        /// NEW ///
        //if碰到底 命-1
        if(y == height)
        {
            healthPoints -= 1;
        }
        if(healthPoints == 0)
        {
            DisplayConfirmSaveAsMessageBox();
        }
        while (_kbhit())//如果有按键按下，则_kbhit()函数返回真
        {

            char ch = _getch();
            //gotoxy(x-5,y), cout<<"!!!"<<endl;
            SetColor(255);

            gotoxy(x-strlen(droppingVocabs[indexOfWordPrinted].letter),y), printSpace(strlen(droppingVocabs[indexOfWordPrinted].letter) + 2);//使用_getch()函数获取按下的键值
            colorChange(droppingVocabs, numOfVocabs, ch);
            SetColor(255);
            gotoxy(x,y), printSpace(strlen(droppingVocabs[indexOfWordPrinted].letter) + 2);
            SetColor();
            //gotoxy(x,y), printInColor(droppingVocabs[indexOfWordPrinted]);
            if (ch == 27){ break; }//当按下ESC时循环，ESC键的键值时27.
            eraseVocabsIfNeeded(droppingVocabs[indexOfWordPrinted], wordDisappear);
            if(wordDisappear == true)
            {
                SetColor(255);
                gotoxy(x,y+1), printSpace(strlen(droppingVocabs[indexOfWordPrinted].letter) + 2);
                SetColor();

                //cout << "EXIT drop FUNCTION" << endl;
                return;
            }

        }

        delay(500 - velocity); // 每次移動之間間隔 0.5 秒 (500ms), 不過input的velocity會越來越大、直到400會固定
        SetColor(255);
        gotoxy(x,y), printSpace(strlen(droppingVocabs[indexOfWordPrinted].letter) + 2); // 移動到下一個座標前先清除原來的文字

        while (_kbhit())//如果有按键按下，则_kbhit()函数返回真
        {
            char ch = _getch();
            //gotoxy(x-5,y), cout<<"!!!"<<endl;
            SetColor(255);

            gotoxy(x-strlen(droppingVocabs[indexOfWordPrinted].letter),y), printSpace(strlen(droppingVocabs[indexOfWordPrinted].letter) + 2);//使用_getch()函数获取按下的键值
            colorChange(droppingVocabs, numOfVocabs, ch);
            SetColor(255);
            gotoxy(x,y), printSpace(strlen(droppingVocabs[indexOfWordPrinted].letter) + 2);
            SetColor();
            //gotoxy(x,y), printInColor(droppingVocabs[indexOfWordPrinted]);
            if (ch == 27){ break; }//当按下ESC时循环，ESC键的键值时27.
            eraseVocabsIfNeeded(droppingVocabs[indexOfWordPrinted], wordDisappear);
            if(wordDisappear == true)
            {
                SetColor(255);
                gotoxy(x,y+1), printSpace(strlen(droppingVocabs[indexOfWordPrinted].letter) + 2);
                SetColor();
                //cout << "EXIT drop FUNCTION" << endl;
                return;
            }

        }
        SetColor(); // 恢復原本的顏色(預設：黑底白字)
        gotoxy(x, y + 1), printInColor(droppingVocabs[indexOfWordPrinted]);
        //cout << "(x,y): " << x << "," << y << " ";
        //cout << "len = " << strlen(droppingVocabs[indexOfWordPrinted].letter);
        SetColor(255);

        //gotoxy(x,y), printSpace(strlen(droppingVocabs[indexOfWordPrinted].letter) + 2); // 移動到下一個座標前先清除原來的文字

        while (_kbhit())//如果有按键按下，则_kbhit()函数返回真
        {
            char ch = _getch();
            //gotoxy(x-5,y), cout<<"!!!"<<endl;
            SetColor(255);

            gotoxy(x-strlen(droppingVocabs[indexOfWordPrinted].letter),y), printSpace(strlen(droppingVocabs[indexOfWordPrinted].letter) + 2);//使用_getch()函数获取按下的键值
            colorChange(droppingVocabs, numOfVocabs, ch);
            SetColor(255);
            gotoxy(x,y), printSpace(strlen(droppingVocabs[indexOfWordPrinted].letter) + 2);
            SetColor();
            //gotoxy(x,y), printInColor(droppingVocabs[indexOfWordPrinted]);
            if (ch == 27){ break; }//当按下ESC时循环，ESC键的键值时27.
            eraseVocabsIfNeeded(droppingVocabs[indexOfWordPrinted], wordDisappear);
            if(wordDisappear == true)
            {
                SetColor(255);
                gotoxy(x,y+1), printSpace(strlen(droppingVocabs[indexOfWordPrinted].letter) + 2);
                SetColor();
                SetColor(255);

                //cout << "EXIT drop FUNCTION" << endl;
                return;
            }

        }

    }
        SetColor(255);
        gotoxy(x,y), printSpace(strlen(droppingVocabs[indexOfWordPrinted].letter) + 2);


}
void printSpace(int numOfSpace)
{
    for(int i = 0; i < numOfSpace; i++)
        cout << " ";
        /// NEW ///
            SetColor();
                gotoOrixy(64,0);
                cout << std::right << setw(5) << "Score:" << " " << score <<"\n";
                gotoOrixy(64,1);
                cout << std::right << setw(7) << "Health Points:" << " " << healthPoints <<endl;
                gotoOrixy(64,2);
                cout << std::right << setw(7) << "Highest Score:" << " " << highest <<endl;
}
void printEndl(int num)
{
    for(int i = 0; i < num; i++)
        cout << endl;
}
void colorChange(Vocab* droppingVocabs, int numOfVocabs, char ch)
{
    for(int i = 0; i < numOfVocabs; i++)
    {
        ///Find the first color-unchanged letter for each vocab, check if it's ch.
        for(int j = 0; j < strlen(droppingVocabs[i].letter); j++)
        {
            if(droppingVocabs[i].color[j] == 0) // the first unchanged letter
            {
                if(droppingVocabs[i].letter[j] == ch) // this letter matches the input char
                {
                    droppingVocabs[i].color[j] = 1; // mark the color as 1, meaning "to be changed"
                }
                break;
            }
        }
    }
    // test:
    //printInColor(droppingVocabs, numOfVocabs);

}
void printInColor(Vocab droppingVocabs)
{
    int j = 0;
    while(droppingVocabs.color[j] != 0)
    {
        HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(hConsole, 224);
        cout << droppingVocabs.letter[j];
        j++;
    }
    SetColor();

    while(j <= strlen(droppingVocabs.letter))
    {
        cout << droppingVocabs.letter[j];
        j++;
    }
}
void printInColor(Vocab* droppingVocabs, int numOfVocabs)
{

    for(int i = 0; i < numOfVocabs; i++)
    {
        int j = 0;
        while(droppingVocabs[i].color[j] != 0)
        {
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, 224);
            cout << droppingVocabs[i].letter[j];
            j++;
        }
        SetColor();

        while(j <= strlen(droppingVocabs[i].letter))
        {
            cout << droppingVocabs[i].letter[j];
            j++;
        }
    }
}
void eraseVocabsIfNeeded(Vocab droppingVocabs, bool& wordDisappear)
{
    int sum = 0;
    for(int i = 0; i < MAX_WORD_LEN; i++)
        sum += droppingVocabs.color[i];

    if(sum == strlen(droppingVocabs.letter)) // 全部變色
        {
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, 4);
            //cout << "======== Erase " << droppingVocabs[i].letter << " !!!========" << endl;
            SetColor();
            //delete[] droppingVocabs[i].letter;
            strcpy(droppingVocabs.letter, "*ERASED*");
            for(int j = 0; j < MAX_WORD_LEN; j++)
            {
                droppingVocabs.color[j] = 0;
            }

            wordDisappear = true;

        }

        if(wordDisappear == true) // if there's a word disappeared, reset all colors
        {
            for(int j = 0; j < MAX_WORD_LEN; j++)
                droppingVocabs.color[j] = 0; /// reset all colors
            /// 記分板 + 1 (宜婕)
            score += 1;

            //cout << "EXIT eraseVocabsIfNeeded FUNCTION." << endl;
            return;
        }
}
void eraseVocabsIfNeeded(Vocab* droppingVocabs, int numOfVocabs, bool& wordDisappear)
{
    /// Calculate the sum of color array for each vocab
    int* sum = new int[numOfVocabs];
    for(int i = 0; i < numOfVocabs; i++)
    {
        sum[i] = 0;
        for(int j = 0; j < MAX_WORD_LEN; j++)
            sum[i] += droppingVocabs[i].color[j];
    }
    /*
    cout << "< test sum >:" << endl;
    for(int i = 0; i < numOfVocabs; i++)
        cout << "sum[" << i << "] = " << sum[i] << endl;

    cout << "< test strlen >:" << endl;
    for(int i = 0; i < numOfVocabs; i++)
        cout << "strlen(droppingVocabs[" << i << "].letter) = " << strlen(droppingVocabs[i].letter) << endl;
    */


    /// if the sum of color array == strlen(letter array of vocab), reset the memory
    wordDisappear = false;
    for(int i = 0; i < numOfVocabs; i++)
    {
        if(sum[i] == strlen(droppingVocabs[i].letter)) // 有字是全部變色的
        {
            HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
            SetConsoleTextAttribute(hConsole, 4);
            //cout << "======== Erase " << droppingVocabs[i].letter << " !!!========" << endl;
            SetColor();
            //delete[] droppingVocabs[i].letter;
            strcpy(droppingVocabs[i].letter, "*ERASED*");
            for(int j = 0; j < MAX_WORD_LEN; j++)
            {
                droppingVocabs[i].color[j] = 0;
            }

            wordDisappear = true;
        }

        if(wordDisappear == true) // if there's a word disappeared, reset all colors
        {
            for(int j = 0; j < MAX_WORD_LEN; j++)
                droppingVocabs[i].color[j] = 0; /// reset all colors
            /// 記分板 + 1 (宜婕)
            score += 1;

            return;
        }
    }

}
void colorPlate() // 可以用來顯示調色盤("數字" 對應 "字體顏色")
{
    cout << "     ===================================== COLOR PLATE ===================================== " << endl;
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    // you can loop k higher to see more color choices
    cout << "||";
    for(int k = 1; k <= 255; k++)
    {
        // pick the colorattribute k you want
        SetConsoleTextAttribute(hConsole, k);
        cout <<"<" << setw(3)<< k <<">";
        if(k % 20 == 0)
        {
            SetColor();
            cout << "||" << endl << "||";
        }

    }
    SetColor();
    cout << endl;
}




//	clock_t sTime = clock();
//	for(i = 0; i < 10000000; i++){
//	}
//	clock_t eTime = clock();
//	cout << sTime << " " << eTime << " " << static_cast<double>(eTime - sTime) / CLOCKS_PER_SEC;
//	cout << " " << randPosition;
