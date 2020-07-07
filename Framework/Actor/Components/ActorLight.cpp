#include "Framework.h"
#include "ActorLight.h"


ActorLight::ActorLight(Shader* shader, Model* model, SharedData* sharedData)
	: Component(shader, model, sharedData)
    , light_type(LightType::Point)
    , range(10.0f)
    , intensity(2.0f)
    , angle_radian(0.5f)
    , bias(0.0f)
    , normal_bias(0.0f)
    , color(1.0f, 0.76f, 0.57f, 1.0f)
    , last_camera_position(0, 0, 0)
    , last_light_position(0, 0, 0)
    , last_light_rotation(0, 0, 0, 1)
    , is_cast_shadow(true)
    , is_update(true)
{
    
}

ActorLight::~ActorLight()
{
}

void ActorLight::OnDestroy()
{
}

void ActorLight::OnStart()
{
    //TODO : Create Shadow map
}

void ActorLight::OnUpdate()
{
}

//void ActorLight::OnUpdate(uint drawCount)
//{
	//if (last_light_position != transform->GetTranslation() || last_light_rotation != transform->GetRotation())
   //{
   //    last_light_position = transform->GetTranslation();
   //    last_light_rotation = transform->GetRotation();

   //    is_update = true;
   //}

   //if (light_type == LightType::Directional)
   //{
   //    if (auto camera = renderer->GetCamera())
   //    {
   //        if (last_camera_position != camera->GetTransform()->GetTranslation())
   //        {
   //            last_camera_position = camera->GetTransform()->GetTranslation();
   //            is_update = true;
   //        }
   //    }
   //}

   //if (!is_update)
   //    return;

   ////TODO : Update view matrix & projection matrix
//}



void ActorLight::OnStop()
{
}




auto ActorLight::GetDirection() const -> Vector3
{
    return Vector3(0,0,0);
}

void ActorLight::SetLightType(const LightType & type)
{
    light_type      = type;
    is_update       = true;
    is_cast_shadow  = light_type == LightType::Directional;

    //TODO : Create Shadow Map
}

void ActorLight::SetRange(const float & range)
{
    this->range = Math::Clamp(range, 0.0f, std::numeric_limits<float>::infinity());
}

void ActorLight::SetAngle(const float & angle)
{
    angle_radian    = Math::Clamp(angle, 0.0f, 1.0f);
    is_update       = true;
}

void ActorLight::SetCastShadow(const bool & is_cast_shadow)
{
    if (this->is_cast_shadow == is_cast_shadow)
        return;

    this->is_cast_shadow = is_cast_shadow;
    //TODO : Create Shadow Map
}

void ActorLight::UpdateConstantBuffer()
{
    /*if (!gpu_buffer)
    {
        gpu_buffer = std::make_shared<ConstantBuffer>(context);
        gpu_buffer->Create<LIGHT_DATA>();
    }

    auto gpu_data = gpu_buffer->Map<LIGHT_DATA>();
    if (!gpu_data)
    {
        LOG_ERROR("Failed to map buffer");
        return;
    }

    gpu_data->color         = color;
    gpu_data->intensity     = intensity;
    gpu_data->position      = transform->GetTranslation();
    gpu_data->range         = range;
    gpu_data->direction     = GetDirection();
    gpu_data->angle         = angle_radian;
    gpu_data->bias          = bias;
    gpu_data->normal_bias   = normal_bias;

    gpu_buffer->Unmap();*/
}
