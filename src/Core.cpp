
#include "Core.hpp"

Core::Core()
{
	return ;
}

Core::~Core()
{
	glfwDestroyWindow(this->window);
	glfwTerminate();
	return ;
}

static void
key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	Core		*core = static_cast<Core *>(glfwGetWindowUserPointer(window));

	(void)scancode;
	(void)mods;
	(void)core;
	if (action == GLFW_PRESS)
	{
		if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, GL_TRUE);
		else if (key == GLFW_KEY_KP_ADD)
			core->pushDemo();
		else if (key == GLFW_KEY_KP_SUBTRACT)
			core->popDemo();
		else if (key == GLFW_KEY_X)
			core->xAxis = !core->xAxis;
		else if (key == GLFW_KEY_Y)
			core->yAxis = !core->yAxis;
		else if (key == GLFW_KEY_Z)
			core->zAxis = !core->zAxis;
	}
}

void
Core::buildProjectionMatrix(Mat4<float> &proj, float const &fov,
							float const &near, float const &far)
{
	float const			f = 1.0f / tan(fov * (M_PI / 360.0));
	float const			ratio = (1.0f * this->windowWidth) / this->windowHeight;

	/*
	1 0 0 0
	0 1 0 0
	0 0 1 0
	0 0 0 1
	*/
	proj.setIdentity();
	/*
	a 0 0 0
	0 b e 0
	0 0 c f
	0 0 d 1
	*/
	proj[0] = f / ratio; // a
	proj[1 * 4 + 1] = f; // b
	proj[2 * 4 + 2] = (far + near) / (near - far); // c
	proj[3 * 4 + 2] = (2.0f * far * near) / (near - far); // d
	proj[2 * 4 + 3] = -1.0f; // e
	proj[3 * 4 + 3] = 0.0f; // f
}

void
Core::setViewMatrix(Mat4<float> &view, Vec3<float> const &dir,
					Vec3<float> const &right, Vec3<float> const &up)
{
	/*
	rx		ux		-dx		0
	ry		uy		-dy		0
	rz		uz		-dz		0
	0		0		0		1
	*/
	// first column
	view[0] = right.x;
	view[4] = right.y;
	view[8] = right.z;
	view[12] = 0.0f;
	// second column
	view[1] = up.x;
	view[5] = up.y;
	view[9] = up.z;
	view[13] = 0.0f;
	// third column
	view[2] = -dir.x;
	view[6] = -dir.y;
	view[10] = -dir.z;
	view[14] = 0.0f;
	// fourth column
	view[3] = 0.0f;
	view[7] = 0.0f;
	view[11] = 0.0f;
	view[15] = 1.0f;
}

void
Core::setCamera(Mat4<float> &view, Vec3<float> const &pos, Vec3<float> const &lookAt)
{
	Vec3<float>		dir;
	Vec3<float>		right;
	Vec3<float>		up;
	Mat4<float>		translation;

	up.set(0.0f, 1.0f, 0.0f);
	dir.set(lookAt - pos);
	dir.normalize();
	right.crossProduct(dir, up);
	right.normalize();
	up.crossProduct(right, dir);
	up.normalize();
	this->setViewMatrix(view, dir, right, up);
	translation.setTranslation(-pos.x, -pos.y, -pos.z);
	view.multiply(translation);
}

int
Core::compileShader(GLuint shader, char const *filename)
{
	GLint			logsize;
	GLint			state;
	char			*compileLog;

	glCompileShader(shader);
	glGetShaderiv(shader, GL_COMPILE_STATUS, &state);
	if (state != GL_TRUE)
	{
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logsize);
		compileLog = new char[logsize + 1];
		std::memset(compileLog, '\0', logsize + 1);
		glGetShaderInfoLog(shader, logsize, &logsize, compileLog);
		std::cerr	<< "Failed to compile shader `"
					<< filename
					<< "`: " << std::endl
					<< compileLog;
		delete compileLog;
		return (0);
	}
	return (1);
}

GLuint
Core::loadShader(GLenum type, char const *filename)
{
	GLuint			shader;
	char			*source;

	shader = glCreateShader(type);
	if (shader == 0)
		return (printError("Failed to create shader !", 0));
	if (!(source = readFile(filename)))
		return (printError("Failed to read file !", 0));
	glShaderSource(shader, 1, (char const **)&source, 0);
	if (!compileShader(shader, filename))
		return (0);
	delete source;
	return (shader);
}

int
Core::loadShaders()
{
	if (!(this->vertexShader = this->loadShader(GL_VERTEX_SHADER, "./shaders/vertex_shader.gls")))
		return (printError("Failed to load vertex shader !", 0));
	if (!(this->fragmentShader = this->loadShader(GL_FRAGMENT_SHADER, "./shaders/fragment_shader.gls")))
		return (printError("Failed to load fragment shader !", 0));
	return (1);
}

void
Core::attachShaders()
{
	glAttachShader(this->program, this->vertexShader);
	glAttachShader(this->program, this->fragmentShader);
}

int
Core::linkProgram()
{
	GLint			logSize;
	GLint			state;
	char			*linkLog;

	glLinkProgram(this->program);
	glGetProgramiv(this->program, GL_LINK_STATUS, &state);
	if (state != GL_TRUE)
	{
		glGetProgramiv(this->program, GL_INFO_LOG_LENGTH, &logSize);
		linkLog = new char[logSize + 1];
		std::memset(linkLog, '\0', logSize + 1);
		glGetProgramInfoLog(this->program, logSize, &logSize, linkLog);
		std::cerr	<< "Failed to link program !" << std::endl
					<< linkLog;
		delete [] linkLog;
		return (0);
	}
	return (1);
}

void
Core::deleteShaders()
{
	glDeleteShader(this->vertexShader);
	glDeleteShader(this->fragmentShader);
}

int
Core::initShaders()
{
	if (!loadShaders())
		return (0);
	if (!(this->program = glCreateProgram()))
		return (printError("Failed to create program !", 0));
	this->attachShaders();
	glBindFragDataLocation(this->program, 0, "out_fragment");
	if (!this->linkProgram())
		return (0);
	this->deleteShaders();
	return (1);
}

void
Core::getLocations()
{
	this->positionLoc = glGetAttribLocation(this->program, "position");
	this->colorLoc = glGetAttribLocation(this->program, "vert_color");
	this->projLoc = glGetUniformLocation(this->program, "proj_matrix");
	this->viewLoc = glGetUniformLocation(this->program, "view_matrix");
	this->objLoc = glGetUniformLocation(this->program, "obj_matrix");
}

void
Core::createAxes()
{
	static GLfloat const		axesVertices[36] =
	{
		0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f
	};
	static GLushort const		axesIndices[6] =
	{
		0, 1,
		2, 3,
		4, 5
	};

	glGenVertexArrays(1, &axesVao);
	glBindVertexArray(axesVao);
	glGenBuffers(2, &axesVbo[0]);
	glBindBuffer(GL_ARRAY_BUFFER, axesVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 36, axesVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(positionLoc);
	glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void *)0);
	glEnableVertexAttribArray(colorLoc);
	glVertexAttribPointer(colorLoc, 3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 6, (void *)(sizeof(GLfloat) * 3));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, axesVbo[1]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLushort) * 6, axesIndices, GL_STATIC_DRAW);

	xAxis = false;
	yAxis = false;
	zAxis = false;
}

int
Core::init()
{
	this->windowWidth = 1280;
	this->windowHeight = 1280;
	if (!glfwInit())
		return (0);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	this->window = glfwCreateWindow(this->windowWidth, this->windowHeight,
									"demo", NULL, NULL);
	if (!this->window)
	{
		glfwTerminate();
		return (0);
	}
	glfwSetWindowUserPointer(this->window, this);
	glfwMakeContextCurrent(this->window); // make the opengl context of the window current on the main thread
	glfwSwapInterval(1); // VSYNC 60 fps max
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glEnable(GL_DEPTH_TEST);
	if (!this->initShaders())
		return (0);
	this->getLocations();
	this->buildProjectionMatrix(this->projMatrix, 53.13f, 0.1f, 1000.0f);
	this->cameraPos.set(3.5f, 3.5f, 3.5f);
	this->cameraLookAt.set(0.0f, 0.0f, 0.0f);
	this->setCamera(this->viewMatrix, this->cameraPos, this->cameraLookAt);
	glfwSetKeyCallback(this->window, key_callback);
	createAxes();
	pushDemo();
	return (1);
}

void
Core::update()
{
	if (glfwGetKey(this->window, GLFW_KEY_ESCAPE))
		glfwSetWindowShouldClose(this->window, GL_TRUE);
	if (glfwGetKey(this->window, GLFW_KEY_1))
	{
		axes.front().translation.x += 0.05f * xAxis;
		axes.front().translation.y += 0.05f * yAxis;
		axes.front().translation.z += 0.05f * zAxis;
	}
	else if (glfwGetKey(this->window, GLFW_KEY_2))
	{
		axes.front().translation.x -= 0.05f * xAxis;
		axes.front().translation.y -= 0.05f * yAxis;
		axes.front().translation.z -= 0.05f * zAxis;
	}
	else if (glfwGetKey(this->window, GLFW_KEY_3))
	{
		axes.front().rotation.x += 2 * xAxis;
		axes.front().rotation.y += 2 * yAxis;
		axes.front().rotation.z += 2 * zAxis;
	}
	else if (glfwGetKey(this->window, GLFW_KEY_4))
	{
		axes.front().rotation.x -= 2 * xAxis;
		axes.front().rotation.y -= 2 * yAxis;
		axes.front().rotation.z -= 2 * zAxis;
	}
	else if (glfwGetKey(this->window, GLFW_KEY_5))
	{
		if (xAxis)
			axes.front().scale.x += 0.01f;
		if (yAxis)
			axes.front().scale.y += 0.01f;
		if (zAxis)
			axes.front().scale.z += 0.01f;
	}
	else if (glfwGetKey(this->window, GLFW_KEY_6))
	{
		if (xAxis)
			axes.front().scale.x -= 0.01f;
		if (yAxis)
			axes.front().scale.y -= 0.01f;
		if (zAxis)
			axes.front().scale.z -= 0.01f;
	}
}

void
Core::render()
{
	glUseProgram(this->program);
	glUniformMatrix4fv(this->projLoc, 1, GL_FALSE, this->projMatrix.val);
	glUniformMatrix4fv(this->viewLoc, 1, GL_FALSE, this->viewMatrix.val);
	renderAxes();
	checkGlError(__FILE__, __LINE__);
}

void
Core::pushDemo()
{
	t_demo		first;

	first.rotation.set(0.0f, 0.0f, 0.0f);
	first.translation.set(0.0f, 0.0f, 0.0f);
	first.scale.set(1.0f, 1.0f, 1.0f);

	axes.push_front(first);
}

void
Core::popDemo()
{
	if (axes.size() > 1)
		axes.pop_front();
}

void
Core::renderAxes()
{
	std::list<t_demo>::iterator		it;
	std::list<t_demo>::iterator		ite;

	ite = axes.end();
	it = axes.begin();
	while (it != ite)
	{
		ms.push();
		ms.translate((*it).translation);
		ms.rotate((*it).rotation.x, 1.0f, 0.0f, 0.0f);
		ms.rotate((*it).rotation.y, 0.0f, 1.0f, 0.0f);
		ms.rotate((*it).rotation.z, 0.0f, 0.0f, 1.0f);
		ms.scale((*it).scale);
		glUniformMatrix4fv(objLoc, 1, GL_FALSE, ms.top().val);
		glBindVertexArray(axesVao);
		glBindBuffer(GL_ARRAY_BUFFER, axesVbo[0]);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, axesVbo[1]);
		glDrawElements(GL_LINES, 6, GL_UNSIGNED_SHORT, (void *)(sizeof(GLfloat) * 0));
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindTexture(GL_TEXTURE_2D, 0);
		it++;
	}

	it = axes.begin();
	while (it != ite)
	{
		ms.pop();
		it++;
	}
}

void
Core::loop()
{
	while (!glfwWindowShouldClose(this->window))
	{
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		this->update();
		this->render();
		glfwSwapBuffers(this->window);
		glfwPollEvents();
	}
}

Core &
Core::operator=(Core const &rhs)
{
	if (this != &rhs)
	{
		// copy members here
	}
	return (*this);
}

std::ostream &
operator<<(std::ostream &o, Core const &i)
{
	o	<< "Core: " << &i;
	return (o);
}
