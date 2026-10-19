#ifndef _GSlib_raylib_HPP_
#define _GSlib_raylib_HPP_


#include <raylib/raylib.h>
#include <iostream>
//#include <vector>
//#include <string>
//#include <cmath>
//#include <unordered_map>

struct _skip_arg_global_scope_struct_t_
{};

namespace gs  /*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
{

//Note: Vec2f et FPoint doivent avoir les mêmes membres, histoire que Vec2f[] puisse être lisible sous FPoint* et vice versa.
template<class T> struct Vec2;

struct Window {};
struct Renderer{};
using Texture = Texture2D;
using Texture_Target = RenderTexture2D;
using Surface = Image;
struct Rect{int x=0, y=0, w=0, h=0;};
struct FRect{float x=0.f, y=0.f, w=0.f, h=0.f;};
struct Point{int x=0, y=0;};
struct FPoint{float x=0.f, y=0.f;};
using Color = ::Color;
struct FColor{float r=0.f,g=0.f,b=0.f,a=0.f;};
enum Blend_Mode{BLENDMODE_NONE, BLENDMODE_BLEND};
enum Flip_Mode{FLIP_NONE, FLIP_HORIZONTAL, FLIP_VERTICAL, FLIP_HORIZONTAL_AND_VERTICAL};
enum Scale_Mode{SCALEMODE_INVALID=-1, SCALEMODE_NEAREST, SCALEMODE_LINEAR, SCALEMODE_PIXELART};
struct Vertex{FPoint position; FColor color; FPoint tex_coord;};



#define BK_Font_t ::Font
#define BK_Event_t _skip_arg_global_scope_struct_t_

#define FONT_STYLE_NORMAL        0x00
#define FONT_STYLE_BOLD          0x01
#define FONT_STYLE_ITALIC        0x02
#define FONT_STYLE_UNDERLINE     0x04
#define FONT_STYLE_STRIKETHROUGH 0x08

#define BK_EVENT_MOUSE_BUTTON_DOWN 1
#define BK_EVENT_MOUSE_BUTTON_UP 2

#define BK_MOUSE_BUTTON_LEFT MouseButton::MOUSE_BUTTON_LEFT
#define BK_MOUSE_BUTTON_RIGHT MouseButton::MOUSE_BUTTON_RIGHT
#define BK_MOUSE_BUTTON_MIDDLE MouseButton::MOUSE_BUTTON_MIDDLE

    


#if SHOW_NON_USED
#define _GS_UNUSED_F_MSG_(f_name)	std::cout << "'"f_name"' not used with raylib.\n"
#else
#define _GS_UNUSED_F_MSG_(f_name)
#endif


void addToClearAtTheEnd_backend(Texture* tex, void* texturesToClearAtTheEnd_ptr);

inline void destroyRenderer(Renderer* ren) {_GS_UNUSED_F_MSG_("destroyRenderer()");}
void destroyTexture(Texture* t);
void destroyTexture(Texture_Target* trgt);
void destroySurface(Surface* s);
Vec2<float> getTextureSize(Texture* t, float* w=nullptr, float* h=nullptr);
Vec2<float> getSurfaceSize(Surface* s, float* w=nullptr, float* h=nullptr);
bool drawTexture(Renderer* ren, Texture* t, FRect* src, FRect* dest);
bool drawTexture(Renderer* ren, Texture_Target* trgt, FRect* src, FRect* dest);
bool drawTextureRotated(Renderer* ren, Texture* t, const FRect* src_rect, const FRect* dest_rect, float angle_degree, const FPoint* rotation_center=nullptr, Flip_Mode flip=FLIP_NONE);
bool drawTextureRotated(Renderer* ren, Texture_Target* trgt, const FRect* src_rect, const FRect* dest_rect, float angle_degree, const FPoint* rotation_center=nullptr, Flip_Mode flip=FLIP_NONE);
Vec2<int> getCurrentRenderOutputSize(Renderer* ren=nullptr, int* w=nullptr, int* h=nullptr);
Vec2<int> getRenderOutputSize(Renderer* ren=nullptr, int* w=nullptr, int* h=nullptr);
#define GetMin(a, b) (a < b ? a : b)
#define GetMax(a, b) (a > b ? a : b)
inline const char* Backend_GetError() {return "";}
inline bool pointInRect(const Point* p, const Rect* r) {if(p and r) return r->x <= p->x and p->x < r->x+r->w and r->y <= p->y and p->y < r->y+r->h; return false;}
inline bool pointInRect(Point p, Rect r) {return pointInRect(&p, &r);}
inline bool hasRectIntersection(Rect* r1, Rect* r2) {std::cout <<"hasRectIntersection(), not yet implemented for raylib backend.\n"; return false;}


inline Color getRenderDrawColor(Renderer* ren) {_GS_UNUSED_F_MSG_("getRenderDrawColor"); return Color();}
inline bool setRenderDrawColor(Renderer* ren, FColor col) {_GS_UNUSED_F_MSG_("setRenderDrawColor"); return false;}
inline bool setRenderDrawColor(Renderer* ren, Color col) {_GS_UNUSED_F_MSG_("setRenderDrawColor"); return false;}
inline Color restaurePrevRenderDrawColor(Renderer* ren) {_GS_UNUSED_F_MSG_("restaurePrevRenderDrawColor"); return Color();}
inline Blend_Mode getRenderBlendMode(Renderer* ren) {_GS_UNUSED_F_MSG_("getRenderBlendMode"); return BLENDMODE_NONE;}
inline bool setRenderBlendMode(Renderer* ren, Blend_Mode blend_mode) {_GS_UNUSED_F_MSG_("setRenderBlendMode"); return false;}
inline Blend_Mode restaurePrevRenderBlendMode(Renderer* ren) {_GS_UNUSED_F_MSG_("restaurePrevRenderBlendMode"); return BLENDMODE_NONE;}


Texture_Target* getRenderTarget(Renderer* ren=nullptr);
bool setRenderTarget(Renderer* ren, Texture_Target* texture_target);
inline Texture* textureOf(Texture_Target* texture_target) {if(texture_target) return &texture_target->texture; return nullptr;}

Surface* textureToSurface(Renderer* ren, Texture* tex);
Texture* surfaceToTexture(Renderer* ren, Surface* surface);
bool drawLines_INTERNAL(Renderer* ren, const FPoint* points, int pts_count, const Color& col, Blend_Mode* b_mode);


Vector2 v2(Vec2<float> v);
inline Rectangle rectangle(FRect r){return Rectangle(r.x,r.y,r.w,r.h);}







//_____________________________________________________________________
//_____________________________________________________________________



class Backend__Mouse_Tracker
{
	public:
        //void processEvent(...); -> non utilisée pour le backend raylib

    protected:
        bool       m_target_btn_is_left=true;
		FPoint      m_pos_when_down;
		bool       m_is_down=false;
		FPoint      m_pos_when_up;
        bool       m_is_multi_click=false;
		char       m_clicks_count=0;
		FPoint      m_prev_down_pos; //Sert à savoir si la souris a clické à deux endroits différents sur les deux derniers clicks.
        FPoint  m_wheel_buff;
};




class Backend__Game
{
    public:
        void backendDestr() {};
       
		//Pour la compatibilité entre backends :
			Renderer* ren() const;
			Window* win() const;

			bool skip_vsync_error_msg=false;

    protected:
        bool isInit{false};
        bool is_main_game{true};

        Window *win_{nullptr};
		Renderer *ren_{nullptr};
		Surface *win_icon{nullptr};

		int Win_w;
		int Win_h;
		int m_ren_w=0;
		int m_ren_h=0;
};



//_____________________________________________________________________
//_____________________________________________________________________


class Backend__Font
{
    public:
		~Backend__Font();
        int getFontLineSkip();
    protected:

		//ATTENTION : Ces deux méthodes sont à "risque" car manipulent des tableaux C, --> dans l'absolue rien n'empêche le fait que 'num_codepoints' ne dépasse la taille réelle du tableau pointé par 'codepoints'.
		bool areAllCodepointsPresent(const int* codepoints, int num_codepoints) const;
		bool loadRayFont(const int* codepoints, int num_codepoints, const char* up_fun_name, bool keep_prev_codepoints);
		
		float INTERNAL_USE_fromPtsizeToBaseSize() const;

		bool m_is_init=false;
		size_t m_ptsize=32u;
		
		BK_Font_t m_ray_font;
		std::string m_font_path;
};




class Backend__Text_Window
{
    public:
        //bool processEvent(...); -> non utilisée pour le backend raylib
	
	protected:
		void INTERNAL_USE_checkEvents();
};



//_____________________________________________________________________
//_____________________________________________________________________

struct INTERNAL_USE_Mouse_Event;
void INTERNAL_USE_checkWidgetsEvent();


class Backend__Scrollbar
{
	friend void INTERNAL_USE_checkWidgetsEvent();

    public:
        //void processEvent(...); -> non utilisée pour le backend raylib
        virtual void setAreaRects(Rect& area, Rect& moveable_thing_area) const = 0;

	protected:
		void INTERNAL_USE_checkEvent(const INTERNAL_USE_Mouse_Event& mouse_event);

		struct INTERNAL__membersTransfer
        {
            bool* m_mouse_clicked_in=nullptr;
            void* m_orient=nullptr;
            short* m_to_move_buf=nullptr;
        };
        virtual void INTERNAL_USE_getMembersPtrs_forEventProcessing(INTERNAL__membersTransfer& ptrs)=0;
};



}//namespace gs



#endif // _GSlib_raylib_HPP_
