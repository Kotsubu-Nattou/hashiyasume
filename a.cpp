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
	const bool IS_FULL_SCREEN = false; // フルスクリーンかどうか 
	const GLint WIDTH  = 1280;         // 画面サイズ。フルスクリーンの場合は、解像度となる
	const GLint HEIGHT = 720;
	const GLint TEX_SIZE = 256;
	const GLint OBJ_MAX = 400;
	const int64_t WAIT_TIME = 33;      // 1ループの最低更新時間（値はfps=60の時の2フレームのms）
	const char *CONFIG_FILE = "Config.bin";
	const double PI_MUL2   = M_PI * 2.0;
	const double PI_DIV180 = M_PI / 180.0;

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

	class CLASS_EFX_FLASH {
		// 【クラス】画面の発光エフェクト
		private:
			// フィールド
			GLclampf intensity   = 0.0f;  // C++11以降対応の初期化方法
			const GLclampf decay = 0.9f;
		
		public:
			GLclampf getIntensity() {
				// 【ゲッタ】現在の発光強度を返す
				return this->intensity;
			}

			void bang() {
				// 【メソッド】エフェクトをトリガー
				this->intensity = 1.0f;
			}

			void proceed() {
				// 【メソッド】エフェクトを推移する
				this->intensity *= this->decay;
				if (this->intensity < 0.01f) this->intensity = 0.0f;
			}
	};
}





void
LoadConfig(const char *fileName, TYPE_ATMOS &atmos)
{
	// 【関数】コンフィグファイルをロード。ファイルが見つからない場合、何もしない
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
	// 【関数】コンフィグファイルをセーブ。ファイルが見つからない場合、新しいファイルが作られる
	ofstream fs(fileName, ios::binary);
	fs.write(reinterpret_cast<char*>(&atmos), sizeof(TYPE_ATMOS));
	fs.close();

	return;
}





bool
CheckControllerEvent(GLFWwindow *window, TYPE_ATMOS &atmos, CLASS_EFX_FLASH &EfxFlash)
{
	// 【関数】入力操作の判定
	// ＜戻り値＞アプリ終了の操作ならtrue、それ以外false
	static bool flgMouseBtn[GLFW_MOUSE_BUTTON_LAST+1];  // 初期化不要。C++のstatic変数は自動で初期化
	static bool flgKey[GLFW_KEY_LAST+1];


	// @ アプリ終了操作
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) return true;
	if (glfwGetKey(window, GLFW_KEY_SPACE)  == GLFW_PRESS) return true;
	if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) return true;

	// @ 雰囲気ランダム
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

	// @ 画面の明るさ調整
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
		atmos.brightness += 0.05f;
		if (atmos.brightness > 1.0f) atmos.brightness = 1.0f;
	}
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
		atmos.brightness -= 0.05f;
		if (atmos.brightness < 0.0f) atmos.brightness = 0.0f;
	}

	// @ カラーフィルター
	// 赤色
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS) {
		if (!flgKey[GLFW_KEY_R]) {
			atmos.filterCol.r = (atmos.filterCol.r == 0.0f) ? atmos.filterColVol : 0.0f;
			flgKey[GLFW_KEY_R] = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_R) == GLFW_RELEASE) {
		flgKey[GLFW_KEY_R] = false;
	}
	// 緑色
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS) {
		if (!flgKey[GLFW_KEY_G]) {
			atmos.filterCol.g = (atmos.filterCol.g == 0.0f) ? atmos.filterColVol : 0.0f;
			flgKey[GLFW_KEY_G] = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE) {
		flgKey[GLFW_KEY_G] = false;
	}
	// 青色
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS) {
		if (!flgKey[GLFW_KEY_B]) {
			atmos.filterCol.b = (atmos.filterCol.b == 0.0f) ? atmos.filterColVol : 0.0f;
			flgKey[GLFW_KEY_B] = true;
		}
	}
	if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE) {
		flgKey[GLFW_KEY_B] = false;
	}

	// @ アルファブレンディング係数の組み合わせ
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

	// @ 雰囲気データのセーブ
	if (glfwGetKey(window, GLFW_KEY_ENTER) == GLFW_PRESS) {
		if (!flgKey[GLFW_KEY_ENTER]) {
			SaveConfig(CONFIG_FILE, atmos);
			EfxFlash.bang();
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
	// 【関数】処理時間に応じたウェイト
	// ＜引数＞elapse_msは計測開始からの経過時間。wait_msは最低ウェイトする時間。
	// 単位はすべてミリ秒。Windows依存（計測開始点でglfwSetTime(0.0)を実行しておくこと）
	// 式。実際にウェイトする時間 = wait_ms - elapse_ms
	int64_t ms = wait_ms - elapse_ms;
	if (ms <= 0) return;
	if (ms > wait_ms) ms = wait_ms;
	Sleep(static_cast<DWORD>(ms));

	return;
}





void
MoveBoll(TYPE_OBJ &obj, const TYPE_ATMOS &atmos)
{
	// 【関数】オブジェクトの移動と反射
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
	// 【関数】雛形のテクスチャを作成
	// <戻り値> 引数にテクスチャ識別子を返す

	// @ テクスチャを生成
	glGenTextures(1, texId);
	glBindTexture(GL_TEXTURE_2D, *texId);
	glTexImage2D(GL_TEXTURE_2D,
				 0, GL_RGBA, TEX_SIZE, TEX_SIZE,
				 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);


	// @ 作業用のフレームバッファを生成
	GLuint fb;
	glGenFramebuffers(1, &fb);
	// テクスチャと紐付ける
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						   GL_TEXTURE_2D, *texId, 0);


	// @ テクスチャの内容を描画
	glBindFramebuffer(GL_FRAMEBUFFER, fb);
	glViewport(0, 0, TEX_SIZE, TEX_SIZE);
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glEnableClientState(GL_VERTEX_ARRAY);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // 通常の合成
	int divide = 50;      // 円の分割数
	GLfloat size = 1.0f;  // 円の大きさ
	GLfloat angle;        // 角度
	TYPE_POS vtx[divide]; // 頂点配列。要素数は分割数
	for (int i = 0; i < 20; ++i) {
		size = 1.0f - i / 50.0f;
		for (int j = 0; j < divide; ++j) {
			angle = PI_MUL2 * j / divide;
			vtx[j].x = static_cast<GLfloat>(sin(angle) * size);
			vtx[j].y = static_cast<GLfloat>(cos(angle) * size);
		}
		glVertexPointer(2, GL_FLOAT, 0, vtx);
		glColor4f(0.0f, 0.0f, 0.0f, 0.02f+i/48.0f); // 後で色づけするのでrgbは0、アルファが濃さになる。また、i/nのnを小さくすると鮮やかになる
		glDrawArrays(GL_TRIANGLE_FAN, 0, divide);
	}
	glDisableClientState(GL_VERTEX_ARRAY);


	// 作業用のフレームバッファを破棄
	glDeleteFramebuffers(1, &fb);

	return;
}





// ------------------------------------------------------------------------------------------
// 【メイン】
// ------------------------------------------------------------------------------------------





int
main()
{
	// @@@ OpenGL及び、GLFWの初期設定
	// GLFWを初期化
	if (!glfwInit()) return -1;
	// ウィンドウの特性
	glfwWindowHint(GLFW_SAMPLES, 4);
	// ウィンドウを生成し、識別子を取得
	// glfwCreateWindowの第4引数にglfwGetPrimaryMonitorを渡すと、フルスクリーンになる。
	// その際、widthとheightが解像度となる。ここではアスペクト比16:9の、1280x720（720p）を指定した。
	GLFWmonitor *monitor = NULL;
	if (IS_FULL_SCREEN) monitor = glfwGetPrimaryMonitor();
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "myApp", monitor , NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	// カレントウィンドウに指定
	glfwMakeContextCurrent(window);
	// GLEWを初期化
	if(glewInit() != GLEW_OK) {
		glfwTerminate();
		return -1;
	}
	// デフォルトのフレームバッファ識別子を取得（後で使用）
	GLint showFrameBuffer;
	glGetIntegerv(GL_FRAMEBUFFER_BINDING, &showFrameBuffer);
	// 垂直同期
	glfwSwapInterval(1);  // 最近はOS自体がダブルバッファを使うため、この設定は無視される可能性がある
	// マウスカーソルを非表示にする
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);



	// @@@ オフスクリーンレンダリングの準備
	// テクスチャを2枚生成
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
	// オフスクリーン用のフレームバッファを生成
	GLuint shadeFrameBuffer;
	glGenFramebuffers(1, &shadeFrameBuffer);
	// テクスチャと紐付ける
	glBindFramebuffer(GL_FRAMEBUFFER, shadeFrameBuffer);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
						   GL_TEXTURE_2D, shadeTex, 0);
	// 画面いっぱいに広がる大きさのテクスチャデータ
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
	


	// @@@ 図形を準備
	TYPE_OBJ obj;
	// 雛形テクスチャを作成
	GLuint texId;
	MakeTexture(&texId);
	// 頂点をランダムで生成
	srand((unsigned int)time(NULL));
	for (int i = 0; i < OBJ_MAX; ++i) {
		obj.vtx[i].x = (rand() % 10000) / 5000.0f - 1.0f;
		obj.vtx[i].y = (rand() % 10000) / 5000.0f - 1.0f;
	}
	// カラーをランダムで生成
	for (int i = 0; i < OBJ_MAX; ++i) {
		do {
			obj.col[i].r = (rand() % 1000) / 1000.0f;
			obj.col[i].g = (rand() % 1000) / 1000.0f;
			obj.col[i].b = (rand() % 1000) / 1000.0f;
		} while(obj.col[i].r+obj.col[i].g+obj.col[i].b < 0.05f);
		obj.col[i].a = 1.0f;
	}
	// その他のデータをランダムで生成
	for (int i = 0; i < OBJ_MAX; ++i) {
		// サイズ
		obj.size[i]  = 10.0f + (rand() % 150);
		if (rand() % 5 == 1) obj.size[i] *= 2.0f;
		// 方角
		do {
			obj.angle[i] = rand() % 360;
		} while(obj.angle[i] % 90 == 0);
		// スピード
		obj.speed[i] = 0.001f + (rand() % 100) / 8000.0f;
	}



	// @@@ 雰囲気パラメータ
	TYPE_ATMOS atmos;
	// @ アルファブレンド係数の組み合わせ
	// 通常の合成
	atmos.blendFactor[0].src = GL_SRC_ALPHA;
	atmos.blendFactor[0].dst = GL_ONE_MINUS_SRC_ALPHA;
	// 加算合成
	atmos.blendFactor[1].src = GL_SRC_ALPHA;
	atmos.blendFactor[1].dst = GL_ONE;
	// ヘンな合成
	atmos.blendFactor[2].src = GL_SRC_ALPHA_SATURATE;
	atmos.blendFactor[2].dst = GL_ONE;
	// ネガっぽい合成
	atmos.blendFactor[3].src = GL_SRC_ALPHA_SATURATE;
	atmos.blendFactor[3].dst = GL_ONE_MINUS_SRC_ALPHA;
	// 組み合わせの総数
	atmos.blendFactorMax     = 4;
	// @ アルファブレンドの計算方法
	// 通常の加算（テクスチャの絵が出力される）
	atmos.blendEquation[0] = GL_FUNC_ADD;
	// 両辺の大きいほうをそのまま使用（テクスチャ枠の四角形が出力される）
	atmos.blendEquation[1] = GL_MAX;
	// 計算方法の総数
	atmos.blendEquationMax = 2;
	// @ 各初期値
	atmos.useObjNum       = 200;    // 表示するボールの数
	atmos.bler            = 0.0f;   // 値が低いほどブラーがかかる
	atmos.focas           = 0.12f;  // 値が高いほど鮮明になる
	atmos.brightness      = 1.0f;   // 画面の明るさ
	atmos.baseCol.r       = 0.0f;   // 下地の色（大きいと飽和する）
	atmos.baseCol.g       = 0.0f;
	atmos.baseCol.b       = 0.0f;
	atmos.filterCol.r     = 0.0f;   // カラーフィルター
	atmos.filterCol.g     = 0.0f;
	atmos.filterCol.b     = 0.0f;
	atmos.filterColVol    = 0.2f;   // カラーフィルターの濃さ
	atmos.fixSize         = 1.0f;   // ボールのサイズの調整
	atmos.fixSpeed        = 1.0f;   // ボールのスピードの調整
	atmos.blendFactorId   = 1;      // アルファブレンド係数の組み合わせ
	atmos.blendEquationId = 0;      // アルファブレンドの計算方法
	// @ もしコンフィグファイルが存在するなら設定を上書き
	LoadConfig(CONFIG_FILE, atmos);


	// @@@ 画面エフェクトのオブジェクトを生成
	CLASS_EFX_FLASH EfxFlash;



	// @@@ GLFWループ -----------------------------------------------------------------------------
	while (!glfwWindowShouldClose(window)) {
		glfwSetTime(0.0);
		if (CheckControllerEvent(window, atmos, EfxFlash)) break;


		// @ 非表示テクスチャに図形を描画
		glBindFramebuffer(GL_FRAMEBUFFER, shadeFrameBuffer);
		glBindTexture(GL_TEXTURE_2D, texId);
		glViewport(0, 0, WIDTH, HEIGHT);
		glClearColor(atmos.baseCol.r + atmos.filterCol.r + EfxFlash.getIntensity(),
					 atmos.baseCol.g + atmos.filterCol.g + EfxFlash.getIntensity(),
					 atmos.baseCol.b + atmos.filterCol.b + EfxFlash.getIntensity(), 
					 atmos.bler);
		glClear(GL_COLOR_BUFFER_BIT);
		// 各設定
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glBlendFunc(atmos.blendFactor[atmos.blendFactorId].src, atmos.blendFactor[atmos.blendFactorId].dst);
		glBlendEquation(atmos.blendEquation[atmos.blendEquationId]);
		// ポイントスプライトの設定
		glEnable(GL_POINT_SPRITE); 
		glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);  // テクスチャ座標を自動計算する
		glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_UPPER_LEFT);  // テクスチャ座標の原点を指定
		// テクスチャの色の付き方。雛形テクスチャの色(0,0,0,濃度)に、付けたい色(r,g,b,1)を合成させる
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND);
		// 移動と反射
		MoveBoll(obj, atmos);
		// ポイントスプライトを描画
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


		// @ 作った画像をメインフレームの描画バッファに、テクスチャとして転写（クリアせず上書き）
		glBindFramebuffer(GL_FRAMEBUFFER, showFrameBuffer);
		glBindTexture(GL_TEXTURE_2D, shadeTex);
		glViewport(0, 0, WIDTH, HEIGHT);
		glEnable(GL_TEXTURE_2D);
		glEnable(GL_BLEND);
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_TEXTURE_COORD_ARRAY);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);  // 通常の合成
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);  // デフォルト
		glVertexPointer(2, GL_FLOAT, 0, groundTexVtx);
		glTexCoordPointer(2, GL_FLOAT, 0, groundTexUv);
		glColor4f(atmos.brightness, atmos.brightness, atmos.brightness, atmos.focas);
		glDrawArrays(GL_QUADS, 0, 4);
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_TEXTURE_COORD_ARRAY);
		glDisable(GL_TEXTURE_2D);
		glDisable(GL_BLEND);


		// @ 後処理
		EfxFlash.proceed();
		// printf("Time(s):%f\n", glfwGetTime());  // 1ループの純粋な処理時間
		DoWait(static_cast<int64_t>(glfwGetTime()*1000.0), WAIT_TIME);  // CPU負荷軽減（30fps）
		glfwSwapBuffers(window);
		glfwPollEvents();
	}



	// @@@ 終了処理
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glDeleteTextures(1, &texId);
	glDeleteTextures(1, &showTex);
	glDeleteTextures(1, &shadeTex);
	glDeleteFramebuffers(1, &shadeFrameBuffer);
	glfwTerminate();


	return 0;
}