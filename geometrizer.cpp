#include <iostream>
#include <random>
#include <format>
#include <functional>

#include "imgui.h"
#include "lodepng.h"

#include "geometrizer.h"
#include "PaintedBlobs.h"

template <typename T>
struct ConfigSetting
{
	T PendingSetting;
	T CommittedSetting;

	std::function<void(T)> CommitSettingFunc;

	void CheckSetting()
	{
		if (PendingSetting == CommittedSetting)
		{
			return;
		}
		CommittedSetting = PendingSetting;
		CommitSettingFunc(CommittedSetting);
	}

	ConfigSetting(T InitialSetting, std::function<void(T)> CommitSettingFunc)
	{
		PendingSetting = InitialSetting;
		CommittedSetting = InitialSetting;
		this->CommitSettingFunc = CommitSettingFunc;
	}
};

PaintedBlobs PB;

ConfigSetting<int> InitialShapeCount(10000, [](int I){PB.SetInitialShapeCount(I);});
ConfigSetting<int> ShapeMutationCount(10000, [](int I){PB.SetShapeMutationCount(I);});

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

	//settings
	ImGui::DragInt("Initial Shape Count: ", &InitialShapeCount.PendingSetting,1.0f, 0, 20000);
	ImGui::DragInt("Shape Mutation Count: ", &ShapeMutationCount.PendingSetting, 1.0f, 0, 20000);

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

	// Do stuff
	InitialShapeCount.CheckSetting();
	ShapeMutationCount.CheckSetting();

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
