//
// Created by Owner on 2024/02/06.
//

#include "camera.h"
#include "BattleEmulator.h"
#include "lcg.h"

int counter = 0;

void camera::Main(int * position, const int32_t actions[5]){
    int32_t before = -1;
    for (int i = 0; i < 5; ++i) {
        int32_t after = actions[i];
        if (after == BattleEmulator::FIRE_BLOWING_ART){
            if (before == BattleEmulator::MERA){
                onFreeCameraMove(position, after);
            }
            if (before == BattleEmulator::BOLT_CUTTER||before == BattleEmulator::MULTITHRUST){
                (*position)++;
            }else{
                (*position)++;
            }
        }else if (
                (
                    before == BattleEmulator::FIRE_BLOWING_ART||
                    before == BattleEmulator::MERA||
                    before == BattleEmulator::BOLT_CUTTER||
                    before == BattleEmulator::MEDICINAL_HERBS
                )
            &&after == BattleEmulator::MERA
        ){
            onFreeCameraMove(position, after);
        }else if(after == BattleEmulator::MEDICINAL_HERBS){

        }
        before = after;
    }
}

void camera::onFreeCameraMove(int * position, const int action){
    (*position)++;
    if (counter == 0){
        counter++;
        return;
    }
    auto ret = lcg::getPercent(position, 5 - counter);
    if(ret == 0||counter == 5){
        counter = 0;
        (*position) += 2;
        if (action == BattleEmulator::FIRE_BLOWING_ART){
            (*position)++;
        }
    }else{
        counter++;
    }
}