#ifndef _GSlib_WIDGET_HPP_
#define _GSlib_WIDGET_HPP_

#include "GSlib.hpp"
#include "GSlib-Text.hpp"
#include <unordered_set>

namespace gs
{

class Button;

//Pour les boutons et scrollbars:
	bool initWidgets(Renderer* ren, std::string font_path);
	//void widgetsProcessEvent(...); -> selon le backend utilisé
	void updateWidgets();
	bool setButtonText(Button& btn, std::string witch, const char* text, Color text_color, bool resize_area_w);




//==============================================================================//
//==============================================================================//
//==============================================================================//






namespace wi
{
	enum Button_State {IDLE, ACTIVE, MOUSE_IN, CLICKED_IN};
	enum Button_Field {AREA, IMG, TEXT};
	enum Action_Val {DOWN_CLICK=BK_EVENT_MOUSE_BUTTON_DOWN, UP_CLICK=BK_EVENT_MOUSE_BUTTON_UP};
	enum Quick_Btns_Field {CHECK_AREA, BUTTONS};

}

class Button
{
	public :
		Button();
		~Button();

		bool init(Renderer* ren, FRect area, const char* img_file_path=nullptr);

		bool loadTexture(Renderer* ren, const char* img_file_path);
		bool setTexture(Texture* texture);

		bool show(Renderer* ren);

		bool setShow(wi::Button_Field field, bool new_state);

		void setArea(FRect new_area);

		void incrArea(short x, short y, short w=0, short h=0);

		void setActiveState(bool new_state);

		void incrOffset(wi::Button_Field field, short x, short y);

		void setDestSize(wi::Button_Field field, short w, short h);

		void setColor(wi::Button_State target_state, FColor c);

		void setTargetEvent(uint32_t target_event);


		void setTargetButtonEvent(uint8_t id);

		bool setCallbackOfTargetEvent(int (*callback)(void*), void* arg);

		friend std::string INTERNAL_USE_switchText(Button& btn, std::string to_witch);

		bool switchText(std::string to_witch="_"); //Avec 'to_witch' != 'active' et de 'idle', ça switch sur le texte qui n'est pas actuellement appliqué.


		bool setSwitchTextByState(bool new_state);

		friend void INTERNAL_USE_editState(Button& btn, wi::Button_State state, bool new_state);
		friend bool setButtonText(Button& btn, std::string witch, const char* text, Color text_color, bool resize_area_w);





		bool isInit() const;

		Texture* getTexture();

		bool getShow(wi::Button_Field field);

		FRect getArea() const;

		bool getActiveState() const;

		Vec2i getOffset(wi::Button_Field field) const;

		Vec2i getDestSize(wi::Button_Field field) const;

		FColor getColor(wi::Button_State target_state) const;

		bool mouseIn() const;

		bool clickedIn() const;

		uint32_t getTargetEvent();

		uint8_t getTargetButtonEvent();

		int getLastCallbackReturnVal();

		bool getSwitchTextByState();





	protected:
		bool 			m_is_init=false;
		FRect 			m_area={0,0,0,0}; //m_area.h est aussi utilisée pour comme ptsize

		bool 			m_active_state=false;

		bool 			m_mouse_in=false;
		bool 			m_clicked_in=false;

		bool  			m_show_text=false;
		bool 			m_show_img=false;
		bool 			m_show_area=true; // Avec une les couleurs ci-dessous selon l'état courant.

		uint32_t  		m_target_action=wi::DOWN_CLICK;
		uint8_t  		m_target_button_id=BK_MOUSE_BUTTON_LEFT;

		int 			(*m_callback)(void*) = nullptr;
		int 			m_last_callback_return_val=0;
		void*  			m_callback_arg=nullptr;


		Texture* 	m_texture=nullptr;
		Vec2i  			m_texture_offset={0,0};
		Vec2i       	m_texture_dest_size;

		Texture*  	m_text_texture=nullptr;
		uint16_t 			m_button_id=0;
		bool  			m_switch_text_by_state=false;
		Vec2i  			m_text_offset={0,0};
		Vec2i       	m_text_dest_size;

		FColor  	m_idle_color={240.f,10.f,10.f,255.f};
		FColor  	m_active_color={0.f,240.f,0.f,255.f};
		FColor 	    m_mouse_in_color={100.f,100.f,100.f,70.f};
		FColor  	m_clicked_in_color={255.f,255.f,40.f,205.f};

};

//==============================================================================//












namespace wi
{
	enum Scrollbar_Orient {VERTICAL, HORIZONTAL};
	enum Scrollbar_Area {MAIN_AREA, MOVEABLE_THING_AREA};
};

class Scrollbar : public Backend__Scrollbar
{
	public :
		Scrollbar();
		~Scrollbar();

		bool show(Renderer* ren);
		void update(); //Note: 'gs::updateWidgets()' en fait appel, il n'est donc pas nécessaire de l'appeler séparément si 'gs::updateWidgets()' est utilisée. 

		//void processEvent(...); selon le backend, sinon, 'update()' suffit. De plus, avec le backend où c'est disponible, 'gs::widgetsProcessEvent()' en fait appel, il n'est donc pas nécessaire de l'appeler séparément si 'gs::widgetsProcessEvent()' est utilisée.

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

		FColor setColor(const wi::Scrollbar_Area& area, const FColor& color);
		FColor getColor(const wi::Scrollbar_Area& area) const;

		Rect getArea(const wi::Scrollbar_Area& area=wi::MAIN_AREA) const;



	protected :

		void checkParam();
		void incrProgr(int val); //Incrémenter la progression.
		void setAreaRects(Rect& area, Rect& moveable_thing_area) const override;

		wi::Scrollbar_Orient 	m_orient=wi::VERTICAL; //Orientation.
		bool  					m_mouse_clicked_in=false;
		short 					m_to_move_buf=0; //Valeur qui fait incrémenter la position de l'objet amovible lors de la mise à jour.
		bool 					m_moved=false; //Témoin du movement à la dernière mise à jour.

		unsigned int 			m_max_val=100; //Valeur maximale obtenable (peut être changée au besoin).
		unsigned int  			m_pixel_progr=0; //Progression en pixel de l'élément amovible, qui permet de déduire la valeur courante.
		unsigned int 			m_moveable_thing_len=30;
		// unsigned int  			m_no_need_scroll_count=50;

		Vec2i 					m_pos={10, 10};
		unsigned int 			m_len=300; //Longueur de la scrollbar (en pixel).
		unsigned int 			m_thickness=15; //Epaisseur de la scrollbar (en pixel).
		unsigned char 			m_min_len=10; //Longueur minimale.
		unsigned char 			m_min_thickness=4; //Epaisseur minimale.

		FColor 				m_bg_col={170.f,170.f,170.f,255.f}; //-> de la zone de scroll.
		FColor 				m_fg_col={240.f,240.f,240.f,255.f}; //-> de l'élément amovible.

	private:
		
		void INTERNAL_USE_getMembersPtrs_forEventProcessing(INTERNAL__membersTransfer& ptrs) override
		{
			ptrs.m_mouse_clicked_in = &m_mouse_clicked_in;
            ptrs.m_orient= (void*)(&m_orient);
            ptrs.m_to_move_buf = &m_to_move_buf;
		}

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
