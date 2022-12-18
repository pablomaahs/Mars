#pragma once

#include "platform/common/msApp.h"
#include "platform/opengl/msWindowGL.h"

namespace ms
{
	class MsAppGL : public MsApp
	{
	public:
		MsAppGL();
		MsAppGL(unsigned int w, unsigned int h, std::string name);

		void Run() override;
	private:
		void Initialize() override;
		MsWindowGL mWindow;
	};
}
