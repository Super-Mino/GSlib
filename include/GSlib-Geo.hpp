#ifndef _GSlib_GEO_HPP_
#define _GSlib_GEO_HPP_

//Note: comportement non implémenté pour raylib :

//bool drawEllipseThickness(Renderer* ren, Vec2f center, Vec2f radii, int thickness, Color col, bool fill, float total_angle, float begin_angle)
//bool drawShape(Renderer* ren, Shape_P s)


//Version modifiée le 19/10/25 (à moins d'oubli de modifier cette ligne ou autre).


	#include "GSlib.hpp"


namespace gs
{

struct Shape_P //Shape parameters.
{
	FRect area;

	enum Shape {E, R, T, S}; //Ellipse, rect, triangle, segment.

	Shape type;
	Vec2f pos={0.1,0.1}, pos2={0.1,0.2}, pos3={0.2,0.15}; //Pourcentages des tailles de 'area', afin se postionner relativement à son coin supérieur gauche.
	Vec2f size={0.1,0.1}; //Pourcentages des tailles de 'area'.

	bool fill=false;
	bool clockwise=true; //Sens de rotation.

	float rot_ang=0;
	Vec2f rot_pt; //Pour les rectangles c'est relatif au rectangle et à son coin supérieur gauche : {0,0} = coin supérieur gauche du rectangle, {1,1} = coin inférieur droit, {0.5, 0.5} = (plus ou moins) le milieu.
					//Pour les triangles {0,0} = centre du triangle, sinon c'est relatif à 'area' (un peu comme pour 'pos', 'pos2', 'pos3').
					//Pour les segements c'est aussi comme pour 'pos', 'pos2', 'pos3'.

	//Pour les ellipses.
	float total_angle=2*_pi_;
	float begin_angle=0;

	float thickness=0; //Si 0 : pas prise en compte (drawEllipse()), sinon prise en compte (drawEllipseThickness()), avec 'thickness' relative à 'area.w';

	float round_percent=0;

	Color color={255,255,255,255};
	Blend_Mode blend_mode=BLENDMODE_BLEND;
	bool set_blend_mode=false;

};


struct Rotation_P
{
	float rot_rad=0.f;
	gs::Vec2f anchor={0,0};
	bool clockwise=true;
};



bool drawRect(Renderer* ren, const FRect* rect, Color col, bool fill, Blend_Mode* b_mode, Rotation_P rotation);
bool drawRoundedRect(Renderer* ren, FRect rect, float percent, Color col, bool fill=false, Blend_Mode* b_mode=nullptr); //Attention le comportement diffère selon le background utilisé. 
bool drawEllipse(Renderer* ren, Vec2f center, Vec2f radii, Color col, bool fill=false, float total_angle=2*_pi_, float begin_angle=0);
bool drawEllipseThickness(Renderer* ren, Vec2f center, Vec2f radii, int thickness, Color col, bool fill=false, float total_angle=2*_pi_, float begin_angle=0);
bool drawCircle(Renderer* ren, Vec2f center, float radius, Color col, bool fill=false, float total_angle=2*_pi_, float begin_angle=0);
bool drawCircleThickness(Renderer* ren, Vec2f center, float radius, int thickness, Color col, bool fill=false, float total_angle=2*_pi_, float begin_angle=0);

bool drawRect(Renderer* ren, FRect rect, Color col, bool fill, Blend_Mode* b_mode, Rotation_P rotation);

bool drawShape(Renderer* ren, Shape_P s);




//=================================================================================//
//=================================================================================//
//=================================================================================//
//=================================================================================//





struct Triangle
{
	Vec2f a, b, c;

	bool draw(Renderer* ren, Color col={0,200,0,255}, bool fill=false, const Blend_Mode* b_mode=nullptr);

	void move(Vec2f speed){a += speed; b += speed; c += speed;}

	void rotate(float rad, const Vec2f* rot_center=nullptr, bool clockwise=true)
	{
		std::vector<gs::Vec2f*> pts={&a, &b, &c};
		Vec2f center = (rot_center ? *rot_center : getCenter());

		for(Vec2f* pt : pts)
		{
			Vec2f new_pos = rotatePoint(center.x, center.y, pt->x, pt->y, rad, clockwise).to<float>();
			*pt = new_pos;
		}
	}

	Vec2f getCenter()
	{
		float center_x = (a.x + b.x + c.x) / 3.0;
		float center_y = (a.y + b.y + c.y) / 3.0;
		return {center_x, center_y};
	}


};



struct Circle
{
	Vec2f center;
	float radius;

	bool draw(Renderer* ren, Color col={100,0,0,255}, bool fill=false, float total_angle=2*_pi_, float begin_angle=0)
	{
		return drawCircle(ren, center, int(radius), col, fill, total_angle, begin_angle);
	}

	bool drawThickness(Renderer* ren, int thickness, Color col={100,0,0,255}, bool fill=false, float total_angle=2*_pi_, float begin_angle=0)
	{
		return drawCircleThickness(ren, center, int(radius), thickness, col, fill, total_angle, begin_angle);
	}

	bool isCollision(Circle other) const
	{
		float centers_dist = gs::pointsDist(center, other.center);
		return centers_dist < radius+other.radius;
	}

};




struct Line
{

	float a, b, c;
	Line()=default;

	Line(float a, float b, float c) : a(a), b(b), c(c){}

	Line(Vec2f point, Vec2f dir_vector) //Création d'une droite à partir d'un point et d'un vecteur.
	: a(dir_vector.y), b(-dir_vector.x), c(-(dir_vector.y*point.x + (-dir_vector.x)*point.y)){}

	Line(float leading_coef, float y_intercept) //Avec coefficient directeur et ordonnée à l'origine.
	: a(leading_coef), b(-1), c(y_intercept){}


	void initWith2pts(Vec2f pt1, Vec2f pt2) //Droite passant par deux points.
	{
		Vec2f dir_vector = pt2-pt1;
		a = dir_vector.y;
		b = -dir_vector.x;
		c = -(dir_vector.y*pt1.x + (-dir_vector.x)*pt1.y);
	}


	void draw(Renderer* ren, Color color={200,100,0,255}, int win_w=3000, int win_h=2000)
	{
		// if(b == 0) return;
		// if(a == 0) return;

		if(win_w<0) win_w = 2000;  if(win_h<0) win_h = 2000;

		float y1, x1, y2, x2;
		if(a == 0) //Ligne horizontal.
		{
			y1=c/(-b);   y2=y1;
			x1=0;        x2=win_w;
		}
		else
		{
			y1 = 0;       x1 = ((-b)*y1-c)/a;
			y2 = win_h;   x2 = ((-b)*y2-c)/a;
		}

		//Pour éviter les valeurs trop grandes, on inverse les axes de référence.
		if(std::abs(x1) > 2147483640.f)
		{x1 = (x1 < 0 ? 0 : win_w);      y1 = ((-(a*x1))-c)/b;}

		if(std::abs(x2) > 2147483640.f)
		{x2 = (x2 < 0 ? 0 : win_w);      y2 = ((-(a*x2))-c)/b;}

		gs::drawLine(ren, Vec2f(x1,y1), Vec2f(x2,y2), color);
	}

	bool intersect(Line ln) const
	{
		//On vérifie si le vecteur directeur de l'un et le vecteur normal de l'autre sont orthogonaux.
		//Si oui, alors les droites sont parallèles : il n'y a pas de point d'intersection.

		gs::Vec2f u={-b, a};
		gs::Vec2f n={ln.a, ln.b};

		if( (u.x*n.x + u.y*n.y) == 0 )
			return false;

		return true;
	}

	Vec2f intersectPoint(Line ln)
	{
		//Retourne le point d'intersection s'il y'en a.

		Vec2f pt;

		if(b == 0)
			b = -0.000000001;

		float x_denominator = ln.b*a - b*ln.a;

		if(x_denominator == 0)
			x_denominator = -0.000000001;

		pt.x = (-(c*ln.b) + b*ln.c) / x_denominator;
		pt.y = (-a* ((-(c*ln.b) + b*ln.c) / x_denominator) -c)/b;

		return pt;
	}

	Vec2i intersectPoint_i(Line ln)
	{
		//Retourne le point d'intersection s'il y'en a. (version int).
		Vec2f pt = intersectPoint(ln);
		return Vec2i(int(pt.x), int(pt.y));
	}

	bool intersect(Circle cir, Vec2f* out__pt1=nullptr, Vec2f* out__pt2=nullptr) const
	{return lineAndCircleIntersect(a, b, c, cir.center, cir.radius, out__pt1, out__pt2);}

	//bool intersect_i(Circle cir, Vec2i* out__pt1=nullptr, Vec2i* out__pt2=nullptr) const
	//{return lineAndCircleIntersect_i(a, b, c, cir.center, cir.radius, out__pt1, out__pt2);}
};





struct Line_Segment
{
	Vec2f a, z; //Les deux bornes du segment.
	Line_Segment()=default;
	Line_Segment(const gs::Vec2f& a, const gs::Vec2f& z) : a(a), z(z){}
	Line_Segment(const gs::Vec2f& o, const gs::Vec2f& dir_vector, const float& len) //D'après un point d'origine, un vecteur directeur et un longueur (en pixel).
	: a(o)
	{
		//Normalisation du vecteur.
		gs::Vec2f dir_vec = normalizedVecOf(dir_vector);

		z.x = a.x + dir_vec.x*len;
		z.y = a.y + dir_vec.y*len;
	}


	void draw(Renderer* ren, Color color={0,200,200,255}) const {gs::drawLine(ren, a.to<float>(), z.to<float>(), color);}

	bool intersect(Line_Segment ln_s) const //Intersection entre deux segments.
	{
		Line&& self_line = getLine();
		Line&& ln_s_line = ln_s.getLine();

		if(not self_line.intersect(ln_s_line))
			return false;

		gs::Vec2f pt = self_line.intersectPoint(ln_s_line);

		// float self_len = gs::pointsDist(a, z);

		// if((gs::pointsDist(a, pt) <= self_len) and (gs::pointsDist(z, pt) <= self_len))
			// return true;

		//Si l'intersection se fait dans les intervalles communs aux deux segments.
		if( (GetMin(a.x, z.x) <= pt.x) and (pt.x <= GetMax(a.x, z.x)) )
		if( (GetMin(a.y, z.y) <= pt.y) and (pt.y <= GetMax(a.y, z.y)) )
		if( (GetMin(ln_s.a.x, ln_s.z.x) <= pt.x) and (pt.x <= GetMax(ln_s.a.x, ln_s.z.x)) )
		if( (GetMin(ln_s.a.y, ln_s.z.y) <= pt.y) and (pt.y <= GetMax(ln_s.a.y, ln_s.z.y)) )
			return true;

		return false;
	}

	Vec2f intersectPoint(Line_Segment ln_s) const {return getLine().intersectPoint(ln_s.getLine());}
	Vec2i intersectPoint_i(Line_Segment ln_s) const {Vec2f pt = intersectPoint(std::move(ln_s)); return Vec2i(int(pt.x), int(pt.y));}
	bool intersect(Line ln) const //Intersection entre un segment et une droite.
	{
		Line self_line = getLine();
		if(not self_line.intersect(ln))
			return false;

		gs::Vec2f pt = self_line.intersectPoint(ln);
		float self_len = gs::pointsDist(a, z);

		if((gs::pointsDist(a, pt) <= self_len) and (gs::pointsDist(z, pt) <= self_len))
			return true;

		return false;
	}

	Vec2f intersectPoint(Line ln) const{return getLine().intersectPoint(ln);}
	Vec2i intersectPoint_i(Line ln) const {Vec2f pt = intersectPoint(std::move(ln)); return gs::Vec2i(int(pt.x), int(pt.y));}

	Vec2b intersect(Circle cir, Vec2f* out__pt1=nullptr, Vec2f* out__pt2=nullptr) const //Intersection entre un segment et un cercle. Renvoie deux booléens car il peut ou ne pas y avoir l'intersection n°1 et de même pour l'intersection n°2.
	{
		Vec2b res = {false, false};
		Vec2f pt1, pt2;

		if(getLine().intersect(cir, &pt1, &pt2)) //Si au moins le cercle et la droite correspondate du segment se croisent.
		{
			//On vérifie pour chaque point s'il est sur le segment.
			//Autrement dit si l'intersection se fait dans les intervalles du segments.
			if( (GetMin(a.x, z.x) <= pt1.x) and (pt1.x <= GetMax(a.x, z.x)) )
			if( (GetMin(a.y, z.y) <= pt1.y) and (pt1.y <= GetMax(a.y, z.y)) )
			{
				res.x = true;
				if(out__pt1) (*out__pt1) = pt1;
			}

			if( (GetMin(a.x, z.x) <= pt2.x) and (pt2.x <= GetMax(a.x, z.x)) )
			if( (GetMin(a.y, z.y) <= pt2.y) and (pt2.y <= GetMax(a.y, z.y)) )
			{
				res.y = true;
				if(out__pt2) (*out__pt2) = pt2;
			}
		}

		return res;
	}

	Vec2b intersect_i(Circle cir, Vec2i* out__pt1=nullptr, Vec2i* out__pt2=nullptr) const
	{
		Vec2f pt1, pt2;
		Vec2b res = this->intersect(cir, &pt1, &pt2);

		if(res.x) (*out__pt1) = pt1.to<int>();
		if(res.y) (*out__pt2) = pt2.to<int>();
		return res;
	}

	Line getLine() const {Vec2f u_vec = {z.x-a.x, z.y-a.y}; return Line(z, u_vec);}
	float getLen() const {return gs::pointsDist(a, z);}
	int getLen_i() const {return int(gs::pointsDist(a, z));}
};


}

std::ostream& operator<<(std::ostream& os, const gs::Line& ln);



#endif // _GSlib_GEO_HPP_
