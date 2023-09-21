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

//GUI stuff
#define CONFIG_SETTING(Type, Name, Default) ConfigSetting<Type> Name(Default, [](Type I){PB.Set##Name(I);});

CONFIG_SETTING(int, InitialShapeCount, 10000);
CONFIG_SETTING(int, ShapeMutationCount, 10000);

CONFIG_SETTING(float, InitialShapeMaxSize, 0.3f);
CONFIG_SETTING(float, SizeMutationScale, 0.05f);
CONFIG_SETTING(float, PositionMutationScale, 0.05f);
CONFIG_SETTING(float, AngleMutationScale, 10.0f);
CONFIG_SETTING(float, BadCoverExclusionThreshold, 0.05f);

CONFIG_SETTING(float, FocusAreaMinX, 0.0f);
CONFIG_SETTING(float, FocusAreaMaxX, 1.0f);
CONFIG_SETTING(float, FocusAreaMinY, 0.0f);
CONFIG_SETTING(float, FocusAreaMaxY, 1.0f);

char* CurrentImagePath = 0;
int TargetShapeCount = 0;

//Images
bool ShouldRedrawImages = true;

Image ShapePreview;
std::vector<Image> SingleShapeLayers;

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

	//Redraw any images that may be used by imgui
	if (ShouldRedrawImages)
	{
		int ShapeCount = PB.GetCommittedShapeCount();

		SingleShapeLayers.resize(ShapeCount);

		ShapePreview.EmptyCanvas(256,256, 0,0,0,0);

		for (int i = 0; i < ShapeCount; i++)
		{
			SingleShapeLayers[i].EmptyCanvas(100,100, 0,0,0,0);
			SingleShapeLayers[i].DrawSingleShape(PB.GetCommittedShape(i));

			ShapePreview.DrawSingleShape(PB.GetCommittedShape(i));
		}
	}

	ImGui::Begin("Test");

	ImGui::BeginTable("TABLE", 2);
	ImGui::TableNextColumn();

	ImGui::Image(ImTextureID(PB.GetSourceImageTextureID()), ImVec2(ShapePreview.Width, ShapePreview.Height));
	ImGui::SameLine();
	//ImGui::Image(ImTextureID(PB.GetCanvasTextureID()), ImVec2(PB.GetWidth(), PB.GetHeight()));
	ImGui::Image(ImTextureID(ShapePreview.GPUTexture), ImVec2(ShapePreview.Width, ShapePreview.Height));

	ImGui::Text("%d", PB.GetCommittedShapeCount());

	//settings
	float NotQuiteZero = 0.002f;
	ImGui::DragInt("Initial Shape Count", &InitialShapeCount.PendingSetting,1.0f, 0, 20000);
	ImGui::DragInt("Shape Mutation Count", &ShapeMutationCount.PendingSetting, 1.0f, 0, 20000);

	ImGui::DragFloat("Initial Shape Max Size", &InitialShapeMaxSize.PendingSetting, 1.0f, NotQuiteZero, 1.0f);
	ImGui::DragFloat("Size Mutation Scale", &SizeMutationScale.PendingSetting, 1.0f, NotQuiteZero, 1.0f);
	ImGui::DragFloat("Position Mutation Scale", &PositionMutationScale.PendingSetting, 1.0f, NotQuiteZero, 1.0f);
	ImGui::DragFloat("Angle Mutation Scale", &AngleMutationScale.PendingSetting, 1.0f, NotQuiteZero, 360.0f);
	ImGui::DragFloat("Bad Cover Exclusion Threshold", &BadCoverExclusionThreshold.PendingSetting, 1.0f, NotQuiteZero, 1.0f);

	ImGui::DragFloat("Focus Area Min X", &FocusAreaMinX.PendingSetting, 1.0f, 0.0f, 1.0f);
	ImGui::DragFloat("Focus Area Max X", &FocusAreaMaxX.PendingSetting, 1.0f, 0.0f, 1.0f);
	ImGui::DragFloat("Focus Area Min Y", &FocusAreaMinY.PendingSetting, 1.0f, 0.0f, 1.0f);
	ImGui::DragFloat("Focus Area Max Y", &FocusAreaMaxY.PendingSetting, 1.0f, 0.0f, 1.0f);

	auto AddNShapes = [](int n)
	{
		if (ImGui::Button(std::format("Add {} Shape", n).c_str()))
		{
			TargetShapeCount += n;
		}
	};

	if (ImGui::Button("Stop making shapes"))
	{
		TargetShapeCount = 0;
	}

	AddNShapes(1);
	AddNShapes(10);
	AddNShapes(50);
	AddNShapes(128);

	if (ImGui::Button("Export PNG"))
	{
		lodepng::encode("output.png", PB.GetPixels(), PB.GetWidth(), PB.GetHeight());
	}

	ImGui::TableNextColumn();

	ImGui::BeginChild("SoloLayer");

	//Draw single layers
	for (int i = SingleShapeLayers.size() - 1; i >= 0; i--)
	{
		ImGui::Image(ImTextureID(SingleShapeLayers[i].GPUTexture), ImVec2(SingleShapeLayers[i].Width, SingleShapeLayers[i].Height));
		ImGui::SameLine();

		if (ImGui::Button(std::format("Delete shape #{}", i).c_str()))
		{
			PB.DeleteShape(i);
			ShouldRedrawImages = true;
		}
	}

	ImGui::EndChild();
	ImGui::EndTable();

	ImGui::End();

	// Do stuff
	InitialShapeCount.CheckSetting();
	ShapeMutationCount.CheckSetting();

	InitialShapeMaxSize.CheckSetting();
	SizeMutationScale.CheckSetting();
	PositionMutationScale.CheckSetting();
	AngleMutationScale.CheckSetting();
	BadCoverExclusionThreshold.CheckSetting();

	FocusAreaMinX.CheckSetting();
	FocusAreaMaxX.CheckSetting();
	FocusAreaMinY.CheckSetting();
	FocusAreaMaxY.CheckSetting();

	if (TargetShapeCount > 0)
	{
		PB.AddOneShape();
		TargetShapeCount--;
		ShouldRedrawImages = true;
	}

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

	PaintedBlobs::InitializeStatic(false);

	PB.Initialize();
}
