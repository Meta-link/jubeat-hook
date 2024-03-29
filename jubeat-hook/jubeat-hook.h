#pragma once
#include <cstdint>

enum Version
{
	jubeat,
	ripples,
	knit,
	copious,
	saucer,
	prop,
	qubell,
	clan,
	festo
};

struct Adresses {
	char Datecode[11];
	uintptr_t ChartAdress;
	uintptr_t HardModeAdress;
	uintptr_t ScoreAdress;
	uintptr_t ResultAdress;
	uintptr_t CardAdress;
	Version Version;
	uintptr_t DatecodeAdress; //Only used for omnimix detection, 0x0 if there is none
};

struct Adresses GameAdresses[] = {
	//Qubell
	{"2017041500", 0x9BD9B44, 0x9BA9527, 0x9B9D880, 0x9BC02F4, 0x9BAAE90, qubell, 0x0},
	{"2017062001", 0x9BDDC0C, 0x9BAD597, 0x9BA18B0, 0x9BC43BC, 0x9BAEF00, qubell, 0x0},
	//clan
	{"2018070901", 0xBDF8B6C, 0xBDC0F5F, 0xBDB4E18, 0xBDDF5B4, 0xBDC2730, clan, 0x0},
	//festo
	{"2018112702", 0x1074F860, 0xF50A6C, 0x11C5E60, 0x122C104, 0xF579C0, festo, 0x0},
	{"2022052400", 0x107752C0, 0xF93084, 0x120D190, 0x124EC64, 0xF9A6C0, festo, 0xE928DB},
};

const int32_t ScoreSize = 0xA0;
const int32_t ResultSize = 0x2A0;

struct ChartData {
	int32_t ChartId; //1074F860
	int8_t ChartDifficulty; //1074F864
	int8_t ChartLevel; //1074F865
	int8_t ChartDecimal; //1074F866
};

struct HardModeData {
	int32_t HardMode; //0xF50A6C
};

struct ScoreData {
	int32_t Score; //11C5E60
	int32_t LastCombo; //11C5E64
	int32_t MaxCombo; //11C5E68
	int32_t Bonus; // 11C5E6C
	int32_t NoteCount; //11C5E70
	int32_t MissCount; //11C5E74
	int32_t PoorCount; //11C5E78
	int32_t GoodCount; //11C5E7C
	int32_t GreatCount; //11C5E80
	int32_t PerfectCount; //11C5E84
	uint8_t padding[ScoreSize - 10 * sizeof(int32_t)];
};

struct Scores {
	ScoreData ScoresData[3];
};

struct ResultData {
	int32_t ChartId; //122C104
	int32_t Score; //122C108
	int32_t Bonus; //122C10C
	int32_t Clear; //122C110
	uint8_t padding[ResultSize - 4 * sizeof(int32_t)];
};

struct Results {
	ResultData ResultsData[3];
};

struct AdditionalData {
	int ScoreTotal;
	int Rating;
	int ClearType;
	double MusicRate;
};

struct CardData {
	char CardTag[17]; //F579C0
	uint8_t padding0[24];
	char CardID[17]; //F579E9
	uint8_t padding1[58];
	char CardHolder[9]; //F57A34
};

struct DatecodeData {
	char Datecode[21]; //E928DB
};

const char* Difficulty[]
{
	"Basic",
	"Advanced",
	"Extreme"
};

const char* DifficultyTag[]
{
	"BSC",
	"ADV",
	"EXT"
};

const char* Rating[]
{
	"EXC",
	"SSS",
	"SS",
	"S",
	"A",
	"B",
	"C",
	"D",
	"E"
};

const char* ClearType[]
{
	"FAILED",
	"CLEAR",
	"FULL COMBO",
	"EXCELLENT"
};

const char* VersionName[]
{
	"jubeat",
	"ripples",
	"knit",
	"copious",
	"saucer",
	"prop",
	"qubell",
	"clan",
	"festo"
};