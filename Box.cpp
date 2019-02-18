#if defined(WIN32)
#  include <GL/glut.h>
#elif defined(__APPLE__) || defined(MACOSX)
#  include <GLUT/glut.h>
#else
#  define GL_GLEXT_PROTOTYPES
#  include <GL/glut.h>
#endif

#include "Box.h"

/* ���̊�{�T�C�Y */
static const float original[][4][3] = {
  { // ��
    { -0.5f, -0.5f, -0.5f },
    {  0.5f, -0.5f, -0.5f },
    {  0.5f, -0.5f,  0.5f },
    { -0.5f, -0.5f,  0.5f }
  },
  { // ��
    {  0.5f, -0.5f, -0.5f },
    { -0.5f, -0.5f, -0.5f },
    { -0.5f,  0.5f, -0.5f },
    {  0.5f,  0.5f, -0.5f }
  },
  { // �E
    {  0.5f, -0.5f,  0.5f },
    {  0.5f, -0.5f, -0.5f },
    {  0.5f,  0.5f, -0.5f },
    {  0.5f,  0.5f,  0.5f }
  },
  {  // �O
    { -0.5f, -0.5f,  0.5f },
    {  0.5f, -0.5f,  0.5f },
    {  0.5f,  0.5f,  0.5f },
    { -0.5f,  0.5f,  0.5f }
  },
  { // ��
    { -0.5f, -0.5f, -0.5f },
    { -0.5f, -0.5f,  0.5f },
    { -0.5f,  0.5f,  0.5f },
    { -0.5f,  0.5f, -0.5f }
  },
  { // ��
    { -0.5f,  0.5f,  0.5f },
    {  0.5f,  0.5f,  0.5f },
    {  0.5f,  0.5f, -0.5f },
    { -0.5f,  0.5f, -0.5f }
  },
};

/* �ʂ̖@���x�N�g�� */
static const float normal[][4][3] = {
  { // ��
    {  0.0f, -1.0f,  0.0f },
    {  0.0f, -1.0f,  0.0f },
    {  0.0f, -1.0f,  0.0f },
    {  0.0f, -1.0f,  0.0f }
  },
  { // ��
    {  0.0f,  0.0f, -1.0f },
    {  0.0f,  0.0f, -1.0f },
    {  0.0f,  0.0f, -1.0f },
    {  0.0f,  0.0f, -1.0f }
  },
  { // �E
    {  1.0f,  0.0f,  0.0f },
    {  1.0f,  0.0f,  0.0f },
    {  1.0f,  0.0f,  0.0f },
    {  1.0f,  0.0f,  0.0f }
  },
  { // �O
    {  0.0f,  0.0f,  1.0f },
    {  0.0f,  0.0f,  1.0f },
    {  0.0f,  0.0f,  1.0f },
    {  0.0f,  0.0f,  1.0f }
  },
  { // ��
    { -1.0f,  0.0f,  0.0f },
    { -1.0f,  0.0f,  0.0f },
    { -1.0f,  0.0f,  0.0f },
    { -1.0f,  0.0f,  0.0f }
  },
  { // ��
    {  0.0f,  1.0f,  0.0f },
    {  0.0f,  1.0f,  0.0f },
    {  0.0f,  1.0f,  0.0f },
    {  0.0f,  1.0f,  0.0f }
  },
};

/* ���_�̃e�N�X�`�����W */
static const float texcoord[][4][2] = {
  { // ��
    { 0.0f,   1.0f },
    { 0.125f, 1.0f },
    { 0.125f, 0.0f },
    { 0.0f,   0.0f }
  },
  { // ��
    { 0.125f, 1.0f },
    { 0.25f,  1.0f },
    { 0.25f,  0.0f },
    { 0.125f, 0.0f }
  },
  { // �E
    { 0.25f,  1.0f },
    { 0.375f, 1.0f },
    { 0.375f, 0.0f },
    { 0.25f,  0.0f }
  },
  { // �O
    { 0.375f, 1.0f },
    { 0.5f,   1.0f },
    { 0.5f,   0.0f },
    { 0.375f, 0.0f }
  },
  { // ��
    { 0.5f,   1.0f },
    { 0.625f, 1.0f },
    { 0.625f, 0.0f },
    { 0.5f,   0.0f }
  },
  { // ��
    { 0.625f, 1.0f },
    { 0.75f,  1.0f },
    { 0.75f,  0.0f },
    { 0.625f, 0.0f }
  },
};

/*
** ���̃R���X�g���N�^
*/
Box::Box(float x, float y, float z)
{
  size(x, y, z);
}

/*
** ���̃T�C�Y�ݒ�
*/
void Box::size(float x, float y, float z)
{
  for (int j = 0; j < 6; ++j) {
    for (int i = 0; i < 4; ++i) {
      vertex[j][i][0] = original[j][i][0] * x;
      vertex[j][i][1] = original[j][i][1] * y;
      vertex[j][i][2] = original[j][i][2] * z;
    }
  }
}

/*
** ���̕`��
*/
void Box::draw(void)
{
  /* ���_�f�[�^�C�@���f�[�^�C�e�N�X�`�����W�̔z���L���ɂ��� */
  glEnableClientState(GL_VERTEX_ARRAY);
  glEnableClientState(GL_NORMAL_ARRAY);
  glEnableClientState(GL_TEXTURE_COORD_ARRAY);

  /* ���_�f�[�^�C�@���f�[�^�C�e�N�X�`�����W�̏ꏊ���w�肷�� */
  glVertexPointer(3, GL_FLOAT, 0, vertex);
  glNormalPointer(GL_FLOAT, 0, normal);
  glTexCoordPointer(2, GL_FLOAT, 0, texcoord);

  /* �}�`��`�悷�� */
  glDrawArrays(GL_QUADS, 0, sizeof(vertex) / sizeof(vertex[0][0]));

  /* ���_�f�[�^�C�@���f�[�^�C�e�N�X�`�����W�̔z��𖳌��ɂ��� */
  glDisableClientState(GL_VERTEX_ARRAY);
  glDisableClientState(GL_NORMAL_ARRAY);
  glDisableClientState(GL_TEXTURE_COORD_ARRAY);
}
