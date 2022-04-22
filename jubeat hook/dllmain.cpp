#include "pch.h"
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include "jubeathook.h"

#include <openssl/ssl.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using namespace std;
using json = nlohmann::json;
using namespace std::chrono;

std::uintptr_t jubeatAdress = 0;
ChartData* chart = nullptr;
HardModeData* hardMode = nullptr;
ScoreData* score = nullptr;
ResultData* result = nullptr;
AdditionalData additionalData;

string URLbase = "";
string URLstatus = "";
string URLimport = "";

string APIKEY = "";

void chartDump()
{
    printf("Song ID =           %i\n", chart->ChartId);
    printf("Song difficulty =   %s\n", Difficulty[chart->ChartDifficulty]);
    printf("Song level =        %i.%i\n", chart->ChartLevel, chart->ChartDecimal);
    printf("Hard mode =         %s\n\n", hardMode->HardMode ? "yes" : "no");
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

    additionalData.ScoreTotal = result->Score + result->Bonus;
    printf("TOTAL SCORE =       %i\n", additionalData.ScoreTotal);

    additionalData.Rating = 0;
    if (additionalData.ScoreTotal <= 499999)
        additionalData.Rating = 8;
    else if (additionalData.ScoreTotal <= 699999)
        additionalData.Rating = 7;
    else if (additionalData.ScoreTotal <= 799999)
        additionalData.Rating = 6;
    else if (additionalData.ScoreTotal <= 849999)
        additionalData.Rating = 5;
    else if (additionalData.ScoreTotal <= 899999)
        additionalData.Rating = 4;
    else if (additionalData.ScoreTotal <= 949999)
        additionalData.Rating = 3;
    else if (additionalData.ScoreTotal <= 979999)
        additionalData.Rating = 2;
    else if (additionalData.ScoreTotal <= 999999)
        additionalData.Rating = 1;
    printf("RATING =            %s\n", Rating[additionalData.Rating]);

    additionalData.ClearType = 0;
    if (additionalData.ScoreTotal >= 700000)
    {
        if (additionalData.ScoreTotal == 1000000)
            additionalData.ClearType = 3;
        else if (score->MaxCombo == score->NoteCount)
            additionalData.ClearType = 2;
        else
            additionalData.ClearType = 1;
    }
    printf("CLEAR =             %s\n", ClearType[additionalData.ClearType]);

    additionalData.MusicRate = ((score->PerfectCount + 0.2 * score->GreatCount + 0.05 * score->GoodCount) / score->NoteCount) * (hardMode->HardMode ? 120 : 100);
    printf("MUSIC RATE =        %f\n\n", additionalData.MusicRate);
}

void SendScore()
{
    json outData = {
        {
            "meta",
            {
                {"service", "jubeat hook"},
                {"game", "jubeat"},
                {"playtype", "Single"}
            }
        },
        {
            "scores",
            {
                {
                    {"score", additionalData.ScoreTotal},
                    {"lamp", ClearType[additionalData.ClearType]},
                    {"percent", additionalData.MusicRate},
                    {"matchType", "inGameID"},
                    {"identifier", to_string(chart->ChartId)},
                    {"difficulty", string(hardMode->HardMode ? "HARD " : "") + string(DifficultyTag[chart->ChartDifficulty])},
                    {"timeAchieved", duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count()},
                    {"judgements",
                        {
                            {"miss", score->MissCount},
                            {"poor", score->PoorCount},
                            {"good", score->GoodCount},
                            {"great", score->GreatCount},
                            {"perfect", score->PerfectCount},
                        }
                    },
                }
            }   
        }
    };
    cout << "[jubeat hook] Sending score to kamaitachi" << endl;
    cout << outData.dump(4) << endl;
    cpr::Response r = cpr::Post(cpr::Url{ URLimport},
                                cpr::Timeout(4000),
                                cpr::Header{ {"Authorization", "Bearer " + APIKEY}, {"Content-Type", "application/json"} },
                                cpr::Body{ outData.dump()});
    cout << "[jubeat hook] Score sent, response code : " << r.status_code << endl;
    cout << "[jubeat hook] Response text : " << r.text << endl;
}

void WriteScore()
{
    cout << "[jubeat hook] Writing score in file" << endl;

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

    cpr::Response rk = cpr::Get(cpr::Url{ URLbase + URLstatus });
    cout << "[jubeat hook] Checking connection to Kamaitachi : " << rk.url << endl;
    cout << "[jubeat hook] Kamaitachi status : " << rk.status_code << endl;

    bool dumped = false;

    do {
        if(!dumped && result->Clear != 0)
        {
            chartDump();
            ScoreDump();
            SendScore();
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
