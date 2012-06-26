#include <iostream>
#include <fstream>
#include <cmath>
#include <cstring>

#if defined(WIN32)
#  include "glut.h"
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/glut.h>
#endif

#include "Obj.h"

/*
** デフォルトコンストラクタ
*/
Obj::Obj(void)
{
  init();
}

/*
** コンストラクタ
*/
Obj::Obj(const char *name)
{
  init();
  load(name);
}

/*
** コピーコンストラクタ
*/
Obj::Obj(const Obj &o)
{
  copy(o);
}

/*
** デストラクタ
*/
Obj::~Obj()
{
  if (vert) delete[] vert;
  if (norm) delete[] norm;
  if (fnorm) delete[] fnorm;
  if (face) delete[] face;
}

/*
** 代入演算子
*/
Obj &Obj::operator=(const Obj &o)
{
  if (this != &o) {
    this->~Obj();
    copy(o);
  }
  return *this;
}

/*
** オブジェクトの初期化
*/
void Obj::init(void)
{
  nv = nf = 0;
  vert = norm = fnorm = 0;
  face = 0;
}

/*
** オブジェクトのコピー
*/
void Obj::copy(const Obj &o)
{
  nv = o.nv;
  nf = o.nf;

  try {
    if (nv > 0) {
      vert = new vec[nv];
      norm = new vec[nv];

      memcpy(vert, o.vert, sizeof(vec) * nv);
      memcpy(norm, o.norm, sizeof(vec) * nv);
    }
    else {
      vert = norm = 0;
    }
    if (nf > 0) {
      fnorm = new vec[nf];
      face = new idx[nf];

      memcpy(fnorm, o.fnorm, sizeof(vec) * nf);
      memcpy(face, o.face, sizeof(idx) * nf);
    }
    else {
      fnorm = 0;
      face = 0;
    }
  }
  catch (std::bad_alloc e) {
    std::cerr << "メモリが足りません"<< std::endl;
    this->~Obj();
    init();
  }
}

/*
** ファイルの読み込み
*/
bool Obj::load(const char *name)
{
  /* メモリの開放 */
  this->~Obj();

  /* ファイルの読み込み */
  std::ifstream file(name, std::ios::binary);
  if (!file) {
    std::cerr << name << " が開けません" << std::endl;
    init();
    return false;
  }

  /* データの数を調べる */
  char buf[1024];
  int v, f;
  v = f = 0;
  while (file.getline(buf, sizeof buf)) {
    if (buf[0] == 'v' && buf[1] == ' ') {
      ++v;
    }
    else if (buf[0] == 'f' && buf[1] == ' ') {
      ++f;
    }
  }

  nv = v;
  nf = f;

  try {
    vert = new vec[v];
    norm = new vec[v];
    fnorm = new vec[f];
    face = new idx[f];
  }
  catch (std::bad_alloc e) {
    std::cerr << "メモリが足りません" << std::endl;
    this->~Obj();
    init();
    return false;
  }

  /* ファイルの巻き戻し */
  file.clear();
  file.seekg(0L, std::ios::beg);

  /* データの読み込み */
  v = f = 0;
  while (file.getline(buf, sizeof buf)) {
    if (buf[0] == 'v' && buf[1] == ' ') {
      sscanf(buf, "%*s %f %f %f", vert[v], vert[v] + 1, vert[v] + 2);
      ++v;
    }
    else if (buf[0] == 'f' && buf[1] == ' ') {
      if (sscanf(buf + 2, "%d/%*d/%*d %d/%*d/%*d %d/%*d/%*d", face[f], face[f] + 1, face[f] + 2) != 3) {
        if (sscanf(buf + 2, "%d//%*d %d//%*d %d//%*d", face[f], face[f] + 1, face[f] + 2) != 3) {
          sscanf(buf + 2, "%d %d %d", face[f], face[f] + 1, face[f] + 2);
        }
      }
      --face[f][0];
      --face[f][1];
      --face[f][2];
      ++f;
    }
  }

  /* 面法線ベクトルの算出 */
  int i;
  for (i = 0; i < f; ++i) {
    float dx1 = vert[face[i][1]][0] - vert[face[i][0]][0];
    float dy1 = vert[face[i][1]][1] - vert[face[i][0]][1];
    float dz1 = vert[face[i][1]][2] - vert[face[i][0]][2];
    float dx2 = vert[face[i][2]][0] - vert[face[i][0]][0];
    float dy2 = vert[face[i][2]][1] - vert[face[i][0]][1];
    float dz2 = vert[face[i][2]][2] - vert[face[i][0]][2];

    fnorm[i][0] = dy1 * dz2 - dz1 * dy2;
    fnorm[i][1] = dz1 * dx2 - dx1 * dz2;
    fnorm[i][2] = dx1 * dy2 - dy1 * dx2;
  }

  /* 頂点の仮想法線ベクトルの算出 */
  for (i = 0; i < v; ++i) {
    norm[i][0] = norm[i][1] = norm[i][2] = 0.0;
  }
  
  for (i = 0; i < f; ++i) {
    norm[face[i][0]][0] += fnorm[i][0];
    norm[face[i][0]][1] += fnorm[i][1];
    norm[face[i][0]][2] += fnorm[i][2];

    norm[face[i][1]][0] += fnorm[i][0];
    norm[face[i][1]][1] += fnorm[i][1];
    norm[face[i][1]][2] += fnorm[i][2];

    norm[face[i][2]][0] += fnorm[i][0];
    norm[face[i][2]][1] += fnorm[i][1];
    norm[face[i][2]][2] += fnorm[i][2];
  }

  /* 頂点の仮想法線ベクトルの正規化 */
  for (i = 0; i < v; ++i) {
    float a = sqrt(norm[i][0] * norm[i][0]
                 + norm[i][1] * norm[i][1]
                 + norm[i][2] * norm[i][2]);

    if (a != 0.0) {
      norm[i][0] /= a;
      norm[i][1] /= a;
      norm[i][2] /= a;
    }
  }

  return true;
}

/*
** 図形の表示
*/
void Obj::draw(void)
{
  /* 頂点データ，法線データ，テクスチャ座標の配列を有効にする */
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  
  /* 頂点データ，法線データ，テクスチャ座標の場所を指定する */
  glNormalPointer(GL_FLOAT, 0, norm);
  glVertexPointer(3, GL_FLOAT, 0, vert);
  
  /* 頂点のインデックスの場所を指定して図形を描画する */
  glDrawElements(GL_TRIANGLES, nf * 3, GL_UNSIGNED_INT, face);

  /* 頂点データ，法線データ，テクスチャ座標の配列を無効にする */
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
}
