#include "Framework.h"
#include "Freedom.h"

Freedom::Freedom()
	: move(20), rotation(2)
{

}

Freedom::~Freedom()
{
}

void Freedom::Update()
{
	if (Mouse::Get()->Press(1) == false)
		return;


	const Vector3& f = Forward();
	const Vector3& r = Right();
	const Vector3& u = Up();

	Vector3 position;
	Position(&position);

	if (Keyboard::Get()->Press('W'))
		position += f * move * Time::Delta();
	else if (Keyboard::Get()->Press('S'))
		position -= f * move * Time::Delta();

	if (Keyboard::Get()->Press('D'))
		position += r * move * Time::Delta();
	else if (Keyboard::Get()->Press('A'))
		position -= r * move * Time::Delta();

	if (Keyboard::Get()->Press('E'))
		position += u * move * Time::Delta();
	else if (Keyboard::Get()->Press('Q'))
		position -= u * move * Time::Delta();
	//Context::Get()->SetTargetPos(position + forward);
	Position(position);

	

	/*if (Mouse::Get()->Press(1))
	{*/
		Vector3 R;
		Rotation(&R);
		Vector3 val = Mouse::Get()->GetMoveValue();
		R.x += val.y*rotation*Time::Delta();
		R.y += val.x*rotation*Time::Delta();
		R.z = 0.0f;
		Rotation(R);
	//}

	
	
}

void Freedom::Speed(float move, float rotation)
{
	this->move = move;
	this->rotation = rotation;
}
