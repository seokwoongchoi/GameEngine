#include "Framework.h"
#include "Billboard.h"


Billboard::Billboard(Shader * shader, vector<wstring> & textureNames)
	:Renderer(shader)
{
	Topology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
	
	textures = new TextureArray(textureNames, 256, 256);
	
	sTexture = shader->AsSRV("BillboardMaps");
	
	
}

Billboard::~Billboard()
{
	SafeDelete(textures);


}


void Billboard::Add(Vector3 & position, Vector2 & scale, uint& TextureNum)
{
	VertexScale vertex;
	vertex.position = position;
	vertex.Scale = scale;
	vertex.TextureNum = TextureNum;
	vertices.emplace_back(vertex);
}

void Billboard::CreateBuffer()
{
	SafeDelete(vertexBuffer);
	//assert(vertexBuffer == NULL);
	if (vertices.size() > 0)
		vertexBuffer = new VertexBuffer(&vertices[0], vertices.size(), sizeof(VertexScale));

}

void Billboard::Update()
{
	Super::Update();
	/*Matrix V = Context::Get()->View();
	D3DXMatrixInverse(&V, NULL, &V);

	float x = 0.0f;
	x = asin(-V._32);
	float z = atan2(V._12, V._22);
	float y = atan2(V._31, V._33);
	quad->GetTransform()->Rotation(x, y, z);
	quad->Update();
*/
//이걸로도 구현해보기
//Matrix Matrix::CreateBillboard(Vector3 objectPosition, Vector3 cameraPosition, Vector3 cameraUpVector, Vector3* cameraForwardVector/*=NULL*/)
//{
//	Vector3 forwardVec;
//	forwardVec.X = objectPosition.X - cameraPosition.X;
//	forwardVec.Y = objectPosition.Y - cameraPosition.Y;
//	forwardVec.Z = objectPosition.Z - cameraPosition.Z;

//	float num = forwardVec.LengthSquared();
//	if ((double)num < 9.99999974737875E-05)
//		forwardVec = cameraForwardVector != NULL ? -(*cameraForwardVector) : Vector3::Forward;
//	else
//		forwardVec = forwardVec * (1.0f / (float)sqrt((double)num));

//	Vector3 rightVec;
//	rightVec = Vector3::Cross(cameraUpVector, forwardVec);
//	rightVec.Normalize();

//	Vector3 upVec;
//	upVec = Vector3::Cross(forwardVec, rightVec);

//	Matrix matrix;
//	matrix.M11 = rightVec.X;      matrix.M12 = rightVec.Y;      matrix.M13 = rightVec.Z;      matrix.M14 = 0.0f;
//	matrix.M21 = upVec.X;         matrix.M22 = upVec.Y;         matrix.M23 = upVec.Z;         matrix.M24 = 0.0f;
//	matrix.M31 = forwardVec.X;      matrix.M32 = forwardVec.Y;      matrix.M33 = forwardVec.Z;      matrix.M34 = 0.0f;
//	matrix.M41 = objectPosition.X;   matrix.M42 = objectPosition.Y;   matrix.M43 = objectPosition.Z;   matrix.M44 = 1.0f;

//	return matrix;
}

void Billboard::Render()
{
	Super::Render();

	if (vertexBuffer)
	{
		sTexture->SetResource(textures->SRV());
		
		Topology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
		shader->Draw(0, Pass(), vertices.size());
	}

}