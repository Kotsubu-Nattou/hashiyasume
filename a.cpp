#include <iostream>
#include <GLEW/glew.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <vector>
#include <cmath>
#include <ctime>
#include <windows.h>
using namespace std;

namespace {
	const bool IS_FULL_SCREEN = false; // �t���X�N���[�����ǂ��� 
	const GLint WIDTH  = 1280;        // ��ʃT�C�Y�B�t���X�N���[���̏ꍇ�́A�𑜓x�ƂȂ�
	const GLint HEIGHT = 720;
	const GLint TEX_SIZE = 256;
	const GLint OBJ_MAX = 400;
	const int64_t WAIT_TIME = 33;  // 1���[�v�̍Œ�X�V���ԁi�l��fps=60�̎���2�t���[����ms�j
	const char *CONFIG_FILE = "Config.bin";
	const double PI_MUL2   = M_PI * 2.0;
	const double PI_DIV180 = M_PI / 180;

	struct TYPE_POS {
		GLfloat x;
		GLfloat y;
	};

	struct TYPE_RGB {
		GLclampf r;
		GLclampf g;
		GLclampf b;
	};

	struct TYPE_RGBA {
		GLfloat r;
		GLfloat g;
		GLfloat b;
		GLfloat a;
	};

	struct TYPE_OBJ {
		TYPE_POS  vtx[OBJ_MAX];
		TYPE_RGBA col[OBJ_MAX];
		GLfloat   size[OBJ_MAX];
		int       angle[OBJ_MAX];
		float     speed[OBJ_MAX];
	};

	struct TYPE_BLEND_FACTOR {
		GLenum src;
		GLenum dst;
	};

	struct TYPE_ATMOS{
		int               useObjNum;
		GLfloat	          bler;
		GLfloat           focas;
		GLfloat           brightness;
		TYPE_RGB          baseCol;
		TYPE_RGB          filterCol;
		GLclampf          filterColVol;
		GLfloat           fixSize;
		GLfloat           fixSpeed;
		int               blendFactorId;
		int               blendFactorMax;
		TYPE_BLEND_FACTOR blendFactor[10];
		int               blendEquationId;
		int               blendEquationMax;
		GLenum            blendEquation[10];
	};
}





void
LoadConfig(const char *fileName, TYPE_ATMOS &atmos)
{
	// �y�֐��z�R���t�B�O�t�@�C�������[�h�B�t�@�C����������Ȃ��ꍇ�A�������Ȃ�
	ifstream fs(fileName, ios::binary);

	if (fs) {
		fs.seekg(0, fs.end);
		uint64_t fileSize = fs.tellg();
		if (fileSize == sizeof(TYPE_ATMOS)) {
			fs.seekg(0, fs.beg);
			fs.read(reinterpret_cast<char*>(&atmos), sizeof(TYPE_ATMOS));
		}
	}

	fs.close();

	return;
}





void
SaveConfig(const char *fileName, TYPE_ATMOS &atmos)
{
	// �y�֐��z�R���t�B�O�t�@�C�����Z�[�u�B�t�@�C����������Ȃ��ꍇ�A�V�����t�@�C���������
	ofstream fs(fileName, ios::binary);
	fs.write(reinterpret_cast<char*>(&atmos), sizeof(TYPE_ATMOS));
	fs.close();

	return;
}




GLclampf
ProcessEfxFlash(bool bang = false)
{
	// �y�֐��z��ʂ̔����G�t�F�N�g
	// ��������   true��n���Ɣ����̃g���K�[�ɂȂ�i�����̃g���K�[���ȊO��false�j
	// ���߂�l�� ���݂̔������x�iintensity�j
	// �g���K�[�ȍ~�A�֐����Ă΂��x��intensity������������i���̊֐���intensity�����������Ȃ��j
	static GLclampf intensity = 0.0f;
	static const GLclampf decay = 0.9f;

	intensity *= decay;
	if (intensity < 0.01f) intensity = 0.0f;
	if (bang) intensity = 1.0f;


	return intensity;
}





bool
CheckControllerEvent(GLFWwindow *window, TYPE_ATMOS &atmos)
{
	// �y�֐��z���͑���̔���
	// ���߂�l���A�v���I���̑���Ȃ�true�A����ȊOfalse
	static bool flgMouseBtn[GLFW_MOUSE_BUTTON_LAST+1];  // �������s�v�BC++��static�ϐ��͎����ŏ�����
	static bool flgKey[GLFW_KEY_LAST+1];                // �������s�v�BC++��static�ϐ��͎����ŏ�����


	// @ �A�v���I������
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) return true;
	if (glfwGetKey(window, GLFW_KEY_SPACE)  == GLFW_PRESS) return true;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) return true;

	// @ ���͋C�����_��
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
		if (!flgMouseBtn[GLFW_MOUSE_BUTTON_RIGHT]) {
			atmos.useObjNum     = 3 + rand() % (OBJ_MAX-3);
			atmos.bler          = (rand() % 1000) / 1000.0f;
			atmos.focas         = (rand() % 1000) / 1000.0f;
			atmos.blendFactorId = rand() % atmos.blendFactorMax;
			atmos.fixSize       = 1.0f;
			if (rand() % 4 == 1) {
				atmos.fixSize = 0.1f + (rand() % 1000) / 200.0f;
			}
			atmos.blendEquationId = 0;
			if (rand() % 5 == 1) {
				atmos.blendEquationId = rand() % atmos.blendEquationMax;
			}
			flgMouseBtn[GLFW_MOUSE_BUTTON_RIGHT] = true;
		}
	}
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE) {
		flgMouseBtn[GLFW_MOUSE_BUTTON_RIGHT] = false;
	}

	// @ ��ʂ̖��邳����
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		atmos.brightness += 0.05f;
		if (atmos.brightness > 1.0f) atmos.brightness = 1.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		atmos.brightness -= 0.05f;
		if (atmos.brightness < 0.0f) atmos.brightness = 0.0f;
	}

	// @ �J���[�t�B���^�[
	// �ԐF
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		if (!flgKey[GLFW_KEY_R]) {
			atmos.filterCol.r = (atmos.filterCol.r == 0.0f) ? atmos.filterColVol : 0.0f;
			flgKey[GLFW_KEY_R] = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
		flgKey[GLFW_KEY_R] = false;
	}
	// �ΐF
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
		if (!flgKey[GLFW_KEY_G]) {
			atmos.filterCol.g = (atmos.filterCol.g == 0.0f) ? atmos.filterColVol : 0.0f;
			flgKey[GLFW_KEY_G] = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) {
		flgKey[GLFW_KEY_G] = false;
	}
	// �F
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
		if (!flgKey[GLFW_KEY_B]) {
			atmos.filterCol.b = (atmos.filterCol.b == 0.0f) ? atmos.filterColVol : 0.0f;
			flgKey[GLFW_KEY_B] = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
		flgKey[GLFW_KEY_B] = false;
	}

	// @ �A���t�@�u�����f�B���O�W���̑g�ݍ��킹
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
		if (!flgKey[GLFW_KEY_LEFT]) {
			atmos.blendFactorId -= 1;
			if (atmos.blendFactorId < 0) atmos.blendFactorId = atmos.blendFactorMax-1;
			flgKey[GLFW_KEY_LEFT] = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE) {
		flgKey[GLFW_KEY_LEFT] = false;
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
		if (!flgKey[GLFW_KEY_RIGHT]) {
			atmos.blendFactorId += 1;
			if (atmos.blendFactorId >= atmos.blendFactorMax) atmos.blendFactorId = 0;
			flgKey[GLFW_KEY_RIGHT] = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE) {
		flgKey[GLFW_KEY_RIGHT] = false;
	}

	// @ ���͋C�f�[�^�̃Z�[�u
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
		if (!flgKey[GLFW_KEY_ENTER]) {
			SaveConfig(CONFIG_FILE, atmos);
			ProcessEfxFlash(true);
			flgKey[GLFW_KEY_ENTER] = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_RELEASE) {
		flgKey[GLFW_KEY_ENTER] = false;
	}


	return false;
}





void
DoWait(const int64_t elapse_ms, const int64_t wait_ms)
{
	// �y�֐��z�������Ԃɉ������E�F�C�g
	// ��������elapse_ms�͌v���J�n����̌o�ߎ��ԁBwait_ms�͍Œ�E�F�C�g���鎞�ԁB
	// ���ۂɃE�F�C�g���鎞�� = wait_ms - elapse_ms
	// �P�ʂ͂��ׂă~���b�BWindows�ˑ��i�v���J�n�_��glfwSetTime(0.0)�����s���Ă������Ɓj
	int64_t ms = wait_ms - elapse_ms;
	if (ms <= 0) return;
	if (ms > wait_ms) ms = wait_ms;
	Sleep(static_cast<DWORD>(ms));

	return;
}





void
MoveBoll(TYPE_OBJ &obj, const TYPE_ATMOS &atmos)
{
	// �y�֐��z�I�u�W�F�N�g�̈ړ��Ɣ���
	for (int i = 0; i < atmos.useObjNum; ++i) {
		float rad = static_cast<float>(obj.angle[i] * PI_DIV180);
		obj.vtx[i].x += cos(rad) * obj.speed[i] * atmos.fixSpeed;
		obj.vtx[i].y += sin(rad) * obj.speed[i] * atmos.fixSpeed;
		if (obj.vtx[i].x < -1.0f || obj.vtx[i].x >= 1.0f) {
			obj.angle[i] = (540 - obj.angle[i]) % 360;
		}
		if (obj.vtx[i].y < -1.0f || obj.vtx[i].y >= 1.0f) {
			obj.angle[i] = (360 - obj.angle[i]) % 360;
		}
	}

	return;
}





void
MakeTexture(GLuint *texId)
{
	// �y�֐��z���`�̃e�N�X�`�����쐬
	// <�߂�l> �����Ƀe�N�X�`�����ʎq��Ԃ�

	// @ �e�N�X�`���𐶐�
	glGenTextures(1, texId);
	glBindTexture(GL_TEXTURE_2D, *texId);
	glTexImage2D(GL_TEXTURE_2D,
				 0, GL_RGBA, TEX_SIZE, TEX_SIZE,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	// @ ��Ɨp�̃t���[���o�b�t�@�𐶐�
	GLuint fb;
	glGenFramebuffers(1, &fb);
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	// �e�N�X�`���ƕR�t����
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						   GL_TEXTURE_2D, *texId, 0);


	// @ �e�N�X�`���̓��e��`��
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glViewport(0, 0, TEX_SIZE, TEX_SIZE);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // �ʏ�̍���
	int divide = 50;      // �~�̕�����
	GLfloat size = 1.0f;  // �~�̑傫��
	GLfloat angle;        // �p�x
	TYPE_POS vtx[divide]; // ���_�z��B�v�f���͕�����
	for (int i = 0; i < 20; ++i) {
		size = 1.0f - i / 50.0f;
		for (int j = 0; j < divide; ++j) {
			angle = PI_MUL2 * j / divide;
			vtx[j].x = static_cast<GLfloat>(sin(angle) * size);
			vtx[j].y = static_cast<GLfloat>(cos(angle) * size);
		}
		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glColor4f(0.0f, 0.0f, 0.0f, 0.02f+i/48.0f); // ��ŐF�Â�����̂�rgb��0�A�A���t�@���Z���ɂȂ�B�܂��Ai/n��n������������ƑN�₩�ɂȂ�
		glDrawArrays(GL_TRIANGLE_FAN, 0, divide);
	}
	glDisableClientState(GL_VERTEX_ARRAY);


	// ��Ɨp�̃t���[���o�b�t�@��j��
	glDeleteFramebuffers(1, &fb);

	return;
}





// ------------------------------------------------------------------------------------------
// �y���C���z
// ------------------------------------------------------------------------------------------





int
main()
{
	// @@@ OpenGL�y�сAGLFW�̏����ݒ�
	// GLFW��������
	if (!glfwInit()) return -1;
	// �E�B���h�E�̓���
	glfwWindowHint(GLFW_SAMPLES, 4);
	// �E�B���h�E�𐶐����A���ʎq���擾
	// glfwCreateWindow�̑�4������glfwGetPrimaryMonitor��n���ƁA�t���X�N���[���ɂȂ�B
	// ���̍ہAwidth��height���𑜓x�ƂȂ�B�����ł̓A�X�y�N�g��16:9�́A1280x720�i720p�j���w�肵���B
	GLFWmonitor *monitor = NULL;
	if (IS_FULL_SCREEN) monitor = glfwGetPrimaryMonitor();
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "myApp", monitor , NULL);
	// GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "myApp", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	// �J�����g�E�B���h�E�Ɏw��
	glfwMakeContextCurrent(window);
	// GLEW��������
	if(glewInit() != GLEW_OK) {
		glfwTerminate();
		return -1;
	}
	// �f�t�H���g�̃t���[���o�b�t�@���ʎq���擾�i��Ŏg�p�j
	GLint showFrameBuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &showFrameBuffer);
	// ��������
	glfwSwapInterval(1);  // �ŋ߂�OS���̂��_�u���o�b�t�@���g�����߁A���̐ݒ�͖��������\��������
	// �}�E�X�J�[�\�����\���ɂ���
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);



	// @@@ �I�t�X�N���[�������_�����O�̏���
	// �e�N�X�`����2������
	GLuint showTex;
	glGenTextures(1, &showTex);
	glBindTexture(GL_TEXTURE_2D, showTex);
	glTexImage2D(GL_TEXTURE_2D,
				 0, GL_RGBA, WIDTH, HEIGHT,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	GLuint shadeTex;
	glGenTextures(1, &shadeTex);
	glBindTexture(GL_TEXTURE_2D, shadeTex);
	glTexImage2D(GL_TEXTURE_2D,
				 0, GL_RGBA, WIDTH, HEIGHT,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	// �I�t�X�N���[���p�̃t���[���o�b�t�@�𐶐�
	GLuint shadeFrameBuffer;
	glGenFramebuffers(1, &shadeFrameBuffer);
	glBindFramebuffer(GL_FRAMEBUFFER, shadeFrameBuffer);
	// �e�N�X�`���ƕR�t����
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						   GL_TEXTURE_2D, shadeTex, 0);
	// ��ʂ����ς��ɍL����傫���̃e�N�X�`���f�[�^
	const GLfloat groundTexVtx[] = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		 1.0f,  1.0f,
		-1.0f,  1.0f
	};
	const GLfloat groundTexUv[] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		1.0f, 1.0f,
		0.0f, 1.0f
	};
	


	// @@@ �I�u�W�F�N�g
	TYPE_OBJ obj;
	// ���`�e�N�X�`�����쐬
	GLuint texId;
	MakeTexture(&texId);
	// ���_�������_���Ő���
	srand((unsigned int)time(NULL));
	for (int i = 0; i < OBJ_MAX; ++i) {
		obj.vtx[i].x = (rand() % 10000) / 5000.0f - 1.0f;
		obj.vtx[i].y = (rand() % 10000) / 5000.0f - 1.0f;
	}
	// �J���[�������_���Ő���
	for (int i = 0; i < OBJ_MAX; ++i) {
		do {
			obj.col[i].r = (rand() % 1000) / 1000.0f;
			obj.col[i].g = (rand() % 1000) / 1000.0f;
			obj.col[i].b = (rand() % 1000) / 1000.0f;
		} while(obj.col[i].r+obj.col[i].g+obj.col[i].b < 0.05f);
		obj.col[i].a = 1.0f;
	}
	// ���̑��̃f�[�^�������_���Ő���
	for (int i = 0; i < OBJ_MAX; ++i) {
		// �T�C�Y
		obj.size[i]  = 10.0f + (rand() % 150);
		if (rand() % 5 == 1) obj.size[i] *= 2.0f;
		// ���p
		do {
			obj.angle[i] = rand() % 360;
		} while(obj.angle[i] % 90 == 0);
		// �X�s�[�h
		obj.speed[i] = 0.001f + (rand() % 100) / 8000.0f;
	}



	// @@@ ���͋C�p�����[�^
	TYPE_ATMOS atmos;
	// @ �A���t�@�u�����h�W���̑g�ݍ��킹
	// �ʏ�̍���
	atmos.blendFactor[0].src = GL_SRC_ALPHA;
	atmos.blendFactor[0].dst = GL_ONE_MINUS_SRC_ALPHA;
	// ���Z����
	atmos.blendFactor[1].src = GL_SRC_ALPHA;
	atmos.blendFactor[1].dst = GL_ONE;
	// �w���ȍ���
	atmos.blendFactor[2].src = GL_SRC_ALPHA_SATURATE;
	atmos.blendFactor[2].dst = GL_ONE;
	// �l�K����
	atmos.blendFactor[3].src = GL_SRC_ALPHA_SATURATE;
	atmos.blendFactor[3].dst = GL_ONE_MINUS_SRC_ALPHA;
	// �g�ݍ��킹�̑���
	atmos.blendFactorMax     = 4;
	// @ �A���t�@�u�����h�̌v�Z���@
	// �ʏ�̉��Z�i�e�N�X�`���̊G���o�͂����j
	atmos.blendEquation[0] = GL_FUNC_ADD;
	// ���ӂ̑傫���ق������̂܂܎g�p�i�e�N�X�`���g�̎l�p�`���o�͂����j
	atmos.blendEquation[1] = GL_MAX;
	// �v�Z���@�̑���
	atmos.blendEquationMax = 2;
	// @ �e�����l
	atmos.useObjNum       = 200;    // �\������{�[���̐�
	atmos.bler            = 0.0f;   // �l���Ⴂ�قǃu���[��������
	atmos.focas           = 0.12f;  // �l�������قǑN���ɂȂ�
	atmos.brightness      = 1.0f;   // ��ʂ̖��邳
	atmos.baseCol.r       = 0.0f;   // ���n�̐F�i�傫���ƖO�a����j
	atmos.baseCol.g       = 0.0f;
	atmos.baseCol.b       = 0.0f;
	atmos.filterCol.r     = 0.0f;   // �J���[�t�B���^�[
	atmos.filterCol.g     = 0.0f;
	atmos.filterCol.b     = 0.0f;
	atmos.filterColVol    = 0.2f;   // �J���[�t�B���^�[�̔Z��
	atmos.fixSize         = 1.0f;   // �{�[���̃T�C�Y�̒���
	atmos.fixSpeed        = 1.0f;   // �{�[���̃X�s�[�h�̒���
	atmos.blendFactorId   = 1;      // �A���t�@�u�����h�W���̑g�ݍ��킹
	atmos.blendEquationId = 0;      // �A���t�@�u�����h�̌v�Z���@
	// @ �����R���t�B�O�t�@�C�������݂���Ȃ�ݒ���㏑��
	LoadConfig(CONFIG_FILE, atmos);
	// @ �G�t�F�N�g�p�̕ϐ�
	GLclampf efxFlashIntensity = 0.0f;



	// @@@ GLFW���[�v -----------------------------------------------------------------------------
	while (!glfwWindowShouldClose(window)) {
		glfwSetTime(0.0);
		if (CheckControllerEvent(window, atmos)) break;


		// @ �L���v�`���B�O�̃R�}���\���e�N�X�`���ɃR�s�[
		glBindFramebuffer(GL_FRAMEBUFFER, shadeFrameBuffer);
		glClearColor(atmos.baseCol.r + atmos.filterCol.r + efxFlashIntensity,
					 atmos.baseCol.g + atmos.filterCol.g + efxFlashIntensity,
					 atmos.baseCol.b + atmos.filterCol.b + efxFlashIntensity, 
					 atmos.bler);
		glClear(GL_COLOR_BUFFER_BIT);
		glCopyImageSubData(showTex,  GL_TEXTURE_2D, 0, 0, 0, 0,
						   shadeTex, GL_TEXTURE_2D, 0, 0, 0, 0,
						   WIDTH, HEIGHT, 0);


		// @ �|�C���g�X�v���C�g
		glBindTexture(GL_TEXTURE_2D, texId);
		glViewport(0, 0, WIDTH, HEIGHT);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glBlendFunc(atmos.blendFactor[atmos.blendFactorId].src, atmos.blendFactor[atmos.blendFactorId].dst);
		glBlendEquation(atmos.blendEquation[atmos.blendEquationId]);
		// �|�C���g�X�v���C�g��L���ɂ���
		glEnable(GL_POINT_SPRITE); 
		// �|�C���g�X�v���C�g�ɑ΂��āA�e�N�X�`�����W�̎����v�Z��ON
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);
		// �|�C���g�X�v���C�g�̂ǂ̈ʒu���e�N�X�`�����W�́u���_�v�Ƃ��邩�w��
		glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);
		// �e�N�X�`���̐F�̕t�����B���`�e�N�X�`���̐F(0,0,0,�Z�x)�ɁA�t�������F(r,g,b,1)������������
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
		// �ړ��Ɣ���
		MoveBoll(obj, atmos);
		// �_��`��
		glVertexPointer(2, GL_FLOAT, 0, obj.vtx);
		glColorPointer(4, GL_FLOAT, 0, obj.col);
		for (int i = 0; i < atmos.useObjNum; ++i) {
			glPointSize(obj.size[i] * atmos.fixSize);
			glDrawArrays(GL_POINTS, i, 1);
		}
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisable(GL_POINT_SPRITE);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);
		glBlendEquation(GL_FUNC_ADD);


		// @ ������摜���A���C���t���[���̕`��o�b�t�@�ɁA�e�N�X�`���Ƃ��ĕ`��
		glBindFramebuffer(GL_FRAMEBUFFER, showFrameBuffer);
		glBindTexture(GL_TEXTURE_2D, shadeTex);
		glViewport(0, 0, WIDTH, HEIGHT);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // �ʏ�̍���
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);  // �f�t�H���g
		glVertexPointer(2, GL_FLOAT, 0, groundTexVtx);
		glTexCoordPointer(2, GL_FLOAT, 0, groundTexUv);
		glColor4f(atmos.brightness, atmos.brightness, atmos.brightness, atmos.focas);
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);


		// @ �㏈��
		efxFlashIntensity = ProcessEfxFlash();
		// printf("Time(s):%f\n", glfwGetTime());  // 1���[�v�̏����ȏ�������
		DoWait(static_cast<int64_t>(glfwGetTime()*1000.0), WAIT_TIME);  // CPU���׌y���i30fps�j
		glfwSwapBuffers(window);
		glfwPollEvents();
	}



	// @@@ �I������
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glDeleteTextures(1, &texId);
	glDeleteTextures(1, &showTex);
	glDeleteTextures(1, &shadeTex);
	glDeleteFramebuffers(1, &shadeFrameBuffer);
	glfwTerminate();


	return 0;
}