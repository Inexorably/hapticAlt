#include "hdSub.h"

void hapticCallback() {
	while(true) {
		// Get the position of the device.
		hduVector3Dd position;
		hdGetDoublev(HD_CURRENT_POSITION, position);

		// you don't have to use the following variables, but they may be useful
		hduVector3Dd normal(0, 0, 0);
		hduVector3Dd f(0, 0, 0);
		double  dist = 0;

		//*** START EDITING HERE ***//////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////

		//Get the velocity at each callback.
		hduVector3Dd velocity;
		hdGetDoublev(HD_CURRENT_VELOCITY, velocity);

		//We use the same stiffness k for all examples below.  In the lab, we will have different cpps for each example, so we will have to redefine it in each.
		//Note that k will be wrt the internal distance units / force units.
		HDdouble k = 0.20;

		//Define the damping coefficient in N-sec/mm.
		HDdouble b = 0.00917;

		/* Implement the algorithm covered in class
		to create a frictionless oriented 3D plane with an offset, as defined by a point on the plane and the normal to the plane.
		Indicate the orientation of the plane you chose clearly in your comment*/

		//Note that in the future, this could be better done by having a plane struct with normal and point vector members.
		hduVector3Dd planeNormal(0, -1, 0);
		hduVector3Dd planePoint(0, 200, 0);

		//Make sure that planeNormal is a unit vector.
		planeNormal.normalize();

		//r is the vector from the point on the plane planePoint to the user position.
		hduVector3Dd r = position - planePoint;

		//d is the impression of r onto planeNormal: if d is negative, the user is on or in the wall.  If d is positive, the user is outside of the wall.
		HDdouble d = dotProduct(r, planeNormal);

		//We initalize our force output var.
		f.set(0, 0, 0);


		//If d is negative, the user is on or in the wall.  If d is positive, the user is outside of the wall.
		if (d <= 0) {
			if (int(abs(position[0])) % 90 > 40){

				f.set(-1 * velocity[0] * b, 0, 0);
					//std::cout << f[0] << ", " << f[1] << ", " << f[2] << "   V :" << velocity.magnitude() << std::endl;
			}

			//The plane exerts normal force on the user.
			f = f + -1 * k * d * planeNormal;
		}

		// command the desired force "f". You must determine the value of "f" before using this line.
		hdSetDoublev(HD_CURRENT_FORCE, f);

		//////////////////////////////////////////////////////////////////////////////////
		//*** STOP EDITING HERE ***//////////////////////////////////////////////////////

		//We apply some "force" to the mouse, where we do A*dt^2/2 for distance change in x y z.

		//Check our elapsed time for force integration.
		LARGE_INTEGER frequency;				// ticks per second
		static LARGE_INTEGER prev_ticks;		// ticks in the last iteration.
		LARGE_INTEGER current_ticks;            // ticks in this iteration.
		double dt;

		// get ticks per second
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&current_ticks);
		dt = (current_ticks.QuadPart - prev_ticks.QuadPart) * 1.0 / frequency.QuadPart;
		prev_ticks = current_ticks;

		//Apply the force based on dt to the cursor.
		POINT point;
		if (!GetCursorPos(&point)) {
			std::cout << "hapticCallback: Could not get mouse coordinates\n";
		}

		TODO leaving off here.

		//Sleep for increased window for smoother integration.
		Sleep(10);
	}
}

int main(){
	//Detach and spawn the output (gui rendering) thread.  We are passing stop by reference, so we need to wrap it in std::ref.
	std::thread printThread(printState);

	//We can go ahead and run the haptic callback thread 
	std::thread hapticRenderThread(hapticCallback);
	hapticRenderThread.detach();			//We don't care about waiting for this thread to complete, exit is controlled by printThread.

	//Wait for the main thread to rejoin with the print thread.
	printThread.join();

	return 1;
}

//TODO: Add haptic callback function in its own thread.