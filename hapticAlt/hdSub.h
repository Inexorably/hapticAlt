//A simple header that contains dummy code for hd specific types and functions.  These functions and classes would typically
//be spread out over different files, but in the interest of keeping things simple for the end user we lump everything into a single file.
//Note that this will mean this file will contain definitions for globals, utility functions, and classes.

#pragma once	//Can consider doing standard include guards if not everyone in vs.

#include <thread>
#include <memory>
#include <iostream>

#include "math.h"
#include "time.h"  
#include "Windows.h"

/***************************************Globals*********************************/
//For hdGetDoublev:
const int HD_CURRENT_POSITION = 0;
const int HD_CURRENT_VELOCITY = 1;
const int HD_CURRENT_ACCELERATION = 2;
const int HD_CURRENT_FORCE = 10;

//What plane are we working in:
const int XY = 0;
const int XZ = 1;
const int ZY = 2;

/*************************************HDU classes***********************************/
//When possible (ie hd type functions exactly the same as a std type), we use typedef for symplicity.
typedef double HDdouble;

class hduVector3Dd {
protected:
	//Store the vector components.  We make this protected as we want the user to either use overloaded [] assignment or .set().
	double m_components[3];

public:
	//Constructor.
	hduVector3Dd(const HDdouble x, const HDdouble y, const HDdouble z);

	//TODO: Overload the assignment operator to allow assignment by array in case anyone uses that.
	//hduVector3Dd& operator = (const hduVector3Dd& vector);

	//Overload [] operator to allow [] indexing / assignment.
	HDdouble operator [](int i) const { return m_components[i]; };
	HDdouble& operator [](int i) { return m_components[i]; };

	//Calculate the magnitude.
	HDdouble magnitude() const;

	//Normalize the vector.
	void normalize();

	//Set all components of the vector.
	void set(const HDdouble x, const HDdouble y, const HDdouble z);
};

/*****************Hardware subsitutions (tracking position / veloc / accel etc)*************************/
//This class tracks the position / velocity / acceleration information of the mouse.
//TODO: Consider making this class singleton (low priority).
class Tracker {
public:
	void start();
	Tracker();

	//Modifies the m_plane variable and changes the viewing / tracking plane.
	void changePlane(const int plane);

	//State data storage.  Would normally want member functions to safely access these, but end user will not be interacting with 
	//the tracker object so this is not a big deal.
	HDdouble m_pos[3];
	HDdouble m_vel[3];
	int m_acc[3];
private:
	//Track the time for differentiation.
	clock_t m_time;

	//What plane we are working in.
	int m_plane;

	//Callback function for running in external thread.
	void trackState();
};

/************************************Non-member utility functions**************/
HDdouble dotProduct(const hduVector3Dd a, const hduVector3Dd b);
hduVector3Dd crossProduct(const hduVector3Dd a, const hduVector3Dd b);
void hdGetDoublev(const int key, hduVector3Dd& v);
void hdSetDoublev(const int key, hduVector3Dd& v);