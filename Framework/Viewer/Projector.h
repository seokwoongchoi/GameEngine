class Projector
{
public:
   Projector(Shader* shader, wstring textureFile, UINT width = 1, UINT height = 1);
   ~Projector();

   void Update();

   Camera* GetCamera() { return (Camera *)camera; }

private:
   struct Desc
   {
      Matrix ProjectorView;
      Matrix Projection2;

      Color Color = D3DXCOLOR(1, 1, 1, 1);
   } desc;

private:
   Shader* shader;

   UINT width, height;

   class Fixity* camera;
   Projection* projection;

   Texture* texture;
   ID3DX11EffectShaderResourceVariable* sMap;

   ConstantBuffer* buffer;
   ID3DX11EffectConstantBuffer* sBuffer;
};
