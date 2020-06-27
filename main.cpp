#include <iostream>
#include <vector>
#include <algorithm>
#include <chrono>
using namespace std;
#include <stdio.h>
#include <Windows.h>
#include <math.h>
int nScreenWidth = 320;			// консоль
int nScreenHeight = 100;
int nMapWidth = 32;				// мир
int nMapHeight = 32;

float fPlayerX = 25.0f;
float fPlayerY = 15.0f;
float fPlayerA = 0.0f;
float fFOV = 3.14159f / 2.5f;	// поле зрения
float fDepth = 48.0f;			// дальность прорисовки
float fSpeed = 5.0f;			// скорость

int main()
{
    // буфер экрана
    wchar_t *screen = new wchar_t[nScreenWidth*nScreenHeight];
    HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL);
    SetConsoleActiveScreenBuffer(hConsole);
     DWORD dwBytesWritten = 0;
    // Мир: " "- пустота, "#"- стена
    wstring map;
    map += L"################################";
    map += L"#                #     #     # #";
    map += L"#                   #     #    #";
    map += L"#  #  #########  #     #     # #";
    map += L"#  #  #       #  #  #     #    #";
    map += L"#  #  #       #  #     #       #";
    map += L"#  #  #  #  ###  ############  #";
    map += L"#  #  #  #                     #";
    map += L"#  #  #  #                     #";
    map += L"#  #     #######################";
    map += L"#  #     #              #      #";
    map += L"#  #######              #      #";
    map += L"#             #  #####  #      #";
    map += L"#             #  #      #      #";
    map += L"###############  #      #      #";
    map += L"#       #     #  #  #########  #";
    map += L"#                #             #";
    map += L"#    #     #     #             #";
    map += L"##  ####################  ######";
    map += L"#                #             #";
    map += L"#                #             #";
    map += L"#                              #";
    map += L"#                              #";
    map += L"#                              #";
    map += L"#                              #";
    map += L"#                              #";
    map += L"#                              #";
    map += L"#                              #";
    map += L"#                              #";
    map += L"#                              #";
    map += L"################################";
    auto tp1 = chrono::system_clock::now();
    auto tp2 = chrono::system_clock::now();

    while (1)
    {

        tp2 = chrono::system_clock::now();
        chrono::duration<float> elapsedTime = tp2 - tp1;
        tp1 = tp2;
        float fElapsedTime = elapsedTime.count();


        // управление
        if (GetAsyncKeyState((unsigned short)'A') & 0x8000)
            fPlayerA -= (fSpeed * 0.5f) * fElapsedTime;

        if (GetAsyncKeyState((unsigned short)'D') & 0x8000)
            fPlayerA += (fSpeed * 0.5f) * fElapsedTime;
        // управление вперёд-назад с учётом стен
        if (GetAsyncKeyState((unsigned short)'W') & 0x8000)
        {
            fPlayerX += sinf(fPlayerA) * fSpeed * 0.6f * fElapsedTime;;
            fPlayerY += cosf(fPlayerA) * fSpeed * 0.6f * fElapsedTime;;
            if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
            {
                fPlayerX -= sinf(fPlayerA) * fSpeed * 0.6f * fElapsedTime;;
                fPlayerY -= cosf(fPlayerA) * fSpeed * 0.6f * fElapsedTime;;
            }
        }

        if (GetAsyncKeyState((unsigned short)'S') & 0x8000)
        {
            fPlayerX -= sinf(fPlayerA) * fSpeed * 0.5f * fElapsedTime;;
            fPlayerY -= cosf(fPlayerA) * fSpeed * 0.5f * fElapsedTime;;
            if (map.c_str()[(int)fPlayerX * nMapWidth + (int)fPlayerY] == '#')
            {
                fPlayerX += sinf(fPlayerA) * fSpeed * 0.5f * fElapsedTime;;
                fPlayerY += cosf(fPlayerA) * fSpeed * 0.5f * fElapsedTime;;
            }
                    }

                    for (int x = 0; x < nScreenWidth; x++)
                    {
                        // определение стен с учётом перспективы и дальности обзора
                        float fRayAngle = (fPlayerA - fFOV/2.0f) + ((float)x / (float)nScreenWidth) * fFOV;

                        float fStepSize = 0.1f;
                        float fDistanceToWall = 0.0f;

                        bool bHitWall = false;
                        bool bBoundary = false;

                        float fEyeX = sinf(fRayAngle);
                        float fEyeY = cosf(fRayAngle);

                        while (!bHitWall && fDistanceToWall < fDepth)
                        {
                            fDistanceToWall += fStepSize;
                            int nTestX = (int)(fPlayerX + fEyeX * fDistanceToWall);
                            int nTestY = (int)(fPlayerY + fEyeY * fDistanceToWall);

                            if (nTestX < 0 || nTestX >= nMapWidth || nTestY < 0 || nTestY >= nMapHeight)
                            {
                                bHitWall = true;
                                fDistanceToWall = fDepth;
                            }
                            else
                            {
                                //черчение рёбер припятствий для структурирования изображения
                                if (map.c_str()[nTestX * nMapWidth + nTestY] == '#')
                                {
                                    bHitWall = true;
                                    vector<pair<float, float>> p;

                                    for (int tx = 0; tx < 2; tx++)
                                        for (int ty = 0; ty < 2; ty++)
                                        {
                                            float vy = (float)nTestY + ty - fPlayerY;
                                            float vx = (float)nTestX + tx - fPlayerX;
                                            float d = sqrt(vx*vx + vy*vy);
                                            float dot = (fEyeX * vx / d) + (fEyeY * vy / d);
                                            p.push_back(make_pair(d, dot));
                                        }

                                    sort(p.begin(), p.end(), [](const pair<float, float> &left, const pair<float, float> &right) {return left.first < right.first; });

                                    // учёт толщены ребра
                                    float fBound = 0.002;
                                    if (acos(p.at(0).second) < fBound) bBoundary = true;
                                    if (acos(p.at(1).second) < fBound) bBoundary = true;
                                }
                            }
                        }

                        // расчёт ростояния от пола до потолка
                        int nCeiling = (float)(nScreenHeight/2.0) - nScreenHeight / ((float)fDistanceToWall);
                        int nFloor = nScreenHeight - nCeiling;

                        // наложение текстур для понимания перспективы
                        short nShade = ' ';
                        if (fDistanceToWall <= fDepth / 26.0f)			nShade = 0x2588;
                        else if (fDistanceToWall < fDepth / 19.0f)		nShade = 0x2590;
                        else if (fDistanceToWall < fDepth / 12.0f)		nShade = 0x2573;
                        else if (fDistanceToWall < fDepth / 8.0f)		nShade = 0x2572;
                        else if (fDistanceToWall < fDepth / 1.0f)		nShade = 0x2502;
                        else											nShade = ' ';

                        if (bBoundary)		nShade = '|';

                        for (int y = 0; y < nScreenHeight; y++)
                        {
                            if(y <= nCeiling)
                                screen[y*nScreenWidth + x] = ' ';
                            else if(y > nCeiling && y <= nFloor)
                                screen[y*nScreenWidth + x] = nShade;
                            else // пол
                            {
                                float b = 1.0f - (((float)y -nScreenHeight/2.0f) / ((float)nScreenHeight / 2.0f));
                                if (b < 0.18f)		nShade = 0x256C;
                                else if (b < 0.48f)	nShade = 0x2592;
                                else if (b < 0.65f)	nShade = 0x2591;
                                else if (b < 0.9f)	nShade = 0x2502;
                                else				nShade = ' ';
                                screen[y*nScreenWidth + x] = nShade;
                            }
                        }
                    }

                    // Статистика
                    swprintf_s(screen, 40, L"X=%3.2f, Y=%3.2f, A=%3.2f FPS=%3.2f ", fPlayerX, fPlayerY, fPlayerA, 1.0f/fElapsedTime);

                    // Мини-карта
                    for (int nx = 0; nx < nMapWidth; nx++)
                        for (int ny = 0; ny < nMapHeight; ny++)
                        {
                            screen[(ny+1)*nScreenWidth + nx] = map[ny * nMapWidth + nx];
                        }
                    screen[((int)fPlayerX+1) * nScreenWidth + (int)fPlayerY] = 'U';
                    // рамка
                    screen[nScreenWidth * nScreenHeight - 1] = '\0';
                     WriteConsoleOutputCharacter(hConsole, screen, nScreenWidth * nScreenHeight, { 0,0 }, &dwBytesWritten);

                } 
                return 0;
            }
