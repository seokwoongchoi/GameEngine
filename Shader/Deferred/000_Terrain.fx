Texture2D BaseMap;
Texture2D LayerMap;
Texture2D LayerMap2;
Texture2D LayerMap3;
Texture2D LayerMap4;
Texture2D AlphaMap;


Texture2D TerrainBumpMap;
Texture2D TerrainRoughMap;
Texture2D displaymentMap;
Texture2D LodHeightMap;



struct BrushDesc
{
    float4 Color;//�������� �ɹ����ȴ�.
    float3 Location;
    uint Shape;
 
    uint Range;
    
   
   
};

cbuffer CB_TerrainBrush //struct�� �����ϸ� �������� cbuffer�� �ϸ� �������� ������.(���̴�����)
{
    BrushDesc TerrainBrush;
};

float3 GetBrushColor(float3 wPosition)
{
    [branch]
    if (TerrainBrush.Shape == 0)
        return float3(0, 0, 0);
     [branch]
    if (TerrainBrush.Shape == 1 )
    {
        [branch]
        if((wPosition.x  > (TerrainBrush.Location.x- TerrainBrush.Range))&&
            (wPosition.x < (TerrainBrush.Location.x + TerrainBrush.Range)) &&
            (wPosition.z > (TerrainBrush.Location.z - TerrainBrush.Range))&&
            (wPosition.z < (TerrainBrush.Location.z + TerrainBrush.Range)))
        {
          return TerrainBrush.Color;
        }
    }
     [branch]
    if (TerrainBrush.Shape == 2)
    {
        float dx = wPosition.x - TerrainBrush.Location.x;
        float dz = wPosition.z - TerrainBrush.Location.z;

        float dist = sqrt(dx * dx + dz * dz);

           [branch]
        if(dist<=TerrainBrush.Range)
            return TerrainBrush.Color;

    }
    return float3(0, 0, 0);
}


//float3 GetTargetPos(float3 wPosition)
//{
    
   
   
//        float dx = wPosition.x - TerrainBrush.Target.x;
//        float dz = wPosition.z - TerrainBrush.Target.z;

//        float dist = sqrt(dx * dx + dz * dz);

//           [flatten]
//        if (dist <= 0.5f)
//        return float3(1, 0, 0);

    
//    return float3(0, 0, 0);
//}
cbuffer CB_GridLine
{
    float4 GridLineColor;

    uint VisibleGridLine; //0 off, 1 on
    float GridLineThickness;
    float GridLineSize;
    uint LayerNum ;
}
//float3 GetLineColor(float3 wPosition)
//{
//    [flatten]
//   if(VisibleGridLine<1)
//        return float3(0, 0, 0);

//    float2 grid = wPosition.xz/GridLineSize;
//    grid = abs(frac(grid - 0.5f) - 0.5f); //0.5�� �������� �з��� �׷�����

//    float thick = GridLineThickness / GridLineSize;

//    [flatten]
//    if(grid.x<thick||grid.y<thick)
//        return GridLineColor.rgb;

//    return float3(0, 0, 0);
  


//}

float3 GetLineColor(float3 wPosition)
{
    [branch]
    if (VisibleGridLine < 1)
        return float3(0, 0, 0);

    float2 grid = wPosition.xz / GridLineSize;
    //float2 speed = ddx(grid.x);
    float2 range = abs(frac(grid - 0.5f) - 0.5f);
    float2 speed = fwidth(grid); //abs(ddx(y)+bby(x)) ȭ����� ����
    
    float2 pixel = range / speed;//������ ���ð�

    float thick = saturate(min(pixel.x, pixel.y) - GridLineThickness);

   


    //return GridLineColor.rgb*thick;
    return lerp(GridLineColor.rgb, float3(0, 0, 0), thick);


}


float4 GetTerrainColor(float2 uv)
{
    float4 base = BaseMap.Sample(LinearSampler,uv);
    float4 layer = LayerMap.Sample(LinearSampler,uv);
    float alpha = AlphaMap.Sample(LinearSampler,uv).r;

    return lerp(base, layer, alpha);
}

float4 GetBaseColor(float2 uv)
{
    float4 base = BaseMap.Sample(LinearSampler, uv);
     return base;

}

///////////////////////////////////////////////////////LOD/////////////////////
cbuffer CB_Terrain
{
    float MinDistance;
    float MaxDistance;
    float MinTessellation;
    float MaxTessellation;

    float TexelCellSpaceU; //1/uv ->uv��ĭ
    float TexelCellSpaceV;
    float WorldCellSpace; //�簢����ĭ��ũ��
    float HeightRatio;

    float2 TexScale;
    float mainActorYPos;
    float CB_Terrain_Padding2;

    float4 WorldFrustumPlanes[6];
    
};
struct VertexInput_Lod
{
    float4 Position : Position0;
    float2 Uv : Uv0;
    float2 BoundsY : Bound0;
  
};
struct VertexOutput_Lod
{
    float4 Position : Position0;
    float2 Uv : Uv0;
    float2 BoundsY : Bound0;
    
};
///////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
float GetHeight(float x, float y)
{
    uint u = x + Factor;
    uint v = (y - Factor) * -1;
    return LodHeightMap[int2(u, v)] * HeightRatio;
}



//float4 LodViewProjection(float4 position)
//{
//    uint u = TargetPos.x + Factor;
//    uint v = (TargetPos.z - Factor) * -1;
    
//    float3 At = TargetPos;
//    At.y += LodHeightMap[int2(u, v)] * HeightRatio;
// //   float3 Eye = float3(ViewInverse._41, ViewInverse._42 + LodHeightMap[int2(u, v)] * HeightRatio, ViewInverse._43);
//    float3 Eye = ViewPosition();
//     Eye.y += LodHeightMap[int2(u, v)] * HeightRatio;
//    float3 zaxis = normalize(At - Eye);

//    float3 xaxis = normalize(cross(float3(0, 1, 0), zaxis));

//    float3 yaxis = cross(zaxis, xaxis);
    
//    matrix view = matrix(xaxis.x, yaxis.x, zaxis.x, 0,
//                         xaxis.y, yaxis.y, zaxis.y, 0,
//                         xaxis.z, yaxis.z, zaxis.z, 0,
//                      -dot(xaxis, Eye), -dot(yaxis, Eye), -dot(zaxis, Eye), 1);
  
//    //matrix vp = mul(view, Projection);
   
   
//    matrix vp = mul(view, Projection);
//    return mul(position, vp);
//}
VertexOutput_Lod VS(VertexInput_Lod input)
{
    VertexOutput_Lod output;
    output.Position = WorldPosition(input.Position);
    output.Uv = input.Uv;
  
    output.BoundsY = input.BoundsY;
   
    return output;
}

struct ConstantHullOutput_Lod
{
    float Edge[4] : SV_TessFactor;
    float Inside[2] : SV_InsideTessFactor;
};
struct HullOutput_Lod
{
    float4 Position : Position0;
    float2 Uv : Uv0;
   
};

struct DomainOutput_Lod
{
    float4 Position : SV_Position0;
    float3 wPosition : Position1;
    float2 Uv : Uv0;
    float2 TiledUv : Uv1; //spacing ����

};

float TessFactor(float3 position)
{
    position = float3(position.x, 0.0f, position.z);
    float3 view = float3(ViewPosition().x, 0.0f, ViewPosition().z);

    float d = distance(position, view);

    float factor = saturate((d - MinDistance) / (MaxDistance - MinDistance));

    return lerp(MaxTessellation, MinTessellation, factor);
};

//float CalculateTessellationFactor(float distance)
//{
//    return MaxTessellation * (1 / (0.015 * distance));
//}

//// to avoid vertex swimming while tessellation varies, one can use mipmapping for displacement maps
//// it's not always the best choice, but it effificiently suppresses high frequencies at zero cost
//float CalculateMIPLevelForDisplacementTextures(float distance)
//{
//    return log2(128 / CalculateTessellationFactor(distance));
//}

bool AABBBehindPlaneTest(float3 center,float3 extents,float4 plane)
{
    float3 normal = abs(plane.xyz);
    float radius = dot(extents, normal);
    
    float s = dot(float4(center, 1.0f), plane);

    return (s + radius) < 0.0f; //true�� �ȵ��°�
}



bool ContainFrustumCube(float3 center,float3 extents)
{
    [unroll(6)]
    for (int i = 0; i < 6;i++)
    {
        [branch]
        if (AABBBehindPlaneTest(center, extents, WorldFrustumPlanes[i]))
        {
            return true;
    
        }
    }
    return false;
}