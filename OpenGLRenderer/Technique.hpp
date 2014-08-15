#pragma once

#include <vector>
#include <string>

namespace Rendering
{
	class Technique
	{
	public:

		Technique();
		virtual ~Technique();

		virtual void Enable();

	protected:

		bool AddShader(int shaderType, const std::string& filename);

		virtual bool Setup() = 0;
		bool Finalize();

		int ProgramIndex;

	private:

		std::vector<int> Shaders;
	};
}