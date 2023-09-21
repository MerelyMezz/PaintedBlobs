#include <vector>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

#define PI 3.141592653589793

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
	static void CompileShaders();

	~Image();

	void LoadImage(const unsigned char* Data, unsigned int Width, unsigned int Height);
	void EmptyCanvas(unsigned int Width, unsigned int Height, unsigned char R, unsigned char G, unsigned char B, unsigned char A);

	void DeleteGPUTexture();
	void SendToGPU(const unsigned char* Data);

	void DrawSingleShape(ExportShape Shape);
	void DrawFocusArea(float MinX, float MaxX, float MinY, float MaxY);

	unsigned int Width;
	unsigned int Height;

	GLuint GPUTexture = 0;

private:
	static GLuint ReDrawShape;
	static GLuint DrawFocusBorder;
};

class PaintedBlobs
{
public:
	static int InitializeStatic(bool CreateGLFWContext);
	static void TerminateStatic();
	static void CompileShaders();

	void Initialize();
	void LoadImage(const unsigned char* Data, unsigned int Width, unsigned int Height);
	void ResetShapes();
	void DeleteShape(int ShapeIndex);
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

	void SetCommittedShape(int Index, ExportShape NewValue);

	//Getters
	std::vector<unsigned char> GetPixels() const;

	GLuint GetSourceImageTextureID() const;
	GLuint GetCanvasTextureID() const;
	int GetWidth() const;
	int GetHeight() const;

	int GetCommittedShapeCount() const;
	ExportShape GetCommittedShape(int Index) const;

private:
	void RedrawShapeCanvas();

	static GLFWwindow* Window;

	std::vector<ExportShape> CommittedShapes;

	//Parameters
	int InitialShapeCount = 10000;
	int ShapeMutationCount = 10000;

	float InitialShapeMaxSize = 0.3f;
	float SizeMutationScale = 0.05f;
	float PositionMutationScale = 0.05f;
	float AngleMutationScale = (10.0 / 180.0f) * PI;
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
	static GLuint MakeRandomShape;
	static GLuint CountColors;
	static GLuint GetAverageColor;
	static GLuint GetScore;
	static GLuint ReplaceBestShape;
	static GLuint MutateBestShape;
	static GLuint ResetBestShape;
	static GLuint DrawShape;
};