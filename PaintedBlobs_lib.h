#include <vector>

typedef unsigned int GLuint;

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
};