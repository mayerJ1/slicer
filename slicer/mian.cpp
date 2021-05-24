#include "sliceProcess.h"
#include <iostream>
#include "infill.h"

int main() {
	const char* f = "3DR_Spool_V2_001_RTP.stl";
	cura::slicerProcess s(f);
	s.has_z();
	s.compute();
	s.get_points();
	std::cout << s.points[0];
	
	return 0;
}