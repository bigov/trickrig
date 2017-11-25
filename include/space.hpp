//============================================================================
//
// file: space.hpp
//
//============================================================================
#ifndef __SPACE_HPP__
#define __SPACE_HPP__

#include "main.hpp"
#include "config.hpp"
#include "rigs.hpp"
#include "gen3d.hpp"
#include "ttf.hpp"

namespace tr
{
	class Scene
	{
		private:
 			Scene(const tr::Scene&);
			Scene operator=(const tr::Scene&);

    	GLuint 
				vaoQuad = 0,
				texColorBuffer = 0,
				text = 0,
				frameBuffer = 0;

			tr::Config* cfg = nullptr;
			tr::TTF ttf {};
			tr::Rigs rigs {};
			tr::Gen3d gen3d {};
			tr::pngImg show_fps {};
			tr::Glsl screenShaderProgram {};

			void space_generate(void);
			void framebuffer_init(void);
			void program2d_init(void);

		public:
			Scene(tr::Config*);
			~Scene(void);
			void draw(const evInput &);
	};

} //namespace
#endif
