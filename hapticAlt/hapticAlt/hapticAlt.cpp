#include "hdSub.h"

int main(){
    Tracker t;

    for (;;) {
        system("cls");
        std::cout << "Position: " << t.m_pos[0] << " " << t.m_pos[1] << t.m_pos[2] << std::endl;
        std::cout << "Velocity: " << t.m_vel[0] << " " << t.m_vel[1] << t.m_vel[2] << std::endl;
        std::cout << "Accelerations: " << t.m_acc[0] << " " << t.m_acc[1] << t.m_acc[2] << std::endl;
        Sleep(100);
    }
}
