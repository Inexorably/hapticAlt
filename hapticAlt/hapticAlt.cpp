#include "hdSub.h"
#include <iomanip>

int main(){
    //Display loop.  TODO: Stick this in its on loop.
    //Limit the decimal places to make output more readable.
    std::cout << std::setprecision(2) << std::fixed;
    while (true) {
        //Clear the console.
        system("cls");

        //Check if the user wants to change planes, using the arrow keys.
        if (GetKeyState(VK_LEFT) & 0x8000){
            globalTracker.m_plane = (++globalTracker.m_plane) % 3;
            
            //Sleep for sometime to prevent repeated changes from user holding key too long.
            Sleep(300);
        }
        else if(GetKeyState(VK_RIGHT) & 0x8000) {
            --globalTracker.m_plane;
            //In c++, -1 % 3 seems to be -1 as % is not truly a mod operator but rather a division operator.  Thus, we handle the negative case.
            if (globalTracker.m_plane < 0) {
                globalTracker.m_plane = 2;
            }

            //Sleep for sometime to prevent repeated changes from user holding key too long.
            Sleep(300);
        }

        //Display the current plane:
        switch (globalTracker.m_plane) {
        case XY:
            std::cout << "Plane: XY\n\n";
            break;
        case XZ:
            std::cout << "Plane: XZ\n\n";
            break;
        case ZY:
            std::cout << "Plane: ZY\n\n";
            break;
        default:
            std::cout << "Plane: Incorrect value - " << globalTracker.m_plane << "\n\n";
        }

        //We limit the decimal places to make the code more readable.  We do everything on seperate lines to avoid problems with tab spacing as numbers change text length / gain negative signs etc.
        std::cout << "Position:\nx: " << globalTracker.m_pos[0] << "\ny: " << globalTracker.m_pos[1] << "\nz: " << globalTracker.m_pos[2] << "\n\n";
        std::cout << "Velocity:\nx: " << globalTracker.m_vel[0] << "\ny: " << globalTracker.m_vel[1] << "\nz: " << globalTracker.m_vel[2] << "\n\n";
        std::cout << "Acceleration:\nx: " << globalTracker.m_acc[0] << "\ny: " << globalTracker.m_acc[1] << "\nz: " << globalTracker.m_acc[2] << "\n\n";
        std::cout << "Force:\nx: " << globalTracker.m_force[0] << "\ny: " << globalTracker.m_force[1] << "\nz: " << globalTracker.m_force[2] << "\n\n";
        Sleep(50);
    }
}

//TODO: Add haptic callback function in its own thread.