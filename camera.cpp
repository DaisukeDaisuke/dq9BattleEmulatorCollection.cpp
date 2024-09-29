//
// Created by Owner on 2024/02/06.
//

#include <vector>
#include "camera.h"
#include "BattleEmulator.h"
#include "lcg.h"

int counter = 0;

bool BeforePreemptive = false;



void camera::Main(int * position, const int32_t actions[5], const std::vector<std::pair<int, int>>& pairs){
    int32_t before = -1;
    bool preemptive = true;
    for (int i = 0; i < 2; ++i) {
        //std::cout << "counter1: " << counter << std::endl;
        int32_t after = actions[i];
        if (after == BattleEmulator::ATTACK_ALLY){
            if (preemptive){
                if (!BeforePreemptive){
                    (*position) += 2;
                }else{
                    BeforePreemptive = false;
                    (*position) += 3;
                }
            }else{
                BeforePreemptive = true;
                (*position)++;
            }
            //onFreeCameraMove(position, after, preemptive ? 1 : 0);
        }else if(after == BattleEmulator::ATTACK_ENEMY){
            (*position)++;
        }
        preemptive = false;
        before = after;
        //std::cout << "counter2: " << counter << std::endl;
    }
}

void camera::onFreeCameraMove(int * position, const int action, const int param5){
    if (param5 == 0){
        (*position)++;
        if (counter == 0){
            counter++;
            return;
        }
        auto ret = lcg::getPercent(position, 5 - counter);
        if(ret == 0||counter == 5){
            counter = 0;
            (*position) += 1;
        }else{
            counter++;
        }
    }else{
        (*position)++;
        if (counter == 0) {
            (*position)++;//引数5が1なら強制的に実行
            return;
        }
        (*position)++;
//        auto ret = lcg::getPercent(position, 5 - counter);
//        if (ret != 0){
//            ret = 1;
//        }else{
//            ret = 1;
//        }
        counter = 0;
//        if (ret == 0 || counter == 5) {
//            counter = 0;
//            //(*position)++;
//        }
        (*position)++;
    }
}

void camera::reset() {
    BeforePreemptive = false;
}
