#ifndef _GSlib_WIDGET_HPP_
#define _GSlib_WIDGET_HPP_

#include "GSlib.hpp"
#include <unordered_set>

namespace gs
{

struct Btn_Param;
struct Scbr_Param;
class Button;
class Scrollbar;

//Pour la gestion interne des scrollbars et boutons:

	//void scrollbarsAndBtnsProcessEvent(...); -> selon le backend utilisé
	void updateScrollbarsAndBtns(float dt);

	Button& btn(int id, bool* is_ok=nullptr); //Afin de récupérer les instances internes, et pouvoir les manipuler.
	Button& btn(const std::string& id, bool* is_ok=nullptr);
	Scrollbar& scbr(int id, bool* is_ok=nullptr);
	Scrollbar& scbr(const std::string& id, bool* is_ok=nullptr);

	bool setBtnsFontPath(uint8_t target_internal_font_idx, const std::string& path);

	void turnOff0IDBtnWarning();
	void turnOn0IDBtnWarning();
	void turnOff0IDScbrWarning();
	void turnOn0IDScbrWarning();

	void setNextScrollbarsAndBtnsTargetGroup(int group_id);
	void setNextScrollbarsAndBtnsTargetGroup(const std::string& group_id);
	
	void setScrollbarsAndBtnsGroupActiveState(int group_id, bool active);
	void setScrollbarsAndBtnsGroupActiveState(const std::string& group_id, bool active);
	bool getScrollbarsAndBtnsGroupActiveState(int group_id);
	bool getScrollbarsAndBtnsGroupActiveState(const std::string& group_id);

	bool newBtn(int id, Vec2f pos, const Btn_Param& param); //Note: les arguments par défaut de ces 'newBtn()' sont mentionnés plus bas.
	bool newBtn(const std::string& id, Vec2f pos, const Btn_Param& param);
	bool newBtn(int id, FRect area, Btn_Param param);
	bool newBtn(const std::string& id, FRect area, Btn_Param param);

	void showBtns(Renderer* ren); //tous
	void showBtns(Renderer* ren, int group_id);
	void showBtns(Renderer* ren, const std::string& group_id);


	bool newScbr(int id, const Scbr_Param& param); //Note: les arguments par défaut de ces 'newScbr()' sont mentionnés plus bas.
	bool newScbr(const std::string& id, const Scbr_Param& param);

	void showScbrs(Renderer* ren); //tous
	void showScbrs(Renderer* ren, int group_id);
	void showScbrs(Renderer* ren, const std::string& group_id);




//==============================================================================//
//==============================================================================//
//==============================================================================//





struct Btn_Param //Les caractéristiques visuelles et autres, d'un bouton
{
	//--------------------------------------------
	Vec2f size{60.f,25.f}; //En pixel

	Color idle_color={240,10,10,255}; //par défaut
	Color down_color={0,240,0,255}; //lorsqu'il est enfoncé
	Color mouse_in_color={100,100,100,70}; //Lorsque la souris passe dessus sans rien faire d'autre

	Texture* texture=nullptr; //Maximum deux tuiles sont attendues (une pour le bouton "élevé", une pour le bouton enfoncé)
	Vec2i texture_tile_size{-1,-1}; //Taille de tuile donnée en pixel
	Vec2i idle_tile_pos{0,0}; //En tuile
	Vec2i down_tile_pos{0,0}; //En tuile - Note: 'texture_tile_size', 'idle_tile_pos' et 'down_tile_pos' sont initialisées avec la volonté que si l'on ne mentionne de valeur que pour 'texture', cette texture soit complètement affichée sur le bonton dans l'état "levé" et enfoncé.
	bool acquire_texture_unloading_responsibility=false; //Indique si le gs::Button qui va prendre ces paramètres doit se charger de la destruction de 'texture' au destructeur de gs::Button.

	//Lorsqu'une texture est mentionnée:
	//c'est la texture qui est affichée (et non 'idle_color' ou 'down_color'), et potentiellement, en fonction des tuiles mentionnées.
	//La couleur de la souris s'applique par dessus ce qui est affiché.

	std::string idle_text; //Les textes à afficher, selon l'état.
	std::string down_text;
	uint16_t font_size=22u; //ptsize du texte - à savoir que c'est bornée par la taille en y du bouton.
	Color text_color={224,224,224,255};
	uint8_t font_idx=0u; //indice du font à utiliser parmi les BTNS_FONTS_COUNT disponibles.
	

	//-----------------------------------------------

	void (*down_click_callback)(Button& btn, void* user_data)=nullptr; //Fonction appelée lorsque le bouton est clické.
	void (*release_callback)(Button& btn, void* user_data)=nullptr; //Fonction appelée lorsque le bouton est relaché.
	void* callback_arg=nullptr;

};

//Je remets ça ici pour pouvoir utliser '=Btn_Param()'.
bool newBtn(int id, Vec2f pos=Vec2f(0.f,0.f), const Btn_Param& param=Btn_Param());
bool newBtn(const std::string& id, Vec2f pos=Vec2f(0.f,0.f), const Btn_Param& param=Btn_Param());
bool newBtn(int id, FRect area, Btn_Param param=Btn_Param());
bool newBtn(const std::string& id, FRect area, Btn_Param param=Btn_Param());
bool newBtn(Vec2f pos, const Btn_Param& param=Btn_Param()); //Avec 'id==0'
bool newBtn(FRect area, Btn_Param param=Btn_Param()); //Avec 'id==0'


namespace wi
{
	enum Button_State {BTN_IDLE, BTN_DOWN, BTN_MOUSE_IN};
	enum Action_Val {DOWN_CLICK=BK_EVENT_MOUSE_BUTTON_DOWN, UP_CLICK=BK_EVENT_MOUSE_BUTTON_UP};
	enum Quick_Btns_Field {CHECK_AREA, BUTTONS};
	enum Btn_Utils{BTN_NONE_ID=0, BTNS_FONTS_COUNT=3}; //'BTN_NONE_ID' sert à mentionner une valeur libre, celle qui ne compte pas dans les identifiants.

}

class Button : public Backend__Button
{
	public :
		Button()=default;
		~Button();
		Button(const Button&)=delete;
		Button& operator=(const Button&)=delete;
		Button(Button&&) noexcept =default;
		Button& operator=(Button&&) noexcept =default;

		void enable() {m_enabled = true;}
		void disable() {m_enabled = false;}
		bool enabled() const {return m_enabled;}

		bool init(FRect area, Btn_Param param=Btn_Param());
		bool init(Vec2f pos=Vec2f(0.f,0.f), const Btn_Param& param=Btn_Param());

		bool loadTexture(Renderer* ren, const std::string& img_file_path);
		bool setTexture(Texture* texture, bool acquire_unloading_responsibility);

		//Si le bouton est géré indépendamment:
			//void processEvent(...); selon le backend, sinon, 'update()' suffit.
			void update(float dt, bool INTERNAL__check_events=true);

		bool show(Renderer* ren) const;

		void setArea(FRect new_area);
		void setPos(float x, float y);
		void incrPos(float x, float y);
		
		void setDownState(bool new_state);

		void setColor(wi::Button_State target_state, Color c);

		bool setDownClickCallback(void (*callback)(Button&, void*));
		bool setReleaseCallback(void (*callback)(Button&, void*));
		void setCallbackArg(void* user_data) {m.callback_arg = user_data;};

		void setText(wi::Button_State target_state, const std::string& text);
		



		bool isInit() const;
		Texture* getTexture();
		FRect getArea() const;
		bool isDown() const;
		Color getColor(wi::Button_State target_state) const;
		bool isMouseIn() const;
		bool downClicked() const {return m_down_clicked;}
		bool released() const {return m_released;} //Si le bouton vient d'être relaché.
		const std::string& getText(wi::Button_State target_state) const;


	protected:

		//Les états
		bool  m_is_init=false;
		Vec2f m_pos={0.f,0.f};
		bool  m_is_down=false;
		bool  m_mouse_in=false;
		bool  m_is_prev_down=false; //reçoit la précédente valeur de 'm_is_down' à chaque à chaque mise à jour. Cela permet de détecter les clicks (bas, et hauts).
		bool m_down_clicked=false;
		bool m_released=false;

		Btn_Param m; //Les caratérisiques.

		bool m_enabled = true; //Permet d'activer/désactiver le bouton. Si désactivé : plus affichée, et ne réagit plus aux events.

	private:

		void INTERNAL_USE_getMembersPtrs_forEventProcessing(INTERNAL__membersTransfer& ptrs) override
		{
			ptrs.m_pos = &m_pos;
			ptrs.m_size = &m.size;
            ptrs.m_is_down= &m_is_down;
            ptrs.m_mouse_in = &m_mouse_in;
			ptrs.enabled = m_enabled;
		}



		friend struct SBIM;

		//Gérées du côté de SBIM et co
			int group_id_i=0;
			std::string group_id_s;
			bool group_id_is_int=true;
};

//==============================================================================//








namespace wi
{
	enum Scrollbar_Orient {VERTICAL, HORIZONTAL};
	enum Scrollbar_Area {MAIN_AREA, MOVEABLE_THING_AREA};
	enum Scbr_Utils{SCBR_NONE_ID=0}; //'BTN_NONE_ID' sert à mentionner une valeur libre, celle qui ne compte pas dans les identifiants.
};


struct Scbr_Param //Les caractéristiques d'un scrollbar
{
	wi::Scrollbar_Orient orient=wi::VERTICAL; //Orientation.
	unsigned int   max_val=100; //Valeur maximale obtenable (peut être changée au besoin).
	unsigned int   moveable_thing_len=30;
	
	Vec2i          pos={10, 10};
	unsigned int   len=300; //Longueur de la scrollbar (en pixel).
	unsigned int   thickness=15; //Epaisseur de la scrollbar (en pixel).
	unsigned char  min_len=10; //Longueur minimale.
	unsigned char  min_thickness=4; //Epaisseur minimale.

	Color          bg_col={170,170,170,255}; //-> de la zone de scroll.
	Color          fg_col={240,240,240,255}; //-> de l'élément amovible.
};


//Je remets ça ici pour pouvoir utliser '=Scbr_Param()'.
bool newScbr(int id, const Scbr_Param& param=Scbr_Param());
bool newScbr(const std::string& id, const Scbr_Param& param=Scbr_Param());
bool newScbr(const Scbr_Param& param=Scbr_Param()); //Avec 'id==0'


class Scrollbar : public Backend__Scrollbar
{
	public :
		bool init(const Scbr_Param& param=Scbr_Param()); //Optionnelle

		bool show(Renderer* ren);

		//Si le scrollbar est géré indépendamment:
			//void processEvent(...); selon le backend, sinon, 'update()' suffit.
			void update(bool INTERNAL__check_events=true);

		bool moved() const;
		unsigned int setProgr(unsigned int val);
		unsigned int getProgr();

		wi::Scrollbar_Orient setOrientation(const wi::Scrollbar_Orient& new_orient);
		wi::Scrollbar_Orient getOrientation() const;

		unsigned int setLen(unsigned int new_len);
		unsigned int getLen() const;

		unsigned int setThickness(unsigned int new_thickness);
		unsigned int getThickness() const;

		Vec2i setPos(Vec2i new_pos);
		Vec2i getPos() const;

		unsigned int setMaxVal(unsigned int max_val);
		unsigned int getMaxVal() const;

		Color setColor(const wi::Scrollbar_Area& area, const Color& color);
		Color getColor(const wi::Scrollbar_Area& area) const;

		Rect getArea(const wi::Scrollbar_Area& area=wi::MAIN_AREA) const;



	protected :

		void checkParam();
		void incrProgr(int val); //Incrémenter la progression.
		void getAreaRects(Rect& area, Rect& moveable_thing_area) const override;

		bool  					m_mouse_clicked_in=false;
		short 					m_to_move_buf=0; //Valeur qui fait incrémenter la position de l'objet amovible lors de la mise à jour.
		bool 					m_moved=false; //Témoin du movement à la dernière mise à jour.
		unsigned int  			m_pixel_progr=0; //Progression en pixel de l'élément amovible, qui permet de déduire la valeur courante.
		// unsigned int  		m_no_need_scroll_count=50;

		Scbr_Param m;
		

	private:
		
		void INTERNAL_USE_getMembersPtrs_forEventProcessing(INTERNAL__membersTransfer& ptrs) override
		{
			ptrs.m_mouse_clicked_in = &m_mouse_clicked_in;
            ptrs.m_orient = (void*)(&m.orient);
            ptrs.m_to_move_buf = &m_to_move_buf;
		}


		friend struct SBIM;

		//Gérées du côté de SBIM et co
			int group_id_i=0;
			std::string group_id_s;
			bool group_id_is_int=true;
};




using easing_function_t = float (*)(float);

class Tweening_Path
{
    public:

        void setFunctionX(float (*xEasingFunction)(float time_progress))
        {
            m_xEasingFunction = xEasingFunction;
        }
        void setFunctionY(float (*yEasingFunction)(float time_progress))
        {
            m_yEasingFunction = yEasingFunction;
        }

		easing_function_t getFunctionX() {return m_xEasingFunction;}
		easing_function_t getFunctionY() {return m_yEasingFunction;}


        bool isRunning() const {return m_is_running;}
        bool run() {if(isRunning() or m_duration <= 0.f) return false; m_timer=0.f; m_is_running=true; return true;}
		void stop() {m_is_running = false;}

        bool setDuration(float dur) {if(isRunning() or dur <= 0.f) return false; m_duration=dur; return true;}
		float getDuration() const {return m_duration;}

        gs::Vec2f getCurr(gs::Vec2f origin=gs::Vec2f(0.f,0.f), gs::Vec2f target=gs::Vec2f(1.f,1.f))
        {
            if(not isRunning()) return origin;

            float time_progress = m_timer/m_duration;

            gs::Vec2f progress{time_progress, time_progress}; //Evolution linéaire par défaut
            if(m_xEasingFunction)
                progress.x = m_xEasingFunction(time_progress); //Fontion personnalisée si présente
            if(m_yEasingFunction)
                progress.y = m_yEasingFunction(time_progress);

            gs::Vec2f target_vec = target-origin;

            //Décomposition de 'target_vec' en deux vecteurs orthogonaux qui serviront de base pour appliquer la progression.
            gs::Vec2f u, v;
            {
                //Implémentation des maths qu'il y a dérrière
                float r=gs::vecLen(target_vec);
                float a = r/1.2f; //Arbitraire : a doit être entre 0 et r.
                float b = std::sqrt(a*(r-a));

                //Base orthogonale adaptée
                gs::Vec2f e1 = gs::normalize(target_vec);
                gs::Vec2f e2(-e1.y, e1.x);

                u = e1*a + e2*b;
                v = e1*(r-a) - e2*b;
            }
            //Ainsi la trajectoire obtenue est toujours la même quelque soit la position de 'target' par rapport à 'origin'.

            return origin + u*progress.x + v*progress.y;
        }

        void update(float dt)
        {
            if(not isRunning()) return;

            if(m_timer > m_duration or m_duration <= 0.f) {m_is_running = false; return;}
            //Le "or m_duration <= 0.f" est là par sécurité, même si c'est déjà fait dans 'run()'.

            m_timer += dt;
        }

    protected:

        bool m_is_running=false;
        float m_timer=0.f; //Croissant, en sec
        float m_duration=1.f; //En sec

        //Si une fonction n'est pas mentionnée (==nullptr) une évolution linéaire est utilisée à la place de celle-ci.
        float (*m_xEasingFunction)(float time_progress)=nullptr;
        float (*m_yEasingFunction)(float time_progress)=nullptr;

};


//==============================================================================//















struct Trail_Vertices //pour le dessin
{
    std::vector<Vertex> vertices;
    std::vector<int> indices;
};


struct Trail_Node
{
    float timer=0.f; //croissant, en seconde
    float duration=1.f; //en seconde
    Vec2f anchor; //point d'ancrage
    Vec2f ortho_dir{0.f,0.f}; //direction orthogonale à la trajectoire
    float width=20.f; //Longueur maximale de la ligne formée par les points du noeud.

    float getTimeProgress() const {if(duration <= 0.f) return 0.f; return 1.f-GetMax(0.f, timer)/duration;}
    gs::Vec2f getPoint1() const {float p = getTimeProgress(); return anchor - ortho_dir*(width*p*p*p);}
    gs::Vec2f getPoint2() const {float p = getTimeProgress(); return anchor + ortho_dir*(width*p*p*p);}
};

class Trail //Une traînée
{
    //Voici le principe :
    //D'abord : chaque noeud a un point d'ancrage et deux points extérieurs positionnés de manière orthogonale à la trajectoire de la traînée.
    //Les trois points sont alignés, et les deux points extérieurs sont d'égale distance du point du milieu.
    //Le principe est que le deux points extrèmes vont se rapprocher de celui du mileu au fur et à mesure que le temps avance. Une fois les points suffisament proches, le noeud est retiré s'il est à l'extrémité le plus "agé" de la traînée.

    public:
        void newNode(Vec2f anchor, float width, float duration=1.f);
        void update(float dt);

		Trail_Vertices getVertices(Color color_0_255=Color(255,255,255,255), const Camera2D_Base& cam=Camera2D_Base()) const;
		bool draw(Renderer* ren, Texture* texture=nullptr, Color tint=Color(255,255,255,255), const Camera2D_Base& cam=Camera2D_Base());

        void setAddingDelay(float delay) {m_adding_dalay = GetMax(0.f, delay);}
        void reset() {m_nodes.clear();}

    protected:
        std::vector<Trail_Node> m_nodes;
        float m_adding_dalay=0.008f; //délai minimum entre deux ajouts de noeuds. En secondes
        float m_adding_timer=0.f; //à rebours - en secondes
};





}//namespace gs

#endif //_GSlib_WIDGET_HPP_
