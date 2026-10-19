#ifndef _GSlib_HPP_
#define _GSlib_HPP_
//27/08/24
//version modifiée le 08/05/26 (à moins d'oubli de modifier cette ligne ou autre). Première version de cette lib : 18/11/22
/*
Mises à jour:
22/07/23:	Ajout de 'Vec2si' et 'Vec2sui', update des 'drawTile()' partout.
03/08/23:	Ajout de 'col()',  'sign()'.
06/08/23:	Ajout de 'strToupper()'.
17/08/23: 	Ajout de 'drawRect()', surchages de 'operator==' et de 'operator<<'.
09/05/24:	Ajout de 'Game::RenderPresent()' et 'Game::RenderClear()' .
22/01/25:	Ajout des 'decimalPart()'.
16/02/25:	-> 'perlinNoise2D()'.
19/10/25:   Passage à la SDL3.
08/05/26:   "loadImg" -> "loadTexture"
06/08/26:   Camera2D_Base

*/
#ifdef GSLIB_RAYLIB_BACKEND // A définir à la compilation (s'il y a compilation des sources de GSlib) - car GSlib ne support qu'un seul backend à la fois.
	// Sinon s'il n'y a que utilisation, possibilité de la définir dans les sources du projet, AVANT inclusion des headers de GSlib.

	#include "backends/raylib/GSlib_raylib_backend.hpp"
#else
	#include "backends/SDL3/GSlib_SDL3_backend.hpp"
#endif
	#include <unordered_map>
	#include <functional>
	#include <iostream>
	#include <cstdlib>
	#include <cstdint>
	#include <cstring>
	#include <vector>
	#include <cstdio>
	#include <random>
	#include <memory>
	#include <string>
	#include <cctype>
	#include <ctime>
	#include <cmath>
	#include <new>


//Note: "Bk-Dp" -> dépendant du backend utilisé

namespace gs /*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
{


#ifndef _pi_
	#define _pi_ 3.141592653f
	#endif
#ifndef _2pi_
	#define _2pi_ 6.2831853f
	#endif



enum Signe 	{NEGATIVE=-1, POSITIVE=1};
enum Axis 	{X_AXIS, Y_AXIS};

#define INT_ERR_VAL -10



template<class T>
struct Vec2
{
	T x=static_cast<T>(0);
	T y=static_cast<T>(0);

	Point toPoint() const;
	FPoint toFPoint() const;

	template<class U>
	Vec2<U> to() const;


	Vec2<T> operator+(const Vec2<T>& right) const;
	Vec2<T> operator-(const Vec2<T>& right) const;
	Vec2<T> operator*(const Vec2<T>& right) const;
	Vec2<T> operator/(const Vec2<T>& right) const;
	Vec2<T> operator%(const Vec2<T>& right) const;
	Vec2<T> operator-() const;

	Vec2<T>& operator+=(const Vec2<T>& right);
	Vec2<T>& operator-=(const Vec2<T>& right);
	Vec2<T>& operator*=(const Vec2<T>& right);
	Vec2<T>& operator/=(const Vec2<T>& right);
	Vec2<T>& operator%=(const Vec2<T>& right);

	bool operator==(const Vec2<T>& right) const;
	bool operator!=(const Vec2<T>& right) const;

	Vec2<T> operator+(const T& val) const;
	Vec2<T> operator-(const T& val) const;
	Vec2<T> operator*(const T& val) const;
	Vec2<T> operator/(const T& val) const;
	Vec2<T> operator%(const T& val) const;

	Vec2<T>& operator+=(const T& val);
	Vec2<T>& operator-=(const T& val);
	Vec2<T>& operator*=(const T& val);
	Vec2<T>& operator/=(const T& val);
	Vec2<T>& operator%=(const T& val);

};




typedef Vec2<int> Vec2i;
typedef Vec2<short> Vec2si, Vec2s;
typedef Vec2<unsigned int> Vec2ui, Vec2u;
typedef Vec2<unsigned short> Vec2usi, Vec2us;

typedef Vec2<float> Vec2f;
typedef Vec2<double> Vec2d;

typedef Vec2<bool> Vec2b;
typedef Vec2<char> Vec2c;
typedef Vec2<unsigned char> Vec2uc;


struct Vec2i_Hash
{
    std::size_t operator()(const Vec2i& v) const
    {
        return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
    }
};


struct Vec_Bool_Int
{
	bool b=false;
	int i=0;
};

struct Vec_Bool_Float
{
	bool b=false;
	int f=0.f;
};


template<class T>
T dotProd(const Vec2<T>& vec1, const Vec2<T>& vec2);

template<class T>
T vecLen(const Vec2<T>& vec);

template<class T>
T vecAngleRad(const Vec2<T>& vec1, const Vec2<T>& vec2); //L'angle entre les deux vecteurs.


class Input
{
	public:

		Input() = default;
		~Input() = default;

		bool& operator[](std::string id);
		bool operator[](std::string id, bool new_state);

		std::unordered_map<std::string, bool>& getInputsMap() {return m_inputs;}


	private:
		std::unordered_map<std::string, bool> m_inputs;
};




class Mouse_Tracker : public Backend__Mouse_Tracker
{
	public:

		Mouse_Tracker()=default;
		~Mouse_Tracker()=default;

		// void processEvent(...); à utiliser si le backend choisi la définie (dans 'Backend__Mouse_Tracker') -- sinon 'update()' suffit.
		void update();

		bool isPreviouslyDown() const;
		bool isDown() const;
		Vec2f getPosWhenDown() const;
		Vec2f getPosWhenUp() const;
		Vec2f getPos() const;
		bool moved() const;
		bool last2DownClicksHaveSamePos() const;
		bool isMultiClick(char* out__clicks=nullptr) const;
		bool isDoubleClick() const;
		void incrMultiClickCount(char val);
		void setTargetBtn(bool is_left);
		bool targetBtnIsLeft() const;
		bool clickedDown() const;
		bool clickedUp() const;
		Vec2f getWheel() const;


	protected:

		bool       m_previously_down=false;
		char       m_next_time_operation=0; //0: rien, 1: activer 'previously_down', 2: désactiver 'previously_down'.
		Vec2f      m_pos; //la position courante
		Vec2f      m_prev_pos; //Sert à savoir s'il y a eu mouvement depuis.
		Vec2f      m_wheel;
};



struct Circle_Hitbox
{
	Vec2f center;
	float radius;
};

//___________________________________________________________________________________________________
//___________________________GESTIONNAIRE GENERAL DU JEU_____________________________________________
//---------------------------------------------------------------------------------------------------


class Game : public Backend__Game
{
	public :

		Game();
			//--------------------------------------------
		~Game();
			//____________________________________________
		
			//____________________________________________

		bool InitTheWindow(int window_w = 800, int window_h = 480, const char* window_title = "MyGame", uint32_t win_flags=0, const char* renderer_name=nullptr, bool try_enable_VSYNC=false); //Bk-Dp

		
	/*bp*/int getWinW(); //Bk-Dp
	/*bp*/int getWinH(); //Bk-Dp
	/*bp*/int getRenW(); //Bk-Dp
	/*bp*/int getRenH(); //Bk-Dp
		float getRenW_f();
		float getRenH_f();
		float dt() const; //Bk-Dp  //Nécessite '(Game ou gs)::processDeltaTime()' dans la boucle principale (selon le backend utilisé).
		void setWinSize(int w, int h);
	
		Texture* loadTexture(const std::string& pth, bool nearest=true);
		bool drawTile(Texture* tileset, FRect src, FRect dest, bool isRenderTheTarget = true);
	/*bp*/void fpsProcessBegin(); //Bk-Dp
	/*bp*/void fpsProcessEnd(uint32_t fps = 60); //Bk-Dp
	/*bp*/void processDeltaTime(); //Bk-Dp
		std::string lastError() const;
		void lastError(const std::string& str_info, const std::string& str_info_2="", const std::string& str_info_3="");
		void addToClearAtTheEnd(Texture* tex);
		void addToClearAtTheEnd(Renderer* renderer);
	
		
	/*bp*/void setWinIcon(const std::string& path); //Bk-Dp
	/*bp*/void setWinTitle(const std::string& new_title); //Bk-Dp
		void setAsMainGame();
		void setIsMainGame(const bool b);
		void setUserData(void* data);
		void* getUserData();
	/*bp*/void renPresent(); //Bk-Dp
	/*bp*/void renClear(); //Bk-Dp
	/*bp*/Color setDrawColor(Color new_color); //Bk-Dp
	/*bp*/Color getDrawColor(); //Bk-Dp
		bool isGameInit() const {return isInit;}


	protected :
		
		uint64_t m_frame_begin_ticks{0};
		uint64_t m_frame_end_ticks{0};
		bool m_fpsProcessBegin_is_ok=false;
		void* m_user_data=nullptr;

};







//--------------------------------------------------------------------------------------------
//____________________________________________________________________________________________
//________________________UTILS_______________________________________________________________

Vec2f getMousePos(float* x=nullptr, float* y=nullptr); //Bk-Dp

void logMessage(const char *msg, const char *_getError = nullptr, bool isFatalError = false);

void drawBegin(Renderer* ren, bool clear=true); //Bk-Dp
void drawEnd(Renderer* ren); //Bk-Dp

Texture* loadTexture(Renderer *renderer, const std::string& path, bool nearest=true); //Bk-Dp
Surface* loadSurface(const std::string& img_path); //Bk-Dp
Texture_Target* createTextureTarget(Renderer* ren, Vec2i size); //Bk-Dp
Surface* createSurface(Vec2i size, Color fill_color={0,0,0,0}); //Bk-Dp

bool drawTile(Renderer* ren, Texture_Target* target, Texture* tileset, FRect src, FRect dest, bool takeInToAccountTheTarget = true); //Bk-Dp
bool drawSurfaceToSurface(Surface* src, Rect* src_rect, Surface* dest, Rect* dest_rect);

void setClearColor(Renderer* ren, Color col); //Bk-Dp
Color getClearColor(Renderer* ren); //Bk-Dp

void addToClearAtTheEnd(Texture* tex);

void addToClearAtTheEnd(Renderer* ren);

void freeGivenResources();

float rad(float degre);

float degree(float rad);

int roundToInt(float x);

float paw2(float x);

int paw2(int x);

std::string lastError();

void lastError(const std::string& str_info, const std::string& str_info_2="", const std::string& str_info_3="");

bool lastErrorAutomaticLog();

std::string str(const char* c_string);

bool isInInterval_in_in(float a, float b, float val);

bool isInInterval_in_out(float a, float b, float val);

bool isInInterval_out_in(float a, float b, float val);

bool isInInterval_out_out(float a, float b, float val);

float euclideanDist(float x1, float y1, float x2, float y2);
int euclideanDist(int x1, int y1, int x2, int y2);

float pointsDist(float x1, float y1, float x2, float y2);
int pointsDist(int x1, int y1, int x2, int y2);

float pointsDist(Vec2f pt1, Vec2f pt2);
int pointsDist(Vec2i pt1, Vec2i pt2);

float manhattanDist(Vec2f pt1, Vec2f pt2);
int manhattanDist(Vec2i pt1, Vec2i pt2);

float meanXY(Vec2f v);
float meanXY(Vec2i v);

float getPointsAngle(float origin_x, float origin_y, float point_x, float point_y, bool clockwise=true);
float getPointsAngle(Vec2f origin, Vec2f point, bool clockwise=true);
#define getAngle getPointsAngle
float meanAngle(float angle_a, float angle_b);
float getVecAngle(Vec2f vec); //L'angle que formerait ce vecteur, s'il prenait origine dans le cercle trigo.

Vec2f rotatePoint(float center_x, float center_y, float point_x, float point_y, float angle_rad, bool clockwise=true);

short sign(float val);
short sign(int val);

Vec2f orthoVecOf(Vec2f vector_);
Vec2i orthoVecOf(Vec2i vector_);
Vec2i orthoVecOf_i(Vec2f vector_); //Avec conversion en int.
Vec2f orthoVecOf_f(Vec2i vector_); //Avec conversion en float.

Vec2f vec(float angle); // -> {cos(angle), sin(angle)}
float vecLen(Vec2f vec);
float vecLen(Vec2i vec);
Vec2f normalize(Vec2f vec);
Vec2f normalize(Vec2i vec);
#define normalizedVecOf normalize
Vec2f resizeVec(Vec2f vec, float new_length);
// dotProd() -> en template plus haut
// vecAngleRad() -> en template plus haut
bool vecsPointingToSameSide(Vec2f vec1, Vec2f vec2);

bool lineAndCircleIntersect(float line_a, float line_b, float line_c,
	Vec2f cir_center, float cir_radius, Vec2f* out__pt1=nullptr, Vec2f* out__pt2=nullptr);
bool lineAndCircleIntersect_i(float line_a, float line_b, float line_c,
	Vec2f cir_center, float cir_radius, Vec2i* out__pt1=nullptr, Vec2i* out__pt2=nullptr);

Vec2f orthoProj(Vec2f point, float line_a, float line_b, float line_c);
Vec2i orthoProj_i(Vec2i point, float line_a, float line_b, float line_c);

bool isInField(Vec2f field_origin, float fov, float dir_rad, float field_radius,
				Vec2f circ_origin, float circ_radius,
				Vec2f* out__closest_pt=nullptr, float* out__origin_proximity=nullptr);

bool isCirclesCollision(Vec2f center1, float radius1, Vec2f center2, float radius2);
bool isCirclesCollision(Circle_Hitbox circ, Vec2f center2, float radius2);
bool isCirclesCollision(Circle_Hitbox circ1, Circle_Hitbox circ2);

float decimalPart(float val);
double decimalPart(double val);

float changeRange(float val, float origin_min, float origin_max, float target_min, float target_max);
float random(float min=0.f, float max=1.f); //uniforme
int randint(int min, int max); //Bk-Dp //distribution uniforme

FRect frect(Rect rect);

Rect rect(FRect rect);

Rect rect(int x, int y, int w, int h);

FColor normalizeColor(FColor color_0_255);

FColor normalizeColor(Color color_0_255);

Color normalizedColorTo255(FColor color_0_1);

FColor normalizedColorTo255_f(FColor color_0_1);

Color lerpColors(Color begin_col, Color end_col, float t);

Color color(int r, int g, int b, int a=255);

Color color(float r, float g, float b, float a=1.f);

Color color(uint32_t ucolor); //RGBA

Color color(FColor col);

FColor fcolor(Color col);

FRect frect(int x, int y, int w, int h);

Rect rect(Vec2i xy, Vec2i wh);

FRect frect(Vec2f xy, Vec2f wh);

void splitRect(Rect r, Vec2i& out_xy, Vec2i& out_wh);

void splitFRect(FRect r, Vec2f& out_xy, Vec2f& out_wh);

FRect getFRectFromPercentParam(FRect reference, FRect percent_param);

FRect scaleFRect(FRect rect, float scale);

FRect scaleFRect(FRect rect, Vec2f scale);

Vec2f getFRectCenter(FRect rect);

//FColor fcolor(int r, int g, int b, int a=255);

FColor fcolor(float r, float g, float b, float a=255.f);

FColor fcolor(uint32_t ucolor); //RGBA

uint32_t u32RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a=255);

uint32_t u32RGBA(Color col);

uint32_t u32ABGR(uint32_t rgba);

uint32_t u32RGBA(uint32_t abgr);

uint32_t u32RGBA(FColor col);

uint8_t alphaByPercentage(float percent); //->a (rgba)

std::string strToupper(std::string str);

std::vector<std::string> strToupper(std::vector<std::string> str_vec);

bool drawRect(Renderer* ren, const FRect* rect, const Color* col=nullptr, bool fill=false, Blend_Mode* b_mode=nullptr); //Bk-Dp
bool drawRect(Renderer* ren, const FRect& rect, const Color* col=nullptr, bool fill=false, Blend_Mode* b_mode=nullptr);

bool drawRect(Renderer* ren, const FRect* rect, const Color& col, bool fill=false, Blend_Mode* b_mode=nullptr);
bool drawRect(Renderer* ren, const FRect& rect, const Color& col, bool fill=false, Blend_Mode* b_mode=nullptr);

bool drawLine(Renderer* ren, Vec2f pt1, Vec2f pt2, const Color& col, Blend_Mode* b_mode=nullptr); //Bk-Dp
bool drawLines(Renderer* ren, const std::vector<FPoint>& points, const Color& col, Blend_Mode* b_mode=nullptr);
bool drawLines(Renderer* ren, const std::vector<Vec2f>& points, const Color& col, Blend_Mode* b_mode=nullptr);

void processDeltaTime(); //Bk-Dp
float dt();
Renderer* ren();
Window* win();

bool drawPoint(Renderer* ren, int x, int y, const Color* col=nullptr, Blend_Mode* b_mode=nullptr); //Bk-Dp
bool drawPoint(Renderer* ren, int x, int y, const Color& col, Blend_Mode* b_mode=nullptr);

bool drawPoint(Renderer* ren, Vec2i pt, const Color* col=nullptr, Blend_Mode* b_mode=nullptr);
bool drawPoint(Renderer* ren, Vec2i pt, const Color& col, Blend_Mode* b_mode=nullptr);


float perlinNoise2D(float x, float y, unsigned int seed=0);

std::vector<Vec2i> bresenham(int x0, int y0, int x1, int y1); //I.A - Algorithme de Bresenham pour tracer une ligne entre deux points.
std::vector<Vec2i> bresenham4(int x0, int y0, int x1, int y1); //I.A - Algorithme de Bresenham pour tracer une ligne entre deux points - sans transition en diagonal.
std::vector<Vec2i> bresenham(Vec2i p0, Vec2i p1);
std::vector<Vec2i> bresenham4(Vec2i p0, Vec2i p1);

std::vector<gs::Vec2i> AStar_onTileMaze(Vec2i in, Vec2i out, std::function<bool(gs::Vec2i tile_pos)> isTileValidAndNotWall=nullptr);
void loadTileMaze(std::vector<int>& maze_, Vec2i maze_size);
float avoidObstacle_changeAngle(float entity_move_angle, Circle_Hitbox entity, Circle_Hitbox obstacle, float p=2.f);

unsigned int getBytesCountOfUTF8Charac_ByTheCharacHeaderByte(char ch, bool& is_ok);
unsigned int getBytesCountOfUTF8Charac_ByTheCharacHeaderByte(char ch);
unsigned int getUTF8CharsCount(const std::string& str, bool& is_ok);
unsigned int getUTF8CharsCount(const std::string& str);

bool isFlagPresentIn(int combined_flags, int specifiq_flag);













//=======================================================================================//

struct Camera_Shaking
{
	float timer=0.f; //à rebours
	gs::Vec2f move;
	gs::Vec2f a; //amplitude
	gs::Vec2f f; //modificateur de fréquence
	void update(float dt);
};

class Camera2D_Base
{
	public:
		void setWorldPos(gs::Vec2f new_pos);
		void incrWorldPos(gs::Vec2f val) {setWorldPos(m_world_pos+val);}
		gs::Vec2f getWorldPos() const {return m_world_pos;}

		void setWorldCenterPos(gs::Vec2f world_pos, gs::Vec2f ren_size);//Agit sur les position et offset mondes, sans tenir compte de ceux écrans.
		void setScreenCenterPos(gs::Vec2f screen_pos, gs::Vec2f ren_size); //Agit sur les position et offset écrans, sans tenir compte de ceux mondes.

		void setZoom(float new_zoom){m_zoom = GetMax(new_zoom, m_min_zoom); }
		void incrZoom(float val) {m_zoom = GetMax(m_zoom+val, m_min_zoom);}
		float getZoom() const {return m_zoom;}

		void setWorldOffset(gs::Vec2f offset){m_world_offset = offset;}
		void incrWorldOffset(gs::Vec2f val){m_world_offset += val;}
		gs::Vec2f getWorldOffset() const {return m_world_offset;}

		void setScreenOffset(gs::Vec2f offset){m_screen_offset = offset;}
		void incrScreenOffset(gs::Vec2f val){m_screen_offset += val;}
		gs::Vec2f getScreenOffset() const {return m_screen_offset;}
		gs::Vec2f getScreenValModifier() const {return m_screen_val_modifier;}

		gs::Vec2f getFormattedPos(gs::Vec2f pos) const;
		gs::Vec2f getFormattedSize(gs::Vec2f size) const;
		gs::FRect getFormattedRect(gs::FRect rect) const;

		void addScreenShaking(float duration);
		virtual void update(float dt);
		gs::Vec2f getCurrScreenShaking() const;

		void setFilterColor(gs::Color c) {filter = c;}
		gs::Color filter{0,0,0,0}; //Une couleur (attendue généralement transparente) à afficher par dessus le rendu.
		bool allow_out_left=false; //Permettre à la caméra de "sortir" de la map à gauche
		bool allow_out_right=false; //Permettre à la caméra de "sortir" de la map à droite (en 'false' : sous réserve de possibilité)
		bool allow_out_up=false; //Permettre à la caméra de "sortir" de la map en haut
		bool allow_out_down=false; //Permettre à la caméra de "sortir" de la map en bas (en 'false' : sous réserve de possibilité)

	protected:
		//Les valeurs par défaut sont importantes.
		gs::Vec2f m_world_pos{0.f,0.f}; //Coin sup gauche de la vue de la caméra (dans la map/world).
		gs::Vec2f m_world_offset{0.f,0.f}; //S'additionne positivement avec 'm_world_pos'.
		gs::Vec2f m_screen_offset{0.f,0.f}; //Uniquement à l'écran (donc non affecté par le zoom).
		std::vector<Camera_Shaking> m_screen_shakings;

	private:
		float m_zoom=1.f;
		gs::Vec2f m_min_world_pos{0.f,0.f}; //Limite minimale
		constexpr static float m_min_zoom=0.12f;
		gs::Vec2f m_screen_val_modifier={-1.f, 1.f};

};





//____________________________________________________________________________________________
//___________________RANDOM___________________________________________________________________


template<typename Distr_Type=std::uniform_real_distribution<float>,
		typename Engine_Type=std::default_random_engine>
class Random
{
	public :

		Random() { std::random_device rd; m_re.seed(rd()); };
		Random(uint32_t seed) { m_re.seed(seed); };
		Random(uint32_t seed, const Distr_Type::param_type& p)
		{
			m_re.seed(seed);
			m_distr.param(p);
		};
		Random(const Distr_Type::param_type& p)
		{
			std::random_device rd; m_re.seed(rd());
			m_distr.param(p);
		}

		virtual ~Random()=default;

		Distr_Type::result_type get() //Résultat avec les paramêtres données.
		{
			return m_distr(m_re);
		}


		//Ces deux méthodes sont particulièrement distinées pour les models de distributions normal et uniform.
		void initForFormattedGet(float mean_or_min, float stddev_or_max, const std::string& str_distr_type)
		{
			//Ces paramêtres donnés sont supposés être les mêmes que les vrais, afin d'avoir le comportement souhaité.
			m_param_a = mean_or_min;
			m_param_b = stddev_or_max;
			m_distr_type = str_distr_type; //Soit "n" ou "normal", soit "u" ou "uniform".
		}

		Distr_Type::result_type get(float mean_or_min, float stddev_or_max) //Résultat formaté par les paramêtres reçus ici.
		{
			//Cette méthode permet de "générer avec des paramètres différents" sans avoir à avoir plusieurs instances de la classe, ou de changer les paramètres bruts de l'instance.

			if(m_distr_type==std::string("u") or m_distr_type==std::string("uniform"))
			{
				float& min = mean_or_min;
				float& max = stddev_or_max;

				float&& val = get(); //Prise de la valeur psuedo aléatoire avec les paramêtres déjà présents.
				float&& percent = (val-m_param_a) / (m_param_b-m_param_a); //Pourcentage de son éloignement au minimum en fonction du maximum.

				float formatted_val = min + (max-min)*percent; //Application avec les paramêtres ici demandés.
				return static_cast<Distr_Type::result_type>(formatted_val);
			}
			else
			if(m_distr_type==std::string("n") or m_distr_type==std::string("normal"))
			{
				float& mean = mean_or_min;
				float& stddev = stddev_or_max;

				float&& val = get(); //Prise de la valeur psuedo aléatoire avec les paramêtres déjà présents.
				float&& percent = (val-m_param_a) / (m_param_b); //Pourcentage de son éloignement au minimum en fonction du maximum.

				float formatted_val = mean + stddev*percent; //Application avec les paramêtres ici demandés.
				return static_cast<Distr_Type::result_type>(formatted_val);
			}
			else
				gs::lastError("||Warning|| in 'Random::get(float, float)', use 'Random::initForFormattedGet()' with right parameters and specify \"u\" or \"uniform\" _OR_ \"n\" or \"normal\". E.g : one_rand_engine.initForFormattedGet(one_rand_engine.getParam().mean(), one_rand_engine.getParam().stddev(), \"n\");.");

			return static_cast<Distr_Type::result_type>(-1);
		}


		void setParam(const Distr_Type::param_type& p)
		{
			m_distr.param(p);
		}


		Distr_Type::param_type getParam()
		{
			return m_distr.param();
		}



		void setSeed(uint32_t seed)
		{
			m_re.seed(seed);
		}


	protected :

		Distr_Type 		m_distr;
		Engine_Type 	m_re;

		//Ci dessous : prevu pour les usages des distributions normal et uniform.
		std::string 	m_distr_type="not_init";
		float 			m_param_a;
		float 			m_param_b;

};




//____________________________________________________________________________________________
//___________________JEU DE PARTICULES________________________________________________________


enum Particle_T{RECT_PARTICLE, ELLIPSE_PARTICLE, CIRCLE_PARTICLE, TEXTURED_PARTICLE};
enum Rand_Distr_T{NORMAL_DISTR, UNIF_DISTR};
enum Particle_Coloring_T{RANDOM_CHOICE, TARGET_COLORS};
enum Particle_Attrib_Field_T{POS_X=0, POS_Y=1, SIZE_X=2, SIZE_Y=3, DIR=4, SPEED=5, TIME=6, COLOR=7, TEXTURE=7, SRC_RECT=7, ROT=7, ROT_SPEED=7};

template<typename User_Data_T=char>
struct Particle
{
	Particle_T type=ELLIPSE_PARTICLE;

	Vec2f pos={10.f,10.f};
	Vec2f size={8.f,8.f};
	float dir=0; 					//Direction : [0; 2pi].
	float speed=20; 				//Pixels par seconde.
	float left_time=10.f; 			//"Durée de vie" restante de la particule.
	float begin_left_time=10.f; 	//"Durée de vie" de la particule.

	Color color={0,200,201,255};
	User_Data_T udata;

	// void (*updater)(Particle& self, float dt)=nullptr;

	virtual void update(float dt);
	bool show(Renderer* ren, const bool& fill=true,  Blend_Mode* b_mode=nullptr);
	void setLeftTime(float t);

};



template<typename User_Data_T=char>
struct Textured_Particle : public Particle<User_Data_T>
{
	Particle_T type=TEXTURED_PARTICLE;

	Texture* texture=nullptr;
	FRect src_rect={0.f,0.f,10000.f,10000.f};
	float rotation=0; //Rotation de la texture [0, 2pi].
	float rotation_speed=0;

	User_Data_T udata;


	bool show(Renderer* ren);
	void update(float dt);
	void setLeftTime(float t);

};





struct Particle_Param
{
	struct Field {
		float a=0, b=0; //a = min ou mean; b = max ou stddev.
		Rand_Distr_T distr_type=NORMAL_DISTR;
	}
		x_pos={400, 10}, //Par exemple pour une distribution normale ça représente {mean, stddev}, et pour uniforme {min, max};
		y_pos={250, 10},
		x_size={7, 2},
		y_size={7, 2},
		dir={0.0, 2*_pi_},
		speed={20, 8},
		time={15, 5};

	void allTo(Rand_Distr_T type)
	{
		x_pos.distr_type = type;
		y_pos.distr_type = type;
		x_size.distr_type = type;
		y_size.distr_type = type;
		dir.distr_type = type;
		speed.distr_type = type;
		time.distr_type = type;
	}
};

struct Particle_Color_Param
{
	struct Field {
		uint8_t a=0, b=0;
		Rand_Distr_T distr_type=UNIF_DISTR;
	}
		r_range={0, 255},
		g_range={0, 255},
		b_range={0, 255},
		a_range={255, 255};

	Particle_Coloring_T col_type=RANDOM_CHOICE;
	std::vector<Color> possible_colors;
	Rand_Distr_T possible_colors_distr_type;

	void allTo(Rand_Distr_T type)
	{
		r_range.distr_type = type;
		g_range.distr_type = type;
		b_range.distr_type = type;
		a_range.distr_type = type;
	}

	void allTo(uint8_t a, uint8_t b)
	{
		r_range.a=a; r_range.b=b;
		g_range.a=a; g_range.b=b;
		b_range.a=a; b_range.b=b;
		a_range.a=a; a_range.b=b;
	}

};

template<typename P_User_Data_T=char>
void defaultParticleColorUpdater(const Particle<P_User_Data_T>& particle, Color& particle_color);


struct Particle_Texture_Param
{
	std::vector<Texture*> possible_textures;
	std::vector<Rect> possible_src_rects;

	Rand_Distr_T textures_distr_type=NORMAL_DISTR;
	Rand_Distr_T src_rects_distr_type=NORMAL_DISTR;

	struct Field {
		float a=0, b=0;
		Rand_Distr_T distr_type=NORMAL_DISTR;
	}
		rotation={0,2*_pi_},
		rotation_speed={0,0.12};
};


template<typename P_User_Data_T=char>
class Particle_Box
{
	public:
		virtual ~Particle_Box()=default;
		Particle_Box();

		void setSeed(Particle_Attrib_Field_T field, Rand_Distr_T distr_type, unsigned int seed); //Permet de déterminer les graines des différents générateurs de nombres pseudo-aléatoires. Par exemple pour le générateur uniforme de la taille en x, ou le générateur normal des couleurs, etc.
		unsigned int getSeed(Particle_Attrib_Field_T field, Rand_Distr_T distr_type) const;

		Particle_Param setParam(const Particle_Param& new_param);
		Particle_Param getParam() const;
		Particle_Param& getParam_ref();

		Particle_Color_Param setColorParam(const Particle_Color_Param& new_col_param);
		Particle_Color_Param getColorParam() const;
		Particle_Color_Param& getColorParam_ref();



		void addParticle(int count, Particle_Param* target_param=nullptr, Particle_T type=ELLIPSE_PARTICLE, const Particle_Color_Param* color_param=nullptr);
		void addParticle(const Particle<P_User_Data_T>& particle, int count=1);
		void addTexturedParticle(int count, Particle_Param* target_param, const Particle_Texture_Param& texture_param);
		void addTexturedParticle(const Textured_Particle<P_User_Data_T>& particle, int count=1);

		void update(float dt);
		void show(Renderer* ren);

		void setParticleColorUpdater(void (*particleColorUpdater)(const Particle<P_User_Data_T>& particle, Color& particle_color));
		void setParticleBehaviorUpdater(void (*particleBehaviorUpdater)(Particle<P_User_Data_T>& particle, float dt));
		void setTParticleBehaviorUpdater(void (*tParticleBehaviorUpdater)(Textured_Particle<P_User_Data_T>& particle, float dt));

		uint32_t getCount() const; //La totale.
		uint32_t getCount(Particle_T type) const; //Au choix.

	protected:

		void adjustParam(const Particle_Param& param);
		float getRandVal(int field_index, const Particle_Param& param);
		Color getColor(const Particle_Color_Param& cp);
		Texture* getTexture(const Particle_Texture_Param& tp);
		Rect getSrcRect(const Particle_Texture_Param& tp);
		Vec2f getRotAndRotSpeed(const Particle_Texture_Param& tp);


		std::vector<Random<std::normal_distribution<float>>> 		m_norm_distrs;
		std::vector<Random<std::uniform_real_distribution<float>>> 	m_unif_distrs;

		std::vector<Particle<P_User_Data_T>> 						m_particles;
		std::vector<Textured_Particle<P_User_Data_T>>  				m_t_particles;
		Particle_Param 												m_param;
		Particle_Color_Param 										m_col_param;

		std::vector<gs::Vec2ui> 									m_seeds; //L'odre (arbitraire) est : 'm_seeds[i].x' = graine du générateur 'normal', 'm_seeds[i].y' = graine du générateur 'uniform'.

		void (*m_particleColorUpdater)(const Particle<P_User_Data_T>& particle, Color& particle_color)=nullptr;
		void (*m_particleBehaviorUpdater)(Particle<P_User_Data_T>& particle, float dt)=nullptr;
		void (*m_t_particleBehaviorUpdater)(Textured_Particle<P_User_Data_T>& particle, float dt)=nullptr;


};











extern bool game_started;
extern bool game_won;
extern bool end_game;
extern bool show_comments;


};/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/

extern gs::Game *ThisGame;







std::ostream& operator<<(std::ostream& os, gs::Rect& rect);
std::ostream& operator<<(std::ostream& os, gs::FRect& rect);
std::ostream& operator<<(std::ostream& os, gs::Point& point);
std::ostream& operator<<(std::ostream& os, gs::FPoint& point);
std::ostream& operator<<(std::ostream& os, gs::Color& col);

bool operator==(const gs::Rect& rect1, const gs::Rect& rect2);
bool operator==(const gs::FRect& rect1, const gs::FRect& rect2);
bool operator==(const gs::Point& pt1, const gs::Point& pt2);
bool operator==(const gs::FPoint& pt1, const gs::FPoint& pt2);
bool operator==(const gs::Color& col1, const gs::Color& col2);



#include "GSlib-template.hpp"


#endif // _GSlib_HPP_
