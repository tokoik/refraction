// aberration.frag

uniform samplerCube cubemap;

varying vec3 r;   // 視線の反射ベクトル
varying vec3 s_r; // 視線の赤方向の屈折ベクトル
varying vec3 s_g; // 視線の緑方向の屈折ベクトル
varying vec3 s_b; // 視線の青方向の屈折ベクトル
varying float t;  // 境界面での反射率

void main(void)
{
  vec4 c;
  
  c.r = textureCube(cubemap, s_r).r;
  c.g = textureCube(cubemap, s_g).g;
  c.b = textureCube(cubemap, s_b).b;
  c.a = 1.0;
  
  gl_FragColor = mix(c, textureCube(cubemap, r), t);
}
