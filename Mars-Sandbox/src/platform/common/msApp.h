#pragma once

namespace ms
{
	class MsApp
	{
	public:
		virtual void Run() = 0;

	protected:
		virtual void Initialize() = 0;
	};
}