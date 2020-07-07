#pragma once
class Effect
{
public:
	Effect() = default;
	virtual ~Effect()= default;

	inline void DrawCount(const uint& drawCount)
	{
		this->drawCount = drawCount;
	}
	inline void AddTransform(Matrix* transform)
	{
		this->transforms=transform;
	}

	void SetShader(class Shader* shader)
	{
		this->shader = shader;
	}

	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void PreviewRender() = 0;


protected:
	class Shader* shader;
	VertexBuffer* vertexBuffer;

	uint drawCount=0;


	Matrix* transforms;
};

