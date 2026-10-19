#include "GSlib-Attack.hpp"


namespace gs
{

std::vector<Att_Target> mkTargets(const std::vector<int>& targets_int_id)
{
	std::vector<Att_Target> att_targets;

	for(int id : targets_int_id)
	{
		Att_Target trgt;
		trgt.id = id;
		trgt.use_str_id_instead = false;
		trgt.time_count = 0.f;
		att_targets.push_back(trgt);
	}

	return att_targets;
}

std::vector<Att_Target> mkTargets(const std::vector<std::string>& targets_str_id)
{
	std::vector<Att_Target> att_targets;

	for(const std::string& id : targets_str_id)
	{
		Att_Target trgt;
		trgt.str_id = id;
		trgt.use_str_id_instead = true;
		trgt.time_count = 0.f;
		att_targets.push_back(trgt);
	}

	return att_targets;
}











//==================================================================================//




bool Attack::setHitboxGetter(gs::Circle_Hitbox (*getCircleHitbox)(const gs::Att_Target& target, bool& is_ok))
{
	if(not getCircleHitbox) return false;
	mm_getCircleHitbox = getCircleHitbox;
	return true;
}

bool Attack::setAction(void (*action)(const gs::When_Touched__Args& args))
{
	if(not action) return false;

	mm_action = action;
	return true;
}

bool Attack::setEasingFunc(Attack_Stage stage, float (*progressionEasingFunc)(float progession))
{
	if(progressionEasingFunc == nullptr)
		return false;

	switch (stage) {
		case ATTACK_FADE_IN: mm_progressionEasingFunc_fadein = progressionEasingFunc; break;
		case ATTACK_STABLE_MOMENT: mm_progressionEasingFunc_stable = progressionEasingFunc; break;
		case ATTACK_FADE_OUT: mm_progressionEasingFunc_fadeout = progressionEasingFunc; break;
		default: return false;
	}

	return true;
}


float Attack::getGlobalTimeProgression(bool* is_ok) const //Sur toute la durée de l'attaque
{
	float total_duration=0.f;
	total_duration += getDuration(ATTACK_FADE_IN);
	total_duration += getDuration(ATTACK_STABLE_MOMENT);
	total_duration += getDuration(ATTACK_FADE_OUT);

	if(total_duration <= 0.f)
	{
		if(is_ok) *is_ok = false;
		return 0.f;
	}

	float total_timer=0.f;
	if(m_stage == ATTACK_FADE_IN)
		total_timer = m_timer;
	else
	if(m_stage == ATTACK_STABLE_MOMENT)
		total_timer = getDuration(ATTACK_FADE_IN) + m_timer;
	else
	if(m_stage == ATTACK_FADE_OUT)
		total_timer = getDuration(ATTACK_FADE_IN) + getDuration(ATTACK_STABLE_MOMENT) + m_timer;

	if(is_ok) *is_ok = true;
	return total_timer/total_duration;
}

float Attack::getCurrStageTimeProgression(bool* is_ok) const //Relativement à l'étape courante.
{
	float duration = getDuration(m_stage);

	if(duration <= 0.f)
	{
		if(is_ok) *is_ok = false;
		return 0.f;
	}

	float progression = m_timer/duration; //Progession basique dans le temps, sur l'étape courante.
	if(progression > 1.f) progression = 1.f; else
	if(progression < 0.f) progression = 0.f;

	if(is_ok) *is_ok = true;
	return progression;

}

float Attack::getFieldProgression() const
{
	bool is_ok;
	float progression = getCurrStageTimeProgression(&is_ok); //Progession basique dans le temps, sur l'étape courante.
	if(not is_ok) return 0.f;


	if(m_stage == ATTACK_FADE_IN)
	{
		if(mm_progressionEasingFunc_fadein)
			return GetMin(1.f, GetMax(mm_progressionEasingFunc_fadein(progression), 0.f));
		return progression;
	}
	if(m_stage == ATTACK_STABLE_MOMENT)
	{
		if(mm_progressionEasingFunc_stable)
			return GetMin(1.f, GetMax(mm_progressionEasingFunc_stable(progression), 0.f));
		return 1.f;
	}
	if(m_stage == ATTACK_FADE_OUT)
	{
		if(mm_progressionEasingFunc_fadeout)
			return GetMin(1.f, GetMax(1.f-mm_progressionEasingFunc_fadeout(progression), 0.f));
		return 1.f-progression;
	}
	//Les 'mm_progressionEasingFunc_' doivent renvoyer (en particulier) un résultat entre 0 et 1.

	return 0.f;
}


void Attack::setDuration(Attack_Stage stage, float duration)
{
	duration = GetMax(0.f, duration);
	switch(stage) {
		case ATTACK_FADE_IN:  if(m_stage != ATTACK_FADE_IN or not m_is_going) m_fade_in_duration = duration;  break;
		case ATTACK_STABLE_MOMENT:  if(m_stage != ATTACK_STABLE_MOMENT) m_stable_duration = duration; break;
		case ATTACK_FADE_OUT:  if(m_stage != ATTACK_FADE_OUT) m_fade_out_duration = duration; break;
		default: break;
	}
}

float Attack::getDuration(Attack_Stage stage) const
{
	switch(stage) {
		case ATTACK_FADE_IN: return m_fade_in_duration;
		case ATTACK_STABLE_MOMENT: return m_stable_duration;
		case ATTACK_FADE_OUT: return m_fade_out_duration;
		default: return 0.f;
	}
}


void Attack::stop(bool skip_fadeout)
{
	if(not m_is_going) return;

	if(skip_fadeout)
	{
		m_is_going = false;
		return;
	}

	Attack_Stage curr_stage = m_stage;
	m_stage = ATTACK_FADE_OUT;

	if(curr_stage == ATTACK_FADE_IN)
	{
		float duration = m_fade_in_duration;
		if(duration <= 0.f){
			m_is_going = false;
			return;
		}

		//On commence le fade-out là où le fade-in se trouvait.
		float progession = m_timer/duration;
		m_timer = m_fade_out_duration*(1.f-progession); //on prend '1-progression' au lieu de 'progession' seul car le fade-out (ici) se dirige dans un sens opposé au sens du fade-in.
	}
	else
	if(curr_stage == ATTACK_STABLE_MOMENT)
		m_timer = 0.f;

}




//==================================================================================//





bool Basic_Attack::attack(gs::Vec2f anchor, float dir, float opening_angle, std::vector<gs::Att_Target> targets)
{
	if(isGoing()) return false; //Si une attaque est déjà en cours.

	//Vérification de la validité.
	if(opening_angle <= 0.f) return false;
	if(opening_angle > 2.f*_pi_) opening_angle = 2.f*_pi_;

	m_anchor = anchor;
	m_timer = 0.f;
	m_dir = dir;
	m_opening_angle = opening_angle;
	m_touched_ones.clear();
	m_targets.clear();
	m_targets = targets;

	std::string error_info;
	if(not launch(error_info))
	{
		std::cout << "!! in 'Basic_Attack::attack()' : " << error_info << "\n";
		return false;
	}

	return true;
}


void Basic_Attack::process(float dt)
{
	if(not isGoing()) return;

	float duration = getCurrDuration();

	if(m_timer >= duration) //Si le durée de la phase courante est accomplie.
	{
		bool finished = goToNextStage(); //Cette fonction modifie 'm_timer' btw.
		if(finished) return;
	}

	if(not m_one_touch_only)
        m_touched_ones.clear(); //Si ce n'est pas en mode "une seule touche par cible", alors l'état 'touché' des cibles s'actualise à chaque appel, d'où le 'clear()'.

	std::vector<Att_Target>::iterator it=m_targets.end();

    //Parcours (à rebours) des cibles (pour pouvoir retirer sans perturber le parcours).
    //for(int i=int(m_targets.size())-1; i>=0; --i)
    while(it != m_targets.begin())
    {
		it--;
		Att_Target& target = *it;

		bool is_ok=true;
		gs::Circle_Hitbox circ_hitbox = mm_getCircleHitbox(target, is_ok);
		if(!is_ok) continue;

		float field_radius = getFieldProgression()*m_dist;
		float origin_proximity=0.f; //va contenir la distance entre la cible et l'origine de l'attaque
		gs::Vec2f target_center = circ_hitbox.center;
		float target_radius = circ_hitbox.radius;
		gs::Vec2f closest_pt;

		if(gs::isInField(m_anchor, m_opening_angle, m_dir, field_radius, target_center, target_radius, &closest_pt, &origin_proximity))
		{
			{//Conversion de la proximité : de la proximité (de la cible, d'avec l'origine) par rapport à la progression ponctuelle de la zone, à la proximité (de la cible, d'avec l'origine) par rapport à la zone complète.
				origin_proximity = ((origin_proximity-1)*field_radius)/m_dist +1;

				//'origin_proximity' est entre 0 (loin) et 1 (proche).
			}

			m_touched_ones.insert(m_touched_ones.end(), target);
			action(target, m_anchor, origin_proximity, closest_pt, dt);

			if(m_one_touch_only)
				m_targets.erase(it);
			else
				target.time_count += dt; //En effet ce timer ne concerne pas le mode à contacte unique.
		}
		else
		if(not m_one_touch_only)
			target.time_count = 0;
	}

	m_timer += dt;
}


void Basic_Attack::show(gs::Renderer* ren)
{
	if(not isGoing()) return;

	int radius = getFieldProgression() * m_dist;

	gs::Vec2f pos = (m_anchor);//. template to<int>());

	gs::drawCircle(ren, pos, radius, {250,250,250,50}, true, m_opening_angle, m_dir-m_opening_angle/2.0);
}


}//namespace gs
