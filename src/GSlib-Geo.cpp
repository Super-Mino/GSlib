#include "GSlib-Geo.hpp"

//Version modifiée le 23/05/26 (à moins d'oubli de modifier cette ligne, ou autre).

std::ostream& operator<<(std::ostream& os, const gs::Line& ln)
{
	os << "[Ln -> a " << ln.a << "| b " << ln.b << "| c " << ln.c << "]";
	return os;
}



namespace gs
{




bool drawRect(Renderer* ren, FRect rect, Color col, bool fill, Blend_Mode* b_mode, Rotation_P rotation)
{return drawRect(ren, &rect, col, fill, b_mode, rotation);}






bool drawShape(Renderer* ren, Shape_P s)
{
	if(!ren)
	{
		gs::lastError("||Error|| in 'drawShape()', invalid renderer. ");
		return false;
	}

	bool success = true;

	auto _X = [&](float percent) {
		return s.area.x + s.area.w*percent;
	};
	auto _Y = [&](float percent) {
		return s.area.y + s.area.h*percent;
	};
	auto _W = [&](float percent) {
		return s.area.w*percent;
	};
	auto _H = [&](float percent) {
		return s.area.h*percent;
	};


	Blend_Mode* bm=nullptr;
	if(s.set_blend_mode)
		bm = &s.blend_mode;

	// if(s.thickness != 1)
	// {
	// 	float prev = s.thickness;
	// 	s.thickness = _W(s.thickness);

	// 	if(s.thickness == 0 and prev != 0)
	// 		s.thickness = 1;
	// }


	if(s.type == Shape_P::E) //Ellipse.
	{
		Vec2f center={(_X(s.pos.x)), (_Y(s.pos.y))};
		Vec2f radii={(_W(s.size.x)), (_H(s.size.y))};

		if(s.thickness == 0)
		{
			success = success & drawEllipse(ren, center, radii, s.color, s.fill, s.total_angle, s.begin_angle);
		}
		else
		{
			s.thickness = _W(s.thickness);

			if(s.thickness == 0)
				success = success & drawEllipse(ren, center, radii, s.color, false, s.total_angle, s.begin_angle);
			else
				success = success & drawEllipseThickness(ren, center, radii, int(s.thickness), s.color, s.fill, s.total_angle, s.begin_angle);
		}


		// if(s.thickness != 1)
		// {
		// 	if(s.thickness != 0)
		// 		success = success & drawEllipseThickness(ren, center, radii, int(s.thickness), s.color, s.fill, s.total_angle, s.begin_angle);
		// 	else
		// 		success = success & drawEllipse(ren, center, radii, s.color, false, s.total_angle, s.begin_angle);
		// }
		// else
		// 	success = success & drawEllipse(ren, center, radii, s.color, s.fill, s.total_angle, s.begin_angle);
	}
	else
	if(s.type == Shape_P::R) //Rectangle.
	{
		FRect r={_X(s.pos.x), _Y(s.pos.y), _W(s.size.x), _H(s.size.y)};

		if(s.round_percent != 0)
			success = success & drawRoundedRect(ren, r, s.round_percent, s.color, s.fill, bm);
		else
		if(s.rot_ang != 0)
			success = success & drawRect(ren, &r, s.color, s.fill, bm, {s.rot_ang, s.rot_pt, s.clockwise});
		else
			success = success & drawRect(ren, &r, s.color, s.fill, bm);
	}
	else
	if(s.type == Shape_P::T) //Triangle.
	{
		Triangle t ={{float(_X(s.pos.x)), float(_Y(s.pos.y))},
					{float(_X(s.pos2.x)), float(_Y(s.pos2.y))},
					{float(_X(s.pos3.x)), float(_Y(s.pos3.y))}};

		if(s.rot_ang != 0)
		{
			Vec2f rot_center;
			if(s.rot_pt == gs::Vec2f(0,0))
				rot_center = t.getCenter();
			else
				rot_center = gs::Vec2f(_X(s.rot_pt.x), _Y(s.rot_pt.y));

			t.rotate((float)s.rot_ang, &rot_center, s.clockwise);
		}

		success = success & t.draw(ren, s.color, s.fill, bm);
	}
	else
	if(s.type == Shape_P::S) //Segment.
	{
		Vec2f pt1 = {_X(s.pos.x), _Y(s.pos.y)};
		Vec2f pt2 = {_X(s.pos2.x), _Y(s.pos2.y)};

		if(s.rot_ang != 0)
		{
			Vec2i rot_center = {static_cast<int>(_X(s.rot_pt.x)), static_cast<int>(_Y(s.rot_pt.y))};

			pt1 = rotatePoint(rot_center.x, rot_center.y, pt1.x, pt1.y, s.rot_ang, s.clockwise).to<float>();
			pt2 = rotatePoint(rot_center.x, rot_center.y, pt2.x, pt2.y, s.rot_ang, s.clockwise).to<float>();
		}

		success = success & drawLine(ren, pt1, pt2, s.color, bm);
	}


	if(not success)
		gs::lastError(gs::str("from 'drawShape()' : ")+gs::lastError());

	return success;
}


}//namespace gs
