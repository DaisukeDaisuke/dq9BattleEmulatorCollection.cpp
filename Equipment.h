#ifndef NEWDIRECTORY_EQUIPMENT_H
#define NEWDIRECTORY_EQUIPMENT_H

#include <iostream>
#include <string>
#include <array>
#include <utility>
#include <initializer_list>

enum class Attribute {
    Fire,
    Ice,
    ThunderExplosion,
    Wind,
    Darkness,
    Earth,
    Light,
    AttributeCount // 属性の数
};

struct Equipment {
    std::string name;
    std::array<int, static_cast<int>(Attribute::AttributeCount)> resistances{};

    // コンストラクタで特定の属性のみ設定可能に
    Equipment(std::string inputName, std::initializer_list<std::pair<Attribute, int>> initList)
            : name(std::move(inputName)) {
        resistances.fill(0); // 全ての耐性を0に初期化
        for (const auto& entry : initList) {
			resistances[static_cast<int>(entry.first)] = entry.second; // 指定された属性の耐性を設定
        }
    }
};

// 装備の初期化
//ダークシールド
// グローバル変数を inline 変数として定義
inline const Equipment DarkShield("Dark shield", {
        {Attribute::Fire, 5},
        {Attribute::Ice, 5}
});

//げんまのよろい
inline const Equipment EtherealArmour("Ethereal armour", {
        {Attribute::Fire, 18},
        {Attribute::Ice, 18},
        {Attribute::Wind, 18},
        {Attribute::ThunderExplosion, 18},
        {Attribute::Darkness, 15},
});

//せいれいのこて
inline const Equipment EnchantedGloves("Enchanted gloves", {
        {Attribute::Fire, 7},
        {Attribute::Ice, 7},
        {Attribute::Wind, 7},
        {Attribute::ThunderExplosion, 7},
        {Attribute::Darkness, 7},
});

//竜戦士のブーツ
inline const Equipment DragonWarriorBoots("Dragon warrior boots", {
        {Attribute::Fire, 5},
        {Attribute::Darkness, 5}
});

// 同様に、配列も inline 化
inline const std::array<Equipment, 4> allEquipments = {
        DarkShield, EtherealArmour, EnchantedGloves, DragonWarriorBoots
};

class Equipments {
public:
    static double calculateTotalResistance(Attribute attribute);
};

// 指定された属性に基づいて合計耐性を計算し、0.01で掛けた結果を返す関数
double Equipments::calculateTotalResistance(Attribute attribute) {
    int totalResistance = 100;
    for (const auto& equipment : allEquipments) {
        totalResistance -= equipment.resistances[static_cast<int>(attribute)];
    }
    return totalResistance * 0.01; // 結果を0.01で掛けて返す
}

#endif //NEWDIRECTORY_EQUIPMENT_H
