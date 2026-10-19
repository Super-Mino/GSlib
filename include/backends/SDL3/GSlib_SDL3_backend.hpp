#ifndef _GSlib_SDL3_HPP_
#define _GSlib_SDL3_HPP_

//Mise en place : 2026/08/26

#include <SDL3/SDL.h>
#include <SDL3/SDL_ttf.h>



namespace gs /*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
{

//Note: Vec2f et FPoint doivent avoir les mêmes membres, histoire que Vec2f[] puisse être lisible sous FPoint* et vice versa.
template<class T> struct Vec2;

using Window = SDL_Window;
using Renderer = SDL_Renderer;
using Texture = SDL_Texture;
using Texture_Target = SDL_Texture;
using Surface = SDL_Surface;
using Rect = SDL_Rect;
using FRect = SDL_FRect;
using Point = SDL_Point;
using FPoint = SDL_FPoint;
using Color = SDL_Color;
using FColor = SDL_FColor;
enum Blend_Mode{BLENDMODE_NONE=SDL_BLENDMODE_NONE, BLENDMODE_BLEND=SDL_BLENDMODE_BLEND};
enum Flip_Mode{FLIP_NONE=SDL_FLIP_NONE, FLIP_HORIZONTAL=SDL_FLIP_HORIZONTAL, FLIP_VERTICAL=SDL_FLIP_VERTICAL, FLIP_HORIZONTAL_AND_VERTICAL=SDL_FLIP_HORIZONTAL_AND_VERTICAL};
enum Scale_Mode{SCALEMODE_INVALID=SDL_SCALEMODE_INVALID, SCALEMODE_NEAREST=SDL_SCALEMODE_NEAREST, SCALEMODE_LINEAR=SDL_SCALEMODE_LINEAR, SCALEMODE_PIXELART=SDL_SCALEMODE_PIXELART};
using Vertex = SDL_Vertex;



#define BK_Font_t TTF_Font
#define BK_Event_t SDL_Event

#define FONT_STYLE_NORMAL TTF_STYLE_NORMAL
#define FONT_STYLE_BOLD TTF_STYLE_BOLD
#define FONT_STYLE_ITALIC TTF_STYLE_ITALIC
#define FONT_STYLE_UNDERLINE TTF_STYLE_UNDERLINE
#define FONT_STYLE_STRIKETHROUGH TTF_STYLE_STRIKETHROUGH

#define BK_EVENT_MOUSE_BUTTON_DOWN SDL_EVENT_MOUSE_BUTTON_DOWN
#define BK_EVENT_MOUSE_BUTTON_UP SDL_EVENT_MOUSE_BUTTON_UP

#define BK_MOUSE_BUTTON_LEFT SDL_BUTTON_LEFT
#define BK_MOUSE_BUTTON_RIGHT SDL_BUTTON_RIGHT
#define BK_MOUSE_BUTTON_MIDDLE SDL_BUTTON_MIDDLE



void addToClearAtTheEnd_backend(Texture* tex, void* texturesToClearAtTheEnd_ptr);

inline void destroyRenderer(Renderer* ren) {SDL_DestroyRenderer(ren);}
inline void destroyTexture(Texture* texture) {SDL_DestroyTexture(texture);}
inline void destroySurface(Surface* surface) {SDL_DestroySurface(surface);}
Vec2<float> getTextureSize(Texture* texture, float* w=nullptr, float* h=nullptr);
Vec2<float> getSurfaceSize(Surface* surface, float* w=nullptr, float* h=nullptr);
inline bool drawTexture(Renderer* ren, Texture* texture, const FRect* src, const FRect* dest) {return SDL_RenderTexture(ren, texture, src, dest);}
inline bool drawTextureRotated(Renderer* ren, Texture* texture, const FRect* src, const FRect* dest, float angle_degree, const FPoint* rotation_center=nullptr, Flip_Mode flip=FLIP_NONE) {return SDL_RenderTextureRotated(ren, texture, src, dest, angle_degree, rotation_center, (SDL_FlipMode)flip);}
Vec2<int> getCurrentRenderOutputSize(Renderer* ren=nullptr, int* w=nullptr, int* h=nullptr);
Vec2<int> getRenderOutputSize(Renderer* ren=nullptr, int* w=nullptr, int* h=nullptr);
#define GetMin SDL_min
#define GetMax SDL_max
#define Backend_GetError SDL_GetError
inline bool pointInRect(const Point* pt, const Rect* rect) {return SDL_PointInRect(pt, rect);}
inline bool pointInRect(Point p, Rect r) {return pointInRect(&p, &r);}
inline bool hasRectIntersection(Rect* r1, Rect* r2) {return SDL_HasRectIntersection(r1, r2);}

SDL_Color getRenderDrawColor(SDL_Renderer* ren);
bool setRenderDrawColor(SDL_Renderer* ren, SDL_FColor col);
bool setRenderDrawColor(SDL_Renderer* ren, SDL_Color col);
SDL_Color restaurePrevRenderDrawColor(SDL_Renderer* ren);

SDL_BlendMode getRenderBlendMode(SDL_Renderer* ren);
bool setRenderBlendMode(SDL_Renderer* ren, SDL_BlendMode blend_mode);
SDL_BlendMode restaurePrevRenderBlendMode(SDL_Renderer* ren);

inline Texture_Target* getRenderTarget(Renderer* ren) {return SDL_GetRenderTarget(ren);}
bool setRenderTarget(Renderer* ren, Texture_Target* texture_target);
inline Texture* textureOf(Texture_Target* texture_target) {return texture_target;}

Surface* textureToSurface(Renderer* ren, Texture* tex);
Texture* surfaceToTexture(Renderer* ren, Surface* surface);
bool drawLines_INTERNAL(Renderer* ren, const FPoint* points, int pts_count, const Color& col, Blend_Mode* b_mode);

void widgetsProcessEvent(SDL_Event& ev);






//_____________________________________________________________________
//_____________________________________________________________________



class Backend__Mouse_Tracker
{
    public:
		void processEvent(const SDL_Event& ev);

    protected:
        bool       m_target_btn_is_left=true;
		SDL_FPoint      m_pos_when_down;
		bool       m_is_down=false;
		SDL_FPoint      m_pos_when_up;
        bool       m_is_multi_click=false;
		char       m_clicks_count=0;
		SDL_FPoint      m_prev_down_pos; //Sert à savoir si la souris a clické à deux endroits différents sur les deux derniers clicks.
        SDL_FPoint  m_wheel_buff;
};

class Backend__Game
{
    public:
        void backendDestr();

        bool skip_vsync_error_msg=false;
		bool allow_GSlib_try_choose_ren_when_no_given_ren_name=true;
		bool InitSDLEngine(int window_w = 800, int window_h = 480, const char* window_title = "MyGame", Uint32 win_flags=0, const char* renderer_name=nullptr, bool try_enable_VSYNC=false);

		bool InitByAssociateTo(SDL_Window *W, SDL_Renderer *newR );

        Renderer* ren() const;
		Window* win() const;

        Uint32 getWinID() const;
        bool isCloseWinEvent(SDL_Event& ev);

        void changeRenderer(Renderer *newR, bool destroy = false);

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
        Backend__Font(){remaining_gs_Font__count++;}
        ~Backend__Font();
        int getFontLineSkip();
    protected:

		bool m_is_init=false;
		size_t m_ptsize=32u;

        BK_Font_t* m_SDL_font=nullptr;

		// !!!!!!!! finalement après avis extérieur : c'est (possiblement) "dangereux" comme méthode (le compteur global des instances). A changer.

		//Pour la gestion de la fermeture de SDL_TTF:
		static bool SDL_TTF__init_by_gs_Font;
    private:
		static int remaining_gs_Font__count;
		//Lorsque qu'un destructeur de gs::Font tombe sur remaining_gs_Font__count<=0 et SDL_TTF__init_by_gs_Font==true
		//alors il appel TTF_Quit() (si nécessaire) et passe SDL_TTF__init_by_gs_Font à false.
		//Note: le constructeur Font() incrémente 'remaining_gs_Font__count' et le destructeur ~Font() le décrémente.
		//Si TTF_Init() est appelée par un gs::Font, alors SDL_TTF__init_by_gs_Font passe à true.
};



class Mouse_Tracker;
template<class T>
struct Vec2;
namespace te {struct Wheel_Details;}
class Input;

class Backend__Text_Window
{
    public:
        bool processEvent(SDL_Event& event, SDL_Window* win);
    protected:
        //Ceux-ci servent à accéder au membres de Text_Window, afin de les utiliser dans 'processEvent()'.
        struct INTERNAL__membersTransfer
        {
            bool* m_is_init=nullptr;
            Mouse_Tracker* m_mouse_tracker=nullptr;
            bool* m_check_mouse=nullptr;
            Vec2<int>* m_mouse_clicked_pos=nullptr;
            te::Wheel_Details* m_wheel=nullptr;
            int* m_curr_active_line_idx=nullptr;
            Input* m_inputs=nullptr;
        };
        virtual void INTERNAL_USE_getMembersPtrs_forEventProcessing(INTERNAL__membersTransfer& ptrs)=0;
};




class Backend__Scrollbar
{
    public:
        void processEvent(const SDL_Event& ev);
        virtual void setAreaRects(Rect& area, Rect& moveable_thing_area) const = 0;

    protected:
        struct INTERNAL__membersTransfer
        {
            bool* m_mouse_clicked_in=nullptr;
            void* m_orient=nullptr;
            short* m_to_move_buf=nullptr;
        };
        virtual void INTERNAL_USE_getMembersPtrs_forEventProcessing(INTERNAL__membersTransfer& ptrs)=0;
};

} //namespace gs

#endif //_GSlib_SDL3_HPP_