//
// Created by Owner on 2024/02/05.
//

#ifndef NEWDIRECTORY_BATTLEEMULATOR_H
#define NEWDIRECTORY_BATTLEEMULATOR_H


#include <cstdint>
#include <optional>
#include "Player.h"
#include "BattleResult.h"

class BattleEmulator {
public:
    static const int TYPE_2A = 0;
    static const int TYPE_2B = 1;
    static const int TYPE_2C = 2;
    static const int TYPE_2E = 3;
    static const int TYPE_2D = 4;


    //0は配列の初期であり、永久欠番とする

    //2A
    static const int ATTACK_ENEMY = 1;
    static const int ULTRA_HIGH_SPEED_COMBO = 2;
    static const int SWITCH_2B = 3;
    static const int SWITCH_2C = 4;
    static const int LIGHTNING_STORM = 5;
    static const int CRITICAL_ATTACK = 6;

    //2C
    static const int SKY_ATTACK = 8;
    static const int MERA_ZOMA = 9;//kafrizzらしい
    static const int FREEZING_BLIZZARD = 10;//ここえるふぶき
    static const int SWITCH_2A = 11;
    static const int LULLAB_EYE = 12;//あやしいひとみ
    static const int SWITCH_2E = 13;


    //2B
    static const int LAUGH = 15;
    static const int DISRUPTIVE_WAVE = 16;//凍てつく波動
    static const int BURNING_BREATH = 17;//やけつくいき
    static const int DARK_BREATH = 18;//黒い霧
    static const int SWITCH_2D = 19;

    static const int INACTIVE_ENEMY = 21;
    static const int INACTIVE_ALLY = 22;
    static const int MEDICINAL_HERBS = 23;
    static const int PARALYSIS = 24;
    static const int ATTACK_ALLY = 25;
    static const int HEAL = 26;
    static const int DEFENCE = 27;
    static const int CURE_PARALYSIS = 28;


    static const int BUFF = 30;
    static const int MAGIC_MIRROR = 31;
    static const int MORE_HEAL = 32;//ベホイム
    static const int DOUBLE_UP = 33;//すてみ
    static const int MULTITHRUST = 34;//さみだれ
    static const int SLEEPING = 35;
    static const int MIDHEAL = 36;//ベホイミ
    static const int FULLHEAL = 37;//ベホマ
    static const int DEFENDING_CHAMPION = 38;//大防御

    //2D
    static const int PSYCHE_UP = 39;//ためる(敵)

    static const int CURE_SLEEPING = 40;//表示用

    //2E
    static const int MEDITATION = 41;//瞑想
    static const int MAGIC_BURST = 42;//マダンテ
    static const int RESTORE_MP = 43;//いのり
    static const int MERCURIAL_THRUST = 44;//しっぷう突き
    static const int THUNDER_THRUST = 45;//一閃突き
    static const int TURN_SKIPPED = 46;//スキップしたとき


    static const int SAGE_ELIXIR = 47;//賢者の聖水
    static const int ELFIN_ELIXIR = 48;//エルフののみぐすり
    static const int MAGIC_WATER = 49;//まほうのせいすい
    static const int SPECIAL_MEDICINE = 50;//特やくそう
    static const int DEAD1 = 51;//ゴスペルソング
    static const int GOSPEL_SONG = 52;//ゴスペルソング


    static bool
    Main(int *position, int RunCount, const int32_t Gene[350], Player *players,
         std::optional<BattleResult> &result, uint64_t seed, const int eActions[350],const int damages[350], int mode, uint64_t *NowState);

    static std::string getActionName(int actionId);
    static int roundCustom(double value);
private:
    static int ProcessMagicBurst(int * position);
    static void ProcessRage(int * position, int baseDamage, Player players[2]);
    static void RecalculateBuff(Player players[2]);
    static int CalculateMoreHealBase(Player players[2]);
    static int CalculateMidHealBase(Player players[2]);
    static int FUN_0208aecc(int *position, uint64_t * NowState);
    static void resetCombo(uint64_t * NowState);
    static double processCombo(int32_t Id, double damage, uint64_t * NowState);
    static int FUN_0207564c(int *position, int atk, int def);

    static int FUN_021e8458_typeC(int *position, double min, double max, double base);

    static int FUN_021e8458_typeD(int *position, double difference, double base);

    static int callAttackFun(int32_t Id, int *position, Player *players, int attacker, int defender, uint64_t * NowState);

    static double FUN_021dbc04(int baseHp, double maxHp);

    static int ProcessEnemyRandomAction2A(int *position);

    static int ProcessEnemyRandomAction44(int *position);
    static void process7A8(int * position,int baseDamage,Player players[2], int defender);
};


#endif //NEWDIRECTORY_BATTLEEMULATOR_H
