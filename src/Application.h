//
// Created by nathan on 1/10/23.
//

#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include "defines.h"

namespace Vulkan {

class Application
{
public:
	Application(const std::string &name, i32 x, i32 y, i32 width, i32 height);
	~Application();

	bool should_close();
	void update();

private:
	bool _initialized_properly;
};

}

#endif //APPLICATION_H
