#ifdef GL_ES
// define default precision for float, vec, mat.
precision lowp float;
#endif
varying vec4 colorVarying;
#ifdef GL_ES
varying lowp vec2 textureCoordinate0;
#else
varying vec2 textureCoordinate0;
#endif
uniform sampler2D stexture0;
void main()
{
vec4 color;
vec4 arg1;
vec4 arg2;
vec4 colorTex;

colorTex = texture2D(stexture0, textureCoordinate0);
arg1 = colorVarying;
arg2 = colorTex;
color = arg1 * arg2;
gl_FragColor = color;
#ifndef GL_ES
if (gl_FragColor.w==0.0)
discard;
#endif
}
