// <Snippetextrusionsample_fxWholePage>
// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

//--------------------------------------------------------------------------------------
// File: dxgisample.fx
//
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
float4 vLightPos[1];
float4 vLightColor[1];

SamplerState samLinear
{
    Filter = MIN_MAG_MIP_LINEAR;
    AddressU = Wrap;
    AddressV = Wrap;
};

cbuffer cbNeverChanges
{
    matrix View;
};

cbuffer cbChangeOnResize
{
    matrix Projection;
};

cbuffer cbChangesEveryFrame
{
    matrix World;
};

struct VS_INPUT
{
    float3 Pos : POSITION;
    float3 Norm : NORMAL;
};

struct PS_INPUT
{
    float4 Pos : SV_POSITION;
    float3 WorldNorm : TEXCOORD0;
    float3 CameraPos : TEXCOORD1;
    float3 WorldPos : TEXCOORD2;
};

struct ColorsOutput
{
    float4 Diffuse;
    float4 Specular;
};

ColorsOutput CalcLighting( float3 worldNormal, float3 worldPos, float3 cameraPos )
{
    ColorsOutput output = (ColorsOutput)0.0;
    
    float specularBrightness = 0.8;
    float shininess = 60;
    float4 specular = float4(specularBrightness*float3(1,1,1),1);

    float ambient = 0.5f;

    float3 lightDir = normalize(vLightPos[0] - worldPos);

    output.Diffuse += min(max(0,dot( lightDir, worldNormal ))+ambient, 1) * vLightColor[0] ;

    float3 halfAngle = normalize( normalize(cameraPos) + lightDir);
    output.Specular += max(0,pow( abs(max(0,dot( halfAngle, worldNormal ))), shininess ) * specular);
    
    return output;
}


//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT VS( VS_INPUT input )
{
    PS_INPUT output = (PS_INPUT)0;
    float4 worldPos = mul( float4(input.Pos,1), World );
    float4 cameraPos = mul( worldPos, View );

    output.WorldPos = worldPos;
    output.WorldNorm = normalize(mul( input.Norm, (float3x3)World ));
    output.CameraPos = cameraPos;
    output.Pos = mul( cameraPos, Projection );

    return output;
}


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 PS( PS_INPUT input) : SV_Target
{
    //float4 finalColor = {1.0f, 0.0f, 0.0f, 1.0f};
    float4 finalColor = 0;
    
    ColorsOutput cOut = CalcLighting( input.WorldNorm, input.WorldPos, input.CameraPos );

    finalColor += cOut.Diffuse + cOut.Specular;

    finalColor.a = 1;
    return finalColor;
}


//--------------------------------------------------------------------------------------
technique10 Render
{
    pass P0
    {
        SetVertexShader( CompileShader( vs_4_0_level_9_1, VS() ) );
        SetGeometryShader( NULL );
        SetPixelShader( CompileShader( ps_4_0_level_9_1, PS() ) );
    }
}
// </Snippetextrusionsample_fxWholePage>
