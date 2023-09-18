#include <vector>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"

struct ExportShape
{
	float PosX;
	float PosY;
	float SizeX;
	float SizeY;
	float Angle;
	float ColorR;
	float ColorG;
	float ColorB;

	ExportShape(	float PosX,
					float PosY,
					float SizeX,
					float SizeY,
					float Angle,
					float ColorR,
					float ColorG,
					float ColorB);
};

class Image
{
public:
	void LoadImage(unsigned char* Data, unsigned int Width, unsigned int Height);
	void EmptyCanvas(unsigned int Width, unsigned int Height, unsigned char R, unsigned char G, unsigned char B, unsigned char A);

	void SendToGPU(unsigned char* Data);

	unsigned int Width;
	unsigned int Height;

	GLuint GPUTexture = 0;
};

class PaintedBlobs
{
public:
	static int CreateGLFWContext();
	static void DestroyGLFWContext();

	void Initialize();
	void LoadImage(unsigned char* Data, unsigned int Width, unsigned int Height);
	void ResetShapes();
	void AddOneShape();

	//Setters
	void SetInitialShapeCount(int NewValue);
	void SetShapeMutationCount(int NewValue);

	void SetInitialShapeMaxSize(float NewValue);
	void SetSizeMutationScale(float NewValue);
	void SetPositionMutationScale(float NewValue);
	void SetAngleMutationScale(float NewValue);
	void SetBadCoverExclusionThreshold(float NewValue);

	void SetFocusAreaMinX(float NewValue);
	void SetFocusAreaMaxX(float NewValue);
	void SetFocusAreaMinY(float NewValue);
	void SetFocusAreaMaxY(float NewValue);

	//Getters
	std::vector<unsigned char> GetPixels() const;

	GLuint GetSourceImageTextureID() const;
	GLuint GetCanvasTextureID() const;
	int GetWidth() const;
	int GetHeight() const;

	int GetCommittedShapeCount() const;
	ExportShape GetCommittedShape(int Index) const;

private:
	void RecompileShaders();

	static GLFWwindow* Window;

	std::vector<ExportShape> CommittedShapes;

	//Parameters
	int InitialShapeCount = 10000;
	int ShapeMutationCount = 10000;

	float InitialShapeMaxSize = 0.3f;
	float SizeMutationScale = 0.05f;
	float PositionMutationScale = 0.05f;
	float AngleMutationScale = glm::radians(10.0f);
	float BadCoverExclusionThreshold = 0.05f;

	float FocusAreaMinX = 0.0f;
	float FocusAreaMaxX = 1.0f;
	float FocusAreaMinY = 0.0f;
	float FocusAreaMaxY = 1.0f;

	//Image resources and buffers
	Image SourceImage;
	Image ShapeCanvas;

	GLuint ShapeBuffer;
	GLuint BestShapeBuffer;
	GLuint AtomicCounters;

	//Shader programs
	GLuint MakeRandomShape;
	GLuint CountColors;
	GLuint GetAverageColor;
	GLuint GetScore;
	GLuint ReplaceBestShape;
	GLuint MutateBestShape;
	GLuint ResetBestShape;
	GLuint DrawShape;
};