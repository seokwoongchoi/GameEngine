#include "Framework.h"
#include "Transform.h"

Transform::Transform()
	: shader(NULL), buffer(NULL)
	, position(0, 0, 0), scale(1, 1, 1), rotation(0,0, 0, 0), bParent(false), parent(nullptr)
{
	D3DXMatrixIdentity(&bufferDesc.World);
	
}

Transform::Transform(Shader * shader)
	: position(0, 0, 0), scale(1, 1, 1), rotation(0, 0, 0,0), bParent(false), parent(nullptr)
{
	SetShader(shader);
	
	D3DXMatrixIdentity(&bufferDesc.World);
}

Transform::~Transform()
{
	SafeDelete(buffer);
}

void Transform::Set(Transform * transform)
{
	position = transform->position;
	scale = transform->scale;
	rotation = transform->rotation;

	//UpdateWorld();
}

void Transform::SetShader(Shader* shader)
{
	this->shader = shader;

	buffer = new ConstantBuffer(&bufferDesc, sizeof(BufferDesc));
	sBuffer = shader->AsConstantBuffer("CB_World");
}

void Transform::Position(float x, float y, float z)
{
	Position(Vector3(x, y, z));
}

void Transform::Position(Vector3 & vec)
{
	
	if (HasParent())
	{
		D3DXMATRIX inv;
		D3DXMatrixInverse(&inv, nullptr, &parent->World());

		D3DXVECTOR3 position;
		D3DXVec3TransformCoord(&position, &vec, &inv);

		SetLocalTranslation(position);
	}
	else
	position = vec;

	UpdateWorld();
}

void Transform::Position(Vector3 * vec)
{
	*vec = position;
}

void Transform::Scale(float x, float y, float z)
{
	Scale(Vector3(x, y, z));
}

void Transform::Scale(Vector3 & vec)
{
	

	if (HasParent())
	{
		D3DXVECTOR3 parentScale = parent->GetScale();
		D3DXVECTOR3 scale;
		scale.x = vec.x / parentScale.x;
		scale.y = vec.y / parentScale.y;
		scale.z = vec.z / parentScale.z;
	
		SetLocalScale(scale);
		
	}
	else
	scale = vec;

	UpdateWorld();
}

void Transform::Scale(Vector3 * vec)
{
	*vec = scale;
}

void Transform::Rotation(float x, float y, float z,float w)
{
	Rotation(Quaternion(x, y, z,w));
}

void Transform::Rotation(Quaternion & vec)
{
	
	if (HasParent())
	{
		D3DXQUATERNION q;
		D3DXMatrixDecompose(&scale, &q, &position, &parent->World());

		

		SetLocalRotation(q);
	}
	else
	rotation = vec;

	UpdateWorld();
}

void Transform::Rotation(Quaternion * vec)
{
	*vec = rotation;
}

Matrix Transform::GetRotationMatrix()
{
	
	return Matrix();
}

void Transform::RotationDegree(float x, float y, float z,float w)
{
	RotationDegree(Quaternion(x, y, z,w));
}

void Transform::RotationDegree(Quaternion & vec)
{
	Quaternion temp;

	temp.x = Math::ToRadian(vec.x);
	temp.y = Math::ToRadian(vec.y);
	temp.z = Math::ToRadian(vec.z);
	temp.w = Math::ToRadian(vec.w);
	Rotation(temp);
}

void Transform::RotationDegree(Quaternion * vec)
{
	Quaternion temp;

	temp.x = Math::ToDegree(rotation.x);
	temp.y = Math::ToDegree(rotation.y);
	temp.z = Math::ToDegree(rotation.z);
	temp.w = Math::ToDegree(rotation.w);
	*vec = temp;
}

Vector3 Transform::Direction()
{
	return Vector3(bufferDesc.World._31, bufferDesc.World._32, bufferDesc.World._33);
}

Vector3 Transform::Up()
{
	return Vector3(bufferDesc.World._21, bufferDesc.World._22, bufferDesc.World._23);
}

Vector3 Transform::Right()
{
	return Vector3(bufferDesc.World._11, bufferDesc.World._12, bufferDesc.World._13);
}

void Transform::WorldDecompose(const Matrix & matrix)
{

	D3DXMatrixDecompose(&scale, &rotation, &position, &matrix);
	
	
	bufferDesc.World = matrix;
	
}

void Transform::SetLocalScale(const D3DXVECTOR3 & vec)
{
	if (scale == vec)
		return;

	scale = vec;
	UpdateWorld();
}

void Transform::SetLocalRotation(const Quaternion & vec)
{
	if (rotation == vec)
		return;

	rotation = vec;
	UpdateWorld();
}

void Transform::SetLocalTranslation(const D3DXVECTOR3 & vec)
{
	if (position == vec)
		return;

	position = vec;
	UpdateWorld();
}

void Transform::UpdateWorld()
{
	D3DXMATRIX S, R, T;
	D3DXMatrixScaling(&S, scale.x, scale.y, scale.z);
	D3DXMatrixRotationQuaternion(&R, &rotation);
	//D3DXMatrixRotationYawPitchRoll(&R, rotation.y, rotation.x, rotation.z);
	D3DXMatrixTranslation(&T, position.x, position.y, position.z);

	local = S * R * T;
	
	if (HasParent())
	{
		bufferDesc.World = local * parent->World();
	}
	else
		bufferDesc.World = local;

	for (const auto& child : childs)
		child->UpdateWorld();
}

void Transform::Update()
{

}

void Transform::Render()
{
	if (shader == NULL) return;

	buffer->Apply();
	sBuffer->SetConstantBuffer(buffer->Buffer());
}