#pragma once
#include <cstdint>

constexpr std::uintptr_t ChartAdress = 0x1074F860;
constexpr std::uintptr_t HardModeAdress = 0xF50A6C;
constexpr std::uintptr_t ScoreAdress = 0x11C5E60;
constexpr std::uintptr_t ResultAdress = 0x122C104;

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
	int32_t LastCombo; //11C5E64 might be last combo before end
	int32_t MaxCombo; //11C5E68
	int32_t Bonus; // 11C5E60
	int32_t NoteCount; //11C5E70
	int32_t MissCount; //11C5E74
	int32_t PoorCount; //11C5E78
	int32_t GoodCount; //11C5E7C
	int32_t GreatCount; //11C5E80
	int32_t PerfectCount; //11C5E84
};

struct ResultData {
	int32_t ChartId; //122C104
	int32_t Score; //122C108
	int32_t Bonus; //122C10C
	int32_t Clear; //122C110
};


const char* Difficulty[]
{
	"Basic",
	"Advanced",
	"Extreme"
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
	"Failed",
	"Cleared",
	"Full Combo"
};