#include "hdSub.h"

/**********************hduVector3Dd member functions*********************************/

//Constructors.
hduVector3Dd::hduVector3Dd() {
	for (int i = 0; i < 3; ++i) {
		m_components[i] = 0;
	}
}

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

//Print the components to console for debugging.
void hduVector3Dd::print() const {
	std::cout << '\n';
	for (int i = 0; i < 3; ++i) {
		std::cout << m_components[i] << '\n';
	}
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
		m_force[i] = 0;
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
	while (true) {
		//TODO: Implement filtering.
		//At the beginning of the iteration we get the new mouse pos.  We then compare to the current member values and divide by
		//delta t to find the velocity.  We then compare the new / old values of velocity, and differentiate to find the new acceleration values.
		
		//Get the cursor position (windows only).  Stored in p.x / p.y.
		POINT p;
		if (!GetCursorPos(&p)) {
			std::cout << "Tracker::trackState() - GetCursorPos() call failed.\n";
		}

		//We check which plane we are in and map screen x y to the appropriate indexes, in order to avoid too much duplicate code in the switch.
		int mapX = -1;
		int mapY = -1;

		switch (m_plane) {
		case XY:
			mapX = 0;
			mapY = 1;
			m_vel[2] = 0;
			m_acc[2] = 0;
			break;
		case XZ:
			mapX = 0;
			mapY = 2;
			m_vel[1] = 0;
			m_acc[1] = 0;
			break;
		case ZY:
			mapX = 2;
			mapY = 1;
			m_vel[0] = 0;
			m_acc[0] = 0;
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

		//We modify the cursor position by the acceleration * dt^2/2.

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
			if (int(abs(position[0])) % 90 > 40) {

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
		
		
		//A simple way to ensure window is large enough for relatively stable state data.  Alternatively can implement some
		//moving avergage / etc filters.
		Sleep(30);
	}
}



/********************************Non-member utility functions***************************/
//TODO: Verify that [] overloads are working as intended.
HDdouble dotProduct(const hduVector3Dd& a, const hduVector3Dd& b) {
	return a[0] * b[0] + a[1] * b[1] + a[2] * b[2];
}

hduVector3Dd crossProduct(const hduVector3Dd& a, const hduVector3Dd& b) {
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
		std::cout << "hdGetDoublev() - Unexpected fallthrough with key " << key << '\n';
	}
}

//We maintain the same syntax here for the sake of compilation, but we do not have the hardware to implement this function in a standard manner.
//We instead update the force values in the globalTracker variable so that we can display them in console.
void hdSetDoublev(const int key, hduVector3Dd& v) {
	switch (key) {
	case HD_CURRENT_FORCE:
		for (int i = 0; i < 3; ++i) {
			globalTracker.m_force[i] = v[i];
		}
		break;
	default:
		std::cout << "hdSetDoublev() - Unexpected fallthrough with key " << key << '\n';
	}
}

/**********************************Generic utility functions**************************/
void printState() {
	//Display loop.
//Limit the decimal places to make output more readable.
	std::cout << std::setprecision(2) << std::fixed;
	while (true) {
		//Clear the console.
		system("cls");

		//Check if the user wants to quit the application (press q).
		//Check if the user wants to change planes, using the arrow keys.
		if (GetKeyState('Q') & 0x8000) {
			return;
		}
		//Check if the user wants to change planes, using the arrow keys.
		else if (GetKeyState(VK_LEFT) & 0x8000) {
			globalTracker.m_plane = (++globalTracker.m_plane) % 3;

			//Sleep for sometime to prevent repeated changes from user holding key too long.
			Sleep(300);
		}
		else if (GetKeyState(VK_RIGHT) & 0x8000) {
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

		//We do everything on seperate lines to avoid problems with tab spacing as numbers change text length / gain negative signs etc.
		std::cout << "Position:\nx: " << globalTracker.m_pos[0] << "\ny: " << globalTracker.m_pos[1] << "\nz: " << globalTracker.m_pos[2] << "\n\n";
		std::cout << "Velocity:\nx: " << globalTracker.m_vel[0] << "\ny: " << globalTracker.m_vel[1] << "\nz: " << globalTracker.m_vel[2] << "\n\n";
		std::cout << "Acceleration:\nx: " << globalTracker.m_acc[0] << "\ny: " << globalTracker.m_acc[1] << "\nz: " << globalTracker.m_acc[2] << "\n\n";
		std::cout << "Force:\nx: " << globalTracker.m_force[0] << "\ny: " << globalTracker.m_force[1] << "\nz: " << globalTracker.m_force[2] << "\n\n";
		Sleep(50);
	}
}

/************************Operator overloads******************************************/

//We define outside of class so that we can have bidirectional arguments.
//For example, if we overloaded the member operators we could only have the class on the lhs, and argument on the rhs.

//Scalar + vector overloads.  Define for common types of numeric scalars.  Define in both directions.

hduVector3Dd operator+(const hduVector3Dd& lhs, const int& rhs) {
	return hduVector3Dd(lhs[0] + static_cast<double>(rhs), lhs[1] + static_cast<double>(rhs), lhs[2] + static_cast<double>(rhs));
}
hduVector3Dd operator+(const int& lhs, const hduVector3Dd& rhs) {
	return hduVector3Dd(rhs[0]+ static_cast<double>(lhs), rhs[1] + static_cast<double>(lhs), rhs[2] + static_cast<double>(lhs));
}
hduVector3Dd operator+(const hduVector3Dd& lhs, const HDdouble& rhs) {
	return hduVector3Dd(lhs[0] + rhs, lhs[1] + rhs, lhs[2] + rhs);
}
hduVector3Dd operator+(const HDdouble& lhs, const hduVector3Dd& rhs) {
	return hduVector3Dd(rhs[0] + lhs, rhs[1] + lhs, rhs[2] + lhs);
}
hduVector3Dd operator+(const hduVector3Dd& lhs, const float& rhs) {
	return hduVector3Dd(lhs[0] + static_cast<double>(rhs), lhs[1] + static_cast<double>(rhs), lhs[2] - static_cast<double>(rhs));
}
hduVector3Dd operator+(const float& lhs, const hduVector3Dd& rhs) {
	return hduVector3Dd(rhs[0] + static_cast<double>(lhs), rhs[1] + static_cast<double>(lhs), rhs[2] - static_cast<double>(lhs));
}

hduVector3Dd operator-(const hduVector3Dd& lhs, const int& rhs) {
	return hduVector3Dd(lhs[0] - static_cast<double>(rhs), lhs[1] - static_cast<double>(rhs), lhs[2] - static_cast<double>(rhs));
}
hduVector3Dd operator-(const int& lhs, const hduVector3Dd& rhs) {
	return hduVector3Dd(rhs[0] - static_cast<double>(lhs), rhs[1] - static_cast<double>(lhs), rhs[2] - static_cast<double>(lhs));
}
hduVector3Dd operator-(const hduVector3Dd& lhs, const HDdouble& rhs) {
	return hduVector3Dd(lhs[0] - rhs, lhs[1] - rhs, lhs[2] - rhs);
}
hduVector3Dd operator-(const HDdouble& lhs, const hduVector3Dd& rhs) {
	return hduVector3Dd(rhs[0] - lhs, rhs[1] - lhs, rhs[2] - lhs);
}
hduVector3Dd operator-(const hduVector3Dd& lhs, const float& rhs) {
	return hduVector3Dd(lhs[0] - static_cast<double>(rhs), lhs[1] - static_cast<double>(rhs), lhs[2] - static_cast<double>(rhs));
}
hduVector3Dd operator-(const float& lhs, const hduVector3Dd& rhs) {
	return hduVector3Dd(rhs[0] - static_cast<double>(lhs), rhs[1] - static_cast<double>(lhs), rhs[2] - static_cast<double>(lhs));
}

hduVector3Dd operator*(const hduVector3Dd& lhs, const int& rhs) {
	return hduVector3Dd(lhs[0] * static_cast<double>(rhs), lhs[1] * static_cast<double>(rhs), lhs[2] * static_cast<double>(rhs));
}
hduVector3Dd operator*(const int& lhs, const hduVector3Dd& rhs) {
	return hduVector3Dd(rhs[0] * static_cast<double>(lhs), rhs[1] * static_cast<double>(lhs), rhs[2] * static_cast<double>(lhs));
}
hduVector3Dd operator*(const hduVector3Dd& lhs, const HDdouble& rhs) {
	return hduVector3Dd(lhs[0] * rhs, lhs[1] * rhs, lhs[2] * rhs);
}
hduVector3Dd operator*(const HDdouble& lhs, const hduVector3Dd& rhs) {
	return hduVector3Dd(rhs[0] * lhs, rhs[1] * lhs, rhs[2] * lhs);
}
hduVector3Dd operator*(const hduVector3Dd& lhs, const float& rhs) {
	return hduVector3Dd(lhs[0] * static_cast<double>(rhs), lhs[1] * static_cast<double>(rhs), lhs[2] * static_cast<double>(rhs));
}
hduVector3Dd operator*(const float& lhs, const hduVector3Dd& rhs) {
	return hduVector3Dd(rhs[0] * static_cast<double>(lhs), rhs[1] * static_cast<double>(lhs), rhs[2] * static_cast<double>(lhs));
}

hduVector3Dd operator/(const hduVector3Dd& lhs, const int& rhs) {
	return hduVector3Dd(lhs[0] / static_cast<double>(rhs), lhs[1] / static_cast<double>(rhs), lhs[2] / static_cast<double>(rhs));
}
hduVector3Dd operator/(const hduVector3Dd lhs, const HDdouble& rhs) {
	return hduVector3Dd(lhs[0] / rhs, lhs[1] / rhs, lhs[2] / rhs);
}
hduVector3Dd operator/(const hduVector3Dd& lhs, const float& rhs) {
	return hduVector3Dd(lhs[0] / static_cast<double>(rhs), lhs[1] / static_cast<double>(rhs), lhs[2] / static_cast<double>(rhs));
}

//Vector + vector overloads.  Could be defined in class, but will declare here so that we have all numeric operators grouped.
hduVector3Dd operator+(const hduVector3Dd& lhs, const hduVector3Dd& rhs) {
	return hduVector3Dd(lhs[0] + rhs[0], lhs[1] + rhs[1], lhs[2] + rhs[2]);
}
hduVector3Dd operator-(const hduVector3Dd& lhs, const hduVector3Dd& rhs) {
	return hduVector3Dd(lhs[0] - rhs[0], lhs[1] - rhs[1], lhs[2] - rhs[2]);
}
