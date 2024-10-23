//
// Created by Owner on 2024/10/20.
//

#ifndef NEWDIRECTORY_EQUIPMENT_H
#define NEWDIRECTORY_EQUIPMENT_H

#include <iostream>
#include <string>

#include <iostream>
#include <string>
#include <array>
#include <utility>

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
    Equipment(std::string  name, std::initializer_list<std::pair<Attribute, int>> initList)
            : name(std::move(name)) {
        resistances.fill(0); // 全ての耐性を0に初期化
        for (const auto& entry : initList) {
            resistances[static_cast<int>(entry.first)] = entry.second;
        }
    }
};


// 装備の初期化
const Equipment DarkShield("ダークシールド", {
        {Attribute::Fire, 5},
        {Attribute::Ice, 5}
});

const Equipment EtherealArmour("げんまのよろい", {
        {Attribute::Fire, 18},
        {Attribute::Ice, 18},
        {Attribute::Wind, 18},
        {Attribute::ThunderExplosion, 18},
        {Attribute::Darkness, 15},
});

const Equipment EnchantedGloves("せいれいのこて", {
        {Attribute::Fire, 7},
        {Attribute::Ice, 7},
        {Attribute::Wind, 7},
        {Attribute::ThunderExplosion, 7},
        {Attribute::Darkness, 7},
});

const Equipment DragonWarriorBoots("竜戦士のブーツ", {
        {Attribute::Fire, 5},
        {Attribute::Darkness, 5}
});

const std::array<Equipment, 4> allEquipments = {
        DarkShield, EtherealArmour, EnchantedGloves, DragonWarriorBoots
};

class Equipments {

public:
    static double calculateTotalResistance(Attribute attribute);

};

// 指定された属性に基づいて合計耐性を計算し、0.01で掛けた結果を返す関数
double Equipments::calculateTotalResistance(Attribute attribute) {
    int totalResistance = 0;

    for (const auto& equipment : allEquipments) {
        totalResistance += equipment.resistances[static_cast<int>(attribute)];
    }

    return 1.0 - (totalResistance * 0.01); // 結果を0.01で掛けて返す
}



#endif //NEWDIRECTORY_EQUIPMENT_H
