//
// Created by Owner on 2024/02/06.
//

#ifndef NEWDIRECTORY_CAMERA_H
#define NEWDIRECTORY_CAMERA_H


#include <cstdint>

class camera {

public:
    static void Main(int *position, const int32_t *actions, const std::vector<std::pair<int, int>> &pairs);

private:
    static void onFreeCameraMove(int *position, int action);


};


#endif //NEWDIRECTORY_CAMERA_H
