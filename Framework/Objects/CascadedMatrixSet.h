#pragma once
class CascadedMatrixSet
{
public:
	CascadedMatrixSet();
	~CascadedMatrixSet();

	void Init(int shdowMapSize);

	void Update();

	const Matrix GetWorldToShadowSpace()const {return worldToShadowSpace;	}
	const Matrix GetWorldToCascadeProj(int i)const { return arrWorldToCascadeProj[i]; }
	const Vector4 GetToCascadeOffsetX()const {return cascadeOffsetX;}
	const Vector4 GetToCascadeOffsetY()const { return cascadeOffsetY; }
	const Vector4 GetToCascadeScale()const { return cascadeScale; }
private:

	// Extract the frustum corners for the given near and far values
	void ExtractFrustumPoints(float fNear, float fFar, D3DXVECTOR3* arrFrustumCorners);

	// Extract the frustum bounding sphere for the given near and far values
	void ExtractFrustumBoundSphere(float fNear, float fFar, D3DXVECTOR3& vBoundCenter, float& fBoundRadius
	,Vector3 camPos,Vector3 camRight,Vector3 camUp,Vector3 camForward);

	// Test if a cascade needs an update
	bool CascadeNeedsUpdate(const D3DXMATRIX& mShadowView, int iCascadeIdx, const D3DXVECTOR3& newCenter, D3DXVECTOR3& vOffset);
private:
	int shadowMapSize;
	float cascadeTotalRange;
	float arrCascadeRanges[4];

	Vector3 shadowBoundCenter;
	float shadowBoundRadius;
	Vector3 arrCascadeBoundCenter[3];
	float arrCascadeBoundRadius[3];
	Matrix worldToShadowSpace;
	Matrix arrWorldToCascadeProj[3];

	Vector4 cascadeOffsetX;
	Vector4 cascadeOffsetY;
	Vector4 cascadeScale;

	Vector3 vDirectionalDir;
private:
	Camera* camera;

	Vector3 camPos;
	Vector3 camRight;
	Vector3 camUp;
	Vector3 camForward;
};
