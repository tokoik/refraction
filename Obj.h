/*
** Alias OBJ 形式データ
*/
#ifndef OBJ_H
#define OBJ_H

typedef float vec[3];
typedef unsigned int idx[3];

class Obj {
  int nv, nf;                   // 頂点の数，面の数
  vec *vert, *norm, *fnorm;     // 頂点，頂点の法線，面の法線
  idx *face;                    // 面データ（頂点のインデックス）
  void init(void);              // 初期化
  void copy(const Obj &);       // メモリのコピー
public:
  Obj(void);
  Obj(const char *name);
  Obj(const Obj &o);
  ~Obj(void);
  Obj &operator=(const Obj &o);
  bool load(const char *name);  // OBJ ファイルの読み込み
  void draw(void);              // 図形の描画
};

#endif
