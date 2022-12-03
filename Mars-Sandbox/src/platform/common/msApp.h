#pragma once

namespace ms
{
	class MsApp
	{
	public:
		static constexpr int WIDTH = 800;
		static constexpr int HEIGHT = 600;

		virtual void Run() = 0;

	protected:
		virtual void Initialize() = 0;
	};
}