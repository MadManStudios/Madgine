#pragma once

namespace Engine {
namespace NodeGraph {

	enum NodeExecutionMask {
		NONE = 0b00,
		CPU  = 0b01,
		GPU  = 0b10,
		ALL  = 0b11		
};

}
}