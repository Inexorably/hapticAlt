#include "hdSub.h"

/*****************************************The primary haptic callback thread.************************************************************************/
/*
* In most cases, the user should make their edits / custom rendering function in this function.
* This function runs at approximately 100 hz.  Faster frequencies cause instabilities for integrating 
* the force for applying psuedo haptic feedback (toggleable) to the mouse.
*/

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

		////////////////////////////////////////////////////Render a 3d rigid sphere//////////////////////////////
		//Define the center in x y z of the sphere.
		//Define the sphere radius.  Is there an advanrage to using the HDdouble over standard c++ types?
		hduVector3Dd sphereCenter(900, 500, 0);
		HDdouble sphereRadius = 250;
		HDdouble k = 1200;	//N/mm.

		//The distance between position and the sphereCenter.  Note that pow might not be defined for HDdouble type...
		hduVector3Dd distanceVector = position - sphereCenter;
		HDdouble distance = distanceVector.magnitude();

		//If distance is less than sphereRadius, we are inside the sphere.
		if (distance < sphereRadius) {
			//We find the unit vector from the center of the sphere to the HIP position.
			hduVector3Dd rHat = position - sphereCenter;
			//Normalize the vector.
			rHat.normalize();

			//Set f.  May have to static cast to double for distanceSquared.  Note that this is still optimal because we
			//don't need to do the sqrt operation in non collision cases.
			f = k * (sphereRadius - distance) * rHat;
		}

		//Output the haptic feedback.
		hdSetDoublev(HD_CURRENT_FORCE, f);

		//////////////////////////////////////////////////////////////////////////////////
		//*** STOP EDITING HERE ***//////////////////////////////////////////////////////

		//We apply some "force" to the mouse, where we do A*dt^2/2 for distance change in x y z.
		//Note that we treat the "force" as an acceleration -- we do not take into account mass as we just have a mouse for ux.

		//If the force's magnitude that we would apply to the mouse is less than globalTracker.m_stationaryForce, we apply no values.
		//Please see the declaration of globalTracker.m_stationaryForce in hdSub.h for further explanation of this variable.

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
		//Get the current cursor position.
		POINT point;
		if (!GetCursorPos(&point)) {
			std::cout << "hapticCallback: Could not get mouse coordinates\n";
		}

		//Modify the cursor position per the output force f.
		//This is dependant on what plane we are operating in.
		//We check which plane we are in and map screen x y to the appropriate indexes.
		int mapX = -1;
		int mapY = -1;
		switch (globalTracker.m_plane) {
		case XY:
			mapX = 0;
			mapY = 1;
			break;
		case XZ:
			mapX = 0;
			mapY = 2;

			break;
		case ZY:
			mapX = 2;
			mapY = 1;

			break;
		default:
			//Catch error fallthroughs.
			std::cout << "hapticCallback() - globalTracker.m_plane set to incorrect value " << globalTracker.m_plane << std::endl;
			return;
		}

		//Integrate the forces (again, treating as acceleration due to no mass info about hip), and find where our new point should be.
		//We are working in px, so we round to the nearest integer.
		point.x += std::round(f[mapX] * pow(dt, 2) / 2);
		point.y += std::round(f[mapY] * pow(dt, 2) / 2);

		//Set the cursor position.
		if (globalTracker.m_enablePseudoHaptics && !SetCursorPos(point.x, point.y)) {
			std::cout << "hapticCallback() - Unable to SetCursorPos\n";
		}
		
		//Sleep for increased window for smoother integration.
		Sleep(10);
	}
}

/********************************************Demo haptic Functions*************************************************************/

//A demo function to show a spring modeled plane with damping fields.
//If the user would like to test this function, this function should replace the hapticCallback function in the call:
//std::thread hapticRenderThread(hapticCallback);
//in main.
void hapticCallbackDampingFieldDemo() {
	while (true) {
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
		HDdouble k = 600;

		//Define the damping coefficient in N-sec/mm.
		HDdouble b = 100;

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
			if (int(abs(position[0])) % 300 > 150) {

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
		//Note that we treat the "force" as an acceleration -- we do not take into account mass as we just have a mouse for ux.

		//If the force's magnitude that we would apply to the mouse is less than globalTracker.m_stationaryForce, we apply no values.
		//Please see the declaration of globalTracker.m_stationaryForce in hdSub.h for further explanation of this variable.

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
		//Get the current cursor position.
		POINT point;
		if (!GetCursorPos(&point)) {
			std::cout << "hapticCallback: Could not get mouse coordinates\n";
		}

		//Modify the cursor position per the output force f.
		//This is dependant on what plane we are operating in.
		//We check which plane we are in and map screen x y to the appropriate indexes.
		int mapX = -1;
		int mapY = -1;
		switch (globalTracker.m_plane) {
		case XY:
			mapX = 0;
			mapY = 1;
			break;
		case XZ:
			mapX = 0;
			mapY = 2;

			break;
		case ZY:
			mapX = 2;
			mapY = 1;

			break;
		default:
			//Catch error fallthroughs.
			std::cout << "hapticCallback() - globalTracker.m_plane set to incorrect value " << globalTracker.m_plane << std::endl;
			return;
		}

		//Integrate the forces (again, treating as acceleration due to no mass info about hip), and find where our new point should be.
		//We are working in px, so we round to the nearest integer.
		point.x += std::round(f[mapX] * pow(dt, 2) / 2);
		point.y += std::round(f[mapY] * pow(dt, 2) / 2);

		//Set the cursor position.
		if (globalTracker.m_enablePseudoHaptics && !SetCursorPos(point.x, point.y)) {
			std::cout << "hapticCallback() - Unable to SetCursorPos\n";
		}


		//Sleep for increased window for smoother integration.
		Sleep(10);
	}
}

//A demo function to show a sphere plane modeled as a spring.
//If the user would like to test this function, this function should replace the hapticCallback function in the call:
//std::thread hapticRenderThread(hapticCallback);
//in main.
void hapticCallbackSpringSphereDemo() {
	while (true) {
		// Get the position of the device.
		hduVector3Dd position;
		hdGetDoublev(HD_CURRENT_POSITION, position);

		// you don't have to use the following variables, but they may be useful
		hduVector3Dd normal(0, 0, 0);
		hduVector3Dd f(0, 0, 0);
		double  dist = 0;

		//*** START EDITING HERE ***//////////////////////////////////////////////////
		//////////////////////////////////////////////////////////////////////////////

		////////////////////////////////////////////////////Render a 3d rigid sphere//////////////////////////////
		//Define the center in x y z of the sphere.
		//Define the sphere radius.  Is there an advanrage to using the HDdouble over standard c++ types?
		hduVector3Dd sphereCenter(900, 500, 0);
		HDdouble sphereRadius = 250;
		HDdouble k = 900;	//N/mm.

		//The distance between position and the sphereCenter.  Note that pow might not be defined for HDdouble type...
		hduVector3Dd distanceVector = position - sphereCenter;
		HDdouble distance = distanceVector.magnitude();

		//If distance is less than sphereRadius, we are inside the sphere.
		if (distance < sphereRadius) {
			//We find the unit vector from the center of the sphere to the HIP position.
			hduVector3Dd rHat = position - sphereCenter;
			//Normalize the vector.
			rHat.normalize();

			//Set f.  May have to static cast to double for distanceSquared.  Note that this is still optimal because we
			//don't need to do the sqrt operation in non collision cases.
			f = k * (sphereRadius - distance) * rHat;
		}

		//std::cout << "x: " << position[0] << ", y: " << position[1] << ", z: " << position[2] << ", dist: " << distance << ", magnitude: " << static_cast<double>(f.magnitude()) << std::endl;

		hdSetDoublev(HD_CURRENT_FORCE, f);

		//////////////////////////////////////////////////////////////////////////////////
		//*** STOP EDITING HERE ***//////////////////////////////////////////////////////

		//We apply some "force" to the mouse, where we do A*dt^2/2 for distance change in x y z.
		//Note that we treat the "force" as an acceleration -- we do not take into account mass as we just have a mouse for ux.

		//If the force's magnitude that we would apply to the mouse is less than globalTracker.m_stationaryForce, we apply no values.
		//Please see the declaration of globalTracker.m_stationaryForce in hdSub.h for further explanation of this variable.

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
		//Get the current cursor position.
		POINT point;
		if (!GetCursorPos(&point)) {
			std::cout << "hapticCallback: Could not get mouse coordinates\n";
		}

		//Modify the cursor position per the output force f.
		//This is dependant on what plane we are operating in.
		//We check which plane we are in and map screen x y to the appropriate indexes.
		int mapX = -1;
		int mapY = -1;
		switch (globalTracker.m_plane) {
		case XY:
			mapX = 0;
			mapY = 1;
			break;
		case XZ:
			mapX = 0;
			mapY = 2;

			break;
		case ZY:
			mapX = 2;
			mapY = 1;

			break;
		default:
			//Catch error fallthroughs.
			std::cout << "hapticCallback() - globalTracker.m_plane set to incorrect value " << globalTracker.m_plane << std::endl;
			return;
		}

		//Integrate the forces (again, treating as acceleration due to no mass info about hip), and find where our new point should be.
		//We are working in px, so we round to the nearest integer.
		point.x += std::round(f[mapX] * pow(dt, 2) / 2);
		point.y += std::round(f[mapY] * pow(dt, 2) / 2);

		//Set the cursor position.
		if (globalTracker.m_enablePseudoHaptics && !SetCursorPos(point.x, point.y)) {
			std::cout << "hapticCallback() - Unable to SetCursorPos\n";
		}


		//Sleep for increased window for smoother integration.
		Sleep(10);
	}
}

/********************************************Main*******************************************************************************/

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