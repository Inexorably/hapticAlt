//A simple header that contains dummy code for hd specific types and functions.  These functions and classes would typically
//be spread out over different files, but in the interest of keeping things simple for the end user we lump everything into a single file.
//Note that this will mean this file will contain definitions for globals, utility functions, and classes.

#pragma once	//Can consider doing standard include guards if not everyone in vs.

#include <thread>
#include <memory>
#include <iostream>
#include <iomanip>

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
	//Constructors.
	hduVector3Dd();
	hduVector3Dd(const HDdouble x, const HDdouble y, const HDdouble z);

	//TODO: Overload the assignment operator to allow assignment by array in case anyone uses that.

	//Overload [] operator to allow [] indexing / assignment.
	HDdouble operator[](int i) const { return m_components[i]; };
	HDdouble& operator[](int i) { return m_components[i]; };

	//Calculate the magnitude.
	HDdouble magnitude() const;

	//Normalize the vector.
	void normalize();

	//Set all components of the vector.
	void set(const HDdouble x, const HDdouble y, const HDdouble z);

	//Print the components to console for debugging.
	void print() const;
};

/*****************Hardware subsitutions (tracking position / veloc / accel etc)*************************/
//This class tracks the position / velocity / acceleration information of the mouse.
//TODO: Consider making this class singleton (low priority).
class Tracker {
public:
	void start();
	Tracker();

	//State data storage.  Would normally want member functions to safely access these, but end user will not be interacting with 
	//the tracker object so this is not a big deal.
	double m_pos[3];
	double m_vel[3];
	double m_acc[3];

	//Store the haptic output force for display.
	double m_force[3];
	
	//What plane we are working in.
	int m_plane;

	//Enable or disable psuedo haptic feedback applied to the mouse.
	bool m_enablePseudoHaptics = true;

private:
	//Track the time for differentiation.
	LARGE_INTEGER m_ticks;

	//Callback function for running in external thread.
	void trackState();
};

//We declare a global, accessible instance so that we can preserve the arguments of hdGetDoubleev etc.
extern Tracker globalTracker;

/************************************Non-member HD utility functions**************/
HDdouble dotProduct(const hduVector3Dd& a, const hduVector3Dd& b);
hduVector3Dd crossProduct(const hduVector3Dd& a, const hduVector3Dd& b);
void hdGetDoublev(const int key, hduVector3Dd& v);
void hdSetDoublev(const int key, hduVector3Dd& v);

/**********************************Generic utility functions**************************/
void printState();

/************************Operator overloads******************************************/

//We define outside of class so that we can have bidirectional arguments.
//For example, if we overloaded the member operators we could only have the class on the lhs, and argument on the rhs.
	
//Scalar + vector overloads.  Define for common types of numeric scalars.  Define in both directions.
hduVector3Dd operator+(const hduVector3Dd& lhs, const int& rhs);
hduVector3Dd operator+(const int& lhs, const hduVector3Dd& rhs);
hduVector3Dd operator+(const hduVector3Dd& lhs, const HDdouble& rhs);
hduVector3Dd operator+(const HDdouble& lhs, const hduVector3Dd& rhs);
hduVector3Dd operator+(const hduVector3Dd& lhs, const float& rhs);
hduVector3Dd operator+(const float& lhs, const hduVector3Dd& rhs);

hduVector3Dd operator-(const hduVector3Dd& lhs, const int& rhs);
hduVector3Dd operator-(const int& lhs, const hduVector3Dd& rhs);
hduVector3Dd operator-(const hduVector3Dd& lhs, const HDdouble& rhs);
hduVector3Dd operator-(const HDdouble& lhs, const hduVector3Dd& rhs);
hduVector3Dd operator-(const hduVector3Dd& lhs, const float& rhs);
hduVector3Dd operator-(const float& lhs, const hduVector3Dd& rhs);

hduVector3Dd operator*(const hduVector3Dd& lhs, const int& rhs);
hduVector3Dd operator*(const int& lhs, const hduVector3Dd& rhs);
hduVector3Dd operator*(const hduVector3Dd& lhs, const HDdouble& rhs);
hduVector3Dd operator*(const HDdouble& lhs, const hduVector3Dd& rhs);
hduVector3Dd operator*(const hduVector3Dd& lhs, const float& rhs);
hduVector3Dd operator*(const float& lhs, const hduVector3Dd& rhs);

hduVector3Dd operator/(const hduVector3Dd& lhs, const int& rhs);
hduVector3Dd operator/(const hduVector3Dd lhs, const HDdouble& rhs);
hduVector3Dd operator/(const hduVector3Dd& lhs, const float& rhs);

//Vector + vector overloads.  Could be defined in class, but will declare here so that we have all numeric operators grouped.
hduVector3Dd operator+(const hduVector3Dd& lhs, const hduVector3Dd& rhs);
hduVector3Dd operator-(const hduVector3Dd& lhs, const hduVector3Dd& rhs);
