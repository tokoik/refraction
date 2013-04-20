#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

#include "gg.h"
using namespace gg;

/*
** シェーダオブジェクト
*/
static GLuint shader1, shader2;
static GLint textureLoc, cubemapLoc;

/*
**トラックボール処理
*/
static GgTrackball *tb1, *tb2;
static int btn = -1;

/*
** 箱
*/
#include "Box.h"
static Box *box;

/*
** OBJ ファイル
*/
static GLuint nv, nf;
static GLfloat (*vert)[3], (*norm)[3];
static GLuint (*face)[3];

/*
** テクスチャ
*/
#define TEXWIDTH  256                           /* テクスチャの幅　　　 */
#define TEXHEIGHT 256                           /* テクスチャの高さ　　 */
static GLuint texname[2];                       /* テクスチャ名（番号） */
static const char *texfile[] = {                /* テクスチャファイル名 */
  "room3ny.tga", /* 下 */
  "room3nz.tga", /* 裏 */
  "room3px.tga", /* 右 */
  "room3pz.tga", /* 前 */
  "room3nx.tga", /* 左 */
  "room3py.tga", /* 上 */
};
static const int target[] = {                /* テクスチャのターゲット名 */
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
  GL_TEXTURE_CUBE_MAP_POSITIVE_X,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
};

/*
** 初期化
*/
static void init(void)
{
  /* OpenGL 拡張機能の初期化ほか */
  ggInit();

  /* OBJ ファイルの読み込み */
  ggLoadObj("bunny.obj", nv, vert, norm, nf, face, false);

  /* 箱のオブジェクトを生成 */
  box = new Box(500.0f, 500.0f, 500.0f);

  /* トラックボール処理用オブジェクトの生成 */
  tb1 = new GgTrackball;
  tb2 = new GgTrackball;

  /* テクスチャ名を２個生成 */
  glGenTextures(2, texname);
  
  /* テクスチャ画像はワード単位に詰め込まれている */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  /* 外側の立方体のテクスチャの割り当て（８枚分） */
  glBindTexture(GL_TEXTURE_2D, texname[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXWIDTH * 8, TEXHEIGHT, 0,
    GL_RGBA, GL_UNSIGNED_BYTE, 0);
  
  /* テクスチャ画像の読み込み */
  for (int i = 0; i < 6; ++i) {
    GLsizei width, height;
    GLenum format;
    GLubyte *image = ggLoadTga(texfile[i], width, height, format);

    if (image) {

      /* 外側の立方体のテクスチャの置き換え */
      glTexSubImage2D(GL_TEXTURE_2D, 0, TEXWIDTH * i, 0, TEXWIDTH, TEXWIDTH,
        format, GL_UNSIGNED_BYTE, image);

      /* キューブマッピングのテクスチャの割り当て */
      glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);
      glTexImage2D(target[i], 0, GL_RGBA, TEXWIDTH, TEXWIDTH, 0, 
        format, GL_UNSIGNED_BYTE, image);

      /* 設定対象を外側の立方体のテクスチャに戻す */
      glBindTexture(GL_TEXTURE_2D, texname[0]);
    }
  }
  
  /* テクスチャを拡大・縮小する方法の指定 */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  /* テクスチャの繰り返し方法の指定 */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  /* 設定対象をキューブマッピングのテクスチャに切り替える */
  glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);

  /* テクスチャを拡大・縮小する方法の指定 */
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  /* テクスチャの繰り返し方法の指定 */
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  /* 設定対象を無名テクスチャに戻す */
  glBindTexture(GL_TEXTURE_2D, 0);
  
  /* シェーダプログラムの作成 */
  shader1 = ggLoadShader("replace.vert", "replace.frag");
  shader2 = ggLoadShader("refract.vert", "refract.frag");
  
  /* テクスチャユニット */
  textureLoc = glGetUniformLocation(shader1, "texture");
  cubemapLoc = glGetUniformLocation(shader2, "cubemap");

  /* 初期設定 */
  glClearColor(0.3f, 0.3f, 1.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
}

/*
** シーンの描画
*/
static void scene(void)
{
  /* 設定対象を外側の立方体のテクスチャに切り替える */
  glBindTexture(GL_TEXTURE_2D, texname[0]);
  
  /* 箱のテクスチャのシェーダプログラムを適用する */
  glUseProgram(shader1);
  glUniform1i(textureLoc, 0);

  /* 箱を描く */
  glPushMatrix();
  glMultMatrixf(tb2->get());
  //box->draw();
  glPopMatrix();

  /* 設定対象をキューブマッピングのテクスチャに切り替える*/
  glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);

  /* キューブマッピングのシェーダプログラムを適用する */
  glUseProgram(shader2);
  glUniform1i(cubemapLoc, 0);

  /* テクスチャ変換行列にトラックボール式の回転を加える */
  glMatrixMode(GL_TEXTURE);
  glLoadTransposeMatrixf(tb2->get());
  glMatrixMode(GL_MODELVIEW);

  /* 視点より少し奥にオブジェクトを描いてトラックボール式の回転を加える */
  glPushMatrix();
  glTranslated(0.0, 0.0, -200.0);
  glMultMatrixf(tb1->get());

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

  glPopMatrix();
  
  /* テクスチャ変換行列を元に戻す */
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);

  /* 設定対象を無名テクスチャに戻す */
  glBindTexture(GL_TEXTURE_2D, 0);
}


/****************************
** GLUT のコールバック関数 **
****************************/

static void display(void)
{
  /* 画面クリア */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  /* モデルビュー変換行列の設定 */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  /* シーンの描画 */
  scene();
  
  /* ダブルバッファリング */
  glutSwapBuffers();
}

static void resize(int w, int h)
{
  /* トラックボールする範囲 */
  tb1->region(w, h);
  tb2->region(w, h);
  
  /* ウィンドウ全体をビューポートにする */
  glViewport(0, 0, w, h);
  
  /* 透視変換行列の指定 */
  glMatrixMode(GL_PROJECTION);
  
  /* 透視変換行列の初期化 */
  glLoadIdentity();
  gluPerspective(60.0, (double)w / (double)h, 100.0, 500.0);
}

static void idle(void)
{
  /* 画面の描き替え */
  glutPostRedisplay();
}

static void mouse(int button, int state, int x, int y)
{
  btn = button;

  switch (btn) {
  case GLUT_LEFT_BUTTON:
    if (state == GLUT_DOWN) {
      /* トラックボール開始 */
      tb1->start(x, y);
      glutIdleFunc(idle);
    }
    else {
      /* トラックボール停止 */
      tb1->stop(x, y);
      glutIdleFunc(0);
    }
    break;
  case GLUT_RIGHT_BUTTON:
    if (state == GLUT_DOWN) {
      /* トラックボール開始 */
      tb2->start(x, y);
      glutIdleFunc(idle);
    }
    else {
      /* トラックボール停止 */
      tb2->stop(x, y);
      glutIdleFunc(0);
    }
    break;
  default:
    break;
  }
}

static void motion(int x, int y)
{
  switch (btn) {
  case GLUT_LEFT_BUTTON:
    /* トラックボール移動 */
    tb1->motion(x, y);
    break;
  case GLUT_RIGHT_BUTTON:
    /* トラックボール移動 */
    tb2->motion(x, y);
    break;
  default:
    break;
  }
}

static void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
  case 'q':
  case 'Q':
  case '\033':
    /* ESC か q か Q をタイプしたら終了 */
    exit(0);
  default:
    break;
  }
}

/*
** メインプログラム
*/
int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutCreateWindow(argv[0]);
  glutDisplayFunc(display);
  glutReshapeFunc(resize);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutKeyboardFunc(keyboard);
  init();
  glutMainLoop();
  return 0;
}
