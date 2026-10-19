
namespace gs
{
	bool drawEllipse(Renderer* ren, const Vec2i& center, const Vec2i& radii, const FColor& color, const bool& fill, float total_angle, float begin_angle);
}


template<class T>
std::ostream& operator<<(std::ostream& os, const gs::Vec2<T>& vec2)
{
	os << "(" << vec2.x << " | " << vec2.y <<")";
	return os;
}






namespace gs
{


template<class T>
Point Vec2<T>::toPoint() const
{
	Point pt{static_cast<int>(x), static_cast<int>(y)};
	return pt;
}

template<class T>
FPoint Vec2<T>::toFPoint() const
{
	FPoint pt{static_cast<float>(x), static_cast<float>(y)};
	return pt;
}

template<class T>
template<class U>
Vec2<U> Vec2<T>::to() const
{
	return Vec2<U>(static_cast<U>(x), static_cast<U>(y));
}


template<class T>
Vec2<T> Vec2<T>::operator+(const Vec2<T>& right) const {return Vec2<T>{x+right.x, y+right.y};}

template<class T>
Vec2<T> Vec2<T>::operator-(const Vec2<T>& right) const {return Vec2<T>{x-right.x, y-right.y};}

template<class T>
Vec2<T> Vec2<T>::operator*(const Vec2<T>& right) const {return Vec2<T>{x*right.x, y*right.y};}

template<class T>
Vec2<T> Vec2<T>::operator/(const Vec2<T>& right) const {return Vec2<T>{x/right.x, y/right.y};}

template<class T>
Vec2<T> Vec2<T>::operator%(const Vec2<T>& right) const {return Vec2<T>{x%right.x, y%right.y};}

template<class T>
Vec2<T> Vec2<T>::operator-() const {return Vec2<T>{-x, -y};}


template<class T>
Vec2<T>& Vec2<T>::operator+=(const Vec2<T>& right)
{	x+=right.x; y+=right.y; return (*this); }

template<class T>
Vec2<T>& Vec2<T>::operator-=(const Vec2<T>& right)
{	x-=right.x; y-=right.y; return (*this); }

template<class T>
Vec2<T>& Vec2<T>::operator*=(const Vec2<T>& right)
{	x*=right.x; y*=right.y; return (*this); }

template<class T>
Vec2<T>& Vec2<T>::operator/=(const Vec2<T>& right)
{	x/=right.x; y/=right.y; return (*this); }

template<class T>
Vec2<T>& Vec2<T>::operator%=(const Vec2<T>& right)
{	x%=right.x; y%=right.y; return (*this); }


template<class T>
bool Vec2<T>::operator==(const Vec2<T>& right) const
{	return (x==right.x) and (y==right.y); }

template<class T>
bool Vec2<T>::operator!=(const Vec2<T>& right) const
{	return !(operator==(right)); }




template<class T>
Vec2<T> Vec2<T>::operator+(const T& val) const
{	return Vec2<T>{x+val, y+val}; }

template<class T>
Vec2<T> Vec2<T>::operator-(const T& val) const
{	return Vec2<T>{x-val, y-val}; }

template<class T>
Vec2<T> Vec2<T>::operator*(const T& val) const
{	return Vec2<T>{x*val, y*val}; }

template<class T>
Vec2<T> Vec2<T>::operator/(const T& val) const
{	return Vec2<T>{x/val, y/val}; }

template<class T>
Vec2<T> Vec2<T>::operator%(const T& val) const
{	return Vec2<T>{x%val, y%val}; }


template<class T>
Vec2<T>& Vec2<T>::operator+=(const T& val)
{	x+=val; y+=val; return (*this); }

template<class T>
Vec2<T>& Vec2<T>::operator-=(const T& val)
{	x-=val; y-=val; return (*this); }

template<class T>
Vec2<T>& Vec2<T>::operator*=(const T& val)
{	x*=val; y*=val; return (*this); }

template<class T>
Vec2<T>& Vec2<T>::operator/=(const T& val)
{	x/=val; y/=val; return (*this); }

template<class T>
Vec2<T>& Vec2<T>::operator%=(const T& val)
{	x%=val; y%=val; return (*this); }











template<class T>
T dotProd(const Vec2<T>& vec1, const Vec2<T>& vec2)
{
	return vec1.x*vec2.x + vec1.y*vec2.y;
}

template<class T>
T vecLen(const Vec2<T>& vec)
{
	return std::sqrt(vec.x*vec.x + vec.y*vec.y);
}


template<class T>
T vecAngleRad(const Vec2<T>& vec1, const Vec2<T>& vec2)
{
	T v1_len = std::sqrt(vec1.x*vec1.x + vec1.y*vec1.y);
	T v2_len = std::sqrt(vec2.x*vec2.x + vec2.y*vec2.y);

	if(v1_len == 0 or v2_len == 0)
		return 0;

	return std::acos(dotProd(vec1, vec2)/(v1_len*v2_len));
}

















template<typename User_Data_T>
void Particle<User_Data_T>::update(float dt)
{
	// if(updater)
	// 	updater(*this, dt);
	// else
	if(left_time > 0)
	{
		static gs::Vec2f dir_vec;

		dir_vec.x = std::cos(dir) * (speed * dt);
		dir_vec.y = std::sin(dir) * (speed * dt);

		pos += dir_vec;
		left_time -= dt;
	}
}


template<typename User_Data_T>
bool Particle<User_Data_T>::show(Renderer* ren, const bool& fill,  Blend_Mode* target_blend_mode)
{
	bool success=true;

	if(type==RECT_PARTICLE)
	{
		static FRect rect;
		rect.x = pos.x;  rect.y = pos.y;
		rect.w = size.x; rect.h = size.y;

		success = gs::drawRect(ren, &rect, &color, fill, target_blend_mode);
	}
	else
	if(type==ELLIPSE_PARTICLE)
	{
		success = gs::drawEllipse(ren, {int(pos.x), int(pos.y)}, {int(size.x), int(size.y)},
										fcolor(color), fill, 2*_pi_, 0);
	}
	else
	if(type==CIRCLE_PARTICLE)
	{
		static int radius;
		radius = int(float(size.x + size.y)/2.f);

		success = gs::drawEllipse(ren, {int(pos.x), int(pos.y)}, {radius, radius},
										fcolor(color), fill, 2*_pi_, 0);
	}

	if(not success)
		gs::lastError(gs::str("from 'Particle::show()' : ")+gs::lastError());


	return success;
}


template<typename User_Data_T>
void Particle<User_Data_T>::setLeftTime(float t)
{
	left_time = t;
	begin_left_time = t;
}





template<typename User_Data_T>
bool Textured_Particle<User_Data_T>::show(Renderer* ren)
{
	static FRect dest_rect;
	dest_rect.x = this->pos.x; dest_rect.y = this->pos.y;
	dest_rect.w = this->size.x; dest_rect.h = this->size.y;

	bool res = drawTextureRotated(ren, texture, &src_rect, &dest_rect, gs::degree(rotation),
								nullptr, FLIP_NONE);

	if(not res)
	{
		gs::lastError(gs::str("In 'Textured_Particle::show()' : ")+Backend_GetError());
		return false;
	}

	return true;
}

template<typename User_Data_T>
void Textured_Particle<User_Data_T>::update(float dt)
{
	this->update(dt);

	if(this->left_time > 0)
		rotation += rotation_speed * dt;
}


template<typename User_Data_T>
void Textured_Particle<User_Data_T>::setLeftTime(float t)
{
	this->left_time = t;
	this->begin_left_time = t;
}






template<typename P_User_Data_T>
void defaultParticleColorUpdater(const Particle<P_User_Data_T>& particle, Color& particle_color)
{
	//C'est un 'cosine fade out' pour la transparence, si je ne m'abuse.
	particle_color.a = std::cos((_pi_/2.0) * ((particle.begin_left_time-particle.left_time)/particle.begin_left_time))*255.0;
}


template<class P_User_Data_T>Particle_Box<P_User_Data_T>::
Particle_Box()
{
	std::random_device rd;
	const int engines_count=8;

	m_norm_distrs.insert(m_norm_distrs.begin(), engines_count, Random<std::normal_distribution<float>>());
	m_unif_distrs.insert(m_unif_distrs.begin(), engines_count, Random<std::uniform_real_distribution<float>>());

	for(int i=0; i<engines_count; ++i)
	{
		unsigned int n_seed = rd();
		unsigned int u_seed = rd();

		m_norm_distrs.at(i).setSeed(n_seed);
		m_unif_distrs.at(i).setSeed(u_seed);

		m_seeds.push_back({n_seed, u_seed});
	}

	//Modificateur par défaut des couleurs.
	setParticleColorUpdater(defaultParticleColorUpdater);
}

template<class P_User_Data_T> void Particle_Box<P_User_Data_T>::
setSeed(Particle_Attrib_Field_T field, Rand_Distr_T distr_type, unsigned int seed)
{
	if(int(field) < 0 or int(field) > 7) return;

	if(distr_type == NORMAL_DISTR)
	{
		m_norm_distrs.at(int(field)).setSeed(seed);
		m_seeds.at(int(field)).x = seed;
	}
	else if(distr_type == UNIF_DISTR)
	{
		m_unif_distrs.at(int(field)).setSeed(seed);
		m_seeds.at(int(field)).y = seed;
	}
}

template<class P_User_Data_T> unsigned int Particle_Box<P_User_Data_T>::
getSeed(Particle_Attrib_Field_T field, Rand_Distr_T distr_type) const
{
	if(int(field) < 0 or int(field) > 7) return 0;

	if(distr_type == NORMAL_DISTR)
		return m_seeds.at(int(field)).x;

	return m_seeds.at(int(field)).y;
}


template<class P_User_Data_T> Particle_Param Particle_Box<P_User_Data_T>::
setParam(const Particle_Param& new_param)
{
	Particle_Param prev_param = m_param;
	m_param = new_param;
	return prev_param;
}

template<class P_User_Data_T> Particle_Param Particle_Box<P_User_Data_T>::
getParam() const
{
	return m_param;
}

template<class P_User_Data_T> Particle_Param& Particle_Box<P_User_Data_T>::
getParam_ref()
{
	return m_param;
}


template<class P_User_Data_T> Particle_Color_Param Particle_Box<P_User_Data_T>::
setColorParam(const Particle_Color_Param& new_col_param)
{
	Particle_Color_Param prev_col_param = m_col_param;
	m_col_param = new_col_param;
	return prev_col_param;
}


template<class P_User_Data_T> Particle_Color_Param Particle_Box<P_User_Data_T>::
getColorParam() const
{
	return m_col_param;
}

template<class P_User_Data_T> Particle_Color_Param& Particle_Box<P_User_Data_T>::
getColorParam_ref()
{
	return m_col_param;
}


template<class P_User_Data_T> void Particle_Box<P_User_Data_T>::
adjustParam(const Particle_Param& param)
{
	typedef std::normal_distribution<float>::param_type N_p_type;
	typedef std::uniform_real_distribution<float>::param_type U_p_type;


	if(param.x_pos.distr_type == NORMAL_DISTR)
		m_norm_distrs.at(0).setParam(N_p_type{param.x_pos.a, param.x_pos.b});
	else
	if(param.x_pos.distr_type == UNIF_DISTR)
		m_unif_distrs.at(0).setParam(U_p_type{param.x_pos.a, param.x_pos.b});

	if(param.y_pos.distr_type == NORMAL_DISTR)
		m_norm_distrs.at(1).setParam(N_p_type{param.y_pos.a, param.y_pos.b});
	else
	if(param.y_pos.distr_type == UNIF_DISTR)
		m_unif_distrs.at(1).setParam(U_p_type{param.y_pos.a, param.y_pos.b});
	//

	if(param.x_size.distr_type == NORMAL_DISTR)
		m_norm_distrs.at(2).setParam(N_p_type{param.x_size.a, param.x_size.b});
	else
	if(param.x_size.distr_type == UNIF_DISTR)
		m_unif_distrs.at(2).setParam(U_p_type{param.x_size.a, param.x_size.b});

	if(param.y_size.distr_type == NORMAL_DISTR)
		m_norm_distrs.at(3).setParam(N_p_type{param.y_size.a, param.y_size.b});
	else
	if(param.y_size.distr_type == UNIF_DISTR)
		m_unif_distrs.at(3).setParam(U_p_type{param.y_size.a, param.y_size.b});
	//

	if(param.dir.distr_type == NORMAL_DISTR)
		m_norm_distrs.at(4).setParam(N_p_type{param.dir.a, param.dir.b});
	else
	if(param.dir.distr_type == UNIF_DISTR)
		m_unif_distrs.at(4).setParam(U_p_type{param.dir.a, param.dir.b});

	if(param.speed.distr_type == NORMAL_DISTR)
		m_norm_distrs.at(5).setParam(N_p_type{param.speed.a, param.speed.b});
	else
	if(param.speed.distr_type == UNIF_DISTR)
		m_unif_distrs.at(5).setParam(U_p_type{param.speed.a, param.speed.b});
	//

	if(param.time.distr_type == NORMAL_DISTR)
		m_norm_distrs.at(6).setParam(N_p_type{param.time.a, param.time.b});
	else
	if(param.time.distr_type == UNIF_DISTR)
		m_unif_distrs.at(6).setParam(U_p_type{param.time.a, param.time.b});
}


template<class P_User_Data_T> float Particle_Box<P_User_Data_T>::
getRandVal(int field_index, const Particle_Param& param)
{
	if(field_index < 0 or field_index >= int(m_norm_distrs.size()))
		return 0;

	switch(field_index)
	{
		default:break;

		case 0:
			if(param.x_pos.distr_type==NORMAL_DISTR)
				return m_norm_distrs.at(0).get();
			if(param.x_pos.distr_type==UNIF_DISTR)
				return m_unif_distrs.at(0).get();
			break;

		case 1:
			if(param.y_pos.distr_type==NORMAL_DISTR)
				return m_norm_distrs.at(1).get();
			if(param.y_pos.distr_type==UNIF_DISTR)
				return m_unif_distrs.at(1).get();
			break;

		case 2:
			if(param.x_size.distr_type==NORMAL_DISTR)
				return m_norm_distrs.at(2).get();
			if(param.x_size.distr_type==UNIF_DISTR)
				return m_unif_distrs.at(2).get();
			break;

		case 3:
			if(param.y_size.distr_type==NORMAL_DISTR)
				return m_norm_distrs.at(3).get();
			if(param.y_size.distr_type==UNIF_DISTR)
				return m_unif_distrs.at(3).get();
			break;

		case 4:
			if(param.dir.distr_type==NORMAL_DISTR)
				return m_norm_distrs.at(4).get();
			if(param.dir.distr_type==UNIF_DISTR)
				return m_unif_distrs.at(4).get();
			break;

		case 5:
			if(param.speed.distr_type==NORMAL_DISTR)
				return m_norm_distrs.at(5).get();
			if(param.speed.distr_type==UNIF_DISTR)
				return m_unif_distrs.at(5).get();
			break;

		case 6:
			if(param.time.distr_type==NORMAL_DISTR)
				return m_norm_distrs.at(6).get();
			if(param.time.distr_type==UNIF_DISTR)
				return m_unif_distrs.at(6).get();
			break;

	}

	return 0;
}



template<class P_User_Data_T> Color Particle_Box<P_User_Data_T>::
getColor(const Particle_Color_Param& cp)
{
	Color c{200,200,200,255};

	typedef std::normal_distribution<float>::param_type N_p_type;
	typedef std::uniform_real_distribution<float>::param_type U_p_type;


	if(cp.col_type==TARGET_COLORS)
	{
		if(cp.possible_colors.empty())
			return c;


		int index=0;
		int max_index = cp.possible_colors.size()-1;

		if(max_index == 0) return cp.possible_colors.front();

		//Sélection d'une couleur.
		if(cp.possible_colors_distr_type == NORMAL_DISTR)
		{
			float mean_index = float(max_index)/2.f;
			float stddev = float(max_index)/4.f;

			m_norm_distrs.at(7).setParam(N_p_type(mean_index, stddev));
			index = gs::roundToInt(m_norm_distrs.at(7).get());
		}
		else
		if(cp.possible_colors_distr_type == UNIF_DISTR)
		{
			m_unif_distrs.at(7).setParam( U_p_type(0, float(max_index)) );
			index = gs::roundToInt(m_unif_distrs.at(7).get());
		}

		//Gestion des valeurs invalides.
		if(index < 0)
			index = -index;
		else
		if(index > max_index)
			index = max_index - (index - max_index);

		if(index < 0)
			index = max_index;
		else
		if(index > max_index)
			index = 0;


		try{
			return cp.possible_colors.at(index);
		}
		catch(std::exception& err)
		{
			gs::lastError("INTERNAL - In 'Particle_Box<P_User_Data_T>::getColor()' : cp.possible_colors indexing error ? ");
		}

		return c;
	}


	if(cp.col_type!=RANDOM_CHOICE)
		return c;



	//Regroupement des plages, afin d'itérer dessus.
	std::vector<const Particle_Color_Param::Field*> ranges;
	ranges.insert(ranges.begin(), {&cp.r_range, &cp.g_range, &cp.b_range, &cp.a_range});

	//Variable et constante d'aide à l'itération.
	const int count=4;
	uint8_t rgba[count] = {100, 100, 100, 255}; //{r, g, b, a} -> {[0], [1], [2], [3]};

	//Parcours des différentes plages, et pour chacune d'elles, on prend "aléatoirement" une valeur comprise dans la plage, avec le type d'alétoire choisi.
	int i=0;
	for(const Particle_Color_Param::Field* range : ranges)
	{
		if(i >= count) //Juste au cas où.
		{
			gs::lastError("Internal Warning /!\\ : 'Particle_Box<P_User_Data_T>::getColor()',  i >= count. ");
			std::cout << gs::lastError() << std::endl;
			break;
		}

		if(range->a == range->b) //Si la plage ne comporte qu'une valeur possible.
		{
			rgba[i] = range->a;

			++i;
			continue;
		}

		int val=0;
		int min = GetMin(range->a, range->b);
		int max = GetMax(range->a, range->b);

		//Obtention de la veuleur.
		if(range->distr_type == NORMAL_DISTR)
		{
			float mean = float(min + max)/2.f;
			float stddev = float(max - min)/4.f;

			m_norm_distrs.at(7).setParam(N_p_type(mean, stddev));
			val = gs::roundToInt(m_norm_distrs.at(7).get());
		}
		else
		if(range->distr_type == UNIF_DISTR)
		{
			m_unif_distrs.at(7).setParam(U_p_type(float(min), float(max)));
			val = gs::roundToInt(m_unif_distrs.at(7).get());
		}

		//Gestion des écarts.
		if(val < min)
			val = min + (min-val); //Tentative de réintroduire la valeur dans l'intervalle.
		else
		if(val > max)
			val = max - (val-max);

		//Si l'écart ne s'est pas réglé.
		if(val < min)
			val = 255;
		else
		if(val > max)
			val = 0;

		rgba[i] = static_cast<uint8_t>(val);

		++i;
	}

	c.r = rgba[0];
	c.g = rgba[1];
	c.b = rgba[2];
	c.a = rgba[3];

	return c;
}


template<class P_User_Data_T> Texture* Particle_Box<P_User_Data_T>::
getTexture(const Particle_Texture_Param& tp)
{
	if(tp.possible_textures.empty())
		return nullptr;


	int index=0;
	int max_index = tp.possible_textures.size()-1;

	if(max_index == 0) return tp.possible_textures.front();

	//Sélection d'une texture.
	if(tp.textures_distr_type == NORMAL_DISTR)
	{
		typedef std::normal_distribution<float>::param_type N_p_type;

		float mean_index = float(max_index)/2.f;
		float stddev = float(max_index)/4.f;

		m_norm_distrs.at(7).setParam(N_p_type(mean_index, stddev));
		index = gs::roundToInt(m_norm_distrs.at(7).get());
	}
	else
	if(tp.textures_distr_type == UNIF_DISTR)
	{
		typedef std::uniform_real_distribution<float>::param_type U_p_type;

		m_unif_distrs.at(7).setParam( U_p_type(0, float(max_index)) );
		index = gs::roundToInt(m_unif_distrs.at(7).get());
	}

	//Gestion des valeurs invalides.
	if(index < 0)
		index = -index;
	else
	if(index > max_index)
		index = max_index - (index - max_index);

	if(index < 0)
		index = max_index;
	else
	if(index > max_index)
		index = 0;

	try{
		return tp.possible_textures.at(index);
	}
	catch(std::exception& err)
	{
		gs::lastError("INTERNAL - In 'Particle_Box<P_User_Data_T>::getTexture()' : tp.possible_textures indexing error ? ");
	}

	return nullptr;
}

template<class P_User_Data_T> Rect Particle_Box<P_User_Data_T>::
getSrcRect(const Particle_Texture_Param& tp)
{
	Rect default_src={0,0, 1000000, 1000000};

	if(tp.possible_src_rects.empty())
		return default_src;


	int index = 0;
	int max_index = tp.possible_src_rects.size()-1;

	if(max_index == 0) return tp.possible_src_rects.front();

	typedef std::normal_distribution<float>::param_type N_p_type;
	typedef std::uniform_real_distribution<float>::param_type U_p_type;


	//Sélection d'un rectangle source.
	if(tp.src_rects_distr_type == NORMAL_DISTR)
	{
		float mean_index = float(max_index)/2.f;
		float stddev = float(max_index)/4.f;

		m_norm_distrs.at(7).setParam(N_p_type(mean_index, stddev));
		index = gs::roundToInt(m_norm_distrs.at(7).get());
	}
	else
	if(tp.src_rects_distr_type == UNIF_DISTR)
	{
		m_unif_distrs.at(7).setParam(U_p_type(0, float(max_index)));
		index = gs::roundToInt(m_unif_distrs.at(7).get());
	}

	if(index < 0)
		index = -index;
	else
	if(index > max_index)
		index = max_index - (index - max_index);

	if(index < 0)
		index = max_index;
	else
	if(index > max_index)
		index = 0;

	try{
		return tp.possible_src_rects.at(index);
	}
	catch(std::exception& err)
	{
		gs::lastError("INTERNAL - In 'Particle_Box<P_User_Data_T>::getSrcRect()' : tp.possible_src_rects indexing error ? ");
	}

	return default_src;
}


template<class P_User_Data_T> Vec2f Particle_Box<P_User_Data_T>::
getRotAndRotSpeed(const Particle_Texture_Param& tp)
{
	typedef std::normal_distribution<float>::param_type N_p_type;
	typedef std::uniform_real_distribution<float>::param_type U_p_type;


	Vec2f res={0,0};

	char i = 0;
	for(const Particle_Texture_Param::Field* field : {&tp.rotation, &tp.rotation_speed})
	{
		float val = 0;
		float a = field->a;
		float b = field->b;

		if(field->distr_type == NORMAL_DISTR)
		{
			m_norm_distrs.at(7).setParam(N_p_type(a, b));
			val = m_norm_distrs.at(7).get();
		}
		else
		if(field->distr_type == UNIF_DISTR)
		{
			m_unif_distrs.at(7).setParam(U_p_type(a, b));
			val = m_unif_distrs.at(7).get();
		}

		if(i == 0)
			res.x = val;
		else
		if(i == 1)
			res.y = val;

		++i;
	}

	return res;
}



template<class P_User_Data_T> void Particle_Box<P_User_Data_T>::
addParticle(int count, Particle_Param* target_param, Particle_T type, const Particle_Color_Param* color_param)
{
	if(count <= 0) return;

	Particle_Param* param_ptr = (target_param ? target_param : &m_param);
	if(not param_ptr) return;

	Particle_Param& param = *param_ptr;

	//Ajustement des paramèrtes.
	adjustParam(param);


	//Ajout des particules.
	for(int i=0; i<count; ++i)
	{
		m_particles.emplace_back();
		Particle<P_User_Data_T>& curr_part = m_particles.back();

		curr_part.type = type;

		curr_part.pos.x = getRandVal(0, param);
		curr_part.pos.y = getRandVal(1, param);

		curr_part.size.x = (unsigned int)(getRandVal(2, param));
		curr_part.size.y = (unsigned int)(getRandVal(3, param));

		curr_part.dir = getRandVal(4, param);
		curr_part.speed = getRandVal(5, param);

		curr_part.setLeftTime(getRandVal(6, param));

		if(color_param)
			curr_part.color = getColor(*color_param);
		else
			curr_part.color = getColor(m_col_param);
	}
}



template<class P_User_Data_T> void Particle_Box<P_User_Data_T>::
addParticle(const Particle<P_User_Data_T>& particle, int count)
{
	if(count <= 0) return;

	m_particles.insert(m_particles.end(), count, particle);
}


template<class P_User_Data_T> void Particle_Box<P_User_Data_T>::
addTexturedParticle(int count, Particle_Param* target_param, const Particle_Texture_Param& texture_param)
{
	if(count <= 0) return;

	Particle_Param* param_ptr = (target_param ? target_param : &m_param);
	if(not param_ptr) return;

	Particle_Param& param = *param_ptr;

	//Ajustement des paramèrtes.
	adjustParam(param);


	//Ajout des particules.
	for(int i=0; i<count; ++i)
	{
		m_t_particles.emplace_back();
		Textured_Particle<P_User_Data_T>& curr_part = m_t_particles.back();

		curr_part.type = TEXTURED_PARTICLE;

		curr_part.pos.x = getRandVal(0, param);
		curr_part.pos.y = getRandVal(1, param);

		curr_part.size.x = (unsigned int)(getRandVal(2, param));
		curr_part.size.y = (unsigned int)(getRandVal(3, param));

		curr_part.dir = getRandVal(4, param);
		curr_part.speed = getRandVal(5, param);

		curr_part.setLeftTime(getRandVal(6, param));

		curr_part.texture = getTexture(texture_param);
		curr_part.src_rect = getSrcRect(texture_param);

		Vec2f rot_rotspeed = getRotAndRotSpeed(texture_param);
		curr_part.rotation = rot_rotspeed.x;
		curr_part.rotation_speed = rot_rotspeed.y;
	}
}


template<class P_User_Data_T> void Particle_Box<P_User_Data_T>::
addTexturedParticle(const Textured_Particle<P_User_Data_T>& particle, int count)
{
	if(count <= 0) return;

	m_t_particles.insert(m_t_particles.end(), count, particle);
}


template<class P_User_Data_T> void Particle_Box<P_User_Data_T>::
update(float dt)
{
	typename std::vector<Particle<P_User_Data_T>>::iterator it=m_particles.end();
	typename std::vector<Particle<P_User_Data_T>>::iterator begin_it=m_particles.begin();


	while(it != begin_it)
	{
		it--; //Parcours à reculons pour faciliter les suppressions. + Cette décrémentation est au début car on commence par end().

		if(it->left_time <= 0)
		{
			it->~Particle();
			m_particles.erase(it);
		}
		else
		{
			if(m_particleColorUpdater)
				m_particleColorUpdater(*it, it->color);

			if(m_particleBehaviorUpdater)
			{
				m_particleBehaviorUpdater(*it, dt);
				it->left_time -= dt;
			}
			else
				it->update(dt);
		}
	}


	typename std::vector<Textured_Particle<P_User_Data_T>>::iterator it2=m_t_particles.end();
	typename std::vector<Textured_Particle<P_User_Data_T>>::iterator begin_it2=m_t_particles.begin();

	while(it2 != begin_it2)
	{
		it2--;

		if(it2->left_time <= 0)
		{
			it2->~Textured_Particle();
			m_t_particles.erase(it2);
		}
		else
		{
			if(m_t_particleBehaviorUpdater)
			{
				m_t_particleBehaviorUpdater(*it2, dt);
				it2->left_time -= dt;
			}
			else
				it2->update(dt);
		}
	}
}

template<class P_User_Data_T> void Particle_Box<P_User_Data_T>::
show(Renderer* ren)
{
	for(Particle<P_User_Data_T>& part : m_particles)
	{
		part.show(ren);
	}

	for(Textured_Particle<P_User_Data_T>& part : m_t_particles)
	{
		part.show(ren);
	}
}



template<class P_User_Data_T> void Particle_Box<P_User_Data_T>::
setParticleColorUpdater(void (*particleColorUpdater)(const Particle<P_User_Data_T>& particle, Color& particle_color))
{
	m_particleColorUpdater = particleColorUpdater;
}

template<class P_User_Data_T> void Particle_Box<P_User_Data_T>::
setParticleBehaviorUpdater(void (*particleBehaviorUpdater)(Particle<P_User_Data_T>& particle, float dt))
{
	m_particleBehaviorUpdater = particleBehaviorUpdater;
}


template<class P_User_Data_T> void Particle_Box<P_User_Data_T>::
setTParticleBehaviorUpdater(void (*tParticleBehaviorUpdater)(Textured_Particle<P_User_Data_T>& particle, float dt))
{
	m_t_particleBehaviorUpdater = tParticleBehaviorUpdater;
}



template<class P_User_Data_T> uint32_t Particle_Box<P_User_Data_T>::
getCount() const
{
	return m_particles.size() + m_t_particles.size();
}


template<class P_User_Data_T> uint32_t Particle_Box<P_User_Data_T>::
getCount(Particle_T type) const
{
	if(type == TEXTURED_PARTICLE)
		return m_t_particles.size();

	uint32_t count = 0;
	for(const Particle<P_User_Data_T>& p : m_particles)
		if(p.type == type)
			++count;

	return count;
}



} //namespace gs
