#include "hdSub.h"

int main(){
	//Detach and spawn the output (gui rendering) thread.
	std::thread printThread(printState);


	//Wait for the printThread to complete.
	printThread.join();
}

//TODO: Add haptic callback function in its own thread.