#include <iostream>
#include <random>
#include <format>

#include "imgui.h"
#include "lodepng.h"

#include "geometrizer.h"
#include "PaintedBlobs.h"

PaintedBlobs PB;

char* CurrentImagePath = 0;

int TargetShapeCount = 0;

void GeometrizerMainLoop()
{
	//Reset all if new image loaded
	if (CurrentImagePath)
	{
		//TODO: implement dimension limit, at ~4000x4000 pixels some of the shader logic will start to break
		std::vector<unsigned char> ImageData;
		unsigned int Width, Height;
		lodepng::decode(ImageData, Width, Height, CurrentImagePath);

		PB.LoadImage(ImageData.data(), Width, Height);
		PB.ResetShapes();
		CurrentImagePath = 0;
	}

	ImGui::Begin("Test");
	ImGui::Image(ImTextureID(PB.GetSourceImageTextureID()), ImVec2(PB.GetWidth(), PB.GetHeight()));
	ImGui::SameLine();
	ImGui::Image(ImTextureID(PB.GetCanvasTextureID()), ImVec2(PB.GetWidth(), PB.GetHeight()));
	ImGui::Text("%d", PB.GetCommittedShapeCount());

	auto AddNShapes = [](int n)
	{
		if (ImGui::Button(std::format("Add {} Shape", n).c_str()))
		{
			TargetShapeCount += n;
		}
	};

	AddNShapes(1);
	AddNShapes(10);
	AddNShapes(128);

	if (ImGui::Button("Export PNG"))
	{
		lodepng::encode("output.png", PB.GetPixels(), PB.GetWidth(), PB.GetHeight());
	}

	if (TargetShapeCount > PB.GetCommittedShapeCount())
	{
		PB.AddOneShape();
	}

	ImGui::End();
}

void SetupGLFWCallbacks(GLFWwindow* W)
{
	glfwSetDropCallback(W, [](GLFWwindow* W, int count, const char** paths)
	{
		if (count != 1)
		{
			return;
		}

		CurrentImagePath = (char*)paths[0];
	});

	PB.Initialize();
}
