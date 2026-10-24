#include "GSlib.hpp"
#include "GSlib-Geo.hpp"
#include <random>
#include <bitset>
#include <stack>




//Version modifiée le 22/02/26 (à moins d'oubli de modifier cette ligne ou autre).

gs::Game *ThisGame = nullptr;



std::vector<gs::Texture*> texturesToClearAtTheEnd{};
std::vector<gs::Renderer*> renderersToClearAtTheEnd{};



std::ostream& operator<<(std::ostream& os, gs::Rect& rect)
{
	os << "["<<rect.x<<"| "<<rect.y<<"| "<<rect.w<<"| "<<rect.h<<"]";
	return os;
}

std::ostream& operator<<(std::ostream& os, gs::FRect& rect)
{
	os << "["<<rect.x<<"| "<<rect.y<<"| "<<rect.w<<"| "<<rect.h<<"]";
	return os;
}

std::ostream& operator<<(std::ostream& os, gs::Point& point)
{
	os << "("<<point.x<<"| "<<point.y<<")";
	return os;
}

std::ostream& operator<<(std::ostream& os, gs::FPoint& point)
{
	os << "("<<point.x<<"| "<<point.y<<")";
	return os;
}

std::ostream& operator<<(std::ostream& os, gs::Color& col)
{
	os << "[r "<<int(col.r)<<"| g "<<int(col.g)<<"| b "<<int(col.b)<<"| a "<<int(col.a)<<"]";
	return os;
}

bool operator==(const gs::Rect& rect1, const gs::Rect& rect2)
{
	return
	((rect1.x == rect2.x) && (rect1.y == rect2.y)
	&& (rect1.w == rect2.w) && (rect1.h == rect2.h));
}

bool operator==(const gs::FRect& rect1, const gs::FRect& rect2)
{
	return
	((rect1.x == rect2.x) && (rect1.y == rect2.y)
	&& (rect1.w == rect2.w) && (rect1.h == rect2.h));
}

bool operator==(const gs::Point& pt1, const gs::Point& pt2)
{
	return ( (pt1.x == pt2.x) && (pt1.y == pt2.y) );
}

bool operator==(const gs::FPoint& pt1, const gs::FPoint& pt2)
{
	return ( (pt1.x == pt2.x) && (pt1.y == pt2.y) );
}

bool operator==(const gs::Color& col1, const gs::Color& col2)
{
	return
	((col1.r == col2.r) && (col1.g == col2.g)
	&& (col1.b == col2.b) && (col1.a == col2.a));
}








namespace gs
{

void ________________FONCTION2_____(){}
void ________________FONCTION3_____(){}

bool game_started = false;
bool game_won = false;
bool end_game = false;
bool show_comments = false;
std::string _lastError_;

bool get_show_comments() {return show_comments;}

void initFreeResources()
{
	std::atexit(freeGivenResources);
}


bool& Input::operator[](std::string id)
{
	std::unordered_map<std::string, bool>::iterator it = m_inputs.find(id);
	if(it != m_inputs.end())
		return it->second;

	m_inputs[id] = false;
	return m_inputs[id];
}





bool Input::operator[](std::string id, bool new_state)
{
	m_inputs[id] = new_state;
	return new_state;
}









bool Mouse_Tracker::isPreviouslyDown() const {return m_previously_down;}

bool Mouse_Tracker::isDown() const {return m_is_down;}

Vec2f Mouse_Tracker::getPosWhenDown() const {return gs::Vec2f(m_pos_when_down.x, m_pos_when_down.y);}

Vec2f Mouse_Tracker::getPosWhenUp() const {return gs::Vec2f(m_pos_when_up.x, m_pos_when_up.y);}

Vec2f Mouse_Tracker::getPos() const {return m_pos;}

bool Mouse_Tracker::moved() const {return m_prev_pos != m_pos;}

bool Mouse_Tracker::last2DownClicksHaveSamePos() const {return m_prev_down_pos.x == m_pos_when_down.x and m_prev_down_pos.y == m_pos_when_down.y;}

bool Mouse_Tracker::isMultiClick(char* out__clicks) const
{
	if(out__clicks)
		*out__clicks=m_clicks_count;

	return m_is_multi_click;
}

bool Mouse_Tracker::isDoubleClick() const {return m_is_multi_click and m_clicks_count==2;}

void Mouse_Tracker::incrMultiClickCount(char val)
{
	m_clicks_count += val;

	if(m_clicks_count < 0)
		m_clicks_count = 0;
}


void Mouse_Tracker::setTargetBtn(bool is_left) {m_target_btn_is_left = is_left;}

bool Mouse_Tracker::targetBtnIsLeft() const {return m_target_btn_is_left;}

bool Mouse_Tracker::clickedDown() const {return m_is_down and not m_previously_down;}

bool Mouse_Tracker::clickedUp() const {return not m_is_down and m_previously_down;}

Vec2f Mouse_Tracker::getWheel() const {return m_wheel;}





void Mouse_Tracker::update()
{
	if(m_next_time_operation)
	{
		if(m_next_time_operation == 1)
			m_previously_down = true;
		else
		if(m_next_time_operation == 2)
			m_previously_down = false;

		m_next_time_operation = 0;
	}

	if(m_is_down and not m_previously_down)
		m_next_time_operation = 1;
	else
	if(not m_is_down and m_previously_down)
		m_next_time_operation = 2;


	m_prev_pos = m_pos;
	getMousePos(&m_pos.x, &m_pos.y); //Pour la position courante

	m_wheel.x = m_wheel_buff.x;
	m_wheel.y = m_wheel_buff.y;
	m_wheel_buff.x = 0.f;
	m_wheel_buff.y = 0.f;
}







//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//___________________________FONCTIONS  DE  GAME______________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________


Game::Game()
{
	// initFreeResources();
	std::atexit(freeGivenResources);
};


//____________________________________________________________________________________


Game::~Game()
{
	backendDestr();

	if(show_comments)
	std::cout<<"|______End Game succes______|" << std::endl;
};


//----------------------------------------------
float Game::getRenW_f()
{
	return float(getRenW());
}
//----------------------------------------------
float Game::getRenH_f()
{
	return float(getRenH());
}

//----------------------------------------------
std::string Game::lastError() const { return lastError();};


//____________________________________________________________________________________


void Game::lastError(const std::string& str_info, const std::string& str_info_2, const std::string& str_info_3) {lastError(str_info, str_info_2, str_info_3);}

void Game::setWinSize(int w, int h) { Win_h = h; Win_w = w; }



//____________________________________________________________________________________
//____________________________________________________________________________________



Texture* Game::loadTexture(const std::string& pth, bool nearest)
{
	if(not isInit)
	{
		gs::lastError("||Warning|| in 'Game::loadTexture()', not yet init. ");
		return nullptr;
	}

	Texture* res = ::gs::loadTexture(ren_, pth, nearest);
	if(res == nullptr)
	{
		gs::lastError(gs::str("from 'gs::Game::loadTexture()' : ") +gs::lastError());
	}
	return res;
};



//____________________________________________________________________________________
//____________________________________________________________________________________



bool Game::drawTile(Texture *tileset, FRect src, FRect dest, bool isRenderTheTarget)
{
	return gs::drawTile(ren_, nullptr, tileset, src, dest, isRenderTheTarget);
};


//____________________________________________________________________________________
//____________________________________________________________________________________


void Game::addToClearAtTheEnd(Texture* tex)
{
	::gs::addToClearAtTheEnd(tex);
}

//____________________________________________________________________________________
//____________________________________________________________________________________


void Game::addToClearAtTheEnd(Renderer* renderer)
{
	::gs::addToClearAtTheEnd(renderer);
}



//____________________________________________________________________________________
//____________________________________________________________________________________



//____________________________________________________________________________________
//____________________________________________________________________________________


//____________________________________________________________________________________
//____________________________________________________________________________________


void Game::setAsMainGame()
{
	is_main_game = true;
}

//____________________________________________________________________________________
//____________________________________________________________________________________


void Game::setIsMainGame(const bool b)
{
	is_main_game = b;
}


void Game::setUserData(void* data)
{
	m_user_data = data;
}

void* Game::getUserData()
{
	return m_user_data;
}












//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//___________________________________UTILS____________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________







//===================================================================================//






//____________________________________________________________________________________
//____________________________________________________________________________________


void logMessage(const char *msg, const char *_getError, bool isFatalError)
{
	if(_getError != nullptr)
		std::cout << msg << " : " << _getError << std::endl;
	else
		std::cout << msg << std::endl;

	if(isFatalError)
		exit(-1);
}

//____________________________________________________________________________________
//____________________________________________________________________________________


//____________________________________________________________________________________
//____________________________________________________________________________________


void addToClearAtTheEnd(Texture* tex)
{
	addToClearAtTheEnd_backend(tex, (void*)(&texturesToClearAtTheEnd));
}


void addToClearAtTheEnd(Renderer* ren)
{
	for(Renderer* r : renderersToClearAtTheEnd)
		if(r == ren)
			return;
	if(ren)
	renderersToClearAtTheEnd.push_back(ren);
}




void freeGivenResources()
{
	if(show_comments)
	std::cout << "\n|______Free ress______|\n";

	for(Texture* tex : texturesToClearAtTheEnd)
	{
		if(tex)
		{
			if(show_comments)
			{
				//std::string name;
				//if((char*)GetTextureUserData(tex))
				//	name = (char*)GetTextureUserData(tex);
				//else
				//	name = "(cannot get title)";

				//std::printf("- texture : info -> %s, addr -> %p\n", name.c_str(), tex);
				std::printf("\t- texture : addr -> %p\n", tex);
			}

			destroyTexture(tex);
		}
		/*else std::printf("*\n");*/
	}
	texturesToClearAtTheEnd.clear();
	texturesToClearAtTheEnd.shrink_to_fit();

	#ifndef GSLIB_RAYLIB_BACKEND
	for(Renderer* renderer : renderersToClearAtTheEnd)
	{
		if(renderer)
		{
			if(show_comments)
			{
				//std::printf("- renderer : info -> %s, addr -> %p\n", getRendererName(renderer), renderer);
				std::printf("- renderer : addr -> %p\n", renderer);
			}

			destroyRenderer(renderer);
		}
	}
	#endif
	renderersToClearAtTheEnd.clear();
	renderersToClearAtTheEnd.shrink_to_fit();


	//for(PixelFormat* fmt : pixelFormatsToClearAtTheEnd)
	//{
	//	if(fmt)
	//	{
	//		if(show_comments)
	//			std::printf("- pixel format, addr -> %p\n", fmt);

	//		FreeFormat(fmt);
	//	}
	//}
	//pixelFormatsToClearAtTheEnd.clear();
	//pixelFormatsToClearAtTheEnd.shrink_to_fit();

	if(show_comments)
	std::cout << "|____Free ress end____|\n";
}


//____________________________________________________________________________________
//____________________________________________________________________________________


float rad(float degre)
{
	return ( degre * 2.0*_pi_) / 360.0;
};

/*-------------------------------*/

float degree(float rad)
{
	return ( (rad * 360.0) / (2.0*_pi_) );
};

/*-------------------------------*/

int roundToInt(float x)
{
	int partie_entiere = (int) x;
	float partie_deci = x - (float) partie_entiere;

	if(partie_deci >= 0.5)
		partie_entiere ++;
	return partie_entiere;
}

/*-------------------------------*/

float paw2(float x)
{
	return x*x;
}


int paw2(int x)
{
	return x*x;
}


std::string lastError()
{
	return _lastError_;
}


void lastError(const std::string& str_info, const std::string& str_info_2, const std::string& str_info_3)
{
	_lastError_ = str_info+str_info_2+str_info_3;
}


bool lastErrorAutomaticLog()
{
	static std::string previous_last_error;

	if(_lastError_ != previous_last_error)
	{
		std::cout << "\n________" << _lastError_ << "\n";
		previous_last_error = _lastError_;
		return true;
	}

	return false;
}




bool isInInterval_in_in(float min, float max, float val)
{
	/*on teste si : "val ∈ [min ; max]" est vrai*/
	return min <= val && val <= max;
}


bool isInInterval_in_out(float min, float max, float val)
{
	/*on teste si : "val ∈ [min ; max[" est vrai*/
	return min <= val && val < max;
}


bool isInInterval_out_in(float min, float max, float val)
{
	/*on teste si : "val ∈ ]min ; max]" est vrai*/
	return min < val && val <= max;
}


bool isInInterval_out_out(float min, float max, float val)
{
	/*on teste si : "val ∈ ]min ; max[" est vrai*/
	return min < val && val < max;
}




float euclideanDist(float x1, float y1, float x2, float y2){return std::sqrt(  (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)  );}
int euclideanDist(int x1, int y1, int x2, int y2){return std::sqrt(  (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2)  );}


float pointsDist(float x1, float y1, float x2, float y2){return euclideanDist(x1, y1, x2, y2);}
int pointsDist(int x1, int y1, int x2, int y2){return euclideanDist(x1, y1, x2, y2);}


float pointsDist(Vec2f pt1, Vec2f pt2){return euclideanDist(pt1.x, pt1.y, pt2.x, pt2.y);}
int pointsDist(Vec2i pt1, Vec2i pt2){return euclideanDist(pt1.x, pt1.y, pt2.x, pt2.y);}

float manhattanDist(Vec2f pt1, Vec2f pt2) {return std::abs(pt2.x - pt1.x) + std::abs(pt2.y - pt1.y);}
int manhattanDist(Vec2i pt1, Vec2i pt2) {return std::abs(pt2.x - pt1.x) + std::abs(pt2.y - pt1.y);}


float meanXY(Vec2f v) {return (v.x+v.y)/2.f;}
float meanXY(Vec2i v) {return float(v.x+v.y)/2.f;}


float getPointsAngle(float origin_x, float origin_y, float point_x, float point_y, bool clockwise)
{
	//Retourne l'angle entre le point donné (P), l'origine donnée (O), et un point (Z) quelconque sur l'axe des abscisses à droite de l'origine.
	//
	//-------O--------Z-------
	//
	//          P
	//         _
	// renvoi POZ.

	if(origin_y == point_y)
	{
		if(origin_x <= point_x) //Angle nul.
			return 0;
		return M_PI; //Angle plat.
	}

	gs::Vec2f vector = {point_x-origin_x, point_y-origin_y};

	//==========================================================//
		// Formule :
		// << cah soh toa >>
		//
		// tan(angle) == opposé / adjacent
		// angle      == arctan(opposé / adjacent)
		//
		// adjacent == vector.x
		// opposé == vector.y
		//
		//
		//         _vy
		//       _v  y
		//     _v    y
		//   _v      y
		// _v a      y
		// oxxxxxxxxxy
		//
		// o : origine
		// a : angle
		// v : vecteur
		// x : adjacent
		// y : opposé
		//
	//==========================================================//

	//En radian
	float angle = std::atan(vector.y/vector.x);

	//Ajustement puisque le triangle rectangle formé n'a pas toujours la même position.
	if(point_x < origin_x)
		angle += M_PI; //En ce qui concerne '+= pi', je l'ai trouvé à tâtons.

	//Facultatif. Afin d'avoir un angle toujours sur [0; 2pi].
	if(angle < 0)
		angle = 2*M_PI + angle; // '+' car 'angle' est négatif.

	if(not clockwise)
		angle = 2*M_PI - angle;

	return angle;
}



float getPointsAngle(Vec2f origin, Vec2f point, bool clockwise)
{
	return getPointsAngle(origin.x, origin.y, point.x, point.y, clockwise);
}


float meanAngle(float angle_a, float angle_b)
{
	Vec2f v1 = gs::vec(angle_a);
    Vec2f v2 = gs::vec(angle_b);
    Vec2f s = v1+v2;
    return getPointsAngle(Vec2f(0.f, 0.f), s);
}

float getVecAngle(Vec2f vec) {return getPointsAngle(Vec2f(0.f, 0.f), vec);}




gs::Vec2f rotatePoint(float center_x, float center_y, float point_x, float point_y, float angle_rad, bool clockwise)
{
	float sign = (clockwise ? 1 : -1);

	float new_angle = getPointsAngle(center_x, center_y, point_x, point_y, true) + angle_rad*sign;

	float dist = gs::euclideanDist(center_x, center_y, point_x, point_y);

	gs::Vec2f vector;
	vector.x = std::cos(new_angle) * dist;
	vector.y = std::sin(new_angle) * dist;

	gs::Vec2f center;
	center.x = center_x;
	center.y = center_y;

	return center + vector;
}



int sign(float val)
{
	if(val < 0.f)
		return -1;
	return 1;
}

int sign(int val)
{
	if(val < 0)
		return -1;
	return 1;
}


Vec2f orthoVecOf(Vec2f vector_)
{
	return Vec2f(-vector_.y, vector_.x);
}

Vec2i orthoVecOf(Vec2i vector_)
{
	return Vec2i(-vector_.y, vector_.x);
}

Vec2i orthoVecOf_i(Vec2f vector_)
{
	return orthoVecOf(vector_).to<int>();
}

Vec2f orthoVecOf_d(Vec2i vector_)
{
	return orthoVecOf(vector_).to<float>();
}


Vec2f vec(float angle) {return Vec2f(std::cos(angle), std::sin(angle));}

float vecLen(Vec2f vec){return std::sqrt(vec.x*vec.x + vec.y*vec.y);}

float vecLen(Vec2i vec){return vecLen(vec.to<float>());}


Vec2f normalize(Vec2f vec)
{
	float len = vecLen(vec);
	if(len)
		return vec/len;
	return vec;
}

Vec2f normalize(Vec2i vec) {return normalize(vec.to<float>());}

Vec2f resizeVec(Vec2f vec, float new_length) {return normalize(vec)*new_length;}

bool vecsPointingToSameSide(Vec2f vec1, Vec2f vec2) {return dotProd(vec1, vec2) > 0.f;}



bool lineAndCircleIntersect(float line_a, float line_b, float line_c,
	Vec2f cir_center, float cir_radius, Vec2f* out__pt1, Vec2f* out__pt2)
{
	//Intersection entre droite et cercle.
	const float  &a=line_a,  &b=line_b,  &c=line_c, &m=cir_center.x,  &n=cir_center.y,  &r=cir_radius;

	float A = a*a + b*b;
	float B = (-2)*b*b*m + 2*a*(c+b*n);
	float C = b*b*m*m + (c+b*n)*(c+b*n) - b*b*r*r;

	if(A!=0)
	{
		float x1 = ((-B)-std::sqrt(B*B - 4*A*C))/(2*A);
		float x2 = ((-B)+std::sqrt(B*B - 4*A*C))/(2*A);

		if(b != 0)
		{
			if(out__pt1)
			{
				out__pt1->x = x1;
				out__pt1->y = (-a*x1-c)/b;
			}

			if(out__pt2)
			{
				out__pt2->x = x2;
				out__pt2->y = (-a*x2-c)/b;
			}

			return true;
		}
	}

	return false;
}

bool lineAndCircleIntersect_i(float line_a, float line_b, float line_c,
	Vec2f cir_center, float cir_radius, Vec2i* out__pt1, Vec2i* out__pt2)
{
	Vec2f out1, out2;
	bool res = lineAndCircleIntersect(line_a, line_b, line_c, cir_center, cir_radius, &out1, &out2);

	if(out__pt1)
		(*out__pt1) = out1.to<int>();
	if(out__pt2)
		(*out__pt2) = out2.to<int>();

	return res;
}





Vec2f orthoProj(Vec2f point, float line_a, float line_b, float line_c)
{
	//Projeté orthogonal du point (m,n) sur la droit d'équation ax + by + c.
	const float  &a=line_a,  &b=line_b,  &c=line_c,  &m=point.x,  &n=point.y;

	Vec2f orth_proj;

	if((-a*a-b*b) != 0 and b != 0)
	{
		orth_proj.x = (c*a +b*(-b*m +a*n))/(-a*a-b*b);
		orth_proj.y = (-a*((c*a +b*(-b*m +a*n))/(-a*a-b*b))-c)/b;
	}

	return orth_proj;
}

Vec2i orthoProj_i(Vec2i point, float line_a, float line_b, float line_c)
{
	return orthoProj(point.to<float>(), line_a, line_b, line_c).to<int>();
}







bool isInField(Vec2f field_origin, float fov, float dir_rad, float field_radius,
				Vec2f circ_origin, float circ_radius,
				Vec2f* out__closest_pt, float* out__origin_proximity)
{
	//Cette fonction vérifie la présence d'un disque/cercle (entité) dans un morceau de disque (champ de vision, zone d'action ...).
	//Le morceau de disque (ou disque complet) est déterminé par : une position d'origine (centrale), un angle (l'ouverture -> fov), une direction centrale associée à un sens (un angle), et un étendu (rayon du champ).
	//Ce morceau peut être assimilé à une part de gateau.

	bool check_extra = (out__closest_pt!=nullptr) or (out__origin_proximity!=nullptr);
	gs::Vec2f closest_pt;
	gs::Vec2f pt1 = circ_origin, pt2=circ_origin;
	bool is_in_field = false;

	////Vérification de la présence du cercle dans le rayon du field.
	////
	////
	int centers_dist = gs::pointsDist(field_origin, circ_origin);

	//Cas particulier: si l'origine du champ est dans le cercle (notamment [mais pas forcément] si le cercle est plus grand que le champ).
	if(centers_dist < circ_radius)
	{
		if(check_extra)
			closest_pt = field_origin;

		is_in_field = true;
	}
	else
	if(centers_dist < (field_radius+circ_radius)) //Si les deux sont dans le rayon de l'un et l'autre.
	{
		////Important : ici le cercle est dans le rayon du field.
		////Vérification de la présence du centre du cercle dans l'angle de vu.
		////
		////
		float dir = dir_rad;
		if(not gs::isInInterval_in_in(0, 2.0*_pi_, dir))
		{
			//Cette projection permet de déterminer la direction du champ formaté dans l'interval [0; 2pi].
			gs::Vec2f projection_pt = {field_origin.x +std::cos(dir)*10.f,
									field_origin.y +std::sin(dir)*10.f};

			dir = gs::getPointsAngle(field_origin.x, field_origin.y, projection_pt.x, projection_pt.y, true);
		}

		float angle_with_circ = gs::getPointsAngle(field_origin.x, field_origin.y, circ_origin.x, circ_origin.y, true);

		float min_dir=dir-fov/2.0; //Direction de la bordure minimale du field.
		float max_dir=dir+fov/2.0; //Direction de la bordure maximale du field.
		if(min_dir<0) //Les réglages suivants, nécessaires au bon fonctionnement de l'implémentation, ont été trouvés de manière expérimentale, empiriquement.
		{
			min_dir+=2.0*_pi_; //On les rajoute un tour, pour éviter les valeurs négatives.
			max_dir+=2.0*_pi_;

			if(angle_with_circ < _pi_) //Si l'angle est dans la première moité du cercle trigonométrique.
				angle_with_circ += 2.0*_pi_; //On le trourne aussi afin qu'il soit au "même tour" que min_dir (pour avoir un résultat correct).
		}
		else
		if(angle_with_circ < _pi_ and angle_with_circ < min_dir)
			angle_with_circ += 2.0*_pi_; //On le trourne afin qu'il soit au "même tour" que min_dir (pour avoir un résultat correct).

		if(gs::isInInterval_in_in(min_dir, max_dir, angle_with_circ)) //Enfin, si le centre du cercle est dans l'angle du field.
		{
			if(check_extra)
			{
				closest_pt.x = field_origin.x+std::cos(angle_with_circ)*(centers_dist-circ_radius);
				closest_pt.y = field_origin.y+std::sin(angle_with_circ)*(centers_dist-circ_radius);
			}

			is_in_field = true; //Ainsi si le cercle est (partiellement ou totalement) dans le rayon du field et est dans l'angle du field, alors le cercle est dans le field (partiellement ou totalement).
		}
		else
		{
			//Sinon, il se peut que malgré le fait que le centre ne soit pas dans l'angle du field, un partie du cercle soit tout de même dans le field.
			//Alors on vérifie l'intersection entre le cercle et les bordures du field.

			gs::Line_Segment field_min_edge(field_origin, {field_origin.x+std::cos(min_dir)*field_radius, field_origin.y+std::sin(min_dir)*field_radius});
			gs::Line_Segment field_max_edge(field_origin, {field_origin.x+std::cos(max_dir)*field_radius, field_origin.y+std::sin(max_dir)*field_radius});

			gs::Circle circle(circ_origin, circ_radius);
			bool edge = false;

			//Teste si le cercle touche la bordure minimale du champ.
			gs::Vec2b res = field_min_edge.intersect(circle, &pt1, &pt2);
			if(res.x or res.y)
			{
				edge = true;
			}
			else
			{
				//Teste si le cercle touche la bordure maximale du champ.
				res = field_max_edge.intersect(circle, &pt1, &pt2);
				if(res.x or res.y)
					edge = true;
			}

			if(edge and check_extra)
			{
				float dist_pt1 = gs::pointsDist(field_origin, pt1);
				float dist_pt2 = gs::pointsDist(field_origin, pt2);

				if(dist_pt2 < dist_pt1)
					closest_pt = pt2;
				else
					closest_pt = pt1;
			}

			is_in_field = edge;
		}


	}

	//Si le cercle n'est pas au moins dans le rayon du field, alors il ne s'y trouve pas.

	if(is_in_field and check_extra)
	{
		if(out__closest_pt)
			(*out__closest_pt) = closest_pt;

		if(out__origin_proximity)
		{
			if(field_radius==0)
				(*out__origin_proximity) = 1.0;
			else
				(*out__origin_proximity) = 1.0-gs::pointsDist(field_origin, closest_pt)/field_radius;
		}
	}

	return is_in_field;
}



bool isCirclesCollision(Vec2f center1, float radius1, Vec2f center2, float radius2)
{
	float centers_dist = gs::pointsDist(center1, center2);
	return centers_dist < radius1+radius2;
}
bool isCirclesCollision(Circle_Hitbox circ, Vec2f center2, float radius2)
{
	return isCirclesCollision(circ.center, circ.radius, center2, radius2);
}
bool isCirclesCollision(Circle_Hitbox circ1, Circle_Hitbox circ2)
{
	return isCirclesCollision(circ1.center, circ1.radius, circ2.center, circ2.radius);
}






float decimalPart(float val)
{
	float int_part = float(int(val));
	return val - int_part;
}

double decimalPart(double val)
{
	double int_part = double(int(val));
	return val - int_part;
}




float changeRange(float val, float origin_min, float origin_max, float target_min, float target_max)
{
	return target_min + (val - origin_min)/(origin_max - origin_min) * (target_max - target_min);
};


float random(float min, float max) //uniforme
{
	static std::random_device rd;
	static std::default_random_engine re(rd());
	static std::uniform_real_distribution<float> distrib{
		std::uniform_real_distribution<float>::param_type{0.f, 1.f}
	};

	float val = distrib(re);
	return changeRange(val, 0.f, 1.f, min, max);
}




FColor normalizeColor(FColor color_0_255)
{
	color_0_255.r/=255.f;
	color_0_255.g/=255.f;
	color_0_255.b/=255.f;
	color_0_255.a/=255.f;
	return color_0_255; //Qui ne porte plus bien son nom maintenant ^^
}


FColor normalizeColor(Color color_0_255)
{
	return normalizeColor(fcolor(color_0_255));
}



Color normalizedColorTo255(FColor color_0_1)
{
	Color c;
	c.r = static_cast<uint8_t>(color_0_1.r * 255.f);
	c.g = static_cast<uint8_t>(color_0_1.g * 255.f);
	c.b = static_cast<uint8_t>(color_0_1.b * 255.f);
	c.a = static_cast<uint8_t>(color_0_1.a * 255.f);
	return c;
}

FColor normalizedColorTo255_f(FColor color_0_1)
{
	return fcolor(normalizedColorTo255(color_0_1));
}





Color lerpColors(Color begin_col, Color end_col, float t)
{
	t = std::clamp(t, 0.f, 1.f);
	Color c;
	c.r = uint8_t(std::lerp(float(begin_col.r), float(end_col.r), t));
	c.g = uint8_t(std::lerp(float(begin_col.g), float(end_col.g), t));
	c.b = uint8_t(std::lerp(float(begin_col.b), float(end_col.b), t));
	c.a = uint8_t(std::lerp(float(begin_col.a), float(end_col.a), t));
	return c;
}




Color color(int r, int g, int b, int a)
{
	bool have_out_of_range_value = false;
	int given_r=r, given_g=g, given_b=b, given_a=a;

	if(r < 0) { r = 0; have_out_of_range_value = true; }
	if(g < 0) { g = 0; have_out_of_range_value = true; }
	if(b < 0) { b = 0; have_out_of_range_value = true; }
	if(a < 0) { a = 0; have_out_of_range_value = true; }

	if(r > 255) { r = 255; have_out_of_range_value = true; }
	if(g > 255) { g = 255; have_out_of_range_value = true; }
	if(b > 255) { b = 255; have_out_of_range_value = true; }
	if(a > 255) { a = 255; have_out_of_range_value = true; }

	uint8_t uint8_r = static_cast<uint8_t>(r);
	uint8_t uint8_g = static_cast<uint8_t>(g);
	uint8_t uint8_b = static_cast<uint8_t>(b);
	uint8_t uint8_a = static_cast<uint8_t>(a);

	if(have_out_of_range_value)
	{
		gs::lastError(gs::str("||Error|| in 'gs::color(4 int)', have out of range value-s. Values must be in [0 ; 255]. ")
					+ gs::str("[given values : r ")+std::to_string(given_r)+gs::str(", ")
					+ gs::str("g ")+std::to_string(given_g)+gs::str(", ")
					+ gs::str("b ")+std::to_string(given_b)+gs::str(", ")
					+ gs::str("a ")+std::to_string(given_a)+gs::str("]. ")
					);
	}

	Color _color = {uint8_r, uint8_g, uint8_b, uint8_a};
	return _color;
}




Color color(float r, float g, float b, float a)
{
	bool have_out_of_range_value = false;
	float given_r=r, given_g=g, given_b=b, given_a=a;

	if(r < 0) { r = 0; have_out_of_range_value = true; }
	if(g < 0) { g = 0; have_out_of_range_value = true; }
	if(b < 0) { b = 0; have_out_of_range_value = true; }
	if(a < 0) { a = 0; have_out_of_range_value = true; }

	if(r > 1) { r = 1; have_out_of_range_value = true; }
	if(g > 1) { g = 1; have_out_of_range_value = true; }
	if(b > 1) { b = 1; have_out_of_range_value = true; }
	if(a > 1) { a = 1; have_out_of_range_value = true; }

	uint8_t uint8_r = static_cast<uint8_t>(std::round(255.0f * r));
	uint8_t uint8_g = static_cast<uint8_t>(std::round(255.0f * g));
	uint8_t uint8_b = static_cast<uint8_t>(std::round(255.0f * b));
	uint8_t uint8_a = static_cast<uint8_t>(std::round(255.0f * a));

	if(have_out_of_range_value)
	{
		gs::lastError(gs::str("||Error|| in 'gs::color(4 float)', have out of range value-s. Values must be in [0 ; 1]. ")
					+ gs::str("[given values : r ")+std::to_string(given_r)+gs::str(", ")
					+ gs::str("g ")+std::to_string(given_g)+gs::str(", ")
					+ gs::str("b ")+std::to_string(given_b)+gs::str(", ")
					+ gs::str("a ")+std::to_string(given_a)+gs::str("]. ")
					);
	}

	Color _color = {uint8_r, uint8_g, uint8_b, uint8_a};

	return _color;
}



Color color(uint32_t ucolor) //RGBA
{
	uint8_t* ucol_ptr = (uint8_t*)(&ucolor);

	Color col = {ucol_ptr[3], ucol_ptr[2], ucol_ptr[1], ucol_ptr[0]};

	return col;
}


Color color(FColor col) {return Color(col.r, col.g, col.b, col.a);}

FColor fcolor(Color col) {return FColor(col.r, col.g, col.b, col.a);}


//FColor fcolor(int r, int g, int b, int a=255);

FColor fcolor(float r, float g, float b, float a) {return FColor(r,g,b,a);}

FColor fcolor(uint32_t ucolor) {return fcolor(color(ucolor));} //RGBA


uint32_t u32RGBA(uint8_t r, uint8_t g, uint8_t b, uint8_t a)
{
	uint32_t ucol;
	uint8_t* ucol_ptr = (uint8_t*)(&ucol);
	ucol_ptr[3] = r;
	ucol_ptr[2] = g;
	ucol_ptr[1] = b;
	ucol_ptr[0] = a;
	return ucol;
}


uint32_t u32RGBA(Color col) {return u32RGBA(col.r, col.g, col.b, col.a);}

uint32_t u32ABGR(uint32_t rgba)
{
	uint8_t* ptr = (uint8_t*)(&rgba);
	uint8_t r=ptr[3], g=ptr[2], b=ptr[1], a=ptr[0];

	ptr[3] = a;
	ptr[2] = b;
	ptr[1] = g;
	ptr[0] = r;

	return rgba;
}

uint32_t u32RGBA(uint32_t abgr)
{
	uint8_t* ptr = (uint8_t*)(&abgr);
	uint8_t a=ptr[3], b=ptr[2], g=ptr[1], r=ptr[0];

	ptr[3] = r;
	ptr[2] = g;
	ptr[1] = b;
	ptr[0] = a;

	return abgr;
}

uint32_t u32RGBA(FColor col) {return u32RGBA(color(col));}

uint8_t alphaByPercentage(float percent)
{
	float res = 255.0*percent;
	if(res < 0) res = 0;
	if(res > 255) res = 255;

	return uint8_t(res);
}



FRect frect(Rect rect)
{
	return FRect(
		static_cast<float>(rect.x), static_cast<float>(rect.y),
		static_cast<float>(rect.w), static_cast<float>(rect.h));
}

Rect rect(FRect rect)
{
	return Rect(
		static_cast<int>(rect.x), static_cast<int>(rect.y),
		static_cast<int>(rect.w), static_cast<int>(rect.h));
}

Rect rect(int x, int y, int w, int h){Rect _rect = {x, y, w, h}; return _rect;}
FRect frect(float x, float y, float w, float h) {return FRect(x,y,w,h);}
Rect rect(Vec2i xy, Vec2i wh) {return Rect(xy.x, xy.y, wh.x, wh.y);}
FRect frect(Vec2f xy, Vec2f wh) {return FRect(xy.x, xy.y, wh.x, wh.y);}

void splitRect(Rect r, Vec2i& out_xy, Vec2i& out_wh) {out_xy.x=r.x; out_xy.y=r.y; out_wh.x=r.w; out_wh.y=r.h;}
void splitFRect(FRect r, Vec2f& out_xy, Vec2f& out_wh) {out_xy.x=r.x; out_xy.y=r.y; out_wh.x=r.w; out_wh.y=r.h;}



FRect getFRectFromPercentParam(FRect reference, FRect percent_param)
{
	return FRect(
		percent_param.x*reference.w+reference.x,
		percent_param.y*reference.h+reference.y,
		percent_param.w*reference.w,
		percent_param.h*reference.h);
}


FRect scaleFRect(FRect rect, float scale) {return FRect(rect.x*scale, rect.y*scale, rect.w*scale, rect.h*scale);}

FRect scaleFRect(FRect rect, Vec2f scale) {return FRect(rect.x*scale.x, rect.y*scale.y, rect.w*scale.x, rect.h*scale.y);}

Vec2f getFRectCenter(FRect rect) {return Vec2f(rect.x+rect.w*0.5f, rect.y+rect.h*0.5f);}


bool hasFRectIntersection(FRect r1, FRect r2)
{
	//Pour qu'il y ait intersection, il faut qu'il y ait intersection sur les deux axes simultanément (intersection des "ombres" projetés).
	//On va alors partir sur le teste contraire : si sur un des axes il n'y a pas d'intersection, alors pas d'intersection dans l'ensemble.

	float min1=r1.x; //Les deux extémités de r1 sur l'axe
	float max1=r1.x+r1.w;
	float min2=r2.x; //Les deux extémités de r2 sur l'axe
	float max2=r2.x+r2.w;

	if(max1 < min2  or  min1 > max2) return false;

	//Même chose pour l'autre axe
	min1=r1.y; //Les deux extémités de r1 sur l'axe
	max1=r1.y+r1.h;
	min2=r2.y; //Les deux extémités de r2 sur l'axe
	max2=r2.y+r2.h;

	if(max1 < min2  or  min1 > max2) return false;

	return true;
}

std::string str(const char* c_string) {return std::string(c_string);}


std::string strToupper(std::string str)
{
	for(char& c : str)
		c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

	return str;
}


std::vector<std::string> strToupper(std::vector<std::string> str_vec)
{
    for(std::string& str : str_vec)
        for(char& c : str)
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));

    return str_vec;
}





bool drawRect(Renderer* ren, const FRect& rect, const Color* col, bool fill, Blend_Mode* b_mode)
{
	return drawRect(ren, &rect, col, fill, b_mode);
}




bool drawRect(Renderer* ren, const FRect* rect, const Color& col, bool fill, Blend_Mode* b_mode)
{
	return drawRect(ren, rect, &col, fill, b_mode);
}

bool drawRect(Renderer* ren, const FRect& rect, const Color& col, bool fill, Blend_Mode* b_mode)
{
	return drawRect(ren, &rect, &col, fill, b_mode);
}









bool drawLines(Renderer* ren, const std::vector<FPoint>& points, const Color& col, Blend_Mode* b_mode)
{
	return drawLines_INTERNAL(ren, points.data(), points.size(), col, b_mode);
}

bool drawLines(Renderer* ren, const std::vector<Vec2f>& points, const Color& col, Blend_Mode* b_mode)
{
	return drawLines_INTERNAL(ren, reinterpret_cast<const FPoint*>(points.data()), points.size(), col, b_mode);
}





Renderer* ren()
{
	if(ThisGame) return ThisGame->ren();
	return nullptr;
}

Window* win()
{
	if(ThisGame) return ThisGame->win();
	return nullptr;
}



bool drawPoint(Renderer* ren, const int& x, const int& y, const Color& col, Blend_Mode* b_mode)
{
	return drawPoint(ren, x, y, &col, b_mode);
}


bool drawPoint(Renderer* ren, Vec2i pt, const Color* col, Blend_Mode* b_mode)
{
	return drawPoint(ren, pt.x, pt.y, col, b_mode);
}

bool drawPoint(Renderer* ren, Vec2i pt, const Color& col, Blend_Mode* b_mode)
{
	return drawPoint(ren, pt.x, pt.y, &col, b_mode);
}





float perlinNoise2D(float x, float y, unsigned int seed)
{
	//Pour l'instant la seed est inutilisée.

	auto getNodeVec = [&seed] (int ix, int iy)
	{
		// float s = seed, x=ix, y=iy;
		float angle = (ix*ix + iy*12.0 - iy*iy + 20*ix);
		// float angle = (x*x*x-y + y*(s/12.0) - y*y*y+x*x + (s/20.0)*x);
		// float angle = std::cos(y) + std::sin(x*x*x)*2.0 +s/40.0;

	    //Renvoie du vecteur.
	    return gs::Vec2f(std::cos(angle), std::sin(angle));
	};



	auto interpol = [](float a, float b, float w) {
		return (b - a) * (3.0 - w * 2.0) * w * w + a;
	};


	//On suppose qu'une cellule a une taille de 1.
	float node_x = std::floor(x);
	float node_y = std::floor(y);
	float node_nx = node_x+1;
	float node_ny = node_y+1;

	//Distances.
	float dx = x - node_x;
	float dy = y - node_y;

	//Vecteurs de la grille.
	gs::Vec2f tl = getNodeVec(node_x,node_y);
	gs::Vec2f tr = getNodeVec(node_nx,node_y);
	gs::Vec2f bl = getNodeVec(node_x,node_ny);
	gs::Vec2f br = getNodeVec(node_nx,node_ny);

	//Les vecteurs distance.
	gs::Vec2f tl2(x-node_x, y-node_y);
	gs::Vec2f tr2(x-node_nx, y-node_y);
	gs::Vec2f bl2(x-node_x, y-node_ny);
	gs::Vec2f br2(x-node_nx, y-node_ny);

	FRect vals;
	vals.x = gs::dotProd(tl,tl2);
	vals.y = gs::dotProd(tr,tr2);
	vals.w = gs::dotProd(bl,bl2);
	vals.h = gs::dotProd(br,br2);


	float interp1 = interpol(vals.x, vals.y, dx);
	float interp2 = interpol(vals.w, vals.h, dx);

	return interpol(interp1, interp2, dy);
}




// Algorithme de Bresenham pour tracer une ligne entre deux points
std::vector<Vec2i> bresenham(int x0, int y0, int x1, int y1)
{
	//Code généré par I.A, et très légèrement adapté.

    std::vector<Vec2i> points; // Vecteur pour stocker les points de la ligne
    int dx = abs(x1 - x0);         // Différence absolue entre les coordonnées x
    int dy = abs(y1 - y0);         // Différence absolue entre les coordonnées y
    int sx = (x0 < x1) ? 1 : -1;   // Direction du pas pour x (1 ou -1)
    int sy = (y0 < y1) ? 1 : -1;   // Direction du pas pour y (1 ou -1)
    int err = dx - dy;             // Calcul de l'erreur initiale

    while (true) { // Boucle qui continue jusqu'à atteindre le point final
        points.push_back({x0, y0}); // Ajoute le point courant à la liste des points
        if (x0 == x1 && y0 == y1) { // Vérifie si le point final est atteint
            break;                  // Sort de la boucle
        }
        int e2 = 2 * err;           // Double l'erreur pour comparer avec dx et dy
        if (e2 > -dy) {             // Vérifie si l'on doit avancer en x
            err -= dy;              // Réajuste l'erreur en retirant dy
            x0 += sx;               // Avance en x dans la direction correcte
        }
        if (e2 < dx) {              // Vérifie si l'on doit avancer en y
            err += dx;              // Réajuste l'erreur en ajoutant dx
            y0 += sy;               // Avance en y dans la direction correcte
        }
    }
    return points; // Renvoie la liste des points de la ligne
}


std::vector<Vec2i> bresenham4(int x0, int y0, int x1, int y1)
{
	//Code généré par IA et adapté

	std::vector<Vec2i> output;

	int dx = abs(x1 - x0);
	int dy = abs(y1 - y0);

	int sx = (x0 < x1) ? 1 : -1;
	int sy = (y0 < y1) ? 1 : -1;

	int err = dx - dy;

	output.push_back(Vec2i(x0, y0));

	while(x0 != x1 || y0 != y1)
	{
		int e2 = err * 2;

		bool moveX = (e2 > -dy);
		bool moveY = (e2 < dx);

		// Si Bresenham voudrait faire une diagonale,
		// on la transforme en deux déplacements.
		if (moveX)
		{
			err -= dy;
			x0 += sx;
			output.push_back(Vec2i(x0, y0));
		}

		if (x0 == x1 && y0 == y1)
			break;

		if (moveY)
		{
			err += dx;
			y0 += sy;
			output.push_back(Vec2i(x0, y0));
		}
	}

	return output;
}


std::vector<Vec2i> bresenham(Vec2i p0, Vec2i p1) {return bresenham(p0.x, p0.y, p1.x, p1.y);}
std::vector<Vec2i> bresenham4(Vec2i p0, Vec2i p1) {return bresenham4(p0.x, p0.y, p1.x, p1.y);}




std::vector<gs::Vec2i> AStar_onTileMaze(Vec2i in, Vec2i out, std::function<bool(gs::Vec2i tile_pos)> isTileValidAndNotWall)
{
	struct Cell
	{
		Vec2i pos{0,0};
		Vec2i parent{0,0};

		int dist_from_begin=1000000; //Distance, en cases, parcourue depuis le début
		int estimate_dist_to_end=1000000; //Estimation de la distance qu'il y'a entre la case et la fin

		bool operator==(const Cell& other) {return pos == other.pos;}
		int total() const {return dist_from_begin+estimate_dist_to_end;}
	};

    struct Vec2i_Hash
    {
        std::size_t operator()(const Vec2i& v) const
        {
            return std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1);
        }
    };

    std::unordered_map<Vec2i, Cell, Vec2i_Hash> open_cells; //Les cases à voir
    std::unordered_map<Vec2i, Cell, Vec2i_Hash> closed_cells; //Les cases à ne plus voir

    Vec2i begin_cell_pos = in;
    Vec2i end_cell_pos = out;
    bool finished = false;
    std::vector<Vec2i> maze_path;

    if(not isTileValidAndNotWall)
    {
        gs::lastError("||Warning|| in 'AStar_onTileMaze()', callback arg 'isTileValidAndNotWall' is nullptr. ");
        return maze_path;
    }

    auto estimateToEndDist_ = [&](Vec2i pos_){ //int
        return gs::manhattanDist(pos_, end_cell_pos);
    };

    auto initAStar_ = [&](){ //void
        Cell begin;
        begin.pos = in;
        begin.parent = in;
        begin.dist_from_begin = 0;
        begin.estimate_dist_to_end = estimateToEndDist_(begin.pos);
        open_cells[begin.pos] = begin;
    };

    auto getTheOneWhichSeemsToBeOnTheShortestPath_ = [&](){ //Cell
        int min_total=100000000;
        Vec2i min_one;
        //int min_one__dist_from_begin=0; //pas le minimum, mais celui correspondant au 'min_one'
        int min_one__estimate_dist_to_end=100000; //pas le minimum, mais celui correspondant au 'min_one'
        bool is_ok = false;
        for(auto& [pos, cell] : open_cells)
        {
            int total = cell.total();
            if(total < min_total
            or (total == min_total and cell.estimate_dist_to_end < min_one__estimate_dist_to_end)
            )
            {
                min_total = total;
                min_one__estimate_dist_to_end = cell.estimate_dist_to_end;
                min_one = pos;
                is_ok = true;
            }
        }

        Cell output;
        if(is_ok)
        {
            output = open_cells.at(min_one);
            open_cells.erase(min_one);
        }

        return output;
    };

    auto getFreeNeighbours_ = [&](const Cell& cell){ //std::vector<Cell> //Les non murs ET non présents dans 'closed'.
        std::vector<Cell> neighbours;
        std::vector<Vec2i> offsets{{0,-1}, {0,1}, {-1,0}, {1,0}};

        for(Vec2i offset : offsets)
        {
            Cell nei;
            nei.pos = cell.pos + offset;

            //On vérifie que la position obtenue est bien dans le laby, et que ce n'est pas sur un mur.
            if(not isTileValidAndNotWall(nei.pos)) continue;

            auto it = closed_cells.find(nei.pos);
            if(it != closed_cells.end()) continue; //On vérifie aussi qu'il n'est pas dans 'closed'.


            nei.parent = cell.pos;
            nei.dist_from_begin = cell.dist_from_begin +1;
            nei.estimate_dist_to_end = estimateToEndDist_(nei.pos);
            neighbours.push_back(nei);
        }

        return neighbours;
    };

    auto addIntoClosed_ = [&](const Cell& cell){ //void
        closed_cells[cell.pos] = cell;
    };

    auto addIntoOpen_ = [&](const Cell& cell){ //void
        auto it = open_cells.find(cell.pos);
        if(it != open_cells.end())
        {
            //Si la case est déjà présente, on regarde si cette version a été atteinte plus rapidement, mettre à jour la case.
            if(cell.dist_from_begin < it->second.dist_from_begin)
            {
                it->second = cell;
                //it->second.dist_from_begin = cell.dist_from_begin;
                //it->second.parent = cell.parent;
            }
        }
        else //Sinon si la case n'y est pas déjà on la rajoute simplement
            open_cells[cell.pos] = cell;
    };


    auto buildFoundPath_ = [&](const Cell& last){//void
        //On va remonter de proche en proche jusqu'au début pour constituer le chemin.
        //Le point d'arrêt est lorsqu'on trouve une case qui a elle-même comme parent.

        auto getParent = [&](Cell cell){
            auto it = closed_cells.find(cell.parent);
            if(it != closed_cells.end()) return it->second;
            cell.parent = cell.pos;
            return cell;
        };

        Cell curr = last;
        maze_path.push_back(curr.pos);

        int para_infinite_loop = 4000000;
        while(curr.pos != curr.parent)
        {
            curr = getParent(curr);
            maze_path.push_back(curr.pos);

            --para_infinite_loop;
            if(para_infinite_loop < 0) break;
        }

		std::reverse(maze_path.begin(), maze_path.end()); //Pour que ça puisse commencer par le début, puisque l'ajout s'est fait en partant de la fin et en remontant.
    };


    initAStar_();

    while(not finished)
    {
        if(open_cells.empty())
            break;

        Cell curr_head = getTheOneWhichSeemsToBeOnTheShortestPath_();
        addIntoClosed_(curr_head);

        if(curr_head.pos == end_cell_pos)
        {
            buildFoundPath_(curr_head);
            break;
        }

        std::vector<Cell> free_neighbours = getFreeNeighbours_(curr_head);
        for(Cell& nei : free_neighbours)
            addIntoOpen_(nei);

        finished = open_cells.empty();
    }

    return maze_path;
}







void loadTileMaze(std::vector<int>& maze_, Vec2i maze_size)
{
    //Génération d'un laby

    if(maze_size.x < 3 or maze_size.y < 3)
    {
        gs::lastError("||Waring|| in 'loadTileMaze()', maze_size too small. ");
        return;
    }

    if(maze_.size() != maze_size.x*maze_size.y)
    {
        gs::lastError("||Waring|| in 'loadTileMaze()', maze' size dont match given size arg. ");
        return;
    }


    Vec2i pseudo_size = (maze_size)/2; //C'est une technique trouvée pour passer de la représentation des liens entre cases, aux tuiles. Cette ligne permet de trouver les dimensions équivalentes en représentation des liens.
    //'maze_size' est la taille en tuile, 'pseudo_size' est la taille en représentation des liens.

    struct Node // -> une case, en représentation des liens avec ses quatre voisins.
    {
        Vec2i pos;
        //Déterminent la présence des murs.
        bool up_wall=true;
        bool down_wall=true;
        bool left_wall=true;
        bool right_wall=true;
        void breakWall(Vec2i offset, Node& other)
        {
            if(offset == Vec2i(0,-1))
            {
                up_wall = false;
                other.down_wall = false;
            }
            else if(offset == Vec2i(0,1))
            {
                down_wall = false;
                other.up_wall = false;
            }
            else if(offset == Vec2i(-1,0))
            {
                left_wall = false;
                other.right_wall = false;
            }
            else if(offset == Vec2i(1,0))
            {
                right_wall = false;
                other.left_wall = false;
            }
        }

        std::vector<Vec2i> getAccessibleNeighboursPos() const
        {
            std::vector<Vec2i> output;
            if(!up_wall) output.push_back(pos + Vec2i(0,-1));
            if(!down_wall) output.push_back(pos + Vec2i(0,1));
            if(!left_wall) output.push_back(pos + Vec2i(-1,0));
            if(!right_wall) output.push_back(pos + Vec2i(1,0));
            return output;
        }
    };

    std::unordered_map<Vec2i, Node, gs::Vec2i_Hash> seen_nodes;
    std::stack<Node> stack;

    auto getUnseenNeighboursPos = [&](Vec2i pos){
        std::vector<Vec2i> neis_pos;
        std::vector<Vec2i> offsets{{0,-1}, {0,1}, {-1,0}, {1,0}};
        for(Vec2i offset : offsets)
        {
            Vec2i nei_pos = pos+offset;
            if(nei_pos.x < 0 or nei_pos.y < 0) continue;
            if(nei_pos.x >= pseudo_size.x or nei_pos.y >= pseudo_size.y) continue;
            if(seen_nodes.contains(nei_pos)) continue;

            neis_pos.push_back(nei_pos);
        }

        return neis_pos;
    };

    auto seen = [&](const Node& node){seen_nodes[node.pos] = node;};


    Node begin;
    begin.pos = Vec2i(0,0);
    stack.push(begin);

    while(seen_nodes.size() < pseudo_size.x*pseudo_size.y//Tant que toutes les cases n'ont pas été parcourues
        or stack.empty()) //Sécurité
    {
        std::vector<Vec2i> neis_pos = getUnseenNeighboursPos(stack.top().pos);

        seen(stack.top()); //On marque la courante comme vue.

        if(neis_pos.empty()) //Si elle n'a pas de voisines non vues, on remonte à la précédente
        {
            stack.pop();
            continue;
        }

        Vec2i chosen_next_pos;
        if(neis_pos.size() == 1) //S'il n'y a qu'une seule voisine
            chosen_next_pos = neis_pos.front();
        else
        {
            //Sinon on en choisi une au hasard
            int idx = int(gs::random(0.f, neis_pos.size()));

            //Sécurité
            if(idx < 0) idx = 0;
            if(idx >= neis_pos.size()) idx = neis_pos.size()-1;

            chosen_next_pos = neis_pos.at(idx);
        }

        //On retire le mur entre la courante et sa voisine choisie.
        Node next;
        next.pos = chosen_next_pos;
        stack.top().breakWall(next.pos - stack.top().pos, next);

        stack.push(next); //Et on passe à la suivante.
    }

    //Une fois la génération terminée, il ne reste qu'à la transcrire en tile-maze.

    auto toTileMazeCoo = [](Vec2i pos){return pos*2 +1;};

    //On rempli d'abord de murs
    int wall_value = 1;
    for(int i=0; i<maze_size.y*maze_size.x; ++i)
        maze_[i] = wall_value;

    auto setMazeValue = [&](const Vec2i& p, int val){
        maze_[p.y*maze_size.x +p.x] = val;
    };

    //Puis on retire les murs en fonction de la génération.
    int empty_value = 0;

    auto isValidTilePos = [&](const Vec2i& pos){
        if(pos.x < 0 or pos.y < 0) return false;
        if(pos.x >= maze_size.x or pos.y >= maze_size.y) return false;
        return true;
    };



    for(const auto& [pos, node] : seen_nodes)
    {
        Vec2i tile_pos = toTileMazeCoo(pos);
        if(not isValidTilePos(tile_pos)) continue;

        setMazeValue(tile_pos, empty_value);

        for(const Vec2i& nei_pos : node.getAccessibleNeighboursPos())
        {
            Vec2i nei_tile_pos = toTileMazeCoo(nei_pos);

            //On libère la voie
            Vec2i access_tile_pos = (tile_pos + nei_tile_pos)/2;
            if(not isValidTilePos(access_tile_pos)) continue;
            setMazeValue(access_tile_pos, empty_value);
        }

        //On rajoute un peu de bruit : casser un mur aléatoirement quelques fois
        float p = gs::random(0.f, 100.f);
        if(p < 20.f)
        {
            tile_pos.x = int(gs::random(2, maze_size.x-2));
            tile_pos.y = int(gs::random(2, maze_size.y-2));

            if(isValidTilePos(tile_pos))
                setMazeValue(tile_pos, empty_value);
        }
    }
}



float avoidObstacle_changeAngle(float entity_move_angle, Circle_Hitbox entity, Circle_Hitbox obstacle, float p)
{
	float ob_dist = pointsDist(entity.center, obstacle.center);
	if(ob_dist < entity.radius * p + obstacle.radius)
	{
		float ob_dir = getPointsAngle(entity.center, obstacle.center);

		float fov = 3.14159f*1.2f;
		Vec2f dir_vec = gs::vec(entity_move_angle);
		if(gs::isInInterval_out_out(entity_move_angle-fov/2.f, entity_move_angle+fov/2.f, ob_dir))
		{
			float ob_ortho_dir1 = ob_dir + 3.14159f/2.f;
			float ob_ortho_dir2 = ob_dir - 3.14159f/2.f;
			float ob_ortho_dir;
			//Celui avec le plus grand produit scalaire avec 'dir_vec', est celui dont l'angle est le plus proche de 'entity_move_angle'.
			if(dotProd(gs::vec(ob_ortho_dir1), dir_vec) > dotProd(gs::vec(ob_ortho_dir2), dir_vec))
				ob_ortho_dir = ob_ortho_dir1;
			else
				ob_ortho_dir = ob_ortho_dir2;

			//entity_move_angle = gs::meanAngle(ob_ortho_dir, entity_move_angle);
			//entity_move_angle = gs::meanAngle(ob_ortho_dir, entity_move_angle);
			entity_move_angle = ob_ortho_dir;
		}
	}

	return entity_move_angle;
}






unsigned int getBytesCountOfUTF8Charac_ByTheCharacHeaderByte(char ch, bool& is_ok)
{
	is_ok=true;

	try{


	std::string byte = std::bitset<8>(ch).to_string();

	if(byte.at(0) == '0') //C'est un caractère composé d'un seul octet.
	{
		return 1u;
	}
	else //Sinon c'est que 'byte.at(0)' c'est (normalement) un '1'.
	{
		if(byte.at(1) == '1' and byte.at(2) == '0') //Suite de 'char's composant un caractère utf-8 de deux octets.
		{
			return 2u;
		}
		else
		if(byte.at(1) == '1' and byte.at(2) == '1' and byte.at(3) == '0') //Suite de 'char's composant un caractère utf-8 de trois octets.
		{
			return 3u;
		}
		else
		if(byte.at(1) == '1' and byte.at(2) == '1' and byte.at(3) == '1' and byte.at(4) == '0') //Suite de 'char's composant un caractère utf-8 de quatre octets.
		{
			return 4u;
		}
	}


	}catch(std::exception& err)
	{
		std::cout << "!! in 'getBytesCountOfUTF8Charac_ByTheCharacHeaderByte()' : " << err.what() << '\n';
		is_ok = false;
	}

	return 0; //Si on arrive ici, et que 'is_ok==true', c'est (probablement) que l'octet reçu est un octet compasant un caractère tout en étant pas le premier octet de ce caractère.
}


unsigned int getBytesCountOfUTF8Charac_ByTheCharacHeaderByte(char ch)
{
	bool skip_arg;
	return getBytesCountOfUTF8Charac_ByTheCharacHeaderByte(ch, skip_arg);
}



unsigned int getUTF8CharsCount(const std::string& str, bool& is_ok)
{
	//Compte le nombre de caractères UTF-8 dans 'str' (selon la "perception humaine"). Par exemple, "été" contient 5 octets, mais en terme de caractères il n'y en a que 3, alors cette fonction va renvoyer 3.

	unsigned int utf8_chars_count = 0;
	unsigned int skip = 0;
	is_ok = true;


	for(const char& ch : str)
	{
		if(skip>0)
		{
			--skip;
			continue;
		}

		unsigned int curr_char_chars_count = getBytesCountOfUTF8Charac_ByTheCharacHeaderByte(ch, is_ok);
		if(not is_ok or curr_char_chars_count==0)
			return 0;

		++utf8_chars_count;
		skip = curr_char_chars_count-1;
	}

	return utf8_chars_count;
}


unsigned int getUTF8CharsCount(const std::string& str)
{
	bool skip_arg;
	return getUTF8CharsCount(str, skip_arg);
}




bool isFlagPresentIn(int combined_flags, int specifiq_flag)
{
	return (combined_flags & specifiq_flag) == specifiq_flag;
}
































//____________________________________________________________________________________
//___________________________________CAMERA___________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________


void Camera_Shaking::update(float dt)
{
	//float a = 2.f;
	//move.x += gs::changeRange(gs::random(), 0.f, 1.f, -1.f, 1.f)*a*dt;
	//move.y += gs::changeRange(gs::random(), 0.f, 1.f, -1.f, 1.f)*a*dt;

	float t = timer;
	move.x = std::sin(f.x*t)*a.x;
	move.y = std::sin(f.y*t)*a.y;
	timer-=dt;
}

void Camera2D_Base::addScreenShaking(float duration)
{
	m_screen_shakings.emplace_back();
	m_screen_shakings.back().timer=duration;
	m_screen_shakings.back().a = gs::Vec2f(gs::random(0.5f, 1.5f), gs::random(0.5f, 1.5f))*2.f;
	m_screen_shakings.back().f = gs::Vec2f(gs::random(0.5f, 2.5f), gs::random(0.5f, 2.f))*40.f;
}

void Camera2D_Base::update(float dt)
{
	for(int i=int(m_screen_shakings.size())-1; i>=0; --i)
		if(m_screen_shakings[i].timer <= 0.f)
			m_screen_shakings.erase(m_screen_shakings.begin()+i); //Retrait des tremblements finis
		else
			m_screen_shakings[i].update(dt);
}


gs::Vec2f Camera2D_Base::getCurrScreenShaking() const
{
	gs::Vec2f sum;
	for(const Camera_Shaking& s : m_screen_shakings)
		sum += s.move;
	return sum;
}



void Camera2D_Base::setWorldPos(gs::Vec2f new_pos)
{
	if(new_pos.x < m_min_world_pos.x) new_pos.x = m_min_world_pos.x;
	if(new_pos.y < m_min_world_pos.y) new_pos.y = m_min_world_pos.y;
	m_world_pos = new_pos;
}

gs::Vec2f Camera2D_Base::getFormattedPos(gs::Vec2f pos) const
{
	return (pos - (m_world_pos+m_world_offset))*m_zoom
			+(m_screen_offset+getCurrScreenShaking())*m_screen_val_modifier;
}

gs::Vec2f Camera2D_Base::getFormattedSize(gs::Vec2f size) const {return size*m_zoom;}

gs::FRect Camera2D_Base::getFormattedRect(gs::FRect rect) const
{
	gs::Vec2f scr_shk = getCurrScreenShaking();
	return gs::FRect(
		(rect.x - (m_world_pos.x+m_world_offset.x))*m_zoom +(m_screen_offset.x +scr_shk.x)*m_screen_val_modifier.x,
		(rect.y - (m_world_pos.y+m_world_offset.y))*m_zoom +(m_screen_offset.y +scr_shk.y)*m_screen_val_modifier.y,
		rect.w*m_zoom, rect.h*m_zoom);
}

void Camera2D_Base::setWorldCenterPos(gs::Vec2f world_pos, gs::Vec2f ren_size)
{
	//Agit sur les position et offset mondes, sans tenir compte de ceux d'écrans.

	setWorldPos(world_pos);
	setWorldOffset((ren_size/m_zoom)*(-0.5f));
}



//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//________________________FONCTIONS  DU MIXER (MUSIQUES)______________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________


/*
	bool Game::initMixer(int frequency, uint16_t format, int channels, int chunksize)
	{

		if(initMix_is_allowed)
		{
			if(Mix_OpenAudio( frequency, format, channels, chunksize) == 0)
			{
				mixActive_ = true;
				initMix_is_allowed = false;
				allocated_channels += channels;
				return true;
			}

			std::cout << "\t|| Erreur || `initMixer()' failed : "<< Mix_GetError() << std::endl;
			mixActive_ = false;
			return false;
		}
		else
		{
			std::cout << "\t|| Warning || `initMixer()' already used. Close the Mixer befor trying " << std::endl;
			return false;
		}
	}

//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//


	void Game::closeMixer()
	{
		if(mixActive_)
		{
			Mix_CloseAudio();
			mixActive_ = false;
			initMix_is_allowed = true;
		}
	}


//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//


int  Game::getFree_channel()
{
	if(allocated_channels == free_channel_)						//allocated_channels est le nombre de channel alloué, free_channel est le numero du plus petit channel libre mais aussi le nb de channel occupés
	{
		allocated_channels = Mix_AllocateChannels(allocated_channels+1);
	}

	free_channel_+= 1;

	return free_channel_-1;
};
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//
//-------------------------------------------------------------------------------//


Music::Music()
{music = nullptr; volume = 50; };

//------------------------------------------------------------

Music::~Music(){ Mix_FreeChunk(music); }

//------------------------------------------------------------

bool Music::load(Game *g, const char *file)
{
	if(music != nullptr)
		Mix_FreeChunk(music);
	music = nullptr;

	music = Mix_LoadWAV(file); selfChannel = g->getFree_channel();

	if(music != nullptr)
		return true;
	return false;
};

//------------------------------------------------------------

void Music::play(int boucle)
{
	if(!Mix_Playing(selfChannel))
		Mix_PlayChannel(selfChannel, music, boucle);
}

//------------------------------------------------------------

void Music::stop(){ Mix_HaltChannel(selfChannel); };

//------------------------------------------------------------

void Music::pause(){ Mix_Pause(selfChannel); };

//------------------------------------------------------------

void Music::resume(){ Mix_Resume(selfChannel); };

//------------------------------------------------------------

void Music::setVolume(int vol)
{
	volume = vol;
	Mix_Volume(selfChannel, volume);
}

//------------------------------------------------------------

void Music::incrVolume(int val)
{
	volume += val;
	Mix_Volume(selfChannel, volume);
}



*/






} //fin namespace gs
