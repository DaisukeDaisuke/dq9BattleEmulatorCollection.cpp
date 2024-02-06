//
// Created by Owner on 2024/02/06.
//

#ifndef NEWDIRECTORY_CAMERA_H
#define NEWDIRECTORY_CAMERA_H


#include <cstdint>

class camera {

    static void onFreeCameraMove(int *position);

public:
    static void Main(int *position, const uint32_t *actions);
};


#endif //NEWDIRECTORY_CAMERA_H
