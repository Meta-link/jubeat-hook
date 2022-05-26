#include "pch.h"
#include <cstdio>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <format>
#include <chrono>
#include <string>
#include "jubeat-hook.h"

#include <openssl/ssl.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <simpleini.h>
#include <tinyxml2.h>

using namespace std;
using json = nlohmann::json;
using namespace std::chrono;
using namespace tinyxml2;

std::uintptr_t jubeatAdress = 0;

ChartData* chart = nullptr;
HardModeData* hardMode = nullptr;
Scores* scores = nullptr;
Results* results = nullptr;
CardData* card = nullptr;

AdditionalData additionalData;
int currentSong = 0;
ofstream scoreFile;
json ScoreData;
json FileContentData;
json KamaiResponse;

int currentGameVersion = -1;
bool showDebug;
string playerID;
bool exportFile;
bool exportKamai;
string statusURL = "";
string importURL = "";
string apiKey = "";

void printDebug(auto str)
{
    if (showDebug)
        cout << "[jubeat-hook] " << str << endl;
}

void ChartDump()
{
    printf("     === TUNE %i ===\n", currentSong - 1);
    printf("Song ID =           %i\n", chart->ChartId);
    printf("Song difficulty =   %s\n", Difficulty[chart->ChartDifficulty]);
    printf("Song level =        %i.%i\n", chart->ChartLevel, chart->ChartDecimal);
    printf("Hard mode =         %s\n\n", hardMode->HardMode ? "yes" : "no");
}

void ScoreDump()
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

    printf("TOTAL SCORE =       %i\n", additionalData.ScoreTotal);
    printf("RATING =            %s\n", Rating[additionalData.Rating]);
    printf("CLEAR =             %s\n", ClearType[additionalData.ClearType]);
    printf("MUSIC RATE =        %f\n\n", additionalData.MusicRate);
}

void SetupJson()
{
    ScoreData = {
        {
            "meta",
            {
                {"service", "jubeat-hook"},
                {"game", "jubeat"},
                {"playtype", "Single"},
                {"version", VersionName[GameAdresses[currentGameVersion].Version]}
            }
        },
        {
            "scores",
            {

            }
        }
    };

    if (exportFile)
    {
        FileContentData = ScoreData;
    }
}

void AddScoreJson()
{
    ScoreData["scores"] =
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
    };

    if (exportFile)
    {
        FileContentData.push_back(ScoreData["scores"][0]);
    }
}

void ProcessScore()
{
    additionalData.ScoreTotal = results->ResultsData[currentSong].Score + results->ResultsData[currentSong].Bonus;

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

    additionalData.MusicRate = ((scores->ScoresData[currentSong].PerfectCount + 0.2 * scores->ScoresData[currentSong].GreatCount + 0.05 * scores->ScoresData[currentSong].GoodCount) / scores->ScoresData[currentSong].NoteCount) * (hardMode->HardMode ? 120 : 100);

    if (showDebug)
    {
        ChartDump();
        ScoreDump();
    }

    AddScoreJson();
    printDebug(ScoreData.dump(4));

    if (exportFile)
    {
        printDebug("Writing scores into file");
        scoreFile << ScoreData.dump(4) << endl;
    }

    if (exportKamai)
    {
        printDebug("Sending score to kamaitachi");
        cpr::Response r = cpr::Post(cpr::Url{ importURL },
            cpr::Timeout(4000),
            cpr::Header{ {"Authorization", "Bearer " + apiKey}, {"Content-Type", "application/json"} },
            cpr::Body{ ScoreData.dump() });
        printDebug("Response text : " + r.text);

        if (!r.text.empty())
        {
            printDebug("Checking kamaitachi import");
            try
            {
                KamaiResponse = json::parse(r.text);
                r = cpr::Get(cpr::Url{ KamaiResponse["body"]["url"] });

                KamaiResponse = json::parse(r.text);
                printDebug(KamaiResponse["description"]);

                if (!KamaiResponse["body"]["import"]["errors"].empty())
                {
                    printDebug("Error :");
                    printDebug(KamaiResponse["body"]["import"]["errors"][0]["message"]);
                }
            }
            catch (json::parse_error& e)
            {
                printDebug("Error parsing the json");
                printDebug(e.what());
            }
        }
    }

}

DWORD WINAPI InitHook(LPVOID dllInstance)
{
    AllocConsole();
    freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

    CSimpleIniA ini;
    SI_Error rc = ini.LoadFile("jubeat-hook.ini");
    if (rc < 0)
    {
        cout << "Error while loading jubeat-hook.ini, ending process ..." << endl;
        return EXIT_FAILURE;
    }
    showDebug = ini.GetBoolValue("general", "showDebug");
    playerID = ini.GetValue("general", "playerID");
    exportFile = ini.GetBoolValue("general", "exportFile");
    exportKamai = ini.GetBoolValue("kamaitachi", "exportKamai");
    statusURL = ini.GetValue("kamaitachi", "statusURL");
    importURL = ini.GetValue("kamaitachi", "importURL");
    apiKey = ini.GetValue("kamaitachi", "apiKey");

    printDebug("Starting hook");

    XMLDocument doc;
    doc.LoadFile("prop/ea3-config.xml");
    const char* dateCode = doc.FirstChildElement("ea3")->FirstChildElement("soft")->FirstChildElement("ext")->GetText();
    printDebug("Datecode : " + (string)dateCode);

    for (int i = 0; i < sizeof(GameAdresses) / sizeof(GameAdresses[0]); i++)
    {
        if (strcmp(dateCode, GameAdresses[i].Datecode) == 0)
        {
            currentGameVersion = i;
            printDebug("Supported version");
            break;
        }
    }

    if (currentGameVersion == -1)
    {
        printDebug("Unsupported version, exiting now");
    }
    else
    {
        SetupJson();

        if (exportFile)
        {
            const auto now = chrono::system_clock::now();
            string fileName = "scores_" + format("{:%d-%m-%Y_%H%M%OS}", now) + ".json";
            scoreFile.open(fileName);
            printDebug("Creating score file : " + fileName);
        }

        if (exportKamai)
        {
            cpr::Response rk = cpr::Get(cpr::Url{ statusURL });
            printDebug("Checking connection to Kamaitachi : " + (string)rk.url);
            printDebug("Kamaitachi status :");
            printDebug(rk.status_code);

            if (rk.status_code != 200)
            {
                printDebug("Kamaitachi has a problem, export disabled");
                exportKamai = false;
            }
        }

        jubeatAdress = (std::uintptr_t)GetModuleHandleA("jubeat.dll");
        chart = (ChartData*)(jubeatAdress + GameAdresses[currentGameVersion].ChartAdress);
        hardMode = (HardModeData*)(jubeatAdress + GameAdresses[currentGameVersion].HardModeAdress);
        scores = (Scores*)(jubeatAdress + GameAdresses[currentGameVersion].ScoreAdress);
        results = (Results*)(jubeatAdress + GameAdresses[currentGameVersion].ResultAdress);
        card = (CardData*)(jubeatAdress + GameAdresses[currentGameVersion].CardAdress);

        do {
            if (currentSong != 0 && results->ResultsData[0].Clear == 0) //New credit
            {
                printDebug("NEW CREDIT");
                currentSong = 0;
            }
            if (results->ResultsData[currentSong].Clear != 0)
            {
                if (playerID.empty() || (strcmp(card->CardID, playerID.c_str()) == 0 || strcmp(card->CardTag, playerID.c_str()) == 0))
                {
                    ProcessScore();
                }
                else printDebug("Score filtered and ignored");

                currentSong++;
            }
            else if (GetAsyncKeyState(VK_F10))
            {
                printDebug(card->CardID);
                printDebug(card->CardTag);
                printDebug(card->CardHolder);
            }
        } while (true);
    }

    if (exportFile)
        scoreFile.close();
    printDebug("Detaching hook");

    FreeConsole();
    FreeLibraryAndExitThread((HMODULE)dllInstance, EXIT_SUCCESS);

    return EXIT_SUCCESS;
}

BOOL APIENTRY DllMain(HMODULE hModule,
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