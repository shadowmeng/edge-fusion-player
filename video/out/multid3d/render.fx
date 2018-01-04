Texture2D tx0 : register(t0);
Texture2D tx1 : register(t1);
Texture2D tx2 : register(t2);
//SamplerState samLinear : register(s0);


SamplerState tex0 : register(s0);
SamplerState tex1 : register(s1);
SamplerState tex2 : register(s2);

//matrix colormatrix : register(c0);

cbuffer ConstantBuffer : register(b0)
{
	matrix World;
	matrix View;
	matrix Projection;
	float4 paintColor;
};

cbuffer ConstantBuffer1: register(b1)
{
	matrix colormatrix;
};

cbuffer ConstantBuffer2: register(b2)
{
	float4 overlapEdge;
	float4 texEdgeCoord;
};

struct VS_INPUT
{
	float4 Pos : POSITION;
	float2 Tex : TEXCOORD0;
};

struct PS_INPUT
{
	float4 Pos : SV_POSITION;
	float2 Tex : TEXCOORD0;
};

PS_INPUT VS(VS_INPUT input)
{
	PS_INPUT output = (PS_INPUT)0;
	output.Pos = mul(input.Pos, World);
	output.Pos = mul(output.Pos, View);
	output.Pos = mul(output.Pos, Projection);
	output.Tex = input.Tex;
	return output;
}

PS_INPUT SCRVS(VS_INPUT Pos)
{
    return Pos;
}

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS(PS_INPUT input) : SV_Target
{
	return paintColor;
}

float4 SCRPS(PS_INPUT input) : SV_Target
{
	return tx0.Sample(tex0, input.Tex);    // Yellow, with Alpha = 1
}


float4 PS1(PS_INPUT input) : SV_Target
{
	float4 y = tx0.Sample(tex0, input.Tex);
	float4 u = tx1.Sample(tex1, input.Tex);
	float4 v = tx2.Sample(tex2, input.Tex);
	float xx = y.x;
	float yy = u.x;
	float zz = v.x;
	
	if (input.Tex.x < overlapEdge.x){
		xx = xx * ((input.Tex.x - texEdgeCoord.x) / (overlapEdge.x - texEdgeCoord.x));
		//yy = yy * ((input.Tex.x - texEdgeCoord.x) / (overlapEdge.x - texEdgeCoord.x));
		//zz = zz * ((input.Tex.x - texEdgeCoord.x) / (overlapEdge.x - texEdgeCoord.x));
	}
	
	if (input.Tex.x > overlapEdge.y){
		xx = xx * (texEdgeCoord.y - input.Tex.x) / (texEdgeCoord.y - overlapEdge.y);
		//yy = yy * (texEdgeCoord.y - input.Tex.x) / (texEdgeCoord.y - overlapEdge.y);
		//zz = zz * (texEdgeCoord.y - input.Tex.x) / (texEdgeCoord.y - overlapEdge.y);
	}
	
	if (input.Tex.y < overlapEdge.w){
		xx = xx * (input.Tex.y - texEdgeCoord.w) / (overlapEdge.w - texEdgeCoord.w);
		//yy = yy * (input.Tex.y - texEdgeCoord.w) / (overlapEdge.w - texEdgeCoord.w);
		//zz = zz * (input.Tex.y - texEdgeCoord.w) / (overlapEdge.w - texEdgeCoord.w);
	}
	
	if (input.Tex.y > overlapEdge.z){
		xx = xx * (texEdgeCoord.z - input.Tex.y) / (texEdgeCoord.z - overlapEdge.z);
		//yy = yy * (texEdgeCoord.z - input.Tex.y) / (texEdgeCoord.z - overlapEdge.z);
		//zz = zz * (texEdgeCoord.z - input.Tex.y) / (texEdgeCoord.z - overlapEdge.z);
	}
	
	float4 c = float4(xx, yy, zz, 1.0); 
	return mul(c, colormatrix);
}
