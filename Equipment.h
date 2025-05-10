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
    const std::string_view name;
    std::array<int, static_cast<int>(Attribute::AttributeCount)> resistances;

    // C++20ではconstexprのループも利用可能
    constexpr Equipment(std::string_view inputName, std::initializer_list<std::pair<Attribute, int> > initList)
        : name(inputName), resistances{} {
        for (auto entry: initList) {
            resistances[static_cast<int>(entry.first)] = entry.second;
        }
    }
};

// 装備の初期化
//まほうのたて
constexpr Equipment DarkShield("Magic shield", {
                                   {Attribute::Fire, 5},
                                   {Attribute::Ice, 5},
                                   {Attribute::Wind, 5},
                                   {Attribute::ThunderExplosion, 5},
                                   {Attribute::Darkness, 5},
                               });

//せいれいのよろい
constexpr Equipment EtherealArmour("Enchanted shield", {
                                       {Attribute::Fire, 15},
                                       {Attribute::Ice, 15},
                                       {Attribute::Wind, 15},
                                       {Attribute::ThunderExplosion, 15},
                                       {Attribute::Darkness, 12},
                                   });

//せいれいのこて
constexpr Equipment EnchantedGloves("Enchanted gloves", {
                                        {Attribute::Fire, 7},
                                        {Attribute::Ice, 7},
                                        {Attribute::Wind, 7},
                                        {Attribute::ThunderExplosion, 7},
                                        {Attribute::Darkness, 7},
                                    });

// 同様に、配列も inline 化
constexpr std::array<Equipment, 3> allEquipments = {
    DarkShield, EtherealArmour, EnchantedGloves
};

constexpr int PrivateCalculateTotalResistance(Attribute attribute) {
    int totalResistance = 100;
    for (const auto &equipment: allEquipments) {
        totalResistance -= equipment.resistances[static_cast<size_t>(attribute)];
    }
    return totalResistance;
}

class Equipments {
private:
    // クラス外の関数を利用して配列を初期化
    inline static constexpr std::array<int, static_cast<size_t>(Attribute::AttributeCount)> resistances = []() constexpr {
        std::array<int, static_cast<size_t>(Attribute::AttributeCount)> res{};
        for (size_t i = 0; i < res.size(); ++i) {
            res[i] = PrivateCalculateTotalResistance(static_cast<Attribute>(i));
        }
        return res;
    }();

public:
    // ダメージ軽減用の関数。
    // 闇属性の場合のみ、倍率 0.73 (整数として 73, 100倍して保存) を用いて軽減処理を行います。
    // コンパイル時定数展開が可能な constexpr 関数です。
    static constexpr int applyDamageReduction(const int damage, const Attribute attr) {
        int multiplier = calculateTotalResistance(attr);
        int scaledDamage = damage * 100;
        return (scaledDamage * multiplier) / 10000;
    }


    // resistances 配列から対象の属性に対応する値を返す関数
    inline static constexpr int calculateTotalResistance(const Attribute attribute) {
        return resistances[static_cast<size_t>(attribute)];
    }
};


#endif //NEWDIRECTORY_EQUIPMENT_H
