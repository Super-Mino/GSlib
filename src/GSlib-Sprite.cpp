#include "GSlib-Sprite.hpp"




//__________________________________________________________________________________________
//____________________________________SPRITE SHEET__________________________________________
//__________________________________________________________________________________________
//__________________________________________________________________________________________
//__________________________________________________________________________________________



namespace gs
{

Sprite_Sheet::~Sprite_Sheet()
{
	unload();
}

void Sprite_Sheet::unload()
{
	if(not m_is_loaded) return;

	if(m_texture and m_texture_is_from_inside)
	{
		destroyTexture(m_texture);
		if(show_comments)
			std::cout << "info : 'Sprite_Sheet::unload()' : texture destroyed.\n";
	}

	m_anim_frames.clear();
	m_curr_anim_pos = 0;
	m_frames_count = gs::Vec2i(0,0);
	m_anim_is_playing = false;
	m_is_loaded = false;
}



bool Sprite_Sheet::load(Renderer* ren, const std::string& img_path, Vec2us frame_size, float frame_delay)
{
	Texture* texture = gs::loadTexture(ren, img_path, true);

	if(not texture)
	{
		gs::lastError("from 'Sprite_Sheet::load()' : ", gs::lastError());
		return false;
	}


	m_called_from_load_with_path = true;

	return load(texture, frame_size, frame_delay);
}


bool Sprite_Sheet::load(Texture* texture, Vec2us frame_size, float frame_delay)
{
	//===Les vérifications===========================//

	if(not texture)
	{
        gs::lastError("||Warning|| in 'Sprite_Sheet::load()', invalid texture. ");
        return false;
    }

    float texture_w=-1.f, texture_h=-1.f;
    gs::getTextureSize(texture, &texture_w, &texture_h);

    if(texture_w <= 0 or texture_h <= 0)
    {
        gs::lastError("||Warning|| in 'Sprite_Sheet::load()', invalid texture (maybe the size). ");
        return false;
    }

    if(frame_size.x == 0 or frame_size.y == 0)
    {
        gs::lastError("||Warning|| in 'Sprite_Sheet::load()', invalid frame size-s (have null value-s). ");
        return false;
    }

    if(texture_w < frame_size.x or texture_h < frame_size.y)
    {
        gs::lastError("||Error|| in 'Sprite_Sheet::load()', 'texture_w < frame_size.x or texture_h < frame_size.y'. ");
        return false;
    }

    //========================//

    if(not m_called_from_changeFrameSize)
		unload();
	else
		m_anim_frames.clear();


	m_texture = texture;
	gs::addToClearAtTheEnd(m_texture);

	m_frame_size = frame_size.to<float>();

	m_frames_count = Vec2i(texture_w, texture_h) / m_frame_size.to<int>();

	if(frame_delay > 0)
		m_frame_delay = frame_delay;
	else
		m_frame_delay = DEFAULT_FRAME_DELAY;


	if(not m_called_from_changeFrameSize)
	{
		if(m_called_from_load_with_path)
			m_texture_is_from_inside = true;
		else
			m_texture_is_from_inside = false;
	}

	m_called_from_load_with_path=false;
	m_called_from_changeFrameSize=false;


	m_is_loaded = true;
	return true;
}





bool Sprite_Sheet::changeFrameSize(Vec2us new_frame_size)
{
	if(not m_is_loaded)
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::changeFrameSize()', this gs::Sprite_Sheet is not loaded. ");
        return false;
	}


	m_called_from_changeFrameSize = true; // Si c'est déjà chargé, on ne modifie le marqueur de l'origine de la texture car ici on veut se concentrer sur la taille d'une frame et ne pas falsifier la provenance de la texture.


	bool success = load(m_texture, new_frame_size, m_frame_delay);

	if(!success) gs::lastError("from 'Sprite_Sheet::changeFrameSize()' : ", gs::lastError());

	return success;
}




bool Sprite_Sheet::setCurrAnim(const std::vector<int>& frames_idx, float frame_delay)
{
	//'frame_delay' est ici facultatif et n'est donc pris en compte que si est strictement positif.

	if(not m_is_loaded)
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::setCurrAnim()', this gs::Sprite_Sheet is not loaded. ");
        return false;
	}

	if(frames_idx.empty())
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::setCurrAnim()', given arg 'frames_idx' is empty. ");
        return false;
	}

	//Vérification de la validité de tous les indices reçus.
	int max_idx = m_frames_count.x * m_frames_count.y -1;
	for(int idx : frames_idx)
		if(idx < 0 or idx > max_idx)
		{
			gs::lastError("||Warning|| in 'Sprite_Sheet::setCurrAnim()', have invalid frame index in the given indexes. ");
        	return false;
		}

	//Affectation.
	if(frame_delay > 0)
		m_frame_delay = frame_delay;

	m_anim_frames = frames_idx;
	m_curr_anim_pos=0; //Note : 'm_curr_anim_pos' pointe dans 'm_anim_frames'.
	m_timer = m_frame_delay; //Afin de commencer cette animation depuis son début, même si une autre animation était en train d'être jouée.

	m_curr_anim_ID=-1;

	return true;
}



bool Sprite_Sheet::setCurrAnim(int anim_id)
{
	if(not m_is_loaded)
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::setCurrAnim()', this gs::Sprite_Sheet is not loaded. ");
        return false;
	}

	auto it = m_anims.find(anim_id);

	if(it == m_anims.end()) //Si l'on ne trouve pas l'animation demandée.
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::setCurrAnim(anim_id)', given 'anim_id' [value : ", std::to_string(anim_id)+"] not found. ");
        return false;
	}

	Sprite_Sheet_Anim& anim = it->second;

	if(setCurrAnim(anim.frames_idx, anim.frame_delay))
	{
		m_curr_anim_ID = it->first;
		return true;
	}

	return false;
}



bool Sprite_Sheet::addAnim(int anim_id, const std::vector<int>& frames_idx, float frame_delay)
{
	if(not m_is_loaded)
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::addAnim()', this gs::Sprite_Sheet is not loaded. ");
        return false;
	}

	if(frames_idx.empty())
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::addAnim()', given arg 'frames_idx' is empty. ");
        return false;
	}

	if(anim_id < 0)
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::addAnim()', given arg 'anim_id' must be >= 0. ");
        return false;
	}

	if(frame_delay <= 0)
		frame_delay = m_frame_delay;

	Sprite_Sheet_Anim anim;
		anim.frames_idx = frames_idx;
		anim.frame_delay = frame_delay;

	m_anims[anim_id] = anim;

	return true;
}




void Sprite_Sheet::update(float dt)
{
	if(not m_is_loaded or not m_anim_is_playing)
		return;

	bool is_cycle_end=false;

	if(m_timer <= 0.f)
	{
		if(m_curr_anim_pos < static_cast<int>(m_anim_frames.size())-1)
			m_curr_anim_pos++;
		else
		{
			if(m_once)
				m_anim_is_playing = false;
			else
				m_curr_anim_pos = 0; //Je le mets dans le 'else' afin que les animations qui ne bouclent pas restent sur leur dernière frame lorsqu'elles se terminent.

			is_cycle_end = true;
		}

		m_timer += m_frame_delay;
	}
	else
		m_timer -= dt; //mis dans un 'else' du fait de l'usage de '+=' au lieu de '=' deux lignes plus haut..

	if(is_cycle_end and m_animEndCycleCallback)
		m_animEndCycleCallback(m_curr_anim_ID);
}



bool Sprite_Sheet::start(bool once)
{
	if(not m_is_loaded)
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::start()', this gs::Sprite_Sheet is not loaded. ");
        return false;
	}

	if(m_anim_frames.empty())
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::start()', there is no frame to animate. Try to 'setCurrAnim()' before. ");
        return false;
	}

	if(m_anim_is_playing) return false;

	m_curr_anim_pos=0; //Afin de commencer l'animation au début.
	m_timer = m_frame_delay;
	m_anim_is_playing = true;
	m_once = once;

	return true;
}


bool Sprite_Sheet::stop()
{
	if(not m_is_loaded)
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::stop()', this gs::Sprite_Sheet is not loaded. ");
        return false;
	}

	if(not m_anim_is_playing) return false;

	m_anim_is_playing = false;

	return true;
}


bool Sprite_Sheet::resume()
{
	if(not m_is_loaded)
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::resume()', this gs::Sprite_Sheet is not loaded. ");
        return false;
	}

	if(m_anim_frames.empty())
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::resume()', there is no frame to animate. ");
        return false;
	}

	m_anim_is_playing = true;;

	return true;
}






bool Sprite_Sheet::drawCurrentFrame(Renderer* ren, const FRect* dest, const float angle_rad, const Flip_Mode flip)
{
	//Dans la cible de rendu courante.

	if(not m_is_loaded)
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::drawCurrentFrame()', this gs::Sprite_Sheet is not loaded. ");
        return false;
	}

	if(m_anim_frames.empty())
		return false; //Pas d'erreur à spécifer, c'est simplement que la requête (aka cette méthode) ne peut être satisfaite en l'état des choses.

	if(m_curr_anim_pos < 0 or m_curr_anim_pos >= m_anim_frames.size()) //Sécurité.
	{
		gs::lastError("||Warning|| in 'Sprite_Sheet::drawCurrentFrame()', m_curr_anim_pos < 0 or m_curr_anim_pos >= m_anim_frames.size(). It might be an internal error. ", std::string("['m_curr_anim_pos' : ")+std::to_string(m_curr_anim_pos)+", 'm_anim_frames.size()' : "+std::to_string(m_anim_frames.size())+"]");
        return false;
	}

	//========================================================//

	int frame_idx = m_anim_frames.at(m_curr_anim_pos);

	FRect src={0.f,0.f, m_frame_size.x, m_frame_size.y};
	src.x = m_frame_size.x * (frame_idx%m_frames_count.x);
	src.y = m_frame_size.y * (frame_idx/m_frames_count.x);


	if(not drawTextureRotated(ren, m_texture, &src, dest, degree(angle_rad), nullptr, flip))
    {
        gs::lastError("||Error|| in 'Sprite_Sheet::drawCurrentFrame()', ", Backend_GetError());
        return false;
    }

    return true;
}


bool Sprite_Sheet::drawCurrentFrame(Renderer* ren, const FRect& dest, const float angle_rad, const Flip_Mode flip)
{
	//Dans la cible de rendu courante.

	return drawCurrentFrame(ren, &dest, angle_rad, flip);
}



bool Sprite_Sheet::setFrameDelay(float sec)
{
	if(sec <= 0) return false;

	m_frame_delay = sec;

	return true;
}



void Sprite_Sheet::setAnimEndCycleCallback(void (*animEndCycleCallback)(int anim_id))
{
	m_animEndCycleCallback = animEndCycleCallback;
}



Vec2f Sprite_Sheet::getTextureSize() const
{
	Vec2f texture_size{0.f,0.f};

	if(m_texture)
		gs::getTextureSize(m_texture, &texture_size.x, &texture_size.y);

	return texture_size;
}



Sprite_Sheet_Anim Sprite_Sheet::getAnim(int anim_id) const
{
	Sprite_Sheet_Anim not_found;
	not_found.frame_delay = -1.f;

	auto it = m_anims.find(anim_id);

	if(it == m_anims.end()) //Si l'on ne trouve pas l'animation demandée.
		return not_found;

	return it->second;
}





} //namespace gs
