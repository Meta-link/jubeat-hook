#pragma pack(1)
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
Scores* scores = nullptr;
Results* results = nullptr;

AdditionalData additionalData;
int currentSong = 0;


string URLbase = "";
string URLstatus = "";
string URLimport = "";

string APIKEY = "";

void chartDump()
{
    printf("=== SONG %i ===", currentSong);
    printf("Song ID =           %i\n", chart->ChartId);
    printf("Song difficulty =   %s\n", Difficulty[chart->ChartDifficulty]);
    printf("Song level =        %i.%i\n", chart->ChartLevel, chart->ChartDecimal);
    printf("Hard mode =         %s\n\n", hardMode->HardMode ? "yes" : "no");
}

void ScoreDump(int trackNumber = 0)
{
    printf("Score =             %i\n", scores->ScoresData[currentSong].Score);
    printf("Last combo =        %i\n", scores->ScoresData[currentSong].LastCombo);
    printf("Max combo =         %i\n", scores->ScoresData[currentSong].MaxCombo);
    printf("Bonus =             %i\n", scores->ScoresData[currentSong].Bonus);
    printf("Total note count =  %i\n", scores->ScoresData[currentSong].NoteCount);
    printf("Miss count =        %i\n", scores->ScoresData[currentSong].MissCount);
    printf("Poor count =        %i\n", scores->ScoresData[currentSong].PoorCount);
    printf("Good count =        %i\n", scores->ScoresData[currentSong].GoodCount);
    printf("Great count =       %i\n", scores->ScoresData[currentSong].GreatCount);
    printf("Perfect count =     %i\n\n", scores->ScoresData[currentSong].PerfectCount);

    printf("Result score =      %i\n", results->ResultsData[currentSong].Score);
    printf("Result bonus =      %i\n", results->ResultsData[currentSong].Bonus);
    printf("Result clear =      %i\n\n", results->ResultsData[currentSong].Clear);

    additionalData.ScoreTotal = results->ResultsData[currentSong].Score + results->ResultsData[currentSong].Bonus;
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
        else if (scores->ScoresData[currentSong].MaxCombo == scores->ScoresData[currentSong].NoteCount)
            additionalData.ClearType = 2;
        else
            additionalData.ClearType = 1;
    }
    printf("CLEAR =             %s\n", ClearType[additionalData.ClearType]);

    additionalData.MusicRate = ((scores->ScoresData[currentSong].PerfectCount + 0.2 * scores->ScoresData[currentSong].GreatCount + 0.05 * scores->ScoresData[currentSong].GoodCount) / scores->ScoresData[currentSong].NoteCount) * (hardMode->HardMode ? 120 : 100);
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
                            {"miss", scores->ScoresData[currentSong].MissCount},
                            {"poor", scores->ScoresData[currentSong].PoorCount},
                            {"good", scores->ScoresData[currentSong].GoodCount},
                            {"great", scores->ScoresData[currentSong].GreatCount},
                            {"perfect", scores->ScoresData[currentSong].PerfectCount},
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
    scores = (Scores*)(jubeatAdress + ScoreAdress);
    results = (Results*)(jubeatAdress + ResultAdress);

    cpr::Response rk = cpr::Get(cpr::Url{ URLbase + URLstatus });
    cout << "[jubeat hook] Checking connection to Kamaitachi : " << rk.url << endl;
    cout << "[jubeat hook] Kamaitachi status : " << rk.status_code << endl;

    do {
        if(results->ResultsData[currentSong].Clear != 0)
        {
            chartDump();
            ScoreDump();
            SendScore();
            currentSong++;
            if (currentSong > 2)
                currentSong = 0;
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
