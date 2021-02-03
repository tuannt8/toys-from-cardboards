#version 120

attribute vec3 position;
attribute vec3 normal;
uniform mat3 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
varying vec3 fColor;

const float ambient_coeff   = 0.25;
const float specular_coeff  = 1.0;
const float specular_exp    = 32.0;
const vec3  light_direction = vec3(0,-1.0,0.0); // stationary light
const vec3  light_color     = vec3(0.7,0.3,0.1);
const vec3  object_color    = vec3(1.0,1.0,1.0); // yellow-ish orange

void main()
{
  vec4 pos = modelViewMatrix * vec4(position.xyz, 1.0);
  gl_Position = projectionMatrix * pos;

  vec3 l = normalize(light_direction);
  vec3 n = normalize(mat3(normalMatrix) * normal);
  vec3 e = normalize(-pos.xyz);
  vec3 h = normalize (e+l);

  vec3 ambient_color  = ambient_coeff  * object_color;
  vec3 specular_color = specular_coeff * pow(max(0.0,dot(n,h)),specular_exp) * light_color;

  fColor=ambient_color+specular_color;
}
