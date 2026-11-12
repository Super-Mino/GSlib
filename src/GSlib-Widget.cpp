#include "GSlib-Widget.hpp"
#include "GSlib-Geo.hpp"
#include "GSlib-Text.hpp"
#include <map>


namespace gs
{

struct Group
{
	bool is_active=true; //Un groupe désactivé n'est pas affiché et ne reçoit pas les événements (ni de mise à jour).

	//Les identifiants des boutons de ce groupe
	std::unordered_set<int> int_btns; //objectif : contenir des valeurs issues des clés de 'SBIM::m_int_ID_btns'
	std::unordered_set<std::string> str_btns; //objectif : contenir des valeurs issues des clés de 'SBIM::m_str_ID_btns'

	//Les identifiants des scrollbars de ce groupe
	std::unordered_set<int> int_scbrs; //objectif : contenir des valeurs issues des clés de 'SBIM::m_int_ID_scbrs'
	std::unordered_set<std::string> str_scbrs; //objectif : contenir des valeurs issues des clés de 'SBIM::m_str_ID_scbrs'


	bool haveBtn(int id) const {return int_btns.contains(id);}
	bool haveBtn(const std::string& id) const {return str_btns.contains(id);}
	void addBtn(int id) {int_btns.insert(id);}
	void addBtn(const std::string& id) {str_btns.insert(id);}
	void removeBtn(int id) {int_btns.erase(id);}
	void removeBtn(const std::string& id) {str_btns.erase(id);}

	bool haveScbr(int id) const {return int_scbrs.contains(id);}
	bool haveScbr(const std::string& id) const {return str_scbrs.contains(id);}
	void addScbr(int id) {int_scbrs.insert(id);}
	void addScbr(const std::string& id) {str_scbrs.insert(id);}
	void removeScbr(int id) {int_scbrs.erase(id);}
	void removeScbr(const std::string& id) {str_scbrs.erase(id);}

};

struct SBIM // "Scrollbars and Buttons Internal Management" 
{
	//----------------------------------------------------------
		//Les boutons (avec deux types d'identifiants autorisés)
		std::map<int, Button> m_int_ID_btns;
		std::map<std::string, Button> m_str_ID_btns;

		//Les boutons (avec deux types d'identifiants autorisés)
		std::map<int, Scrollbar> m_int_ID_scbrs;
		std::map<std::string, Scrollbar> m_str_ID_scbrs;


		//Les boutons et les scrollbars seront rangés dans des groupes.
		std::unordered_map<int, Group> m_int_ID_groups;
		std::unordered_map<std::string, Group> m_str_ID_groups;
		bool m_curr_target_group_type_id_is_int = true;
		int m_curr_target_group_id__i = 0;
		std::string m_curr_target_group_id__s;

		bool m_btn_id_0_warning=true; //Pour la sécurité
		bool m_scbr_id_0_warning=true;

		Text_Line_Self_Contained m_tlsc; //Pour l'affichage des textes.
		std::string m_btns_font_path="C:/Windows/Fonts/consola.ttf"; 

	//----------------------------------------------------------

	struct R
	{
		std::map<int, Button>::iterator int_btn_it;
		std::map<std::string, Button>::iterator str_btn_it;

		std::map<int, Scrollbar>::iterator int_scbr_it;
		std::map<std::string, Scrollbar>::iterator str_scbr_it;
	};

	bool containsBtn(int id, R& res)
	{
		res.int_btn_it = m_int_ID_btns.find(id);
		return res.int_btn_it != m_int_ID_btns.end();
	}

	bool containsBtn(const std::string& id, R& res)
	{
		res.str_btn_it = m_str_ID_btns.find(id);
		return res.str_btn_it != m_str_ID_btns.end();
	}

	template<typename ID_t>
	void addInGroup(ID_t id, Button& new_one)
	{
		if(m_curr_target_group_type_id_is_int)
		{
			m_int_ID_groups[m_curr_target_group_id__i].addBtn(id);
			new_one.group_id_i = m_curr_target_group_id__i;
		}
		else
		{
			m_str_ID_groups[m_curr_target_group_id__s].addBtn(id);
			new_one.group_id_s = m_curr_target_group_id__s;
		}

		new_one.group_id_is_int = m_curr_target_group_type_id_is_int;
	}



	//--------------------------------------------//

	bool containsScbr(int id, R& res)
	{
		res.int_scbr_it = m_int_ID_scbrs.find(id);
		return res.int_scbr_it != m_int_ID_scbrs.end();
	}

	bool containsScbr(const std::string& id, R& res)
	{
		res.str_scbr_it = m_str_ID_scbrs.find(id);
		return res.str_scbr_it != m_str_ID_scbrs.end();
	}

	template<typename ID_t>
	void addInGroup(ID_t id, Scrollbar& new_one)
	{
		if(m_curr_target_group_type_id_is_int)
		{
			m_int_ID_groups[m_curr_target_group_id__i].addScbr(id);
			new_one.group_id_i = m_curr_target_group_id__i;
		}
		else
		{
			m_str_ID_groups[m_curr_target_group_id__s].addScbr(id);
			new_one.group_id_s = m_curr_target_group_id__s;
		}

		new_one.group_id_is_int = m_curr_target_group_type_id_is_int;
	}

	//--------------------------------------------//
	
	void initTheTLSC(Renderer* ren)
	{
		#ifdef GSLIB_RAYLIB_BACKEND
		return; //Car sur raylib ça ne fonctionnne pas correctement au moment où j'écris ce commentaire.
		#endif //GSLIB_RAYLIB_BACKEND

		if(not m_tlsc.isInit_TLSC())
			m_tlsc.init_TLSC(ren, m_btns_font_path, 28, 0xE0E0E0FFu);
	}
	
	void showBtnText(Renderer* ren, const std::string& text, FRect allowed_area)
	{
		#ifdef GSLIB_RAYLIB_BACKEND
		return; //Car sur raylib ça ne fonctionnne pas correctement au moment où j'écris ce commentaire.
		#endif //GSLIB_RAYLIB_BACKEND

		if(not m_tlsc.isInit_TLSC()) return;
		
		m_tlsc.setMaxSize(Vec2u(allowed_area.w, allowed_area.h));
		m_tlsc.setText(text);
		Vec2u line_size = m_tlsc.update();

		//Centrage du text dans la zone:
		Vec2i area_pos(allowed_area.x, allowed_area.y);
		Vec2i line_pos(area_pos.x+allowed_area.w/2 -line_size.x/2,
				area_pos.y+allowed_area.h/2 -line_size.y/2);

		m_tlsc.setPos(line_pos);
		m_tlsc.show(ren);
	}
};
SBIM _sbim_;


Button& btn(int id, bool* is_ok)
{
	if(is_ok) *is_ok=true;
	SBIM::R res;
	if(_sbim_.containsBtn(id, res))
		return res.int_btn_it->second;

	if(is_ok) *is_ok=false;
	gs::lastError("||Warning|| in 'btn(int)', button id [value ",gs::str(id)+"] not found.");
	static Button default_btn;
	return default_btn;
}

Button& btn(const std::string& id, bool* is_ok)
{
	if(is_ok) *is_ok=true;
	SBIM::R res;
	if(_sbim_.containsBtn(id, res))
		return res.str_btn_it->second;

	if(is_ok) *is_ok=false;
	gs::lastError("||Warning|| in 'btn(str)', button id [value ",id+"] not found.");
	static Button default_btn;
	return default_btn;
}

Scrollbar& scbr(int id, bool* is_ok)
{
	if(is_ok) *is_ok=true;
	SBIM::R res;
	if(_sbim_.containsScbr(id, res))
		return res.int_scbr_it->second;

	if(is_ok) *is_ok=false;
	gs::lastError("||Warning|| in 'scbr(int)', scrollbar id [value ",gs::str(id)+"] not found.");
	static Scrollbar default_scbr;
	return default_scbr;
}

Scrollbar& scbr(const std::string& id, bool* is_ok)
{
	if(is_ok) *is_ok=true;
	SBIM::R res;
	if(_sbim_.containsScbr(id, res))
		return res.str_scbr_it->second;

	if(is_ok) *is_ok=false;
	gs::lastError("||Warning|| in 'scbr(str)', scrollbar id [value ",id+"] not found.");
	static Scrollbar default_scbr;
	return default_scbr;
}



void turnOff0IDBtnWarning(){_sbim_.m_btn_id_0_warning = false;}
void turnOn0IDBtnWarning(){_sbim_.m_btn_id_0_warning = true;}
void turnOff0IDScbrWarning(){_sbim_.m_scbr_id_0_warning = false;}
void turnOn0IDScbrWarning(){_sbim_.m_scbr_id_0_warning = true;}

void setNextScrollbarsAndBtnsTargetGroup(int group_id)
{
	_sbim_.m_curr_target_group_type_id_is_int = true;
	_sbim_.m_curr_target_group_id__i = group_id;
}

void setNextScrollbarsAndBtnsTargetGroup(const std::string& group_id)
{
	_sbim_.m_curr_target_group_type_id_is_int = false;
	_sbim_.m_curr_target_group_id__s = group_id;
}


Group& INTERNAL_USE_getGroup(int group_id, bool& is_ok)
{
	is_ok = true;
	std::unordered_map<int, Group>::iterator it = _sbim_.m_int_ID_groups.find(group_id);
	if(it != _sbim_.m_int_ID_groups.end())
		return it->second;

	is_ok = false;
	static Group default_group;
	return default_group;
}

Group& INTERNAL_USE_getGroup(const std::string& group_id, bool& is_ok)
{
	is_ok = true;
	std::unordered_map<std::string, Group>::iterator it = _sbim_.m_str_ID_groups.find(group_id);
	if(it != _sbim_.m_str_ID_groups.end())
		return it->second;

	is_ok = false;
	static Group default_group;
	return default_group;
}


void setScrollbarsAndBtnsGroupActiveState(int group_id, bool active)
{
	bool is_ok; INTERNAL_USE_getGroup(group_id, is_ok).is_active = active;
}

void setScrollbarsAndBtnsGroupActiveState(const std::string& group_id, bool active)
{
	bool is_ok; INTERNAL_USE_getGroup(group_id, is_ok).is_active = active;
}


bool getScrollbarsAndBtnsGroupActiveState(int group_id)
{
	bool is_ok, is_active = INTERNAL_USE_getGroup(group_id, is_ok).is_active;
	return is_ok and is_active;
}

bool getScrollbarsAndBtnsGroupActiveState(const std::string& group_id)
{
	bool is_ok, is_active = INTERNAL_USE_getGroup(group_id, is_ok).is_active;
	return is_ok and is_active;
}





bool newBtn(int id, Vec2f pos, const Btn_Param& param)
{
	bool skip_presence_checking = false;

	if(id == wi::BTN_NONE_ID) //Cas particulier
	{
		//Rajout d'un bouton avec la première clé libre trouvée.
		
		//Recherche d'un trou dans les valeurs des clés
		bool is_first_loop = true;
		int prev_id=wi::BTN_NONE_ID; //important d'initialiser à cette valeur.
		bool found=false;
		for(const auto& [curr_id, btn] : _sbim_.m_int_ID_btns)
		{
			if(is_first_loop) //Pour comparer deux clés successives..., il faut deux clés. Donc, on commence la comparaison à partir du deuxième tour.
			{
				prev_id = curr_id;
				is_first_loop = false;
				continue;
			}

			//Les clés sont des entiers, et le conteneur utilisé est 'std::map' (afin d'avoir les clés dans l'ordre).
			//Par conséquent, si l'on trouve deux clés successives distantes de plus d'un, alors on a trouvé un trou.
			int dist = std::abs(curr_id-prev_id); //Le 'std::abs()' n'est pas nécessaire ici, mais je le laisse.
			
			if(dist > 1)
			{
				id = prev_id+1; //récupération de l'ID normalement libre.
				found = true;
				break;
			}

			//Sinon on poursuit
			prev_id = curr_id;
		}

		if(not found) //Si aucun trou n'a été trouvé
			id = prev_id+1;

		skip_presence_checking = true;

		if(_sbim_.m_btn_id_0_warning)
			gs::lastError("||Warning|| in 'newBtn(int)', called with 'wi::BTN_NONE_ID' value [aka ",gs::str(int(wi::BTN_NONE_ID))+"], so a free id will be used instead [free found id : "+gs::str(id)+"]. If it's intentional (using this feature), you can turn off this warning with 'turnOff0IDBtnWarning()'.");
		
	}

	SBIM::R res;
	if(not skip_presence_checking and _sbim_.containsBtn(id, res))
	{
		gs::lastError("||Warning|| in 'newBtn(int)', button id already used. [id = ",gs::str(id)+"]");
		return false;
	}


	auto r = _sbim_.m_int_ID_btns.emplace(id, Button());
	
	if(not r.second)
		gs::lastError("||Warning|| in 'newBtn(int)', couldn't add button. [id = ",gs::str(id)+"]");
	else
	{
		bool is_ok = r.first->second.init(pos, param);
		
		if(not is_ok)
		{
			_sbim_.m_int_ID_btns.erase(id);

			gs::lastError("||Warning|| in 'newBtn(int)', couldn't init button, so addition is rejected. [id = ",gs::str(id)+"] : "+gs::lastError());
			return false;
		}

		_sbim_.addInGroup(id, r.first->second); //Une fois que tout est OK, on met le bouton dans le groupe d'accueil actif.
	}

	return r.second;
}


bool newBtn(const std::string& id, Vec2f pos, const Btn_Param& param)
{
	if(id.length() == 0)
	{
		gs::lastError("||Warning|| in 'newBtn(str)', button id must not be an empty string.");
		return false;
	}

	SBIM::R res;
	if(_sbim_.containsBtn(id, res))
	{
		gs::lastError("||Warning|| in 'newBtn(str)', button id already used. [id = ",id+"]");
		return false;
	}


	auto r = _sbim_.m_str_ID_btns.emplace(id, Button());
	
	if(not r.second)
		gs::lastError("||Warning|| in 'newBtn(str)', couldn't add button. [id = ",id+"]");
	else
	{
		bool is_ok = r.first->second.init(pos, param);
		
		if(not is_ok)
		{
			_sbim_.m_str_ID_btns.erase(id);

			gs::lastError("||Warning|| in 'newBtn(str)', couldn't init button, so addition is rejected. [id = ",id+"] : "+gs::lastError());
			return false;
		}

		_sbim_.addInGroup(id, r.first->second); //Une fois que tout est OK, on met le bouton dans le groupe d'accueil actif.
	}

	return r.second;
}


bool newBtn(int id, FRect area, Btn_Param param)
{
	Vec2f pos; gs::splitFRect(area, pos, param.size);
	return newBtn(id, pos, param);
}

bool newBtn(const std::string& id, FRect area, Btn_Param param)
{
	Vec2f pos; gs::splitFRect(area, pos, param.size);
	return newBtn(id, pos, param);
}



void INTERNAL_USE_showBtnsOfGroup(const Group& group, Renderer* ren)
{
	if(not group.is_active) return;

	SBIM::R res;

	for(int id : group.int_btns)
	{
		if(_sbim_.containsBtn(id, res)) //Sécurité
			res.int_btn_it->second.show(ren);
	}

	for(const std::string& id : group.str_btns)
	{
		if(_sbim_.containsBtn(id, res)) //Sécurité
			res.str_btn_it->second.show(ren);
	}
}


void showBtns(Renderer* ren) //tous
{
	for(const auto& [group_id, group] : _sbim_.m_int_ID_groups)
		INTERNAL_USE_showBtnsOfGroup(group, ren);

	for(const auto& [group_id, group] : _sbim_.m_str_ID_groups)
		INTERNAL_USE_showBtnsOfGroup(group, ren);
}


void showBtns(Renderer* ren, int group_id)
{
	bool is_ok;
	INTERNAL_USE_showBtnsOfGroup(INTERNAL_USE_getGroup(group_id, is_ok), ren);
}

void showBtns(Renderer* ren, const std::string& group_id)
{
	bool is_ok;
	INTERNAL_USE_showBtnsOfGroup(INTERNAL_USE_getGroup(group_id, is_ok), ren);
}

std::vector<Button*> INTERNAL_USE_getActiveBtns()
{
	std::vector<Button*> atv_btns; // "active buttons"
	SBIM::R res;

	std::vector<Group*> groups;
	for(auto& [_i, group] : _sbim_.m_int_ID_groups) groups.push_back(&group);
	for(auto& [_s, group] : _sbim_.m_str_ID_groups) groups.push_back(&group);


	for(Group* g : groups)
	{
		if(not g) continue;
		Group& group = *g;

		if(not group.is_active) continue;

		for(int id : group.int_btns) //Parcours des identifiants int des boutons de ce groupe
		{
			if(_sbim_.containsBtn(id, res)) //Récupération des boutons correspondants
				atv_btns.push_back(&res.int_btn_it->second);
		}

		for(const std::string& id : group.str_btns) //Même chose pour les identifiants str
		{
			if(_sbim_.containsBtn(id, res))
				atv_btns.push_back(&res.str_btn_it->second);
		}
	}

	return atv_btns;
}





bool newScbr(int id, const Scbr_Param& param)
{
	bool skip_presence_checking = false;

	if(id == wi::SCBR_NONE_ID) //Cas particulier
	{
		//Rajout d'un scrollbar avec la première clé libre trouvée.
		
		//Recherche d'un trou dans les valeurs des clés
		bool is_first_loop = true;
		int prev_id=wi::SCBR_NONE_ID; //important d'initialiser à cette valeur.
		bool found=false;
		for(const auto& [curr_id, scbr] : _sbim_.m_int_ID_scbrs)
		{
			if(is_first_loop) //Pour comparer deux clés successives..., il faut deux clés. Donc, on commence la comparaison à partir du deuxième tour.
			{
				prev_id = curr_id;
				is_first_loop = false;
				continue;
			}

			//Les clés sont des entiers, et le conteneur utilisé est 'std::map' (afin d'avoir les clés dans l'ordre).
			//Par conséquent, si l'on trouve deux clés successives distantes de plus d'un, alors on a trouvé un trou.
			int dist = std::abs(curr_id-prev_id); //Le 'std::abs()' n'est pas nécessaire ici, mais je le laisse.
			
			if(dist > 1)
			{
				id = prev_id+1; //récupération de l'ID normalement libre.
				found = true;
				break;
			}

			//Sinon on poursuit
			prev_id = curr_id;
		}

		if(not found) //Si aucun trou n'a été trouvé
			id = prev_id+1;

		skip_presence_checking = true;

		if(_sbim_.m_scbr_id_0_warning)
			gs::lastError("||Warning|| in 'newScbr(int)', called with 'wi::SCBR_NONE_ID' value [aka ",gs::str(int(wi::SCBR_NONE_ID))+"], so a free id will be used instead [free found id : "+gs::str(id)+"]. If it's intentional (using this feature), you can turn off this warning with 'turnOff0IDScbrWarning()'.");
		
	}

	SBIM::R res;
	if(not skip_presence_checking and _sbim_.containsScbr(id, res))
	{
		gs::lastError("||Warning|| in 'newScbr(int)', scrollbar id already used. [id = ",gs::str(id)+"]");
		return false;
	}


	auto r = _sbim_.m_int_ID_scbrs.emplace(id, Scrollbar());
	
	if(not r.second)
		gs::lastError("||Warning|| in 'newScbr(int)', couldn't add scrollbar. [id = ",gs::str(id)+"]");
	else
	{
		bool is_ok = r.first->second.init(param);
		
		if(not is_ok)
		{
			_sbim_.m_int_ID_scbrs.erase(id);

			gs::lastError("||Warning|| in 'newScbr(int)', couldn't init scrollbar, so addition is rejected. [id = ",gs::str(id)+"] : "+gs::lastError());
			return false;
		}

		_sbim_.addInGroup(id, r.first->second); //Une fois que tout est OK, on met le scrollbar dans le groupe d'accueil actif.
	}

	return r.second;
}


bool newScbr(const std::string& id, const Scbr_Param& param)
{
	if(id.length() == 0)
	{
		gs::lastError("||Warning|| in 'newScbr(str)', scrollbar id must not be an empty string.");
		return false;
	}

	SBIM::R res;
	if(_sbim_.containsScbr(id, res))
	{
		gs::lastError("||Warning|| in 'newScbr(str)', scrollbar id already used. [id = ",id+"]");
		return false;
	}


	auto r = _sbim_.m_str_ID_scbrs.emplace(id, Scrollbar());
	
	if(not r.second)
		gs::lastError("||Warning|| in 'newScbr(str)', couldn't add scrollbar. [id = ",id+"]");
	else
	{
		bool is_ok = r.first->second.init(param);
		
		if(not is_ok)
		{
			_sbim_.m_str_ID_scbrs.erase(id);

			gs::lastError("||Warning|| in 'newScbr(str)', couldn't init scrollbar, so addition is rejected. [id = ",id+"] : "+gs::lastError());
			return false;
		}

		_sbim_.addInGroup(id, r.first->second); //Une fois que tout est OK, on met le scrollbar dans le groupe d'accueil actif.
	}

	return r.second;
}


void INTERNAL_USE_showScbrsOfGroup(const Group& group, Renderer* ren)
{
	if(not group.is_active) return;

	SBIM::R res;

	for(int id : group.int_scbrs)
	{
		if(_sbim_.containsScbr(id, res)) //Sécurité
			res.int_scbr_it->second.show(ren);
	}

	for(const std::string& id : group.str_scbrs)
	{
		if(_sbim_.containsScbr(id, res)) //Sécurité
			res.str_scbr_it->second.show(ren);
	}
}


void showScbrs(Renderer* ren) //tous
{
	for(const auto& [group_id, group] : _sbim_.m_int_ID_groups)
		INTERNAL_USE_showScbrsOfGroup(group, ren);

	for(const auto& [group_id, group] : _sbim_.m_str_ID_groups)
		INTERNAL_USE_showScbrsOfGroup(group, ren);
}


void showScbrs(Renderer* ren, int group_id)
{
	bool is_ok;
	INTERNAL_USE_showScbrsOfGroup(INTERNAL_USE_getGroup(group_id, is_ok), ren);
}

void showScbrs(Renderer* ren, const std::string& group_id)
{
	bool is_ok;
	INTERNAL_USE_showScbrsOfGroup(INTERNAL_USE_getGroup(group_id, is_ok), ren);
}


std::vector<Scrollbar*> INTERNAL_USE_getActiveScbrs()
{
	std::vector<Scrollbar*> atv_scbrs; // "active scrollbars"
	SBIM::R res;

	std::vector<Group*> groups;
	for(auto& [_i, group] : _sbim_.m_int_ID_groups) groups.push_back(&group);
	for(auto& [_s, group] : _sbim_.m_str_ID_groups) groups.push_back(&group);


	for(Group* g : groups)
	{
		if(not g) continue;
		Group& group = *g;

		if(not group.is_active) continue;

		for(int id : group.int_scbrs) //Parcours des identifiants int des scrollbars de ce groupe
		{
			if(_sbim_.containsScbr(id, res)) //Récupération des scrollbars correspondants
				atv_scbrs.push_back(&res.int_scbr_it->second);
		}

		for(const std::string& id : group.str_scbrs) //Même chose pour les identifiants str
		{
			if(_sbim_.containsScbr(id, res))
				atv_scbrs.push_back(&res.str_scbr_it->second);
		}
	}

	return atv_scbrs;
}




void updateScrollbarsAndBtns(float dt)
{
	#ifdef GSLIB_RAYLIB_BACKEND
	INTERNAL_USE_checkScrollbarsAndBtnsEvent();
	#endif // GSLIB_RAYLIB_BACKEND

	bool INTERNAL__check_events = false; //Pour ne pas traiter les events deux fois par élément.

	for(Button* btn : INTERNAL_USE_getActiveBtns())
		if(btn)
			btn->update(dt, INTERNAL__check_events);

	for(Scrollbar* bar :  INTERNAL_USE_getActiveScbrs())
		if(bar)
			bar->update(INTERNAL__check_events);
}























//==============================================================================//
//==============================================================================//
//==============================================================================//




Button::~Button()
{
	if(m.acquire_texture_unloading_responsibility)
		if(m.texture)
			gs::destroyTexture(m.texture);
}



bool Button::init(FRect area, Btn_Param param)
{
	param.size.x = area.w;
	param.size.y = area.h;
	return init(Vec2f(area.x, area.y), param);
}

bool Button::init(Vec2f pos, const Btn_Param& param)
{
	if(m_is_init)
	{
		gs::lastError("in 'Button::init()', button already init.");
		return false;
	}

	if(param.size.x <= 0 or param.size.y <= 0)
	{
		gs::lastError("in 'Button::init()', given size is invalid (negative size or empty rect).");
		return false;
	}

	m_pos = pos;
	m = param;

	m_is_init = true;
	return true;
}



bool Button::loadTexture(Renderer* ren, const std::string& img_file_path)
{
	if(not m_is_init)
	{
		gs::lastError("In 'Button::loadTexture()', button not yet init. ");
		return false;
	}

	Texture* texture = gs::loadTexture(ren, img_file_path);

	if(not texture)
	{
		gs::lastError(gs::str("from 'Button::init()' : ")+gs::lastError());
		return false;
	}
	
	//gs::addToClearAtTheEnd(m_texture); //Non puisqu'il y a le 'acquire_unloading_responsibility' à 'true'.
	return setTexture(texture, true);
}

bool Button::setTexture(Texture* texture, bool acquire_unloading_responsibility)
{
	if(not m_is_init)
	{
		gs::lastError("In 'Button::setTexture()', button not yet init.");
		return false;
	}

	if(not texture)
	{
		gs::lastError("In 'Button::setTexture()', given texture is invalid.");
		return false;
	}

	//Gestion de la précédente texture
	if(m.texture and m.acquire_texture_unloading_responsibility)
		gs::destroyTexture(m.texture);

	m.texture = texture;
	m.acquire_texture_unloading_responsibility = acquire_unloading_responsibility;
		
	return true;
}


void Button::update(float dt, bool INTERNAL__check_events)
{
	if(not m_is_init) return;

	#ifdef GSLIB_RAYLIB_BACKEND
		if(INTERNAL__check_events)
			INTERNAL_USE_checkEvents();
	#endif // GSLIB_RAYLIB_BACKEND

	bool down_click_occured = not m_is_prev_down and m_is_down;
	bool released = m_is_prev_down and not m_is_down;

	m_is_prev_down = m_is_down;

	if(down_click_occured and m.down_click_callback)
		m.down_click_callback(*this, m.callback_arg);

	if(released and m.release_callback)
		m.release_callback(*this, m.callback_arg);
}

bool Button::show(Renderer* ren) const
{
	if(not enabled()) return true;

	if(not m_is_init)
	{
		gs::lastError("In 'Button::show()', button not yet init. ");
		return false;
	}


	bool success = true;
	Blend_Mode blend_mode = BLENDMODE_BLEND;

	FRect area{m_pos.x,m_pos.y,m.size.x,m.size.y};

	if(not m.texture)
	{
		Color curr_col = m.idle_color;

		if(m_is_down)
			curr_col = m.down_color;

		gs::drawRect(ren, &area, curr_col, true, &blend_mode);
	}
	else
	{
		//Détermination d'un rectangle source pour la texture.
		FRect src_rect{0.f,0.f,0.f,0.f};

		if(m.texture_tile_size.x > 0 and m.texture_tile_size.y > 0) //Si une taille de tuile correcte est mentionnée
		{
			src_rect.w = m.texture_tile_size.x;
			src_rect.h = m.texture_tile_size.y;

			Vec2i tile_pos = m.idle_tile_pos;
			if(m_is_down)
				tile_pos = m.down_tile_pos;

			src_rect.x = src_rect.w*tile_pos.x;
			src_rect.y = src_rect.h*tile_pos.y;
		}
		else //Sinon on considère toute la texture
		{
			Vec2f img_size = gs::getTextureSize(m.texture);
			src_rect.w = img_size.x;
			src_rect.h = img_size.y;
		}

		bool res = gs::drawTexture(ren, m.texture, &src_rect, &area);

		if(!res)
		{
			gs::lastError(gs::str("In 'Button::show()', show img failed : ")+gs::str(Backend_GetError()));
			success = false;
		}
	}


	if(m_mouse_in)
		gs::drawRect(ren, &area, m.mouse_in_color, true, &blend_mode);


	//Dessin du texte
	{
		if(not _sbim_.m_tlsc.isInit_TLSC())
			_sbim_.initTheTLSC(ren);
		
		if(_sbim_.m_tlsc.isInit_TLSC())
			_sbim_.showBtnText(ren, "test", area);
		
	}

	return success;
}



void Button::setArea(FRect new_area)
{
	if(not m_is_init)
	{
		gs::lastError("In 'Button::setArea()', button not yet init.");
		return;
	}

	if(new_area.w <= 0.f or new_area.h <= 0.f)
	{
		gs::lastError("In 'Button::setArea()', invalid given size.");
		return;
	}

	m_pos.x = new_area.x;
	m_pos.y = new_area.y;
	m.size.x = new_area.w;
	m.size.y = new_area.h;
}

void Button::setPos(float x, float y)
{
	if(not m_is_init)
	{
		gs::lastError("In 'Button::setPos()', button not yet init.");
		return;
	}

	m_pos.x = x;
	m_pos.y = y;
}

void Button::incrPos(float x, float y)
{
	if(not m_is_init)
	{
		gs::lastError("In 'Button::incrPos()', button not yet init.");
		return;
	}

	m_pos.x += x;
	m_pos.y += y;
}

void Button::setDownState(bool new_state)
{
	if(not m_is_init)
	{
		gs::lastError("In 'Button::setDownState()', button not yet init.");
		return;
	}

	m_is_down = new_state;
}



void Button::setColor(wi::Button_State target_state, Color c)
{
	if(not m_is_init)
	{
		gs::lastError("In 'Button::setColor()', button not yet init.");
		return;
	}

	switch(target_state)
	{
		case wi::BTN_IDLE: m.idle_color = c; break;
		case wi::BTN_DOWN: m.down_color = c; break;
		case wi::BTN_MOUSE_IN: m.mouse_in_color = c; break;
		default: break;
	}
}





bool Button::setDownClickCallback(void (*callback)(const Button&, void*))
{
	if(not m_is_init)
	{
		gs::lastError("In 'Button::setDownClickCallback()', button not yet init.");
		return false;
	}

	m.down_click_callback = callback;
	return true;
}

bool Button::setUpClickCallback(void (*callback)(const Button&, void*))
{
	if(not m_is_init)
	{
		gs::lastError("In 'Button::setUpClickCallback()', button not yet init.");
		return false;
	}

	m.release_callback = callback;
	return true;
}






bool Button::isInit() const {return m_is_init;}

Texture* Button::getTexture() {return m.texture;}

FRect Button::getArea() const {return FRect(m_pos.x,m_pos.y,m.size.x,m.size.y);}

bool Button::isDown() const {return m_is_down;}


Color Button::getColor(wi::Button_State target_state) const
{
	switch(target_state)
	{
		case wi::BTN_IDLE: return m.idle_color;
		case wi::BTN_DOWN: return m.down_color;
		case wi::BTN_MOUSE_IN: return m.mouse_in_color;
		default: break;
	}

	return {0,0,0,0};
}

bool Button::isMouseIn() const {return m_mouse_in;}






















//==============================================================================//
//==============================================================================//
//==============================================================================//


bool Scrollbar::init(const Scbr_Param& param) //Optionnelle
{
	if(param.orient != wi::VERTICAL and param.orient != wi::HORIZONTAL)
	{
		gs::lastError("||Warning|| in 'Scrollbar::init()', invalid 'param.orient'.");
		return false;
	}

	m = param;
	return true;
}


void Scrollbar::checkParam()
{
	if(m.len < m.min_len)
		m.len = m.min_len;

	if(m.thickness < m.min_len)
		m.thickness = m.min_thickness;

	int max_pixel_progr = m.len - m.moveable_thing_len;
	if(max_pixel_progr < 0) max_pixel_progr = 0;

	if(m_pixel_progr > max_pixel_progr)
		m_pixel_progr = max_pixel_progr;
	else
	if(m_pixel_progr < 0)
		m_pixel_progr = 0;
}


void Scrollbar::incrProgr(int val)
{
	int max_pixel_progr = m.len - m.moveable_thing_len;
	if(max_pixel_progr < 0) max_pixel_progr = 0;

	int new_progr = m_pixel_progr + val;

	if(new_progr > max_pixel_progr)
		new_progr = max_pixel_progr;
	else
	if(new_progr < 0)
		new_progr = 0;

	m_pixel_progr = new_progr;
}


void Scrollbar::getAreaRects(Rect& area, Rect& moveable_thing_area) const
{
	//Définition de la zone globale de la scrollbar, et de l'élément amovible.
	area.x = m.pos.x;
	area.y = m.pos.y;
	if(m.orient == wi::VERTICAL)
	{
		area.w = m.thickness;
		area.h = m.len;

		moveable_thing_area.x = area.x;
		moveable_thing_area.y = area.y + m_pixel_progr;
		moveable_thing_area.w = area.w;
		moveable_thing_area.h = m.moveable_thing_len;
	}
	else
	{
		area.w = m.len;
		area.h = m.thickness;

		moveable_thing_area.x = area.x + m_pixel_progr;
		moveable_thing_area.y = area.y;
		moveable_thing_area.w = m.moveable_thing_len;
		moveable_thing_area.h = area.h;
	}
}



bool Scrollbar::show(Renderer* ren)
{
	checkParam();

	static Rect area;
	static Rect moveable_thing_area;

	//Définition de la zone globale de la scrollbar, et de l'élément amovible.
	getAreaRects(area, moveable_thing_area);
	FRect area_f = frect(area);
	FRect moveable_thing_area_f = frect(moveable_thing_area);

	bool success=true;
	success = success & gs::drawRect(ren, &area_f, m.bg_col, true);
	success = success & gs::drawRoundedRect(ren, moveable_thing_area_f, 0.35, m.fg_col, true);
	success = success & gs::drawRect(ren, &area_f, m.bg_col, false);


	if(not success)
		gs::lastError(std::string("from 'Scrollbar::show()' : ")+gs::lastError());

	return success;
}


void Scrollbar::update(bool INTERNAL__check_events)
{
	#ifdef GSLIB_RAYLIB_BACKEND
		if(INTERNAL__check_events)
			INTERNAL_USE_checkEvents();
	#endif // GSLIB_RAYLIB_BACKEND

	checkParam();

	if(m_to_move_buf != 0)
	{
		incrProgr(m_to_move_buf);
		m_to_move_buf = 0;
		m_moved = true;
	}
	else
		m_moved = false;
}




bool Scrollbar::moved() const {return m_moved;}


unsigned int Scrollbar::setProgr(unsigned int val)
{
	unsigned int prev_progr = m_pixel_progr;

	double max_pixel_progr = m.len - m.moveable_thing_len;
	if(max_pixel_progr < 0) max_pixel_progr = 0;

	m_pixel_progr = (unsigned int)(max_pixel_progr*(double(val)/double(m.max_val)));
	checkParam();

	return prev_progr;
}


unsigned int Scrollbar::getProgr()
{
	checkParam();

	int max_pixel_progr = m.len - m.moveable_thing_len;
	if(max_pixel_progr < 0) max_pixel_progr = 0;

	double progr_percent = double(m_pixel_progr) / double(max_pixel_progr);

	unsigned int val = (unsigned int)(progr_percent * double(m.max_val));

	return val;
}



wi::Scrollbar_Orient Scrollbar::setOrientation(const wi::Scrollbar_Orient& new_orient)
{
	wi::Scrollbar_Orient prev_orient = m.orient;
	m.orient = new_orient;
	return prev_orient;
}

wi::Scrollbar_Orient Scrollbar::getOrientation() const {return m.orient;}


unsigned int Scrollbar::setLen(unsigned int new_len)
{
	unsigned int prev_len = m.len;
	m.len = new_len;
	m.moveable_thing_len = static_cast<unsigned int>(double(m.len)*0.1);

	return prev_len;
}


unsigned int Scrollbar::getLen() const {return m.len;}



unsigned int Scrollbar::setThickness(unsigned int new_thickness)
{
	unsigned int prev_thickness = m.thickness;
	m.thickness = new_thickness;
	return prev_thickness;
}

unsigned int Scrollbar::getThickness() const {return m.thickness;}




Vec2i Scrollbar::setPos(Vec2i new_pos)
{
	Vec2i prev_pos = m.pos;
	m.pos = new_pos;
	return prev_pos;
}


Vec2i Scrollbar::getPos() const {return m.pos;}



unsigned int Scrollbar::setMaxVal(unsigned int max_val)
{
	unsigned int prev_max_v = m.max_val;
	m.max_val = max_val;
	return prev_max_v;
}


unsigned int Scrollbar::getMaxVal() const {return m.max_val;}



Color Scrollbar::setColor(const wi::Scrollbar_Area& area, const Color& color)
{
	Color prev_col;

	switch(area)
	{
		case wi::MAIN_AREA: prev_col = m.bg_col; m.bg_col = color; break;
		case wi::MOVEABLE_THING_AREA: prev_col = m.fg_col; m.fg_col = color; break;
		default : break;
	}

	return prev_col;
}


Color Scrollbar::getColor(const wi::Scrollbar_Area& area) const
{
	switch(area)
	{
		case wi::MAIN_AREA: return m.bg_col;
		case wi::MOVEABLE_THING_AREA: return m.fg_col;
		default : break;
	}

	return m.bg_col;
}




Rect Scrollbar::getArea(const wi::Scrollbar_Area& area) const
{
	static Rect area_rect;
	static Rect moveable_thing_area_rect;

	//Définition de la zone globale de la scrollbar, et de l'élément amovible.
	getAreaRects(area_rect, moveable_thing_area_rect);

	switch(area)
	{
		case wi::MAIN_AREA: return area_rect;
		case wi::MOVEABLE_THING_AREA: return moveable_thing_area_rect;
		default : break;
	}

	return area_rect;
}












//==============================================================================//
//==============================================================================//
//==============================================================================//


void Trail::newNode(Vec2f anchor, float width, float duration)
{
    if(m_adding_timer > 0.f) return; //Limitation de la fréquence d'ajout.

    Trail_Node node;
    node.anchor = anchor;
    node.duration = GetMax(0.f, duration);
    node.width = width;

    if(m_nodes.size()) //S'il y a un noeud avant lui, on s'en sert pour estimer l'orthogonale à la trajectoire, entre ces deux points.
    {
        const Trail_Node& prev_node = m_nodes.front();
        node.ortho_dir = gs::normalize(gs::orthoVecOf(node.anchor - prev_node.anchor));
    }
    //Note: le premier noeud a être rajouté n'aura pas de prédécesseur, '.ortho_dir' restera nul => les trois points seront confondus, ce qui aura pour effet de faire une pointe, aka le comportement voulu.

    m_nodes.insert(m_nodes.begin(), node); //Les nouveaux sont rajoutés au début du vecteur 'm_nodes', cela permet de simplifier le retrait plus tard.
    m_adding_timer = m_adding_dalay; //Afin de limiter la fréquence d'ajout.
}


void Trail::update(float dt)
{
    //On ne retire les noeuds finis que s'ils sont à l'extrémité le plus "agé" (aka la fin du vecteur 'm_nodes').

    bool can_remove = true; //Détermine si le noeud courant est à l'extrémité le plus "agé"
    for(int i=int(m_nodes.size())-1; i>=0; --i) //parcours à rebours, afin de pouvoir facilement retirer les noeuds finis.
    {
        Trail_Node& node = m_nodes.at(i);

        if(can_remove and  node.timer >= node.duration)
        {
            m_nodes.erase(m_nodes.begin()+i);
            //Et 'can_remove' est laissé à 'true', car par récurrence le noeud suivant est désormais le plus "agé".
            continue;
        }

        //Sinon, le noeud est encore en cours, donc non retiré, ce qui fait que le suivant n'est pas le plus "agé" et donc ne peut être retiré.
        node.timer+=dt;
        can_remove = false;
    }

    if(m_adding_timer > 0.f)
        m_adding_timer -= dt;
}


Trail_Vertices Trail::getVertices(Color color_0_255, const Camera2D_Base& cam) const
{
    Trail_Vertices output;
    if(m_nodes.size() <= 1) return output; //Il faut au moins deux noeud pour former un quad.

    //Le principe est la suivante :
    //Entre chaque noeuds succéssifs, on forme un quadrilatère avec les points extrêmes de chaque noeud.
    //En vue d'utilisation de texture, les noeuds autres que le premier et le dernier, seront impliqués dans deux quadrilatère :
    //un où ils sont le côté min, un où ils sont le côté max. Ainsi la texture sera répétée.
    //La répétition se fait sur l'horizontal de la texture.

    Vertex v; //Le buffer
    v.color = gs::normalizeColor(color_0_255);

    std::vector<int> new_indices_buff;

    for(int i=0; i<m_nodes.size()-1; ++i) //Jusqu'à l'avant dernier (car dans un tour on traite le courant et le suivant).
    {
        const Trail_Node& curr = m_nodes[i];
        const Trail_Node& next = m_nodes[i+1];

        int fvi = output.vertices.size(); //first_vertex_idx
        bool is_prev_oldest = i==m_nodes.size()-2;

        //coordonnées de textures de la façon
        // (0,0)---(1,0)
        //  |         |
        // (0,1)---(1,1)

        //Les sommets
        v.position=cam.getFormattedPos(curr.getPoint1()).toFPoint();
        v.tex_coord=FPoint(0.f,0.f);
        output.vertices.push_back(v);

        v.position=cam.getFormattedPos(curr.getPoint2()).toFPoint();
        v.tex_coord=FPoint(0.f,1.f);
        output.vertices.push_back(v);

        v.position=cam.getFormattedPos(next.getPoint1()).toFPoint();
        v.tex_coord=FPoint(1.f,0.f);
        output.vertices.push_back(v);

        v.position=cam.getFormattedPos(next.getPoint2()).toFPoint();
        v.tex_coord=FPoint(1.f,1.f);
        output.vertices.push_back(v);


        if(is_prev_oldest) //organisation différente pour la "queue", en vu d'un meilleur rendu pour texture.
        {
            v.position=cam.getFormattedPos(curr.anchor).toFPoint();
            v.tex_coord=FPoint(1.f,0.5f);
            output.vertices.push_back(v);
            new_indices_buff = std::vector<int>({fvi+0,fvi+2,fvi+4,   fvi+1,fvi+4,fvi+3}); //Deux triangles
        }
        else
            new_indices_buff = std::vector<int>({fvi+0,fvi+2,fvi+1,   fvi+1,fvi+2,fvi+3}); //Deux triangles


        output.indices.insert(output.indices.end(), new_indices_buff.begin(), new_indices_buff.end());

    }

    return output;
}






} //namespace gs
