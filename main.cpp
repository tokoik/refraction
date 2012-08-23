#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

#if defined(WIN32)
//#  pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"")
#  include "glut.h"
#  include "glext.h"
PFNGLLOADTRANSPOSEMATRIXDPROC glLoadTransposeMatrixd;
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/glut.h>
#endif

/*
** シェーダオブジェクト
*/
#include "glsl.h"
static GLuint shader1, shader2;

/*
**トラックボール処理
*/
#include "Trackball.h"
static Trackball *tb1, *tb2;
static int btn = -1;

/*
** 箱
*/
#include "Box.h"
static Box *box;

/*
** OBJ ファイル
*/
#include "Obj.h"
static Obj *obj;

/*
** テクスチャ
*/
#define TEXWIDTH  256                           /* テクスチャの幅　　　 */
#define TEXHEIGHT 256                           /* テクスチャの高さ　　 */
static GLuint texname[2];                       /* テクスチャ名（番号） */
static const char *texfile[] = {                /* テクスチャファイル名 */
  "room3ny.raw", /* 下 */
  "room3nz.raw", /* 裏 */
  "room3px.raw", /* 右 */
  "room3pz.raw", /* 前 */
  "room3nx.raw", /* 左 */
  "room3py.raw", /* 上 */
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
** シェーダプログラムの作成
*/
static GLuint loadShader(const char *vert, const char *frag)
{
  /* シェーダオブジェクトの作成 */
  GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
  GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
  
  /* シェーダのソースプログラムの読み込み */
  if (readShaderSource(vertShader, vert)) exit(1);
  if (readShaderSource(fragShader, frag)) exit(1);
  
  /* シェーダプログラムのコンパイル／リンク結果を得る変数 */
  GLint compiled, linked;

  /* バーテックスシェーダのソースプログラムのコンパイル */
  glCompileShader(vertShader);
  glGetShaderiv(vertShader, GL_COMPILE_STATUS, &compiled);
  printShaderInfoLog(vertShader);
  if (compiled == GL_FALSE) {
    std::cerr << "Compile error in vertex shader." << std::endl;
    exit(1);
  }
  
  /* フラグメントシェーダのソースプログラムのコンパイル */
  glCompileShader(fragShader);
  glGetShaderiv(fragShader, GL_COMPILE_STATUS, &compiled);
  printShaderInfoLog(fragShader);
  if (compiled == GL_FALSE) {
    std::cerr << "Compile error in fragment shader." << std::endl;
    exit(1);
  }
  
  /* プログラムオブジェクトの作成 */
  GLuint gl2Program = glCreateProgram();
  
  /* シェーダオブジェクトのシェーダプログラムへの登録 */
  glAttachShader(gl2Program, vertShader);
  glAttachShader(gl2Program, fragShader);
  
  /* シェーダオブジェクトの削除 */
  glDeleteShader(vertShader);
  glDeleteShader(fragShader);
  
  /* シェーダプログラムのリンク */
  glLinkProgram(gl2Program);
  glGetProgramiv(gl2Program, GL_LINK_STATUS, &linked);
  printProgramInfoLog(gl2Program);
  if (linked == GL_FALSE) {
    std::cerr << "Link error" << std::endl;
    exit(1);
  }

  return gl2Program;
}

/*
** 初期化
*/
static void init(void)
{
  /* 箱のオブジェクトを生成 */
  box = new Box(500.0f, 500.0f, 500.0f);

  /* OBJ ファイルの読み込み */
  obj = new Obj("bunny.obj");

  /* トラックボール処理用オブジェクトの生成 */
  tb1 = new Trackball;
  tb2 = new Trackball;

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
    std::ifstream file;

    file.open(texfile[i], std::ios::binary);
    if (file) {
      GLubyte image[TEXHEIGHT * TEXWIDTH * 4]; // テクスチャ画像の読み込み用

      file.read(reinterpret_cast<char *>(image), sizeof image);
      file.close();

      /* 外側の立方体のテクスチャの置き換え */
      glTexSubImage2D(GL_TEXTURE_2D, 0, TEXWIDTH * i, 0, TEXWIDTH, TEXHEIGHT,
        GL_RGBA, GL_UNSIGNED_BYTE, image);

      /* キューブマッピングのテクスチャの割り当て */
      glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);
      glTexImage2D(target[i], 0, GL_RGBA, TEXWIDTH, TEXHEIGHT, 0, 
        GL_RGBA, GL_UNSIGNED_BYTE, image);

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
  
  /* GLSL の初期化 */
  if (glslInit()) exit(1);

  /* シェーダプログラムの作成 */
  shader1 = loadShader("replace.vert", "replace.frag");
  shader2 = loadShader("refract.vert", "refract.frag");
  
  /* テクスチャユニット０を指定する */
  glUniform1i(glGetUniformLocation(shader1, "texture"), 0);
  glUniform1i(glGetUniformLocation(shader2, "cubemap"), 0);

  /* 初期設定 */
  glClearColor(0.3, 0.3, 1.0, 0.0);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  
#if defined(WIN32)
  glLoadTransposeMatrixd =
    (PFNGLLOADTRANSPOSEMATRIXDPROC)wglGetProcAddress("glLoadTransposeMatrixd");
#endif
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

  /* 箱を描く */
  glPushMatrix();
  glMultMatrixd(tb2->rotation());
  box->draw();
  glPopMatrix();

  /* 設定対象をキューブマッピングのテクスチャに切り替える*/
  glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);

  /* キューブマッピングのシェーダプログラムを適用する */
  glUseProgram(shader2);

  /* テクスチャ変換行列にトラックボール式の回転を加える */
  glMatrixMode(GL_TEXTURE);
  glLoadTransposeMatrixd(tb2->rotation());
  glMatrixMode(GL_MODELVIEW);

  /* 視点より少し奥にオブジェクトを描いてトラックボール式の回転を加える */
  glPushMatrix();
  glTranslated(0.0, 0.0, -200.0);
  glMultMatrixd(tb1->rotation());
  obj->draw();
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
