#include "hdSub.h"

/**********************hduVector3Dd member functions*********************************/

//Constructor.
hduVector3Dd::hduVector3Dd(const HDdouble x, const HDdouble y, const HDdouble z) {
	m_components[0] = x;
	m_components[1] = y;
	m_components[2] = z;
}

//Calculate the magnitude.
HDdouble hduVector3Dd::magnitude() const {
	return sqrt(pow(m_components[0], 2) + pow(m_components[1], 2) + pow(m_components[2], 2));
}

//Normalize the vector.
void hduVector3Dd::normalize() {
	HDdouble mag = magnitude();
	for (int i = 0; i < 3; ++i) {
		m_components[i] = m_components[i] / mag;
	}
}

//Set the vector to the given values.
void hduVector3Dd::set(const HDdouble x, const HDdouble y, const HDdouble z) {
	m_components[0] = x;
	m_components[1] = y;
	m_components[2] = z;
}

/*****************Hardware subsitutions (tracking position / veloc / accel etc)*************************/
//We start tracking position / velocity / acceleration on start up.
Tracker::Tracker() {
	std::cout << "Tracker::Tracker() - Entering constructor.\n";

	//Initialize the member values.
	//Get the cursor position (windows only).  Stored in p.x / p.y.
	POINT p;
	if (!GetCursorPos(&p)){
		std::cout << "Tracker::Tracker() - GetCursorPos() call failed.\n";
	}

	//The coordinates that we set are dependant on the plane.
	//For this initial prototyping, we allow the user to switch between the xy, xz, and zy planes.  Unfortunately, this means that controls
	//are limited to two planes at a time, though output can still be displayed in 3d.
	//This is because we avoid any external libraries to handle 3d rendering etc, both to minimize implementation time but also to 
	//avoid problems with users trying to install libraries / etc on their home systems.
	
	//Initialize to the xy plane.
	m_plane = XY;
	
	//We check which plane we are in and map screen x y to the appropriate indexes.
	int mapX;
	int mapY;

	switch (m_plane) {
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
		std::cout << "Tracker::Tracker() - m_plane set to incorrect value " << m_plane << std::endl;
	}

	for (int i = 0; i < 3; ++i) {
		m_pos[i] = 0;
		m_vel[i] = 0;
		m_acc[i] = 0;
	}

	m_pos[mapX] = p.x;
	m_pos[mapY] = p.y;

	//Start the detached thread.
	//*m_runTracker = true;

	start();

}

//Start the tracking thread.
void Tracker::start() {
	std::cout << "Tracker::start() - Detaching thread.\n";
	std::thread(&Tracker::trackState, this).detach();
}

//Callback function for running in external thread.
void Tracker::trackState() {
	//Check the start time for the first loop iteration.
	QueryPerformanceCounter(&m_ticks);

	//Can escape this loop by calling Tracker::stop() which sets *m_runTracker to false.
	while (true) {//*m_runTracker) {
		//TODO: Implement filtering.
		//At the beginning of the iteration we get the new mouse pos.  We then compare to the current member values and divide by
		//delta t to find the velocity.  We then compare the new / old values of velocity, and differentiate to find the new acceleration values.
		
		//Get the cursor position (windows only).  Stored in p.x / p.y.
		POINT p;
		if (!GetCursorPos(&p)) {
			std::cout << "Tracker::trackState() - GetCursorPos() call failed.\n";
		}

		//We check which plane we are in and map screen x y to the appropriate indexes.
		int mapX = -1;
		int mapY = -1;

		switch (m_plane) {
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
			std::cout << "Tracker::trackState() - m_plane set to incorrect value " << m_plane << std::endl;
		}

		//We need to know delta t for differentiation.  https://stackoverflow.com/questions/14337278/precise-time-measurement
		LARGE_INTEGER frequency;        // ticks per second
		LARGE_INTEGER current_ticks;           // ticks
		double dt;
		// get ticks per second
		QueryPerformanceFrequency(&frequency);
		QueryPerformanceCounter(&current_ticks);
		dt = (current_ticks.QuadPart - m_ticks.QuadPart) * 1.0 / frequency.QuadPart;
		m_ticks = current_ticks;

		//Handle veloc, accel before overwriting the current m_pos values.
		//TODO: Consider filtering.  Acceleration in particular is quite unstable.
		//Distance units are pixels (px).  Time units seconds.
		double vx = (p.x - m_pos[mapX]) / dt;
		double vy = (p.y - m_pos[mapY]) / dt;

		m_acc[mapX] = (vx - m_vel[mapX]) / dt;
		m_acc[mapY] = (vy - m_vel[mapY]) / dt;

		m_vel[mapX] = vx;
		m_vel[mapY] = vy;

		m_pos[mapX] = p.x;
		m_pos[mapY] = p.y;
		
		//A simple way to ensure window is large enough for relatively stable state data, as opposed to having nanosecond callback periods.
		Sleep(30);
	}
}



/********************************Non-member utility functions***************************/
//TODO: Verify that [] overloads are working as intended.
HDdouble dotProduct(const hduVector3Dd a, const hduVector3Dd b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

hduVector3Dd crossProduct(const hduVector3Dd a, const hduVector3Dd b) {
	return hduVector3Dd(a[1]*b[2]-a[2]*b[1], a[2]*b[0]-a[0]*b[2], a[0]*b[1]-a[1]*b[0]);
}

//In the header file, we declare globalTracker with extern.
//We now define it so it is globally accessible for maintaining hdGetDoublev syntax.
Tracker globalTracker;

//Get the vector requested by the key.
void hdGetDoublev(const int key, hduVector3Dd& v) {
	switch (key) {
	case HD_CURRENT_POSITION:
		v.set(globalTracker.m_pos[0], globalTracker.m_pos[1], globalTracker.m_pos[2]);
		break;
	case HD_CURRENT_VELOCITY:
		v.set(globalTracker.m_vel[0], globalTracker.m_vel[1], globalTracker.m_vel[2]);
		break;
	case HD_CURRENT_ACCELERATION:
		v.set(globalTracker.m_acc[0], globalTracker.m_acc[1], globalTracker.m_acc[2]);
		break;
	default:
		std::cout << "hdGetDoublev() - Unexpected fallthrough.\n";
	}
}

//We maintain the same syntax here for the sake of compilation, but we do not have the hardware to implement this function in a standard manner.
void hdSetDoublev(const int key, hduVector3Dd& v) {

}