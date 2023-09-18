#include <vector>

#include "GL/glew.h"
#include "GLFW/glfw3.h"

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

	std::vector<unsigned char> GetPixels() const;

	GLuint GetSourceImageTextureID() const;
	GLuint GetCanvasTextureID() const;
	int GetWidth() const;
	int GetHeight() const;

	int GetCommittedShapeCount() const;
	ExportShape GetCommittedShape(int Index) const;

private:
	static GLFWwindow* Window;

	std::vector<ExportShape> CommittedShapes;

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