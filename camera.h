//
// Created by Owner on 2024/02/06.
//

#ifndef NEWDIRECTORY_CAMERA_H
#define NEWDIRECTORY_CAMERA_H


#include <cstdint>

class camera {

public:
    static void Main(int *position, const int32_t *actions, int * NowState);

private:
    static void onFreeCameraMove(int *position, int action, const int param5, int * NowState);
};


#endif //NEWDIRECTORY_CAMERA_H
