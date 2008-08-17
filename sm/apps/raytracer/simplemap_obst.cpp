#include "simplemap.h"

namespace PPU {
	
	bool Map::isCellObst(const Viewport& cbb) const {
		for (unsigned int i = 0; i < size(); i++) {
			const Segment& s = at(i);	
			if(s.obstType != ObstSolid) continue;
			
			if(cbb.crossesSegment(s.p[0], s.p[1]))
				return true;
		}
		return false;
	}
	
	
}
