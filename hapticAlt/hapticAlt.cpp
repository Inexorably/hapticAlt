#include "hdSub.h"

void hapticCallback() {
	return;
}

int main(){
	//Detach and spawn the output (gui rendering) thread.  We are passing stop by reference, so we need to wrap it in std::ref.
	std::thread printThread(printState);

	//Wait for the main thread to rejoin with the print thread.
	printThread.join();

	return 1;
}

//TODO: Add haptic callback function in its own thread.