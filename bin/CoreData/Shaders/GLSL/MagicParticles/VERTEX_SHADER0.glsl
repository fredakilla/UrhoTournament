#include "Uniforms.glsl"
#include "Transform2.glsl"
attribute vec4 iColor;
attribute vec2 iTexCoord0;
varying vec4 colorVarying;
varying vec2 textureCoordinate0;

void main()
{
mat4 modelMatrix = iModelMatrix;
vec3 worldPos = GetWorldPos(modelMatrix);
gl_Position = GetClipPos(worldPos);

colorVarying = iColor;

textureCoordinate0 = iTexCoord0;
}
