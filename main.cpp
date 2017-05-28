#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>

#include "gg.h"
using namespace gg;

/*
** �V�F�[�_�I�u�W�F�N�g
*/
static GLuint shader1, shader2;
static GLint textureLoc, cubemapLoc;

/*
**�g���b�N�{�[������
*/
static GgTrackball *tb1, *tb2;
static int btn = -1;

/*
** ��
*/
#include "Box.h"
static Box *box;

/*
** OBJ �t�@�C��
*/
static GLuint nv, nf;
static GLfloat (*vert)[3], (*norm)[3];
static GLuint (*face)[3];

/*
** �e�N�X�`��
*/
#define TEXWIDTH  256                           /* �e�N�X�`���̕��@�@�@ */
#define TEXHEIGHT 256                           /* �e�N�X�`���̍����@�@ */
static GLuint texname[2];                       /* �e�N�X�`�����i�ԍ��j */
static const char *texfile[] = {                /* �e�N�X�`���t�@�C���� */
  "room3ny.tga", /* �� */
  "room3nz.tga", /* �� */
  "room3px.tga", /* �E */
  "room3pz.tga", /* �O */
  "room3nx.tga", /* �� */
  "room3py.tga", /* �� */
};
static const int target[] = {                /* �e�N�X�`���̃^�[�Q�b�g�� */
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
  GL_TEXTURE_CUBE_MAP_POSITIVE_X,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
  GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
  GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
};

/*
** ������
*/
static void init(void)
{
  /* OpenGL �g���@�\�̏������ق� */
  ggInit();

  /* OBJ �t�@�C���̓ǂݍ��� */
  ggLoadObj("bunny.obj", nv, vert, norm, nf, face, false);

  /* ���̃I�u�W�F�N�g�𐶐� */
  box = new Box(500.0f, 500.0f, 500.0f);

  /* �g���b�N�{�[�������p�I�u�W�F�N�g�̐��� */
  tb1 = new GgTrackball;
  tb2 = new GgTrackball;

  /* �e�N�X�`�������Q���� */
  glGenTextures(2, texname);
  
  /* �e�N�X�`���摜�̓��[�h�P�ʂɋl�ߍ��܂�Ă��� */
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  /* �O���̗����̂̃e�N�X�`���̊��蓖�āi�W�����j */
  glBindTexture(GL_TEXTURE_2D, texname[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, TEXWIDTH * 8, TEXHEIGHT, 0,
    GL_RGBA, GL_UNSIGNED_BYTE, 0);
  
  /* �e�N�X�`���摜�̓ǂݍ��� */
  for (int i = 0; i < 6; ++i) {
    GLsizei width, height;
    GLenum format;
    GLubyte *image = ggLoadTga(texfile[i], width, height, format);

    if (image) {

      /* �O���̗����̂̃e�N�X�`���̒u������ */
      glTexSubImage2D(GL_TEXTURE_2D, 0, TEXWIDTH * i, 0, TEXWIDTH, TEXWIDTH,
        format, GL_UNSIGNED_BYTE, image);

      /* �L���[�u�}�b�s���O�̃e�N�X�`���̊��蓖�� */
      glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);
      glTexImage2D(target[i], 0, GL_RGBA, TEXWIDTH, TEXWIDTH, 0, 
        format, GL_UNSIGNED_BYTE, image);

      /* �ݒ�Ώۂ��O���̗����̂̃e�N�X�`���ɖ߂� */
      glBindTexture(GL_TEXTURE_2D, texname[0]);
    }
  }
  
  /* �e�N�X�`�����g��E�k��������@�̎w�� */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  /* �e�N�X�`���̌J��Ԃ����@�̎w�� */
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  /* �ݒ�Ώۂ��L���[�u�}�b�s���O�̃e�N�X�`���ɐ؂�ւ��� */
  glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);

  /* �e�N�X�`�����g��E�k��������@�̎w�� */
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  
  /* �e�N�X�`���̌J��Ԃ����@�̎w�� */
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  
  /* �ݒ�Ώۂ𖳖��e�N�X�`���ɖ߂� */
  glBindTexture(GL_TEXTURE_2D, 0);
  
  /* �V�F�[�_�v���O�����̍쐬 */
  shader1 = ggLoadShader("replace.vert", "replace.frag");
  shader2 = ggLoadShader("refract.vert", "refract.frag");
  
  /* �e�N�X�`�����j�b�g */
  textureLoc = glGetUniformLocation(shader1, "texture");
  cubemapLoc = glGetUniformLocation(shader2, "cubemap");

  /* �����ݒ� */
  glClearColor(0.3f, 0.3f, 1.0f, 0.0f);
  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
}

/*
** �V�[���̕`��
*/
static void scene(void)
{
  /* �ݒ�Ώۂ��O���̗����̂̃e�N�X�`���ɐ؂�ւ��� */
  glBindTexture(GL_TEXTURE_2D, texname[0]);
  
  /* ���̃e�N�X�`���̃V�F�[�_�v���O������K�p���� */
  glUseProgram(shader1);
  glUniform1i(textureLoc, 0);

  /* ����`�� */
  glPushMatrix();
  glMultMatrixf(tb2->get());
  box->draw();
  glPopMatrix();

  /* �ݒ�Ώۂ��L���[�u�}�b�s���O�̃e�N�X�`���ɐ؂�ւ���*/
  glBindTexture(GL_TEXTURE_CUBE_MAP, texname[1]);

  /* �L���[�u�}�b�s���O�̃V�F�[�_�v���O������K�p���� */
  glUseProgram(shader2);
  glUniform1i(cubemapLoc, 0);

  /* �e�N�X�`���ϊ��s��Ƀg���b�N�{�[�����̉�]�������� */
  glMatrixMode(GL_TEXTURE);
  glLoadTransposeMatrixf(tb2->get());
  glMatrixMode(GL_MODELVIEW);

  /* ���_��菭�����ɃI�u�W�F�N�g��`���ăg���b�N�{�[�����̉�]�������� */
  glPushMatrix();
  glTranslated(0.0, 0.0, -200.0);
  glMultMatrixf(tb1->get());

  /* ���_�f�[�^�C�@���f�[�^�C�e�N�X�`�����W�̔z���L���ɂ��� */
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  
  /* ���_�f�[�^�C�@���f�[�^�C�e�N�X�`�����W�̏ꏊ���w�肷�� */
  glNormalPointer(GL_FLOAT, 0, norm);
  glVertexPointer(3, GL_FLOAT, 0, vert);
  
  /* ���_�̃C���f�b�N�X�̏ꏊ���w�肵�Đ}�`��`�悷�� */
  glDrawElements(GL_TRIANGLES, nf * 3, GL_UNSIGNED_INT, face);

  /* ���_�f�[�^�C�@���f�[�^�C�e�N�X�`�����W�̔z��𖳌��ɂ��� */
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);

  glPopMatrix();
  
  /* �e�N�X�`���ϊ��s������ɖ߂� */
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();
  glMatrixMode(GL_MODELVIEW);

  /* �ݒ�Ώۂ𖳖��e�N�X�`���ɖ߂� */
  glBindTexture(GL_TEXTURE_2D, 0);
}


/****************************
** GLUT �̃R�[���o�b�N�֐� **
****************************/

static void display(void)
{
  /* ��ʃN���A */
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  /* ���f���r���[�ϊ��s��̐ݒ� */
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  /* �V�[���̕`�� */
  scene();
  
  /* �_�u���o�b�t�@�����O */
  glutSwapBuffers();
}

static void resize(int w, int h)
{
  /* �g���b�N�{�[������͈� */
  tb1->region(w, h);
  tb2->region(w, h);
  
  /* �E�B���h�E�S�̂��r���[�|�[�g�ɂ��� */
  glViewport(0, 0, w, h);
  
  /* �����ϊ��s��̎w�� */
  glMatrixMode(GL_PROJECTION);
  
  /* �����ϊ��s��̏����� */
  glLoadIdentity();
  gluPerspective(60.0, (double)w / (double)h, 100.0, 500.0);
}

static void idle(void)
{
  /* ��ʂ̕`���ւ� */
  glutPostRedisplay();
}

static void mouse(int button, int state, int x, int y)
{
  btn = button;

  switch (btn) {
  case GLUT_LEFT_BUTTON:
    if (state == GLUT_DOWN) {
      /* �g���b�N�{�[���J�n */
      tb1->start(x, y);
      glutIdleFunc(idle);
    }
    else {
      /* �g���b�N�{�[����~ */
      tb1->stop(x, y);
      glutIdleFunc(0);
    }
    break;
  case GLUT_RIGHT_BUTTON:
    if (state == GLUT_DOWN) {
      /* �g���b�N�{�[���J�n */
      tb2->start(x, y);
      glutIdleFunc(idle);
    }
    else {
      /* �g���b�N�{�[����~ */
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
    /* �g���b�N�{�[���ړ� */
    tb1->motion(x, y);
    break;
  case GLUT_RIGHT_BUTTON:
    /* �g���b�N�{�[���ړ� */
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
    /* ESC �� q �� Q ���^�C�v������I�� */
    exit(0);
  default:
    break;
  }
}

/*
** ���C���v���O����
*/
int main(int argc, char *argv[])
{
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH | GLUT_DOUBLE);
  glutInitWindowSize(1280, 800);
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
