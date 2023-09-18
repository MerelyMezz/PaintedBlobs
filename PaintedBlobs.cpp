#include "PaintedBlobs.h"

#include <iostream>
#include <random>

std::random_device RandomDevice;
std::mt19937_64 MT(RandomDevice());
std::uniform_real_distribution<float> dist(0.0f, 1.0f);

ExportShape::ExportShape(	float PosX,
							float PosY,
							float SizeX,
							float SizeY,
							float Angle,
							float ColorR,
							float ColorG,
							float ColorB)
{
	this->PosX = PosX;
	this->PosY = PosY;
	this->SizeX = SizeX;
	this->SizeY = SizeY;
	this->Angle = Angle;
	this->ColorR = ColorR;
	this->ColorG = ColorG;
	this->ColorB = ColorB;
}

void Image::LoadImage(unsigned char* Data, unsigned int Width, unsigned int Height)
{
	//TODO: crash when too many pixels

	this->Width = Width;
	this->Height = Height;

	SendToGPU(Data);
}

void Image::EmptyCanvas(unsigned int Width, unsigned int Height, unsigned char R, unsigned char G, unsigned char B, unsigned char A)
{
	this->Width = Width;
	this->Height = Height;

	std::vector<unsigned char> ImageData;
	ImageData.resize(Width*Height*4);
	for (int i = 0; i < Width*Height; i++)
	{
		int Base = i*4;
		ImageData[Base] = R;
		ImageData[Base+1] = G;
		ImageData[Base+2] = B;
		ImageData[Base+3] = A;
	}
	SendToGPU(ImageData.data());
}

void Image::SendToGPU(unsigned char* Data)
{
	//Free previous texture
	if (GPUTexture)
	{
		glDeleteTextures(1, &GPUTexture);
	}

	glGenTextures(1, &GPUTexture);
	glBindTexture(GL_TEXTURE_2D, GPUTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, Data);
	glBindTexture(GL_TEXTURE_2D, 0);
}


GLFWwindow* PaintedBlobs::Window;

int PaintedBlobs::CreateGLFWContext()
{
	if (!glfwInit())
	{
		glfwTerminate();
		return 1;
	}

	glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
	Window = glfwCreateWindow(1024, 1024, "Test", nullptr, nullptr);
	glfwMakeContextCurrent(Window);

	if (glewInit() != GLEW_OK)
	{
		return 1;
	}

	return 0;
}

void PaintedBlobs::DestroyGLFWContext()
{
	glfwDestroyWindow(Window);
	glfwTerminate();
}

void PaintedBlobs::Initialize()
{
	auto CompileShader = [](const char* SourceCode)
	{
		GLuint Shader = glCreateShader(GL_COMPUTE_SHADER);
		glShaderSource(Shader, 1, &SourceCode, nullptr);
		glCompileShader(Shader);

		GLint success = 0;
		glGetShaderiv(Shader, GL_COMPILE_STATUS, &success);
		if (success == GL_FALSE)
		{
			char CompileError[2048];
			glGetShaderInfoLog(Shader, 2048, NULL, CompileError);
			std::cerr<<"Shader compilation failed"<<std::endl<<CompileError<<std::endl;
		}

		GLuint Program = glCreateProgram();
		glAttachShader(Program, Shader);
		glLinkProgram(Program);

		glGetProgramiv(Program, GL_LINK_STATUS, &success);
		if(!success)
		{
			char CompileError[2048];
			glGetProgramInfoLog(Program, 2048, NULL, CompileError);
			std::cerr<<"Program link failed"<<std::endl<<CompileError<<std::endl;
		}

		return Program;
	};

	//Compile programs
	MakeRandomShape = CompileShader(
		#include "MakeRandomShape.glsl"
	);

	CountColors = CompileShader(
		#include "CountColors.glsl"
	);

	GetAverageColor = CompileShader(
		#include "GetAverageColor.glsl"
	);

	GetScore = CompileShader(
		#include "GetScore.glsl"
	);

	ReplaceBestShape = CompileShader(
		#include "ReplaceBestShape.glsl"
	);

	MutateBestShape = CompileShader(
		#include "MutateBestShape.glsl"
	);

	ResetBestShape = CompileShader(
		#include "ResetBestShape.glsl"
	);

	DrawShape = CompileShader(
		#include "DrawShape.glsl"
	);

	auto MakeBuffer = [](GLuint* Buffer, int size, GLuint type, GLuint flags)
	{
		glGenBuffers(1, Buffer);
		glBindBuffer(type, *Buffer);
		glBufferStorage(type, 4 * size, nullptr, flags);
		glBindBuffer(type, 0);
	};

	//Generate buffers
	MakeBuffer(&ShapeBuffer, 24, GL_SHADER_STORAGE_BUFFER, GL_MAP_READ_BIT);
	MakeBuffer(&BestShapeBuffer, 24, GL_SHADER_STORAGE_BUFFER, GL_MAP_READ_BIT);
	MakeBuffer(&AtomicCounters, 6, GL_ATOMIC_COUNTER_BUFFER, GL_MAP_READ_BIT);
}

void PaintedBlobs::LoadImage(unsigned char* Data, unsigned int Width, unsigned int Height)
{
	SourceImage.LoadImage(Data, Width, Height);
}

void PaintedBlobs::ResetShapes()
{
	ShapeCanvas.EmptyCanvas(SourceImage.Width, SourceImage.Height, 0,0,0,0);
	CommittedShapes.clear();
}

void PaintedBlobs::AddOneShape()
{
	auto EvaluateCurrentShape = [&]()
	{
		// run shader that: determines average color of shape, saves into atomic counters
		glUseProgram(CountColors);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ShapeBuffer);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, AtomicCounters);
		glBindImageTexture(2, SourceImage.GPUTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
		glDispatchCompute(SourceImage.Width, SourceImage.Height,1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// run shader that: turns atomic counters into average color
		glUseProgram(GetAverageColor);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ShapeBuffer);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, AtomicCounters);
		glBindImageTexture(2, SourceImage.GPUTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
		glDispatchCompute(1,1,1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);


		// run shader that: determines score of current shape
		glUseProgram(GetScore);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ShapeBuffer);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, AtomicCounters);
		glBindImageTexture(2, SourceImage.GPUTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
		glBindImageTexture(3, ShapeCanvas.GPUTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
		glDispatchCompute(SourceImage.Width, SourceImage.Height,1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		// overwrite best shape, if high enough score
		glUseProgram(ReplaceBestShape);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ShapeBuffer);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, AtomicCounters);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, BestShapeBuffer);
		glDispatchCompute(1,1,1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	};

	glUseProgram(ResetBestShape);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, BestShapeBuffer);
	glDispatchCompute(1,1,1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	for (int i = 0; i < InitialShapeCount; i++)
	{
		// for N times:
		// run shader that: generates random shape, puts data and matrix into buffer
		glUseProgram(MakeRandomShape);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ShapeBuffer);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, AtomicCounters);
		glBindImageTexture(2, SourceImage.GPUTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
		float RandomNumbers[5] = {dist(MT),dist(MT),dist(MT),dist(MT),dist(MT)};
		glUniform1fv(0, 5, RandomNumbers);
		glDispatchCompute(1,1,1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		EvaluateCurrentShape();
	}

	for (int i = 0; i < ShapeMutationCount; i++)
	{
		// mutate best shape
		glUseProgram(MutateBestShape);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ShapeBuffer);
		glBindBufferBase(GL_ATOMIC_COUNTER_BUFFER, 1, AtomicCounters);
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, BestShapeBuffer);
		glBindImageTexture(3, SourceImage.GPUTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
		float RandomNumbers[5] = {dist(MT),dist(MT),dist(MT),dist(MT),dist(MT)};
		glUniform1fv(0, 5, RandomNumbers);
		glDispatchCompute(1,1,1);
		glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

		EvaluateCurrentShape();
	}

	glUseProgram(DrawShape);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, BestShapeBuffer);
	glBindImageTexture(3, ShapeCanvas.GPUTexture, 0, GL_TRUE, 0, GL_READ_ONLY, GL_RGBA32F);
	glDispatchCompute(ShapeCanvas.Width, ShapeCanvas.Height, 1);
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

	//GLsync SyncObj = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);
	//glClientWaitSync(SyncObj, 0, 100000000);

	float BestShapeData[24];
	glGetNamedBufferSubData(BestShapeBuffer, 0, 24*4, BestShapeData);


	int Score = *(int*)&BestShapeData[23];

	if (Score > 0)
	{
		float PosX = BestShapeData[0];
		float PosY = BestShapeData[1];
		float SizeX = BestShapeData[2];
		float SizeY = BestShapeData[3];
		float Angle = BestShapeData[4];
		float ColorR = BestShapeData[20];
		float ColorG = BestShapeData[21];
		float ColorB = BestShapeData[22];

		CommittedShapes.push_back(ExportShape(PosX, PosY, SizeX, SizeY, Angle, ColorR, ColorG, ColorB));
	}
}

void PaintedBlobs::SetInitialShapeCount(int NewValue)
{
	InitialShapeCount = NewValue;
}

void PaintedBlobs::SetShapeMutationCount(int NewValue)
{
	ShapeMutationCount = NewValue;
}

void PaintedBlobs::SetInitialShapeMaxSize(float NewValue)
{
	InitialShapeMaxSize = NewValue;
}

void PaintedBlobs::SetSizeMutationScale(float NewValue)
{
	SizeMutationScale = NewValue;
}

void PaintedBlobs::SetPositionMutationScale(float NewValue)
{
	PositionMutationScale = NewValue;
}

void PaintedBlobs::SetAngleMutationScale(float NewValue)
{
	AngleMutationScale = glm::radians(NewValue);
}

void PaintedBlobs::SetBadCoverExclusionThreshold(float NewValue)
{
	BadCoverExclusionThreshold = NewValue;
}

void PaintedBlobs::SetFocusAreaMinX(float NewValue)
{
	FocusAreaMinX = NewValue;
}

void PaintedBlobs::SetFocusAreaMaxX(float NewValue)
{
	FocusAreaMaxX = NewValue;
}

void PaintedBlobs::SetFocusAreaMinY(float NewValue)
{
	FocusAreaMinY = NewValue;
}

void PaintedBlobs::SetFocusAreaMaxY(float NewValue)
{
	FocusAreaMaxY = NewValue;
}

std::vector<unsigned char> PaintedBlobs::GetPixels() const
{
	std::vector<unsigned char> Pixels;
	Pixels.resize(ShapeCanvas.Width*ShapeCanvas.Height*4);

	glGetTextureImage(ShapeCanvas.GPUTexture, 0, GL_RGBA, GL_UNSIGNED_BYTE, Pixels.size(), Pixels.data());
	return Pixels;
}

GLuint PaintedBlobs::GetSourceImageTextureID() const
{
	return SourceImage.GPUTexture;
}

GLuint PaintedBlobs::GetCanvasTextureID() const
{
	return ShapeCanvas.GPUTexture;
}

int PaintedBlobs::GetWidth() const
{
	return SourceImage.Width;
}

int PaintedBlobs::GetHeight() const
{
	return SourceImage.Height;
}

int PaintedBlobs::GetCommittedShapeCount() const
{
	return CommittedShapes.size();
}

ExportShape PaintedBlobs::GetCommittedShape(int Index) const
{
	return CommittedShapes[Index];
}