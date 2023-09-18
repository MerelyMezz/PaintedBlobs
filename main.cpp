#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include <iostream>

#include "geometrizer.h"

int main()
{
	//imgui basic window code stolen from: https://blog.conan.io/2019/06/26/An-introduction-to-the-Dear-ImGui-library.html
	//This is all boiler plate, the meat and potatoes are in geometrizer.cpp

	if (!glfwInit())
	{
		glfwTerminate();
		return 1;
	}

	GLFWwindow* W = glfwCreateWindow(1024, 1024, "Test", nullptr, nullptr);
	glfwMakeContextCurrent(W);


	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO &io = ImGui::GetIO();

	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(W, true);
	ImGui_ImplOpenGL3_Init();

	if (glewInit() != GLEW_OK)
	{
		return 1;
	}

	SetupGLFWCallbacks(W);

	while (!glfwWindowShouldClose(W))
	{
		glfwPollEvents();

		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);

		GeometrizerMainLoop();

		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
		glfwSwapBuffers(W);
	}

	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwDestroyWindow(W);
	glfwTerminate();

	return 0;
}
