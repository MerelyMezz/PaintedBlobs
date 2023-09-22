#include <fstream>
#include <iostream>
#include <random>
#include <format>
#include <functional>

#include "imgui.h"
#include "lodepng.h"
#include "json.hpp"

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

//Images and GUI stuff
bool ShouldRedrawImages = true;
int SelectedShape = -1;
ExportShape SelectedShapeData(0.0f, 0.0f ,0.0f ,0.0f ,0.0f ,0.0f ,0.0f, 0.0f);

Image ShapePreview;
Image SoloShapePreview;
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

		// Draw full drawing and individual shapes
		SingleShapeLayers.resize(ShapeCount);

		ShapePreview.EmptyCanvas(256,256, 0,0,0,0);

		for (int i = 0; i < ShapeCount; i++)
		{
			ExportShape Shape = i == SelectedShape ? SelectedShapeData : PB.GetCommittedShape(i);

			SingleShapeLayers[i].EmptyCanvas(100,100, 0,0,0,0);
			SingleShapeLayers[i].DrawSingleShape(Shape);

			ShapePreview.DrawSingleShape(Shape);
		}

		ShapePreview.DrawFocusArea(
			FocusAreaMinX.CommittedSetting,
			FocusAreaMaxX.CommittedSetting,
			FocusAreaMinY.CommittedSetting,
			FocusAreaMaxY.CommittedSetting
		);

		// Draw selected shape only
		SoloShapePreview.EmptyCanvas(256,256, 0,0,0,0);
		if (SelectedShape >= 0 && SelectedShape < PB.GetCommittedShapeCount())
		{
			SoloShapePreview.DrawSingleShape(SelectedShapeData);
		}
	}

	// Draw GUI
	float SliderSpeed = 0.01f;

	ImGui::Begin("Test");

	ImGui::BeginTable("TABLE", 2);
	ImGui::TableNextColumn();

	ImGui::Image(ImTextureID(PB.GetSourceImageTextureID()), ImVec2(ShapePreview.Width, ShapePreview.Height));
	ImGui::SameLine();
	ImGui::Image(ImTextureID(ShapePreview.GPUTexture), ImVec2(ShapePreview.Width, ShapePreview.Height));

	//Selected Soloshape
	bool WasDisabled = false;
	if (SelectedShape < 0)
	{
		ImGui::BeginDisabled();
		WasDisabled = true;
	}

	ImGui::BeginTable("SoloSelectedShape", 2);
	ImGui::TableNextColumn();
	ShouldRedrawImages |= ImGui::DragFloat("Position X", &SelectedShapeData.PosX, SliderSpeed, 0.0f, 1.0f);
	ShouldRedrawImages |= ImGui::DragFloat("Position Y", &SelectedShapeData.PosY, SliderSpeed, 0.0f, 1.0f);
	ShouldRedrawImages |= ImGui::DragFloat("Size X", &SelectedShapeData.SizeX, SliderSpeed, 0.0f, 1.0f);
	ShouldRedrawImages |= ImGui::DragFloat("Size Y", &SelectedShapeData.SizeY, SliderSpeed, 0.0f, 1.0f);
	ShouldRedrawImages |= ImGui::DragFloat("Angle", &SelectedShapeData.Angle, SliderSpeed, 0.0f, 2*PI);
	ShouldRedrawImages |= ImGui::ColorEdit3("ShapeColor", &SelectedShapeData.ColorR);

	if (ImGui::Button("Reset Changes"))
	{
		SelectedShapeData = PB.GetCommittedShape(SelectedShape);
	}

	if (ImGui::Button("Commit Changes"))
	{
		PB.SetCommittedShape(SelectedShape, SelectedShapeData);
	}

	if (ImGui::Button("Delete Shape"))
	{
		PB.DeleteShape(SelectedShape);

		//reselect the same-ish id
		SelectedShape = std::min(SelectedShape, PB.GetCommittedShapeCount() - 1);

		if (SelectedShape >= 0)
		{
			SelectedShapeData = PB.GetCommittedShape(SelectedShape);
		}
	}

	ImGui::TableNextColumn();
	ImGui::Image(ImTextureID(SoloShapePreview.GPUTexture), ImVec2(SoloShapePreview.Width, SoloShapePreview.Height));
	ImGui::EndTable();

	if (WasDisabled)
	{
		ImGui::EndDisabled();
	}

	ImGui::Text("%d", PB.GetCommittedShapeCount());

	//settings
	float NotQuiteZero = 0.002f;

	ImGui::DragInt("Initial Shape Count", &InitialShapeCount.PendingSetting,1.0f, 0, 20000);
	ImGui::DragInt("Shape Mutation Count", &ShapeMutationCount.PendingSetting, 1.0f, 0, 20000);

	ImGui::DragFloat("Initial Shape Max Size", &InitialShapeMaxSize.PendingSetting, SliderSpeed, NotQuiteZero, 1.0f);
	ImGui::DragFloat("Size Mutation Scale", &SizeMutationScale.PendingSetting, SliderSpeed, NotQuiteZero, 1.0f);
	ImGui::DragFloat("Position Mutation Scale", &PositionMutationScale.PendingSetting, SliderSpeed, NotQuiteZero, 1.0f);
	ImGui::DragFloat("Angle Mutation Scale", &AngleMutationScale.PendingSetting, SliderSpeed, NotQuiteZero, 360.0f);
	ImGui::DragFloat("Bad Cover Exclusion Threshold", &BadCoverExclusionThreshold.PendingSetting, SliderSpeed, NotQuiteZero, 1.0f);

	ImGui::DragFloat("Focus Area Min X", &FocusAreaMinX.PendingSetting, SliderSpeed, 0.0f, 1.0f);
	ImGui::DragFloat("Focus Area Max X", &FocusAreaMaxX.PendingSetting, SliderSpeed, 0.0f, 1.0f);
	ImGui::DragFloat("Focus Area Min Y", &FocusAreaMinY.PendingSetting, SliderSpeed, 0.0f, 1.0f);
	ImGui::DragFloat("Focus Area Max Y", &FocusAreaMaxY.PendingSetting, SliderSpeed, 0.0f, 1.0f);

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

	if (ImGui::Button("Export to Tw1ddle JSON"))
	{
		int FinalScale = 1024;

		nlohmann::json J;
		J.emplace_back( nlohmann::json({{"type", 0}, {"data", {0,0,FinalScale,FinalScale}}, {"color", {0,0,0,0}}}) );

		for (int i = 0; i < PB.GetCommittedShapeCount(); i++)
		{
			ExportShape CurrentShape = PB.GetCommittedShape(i);

			int PosX = CurrentShape.PosX * FinalScale;
			int PosY = CurrentShape.PosY * FinalScale;
			int SizeX = CurrentShape.SizeX * FinalScale;
			int SizeY = CurrentShape.SizeY * FinalScale;
			int Angle = (CurrentShape.Angle / PI) * 180.0f;

			int ColorR = CurrentShape.ColorR * 255;
			int ColorG = CurrentShape.ColorG * 255;
			int ColorB = CurrentShape.ColorB * 255;

			J.emplace_back( nlohmann::json({{"type", 4}, {"data", {PosX,PosY,SizeX,SizeY,Angle}}, {"color", {ColorR,ColorG,ColorB,255}}}) );
		}

		std::ofstream F;
		F.open("output.json");
		F<<J;
		F.close();
	}

	ImGui::TableNextColumn();
	ImGui::BeginChild("SoloLayer");
	ImGui::BeginTable("SoloLayerEntries", 1);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(1.f, 1.f, 1.f, 0.1f));
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1.f, 1.f, 1.f, 0.25f));

	//Draw single layers
	for (int i = SingleShapeLayers.size() - 1; i >= 0; i--)
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn();

		unsigned int SelectColor = 0x80ffffff;
		ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, i == SelectedShape ? SelectColor : 0);

		if (ImGui::ImageButton(ImTextureID(SingleShapeLayers[i].GPUTexture), ImVec2(SingleShapeLayers[i].Width, SingleShapeLayers[i].Height)))
		{
			SelectedShape = i;
			SelectedShapeData = PB.GetCommittedShape(i);
		}
	}

	ImGui::PopStyleColor(3);

	ImGui::EndTable();
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
