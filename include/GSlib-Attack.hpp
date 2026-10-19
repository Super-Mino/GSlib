#ifndef _GSlib_ATT_HPP_
#define _GSlib_ATT_HPP_

#include "GSlib-Geo.hpp"


namespace gs
{

//Les différents moments/étapes d'une attaque
enum Attack_Stage{ATTACK_FADE_IN, ATTACK_STABLE_MOMENT, ATTACK_FADE_OUT};


struct Att_Target //Représente une cible possible pour une attaque.
{
	float time_count=0.f; //Chronomètre le temps de contacte pour le mode non "contacte unique".
	int id=0; //identifiant de la cible
	std::string str_id;
	bool use_str_id_instead=false;
	//Je laisse le choix entre seulement deux types d'identifiants pour éviter d'utiliser des templates.
};


std::vector<Att_Target> mkTargets(const std::vector<int>& targets_int_id);
std::vector<Att_Target> mkTargets(const std::vector<std::string>& targets_str_id);


struct Att_User_Data //Pour laisser la possibilité à l'utilisateur de donner quelques informations supplémentaires à une attaque.
{
	int i=0;
	float f=0.f;
	bool b=false;
	//std::string str;
	void* ptr=nullptr;
};


struct When_Touched__Args //Les arguments passés au callback d'action lorsque qu'une cible est touchée.
{
	Att_Target target;
	float global_time_progress=0.f; //Sur toute la durée de l'attaque
	float stage_time_progress=0.f; //Seulement sur l'étape courante
	float field_progress=0.f; //Evolution courante de la zone de l'attaque
	Vec2f att_origin; //Anchor courant de l'attaque
	float origin_proximity=0.f; //'origin_proximity' est entre 0 (loin) et 1 (proche).
	gs::Vec2f contact_point;
	float dt=1.f;
	const Att_User_Data* user_data_ptr=nullptr;
};

class Attack //Base des attaques.
{
	public:

		bool setHitboxGetter(gs::Circle_Hitbox (*getCircleHitbox)(const gs::Att_Target& target, bool& is_ok));

		bool setAction(void (*action)(const gs::When_Touched__Args& args));

		bool setEasingFunc(Attack_Stage stage, float (*progressionEasingFunc)(float progession));

		void setUserData(const Att_User_Data& ud) {m_user_data = ud;}
		void setIntUserData(int val) {m_user_data.i = val;}
		void setFloatUserData(float val) {m_user_data.f = val;}
		void setBoolUserData(float val) {m_user_data.b = val;}
		void setPtrUserData(void* ptr) {m_user_data.ptr = ptr;}

		const Att_User_Data& getUserData() const {return m_user_data;}

		float getGlobalTimeProgression(bool* is_ok=nullptr) const; //Sur toute la durée de l'attaque

		float getCurrStageTimeProgression(bool* is_ok=nullptr) const; //Relativement à l'étape courante.

		float getFieldProgression() const; //L'évolution courante du champ de l'attaque

		bool isGoing() const {return m_is_going and mm_getCircleHitbox != nullptr;}

		void setDuration(Attack_Stage stage, float duration);

		float getDuration(Attack_Stage stage) const;

		float getCurrDuration() const {return getDuration(m_stage);}

		Attack_Stage getStage() const {return m_stage;}

		void setAnchor(gs::Vec2f pos) {m_anchor = pos;}

		void stop(bool skip_fadeout=false);

		void setOneTouchOnly(bool state) {m_one_touch_only = state;}

		virtual bool isInit() const {return mm_getCircleHitbox != nullptr;}

		float getDir() const {return m_dir;}

	private :
		bool m_is_going = false;
		Attack_Stage m_stage=ATTACK_FADE_IN;


	protected :

		bool launch(std::string& error_info)
		{
			if(m_is_going)
			{
				error_info = "already launched.";
				return false;
			}
			if(not mm_getCircleHitbox)
			{
				error_info = "missing a hitbox getter : use 'setHitboxGetter(valid function pointer)' before.";
				return false;
			}

			m_is_going = true;
			m_stage = ATTACK_FADE_IN;
			return true;
		}


		gs::Vec2f m_anchor; //Point de départ (d'ancrage) de l'attaque.
		float m_dir=0.f; //Orientation de l'attaque, etc. En radian.
		Att_User_Data m_user_data; //Pour laisser la possibilité à l'utilisateur de donner quelques informations supplémentaires à une attaque.

		bool m_one_touch_only=true; //Pour les cibles.
		std::vector<Att_Target> m_touched_ones;
		std::vector<Att_Target> m_targets;

		float m_fade_in_duration=0.5f; //Durée de la phase d'entrée de l'attaque, en seconde..
		float m_stable_duration=5.f;//Durée de la phase "statble" de l'attaque, en seconde.
		float m_fade_out_duration=0.5f; //Durée de la phase de sortie de l'attaque, en seconde..
		float m_timer=0.f; //Chronomètre, de la phase courante, en seconde.

		gs::Circle_Hitbox (*mm_getCircleHitbox)(const Att_Target& target, bool& is_ok)=nullptr;
		void (*mm_action)(const gs::When_Touched__Args& args)=nullptr; //La fonction appelée lorsque une cible est touchée. 'origin_proximity' est entre 0 (loin) et 1 (proche).
		float (*mm_progressionEasingFunc_fadein)(float progession)=nullptr;
		float (*mm_progressionEasingFunc_stable)(float progession)=nullptr;
		float (*mm_progressionEasingFunc_fadeout)(float progession)=nullptr;

		void action(Att_Target& target, gs::Vec2f att_origin, float origin_proximity, gs::Vec2f contact_point, float dt)
		{
			if(mm_action)
			{
				When_Touched__Args args;
				args.target = target;
				args.global_time_progress = getGlobalTimeProgression();
				args.stage_time_progress = getCurrStageTimeProgression();
				args.field_progress = getFieldProgression();
				args.att_origin = att_origin;
				args.origin_proximity = origin_proximity;
				args.contact_point = contact_point;
				args.dt = dt;
				args.user_data_ptr = &m_user_data;

				mm_action(args);
			}
		}


		bool goToNextStage()
		{
			bool finished=false;
			m_timer = 0.f;

			switch(m_stage) //On procède en fonction de l'étape courante.
			{
				case ATTACK_FADE_IN: m_stage = ATTACK_STABLE_MOMENT; break;

				case ATTACK_STABLE_MOMENT: m_stage = ATTACK_FADE_OUT; break;

				case ATTACK_FADE_OUT:
					m_is_going = false;
					m_touched_ones.clear();
					m_targets.clear();
					finished = true; //important -> car sert aussi à assurer la fin de la récursion
					break;

				default: m_stage = ATTACK_FADE_IN; break;
			}

			//Pour le cas où il y a des étapes qui sont inexistantes.
			if(not finished)
			{
				if(getDuration(m_stage) <= 0.f)
					goToNextStage();
			}

			return finished;
		}

};


class Basic_Attack : public Attack //Attaque progressif (ou statique) sur un rayon.
{
	public :

		void setDist(float dist) {m_dist = GetMax(0.02f, std::abs(dist));}
		virtual bool attack(gs::Vec2f anchor, float dir, float opening_angle, std::vector<Att_Target> targets);
		virtual void process(float dt);
	    virtual void show(gs::Renderer* ren);
		float getDist() const {return m_dist;}
		float getOpeningAngle() const {return m_opening_angle;}

	protected :

		float m_dist=100.f; //Etendu de la propagation, en pixel.
		float m_opening_angle=_pi_/3.0; //Angle d'ouverture de la propagation, en radian.

};




}//namespace gs

#endif //_GSlib_ATT_HPP_
