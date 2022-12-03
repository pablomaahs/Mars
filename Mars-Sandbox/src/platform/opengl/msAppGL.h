#pragma once

#include "platform/common/msApp.h"
#include "platform/opengl/msWindowGL.h"

namespace ms
{
	class MsAppGL : public MsApp
	{
	public:
		MsAppGL();
		void Run() override;
	private:
		void Initialize() override;
		MsWindowGL mWindow{ WIDTH, HEIGHT, "OpenGL App" };
	};
}
