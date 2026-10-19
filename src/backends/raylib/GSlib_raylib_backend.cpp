#include "backends/raylib/GSlib_raylib_backend.hpp"
#include <raylib/rlgl.h>
#include "GSlib-Geo.hpp"
#include "GSlib-Text.hpp"
#include "GSlib-Widget.hpp"
#include <list>
#include <random>
#include <filesystem>
#include <set>

namespace gs
{

void INTERNAL_USE_setTextureScaleMode(Texture& t, Scale_Mode scale_mode)
{
	TextureFilter filter = TEXTURE_FILTER_POINT;
	switch(scale_mode)
	{ 
		case SCALEMODE_INVALID: return;
		case SCALEMODE_NEAREST: filter = TEXTURE_FILTER_POINT; break;
		case SCALEMODE_LINEAR: filter = TEXTURE_FILTER_BILINEAR; break;
		case SCALEMODE_PIXELART: filter = TEXTURE_FILTER_POINT; break;
		default:return;
	}
	
	SetTextureFilter(t, filter);
}


struct INTERNAL_Textures_Manager_
{
	std::unordered_map<unsigned int, std::unique_ptr<Texture>> loaded_textures; // <id, texture>
	std::unordered_map<unsigned int, std::unique_ptr<Texture_Target>> loaded_texture_targets; // <id, texture>
	
	static bool destroyed;

	~INTERNAL_Textures_Manager_()
	{
		for(auto& [t_id, t] : loaded_textures)
			if(t)
				UnloadTexture(*t);
		
		for(auto& [t_id, t_trgt] : loaded_texture_targets)
			if(t_trgt)
				UnloadRenderTexture(*t_trgt);

		loaded_textures.clear();
		loaded_texture_targets.clear();
		destroyed = true;
	}

	Texture* loadTexture(const std::string& path)
	{
		Texture t = LoadTexture(path.c_str());

		if(not IsTextureValid(t))
		{
			gs::lastError("||Warning|| in '[...]::loadTexture()', load texture error. '", path+"' ");
			return nullptr;
		}

		loaded_textures[t.id] = std::make_unique<Texture>(t);
		return loaded_textures[t.id].get();
	}

	void destroyTexture(Texture* tex)
	{
		if(not tex) return;
		Texture& t = *tex;

		UnloadTexture(t); //Important : l'ordre entre ces deux lignes
		loaded_textures.erase(t.id);
	}

	void destroyTexture(Texture_Target* trgt)
	{
		if(not trgt) return;
		Texture_Target& t = *trgt;

		UnloadRenderTexture(t); //Important : l'ordre entre ces deux lignes
		loaded_texture_targets.erase(t.id);
	}

	Texture* addTexture(Texture* tex)
	{
		if(not tex) return nullptr;
		
		Texture& t = *tex;

		loaded_textures[t.id] = std::make_unique<Texture>(t);
		return loaded_textures[t.id].get();
	}

	Texture_Target* createTextureTarget(Vec2i size)
	{
		if(size.x <= 0 or size.y <= 0)
		{
			gs::lastError("||Warning|| in '[...]::createTextureTarget()', invalid size. [given arg 'size' : (",std::to_string(size.x)+"; "+std::to_string(size.y)+")]");
			return nullptr;
		}

		Texture_Target tex_target = LoadRenderTexture(size.x, size.y);

		if(not IsRenderTextureValid(tex_target))
		{
			gs::lastError("||Warning|| in '[...]::createTextureTarget()', failed.");
			return nullptr;
		}

		loaded_texture_targets[tex_target.id] = std::make_unique<Texture_Target>(tex_target);
		return loaded_texture_targets[tex_target.id].get();
	}
};

bool INTERNAL_Textures_Manager_::destroyed=false;
INTERNAL_Textures_Manager_ _textures_manager_;
Color clear_color{0,0,0,255};
Texture_Target* _BK_render_target_=nullptr; //Important : l'initialiser à 'nullptr'

//====================================================================//



Vec2f getMousePos(float* x, float* y)
{
	Vec2f pos{float(GetMouseX()), float(GetMouseY())};
	if(x) *x=pos.x;
	if(y) *y=pos.y;
	return pos;
}


void drawBegin(Renderer* ren, bool clear)
{
	BeginDrawing();
	if(clear) ClearBackground(clear_color);
}

void drawEnd(Renderer* ren) {EndDrawing();}



Texture* loadTexture(Renderer* ren, const std::string& path, bool nearest) {return _textures_manager_.loadTexture(path); std::cout << "||Warning|| in 'loadTexture()', 'nearest' arg not yet took into account.\n";}

Surface* loadSurface(const std::string& img_path)
{
	std::cout << "loadSurface(), not yet implemented for raylib backend.\n";
	return nullptr;
}

Texture_Target* createTextureTarget(Renderer* ren, Vec2i size)
{
	return _textures_manager_.createTextureTarget(size);
}


bool drawTile(Renderer* ren, Texture* target, Texture* tileset,
					FRect src, FRect dest, bool takeInToAccountTheTarget)
{
    if(not tileset)
    {
        gs::lastError("!! drawTile(), invalid tileset.");
        return false;
    }

    DrawTexturePro(*tileset, rectangle(src), rectangle(dest), {0.f,0.f}, 0.f, WHITE);
    return true;
}


void setClearColor(Renderer* ren, Color col) {clear_color = col;}
Color getClearColor(Renderer* ren) {return clear_color;}


bool drawRect(Renderer* ren, const FRect* rect, const Color* col, bool fill, Blend_Mode* b_mode)
{
	//Warning : 'b_mode' non exploitée
	gs::lastError("||Warning|| in 'drawRect()', untapped param : 'b_mode'");


	const Color& color = (col ? *col : clear_color);
	FRect r{0.f,0.f,0.f,0.f};
	if(rect)
		r = *rect;
	else
	{
		r.w = GetRenderWidth();
		r.h = GetRenderHeight();
	}
	
	if(fill)
		DrawRectangle(r.x, r.y, r.w, r.h, color);
	else
		DrawRectangleLines(r.x, r.y, r.w, r.h, color);

    return true;
}

bool drawLine(Renderer* ren, Vec2f pt1, Vec2f pt2, const Color& col, Blend_Mode* b_mode)
{
	//Warning : 'b_mode' non exploitée
	gs::lastError("||Warning|| in 'drawLine()', untapped param : 'b_mode'");

	DrawLine(pt1.x, pt1.y, pt2.x, pt2.y, col);
	return true;
}

bool drawPoint(Renderer* ren, int x, int y, const Color* col, Blend_Mode* b_mode)
{
	//Warning : 'b_mode' non exploitée
	gs::lastError("||Warning|| in 'drawPoint()', untapped param : 'b_mode'");

	const Color& color = (col ? *col : clear_color);
	DrawPixel(x, y, color);
	return true;
}

int randint(int min, int max) {return GetRandomValue(min, max);} //distribution uniforme, avec min et max incluses.


void processDeltaTime()
{
	//ça ne fais rien : on laisse raylib s'en occuper
}

float dt() { return GetFrameTime();};


//_______________________________________




void addToClearAtTheEnd_backend(Texture* tex, void* texturesToClearAtTheEnd_ptr)
{
	_textures_manager_.addTexture(tex);//, false);
}


void destroyTexture(Texture* t)
{
	if(not INTERNAL_Textures_Manager_::destroyed)
		return _textures_manager_.destroyTexture(t);

	if(t)
		UnloadTexture(*t);
}

void destroyTexture(Texture_Target* trgt)
{
	if(not INTERNAL_Textures_Manager_::destroyed)
		return _textures_manager_.destroyTexture(trgt);

	if(trgt)
		UnloadRenderTexture(*trgt);
}

void destroySurface(Surface* s)
{
	if(s) UnloadImage(*s);
}

Vec2<float> getTextureSize(Texture* t, float* w, float* h)
{
	Vec2f size{0.f, 0.f};
	if(t)
	{
		size.x = t->width;
		size.y = t->height;
	}

	if(w) *w = size.x;
	if(h) *h = size.y;

	return size;
}

Vec2<float> getSurfaceSize(Surface* s, float* w, float* h)
{
	Vec2f size{0.f, 0.f};
	if(s)
	{
		size.x = s->width;
		size.y = s->height;
	}
	
	if(w) *w = size.x;
	if(h) *h = size.y;

	return size;
}

bool INTERNAL_USE_drawTexture(Renderer* ren, Texture* t, FRect* src, FRect* dest, bool flip_Y)
{
	if(not t) return false;

	auto ren_size = getCurrentRenderOutputSize();

	Rectangle dest_r = Rectangle(0.f, 0.f, ren_size.x, ren_size.y);
	if(dest) dest_r = gs::rectangle(*dest);

	Rectangle src_r(0.f, 0.f, t->width, t->height);
	if(src) src_r = gs::rectangle(*src);

	if(flip_Y) src_r.height*=-1.f;

	DrawTexturePro(*t, src_r, dest_r, {0.f,0.f}, 0.f, WHITE);
    return true;
}

bool drawTexture(Renderer* ren, Texture* t, FRect* src, FRect* dest)
{
	return INTERNAL_USE_drawTexture(ren, t, src, dest, false);
}
bool drawTexture(Renderer* ren, Texture_Target* trgt, FRect* src, FRect* dest)
{
	if(not trgt) return false;
	return INTERNAL_USE_drawTexture(ren, &trgt->texture, src, dest, true);
}


bool INTERNAL_USE_drawTextureRotated(Renderer* ren, Texture* t, const FRect* src_rect, const FRect* dest_rect, float angle_degree, const FPoint* rotation_center, Flip_Mode flip, bool flip_Y)
{
	if(not t) return false;

	auto ren_size = getCurrentRenderOutputSize();

	Rectangle dest_r = Rectangle(0.f, 0.f, ren_size.x, ren_size.y);
	if(dest_rect) dest_r = gs::rectangle(*dest_rect);

	Rectangle src_r(0.f, 0.f, t->width, t->height);
	if(src_rect) src_r = gs::rectangle(*src_rect);

	if(flip_Y) src_r.height*=-1.f; // -> INTERNAL_USE_

	switch(flip) // -> utilisateur
	{
		case Flip_Mode::FLIP_HORIZONTAL : src_r.width*=-1.f; break;
		case Flip_Mode::FLIP_VERTICAL : src_r.height*=-1.f; break;
		case Flip_Mode::FLIP_HORIZONTAL_AND_VERTICAL : src_r.width*=-1.f; src_r.height*=-1.f; break;
		default:break;
	}

	Vector2 rot_center{dest_r.width*0.5f, dest_r.height*0.5f};
	if(rotation_center)
	{
		rot_center.x = rotation_center->x;
		rot_center.y = rotation_center->y;
	}

	dest_r.x+=rot_center.x; //pour avoir le comportement attendu, et non celui que produit raylib
	dest_r.y+=rot_center.y;

	DrawTexturePro(*t, src_r, dest_r, rot_center, angle_degree, WHITE);
    return true;
}

bool drawTextureRotated(Renderer* ren, Texture* t, const FRect* src_rect, const FRect* dest_rect, float angle_degree, const FPoint* rotation_center, Flip_Mode flip)
{
	return INTERNAL_USE_drawTextureRotated(ren, t, src_rect, dest_rect, angle_degree, rotation_center, flip, false);
}

bool drawTextureRotated(Renderer* ren, Texture_Target* trgt, const FRect* src_rect, const FRect* dest_rect, float angle_degree, const FPoint* rotation_center, Flip_Mode flip)
{
	if(not trgt) return false;
	return INTERNAL_USE_drawTextureRotated(ren, &trgt->texture, src_rect, dest_rect, angle_degree, rotation_center, flip, true);
}


Vec2<int> getCurrentRenderOutputSize(Renderer* ren, int* w, int* h)
{
	Vec2i size;
	if(_BK_render_target_) //S'il y a un render target enregistré sur le moment
	{
		size.x = _BK_render_target_->texture.width;
		size.y = _BK_render_target_->texture.height;
	}
	else
	{
		size.x = GetRenderWidth();
		size.y = GetRenderHeight();
	}

	if(w) *w = size.x;
	if(h) *h = size.y;

	return size;
}

Vec2<int> getRenderOutputSize(Renderer* ren, int* w, int* h)
{
	Vec2i size;
	size.x = GetRenderWidth();
	size.y = GetRenderHeight();

	if(w) *w = size.x;
	if(h) *h = size.y;

	return size;
}

Texture_Target* getRenderTarget(Renderer* ren) {return _BK_render_target_;}

bool setRenderTarget(Renderer* ren, Texture_Target* texture_target)
{
	if(_BK_render_target_)
		EndTextureMode();

	_BK_render_target_ = texture_target;

	if(texture_target)
		BeginTextureMode(*texture_target);

	return true;
}




Surface* textureToSurface(Renderer* ren, Texture* tex)
{
	std::cout<<"textureToSurface(), not yet implemented for raylib backend.\n";
	return nullptr;
}

bool drawLines_INTERNAL(Renderer* ren, const FPoint* points, int pts_count, const Color& col, Blend_Mode* b_mode)
{
	std::cout<<"textureToSurface(), not yet implemented for raylib backend.\n";
	return false;
}

Vector2 v2(Vec2<float> v) {return Vector2(v.x, v.y);}










Renderer *Backend__Game::ren() const
{
	static Renderer COMP_ren;
	if(isInit) return &COMP_ren; //Pour la compatibilité
	return nullptr;
};

Window *Backend__Game::win() const
{
	static Window COMP_win;
	if(isInit) return &COMP_win; //Pour la compatibilité
	return nullptr;
};


bool Game::InitTheWindow(int window_w, int window_h, const char* window_title, uint32_t COMP_win_flags, const char* COMP_renderer_name, bool COMP_try_enable_VSYNC)
{
	InitWindow(window_w, window_h, window_title);
	isInit = IsWindowReady();
	return isInit;
}


int Game::getWinW() {return GetScreenWidth();}

int Game::getWinH() {return GetScreenHeight();}

int Game::getRenW() {return GetRenderWidth();}

int Game::getRenH() {return GetRenderHeight();}



void Game::fpsProcessBegin()
{
	//ne fais rien : laisse raylib s'en occuper
}
void Game::fpsProcessEnd(uint32_t fps) 
{
	static uint32_t _FPS_set_ = -1;
	if(_FPS_set_ != fps)
	{
		SetTargetFPS(fps);
		_FPS_set_ = fps;
	}
}
void Game::processDeltaTime()
{
	//ne fais rien : on laisse raylib s'en occuper
}

float Game::dt() const { return GetFrameTime();};













//=====================================================================================//
//====================================- Geo -==========================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//====================================- Geo -==========================================//
//=====================================================================================//



bool Triangle::draw(Renderer* ren, Color col, bool fill, const Blend_Mode* b_mode)
{
	//Warning : 'b_mode' non exploitée
	gs::lastError("||Warning|| in 'Triangle::draw()', untapped param : 'b_mode'");

	bool success = true;
	
	Vector2 pts[3]; //raylib attend les points rangés clockwise
	{
		pts[0]=Vector2(a.x, a.y);

		float d = (b.x - a.x)*(c.y - a.y) - (b.y - a.y)*(c.x - a.x);
		if(d < 0)
		{
			pts[1]=Vector2(b.x, b.y);
			pts[2]=Vector2(c.x, c.y);
		}
		else
		{
			pts[1]=Vector2(c.x, c.y);
			pts[2]=Vector2(b.x, b.y);
		}
	}

	if(fill)
		DrawTriangle(pts[0], pts[1], pts[2], col);
	else
		DrawTriangleLines(pts[0], pts[1], pts[2], col);

	return success;
}



bool drawRect(Renderer* ren, const FRect* rect, Color col, bool fill, /*short thickness,*/ Blend_Mode* b_mode, Rotation_P rotation)
{
	//Warning : 'b_mode' non exploitée
	gs::lastError("||Warning|| in 'drawRect()', untapped param : 'b_mode'");


	float rot_rad = rotation.rot_rad;
	Vec2f anchor = rotation.anchor;
	bool clockwise = rotation.clockwise;

	FRect rect_cpy={0.f, 0.f, 0.f, 0.f};

	if(rect) //S'il y'a un rectangle de donné.
		rect_cpy = *rect;

	else //Sinon on prend le rectangle de l'affichage.
	{
		rect_cpy.w = GetRenderWidth();
		rect_cpy.h = GetRenderHeight();
	}


	if(fill)
	{
		float rad = (clockwise ? rot_rad : -rot_rad);
		float degree = rad*RAD2DEG;
		Vec2f size(rect_cpy.w, rect_cpy.h);
		Vec2f rot_center = anchor*size;
		rect_cpy.x+=rot_center.x; //pour avoir le comportement attendu, et non celui que produit raylib
		rect_cpy.y+=rot_center.y;
		DrawRectanglePro(rectangle(rect_cpy), v2(rot_center), degree, col);
	}
	else
	{
		//Les quatres points du rectangle
		//
		// nw       ne
		//
		// sw       se
		gs::Vec2f nw, ne, se, sw;
		nw.x = rect_cpy.x;
		nw.y = rect_cpy.y;
		ne.x = nw.x + rect_cpy.w-1;
		ne.y = nw.y;
		se.x = ne.x;
		se.y = ne.y + rect_cpy.h-1;
		sw.x = nw.x;
		sw.y = se.y;


		//Centre de rotation.
		Vec2f rot_center;
		rot_center.x = rect_cpy.x + (rect_cpy.w*anchor.x);
		rot_center.y = rect_cpy.y + (rect_cpy.h*anchor.y);


		//Calculs des coordonnées des différents points par rapport au centre de rotation après rotation.
		if(rot_rad != 0)
		{
			nw = rotatePoint(rot_center.x, rot_center.y, nw.x, nw.y, rot_rad, clockwise).to<float>();
			ne = rotatePoint(rot_center.x, rot_center.y, ne.x, ne.y, rot_rad, clockwise).to<float>();
			se = rotatePoint(rot_center.x, rot_center.y, se.x, se.y, rot_rad, clockwise).to<float>();
			sw = rotatePoint(rot_center.x, rot_center.y, sw.x, sw.y, rot_rad, clockwise).to<float>();
		}


		std::vector<Vector2> points{v2(nw), v2(ne), v2(se), v2(sw), v2(nw)};
		float thickness = 1.f;
		DrawSplineLinear(points.data(), points.size(), thickness, col);      	
	}


	return true;
}


bool drawRoundedRect(Renderer* ren, FRect rect, float percent, Color col, bool fill, Blend_Mode* b_mode)
{
	//Warning : 'b_mode' non exploitée
	gs::lastError("||Warning|| in 'drawRoundedRect()', untapped param : 'b_mode'");


	if(rect.w <= 0.f or rect.h <= 0.f) return true;

	if(percent < 0.f) percent = 0.f;
	if(percent > 0.5f) percent = 0.5f;

	int segments = 0;

	if(fill)
		DrawRectangleRounded(rectangle(rect), percent, segments, col);
	else
		DrawRectangleRoundedLines(rectangle(rect), percent, segments, col);
	
	return true;
}




bool drawEllipse(Renderer* ren, Vec2f center, Vec2f radii, Color col, bool fill, float total_angle, float begin_angle)
{
	//Warning : 'total_angle' et 'begin_angle' non exploitées
	gs::lastError("||Warning|| in 'drawEllipse()', untapped params : 'total_angle', 'begin_angle'");

	if(fill)
		DrawEllipse(center.x, center.y, radii.x, radii.y, col);
	else
		DrawEllipseLines(center.x, center.y, radii.x, radii.y, col);
	
	return true;
}

bool drawEllipseThickness(Renderer* ren, Vec2f center, Vec2f radii, int thickness, Color col, bool fill, float total_angle, float begin_angle)
{
	std::cout << "!! gs::drawEllipseThickness() not yet implemented for raylib backend.\n";
	return false;
}



bool drawCircle(Renderer* ren, Vec2f center, float radius, Color col, bool fill, float total_angle, float begin_angle)
{
	begin_angle*=RAD2DEG;
	total_angle*=RAD2DEG;
	Vector2 c; c.x=center.x; c.y=center.y;
	float endAngle = begin_angle+total_angle;
	int segments = 0;

	if(total_angle>=360.f)
	{
		if(fill)
			DrawCircle(center.x, center.y, radius, col);
		else
			DrawCircleLines(center.x, center.y, radius, col);
		return true;
	}


	if(fill)
		DrawCircleSector(c, radius, begin_angle, endAngle, segments, col); // Draw a piece of a circle
	else
		DrawCircleSectorLines(c, radius, begin_angle, endAngle, segments, col); // Draw circle sector outline

	return true;
}


bool drawCircleThickness(Renderer* ren, Vec2f center, float radius, int thickness, Color col, bool fill, float total_angle, float begin_angle)
{
	begin_angle*=RAD2DEG;
	total_angle*=RAD2DEG;
	Vector2 c; c.x=center.x; c.y=center.y;
	float endAngle = begin_angle+total_angle;
	int segments = 0;

	if(fill)
		DrawRing(c, float(radius-thickness), radius, begin_angle, endAngle, segments, col); // Draw ring
	else
		DrawRingLines(c, float(radius-thickness), radius, begin_angle, endAngle, segments, col); // Draw ring outline

	return true;
}















//=====================================================================================//
//====================================- Text -=========================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//====================================- Text -=========================================//
//=====================================================================================//


Backend__Font::~Backend__Font()
{
	if(IsFontValid(m_ray_font))
		UnloadFont(m_ray_font);
}

int Backend__Font::getFontLineSkip()
{
	return INTERNAL_USE_fromPtsizeToBaseSize()*1.f;//m_ptsize*1.12f;//m_ray_font.baseSize*1.12f;
}


bool Font::init(Renderer* ren, const std::string& font_path, size_t ptsize, te::Font_Rendering_Type default_type)
{
	if(m_is_init)
	{
		gs::lastError("||Warning|| in 'Font::init()', cannot init if it is already init.");
		return false;
	}

	//Comportement choisi pour le backend raylib:
	// A chaque 'render text' on charge le font avec uniquement les caractères demandés (à la taille demandée).
	// Le but de gs::Font n'est pas la rapidité mais la mise en oeuvre d'un service qui répond aux requêtes.

	m_default_type = default_type; //Note: pour le backend raylib il n'y a pas de différence entre SOLID et BLENDED.
	m_ptsize = ptsize;
	m_font_path = font_path;
	

	m_is_init = true;
	//m_font_style = FONT_STYLE_NORMAL; //Non exploité
	return true;
}


bool Font::try_changeFontSource(const std::string& target_font_path)
{
	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Font::try_changeFontSource()', this 'gs::Font' is not yet init");
		return false;
	}

	std::filesystem::directory_entry ent{
		std::filesystem::path(target_font_path)
	};
	
	bool success = ent.exists() and ent.is_regular_file();
	
	if(success)
		m_font_path = target_font_path;

	return success;
}


void Font::setPtsize(size_t ptsize)
{
	if(not m_is_init) return;
	if(ptsize < 1) ptsize = 1;

	m_ptsize = ptsize;
}

float Backend__Font::INTERNAL_USE_fromPtsizeToBaseSize() const
{
	return float(int(m_ptsize*6.f/5.f)); //Bricolage d'un patch entre ptsize un baseSize
}




gs::Vec2i Font::getTextPixelSize(const std::string& str) //const
{
	Vec2i output{0,0};

	if(not m_is_init or str.empty())
		return output;

	//On vérifie la présence de tous les codepoints demandés
	{
		int cpts_count=0; //pour contenir le nombre de codepoints
		int* codepoints = LoadCodepoints(str.c_str(), &cpts_count);

		if(not codepoints or cpts_count<=0)
		{
			if(codepoints)UnloadCodepoints(codepoints);
			gs::lastError("||Warning|| in 'Font::getTextPixelSize()', couldn't check required codepoints ( LoadCodepoints() error ? ). [given arg 'str' : \"", str+"\"]");
			return output;
		}

		bool all_is_present = areAllCodepointsPresent(codepoints, cpts_count);

		if(not all_is_present) //S'il y a au moins un codepoint non présent.
		{
			if(not loadRayFont(codepoints, cpts_count, "'Font::getTextPixelSize()'", true)) //On recharge le font avec les codepoints demandés.
			{
				UnloadCodepoints(codepoints);
				return output;
			}
		}

		UnloadCodepoints(codepoints);
	}

	Vector2 size = MeasureTextEx(m_ray_font, str.c_str(), INTERNAL_USE_fromPtsizeToBaseSize(), 0.f);
	output.x = size.x;
	output.y = size.y;

	if(str == " " and m_wider_space_token_mode)
		output.x = output.x*m_wider_space_factor;

	return output;
}


BK_Font_t* Font::getFont() {return &m_ray_font;}


int Font::getFontStyle() const
{
	gs::lastError("||Warning|| in 'Font::getFontStyle()', custom font style feature is not available in 'gs::Font' with raylib backend.");
	return 0;
}

void Font::setFontStyle(int new_font_style)
{
	gs::lastError("||Warning|| in 'Font::setFontStyle()', custom font style feature is not available in 'gs::Font' with raylib backend.");
}



bool Backend__Font::areAllCodepointsPresent(const int* codepoints, int num_codepoints) const
{
	if(not m_is_init or not IsFontValid(m_ray_font) or not codepoints or num_codepoints <= 0)
		return false;

	for(int i=0; i<num_codepoints; ++i) //On vérifie la présence de chaque codepoint.
	{
		int value = codepoints[i];
		bool present = false;

		for(int j=0; j<m_ray_font.glyphCount; ++j) //Parcours des glyphs
		{
			if(value == m_ray_font.glyphs[j].value)
			{
				present = true;
				break;
			}
		}

		if(not present)
			return false;
	}

	return true;
}


bool Backend__Font::loadRayFont(const int* codepoints, int num_codepoints, const char* up_fun_name, bool keep_prev_codepoints)
{
	//Pour énoncer les messages d'erreur du point de vue de la fonction qui appelle cette fonction-ci.
	std::string err_begin_str = std::string("from ") + std::string(up_fun_name) + std::string(" : ");

	if(not m_is_init)
	{
		//Message d'erreur énoncé du point de vue de la fonction qui appelle cette fonction-ci.
		gs::lastError(err_begin_str, "||Warning|| in 'Backend__Font::loadRayFont()', '*this' is not init.");
		return false;
	}

	if(not codepoints or num_codepoints <= 0)
	{
		//Message d'erreur énoncé du point de vue de la fonction qui appelle cette fonction-ci.
		gs::lastError(err_begin_str,"||Warning|| in 'Backend__Font::loadRayFont()', 'not codepoints or num_codepoints <= 0'.");
		return false;
	}


	std::set<int> cpts; //Récupérations des codepoints dans un 'std::set' afin de pouvoir rajouter, au besoin, les précédents codepoints + le 'std::set' permettra notamment de retirer les doublons.
	for(int i=0; i<num_codepoints; ++i)
		cpts.insert(codepoints[i]);

	if(keep_prev_codepoints and IsFontValid(m_ray_font))
		for(int i=0; i<m_ray_font.glyphCount; ++i)
			cpts.insert(m_ray_font.glyphs[i].value);

	std::vector<int> cpts_vec;
	for(int cp : cpts) cpts_vec.push_back(cp); //On met le tout dans un tableau pour les besoins de 'LoadFontEx()'.


	UnloadFont(m_ray_font); //Important
	m_ray_font = LoadFontEx(m_font_path.c_str(), INTERNAL_USE_fromPtsizeToBaseSize(), cpts_vec.data(), cpts_vec.size());

	if(not IsFontValid(m_ray_font))
	{
		gs::lastError(err_begin_str, std::string("||Error|| in 'Backend__Font::loadRayFont()', LoadFontEx()")
							+ std::string(". [path : \"")+m_font_path+"\"] ");

		return false;
	}

	return true;
}




Surface* Font::hackRenderedWidth(Surface* src, float scale_factor) const //Ce hack permet d'étirer le str généré.
{
	if(not src) return nullptr;
	
	int dest_w = src->width*scale_factor;
	int dest_h = src->height;

	ImageResize(src, dest_w, dest_h);

	return src;
}




Texture* Font::render(const std::string& text, Color text_color,
						te::Font_Rendering_Type rendering_type,
						Color* bg_color,
						Vec2f* text_size_out) //const
{
	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Font::render()', cannot render : '*this' is not yet init. [given arg 'text' : \"", text+"\"]");
		return nullptr;
	}

	if(text.empty())
	{
		gs::lastError("||Warning|| in 'Font::render()', given text empty");
		return nullptr;
	}

	
	te::Font_Rendering_Type type = rendering_type; //Note: pour le backend raylib il n'y a pas de différence entre SOLID et BLENDED.
	if(type == te::SELF_DEFAULT)
		type = m_default_type;

	Color bg_color_to_use{0,0,0,0}; //Pas d'arrière plan par défaut

	if(type == te::SHADED and m_show_background)
	{
		if(bg_color)
			bg_color_to_use = *bg_color;
		else
			bg_color_to_use = m_bg_color;
	}
	

	const float required_base_size = INTERNAL_USE_fromPtsizeToBaseSize(); //Note: 'm_ptsize' peut évoluer indépendamment de 'm_ray_font.baseSize', et est celle qui fixe la taille du rendu.

	//Setup du font pour le texte reçu
	{
		int cpts_count=0; //pour contenir le nombre de codepoints
		int* codepoints = LoadCodepoints(text.c_str(), &cpts_count);

		if(not codepoints or cpts_count<=0)
		{
			if(codepoints)UnloadCodepoints(codepoints);
			gs::lastError("||Warning|| in 'Font::render()', couldn't check required codepoints ( LoadCodepoints() error ? ). [given arg 'text' : \"", text+"\"]");
			return nullptr;
		}

		bool need_reload = 
			not (areAllCodepointsPresent(codepoints, cpts_count)
				and m_ray_font.baseSize == required_base_size);

		bool loaded = false;
		if(need_reload)
			loaded = loadRayFont(codepoints, cpts_count, "'Font::render()'", false); //On recharge le font avec les codepoints du texte. Note: pas de conservation des précédent codes points ici.
		
		UnloadCodepoints(codepoints);

		if(need_reload and not loaded)
			return nullptr;
	}


	//Mesure du texte, afin de correctement dimensionner le support.
	float spacing = 0.f;
	Vector2 text_pix_size = MeasureTextEx(m_ray_font, text.c_str(), required_base_size, spacing);
	
	//Génération du support du texte
	Surface surface = GenImageColor(text_pix_size.x, text_pix_size.y, bg_color_to_use);
	if(not IsImageValid(surface))
	{
		gs::lastError("||Warning|| in 'Font::render()', couldn't generate text support image ( GenImageColor() error ? ). [given arg 'text' : \"", text+"\"]");
		return nullptr;
	}

	//Ecriture du texte par dessus
	ImageDrawTextEx(&surface, m_ray_font, text.c_str(), Vector2(0,0), required_base_size, 0.f, text_color);


	//Post-traitements...
	if(text == " " and m_wider_space_token_mode)
		hackRenderedWidth(&surface, m_wider_space_factor);

	if(m_userInterventionCallback)
		m_userInterventionCallback(&surface, text);


	//Passage à la texture
	Texture text_texture = LoadTextureFromImage(surface);
	UnloadImage(surface);

	if(not IsTextureValid(text_texture))
	{
		gs::lastError("||Warning|| in 'Font::render()', couldn't generate text texture ( LoadTextureFromImage() error ? ). [given arg 'text' : \"", text+"\"]");
		return nullptr;
	}


	INTERNAL_USE_setTextureScaleMode(text_texture, m_scale_mode);

	if(text_size_out != nullptr)
	{
		text_size_out->x = text_texture.width;
		text_size_out->y = text_texture.height;
	}

	return _textures_manager_.addTexture(&text_texture);;
}







Texture* Font::renderMultiLines(const std::vector<std::string>& text, Color text_color,
					te::Paragraph_Lines_Type paragraph_lines_type,
					te::Font_Rendering_Type rendering_type,
					Color* bg_color,
					Vec2f* text_size_out) //const
{

	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Font::renderMultiLines()', cannot render : not yet init");
		return nullptr;
	}


	if(std::empty(text))
	{
		gs::lastError("||Warning|| in 'Font::renderMultiLines()', given empty text. ");
		return nullptr;
	}


	int max_w=0;
	int max_h=0;
	int w_buf=0;
	int line_y_space = getFontLineSkip(); //Espace recommandé entre les lignes.

	//Détermination de la taille totale, en pixel.
	for(const std::string& text_line : text)
	{
		w_buf = getTextPixelSize(text_line.c_str()).x;

		if(w_buf > max_w)
			max_w = w_buf;

		max_h += line_y_space;
	}

	
	RenderTexture2D support = LoadRenderTexture(max_w, max_h);

	if(not IsRenderTextureValid(support))
	{
		gs::lastError("||Warning|| in 'Font::renderMultiLines()', create support texture failed.");
		return nullptr;
	}



	BeginTextureMode(support); //--------------------------

	std::vector<gs::Texture*> to_destroy; //Les textures temporaires ne doivent pas être détruites trop tôt dans le déroulé (fonctionnement de raylib/OpenGL) ce buffer sert à les mémoriser pour les détruire plus tard dans la fonction.

	short current_y=0;
	gs::Texture* tmp_line_texture=nullptr;
	gs::Texture* output_texture=nullptr;

	for(const std::string& text_line : text) //Dessin des différentes lignes de texte, sur le support
	{
		Vec2f size_buf;
		tmp_line_texture = render(text_line.c_str(), text_color,
							rendering_type, bg_color, &size_buf);

		if(not tmp_line_texture)
			goto failure;

		FRect dest_rect = {0.f, static_cast<float>(current_y), size_buf.x, size_buf.y};

		switch(paragraph_lines_type) {

			case te::LEFT_BEGIN:
				break; // car (x == 0) -> true

			case te::RIGHT_BEGIN:
				dest_rect.x = max_w - dest_rect.w;
				break;

			case te::CENTERED:
				dest_rect.x = max_w/2 - dest_rect.w/2;
				break;

			default:
				break;
		}

		DrawTexture(*tmp_line_texture, dest_rect.x, dest_rect.y, {255,255,255,255});
		
		to_destroy.push_back(tmp_line_texture); //Note: appeler 'gs::destroyTexture(tmp_line_texture)' ici serait trop "tôt".
		tmp_line_texture = nullptr;

		current_y += line_y_space;
	}

	EndTextureMode(); //--------------------------

	//Récupération de la texture finale
	{
		Image img_buf = LoadImageFromTexture(support.texture);
		UnloadRenderTexture(support);

		if(not IsImageValid(img_buf))
			gs::lastError(std::string("||Warning|| in 'Font::renderMultiLines()', ending stages : --to image buffer-- failed."));
		else
		{
			ImageFlipVertical(&img_buf); //Pour régler le passage de 'RenderTexture2D' à 'Image'/'Texture2D'.
			
			gs::Texture tex = LoadTextureFromImage(img_buf);
			UnloadImage(img_buf);

			if(not IsTextureValid(tex))
				gs::lastError(std::string("||Warning|| in 'Font::renderMultiLines()', ending stages : --image buffer to texture-- failed."));
			else
			{
				INTERNAL_USE_setTextureScaleMode(tex, m_scale_mode);

				if(text_size_out) {
					text_size_out->x = max_w;
					text_size_out->y = max_h;
				}

				output_texture = _textures_manager_.addTexture(&tex);
			}
		}
	}

	for(gs::Texture* t : to_destroy) if(t) gs::destroyTexture(t); //Important

	return output_texture;


	failure:

	gs::lastError(std::string("from 'Font::renderMultiLines()' : ")+gs::lastError());

	EndTextureMode();

	for(gs::Texture* t : to_destroy) if(t) gs::destroyTexture(t); //Important

	if(IsRenderTextureValid(support))
		UnloadRenderTexture(support);

	if(tmp_line_texture)
		destroyTexture(tmp_line_texture);

	return nullptr;
}




	











//=====================================================================================//
//===================================- Widget -========================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//=====================================================================================//
//===================================- Widget -========================================//
//=====================================================================================//


struct INTERNAL_USE_Mouse_Event
{
	uint32_t type=0; //wi::DOWN_CLICK ou wi::UP_CLICK,  si différent des deux : c'est un mouvement de souris
	uint32_t button=0; //BK_MOUSE_BUTTON_LEFT, BK_MOUSE_BUTTON_RIGHT, BK_MOUSE_BUTTON_MIDDLE
	Vec2f mouse_pos;
	Vec2f mouse_delta;
};

std::vector<INTERNAL_USE_Mouse_Event> INTERNAL_USE_getMouseEvents()
{
	//Mapping des événements attendus

	std::vector<INTERNAL_USE_Mouse_Event> occured;
	Vec2f mouse_curr_pos;
	getMousePos(&mouse_curr_pos.x, &mouse_curr_pos.y);

	std::vector<uint32_t> mouse_btns{BK_MOUSE_BUTTON_LEFT, BK_MOUSE_BUTTON_RIGHT, BK_MOUSE_BUTTON_MIDDLE}; //Les boutons de la souris qui vont être pris en compte.
	
	for(uint32_t mouse_btn : mouse_btns) //Parcours des boutons pris en compte
	{
		//On considère qu'il y a un événement sur le bouton uniquement s'il y a eu un click (que ce soit haut ou bas).

		INTERNAL_USE_Mouse_Event ms_ev;
		ms_ev.button = mouse_btn;

		if(IsMouseButtonPressed(ms_ev.button))
			ms_ev.type = wi::DOWN_CLICK;
		else if(IsMouseButtonReleased(ms_ev.button))
			ms_ev.type = wi::UP_CLICK;
		else
			continue;

		ms_ev.mouse_pos = mouse_curr_pos;
		occured.push_back(ms_ev);
	}


	//Traitement du cas du déplacement de la souris
	{
		INTERNAL_USE_Mouse_Event ms_ev;
		ms_ev.type = (wi::DOWN_CLICK + wi::UP_CLICK)*2; // -> Une valeur différente des deux autres (car il ne s'agit pas d'un événement de click).
		
		Vector2 motion = GetMouseDelta();
		if(motion.x!=0.f or motion.y!=0.f)
		{
			ms_ev.mouse_delta.x = motion.x;
			ms_ev.mouse_delta.y = motion.y;

			ms_ev.mouse_pos = mouse_curr_pos;
			occured.push_back(ms_ev);
		}
	}
	
	return occured;
}

std::vector<Button*>& INTERNAL_USE_getButtonsVec();
std::vector<Scrollbar*>& INTERNAL_USE_getScrollbarsVec();

void INTERNAL_USE_checkWidgetsEvent()
{
	std::vector<Button*>& buttons_vec = INTERNAL_USE_getButtonsVec();
	std::vector<Scrollbar*>& scrollbars_vec = INTERNAL_USE_getScrollbarsVec();

	
	std::vector<INTERNAL_USE_Mouse_Event> occured_mouse_events = INTERNAL_USE_getMouseEvents();
	 
	
	for(const INTERNAL_USE_Mouse_Event& ms_ev : occured_mouse_events) 
	{
		const uint32_t& event_type = ms_ev.type;
		const uint32_t& mouse_btn = ms_ev.button;
		const Vec2f& mouse_pos = ms_ev.mouse_pos;
		


		for(Button* btn : buttons_vec)
		{
			if(not btn)
				continue;

			Rect btn_area = rect(btn->getArea());
			Point mouse_pos_i = Point(int(mouse_pos.x), int(mouse_pos.y));

			if(pointInRect(&mouse_pos_i, &btn_area))
			{
				INTERNAL_USE_editState(*btn, wi::MOUSE_IN, true);

				if(event_type == btn->getTargetEvent())
				{
					if(mouse_btn == btn->getTargetButtonEvent())
						INTERNAL_USE_editState(*btn, wi::CLICKED_IN, true);
					else
						INTERNAL_USE_editState(*btn, wi::CLICKED_IN, false);
				
				}
				else
					INTERNAL_USE_editState(*btn, wi::CLICKED_IN, false);

			}
			else
			{
				INTERNAL_USE_editState(*btn, wi::MOUSE_IN, false);
				INTERNAL_USE_editState(*btn, wi::CLICKED_IN, false);
			}
		}




		for(Scrollbar* bar : scrollbars_vec)
			if(bar)
				bar->INTERNAL_USE_checkEvent(ms_ev);

	}
}




void Backend__Scrollbar::INTERNAL_USE_checkEvent(const INTERNAL_USE_Mouse_Event& mouse_event)
{
	//Récupération des membres requis ---------------------------------
	INTERNAL__membersTransfer mb;
	INTERNAL_USE_getMembersPtrs_forEventProcessing(mb);
	if( !mb.m_mouse_clicked_in or
		!mb.m_orient or
		!mb.m_to_move_buf
		)
	{
		gs::lastError("||INTERAL|| : in 'Backend__Scrollbar::INTERNAL_USE_checkEvents()', after 'INTERNAL_USE_getMembersPtrs_forEventProcessing()', have invalid ptr.");
		return;
	}
	bool& m_mouse_clicked_in = *mb.m_mouse_clicked_in;
	wi::Scrollbar_Orient& m_orient = *((wi::Scrollbar_Orient*)mb.m_orient);
	short& m_to_move_buf = *mb.m_to_move_buf;
	//------------------------------------------------------------------


	static Rect area;
	static Rect moveable_thing_area;

	//Définition de la zone globale de la scrollbar, et de l'élément amovible.
	setAreaRects(area, moveable_thing_area);


	const uint32_t& event_type = mouse_event.type;
	const uint32_t& mouse_btn = mouse_event.button;
	const Vec2f& mouse_pos = mouse_event.mouse_pos;
	
	
	if(event_type == wi::UP_CLICK and mouse_btn == BK_MOUSE_BUTTON_LEFT)
		m_mouse_clicked_in = false;
	else
	if(event_type == wi::DOWN_CLICK and mouse_btn == BK_MOUSE_BUTTON_LEFT)
	{
		Point click_pos=mouse_pos.toPoint();
		if(pointInRect(&click_pos, &moveable_thing_area))
			m_mouse_clicked_in = true;
	}
	else
	if(m_mouse_clicked_in)
	{
		//Déplacement de la souris
		if(event_type != wi::DOWN_CLICK and event_type != wi::UP_CLICK)
		{
			if(m_orient == wi::VERTICAL)
				m_to_move_buf = mouse_event.mouse_delta.y;
			else
				m_to_move_buf = mouse_event.mouse_delta.x;
		}
	}
}




bool Trail::draw(Renderer* ren, Texture* texture, Color tint, const Camera2D_Base& cam)
{
    bool return_val = true;
	Trail_Vertices d=getVertices(tint, cam);
	if(d.vertices.size()==0 or d.indices.size()==0) return return_val;


	bool use_texture = texture != nullptr;

	rlBegin(RL_TRIANGLES);

		if(texture)
			rlSetTexture(texture->id);

		FColor color = d.vertices.front().color; //Récupération de la couleur de fond (tous les vertices ont normalement la même couleur de fond, celle indiquée par 'tint').
		rlColor4f(color.r, color.g, color.b, color.a);

		for(int indice : d.indices) //Reconstitution des tous les triangles
		{
			try{
			Vertex& v = d.vertices.at(indice);

			if(texture)
				rlTexCoord2f(v.tex_coord.x, v.tex_coord.y);

			rlVertex2f(v.position.x, v.position.y);
			
			}catch(std::exception& excep){
				return_val = false;
				gs::lastError("||Warning|| in 'Trail::draw()', vertices drawing, exception occured : ", excep.what());
			}
		}

	rlEnd();

	if(texture)
		rlSetTexture(0);

	return return_val;
}







} //namespace gs
