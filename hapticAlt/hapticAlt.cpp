#include "hdSub.h"

int main(){
    while (true) {
        system("cls");
        std::cout << "Position: " << globalTracker.m_pos[0] << " " << globalTracker.m_pos[1] << " " << globalTracker.m_pos[2] << std::endl;
        std::cout << "Velocity: " << globalTracker.m_vel[0] << " " << globalTracker.m_vel[1] << " " << globalTracker.m_vel[2] << std::endl;
        std::cout << "Accelerations: " << globalTracker.m_acc[0] << " " << globalTracker.m_acc[1] << " " << globalTracker.m_acc[2] << std::endl;
        Sleep(50);
    }
}
