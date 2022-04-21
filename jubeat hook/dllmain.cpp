#include "pch.h"
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include "jubeathook.h"

std::uintptr_t jubeatAdress = 0;
ChartData* chart = nullptr;
HardModeData* hardMode = nullptr;
ScoreData* score = nullptr;
ResultData* result = nullptr;


void chartDump()
{
    printf("Song ID =           %i\n", chart->ChartId);
    printf("Song difficulty =   %s\n", Difficulty[chart->ChartDifficulty]);
    printf("Song level =        %i.%i\n", chart->ChartLevel, chart->ChartDecimal);
    printf("Hard mode =         %s\n\n", hardMode->HardMode == 1 ? "yes" : "no");
}

void ScoreDump(int trackNumber = 0)
{
    printf("Score =             %i\n", score->Score);
    printf("Last combo =        %i\n", score->LastCombo);
    printf("Max combo =         %i\n", score->MaxCombo);
    printf("Bonus =             %i\n", score->Bonus);
    printf("Total note count =  %i\n", score->NoteCount);
    printf("Miss count =        %i\n", score->MissCount);
    printf("Poor count =        %i\n", score->PoorCount);
    printf("Good count =        %i\n", score->GoodCount);
    printf("Great count =       %i\n", score->GreatCount);
    printf("Perfect count =     %i\n\n", score->PerfectCount);

    printf("Result score =      %i\n", result->Score);
    printf("Result bonus =      %i\n", result->Bonus);
    printf("Result clear =      %i\n\n", result->Clear);

    int scoreTotal = result->Score + result->Bonus;
    printf("TOTAL SCORE =       %i\n", scoreTotal);

    int rating = 0;
    if (scoreTotal <= 499999)
        rating = 8;
    else if (scoreTotal <= 699999)
        rating = 7;
    else if (scoreTotal <= 799999)
        rating = 6;
    else if (scoreTotal <= 849999)
        rating = 5;
    else if (scoreTotal <= 899999)
        rating = 4;
    else if (scoreTotal <= 949999)
        rating = 3;
    else if (scoreTotal <= 979999)
        rating = 2;
    else if (scoreTotal <= 999999)
            rating = 1;
    printf("RATING =            %s\n", Rating[rating]);

    int clear = 0;
    if (scoreTotal >= 700000)
    {
        if (score->MaxCombo == score->NoteCount)
            clear = 2;
        else
            clear = 1;
    }
    printf("CLEAR =             %s\n", ClearType[clear]);

    float musicRate = ((score->PerfectCount + 0.2 * score->GreatCount + 0.05 * score->GoodCount) / score->NoteCount) * (hardMode->HardMode ? 120 : 100);
    printf("MUSIC RATE =        %f\n\n", musicRate);
}

DWORD WINAPI InitHook(LPVOID dllInstance)
{
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

    jubeatAdress = (std::uintptr_t)GetModuleHandleA("jubeat.dll");
    chart = (ChartData*)(jubeatAdress + ChartAdress);
    hardMode = (HardModeData*)(jubeatAdress + HardModeAdress);
    score = (ScoreData*)(jubeatAdress + ScoreAdress);
    result = (ResultData*)(jubeatAdress + ResultAdress);

    bool dumped = false;

    do {
        if(!dumped && result->Clear != 0)
        {
            chartDump();
            ScoreDump();
            dumped = true;
        }
        else if (GetAsyncKeyState(VK_F10))
            break;

    } while (true);

    printf("Detaching jubeat hook");

    FreeConsole();
    FreeLibraryAndExitThread((HMODULE)dllInstance, EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, NULL, InitHook, hModule, NULL, NULL);
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}
