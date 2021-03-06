//--------------------------------------------------------------------------------------
// File: SSAO.fx
//--------------------------------------------------------------------------------------

Texture2D txNormalDepthMap : register(t0);
Texture2D txRandomVectorMap : register(t1);

SamplerState samNormalDepth : register(s0);

SamplerState samRandomVec : register(s1);

SamplerState samLinear : register(s2);

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------

cbuffer ConstantBuffer : register(b0)
{
    float4x4 ViewToTexSpace; // Proj*Texture
    float4 OffsetVectors[14];
    float4 FrustumCorners[4];

    // Coordinates given in view space
    float OcclusionRadius = 0.05f;
    float OcclusionFadeStart = 0.2f;
    float OcclusionFadeEnd = 2.0f;
    float SurfaceEpsilon = 0.05f;

    int gSampleCount = 14;

}

struct VS_INPUT
{
    float3 PosH : POSITION;
    float3 ToFarPlaneIndex : NORMAL;
    //float2 Tex : TEXCOORD;
};

struct VS_OUTPUT
{
    float4 PosH : SV_POSITION;
    float3 ToFarPlane : TEXCOORD0;
    float2 Tex : TEXCOORD1;
};

VS_OUTPUT SSAOVS(VS_INPUT input)
{
    VS_OUTPUT output = (VS_OUTPUT) 0;

    output.PosH = float4(input.PosH, 1.0f);

    output.ToFarPlane = FrustumCorners[input.ToFarPlaneIndex.x].xyz;

    //output.Tex = input.Tex;

    output.Tex = input.ToFarPlaneIndex.yz;
    return output;
}
// Determines how much the sample point q occludes the point p as a function
// of distZ.
float OcclusionFunction(float distZ)
{
	//
	// If depth(q) is "behind" depth(p), then q cannot occlude p.  Moreover, if 
	// depth(q) and depth(p) are sufficiently close, then we also assume q cannot
	// occlude p because q needs to be in front of p by Epsilon to occlude p.
	//
	// We use the following function to determine the occlusion.  
	// 
	//
	//       1.0     -------------\
	//               |           |  \
	//               |           |    \
	//               |           |      \ 
	//               |           |        \
	//               |           |          \
	//               |           |            \
	//  ------|------|-----------|-------------|---------|--> zv
	//        0     Eps          z0            z1        
	//
	
    float occlusion = 0.0f;
    if (distZ > SurfaceEpsilon)
    {
        float fadeLength = OcclusionFadeEnd - OcclusionFadeStart;
		
		// Linearly decrease occlusion from 1 to 0 as distZ goes 
		// from gOcclusionFadeStart to gOcclusionFadeEnd.	
        occlusion = saturate((OcclusionFadeEnd - distZ) / fadeLength);
    }
    return occlusion;
}

float4 SSAOPS(VS_OUTPUT input) : SV_Target
{
	// p -- the point we are computing the ambient occlusion for.
	// n -- normal vector at p.
	// q -- a random offset from p.
	// r -- a potential occluder that might occlude p.

	// Get viewspace normal and z-coord of this pixel.  The tex-coords for
	// the fullscreen quad we drew are already in uv-space.
    float4 normalDepth = txNormalDepthMap.SampleLevel(samNormalDepth, input.Tex, 0.0f);
 
    float3 n = normalDepth.xyz;
    float pz = normalDepth.w;

    
	//
	// Reconstruct full view space position (x,y,z).
	// Find t such that p = t*input.ToFarPlane.
	// p.z = t*input.ToFarPlane.z
	// t = p.z / input.ToFarPlane.z
	//
    float3 p = (pz / input.ToFarPlane.z) * input.ToFarPlane;

	// Extract random vector and map from [0,1] --> [-1, +1].
    float3 randVec = 2.0f * txRandomVectorMap.SampleLevel(samRandomVec, 4.0f * input.Tex, 0.0f).rgb - 1.0f;
    
    float occlusionSum = 0.0f;
	
	// Sample neighboring points about p in the hemisphere oriented by n.
	[unroll]
    for (int i = 0; i < gSampleCount; ++i)
    {
		// Are offset vectors are fixed and uniformly distributed (so that our offset vectors
		// do not clump in the same direction).  If we reflect them about a random vector
		// then we get a random uniform distribution of offset vectors.
        float3 offset = reflect(OffsetVectors[i].xyz, randVec);
	
		// Flip offset vector if it is behind the plane defined by (p, n).
        float flip = sign(dot(offset, n));
		
		// Sample a point near p within the occlusion radius.
        float3 q = p + flip * OcclusionRadius * offset;
		
		// Project q and generate projective tex-coords.  
        float4 projQ = mul(float4(q, 1.0f), ViewToTexSpace);
        projQ /= projQ.w;

		// Find the nearest depth value along the ray from the eye to q (this is not
		// the depth of q, as q is just an arbitrary point near p and might
		// occupy empty space).  To find the nearest depth we look it up in the depthmap.

        float rz = txNormalDepthMap.SampleLevel(samNormalDepth, projQ.xy, 0.0f).a;

		// Reconstruct full view space position r = (rx,ry,rz).  We know r
		// lies on the ray of q, so there exists a t such that r = t*q.
		// r.z = t*q.z ==> t = r.z / q.z

        float3 r = (rz / q.z) * q;
		
		//
		// Test whether r occludes p.
		//   * The product dot(n, normalize(r - p)) measures how much in front
		//     of the plane(p,n) the occluder point r is.  The more in front it is, the
		//     more occlusion weight we give it.  This also prevents self shadowing where 
		//     a point r on an angled plane (p,n) could give a false occlusion since they
		//     have different depth values with respect to the eye.
		//   * The weight of the occlusion is scaled based on how far the occluder is from
		//     the point we are computing the occlusion of.  If the occluder r is far away
		//     from p, then it does not occlude it.
		// 
		
        float distZ = p.z - r.z;
        float dp = max(dot(n, normalize(r - p)), 0.0f);
        float occlusion = dp * OcclusionFunction(distZ);
		
        occlusionSum += occlusion;
    }
	
    occlusionSum /= gSampleCount;
	
    float access = 1.0f - occlusionSum;

    

	// Sharpen the contrast of the SSAO map to make the SSAO affect more dramatic.
    return saturate(pow(access, 4.0f));
}
