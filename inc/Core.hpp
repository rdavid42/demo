
#ifndef CORE_HPP
# define CORE_HPP

# include <cstring>
# include <iostream>
# include <cstdlib>
# include <list>
# include "Utils.hpp"
# include "Vec3.hpp"
# include "Mat4Stack.hpp"

typedef struct		s_demo
{
	Vec3<float>		rotation;
	Vec3<float>		translation;
	Vec3<float>		scale;
}					t_demo;

class Core
{
public:

	/*
	** Window
	*/
	GLFWwindow				*window;
	int						windowWidth;
	int						windowHeight;

	/*
	** Matrices
	*/
	Mat4Stack<float>		ms;
	Mat4<float>				projMatrix;
	Mat4<float>				viewMatrix;

	/*
	** Camera
	*/
	Vec3<float>				cameraPos;
	Vec3<float>				cameraLookAt;

	/*
	** Shaders
	*/
	GLuint					vertexShader;
	GLuint					fragmentShader;
	GLuint					program;

	/*
	** Locations
	*/
	GLuint					projLoc;
	GLuint					viewLoc;
	GLuint					objLoc;
	GLuint					positionLoc;
	GLuint					colorLoc;

	/*
	** 3d Axes
	*/
	GLuint					axesVao;
	GLuint					axesVbo[2];

	bool					xAxis;
	bool					yAxis;
	bool					zAxis;

	std::list<t_demo>		axes;

	Core(void);
	~Core(void);

	/* general */
	int						init();
	void					update();
	void					render();
	void					loop();
	void					createAxes();
	void					renderAxes();
	void					pushDemo();
	void					popDemo();

	/* locations */
	void					getLocations();

	/* shaders */
	int						compileShader(GLuint shader, char const *filename);
	GLuint					loadShader(GLenum type, char const *filename);
	int						loadShaders();
	void					attachShaders();
	int						linkProgram();
	void					deleteShaders();
	int						initShaders();

	/* matrices */
	void					setViewMatrix(Mat4<float> &view, Vec3<float> const &dir,
										Vec3<float> const &right, Vec3<float> const &up);
	void					setCamera(Mat4<float> &view, Vec3<float> const &pos, Vec3<float> const &lookAt);
	void					buildProjectionMatrix(Mat4<float> &proj, float const &fov,
												float const &near, float const &far);

	Core &					operator=(Core const &rhs);

private:
	Core(Core const &src);
};

std::ostream &				operator<<(std::ostream &o, Core const &i);

#endif
