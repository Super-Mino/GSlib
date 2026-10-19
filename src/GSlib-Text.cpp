#include "GSlib-Text.hpp"
#include <cctype>
#include <stack>
#include <functional>
#include <algorithm>


//Maj : 26/08/26 (à moins d'oubli de modifier cette ligne ou autre).



namespace gs
{


std::string getLastTextChunk(std::string text, char end_ln_ch)
{
	//Sur un texte/str qui contient plusieurs lignes (ou retour à la ligne) cette fonction renvoie la dernière ligne qui n'a pas de caractère de fin de ligne. Par exemple, considérons '%' comme le caractère de fin de ligne, alors dans "abc%defgh%ijk" le return serait "ijk", et dans "abc%defgh%ijk%" le return serait "" (str vide, car ça se termine sur un caractère de fin de ligne).
	//Et oui par soucis de simplicité, on n'autorise pas de caractère fin de ligne UTF-8 autre que ASCII, permettant ainsi de se servir du 'char' et du 'std::string' tranquillement.

	try{

	if(text.empty()) return text;

	const int not_yet_initialized = -12345;
	int last_non_end_ln_ch__char_idx=not_yet_initialized; //A rebours, l'indice du dernier 'char' qui n'était pas un caractère de fin de ligne.

	for(int i=static_cast<int>(text.length())-1; i>=0; --i) //Parcours à rebours.
	{
		//On parcours à reculons jusqu'à peut-être trouver un caractère de fin de ligne, et entre-temps on met à jour l'indice du dernier caractère qui n'était pas un caractère de fin de ligne.

		if(text.at(i) == end_ln_ch)
			break;

		last_non_end_ln_ch__char_idx = i;
	};

	if(last_non_end_ln_ch__char_idx==not_yet_initialized) //Si l'on a pas trouvé (en partant de la fin) un caractère différent de celui de fin de ligne,
		return std::string(); //alors on renvoie une chaîne vide, puisque cela veut dire que dès la fin de la chaîne il y a eu un caractère de fin de ligne (donc un retour à la ligne) et d'après les attentes de la fonction, dans ce cas de figure on renvoie une chaîne vide.

	if(last_non_end_ln_ch__char_idx < 0 or last_non_end_ln_ch__char_idx > text.size()) //Pour sécuriser notamment le '.substr()'.
		return std::string();

	return text.substr(last_non_end_ln_ch__char_idx);

	}
	catch(std::exception& excep)
	{
		std::cout << "!! excep from 'getLastTextChunk()' : " << excep.what() << '\n';
	}
	return std::string(); //En cas d'exception.
};










Font::Font(Renderer* ren, const std::string& font_path, size_t ptsize, te::Font_Rendering_Type default_type)
: m_default_type(default_type)
{
	//m_ptsize = ptsize;
	init(ren, font_path, ptsize, default_type);
}


//____________________________________________________________________________________
//____________________________________________________________________________________


bool Font::isInit() const {return m_is_init;}

void Font::setShowBackground(bool val) {m_show_background = val;}

bool Font::getShowBackground() const {return m_show_background;}

size_t Font::getPtsize() const {return m_ptsize;}

void Font::setRenderingType(te::Font_Rendering_Type new_type) {	m_default_type = new_type;	}

te::Font_Rendering_Type Font::getRenderingType() const {return m_default_type;}

void Font::setBackgroundColor(Color bg_color) {m_bg_color = bg_color;}

Color Font::getBackgroundColor() const {return m_bg_color; }

void Font::setRenderer(Renderer* ren) {	m_ren_ref = ren;}

Renderer* Font::getRenderer() {return m_ren_ref;}

void Font::setBlendMode(Blend_Mode blend_mode) {m_blend_mode = blend_mode;}

Blend_Mode Font::getBlendMode() const {return m_blend_mode;}

void Font::setScaleMode(Scale_Mode scale_mode) {m_scale_mode = scale_mode;}

Scale_Mode Font::getScaleMode() const {return m_scale_mode;}

void Font::setWiderSpaceMode(bool active_state, float scale_factor)
{
	if(scale_factor <= 0.f) scale_factor = 2.4f;
	else
	if(scale_factor < 1.f) scale_factor += 1.f;

	m_wider_space_token_mode = active_state;
	m_wider_space_factor = scale_factor;
}

gs::Vec_Bool_Float Font::getWiderSpaceMode() const
{
	gs::Vec_Bool_Float res;
	res.b = m_wider_space_token_mode;
	res.f = m_wider_space_factor;

	return res;
}


void Font::setUserInterventionCallback(void (*userInterventionCallback)(Surface*, const std::string&))
{m_userInterventionCallback = userInterventionCallback;}


void (*Font::getUserInterventionCallback())(Surface*, const std::string&)
{return m_userInterventionCallback;}


std::string Font::lastError() const {return gs::lastError();}


}





namespace gs
{

Rendered_Str_Bank::Rendered_Str_Bank(Font* font)
{
	setFont(font, false);
}


Texture* Rendered_Str_Bank::get(std::string str,
							uint8_t style, uint16_t ptsize,
							uint32_t fg_color, uint32_t bg_color)
//Méthode 1
/*
{
	//Vérifications de validité
	if(not m_font)
	{
		gs::lastError("||Warning|| in 'Rendered_Str_Bank::get()', gs::Font* is nullptr (no have associated font). Use 'Rendered_Str_Bank::setFont()'. ");
		return nullptr;
	}

	if(not m_font->isInit())
	{
		gs::lastError("||Warning|| in 'Rendered_Str_Bank::get()', gs::Font is not init. Use 'Rendered_Str_Bank::setFont()' with a loaded gs::Font. ");
		return nullptr;
	}



	//Vérifications de présence.
	{
		if(not m_bank.contains(style))
			m_bank[style] =
				std::unordered_map<uint16_t,
					std::unordered_map<uint32_t,
						std::unordered_map<uint32_t,
							std::unordered_map<std::string, Texture*>>>>();

		if(not m_bank.at(style).contains(ptsize))
			m_bank.at(style)[ptsize] =
					std::unordered_map<uint32_t,
						std::unordered_map<uint32_t,
							std::unordered_map<std::string, Texture*>>>();

		if(not m_bank.at(style).at(ptsize).contains(fg_color))
			m_bank.at(style).at(ptsize)[fg_color] =
						std::unordered_map<uint32_t,
							std::unordered_map<std::string, Texture*>>();

		if(not m_bank.at(style).at(ptsize).at(fg_color).contains(bg_color))
			m_bank.at(style).at(ptsize).at(fg_color)[bg_color] =
							std::unordered_map<std::string, Texture*>();
	}



	//Vérification de présence n°2.
	auto& bank_s_p_f_b = m_bank.at(style).at(ptsize).at(fg_color).at(bg_color);
	auto it = bank_s_p_f_b.find(str);
	if(it == bank_s_p_f_b.end())
	{
		te::Font_Rendering_Type ren_type = te::SHADED;
		if(bg_color == 0) //Si la couleur d'arrière plan est totalement transparente.
			ren_type = te::BLENDED;


		//Afin d'avoir accès à chaque octet, l'octet du rouge, du vert ...
		uint8_t* bg_col_ptr = (uint8_t*)(&bg_color);
		uint8_t* fg_col_ptr = (uint8_t*)(&fg_color);
		Color bg_color_ = color(int(bg_col_ptr[3]),int(bg_col_ptr[2]),
									int(bg_col_ptr[1]),int(bg_col_ptr[0])); //(RGBA).

		if(style != m_font->getFontStyle()) m_font->setFontStyle(style);
		if(ptsize != m_font->getPtsize()) m_font->setPtsize(int(ptsize));


		//Génération/dessin de la chaine de caractères.
		Texture* rendered_texture =
			m_font->render(str.c_str(), color(int(fg_col_ptr[3]),int(fg_col_ptr[2]),
											  int(fg_col_ptr[1]),int(fg_col_ptr[0])),
						   ren_type, &bg_color_, nullptr); //Couleur en RGBA.

		if(not rendered_texture)
		{
			gs::lastError(std::string("from 'Rendered_Str_Bank::get()' : ") +gs::lastError());
			return nullptr;
		}


		//Rajout dans la banque.
		bank_s_p_f_b[str] = rendered_texture;
		addToClearAtTheEnd(rendered_texture); //Sécurité.
		return rendered_texture;
	}


	return it->second; //m_bank.at(style).at(ptsize).at(fg_color).at(bg_color).at(str);
}
*/

//Méthode 2
/*
{
	//Vérifications de validité
	if(not m_font)
	{
		gs::lastError("||Warning|| in 'Rendered_Str_Bank::get()', gs::Font* is nullptr (no have associated font). Use 'Rendered_Str_Bank::setFont()'. ");
		return nullptr;
	}

	if(not m_font->isInit())
	{
		gs::lastError("||Warning|| in 'Rendered_Str_Bank::get()', gs::Font is not init. Use 'Rendered_Str_Bank::setFont()' with a loaded gs::Font. ");
		return nullptr;
	}

	if(str.length() == 0)
	{
		gs::lastError("||Warning|| in 'Rendered_Str_Bank::get()', 'str.length() == 0'. ");
		return nullptr;
	}




	//Déduction du hash code en fonction du texte, et des différents paramètres. On va rassembler ces derniers dans une chaîne et en ressortir un hash code.

	size_t hash_code;

		//Composition de l'identifiant de la commande.

		//Organisation de order_str_id : "texteSSTTTRRRGGGBBBAAARRRGGGBBBAAA" -> S: style e.g 04, T: taille e.g 035, RGBA fg et RGBA bg.

		//Je complète avec des '0', ainsi par exemple 24 -> "024", 3 -> "003", pour garder un format cohérent et éviter que les commandes ne rentrent en conflit.
		//Par exemple si on a deux commandes avec : même texte, même style, même ptsize et même fg_color, mais avec des bg_color différents qui sont pour l'un (en format rgba) : 231,12,48,255 et pour l'autre 23,112,48,255,
		//et bien, sans '0' de complétion, les deux auraient la même order_str_id (dont la fin serait "2311248255") ce qui n'est pas souhaitable ici. Alors qu'avec les '0' de complétion,
		//l'un aurait "...231012048255" et l'autre "...023112048255" qui sont bien distinct.

		std::string order_str_id = str; //On commence par placer le texte.


		auto toStr = [](uint32_t value, uint8_t target_length){
			std::string value_str = std::to_string(value);

			if(value_str.length() > target_length)
				value_str = value_str.substr(0, target_length);

			else if(value_str.length() < target_length)
				value_str = std::string(target_length-value_str.length(), '0') + value_str;

			return value_str;
		};

		std::string style_str = toStr(style, 2u); //Je réserve deux chiffres pour le style, même si au moment où j'écris ce code le style avec la plus grande valeur est FONT_STYLE_STRIKETHROUGH avec 8 pour valeur.
		std::string ptsize_str = toStr(ptsize, 3u); //Je réserve trois chiffres pour le ptsize.

		std::string colors_str;
		std::vector<Color> colors{color(fg_color), color(bg_color)};

		for(Color& col : colors)
		{
			uint8_t rgba[4]={col.r,col.g,col.b,col.a};

			for(int i=0; i<4; ++i)
				colors_str += toStr(rgba[i], 3u);
		}

		order_str_id += style_str + ptsize_str + colors_str;
		static std::hash<std::string> hasher;

	hash_code = hasher(order_str_id);


	//Vérifications de présence. Si non présente, on génère la commande.
	auto it = m_bank.find(hash_code);
	if(it == m_bank.end())
	{
		te::Font_Rendering_Type ren_type = te::SHADED;
		if(bg_color == 0) //Si la couleur d'arrière plan est totalement transparente.
			ren_type = te::BLENDED;


		//Application du style et de la taille du texte.
		if(style != m_font->getFontStyle()) m_font->setFontStyle(style);
		if(ptsize != m_font->getPtsize()) m_font->setPtsize(ptsize);


		//Génération/dessin de la chaine de caractères.
		Color fg_color_ = colors.at(0);
		Color bg_color_ = colors.at(1);

		Texture* rendered_texture =
			m_font->render(str.c_str(), fg_color_, ren_type, &bg_color_, nullptr);

		if(not rendered_texture)
		{
			gs::lastError(std::string("from 'Rendered_Str_Bank::get()' : ") +gs::lastError());
			return nullptr;
		}


		//Rajout dans la banque.
		m_bank[hash_code] = rendered_texture;
		addToClearAtTheEnd(rendered_texture); //Sécurité.
		return rendered_texture;
	}


	return it->second; //m_bank.at(hash_code);
}
*/

//Méthode 3
{
	//Vérifications de validité
	if(not m_font)
	{
		gs::lastError("||Warning|| in 'Rendered_Str_Bank::get()', gs::Font* is nullptr (no have associated font). Use 'Rendered_Str_Bank::setFont()'. ");
		return nullptr;
	}

	if(not m_font->isInit())
	{
		gs::lastError("||Warning|| in 'Rendered_Str_Bank::get()', gs::Font is not init. Use 'Rendered_Str_Bank::setFont()' with a loaded gs::Font. ");
		return nullptr;
	}

	if(str.length() == 0)
	{
		gs::lastError("||Warning|| in 'Rendered_Str_Bank::get()', 'str.length() == 0'. ");
		return nullptr;
	}




	//Vérifications de présence. Si non présente, on génère la commande.
	auto it = m_bank.find({str, style, ptsize, fg_color, bg_color});
	if(it == m_bank.end())
	{
		te::Font_Rendering_Type ren_type = te::SHADED;
		if(bg_color == 0u) //Si la couleur d'arrière plan est totalement transparente.
			ren_type = te::BLENDED;


		//Application du style et de la taille du texte.
		if(style != m_font->getFontStyle()) m_font->setFontStyle(style);
		if(ptsize != m_font->getPtsize()) m_font->setPtsize(ptsize);


		//Génération/dessin de la chaine de caractères.
		Color fg_color_ = color(fg_color);
		Color bg_color_ = color(bg_color);

		Texture* rendered_texture =
			m_font->render(str.c_str(), fg_color_, ren_type, &bg_color_, nullptr);

		if(not rendered_texture)
		{
			gs::lastError(std::string("from 'Rendered_Str_Bank::get()' : ") +gs::lastError());
			return nullptr;
		}

		//Rajout dans la banque.
		m_bank[std::make_tuple(str, style, ptsize, fg_color, bg_color)] = rendered_texture;
		addToClearAtTheEnd(rendered_texture); //Sécurité.
		return rendered_texture;
	}


	return it->second;
}





bool Rendered_Str_Bank::setFont(Font& font, bool free_bank_prev_memory)
{
	m_font = &font;
	this->clear(free_bank_prev_memory); //On vide car les caractères précédemment générés ne sont probablement pas de la même police que le nouveau font.
	return true;
}

bool Rendered_Str_Bank::setFont(Font* font, bool free_bank_prev_memory)
{
	if(font)
	{
		m_font = font;
		this->clear(free_bank_prev_memory); //On vide car les caractères précédemment générés ne sont probablement pas de la même police que le nouveau font.
		return true;
	}

	return false;
}

Font* Rendered_Str_Bank::getFont() const {return m_font;}

void Rendered_Str_Bank::clear(bool free_memory)
{
	if(free_memory)
		for(auto [key, texture] : m_bank)
			if(texture)
				destroyTexture(texture);

	m_bank.clear();
}


} //namespace gs







namespace gs
{

bool Text_Line::debug__show_area_rect = false;

Text_Line::Text_Line(Rendered_Str_Bank* bank)
{
	m_str_bank = bank;
}


bool Text_Line::initAll(Renderer* ren, Font& font, Rendered_Str_Bank& bank, const char* font_path, uint16_t ptsize, uint32_t fg_rgba_col)
{
	//'Text_Line::initAll()' initialise le font et la banque (si pas déjà fait), afin de pouvoir être utilisés par le gs::Text_Line.

	if(not ren)
	{
		gs::lastError("||Warning|| in 'Text_Line::initAll()', invalid given renderer.");
		return false;
	}

	bool success = true;

	if(not font.isInit())
		success = font.init(ren, font_path, ptsize);

	if(not success) goto _end_;

	bank.setFont(font, false);

	this->setBank(bank);
	this->clearRules(FONT_STYLE_NORMAL,ptsize,fg_rgba_col,0u);

	_end_:

	if(not success)
		gs::lastError(gs::str("from 'Text_Line::initAll()' : ")+gs::lastError());
	else
		unselectTextPortion();

	return success;
}


bool Text_Line::setBank(Rendered_Str_Bank& bank)
{
	m_str_bank = &bank;
	m_is_up_to_date = false;
	unselectTextPortion();
	return true;
}

bool Text_Line::setBank(Rendered_Str_Bank* bank)
{
	m_str_bank = bank;
	m_is_up_to_date = false;
	unselectTextPortion();
	return bank!=nullptr;
}

Rendered_Str_Bank* Text_Line::getBank() const
{
	return m_str_bank;
}



bool Text_Line::setEndLineChar(const char ch)
{
	m_end_line_char = ch; return true;
}

char Text_Line::getEndLineChar() const
{
	return m_end_line_char;
}


std::string Text_Line::setText(std::string text)
{
	//return : la partie du texte non prise en compte (ou rien si tout est pris).

	std::string to_apply_str = text;

	if(text.contains(m_end_line_char)) //Si le texte reçu contient un caractère de fin de ligne, on ne prend que ce qu'il y a avant le caractère de fin de ligne (lui-même inclus).
	{
		int end_line_char_index = text.find(m_end_line_char);

		//La chaîne prise en compte.
		to_apply_str = text.substr(0, end_line_char_index+1); //+1 car on prend aussi le caractère de fin de ligne.

		//Chaine rejetée.
		text = text.substr(end_line_char_index+1);
	}
	else
		text = ""; //Sinon c'est qu'on peut tout prendre.


	m_text = to_apply_str;
	m_is_up_to_date = false;
	unselectTextPortion();
	return text;
}



std::string Text_Line::getText() const
{
	return m_text;
}



std::string Text_Line::getCursorChar() const
{
	if(not m_is_up_to_date)
	{
		gs::lastError("||Warning|| in 'Text_Line::getCursorChar()', Text_Line not up to date. [Use 'Text_Line::update()' before.] ");
		return std::string();
	}

	return getChar(m_virtual_cursor_pos);
}




std::string Text_Line::getTextFromBeginToCursor() const
{
	if(not m_is_up_to_date)
	{
		gs::lastError("||Warning|| in 'Text_Line::getTextFromBeginToCursor()', `*this' is not up to date. [Use 'Text_Line::update()' before.] ");
		return std::string();
	}

	if(m_text.empty() or m_chars_byte_count.empty()) return std::string();

	int text_cursor_index = getStdstringPosByVirtualCurPos(m_virtual_cursor_pos);

	return m_text.substr(0, text_cursor_index);
}




std::string Text_Line::getTextFromCursorToEnd() const
{
	if(not m_is_up_to_date)
	{
		gs::lastError("||Warning|| in 'Text_Line::getTextFromCursorToEnd()', `*this' is not up to date. [Use 'Text_Line::update()' before.] ");
		return std::string();
	}

	if(m_text.empty() or m_chars_byte_count.empty()) return std::string();

	int max_cursor_pos = getMaxCursorPos();
	int text_cursor_index = getStdstringPosByVirtualCurPos(m_virtual_cursor_pos);

	if(text_cursor_index >= m_text.size() or m_virtual_cursor_pos > max_cursor_pos)
		return std::string();

	if(m_virtual_cursor_pos == max_cursor_pos) //Si le curseur est au max.
		return m_text.substr(m_text.size()-1);


	return m_text.substr(text_cursor_index);
}




std::string Text_Line::getChar(int cursor_pos) const
{
	if(not m_is_up_to_date)
	{
		gs::lastError("||Warning|| in 'Text_Line::getChar()', Text_Line not up to date. [Use 'Text_Line::update()' before.] ");
		return std::string();
	}

	int max_cursor_pos = getMaxCursorPos();

	if(m_text.empty() or m_chars_byte_count.empty() or cursor_pos < 0 or cursor_pos > max_cursor_pos)
		return std::string();


	int stdstringchar_idx = getStdstringPosByVirtualCurPos(cursor_pos);

	if(stdstringchar_idx >= m_text.size())
		return std::string();


	if(cursor_pos == max_cursor_pos)//m_chars_byte_count.size()) //Si le curseur est au max.
		return m_text.substr(m_text.size()-1);


	return m_text.substr(stdstringchar_idx, m_chars_byte_count.at(cursor_pos));
}




int Text_Line::getStdstringPosByVirtualCurPos(int virtual_cursor_pos) const
{
	int index = 0;

	//Cumul des tailles de tous les caractères jusqu'à la position du curseur. (Cela afin de retrouver la position du curseur au niveau du std::string).
	for(int i=0; i<(virtual_cursor_pos); i++)
	{
		if(i >= int(m_chars_byte_count.size())) //Tous les caractères ont été parcourus.
		{
			index += 1; //Pour le caractère de fin de ligne.
			break;
		}

		index += m_chars_byte_count.at(i);;
	}

	if(index < 0)
		return 0;

	return index;
}



bool Text_Line::setPos(gs::Vec2i pos)
{
	m_pos = pos; return true;
}

gs::Vec2i Text_Line::getPos() const
{
	return m_pos;
}



bool Text_Line::setSize(gs::Vec2u size)
{
	m_size = size; return true;
}

gs::Vec2u Text_Line::getSize() const
{
	return m_size;
}

gs::Vec2i Text_Line::getSize_i() const
{
	return m_size.to<int>();
}


Rect Text_Line::getArea() const
{
	try{

	if(m_size.x != 0 and m_size.y != 0)
		return Rect(m_pos.x, m_pos.y, static_cast<int>(m_size.x), static_cast<int>(m_size.y));

	int max = GetMax(m_size.x, m_size.y);
	if(max > 0)
		return Rect(m_pos.x, m_pos.y, max, max);

	return Rect(m_pos.x, m_pos.y, m_curr_rules.ptsize, m_curr_rules.ptsize);

	}
	catch(std::exception& excep)
	{
		gs::lastError("!! excep in 'Text_Line::getArea()' : ", excep.what());
	}

	return Rect(m_pos.x, m_pos.y, -1, -1);
}


unsigned int Text_Line::getTextTotalRequiredWidth() const
{
	if(not m_is_up_to_date)
	{
		gs::lastError("||Warning|| in 'Text_Line::getTextTotalRequiredWidth()', line not up to date. [Use Text_Line::update() before.] ");
		return 0;
	}

	// unsigned int total=0;
	// for(const short& w : m_chars_width)
	// 	total += w;

	// return total;

	return m_total_required_w; //Calculé dans 'Text_Line::update()'.
}



bool Text_Line::setMaxSize(gs::Vec2u max_size)
{
	m_max_size = max_size; return true;
}

gs::Vec2u Text_Line::getMaxSize() const
{
	return m_max_size;
}


int Text_Line::determineCursorPosByPixelsPos(int pixel_pos) //Renvoie la position (en unité de caractère) qu'aurait le curseur si on le déterminait par rapport à une position en pixel (position relative à la 'gs::Text_Line').
{
	int virtual_cursor_pos=0;
	int sum = 0;
	pixel_pos = ((pixel_pos < 0) ? 0 : pixel_pos);

	for(int i=0; i<m_chars_width.size(); ++i)
	{
		short width = m_chars_width[i];
		sum += width/2;
		if(pixel_pos < sum)
		{
			virtual_cursor_pos = i;
			break;
		}
		sum -= width/2;
		sum += width;

		if(i == int(m_chars_width.size())-1 ) //Si on est au dernier tour de boucle, et pas défini de postion, on met le curseur après le dernier caractère (juste avant celui de fin de ligne).
			virtual_cursor_pos = getMaxCursorPos();
	}

	return virtual_cursor_pos;
}


int Text_Line::setCursorPosByPixels(int pixel_index)
{
	m_virtual_cursor_pos = determineCursorPosByPixelsPos(pixel_index);
	return m_virtual_cursor_pos;

	// int sum = 0;
	// pixel_index = ((pixel_index < 0) ? 0 : pixel_index);

	// for(int i=0; i<m_chars_width.size(); ++i)
	// {
		// short width = m_chars_width[i];
		// sum += width/2;
		// if(pixel_index < sum)
		// {
			// m_virtual_cursor_pos = i;
			// break;
		// }
		// sum -= width/2;
		// sum += width;

		// if(i == int(m_chars_width.size())-1 ) //Si on est au dernier tour de boucle, et pas défini de postion, on met le curseur après le dernier caractère (juste avant celui de fin de ligne).
			// m_virtual_cursor_pos = m_chars_width.size(); //Ps: m_chars_width ne comptabilise pas le caractère de fin de ligne. D'ailleur le caractère de fin de ligne est généralement ignoré dans les traitements relatifs à l'affichage.
	// }

	// return m_virtual_cursor_pos;
}



int Text_Line::getCursorPosPixelIndex() const
{
	int pos = 0;
	int i=0;
	for(auto& ch_width : m_chars_width)
	{
		if(i == m_virtual_cursor_pos) return pos;

		pos+=ch_width;
		++i;
	}
	return pos;
}



bool Text_Line::setShowCursor(bool new_state)
{
	m_show_cursor = new_state; return new_state;
}

bool Text_Line::getShowCursor() const
{
	return m_show_cursor;
}



int Text_Line::setCursorPos(int cursor_index)
{
	if(cursor_index < 0)
		m_virtual_cursor_pos = 0;
	else
	{
		int max_cursor_pos = getMaxCursorPos();

		if(cursor_index > max_cursor_pos)
			m_virtual_cursor_pos = max_cursor_pos;
		else
			m_virtual_cursor_pos = cursor_index;
	}

	return m_virtual_cursor_pos;
}



int Text_Line::getCursorPos() const
{
	return m_virtual_cursor_pos;
}

int Text_Line::incrCursorPos(int val)
{
	int pos = m_virtual_cursor_pos+val;

	pos = ((pos > int(m_chars_width.size())) ? int(m_chars_width.size()) : pos); //Même raisonnement qu'avec 'setCursorPos()'.
	pos = ((pos < 0) ? 0 : pos);

	m_virtual_cursor_pos = pos;

	return m_virtual_cursor_pos;
}


int Text_Line::setMaxCursorPos()
{
	return setCursorPos(getMaxCursorPos());
}




int Text_Line::getMaxCursorPos() const
{
	return static_cast<int>(m_chars_width.size()); //Ps: m_chars_width ne comptabilise pas le caractère de fin de ligne. D'ailleur le caractère de fin de ligne est généralement ignoré dans les traitements relatifs à l'affichage.
}



bool Text_Line::show(Renderer* ren, int start_x, char tabulation_size, Vec2i up_down_limit_y, bool hide_the_after_cursor_part)
{
	if(not ren)
	{
		gs::lastError("||Warning|| in 'Text_Line::show()', given renderer is nullptr. ");
		return false;
	}

	if(not m_str_bank)
	{
		gs::lastError("||Warning|| in 'Text_Line::show()', have invalid gs::Rendered_Str_Bank*. Use 'Text_Line::setBank()' with a valid one before. ");
		return false;
	}

	bool full_success = true;

	tabulation_size = ((tabulation_size < 2) ? 2 : tabulation_size);

	if(not m_is_up_to_date)
		update(tabulation_size);


	//Afin de parcourir les différents changements de règles.
	bool edit_rules = (m_rules.size() == 0 ? false : true);
	std::map<int, Print_Rules>::iterator rules_it = m_rules.begin();


	int curr_index = 0; //Indices des lettres (leur rang) selon la perception humaine (UTF-8).
	//On va coller les caractère depuis le bas. S'il y a de l'espace en trop ce sera au dessus du caractère.
	int botom_y = m_pos.y + m_size.y-1;
	int curr_x = m_pos.x;
	int max_x = m_pos.x + m_size.x-1;

	start_x = ((start_x < 0) ? 0 : start_x); //La marge pour savoir à quel caractère commencer à dessiner.
	int start_x_count = 0;
	bool is_visible = false;

	bool show_cursor=false;
	int cursor_x=0;
	Color cursor_col={255,100,0,255};
	int cursor_up_y = m_pos.y - int(float(m_size.y)*0.2);
		if(cursor_up_y < up_down_limit_y.x) cursor_up_y = up_down_limit_y.x;

	int cursor_down_y = m_pos.y + m_size.y-1 + int(float(m_size.y)*0.2);
		if(cursor_down_y > up_down_limit_y.y) cursor_down_y = up_down_limit_y.y;
	// bool cursor_showed=false;


	bool is_on_multi_bytes_char = false;
	std::string _multi_bytes_char; //La compilation des différents 'char' qui forment le caractère courant, si c'est un caractère composé de plusieurs octets (en UTF-8).
	int remain_multi_char___count=0;


	auto isTheCharInSelection = [&](int ch_idx) {
		if(not m_have_a_selection) return false;
		return (m_selected_portion_boundaries.x <= ch_idx and ch_idx <= m_selected_portion_boundaries.y);
	};


	int drawn_ch_count = 0;
	for(char& ch : m_text)
	{
		//Le caractère de fin de ligne n'est pas comptabilisé;
		if(ch == m_end_line_char) break;

		if(hide_the_after_cursor_part and curr_index == m_virtual_cursor_pos) //Ce mode permet de n'afficher que ce qu'il y a avant le curseur.
			break;

		if(is_on_multi_bytes_char) //Si on est actuellement en train de compiler les différents 'char' qui composent le caractère courant.
		{
			_multi_bytes_char +=  ch;
			remain_multi_char___count--;

			if(remain_multi_char___count > 0) //Tant qu'il reste des 'char' pour compléter ce caractère on 'continue'.
				continue;
		}
		else
		{

			//On regarde si le caratère est composé de plusieurs 'char'.
			bool is_ok=false;
			unsigned int bytes_count = getBytesCountOfUTF8Charac_ByTheCharacHeaderByte(ch, is_ok); //Le nombre d'octets (de 'char') qu'occupe le caractère courant.
			if(not is_ok or bytes_count==0) //S'il y a erreur:
				continue;

			if(bytes_count > 1)
			{
				is_on_multi_bytes_char = true;
				_multi_bytes_char +=  ch;
				remain_multi_char___count = bytes_count-1;
				continue;
			}
		}



		//Gestion des mises à jour des règles.
		if(edit_rules)
		{
			if(curr_index == rules_it->first) //Si le rang du caractère est un rang qui change de règles (taille, style etc).
			{
				applyRules(rules_it->second); //m_rules[rules_it->first]);

				//On passe aux potentielles règles suivantes.
				rules_it ++;
				if(rules_it == m_rules.end())
					edit_rules = false;
			}
		}

		if(m_show_cursor)
			if(curr_index == m_virtual_cursor_pos)
			{
				cursor_x = curr_x;
				show_cursor = true;
			}


		char times_to_blit = 1;

		//Demande et réception de la commande.
		uint8_t style = m_curr_rules.style;
		uint16_t ptsize = m_curr_rules.ptsize;
		uint32_t fg = m_curr_rules.fg_color;
		uint32_t bg = m_curr_rules.bg_color;

		Texture* texture = nullptr;
		std::string str_key;

		if(is_on_multi_bytes_char)
		{
			str_key = _multi_bytes_char;
			is_on_multi_bytes_char = false;
			_multi_bytes_char.clear();
		}
		else
		if(ch == '\t')
		{
			str_key = std::string(1, ' ');
			times_to_blit = tabulation_size;
		}
		else
			str_key = std::string(1, ch);


		if(isTheCharInSelection(curr_index)) //En ce qui concerne la selection, on change l'arrière plan des caractères concernés. Cependant ça fait que dans le "pire" des cas d'utilisation de la selection, la banque stockerait deux versions pour un même caractère (une avec le surlignement, une sans). Ce n'est pas la seule manière de gérer la selection, mais j'ai opté pour c'est celle-ci.
			bg = m_selection_col;

		texture = m_str_bank->get(str_key, style, ptsize, fg, bg);


		if(not texture)
		{
			gs::lastError(std::string("from 'Text_Line::show()' : ")+gs::lastError());
			curr_x += int(float(ptsize)*0.8); //On avance de 80% du ptsize pour laisser un trou.
			full_success = false;
			continue;
		}


		//Dessin de la commande.
		for(int i=0; i< times_to_blit; i++)
		{
			float texture_w=0.f, texture_h=0.f;
			getTextureSize(texture, &texture_w, &texture_h);

			//Ajustement du w que l'on va prendre en compte:
			bool is_particular_case=false;
			int particular_case_w;
			if(isFlagPresentIn(style, FONT_STYLE_ITALIC)) //S'il y a de l'italique dans le style du caractère courant:
			{
				//if(str_key == "d" or str_key == "u")
				//{
				//	particular_case_w = static_cast<int>(texture_w*m_italic_w_percentage_to_take);
				//	is_particular_case = true; //Le but du cas particulier est d'afficher le caractère italic concerné sans rognage (notamment si le rognage empiète sur le-s trait-s du caractère), mais pour l'incrémentation de 'curr_x' l'on le considère comme étant rogné. Toutefois si les traitements ont fait que la taille finale soit plus petite que celle prévue par le rognage on incrémente avec la plus petite.
				//}
				//else
					texture_w = texture_w*m_italic_w_percentage_to_take;
			}


			FRect texture_src_rect = {0.f,0.f, texture_w,texture_h};


			//Contraintes de début d'affichage (sur la largeur).
			if(not is_visible)
			{
				if((start_x_count + texture_src_rect.w-1) >= start_x)
				{
					int prev_w = texture_src_rect.w;
					texture_src_rect.w = (start_x_count + texture_src_rect.w) - start_x;
					texture_src_rect.x = prev_w - texture_src_rect.w;

					is_visible = true;
				}
				else
				{
					start_x_count += texture_src_rect.w;
					continue;
				}
			}


			//Rognage si besoin. (Ps: tout est à la même échelle [src et dest] (pixel près).)
			if(texture_h > m_size.y) //Sur la hauteur.
			{
				texture_src_rect.y = texture_src_rect.h - m_size.y; //On décale le début de la source à là où c'est inclus dans la plage 'visible'.
				texture_src_rect.h = m_size.y; //On réduit la partie à afficher (pour notamment garder les proportions).
			}

			if((curr_x + texture_w-1) > max_x) //Sur la largeur.
			{
				texture_src_rect.w -= (curr_x + texture_w) - max_x; //Ceci agit aussi (juste un peu plus loin dans la fonction) sur le rectangle de destination.
			}


			//Les contraintes de coordonnées à ne pas dépasser.
			{
				int up_limit_y = up_down_limit_y.x;  //Les limites à ne pas franchir (à ne pas dépasser par le haut).
				int down_limit_y = up_down_limit_y.y; //(à ne pas dépasser par le bas).

				if(m_pos.y < up_limit_y)
				{
					texture_src_rect.y += (up_limit_y - m_pos.y);
					texture_src_rect.h -= (up_limit_y - m_pos.y);
				}

				if((m_pos.y + m_size.y-1) > down_limit_y)
				{
					texture_src_rect.h -= (m_pos.y + m_size.y-1) - down_limit_y;
					botom_y = down_limit_y;
				}

				//Pour la propreté.
				if(texture_src_rect.w < 0) texture_src_rect.w = 0;
				if(texture_src_rect.h < 0) texture_src_rect.h = 0;
			}


			FRect texture_dest_rect = texture_src_rect; //Les rectangles source et destination sont confondus au niveau des tailes car tout est à la même échelle.
			texture_dest_rect.x = curr_x;
			texture_dest_rect.y = botom_y - texture_dest_rect.h +1;
			bool res = drawTexture(ren, texture, &texture_src_rect, &texture_dest_rect);
			if(!res)
			{
				gs::lastError(std::string("from : 'Text_Line::show()' at 'drawTexture()' : ")+Backend_GetError());
				full_success = false;
			}
			else
				drawn_ch_count++;

			if(is_particular_case)
				curr_x += GetMin(texture_dest_rect.w, particular_case_w);
			else
				curr_x += texture_dest_rect.w;

			if(curr_x > max_x) //Oui la comparaison est '>' et non '>=', car le pixel maximum est inclus dans la plage "valide".
				break;
		}




		if(show_cursor)
		{
			if(is_visible)
			if(cursor_x < max_x)
			{
				drawLine(ren, {static_cast<float>(cursor_x-1), static_cast<float>(cursor_up_y)}, {static_cast<float>(cursor_x-1), static_cast<float>(cursor_down_y)}, cursor_col);
				drawLine(ren, {static_cast<float>(cursor_x), static_cast<float>(cursor_up_y)}, {static_cast<float>(cursor_x), static_cast<float>(cursor_down_y)}, cursor_col);
			}

			show_cursor = false;
			// cursor_showed = true;
		}


		curr_index++;

		if(curr_x > max_x)
			break;
	}

	//Dessin du curseur lorsqu'il se trouve à la dernière position.
	if(m_show_cursor and (curr_index == m_virtual_cursor_pos or curr_index == m_virtual_cursor_pos-1)) //C'est à dire que le curseur est à la dernière position, au vu du code plus haut.
	{
		cursor_x = GetMin(curr_x, max_x);

		if(is_visible or int(m_text.size()) <= 1)
		if(cursor_x <= max_x)
		{
			if(not (int(m_text.size()) <= 1 and start_x_count < start_x)) //Si le texte ne comporte rien ou seulement le caractère de fin de ligne (le curseur est alors au début), et que le point de commencement n'est pas atteint : on affiche pas le curseur (car masqué par le scrolling du start_x).
			{
				drawLine(ren, {static_cast<float>(cursor_x-1), static_cast<float>(cursor_up_y)}, {static_cast<float>(cursor_x-1), static_cast<float>(cursor_down_y)}, cursor_col);
				drawLine(ren, {static_cast<float>(cursor_x), static_cast<float>(cursor_up_y)}, {static_cast<float>(cursor_x), static_cast<float>(cursor_down_y)}, cursor_col);
			}
		}
	}


	if(m_have_a_selection and drawn_ch_count == 0) //Pour les lignes "vides" et sélectionnées.
	{
		Blend_Mode bm = BLENDMODE_BLEND;
		drawRect(ren, frect(getArea()), color(m_selection_col), true, &bm);
	}

	//Partie debug
	//{
		//drawRect(ren, getArea(), {0,150,255,255}, false);
		//if(m_have_a_selection)
		//{
		//	Rect r = getArea(); r.w = 15; r.h =12;
		//	drawRect(ren, r, {255,80,0,255}, true);
		//}
	//}

	if(debug__show_area_rect)
		gs::drawRect(ren, frect(getArea()), {0,50,100,255}, false);

	return full_success;
}


gs::Vec2u Text_Line::update(char tabulation_size)
{
	if(not m_str_bank)
	{
		gs::lastError("||Warning|| in 'Text_Line::update()', have invalid gs::Rendered_Str_Bank*. Use 'Text_Line::setBank()' with a valid one. ");
		return {0,0};
	}

	if(m_is_up_to_date)
		return m_size;


	if(tabulation_size < 2)
		tabulation_size = 2;

	//Afin de parcourir les différents changements de règles.
	bool edit_rules = (m_rules.size() == 0 ? false : true);
	std::map<int, Print_Rules>::iterator rules_it = m_rules.begin();

	//Indices des lettres (leur rang).
	int curr_index = 0;
	unsigned int max_w = 0;
	unsigned int max_h = 0;
	unsigned int max_ptsize = 0; //m_curr_rules[1];

	m_total_required_w = 0;
	m_chars_width.clear(); //Cette variable permet de garder en mémoire les tailles des textures des différents caractères, sans avoir à refaire une boucle de 'm_str_bank->get()';
	m_chars_byte_count.clear(); //Celle-ci permet de garder en mémoire le nombre d'octets qu'occupe les différents caractères.


	bool is_on_multi_bytes_char = false;
	std::string _multi_bytes_char; //La compilation des différents 'char' qui forment le caractère courant, si c'est un caractère composé de plusieurs octets (en UTF-8).
	int remain_multi_char___count=0;


	for(char& ch : m_text)
	{
		//Le caractère de fin de ligne n'est pas comptabilisé;
		if(ch == m_end_line_char) break;


		if(is_on_multi_bytes_char) //Si on est actuellement en train de compiler les différents 'char' qui composent le caractère courant.
		{
			_multi_bytes_char +=  ch;
			remain_multi_char___count--;

			if(remain_multi_char___count > 0) //Tant qu'il reste des 'char' pour compléter ce caractère on 'continue'.
				continue;
		}
		else
		{

			//On regarde si le caratère est composé de plusieurs 'char'.
			bool is_ok=false;
			unsigned int bytes_count = getBytesCountOfUTF8Charac_ByTheCharacHeaderByte(ch, is_ok); //Le nombre d'octets (de 'char') qu'occupe le caractère courant.
			if(not is_ok or bytes_count==0) //S'il y a erreur:
				continue;

			if(bytes_count > 1)
			{
				is_on_multi_bytes_char = true;
				_multi_bytes_char +=  ch;
				remain_multi_char___count = bytes_count-1;
				continue;
			}
		}



		//Gestion des mises à jour des règles.
		if(edit_rules)
		{
			if(curr_index == rules_it->first) //Si le rang du caractère courant est un rang qui change de règles (taille, style etc).
			{
				applyRules(rules_it->second); //m_rules[rules_it->first]);

				//On passe aux potentielles règles suivantes.
				rules_it ++;
				if(rules_it == m_rules.end())
					edit_rules = false;
			}
		}

		curr_index++;

		uint8_t style = m_curr_rules.style;
		uint16_t ptsize = m_curr_rules.ptsize;
		uint32_t fg = m_curr_rules.fg_color;
		uint32_t bg = m_curr_rules.bg_color;

		Texture* texture = nullptr;
		char times_to_count = 1;
		std::string str_key;

		if(is_on_multi_bytes_char)
		{
			str_key = _multi_bytes_char;
			is_on_multi_bytes_char = false;
			_multi_bytes_char.clear();
			m_chars_byte_count.push_back(str_key.length());
		}
		else
		{
			m_chars_byte_count.push_back(1);

			if(ch == '\t')
			{
				str_key = std::string(1, ' ');
				times_to_count = tabulation_size;
			}
			else
				str_key = std::string(1, ch);
		}

		texture = m_str_bank->get(str_key, style, ptsize, fg, bg);


		if(not texture)
		{
			gs::lastError(std::string("from 'Text_Line::update()' : ")+gs::lastError());
			continue;
		}

		//Ajustement de la taille de la ligne en fonction des caractères à dessiner.
		float texture_w=0, texture_h=0;
		getTextureSize(texture, &texture_w, &texture_h);

		//Ajustement du w que l'on va prendre en compte:
		if(isFlagPresentIn(style, FONT_STYLE_ITALIC)) //S'il y a de l'italique dans le style du caractère courant:
			texture_w = texture_w*m_italic_w_percentage_to_take;

		max_w += texture_w*times_to_count;
		max_h = GetMax(max_h, texture_h);
		max_ptsize = GetMax(max_ptsize, ptsize);

		m_chars_width.push_back(texture_w*times_to_count);
		m_total_required_w+=texture_w*times_to_count;

	}

	//if(max_h <= 1) max_h = max_ptsize;
	if(max_h < 1)
	{
		if(max_ptsize > 0)
			max_h = max_ptsize;
		else
		if(m_rules.size())
		{
			applyRules(m_rules.begin()->second);
			max_h = m_curr_rules.ptsize;
		}
		else
			max_h = m_size.y;
	}

	//Limitations des bords.
	if(max_w > m_max_size.x)
		max_w = m_max_size.x;

	if(max_h > m_max_size.y)
		max_h = m_max_size.y;


	m_size = {max_w, max_h};
	m_is_up_to_date = true;

	return m_size;
}




bool Text_Line::isUpToDate() const {return m_is_up_to_date;}




bool Text_Line::addRules(int begin_index, uint8_t style, uint16_t ptsize, uint32_t fg_color, uint32_t bg_color)
{
	Print_Rules rules;
		rules.style = style;
		rules.ptsize = ptsize;
		rules.fg_color = fg_color;
		rules.bg_color = bg_color;

	m_rules[begin_index] = rules;
	m_is_up_to_date = false;

	return true;
}





bool Text_Line::clearRules(uint8_t style, uint16_t ptsize, uint32_t fg_color, uint32_t bg_color)
{
	m_rules.clear();
	return addRules(0, style, ptsize, fg_color, bg_color);
}



bool Text_Line::addText(const std::string& str, std::string* out, bool allow_cursor_to_be_moved) //Au niveau du curseur.
{
	if(not m_is_up_to_date)
	{
		gs::lastError("||Warning|| in 'Text_Line::addText()', edit not allowed : line not up to date. [Use Text_Line::update() before.] ");
		return false;
	}

	int index = 0;

	for(int i=0; i<m_virtual_cursor_pos; i++) //Cumul des tailles de tous les caractères jusqu'à la position du curseur. (Cela afin de retrouver la position du curseur au niveau du std::string).
	{
		if(i >= int(m_chars_byte_count.size())) //J'imagine qu'ici ça fait en sorte que l'on ne puisse pas ajouter de texte après le caractère de fin de ligne.
			break;

		index += m_chars_byte_count.at(i);
	}

	m_text = m_text.substr(0, index) + str + m_text.substr(index);

	if(allow_cursor_to_be_moved)
		m_virtual_cursor_pos += getUTF8CharsCount(str); // #<A>

	m_is_up_to_date = false;


	if(str.contains(m_end_line_char)) //Si le texte ajouté rajoute un caractère de fin de ligne (faisant changer le texte à prendre en compte).
	{
		//Utilisation de `setText()' afin de (re)filtrer la chaîne de caractère.
		if(out)
			*out = setText(m_text);
		else
			setText(m_text);

		//Et on ajuste aussi la position du curseur pour ne pas aller au dela du caractère de fin de ligne (voir la ligne avec la balise #<A>, où le caractère de fin est prise en compte si présent).
		--m_virtual_cursor_pos;
		if(m_virtual_cursor_pos < 0) m_virtual_cursor_pos = 0;
	}
	else
		if(out) *out = std::string();


	unselectTextPortion();

	return true;
}


std::string Text_Line::removeFromCursorLeftSide(int count)
{
	std::string rmvd_part;

	if(not m_is_up_to_date)
	{
		gs::lastError("||Warning|| in 'Text_Line::removeFromCursorLeftSide()', removeFromCursorLeftSide not allowed : line not up to date. [Use Text_Line::update() before.] ");
		return rmvd_part;
	}

	if(count <= 0) return rmvd_part;

	for(int i = 0; i<count; i++)
	{
		if(m_virtual_cursor_pos <= 0)
			return rmvd_part;

		int index = 0; //C'est `m_virtual_cursor_pos' mais au niveau de std::string().
		char last_added_char_size=0; //Afin de reculer l'index, de la taille du dernier caractère (utf-8) pour ne pas le prendre en compte (le supprimer).
		//`last_added_char_size' est généralement soit 1 (pour les ascii), soit 2 (mais peut aller jusqu'à 4).

		for(int j=0; j<(m_virtual_cursor_pos); j++) //Cumul des tailles de tous les caractères jusqu'à la position du curseur. (Cela afin de retrouver la position du curseur au niveau du std::string).
		{
			if(j >= int(m_chars_byte_count.size())) //Si tous les caractères ont été parcourus.
				break;


			last_added_char_size = m_chars_byte_count.at(j);
			index += last_added_char_size;
		}

		rmvd_part = m_text.substr(index-last_added_char_size, last_added_char_size) + rmvd_part;
		m_text = m_text.substr(0, index-last_added_char_size) + m_text.substr(index);
		//Par exemple : nouveau text = [azer]t|[yuiop] = [azer|yuiop]. | = curseur.

		incrCursorPos(-1);

		m_is_up_to_date = false;
	}

	unselectTextPortion();

	return rmvd_part;
}




std::string Text_Line::removeFromCursorRightSide(int count)
{
	std::string empty_str;

	if(not m_is_up_to_date)
	{
		gs::lastError("||Warning|| in 'Text_Line::removeFromCursorRightSide()', removeFromCursorRightSide not allowed : line not up to date. [Use Text_Line::update() before.] ");
		return empty_str;
	}

	//Le principe ici est de déplacer le curseur de 'count' caractères vers la droite (si possible), et ainsi effacer les caractères qu'on a sauté, par la gauche. Cette technique permet d'éviter de réécrire complètement une méthode qui efface vers la droite, mais plutôt d'utiliser ce qu'on a déjà à disposition ^^.

	int prev_cursor_pos = getCursorPos();
	setCursorPos(prev_cursor_pos +count);
	int new_cursor_pos = getCursorPos(); //Il est important de récupérer la nouvelle position du curseur APRES l'appel de 'setCursorPos()', et non directement par la valeur de 'prev_cursor_pos + count', car 'setCursorPos()' peut limiter la position réellement affectée si la position reçue en argument dépasse la position maximale du curseur.


	if(prev_cursor_pos == new_cursor_pos) //S'il n'y a pas moyen de retirer vers la droite.
		return empty_str;

	return removeFromCursorLeftSide(new_cursor_pos - prev_cursor_pos);
}




void Text_Line::applyRules(const Print_Rules& rules)
{
	m_curr_rules = rules;
}



bool Text_Line::selectTextPortionByPixelsPos(int min_pix_pos, int max_pix_pos)
{
	if(not m_is_up_to_date)
	{
		gs::lastError("||Warning|| in 'Text_Line::selectTextPortionByPixelsPos()', line not up to date. [Use Text_Line::update() before.] ");
		m_have_a_selection = false;
		return false;
	}

	if(max_pix_pos < min_pix_pos)
		std::swap(min_pix_pos, max_pix_pos);


	//if(min_pix_pos >= m_size.x) //Si la sélection est hors text.
		//return false; //!! Ne pas limiter ainsi, car le texte peut être plus long que la taille de la zone "réservée" à l'affichage de la ligne, et on peut vouloir sélectionner du contenue hors champ d'affichage.


	m_selected_portion_boundaries.x = determineCursorPosByPixelsPos(min_pix_pos);
	m_selected_portion_boundaries.y = determineCursorPosByPixelsPos(max_pix_pos)-1;

	m_have_a_selection = m_selected_portion_boundaries.x <= m_selected_portion_boundaries.y;

	return m_have_a_selection;
}

bool Text_Line::selectTextPortionByPixelsPos_From(bool from_begin, int pix_pos) //Si 'from_begin==false', alors on selectionne à partir de la fin.
{
	if(not m_is_up_to_date)
	{
		gs::lastError("||Warning|| in 'Text_Line::selectTextPortionByPixelsPos_From()', line not up to date. [Use Text_Line::update() before.] ");
		m_have_a_selection = false;
		return false;
	}

	if(from_begin)
	{
		m_selected_portion_boundaries.x = 0;
		m_selected_portion_boundaries.y = determineCursorPosByPixelsPos(pix_pos)-1;
	}
	else //from end
	{
		m_selected_portion_boundaries.x = determineCursorPosByPixelsPos(pix_pos);
		m_selected_portion_boundaries.y = getMaxCursorPos();
	};

	m_have_a_selection = m_selected_portion_boundaries.x <= m_selected_portion_boundaries.y;

	return m_have_a_selection;
}


bool Text_Line::selectTextPortionByCursorsPos(int left_cursor_pos, int right_cursor_pos)
{
	//Si c'est un succès, la zone sélectionnée est celle entre les deux curseurs, ce qui fait que le dernier caractère sélectionné est celui juste à gauche du curseur de droite, et non le caractère qui est au niveau du curseur de droite. Exemple: dans "ab[cde(fghij", le '[' représente le curseur de gauche avec comme indice (pos) 2, et le '(' celui de droite avec comme indice (pos) 5. Le texte qui sera sélectionné est "cde", et le 'f' qui est lui au niveau du curseur de droite n'y est pas inclus. Note: si les arguments reçus ne correspondent pas à ce qui est attendu (c-à-d que celui de gauche est celui qui est à droite, et donc que celui de droite est celui qui est à gauche), la méthode se débrouille pour remettre les choses en place.

	if(not m_is_up_to_date)
	{
		gs::lastError("||Warning|| in 'Text_Line::selectTextPortionByCursorsPos()', line not up to date. [Use Text_Line::update() before.] ");
		m_have_a_selection = false;
		return false;
	}

	if(right_cursor_pos < left_cursor_pos)
		std::swap(left_cursor_pos, right_cursor_pos);

	int max_cursor_pos = getMaxCursorPos();

	if(left_cursor_pos < 0) left_cursor_pos = 0;

	int max_offset = 1;
	if(right_cursor_pos > max_cursor_pos)
	{
		right_cursor_pos = max_cursor_pos;
		max_offset=0; //Pour garder la position du curseur de droite au max, lorsque c'est au max.
	}

	m_selected_portion_boundaries.x = left_cursor_pos;
	m_selected_portion_boundaries.y = right_cursor_pos-max_offset;


	m_have_a_selection = m_selected_portion_boundaries.x <= m_selected_portion_boundaries.y;


	return m_have_a_selection;
}



bool Text_Line::selectTextPortionByCursorToken(const std::string& ASCII_token_separators)
{
	if(not m_is_up_to_date)
	{
		gs::lastError("||Warning|| in 'Text_Line::selectTextPortionByCursorToken()', line not up to date. [Use Text_Line::update() before.] ");
		m_have_a_selection = false;
		return false;
	}

	//Pour cela, on va faire étendre le curseur des deux côtés jusqu'à trouver une condition d'arrêt (sépratateur rencontré, fin ou début de ligne).

	int min_cursor_pos = 0;
	int max_cursor_pos = getMaxCursorPos();

	int curr_left_cursor_pos = m_virtual_cursor_pos;
	int curr_right_cursor_pos = m_virtual_cursor_pos;

	std::string cursor_ch = getCursorChar();
	bool one_type_allowed = (cursor_ch.length() == 1 and ASCII_token_separators.contains(cursor_ch.front())); //Si le caractère au niveau du curseur est un séparateur, on ne sélectionne que ce caractère de manière contiguë si possible.

	std::string end_ln_ch = std::string(1, m_end_line_char);


	//A gauche
	while(min_cursor_pos < curr_left_cursor_pos)
	{
		//Récupération du caractère qui précède, et on détermine si l'on recule ou non.

		std::string ch = getChar(curr_left_cursor_pos-1);

		if(one_type_allowed)
		{
			if(ch != cursor_ch)
				break;
		}
		else
		if(ch.length() == 1 and ASCII_token_separators.contains(ch.front())) //Si c'est un séprateur, on arrête là la progression de ce curseur.
			break;

		curr_left_cursor_pos--;
	}


	//A droite
	while(curr_right_cursor_pos < max_cursor_pos)
	{
		//Récupération du caractère courant, et on détermine si l'on avance ou non.

		std::string ch = getChar(curr_right_cursor_pos+1); // #<A>

		if(one_type_allowed)
		{
			if(ch != cursor_ch)
				break;
		}
		else
		if((ch.length() == 1 and ASCII_token_separators.contains(ch.front())) or ch == end_ln_ch) //Si c'est un séprateur, on arrête là la progression de ce curseur.
			break;

		curr_right_cursor_pos++; // #<B>
	}


	m_selected_portion_boundaries.x = curr_left_cursor_pos;
	m_selected_portion_boundaries.y = curr_right_cursor_pos; //Ici pas besoin du '-1' d'inclusion de la borne droite, car on regarde d'abord si la position peut être incluse ('curr_right_cursor_pos+1' (voir vers balise #<A>)) avant d'inclure ('curr_right_cursor_pos++' (voir vers balise #<B>)). Note: en effet la borne droite est incluse, or ce qu'on veut sélectionner est censé se trouver (à droite de la borne gauche (borne gauche incluse) et) à gauche de la borne droite (borne droite incluse) (selon les représentations visuelles du curseur/sélection en général). C'est pour cela que l'on réduit généralement de 1 la borne droite afin que le dernier caractère sélectionné soit bien celui "attendu" et non le caractère qui le suit.

	m_have_a_selection = m_selected_portion_boundaries.x <= m_selected_portion_boundaries.y;

	return m_have_a_selection;
}



bool Text_Line::selectAll()
{
	if(not m_is_up_to_date)
	{
		gs::lastError("||Warning|| in 'Text_Line::selectAll()', line not up to date. [Use Text_Line::update() before.] ");
		m_have_a_selection = false;
		return false;
	}

	m_selected_portion_boundaries.x = 0;
	m_selected_portion_boundaries.y = getMaxCursorPos();

	m_have_a_selection = m_selected_portion_boundaries.x <= m_selected_portion_boundaries.y;

	return m_have_a_selection;

}


bool Text_Line::haveSelection() const {return m_have_a_selection;}

void Text_Line::unselectTextPortion() {m_have_a_selection = false;}

void Text_Line::setSelectionColor(uint32_t col) {m_selection_col = col;}


std::string Text_Line::getSelectedText() const
{
	if(not m_have_a_selection)
		return std::string();

	//Récupération des indices des 'char' des bordures dans le 'std::string' par rapport aux positions (de curseur) des bordures de la selection.
	int stdstringchar_min_idx = getStdstringPosByVirtualCurPos(m_selected_portion_boundaries.x);
	int stdstringchar_max_idx = getStdstringPosByVirtualCurPos(m_selected_portion_boundaries.y+1);

	int stdstringchar_count = stdstringchar_max_idx-stdstringchar_min_idx;
	if(stdstringchar_count < 0)
		return std::string();

	return m_text.substr(stdstringchar_min_idx, stdstringchar_count);
}


bool Text_Line::dealWithSelection(bool it_is_for_replacement, const std::string& str, std::string* out)
{
	if(not m_is_up_to_date) //Sécurité
	{
		gs::lastError("||Warning|| in 'Text_Line::dealWithSelection()', line not up to date. [Use Text_Line::update() before.] ");
		return false;
	}
	if(not m_have_a_selection)
	{
		gs::lastError("!! in 'Text_Line::dealWithSelection()', not have a selection. ");
		return false;
	}

	//Récupération des indices des 'char' des bordures dans le 'std::string' par rapport aux positions (de curseur) des bordures de la selection.
	int max_offset = (m_selected_portion_boundaries.y >= getMaxCursorPos() ? 0 : 1);
	int stdstringchar_min_idx = getStdstringPosByVirtualCurPos(m_selected_portion_boundaries.x);
	int stdstringchar_max_idx = getStdstringPosByVirtualCurPos(m_selected_portion_boundaries.y+max_offset);

	//Sécurité:
	if(stdstringchar_max_idx > m_text.size())
	{
		stdstringchar_max_idx = m_text.size();
		gs::lastError("||Warning|| in 'Text_Line::dealWithSelection()', there was 'stdstringchar_max_idx > m_text.size()', then to fix it : 'stdstringchar_max_idx = m_text.size();' is done. ");
	}

	if(stdstringchar_min_idx < 0)
	{
		stdstringchar_min_idx = 0;
		gs::lastError("||Warning|| in 'Text_Line::dealWithSelection()', there was 'stdstringchar_min_idx < 0', then to fix it : 'stdstringchar_min_idx = 0;' is done. ");
	}

	if(stdstringchar_min_idx >= m_text.size())
	{
		if(m_text.size() >= 1)
		{
			stdstringchar_min_idx = m_text.size() -1;
			gs::lastError("||Warning|| in 'Text_Line::dealWithSelection()', there was 'stdstringchar_min_idx >= m_text.size()', then to fix it : 'stdstringchar_min_idx = m_text.size() -1;' is done. ");
		}
		else
		{
			gs::lastError("||Warning|| in 'Text_Line::dealWithSelection()', 'stdstringchar_min_idx >= m_text.size()'. ['stdstringchar_min_idx==", std::to_string(stdstringchar_min_idx)+"', 'm_text.size()=="+std::to_string(m_text.size())+"'] ");
			return false;
		}
	}

	if(not (stdstringchar_min_idx <= stdstringchar_max_idx))
	{
		gs::lastError("||Warning|| in 'Text_Line::dealWithSelection()', 'not (stdstringchar_min_idx <= stdstringchar_max_idx)'. ['stdstringchar_min_idx==", std::to_string(stdstringchar_min_idx)+"', 'stdstringchar_max_idx=="+std::to_string(stdstringchar_max_idx)+"'] ");
		return false;
	}

	//Pour l'enregistrement de l'action.
	Selection_Act_Record act_record;
	act_record.it_is_for_replacement = it_is_for_replacement;
	act_record.begin_cursor_pos = m_virtual_cursor_pos;
	act_record.selection_bounds = m_selected_portion_boundaries;
	act_record.origin_str = m_text.substr(stdstringchar_min_idx, stdstringchar_max_idx-stdstringchar_min_idx);
	act_record.new_str = (it_is_for_replacement ? str : "");


	try{

	if(it_is_for_replacement)
	{
		bool contains_end_line_char = str.contains(m_end_line_char);

		if(contains_end_line_char) //Si, dans le texte qui va supplenter le texte sélectionné, il y'a un caractère de fin de ligne, alors non seulement la partie sélectionnée sera remplacée, toute la partie qui se trouve après cette dernière (jusqu'à la fin de la ligne) sera rejetée.
		{
			act_record.origin_str = m_text.substr(stdstringchar_min_idx); //Alors on prend en compte dans l'enregistrement toute la partie qui ne sera plus là.
			act_record.new_str = str.substr(0, str.find(m_end_line_char)+1); //De plus, dans le texte qui va remplacer, seule la portion de son début au premier caractère de fin de ligne (caractère de fin de ligne inclus (voir 'Text_Line::setText())) sera prise.
		}

		m_text.replace(m_text.begin()+stdstringchar_min_idx, m_text.begin()+stdstringchar_max_idx, str);

		if(contains_end_line_char)
		{
			//S'il y avait un caractère de fin de ligne, on finalise l'opération avec un filtrage (au travers de 'setText()').
			if(out)
				*out = setText(m_text); //Tout en récupérant la partie rejetée si demandée.
			else
				setText(m_text);
		}
		else
			if(out) *out = std::string();
	}
	else
		m_text.erase(m_text.begin()+stdstringchar_min_idx, m_text.begin()+stdstringchar_max_idx);


	}catch(std::exception& err)
	{
		gs::lastError(std::string("!! excep in 'Text_Line::dealWithSelection()' : ")+err.what());
		return false;
	}

	if(it_is_for_replacement)
		m_virtual_cursor_pos = m_selected_portion_boundaries.x + getUTF8CharsCount(str); //Faire ceci est utile.
	else
		m_virtual_cursor_pos = m_selected_portion_boundaries.x; //Faire ceci est utile.

	act_record.end_cursor_pos = m_virtual_cursor_pos;
	m_last_recorded_selection_act = act_record;

	m_is_up_to_date = false;

	unselectTextPortion();

	return true;
}



bool Text_Line::eraseSelection()
{
	if(not dealWithSelection(false))
	{
		gs::lastError(std::string("!! from 'Text_Line::eraseSelection()' : ")+gs::lastError());
		return false;
	}

	return true;
}


bool Text_Line::replaceSelection(const std::string& str_to_replace_with, std::string* out)
{
	if(not dealWithSelection(true, str_to_replace_with, out))
	{
		gs::lastError(std::string("!! from 'Text_Line::replaceSelection()' : ")+gs::lastError());
		return false;
	}

	//Si l'ajout est reussi, on vérifie s'il y a eu caractère de fin de ligne dans cet ajout.
	//if(str_to_replace_with.contains(m_end_line_char))
	//{
		//Utilisation de `setText()' afin de (re)filtrer la chaîne de caractère.
	//	std::string returned_part = setText(m_text);

		//if(out)
		//	*out = returned_part;

		//Et on metionne cette partie dans l'enregistrement du texte remplacé.
		//m_last_recorded_selection_act.str += returned_part;
	//}
	//else
	//	if(out) *out = std::string();

	return true;
}


Selection_Act_Record Text_Line::getLastRecordedSelectionAct() const
{
	return m_last_recorded_selection_act;
}



bool Text_Line::replace(int left_cursor_pos, int right_cursor_pos, const std::string& str, std::string* out)
{
	if(not m_is_up_to_date)
	{
		gs::lastError("||Warning|| in 'Text_Line::replace(int, int, const str&, str*)', line not up to date. [Use Text_Line::update() before.] ");
		return false;
	}

	if(not selectTextPortionByCursorsPos(left_cursor_pos, right_cursor_pos))
	{
		gs::lastError("!! in 'Text_Line::replace(int, int, const str&, str*)', selection failed, so replacement process stopped. [given args: 'left_cursor_pos==", std::to_string(left_cursor_pos)+"', 'right_cursor_pos=="+std::to_string(right_cursor_pos)+"'] ");
		return false;
	}

	if(not replaceSelection(str, out))
	{
		gs::lastError(std::string("!! from 'Text_Line::replace(int, int, const str&, str*)' : ")+gs::lastError());
		return false;
	}

	return true;
}




std::string Text_Line::getTextBeginBlank() const
{
	//Renvoie les premiers caractères "blancs" de la ligne (les ' ' et '\t' du début de ligne jusqu'au premier caractère non ' ' ou '\t' [ce premier caractère non ' ' ou '\t' n'est pas pris]).

	if(m_text.size() == 0)
		return std::string();

	int last_blank_idx=-1; //Indice du dernier caractère "blanc" du début de ligne. On commence à -1 car il se peut que le texte ne commence pas par des caractères "blancs".

	for(const char& ch : m_text) //Note: ici, il n'y a pas tellement besoin de s'occuper de la taille en octet des différents caractères UTF8.
	{
		//On incrémente 'last_blank_idx' tant qu'on a des caractères "blancs".
		//Note: ce que j'appelle caractères "blancs" ce sont le caractère espace et le caractère tabulation (' ' et '\t').

		if(ch != ' ' and ch != '\t')
			break;

		last_blank_idx++; //A cette ligne, le caractère 'ch' est "blanc", et 'last_blank_idx' récupère son indice.
	}

	return m_text.substr(0, last_blank_idx+1);
}


} //namespace gs
















//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//
//==========================================================================//









namespace gs
{

void Blocks_Folder::mapBlocksRanges()
{
	m_blocks_ranges.clear();

	for(const Blocks_By_Depth& some_depth_lvl : m_depths)
	for(const Code_Block& some_block : some_depth_lvl.blocks)
	{
		Block_Range block_range = {some_block.header_line_index+1,
									some_block.header_line_index+some_block.lines_count,
									some_block.is_folded_block};

		//Insertion des plages par ordre croissant des indices des premières lignes des plages.
		bool inserted = false;
		for(int i=0; i<m_blocks_ranges.size(); ++i)
		{
			Block_Range& range = m_blocks_ranges.at(i);

			if(block_range.min < range.min)
			{
				m_blocks_ranges.insert(m_blocks_ranges.begin()+i, block_range);
				inserted = true;
				break;
			}
		}

		if(not inserted)
			m_blocks_ranges.push_back(block_range);
	}


	m_need_to_map_blocks_ranges = false;
}


bool Blocks_Folder::isLineBlockHeader(int line_index, bool* out__is_folded_block)
{
	if(line_index < 0)
	{
		if(out__is_folded_block) *out__is_folded_block = false;
		return false;
	}

	if(m_need_to_map_blocks_ranges)
		mapBlocksRanges();

	for(Block_Range range : m_blocks_ranges)
	{
		if(range.min-1 == line_index)
		{
			if(out__is_folded_block) *out__is_folded_block = range.is_folded_block;
			return true;
		}

		if(line_index < range.min) //Utilisation de l'ordre croissant.
			break;
	}

	if(out__is_folded_block) *out__is_folded_block = false;
	return false;
}


bool Blocks_Folder::isLineHidden(int line_index)
{
	if(line_index < 0)
		return false;

	if(m_need_to_map_blocks_ranges)
		mapBlocksRanges();

	for(Block_Range range : m_blocks_ranges)
	{
		if(range.is_folded_block and range.min <= line_index and line_index <= range.max)
			return true;

		if(line_index < range.min) //Utilisation de l'ordre croissant.
			break;
	}

	return false;
}


int Blocks_Folder::getHiddenLinesCount(int from_begin_to__ln_idx)
{
	if(m_need_to_map_blocks_ranges)
		mapBlocksRanges();

	int hidden_lines_count = 0;

	//Le but est de déterminer de manière linéaire les plages repliées, afin de compter les lignes repliées.

	std::vector<Block_Range> folded_ranges; //Dans l'ordre croissant des '.min'.

	for(Block_Range range : m_blocks_ranges)
	{
		if(not range.is_folded_block)
			continue;

		bool is_in = false;
		for(int i=0; i<folded_ranges.size(); ++i) //Parcours des plages déjà enregistrées pour déterminer si la plage courant y est déjà incluse ou non.
		{
			Block_Range already_in__rg = folded_ranges.at(i);

			//Note: les différentes plages n'ont pas d'inclusions partielles, seulement des inclusions totales (AnB==A ou AnB==B ou AnB==0).

			if(already_in__rg.min <= range.min and range.max <= already_in__rg.max) //Si la plage courante ('range') est incluse dans cette plage ('already_in__rg').
			{
				is_in = true;
				break;
			}

			if(range.min <= already_in__rg.min) //Sinon si 'range' commence avant 'already_in__rg', on le place avant celle-ci, et puisque les ajouts dans 'folded_ranges' se font de manière progessive, par récurrence l'ordre croissant est conservé.
			{
				folded_ranges.insert(folded_ranges.begin()+i, range);
				is_in = true;
				break;
			}
		}

		if(not is_in)
			folded_ranges.push_back(range); //Si non déjà présente et non avant toutes celles déjà présentes, on la rajoute à la fin.

	}

	//A ce stade on a les plages des blocs repliés, de manière "linéaire" (= sans lignes comptées plusieurs fois) et par ordre croissant.
	//Maintenant on compte les nombre totale de lignes repliées que ça donne.
	for(Block_Range rg : folded_ranges)
	{
		if(from_begin_to__ln_idx >= 0 and from_begin_to__ln_idx <= rg.min) //Si la plage n'est pas avant la ligne max, on stop ici.
			break;

		if(from_begin_to__ln_idx >= 0 and from_begin_to__ln_idx <= rg.max) //Si la ligne max est dans la plage, on ne prend que les lignes avant la ligne max, et stop ici.
		{
			hidden_lines_count += from_begin_to__ln_idx - rg.min;
			break;
		}
		//sinon
		hidden_lines_count += rg.max-rg.min +1;
	}

	return hidden_lines_count;
}


int Blocks_Folder::getLineDepth(const std::string& line_text, char end_line_char, int tab_size)
{
	if(line_text.empty()) return Blocks_Folder::INVALID_DEPTH;

	int depth=0;
	bool have_text=false;

	//On parcours le texte depuis le début, afin de compter les espaces (et tabulations).
	for(int j=0; j<line_text.length(); ++j)
	{
		char curr_ch = line_text.at(j);

		if(curr_ch == ' ') ++depth;
		else
		if(curr_ch == '\t') depth+=tab_size;
		else
		{
			if(curr_ch != end_line_char)
				have_text = true;

			break; //On s'arrête dès que l'on trouve autre chose qu'un espace ou une tabulation.
		}
	}

	if(not have_text)
		return Blocks_Folder::INVALID_DEPTH;

	return depth;
};


bool Blocks_Folder::_isLineBlockHeader(int line_index, Blocks_Folder::Code_Block* out) const
{
	if(line_index < 0)
		return false;


	for(const Blocks_By_Depth& some_depth_lvl : m_depths)
	{
		for(const Code_Block& some_block : some_depth_lvl.blocks)
		{
			if(some_block.header_line_index == line_index)
			{
				if(out)
					*out = some_block;

				return true;
			}
		}
	}


	return false;
}


void Blocks_Folder::processLinesIndentsMapping(const std::vector<Text_Line>& lines, int tab_size)
{
	//Mappe les profondeurs des différentes lignes.
	//Pour simplifier, les tabulations sont compté en espaces.

	m_lines_depth.clear();

	for(const Text_Line& line : lines)
		m_lines_depth.push_back(getLineDepth(line.getText(), line.getEndLineChar(), tab_size)); //Note: la profondeur ici c'est la quantité de "vide" avant le "vrai" texte. Plus il y a d'espace au début de la ligne, plus la profondeur est grande.

	m_blocks_folder_is_up_to_date = false;

}



void Blocks_Folder::updateByIndents()
{
	//Recherche les différents blocs dans le texte.

	//Les blocs de code vont être déterminés par l'indentation/espaces.

	if(m_blocks_folder_is_up_to_date)
		return;


	std::vector<Code_Block> blocks; //On va d'abord déposer tous les blocs dans un même conteneur, avant de les ranger par profondeur d'en-tête.

	if(m_lines_depth.empty())
		return;


	std::stack<Code_Block> blocks_stack;


	int prev_valid_depth = Blocks_Folder::INVALID_DEPTH;
	int prev_valid_ln_idx = 0;


	for(int i=0; i<m_lines_depth.size(); ++i)
	{
		//On commence par déterminer la profondeur de la ligne courante, en espaces.
		int depth = m_lines_depth.at(i); //Note: la profondeur ici c'est la quantité de "vide" avant le "vrai" texte. Plus il y a d'espace au début de la ligne, plus la profondeur est grande.


		if(depth == Blocks_Folder::INVALID_DEPTH)
			continue;


		if(prev_valid_depth == Blocks_Folder::INVALID_DEPTH or i == 0)
		{
			prev_valid_depth = depth;
			prev_valid_ln_idx = i;
			continue;
		}



		if(prev_valid_depth < depth)
		{
			Code_Block new_block;
			new_block.header_line_index = prev_valid_ln_idx;
			new_block.header_line_depth = prev_valid_depth;

			blocks_stack.push(new_block);

			prev_valid_depth = depth;
			prev_valid_ln_idx = i;
			continue;
		}



		if(not blocks_stack.empty())
		{
			int curr_block__header_depth = blocks_stack.top().header_line_depth;

			while(depth <= curr_block__header_depth) //J'utilise une boucle au lieu d'un unique 'if' car une ligne peut être la limite de plus d'un bloc. Ainsi, ça permet de traiter les blocs qui y sont limités.
			{

				Code_Block& completed_block = blocks_stack.top();
				//completed_block.lines_count = i - completed_block.header_line_index -1; // '-1' car on ne compte pas la ligne d'en-tête.
				completed_block.lines_count = prev_valid_ln_idx - completed_block.header_line_index;

				blocks.push_back(completed_block);
				blocks_stack.pop();

				if(blocks_stack.empty())
					break;

				curr_block__header_depth = blocks_stack.top().header_line_depth;
			}


		}

		prev_valid_depth = depth;
		prev_valid_ln_idx = i;
	}


	if(not blocks_stack.empty())
	{
		Code_Block& last_completed_block = blocks_stack.top();

		last_completed_block.lines_count = prev_valid_ln_idx - last_completed_block.header_line_index -1; //static_cast<int>(m_lines_depth.size()) - last_completed_block.header_line_index -1;

		if(last_completed_block.lines_count) //Sécurité
			blocks.push_back(last_completed_block);

		blocks_stack.pop();
	}


	m_depths.clear();

	for(Code_Block& block : blocks)
		insertCodeBlock(block);


	m_blocks_folder_is_up_to_date = true;
}




void Blocks_Folder::insertCodeBlock(const Code_Block& block)
{
	//Et les niveaus de profondeur, et les blocs dans chaque niveau, sont classés par ordre croissant. Les niveaux par profondeur, les blocs par indice de ligne d'en-tête.

	bool put = false;
	for(int i = 0; i<m_depths.size(); ++i) //Parcours des différentes profondeurs enregistrées.
	{
		if(block.header_line_depth == m_depths.at(i).header_depth) //On range le bloc à la profondeur correspodante.
		{
			for(int j=0; j<m_depths.at(i).blocks.size(); ++j) //Insertion au bon endroit, afin de respecter l'ordre croissant.
			{
				if(block.header_line_index < m_depths.at(i).blocks.at(j).header_line_index)
				{
					m_depths.at(i).blocks.insert(m_depths.at(i).blocks.begin()+j, block);
					put = true;
					break;
				}
				else
				if(block.header_line_index == m_depths.at(i).blocks.at(j).header_line_index) //Màj si déjà présent.
				{
					m_depths.at(i).blocks.at(j) = block;
					put = true;
					break;
				}
			}

			if(not put)
			{
				m_depths.at(i).blocks.push_back(block);
				put = true;
			}
		}
		else
		if(block.header_line_depth < m_depths.at(i).header_depth) //Sinon, pour garder les profondeurs dans l'ordre croissant, on insert ici.
		{
			m_depths.insert(m_depths.begin()+i, Blocks_By_Depth());

			m_depths.at(i).header_depth = block.header_line_depth;
			m_depths.at(i).blocks.push_back(block);
			put = true;
		}

		if(put)
			break;
	}

	if(not put)
	{
		m_depths.emplace_back();
		m_depths.back().header_depth = block.header_line_depth;
		m_depths.back().blocks.push_back(block);
	}

	m_need_to_map_blocks_ranges = true;
}




void Blocks_Folder::takeEdit(te::Text_Win_Action_Type processed_edit, int line_index, const Text_Line* line_ptr, int tab_size)
{
	try{

	auto removeItsBlock = [&](int hd_line_index, int depth){
		for(Blocks_By_Depth& some_depth_lvl : m_depths)
		{
			if(some_depth_lvl.header_depth != depth)
				continue;

			int found = -1;
			for(int i=0; i<some_depth_lvl.blocks.size(); ++i)
				if(some_depth_lvl.blocks.at(i).header_line_index == hd_line_index)
				{
					found = i;
					break;
				}

			if(found >= 0 and found < some_depth_lvl.blocks.size())
				some_depth_lvl.blocks.erase(some_depth_lvl.blocks.begin()+found);
		}
	};


	auto insertItsBlock = [&](int hd_line_index, int now_line_depth){
		//if(_isLineBlockHeader(hd_line_index, nullptr)) pas nécessaire (car déjà traité dans 'insertCodeBlock()'), de plus ça peut pertureber les choses.
		//{
			//debug
			//std::cout << "!! Blocks_Folder::takeEdit::insertItsBlock(), already ["<<hd_line_index+1 <<"]\n";
			//return false;
		//}

		if(now_line_depth <= Blocks_Folder::INVALID_DEPTH)
		{
			//gs::lastError("!! Blocks_Folder::takeEdit::insertItsBlock(), line depth invalid. [block line num : ", std::to_string(hd_line_index+1)+", depth : "+std::to_string(now_line_depth)+"]");
			//debug
			std::cout << "!! Blocks_Folder::takeEdit::insertItsBlock(), line depth invalid. [block line num : "<<hd_line_index+1 <<", depth : " <<now_line_depth <<"]\n";
			return false;
		}

		Code_Block its_block;
		its_block.header_line_index = hd_line_index;
		its_block.header_line_depth = now_line_depth;
		int new_lines_count=0;
		for(int i=hd_line_index+1; i<m_lines_depth.size(); ++i) //On cherche la ligne qui limite ce bloc.
		{
			//Lorsque la ligne d'en-tête est moins profonde, le bloc englobe peut-être plus de lignes,
			//et lorsqu'elle est plus profonde, le bloc englobe peut-être moins de lignes, d'où l'intérêt de (re)compter les lignes (nouvellement) englobées dans le bloc.

			if(m_lines_depth.at(i) != Blocks_Folder::INVALID_DEPTH)
				if(m_lines_depth.at(i) <= its_block.header_line_depth)
					break;

			++new_lines_count;
		}

		//Retrait des potentielles dernières lignes "vides" du bloc.
		try{
		for(; hd_line_index+new_lines_count > hd_line_index; --new_lines_count)
		{
			if(m_lines_depth.at(hd_line_index+new_lines_count) != Blocks_Folder::INVALID_DEPTH)
				break;
		}//debug
		}catch(std::exception& excep){std::cout << "Excep 1 _ 'Blocks_Folder::takeEdit()' :"<<excep.what() << "\n";}


		if(new_lines_count > 0) //En effet, il se peut que le bloc concerné disparaisse, si l'en-tête a gagné en profondeur (est plus à droite qu'avant).
		{
			its_block.lines_count = new_lines_count;
			insertCodeBlock(its_block);

			return true;
		}
		//else
		//if(_isLineBlockHeader(hd_line_index, nullptr))



		return false;
	};



	auto insertSomeBlock = [&](int hd_line_index, int now_line_depth){
		if(now_line_depth <= Blocks_Folder::INVALID_DEPTH)
		{
			//gs::lastError("!! Blocks_Folder::takeEdit::insertSomeBlock(), line depth invalid. [block line num : ", std::to_string(hd_line_index+1)+", depth : "+std::to_string(now_line_depth)+"]");
			//debug
			std::cout << "!! Blocks_Folder::takeEdit::insertSomeBlock(), line depth invalid. [block line num : "<<hd_line_index+1 <<", depth : " <<now_line_depth <<"]\n";
			return false;
		}

		return insertItsBlock(hd_line_index, now_line_depth);
	};



	auto isThisLineAWallForThisBlock = [&](int line_idx, Code_Block& some_block){
		bool is_a_wall_for_this_block = false;

		if(some_block.header_line_index+some_block.lines_count+1 == line_index) //Si cette ligne y est tout juste la frontière.
			is_a_wall_for_this_block = true;
		else
		{
			//Sinon on vérifie si ce n'est pas une frontière reculée (ou avancée (ça dépend de comment on le voit ^^)) par de dernières lignes "vides".

			gs::Vec2i the_between_area; //La zone entre cette ligne et la dernière ligne incluse dans ce bloc.
			the_between_area.x = some_block.header_line_index+some_block.lines_count+1;
			the_between_area.y = line_index-1; //les deux sont des indices de lignes incluses dans cette zone (x--> min (inclus), y--> max (inclus)).

			bool all_area_is_invalid = false;

			if(the_between_area.x <= the_between_area.y and the_between_area.x >= 0 and the_between_area.y < m_lines_depth.size())
			for(int k=the_between_area.x; k<=the_between_area.y; ++k)
			{
				if(m_lines_depth.at(k) != Blocks_Folder::INVALID_DEPTH)
				{
					all_area_is_invalid = false;
					break;
				}
				else
					all_area_is_invalid = true;
			}

			is_a_wall_for_this_block = all_area_is_invalid;
		}

		return is_a_wall_for_this_block;
	};


	auto processSurrendedBlocksUpdate = [&](int prev_depth, int now_line_depth, std::vector<gs::Vec2i>& vanished_blocks, std::vector<gs::Vec2i>& can_be_extended_blocks){
		for(int i=0; i<m_depths.size(); ++i)
		{
			Blocks_By_Depth& some_depth_lvl = m_depths.at(i);

			for(int j=0; j<some_depth_lvl.blocks.size(); ++j)
			{
				Code_Block& some_block = some_depth_lvl.blocks.at(j);

				//Note: les blocs dans un niveau sont classés par ordre croissant des indices de ligne d'en-tête.
				if(some_block.header_line_index >= line_index) // En ce qui concerne la comparaison '>=' : '>' car ce bloc ne peut être influencé par cette ligne (puisque ce bloc est plus bas), et '=' car si c'est une ligne d'en-tête c'est normalement déjà traité plus haut (voir 'its_block').
					break;

				if(line_index <=  some_block.header_line_index + some_block.lines_count) //Si la ligne en question est contenue dans ce bloc.
				{
					if(now_line_depth <= some_block.header_line_depth)//Si la ligne est désormais moins profonde ou au même niveau que l'en-tête de ce bloc, alors ce bloc se voit limiter par cette ligne.
					{
						int new_lines_count = some_block.lines_count;

						if(now_line_depth != Blocks_Folder::INVALID_DEPTH) //On vérifie bien que la ligne est "valide"
						{
							//On limite ce bloc à cette ligne, sans oublier d'exclure les potentielles lignes "vides" qu'il y aurait entre cette ligne et la dernière ligne à prendre en compte dans ce bloc.
							new_lines_count = line_index - some_block.header_line_index -1; // '-1' car la ligne d'en-tête n'est pas comptée.
						}
						//sinon si elle n'est plus "valide", on se contente seulement d'enlever les potentielles dernières lignes "vides", car il se peut que cette ligne était la dernière incluse dans ce bloc, et qu'en devenant ainsi "invalide"(aka "vide"), les lignes "vides" contiguës au dessus d'elle ne sont plus à comptabiliser dans le bloc.


						//Retrait des potentielles dernières lignes "vides" du bloc.
						try{
						for(; some_block.header_line_index+new_lines_count > some_block.header_line_index; --new_lines_count)
						{
							if(m_lines_depth.at(some_block.header_line_index+new_lines_count) != Blocks_Folder::INVALID_DEPTH)
								break;
						}//debug
						}catch(std::exception& excep){std::cout << "Excep 2 _ 'Blocks_Folder::takeEdit()' :"<<excep.what() << "\n";}


						if(new_lines_count > 0)
							some_block.lines_count = new_lines_count;

						else
							vanished_blocks.push_back({i,j}); //Sinon c'est que le bloc a dû disparaître.

					}

					//Sinon si elle est plus profonde, ça ne change rien pour le bloc qui le contient (si je ne m'abuse).

				}
				else if(prev_depth <= some_block.header_line_depth and (some_block.header_line_depth < now_line_depth or now_line_depth == Blocks_Folder::INVALID_DEPTH)) //Si avant, la ligne était moins (ou autant) profonde que l'en-tête de ce bloc, et que maintenant, cette ligne est plus profonde que l'en-tête de ce bloc (ou "invalide"/"vide"), alors peut-être que cette ligne lui était une frontière, et si c'est le cas elle ne l'est plus et le laisse "passer"/s'agrandir.
				{
					bool was_a_wall_for_this_block = isThisLineAWallForThisBlock(line_index, some_block);

					if(was_a_wall_for_this_block)
						can_be_extended_blocks.push_back({i,j});
				}


			}
		}

	};


	auto extendExtedableBlocks = [&](std::vector<gs::Vec2i>& vanished_blocks, std::vector<gs::Vec2i>& can_be_extended_blocks){
		int last_valid_line_idx = Blocks_Folder::INVALID_DEPTH;
		for(int i=line_index+1; i<m_lines_depth.size() and not can_be_extended_blocks.empty(); ++i)
		{
			if(m_lines_depth.at(i) != Blocks_Folder::INVALID_DEPTH)
			for(int j=can_be_extended_blocks.size()-1; j>=0; --j) //Parcours à rebours pour permettre la suppression dans 'can_be_extended_blocks' en direct.
			{
				gs::Vec2i location = can_be_extended_blocks.at(j);
				Code_Block& extendable_block = m_depths.at(location.x).blocks.at(location.y);


				//Là le but est de retirer de 'can_be_extended_blocks' les blocs dont la progression se stoppe ici, en méttant ainsi à jour leur compte de lignes (notamment en utilisant 'last_valid_line_idx').

				if(m_lines_depth.at(i) <= extendable_block.header_line_depth)
				{
					int last_line_idx = (last_valid_line_idx == Blocks_Folder::INVALID_DEPTH ? i-1 : last_valid_line_idx);

					extendable_block.lines_count = last_line_idx - extendable_block.header_line_index; //Note: la ligne d'en-tête n'est pas comptabilisée dans '.lines_count'.


						//Retrait des potentielles dernières lignes "vides" du bloc.
						try{
						int hd_line_index = extendable_block.header_line_index;
						int new_lines_count = extendable_block.lines_count;

						for(; hd_line_index+new_lines_count > hd_line_index; --new_lines_count)
						{
							if(m_lines_depth.at(hd_line_index+new_lines_count) != Blocks_Folder::INVALID_DEPTH)
								break;
						}

						extendable_block.lines_count = new_lines_count;

						//debug
						}catch(std::exception& excep){std::cout << "Excep A _ 'Blocks_Folder::takeEdit()' :"<<excep.what() << "\n";}



					if(extendable_block.lines_count <= 0) //Si une erreur quelconque s'est produite.
						vanished_blocks.push_back(location); //On retire le bloc par sécurité.

					can_be_extended_blocks.erase(can_be_extended_blocks.begin()+j);

				}
			}

			if(m_lines_depth.at(i) != Blocks_Folder::INVALID_DEPTH)
				last_valid_line_idx = i;
		}

	};


	auto removeVanishedBlocks = [&](std::vector<gs::Vec2i>& vanished_blocks){
		std::sort(vanished_blocks.begin(), vanished_blocks.end(), [](gs::Vec2i& a, gs::Vec2i& b){

			if(a.x < b.x) return true; //On classe aussi par niveau de profondeur (pour avoir les différents niveaux de profondeurs de manière contiguë).

			if(a.x == b.x) return a.y > b.y; //Et les blocs dans chaque niveau sont classés dans l'ordre décroissant pour faciliter la suppression.

			return false;
		});

		for(gs::Vec2i location : vanished_blocks)
		{
			Blocks_By_Depth& some_depth_lvl = m_depths.at(location.x);
			some_depth_lvl.blocks.erase(some_depth_lvl.blocks.begin()+location.y);
		}
	};


	auto beforeExit = [&](){if(isLineHidden(line_index)) unhideLine(line_index);};

	//-----------------------

	if(line_index < 0)
		return;


	//debug
	if(0)
	switch(processed_edit)
	{
		case te::INSERT_CHAR: std::cout << "[[edit: INSERT_CHAR_"<<line_index+1<<"\n"; break;
		case te::REMOVE_CHAR: std::cout << "[[edit: REMOVE_CHAR_"<<line_index+1<<"\n"; break;
		case te::ERASE_LINE_SELECTION: std::cout << "[[edit: ERASE_LINE_SELECTION_"<<line_index+1<<"\n"; break;
		case te::REPLACE_LINE_SELECTION: std::cout << "[[edit: REPLACE_LINE_SELECTION_"<<line_index+1<<"\n"; break;
		case te::NEW_LINE: std::cout << "[[edit: NEW_LINE_"<<line_index+1<<"\n"; break;
		case te::REMOVE_LINE: std::cout << "[[edit: REMOVE_LINE_"<<line_index+1<<"\n"; break;
		default: std::cout << "[[OTHER_EDIT_"<<line_index+1<<"\n"; break;
	}


	if(processed_edit == te::INSERT_CHAR          or processed_edit == te::REMOVE_CHAR
	or processed_edit == te::ERASE_LINE_SELECTION or processed_edit == te::REPLACE_LINE_SELECTION
	or processed_edit == te::LINE_NEW_TEXT ) //--> possible modifaction de la profondeur de la ligne.
	{
		if(line_index >= m_lines_depth.size() or not line_ptr)
			return;

		int prev_depth = m_lines_depth.at(line_index);
		int now_line_depth = getLineDepth(line_ptr->getText(), line_ptr->getEndLineChar(), tab_size);
		if(now_line_depth == prev_depth)
			return beforeExit();

		 m_lines_depth.at(line_index) = now_line_depth; //Mise à jour.


		//Plusieurs cas de figures se présentent au niveau de l'impact sur la structure des blocs.


		bool is_line_block_header = _isLineBlockHeader(line_index, nullptr);
		if(is_line_block_header) //Le cas de la mis à jour de la profondeur d'une ligne d'en-tête.
		{
			removeItsBlock(line_index, prev_depth); //On commence par retirer ce bloc.
		}


		if(is_line_block_header or (now_line_depth < prev_depth and now_line_depth != Blocks_Folder::INVALID_DEPTH)) //Si c'est une ligne d'en-tête, c'est la poursuite du traitement, sinon si la ligne est moins profonde, c'est peut-être l'apparition d'un nouveau bloc.
		{
			//Puis on le met à jour, et le rajoute (s'il y a au moins une ligne autre que l'en-tête).
			insertItsBlock(line_index, now_line_depth);

			//Le bloc est mis à jour, ok.
		}


		if(prev_depth < now_line_depth)
		{
			//Si la ligne est plus profonde, alors peut-être qu'elle instancie un nouveau bloc à partir de la première ligne "valide" qui se trouve avant elle.

			for(int i=line_index-1; i>=0; --i) //Recherche de la première ligne "valide" qui précède.
			{
				if(m_lines_depth.at(i) != Blocks_Folder::INVALID_DEPTH)
				{
					insertSomeBlock(i, m_lines_depth.at(i));
					break;
				}
			}
		}



		//Cette ligne mise à jour a peut-être réduit un ou plusieurs blocs au dessus qui le contenaient, et ouvre peut-être le passage à d'autres blocs dont elle servait de frontière.
		//De ce fait on va chercher les blocs qui : commencent avant elle et la contiennent, et les blocs qui commencent avant elle en étant limités par elle.


		std::vector<gs::Vec2i> vanished_blocks; //Pour les blocs qui disparaitraient à la suite de la mise à jour de cette ligne.
		std::vector<gs::Vec2i> can_be_extended_blocks; //Pour les blocs qui ne sont plus limités par cette ligne.

		processSurrendedBlocksUpdate(prev_depth, now_line_depth, vanished_blocks, can_be_extended_blocks);



		//Mise à jour des blocs qui en ont besoins.
		try{

		//On commence par étendre les blocs qui le peuvent maintenant.
		extendExtedableBlocks(vanished_blocks, can_be_extended_blocks);

		}                           //debug
		catch(std::exception& excep){std::cout << "Excep 3 _ 'Blocks_Folder::takeEdit()' :"<<excep.what() << "\n";}


		//Suppresion des blocs qui ne sont plus.
		try{

		removeVanishedBlocks(vanished_blocks);

		}                           //debug
		catch(std::exception& excep){std::cout << "Excep 4 _ 'Blocks_Folder::takeEdit()' :"<<excep.what() << "\n";}



		m_need_to_map_blocks_ranges = true;
	}
	else
	if(processed_edit == te::NEW_LINE)
	{
		if(line_index > m_lines_depth.size() or not line_ptr)
			return;

		int line_depth = getLineDepth(line_ptr->getText(), line_ptr->getEndLineChar(), tab_size);

		int prev_depth = Blocks_Folder::INVALID_DEPTH;
		if(line_index < m_lines_depth.size())
			prev_depth = m_lines_depth.at(line_index);


		m_lines_depth.insert(m_lines_depth.begin()+line_index, line_depth);

		//Décalage des blocs en conséquence.
		for(Blocks_By_Depth& some_depth_lvl : m_depths)
		for(Code_Block& some_block : some_depth_lvl.blocks)
		{
			if(some_block.header_line_index >= line_index)
				some_block.header_line_index++;

			//Et au passage on augmente de 1 la quantité de lignes incluses dans les blocs contenant cette insertion.
			if(some_block.header_line_index < line_index and line_index <= some_block.header_line_index+some_block.lines_count)
				some_block.lines_count++;
		}

		//Rajout de bloc si cette ligne en ajoute un.
		if(line_depth != Blocks_Folder::INVALID_DEPTH)
		{
			insertItsBlock(line_index, line_depth);
		}

		// (Cette petite partie-ci est peut-être incomplète pour ici).
		for(int i=line_index-1; i>=0; --i) //Recherche de la première ligne "valide" qui précède.
			if(m_lines_depth.at(i) != Blocks_Folder::INVALID_DEPTH)
			{
				bool res = insertSomeBlock(i, m_lines_depth.at(i)); //Tentative.
				break;
			}




		std::vector<gs::Vec2i> vanished_blocks; //Pour les blocs qui disparaitraient à la suite de la mise à jour.
		std::vector<gs::Vec2i> can_be_extended_blocks; //Pour les blocs qui ne sont plus limités par cette ligne.

		processSurrendedBlocksUpdate(prev_depth, line_depth, vanished_blocks, can_be_extended_blocks);

		//Mise à jour des blocs qui en ont besoins.
		try{

		//On commence par étendre les blocs qui le peuvent maintenant.
		extendExtedableBlocks(vanished_blocks, can_be_extended_blocks);

		}                           //debug
		catch(std::exception& excep){std::cout << "Excep 5 _ 'Blocks_Folder::takeEdit()' :"<<excep.what() << "\n";}


		//Suppresion des blocs qui ne sont plus.
		try{

		removeVanishedBlocks(vanished_blocks);

		}                           //debug
		catch(std::exception& excep){std::cout << "Excep 6 _ 'Blocks_Folder::takeEdit()' :"<<excep.what() << "\n";}


		m_need_to_map_blocks_ranges = true;
	}
	else
	if(processed_edit == te::REMOVE_LINE)
	{
		if(line_index >= m_lines_depth.size())
			return;


		if(_isLineBlockHeader(line_index, nullptr))
		{
			removeItsBlock(line_index,  m_lines_depth.at(line_index));
		}


		std::vector<gs::Vec2i> vanished_blocks; //Pour les blocs qui disparaitraient (de manière confirmée) à la suite de la mise à jour.
		std::vector<gs::Vec2i> can_be_extended_blocks; //Pour les blocs qui ne sont plus limités par cette ligne.

		std::vector<gs::Vec2i> could_be_vanished_blocks; //Pour les blocs qui peuvent disparaitre (de manière non confirmée, mais plutôt à vérifier) à la suite de la mise à jour.


		//Parcours de tous les blocs, afin de déterminer quels sont ceux qui contiennent la ligne à retirer afin de réduire le compte de lignes, et quels sont ceux qui débutent après la ligne à retirer afin de les remonter.
		for(int i=0; i<m_depths.size(); ++i)
		{
			Blocks_By_Depth& some_depth_lvl = m_depths.at(i);

			for(int j=0; j<some_depth_lvl.blocks.size(); ++j)
			{
				Code_Block& some_block = some_depth_lvl.blocks.at(j);

				if(isThisLineAWallForThisBlock(line_index, some_block))
					can_be_extended_blocks.push_back({i,j});
				else
				if(some_block.header_line_index < line_index and line_index <= some_block.header_line_index+some_block.lines_count)
				{
					some_block.lines_count--;

					if(some_block.lines_count <= 0)
						vanished_blocks.push_back({i,j});
					else
					{
						//On vérifie si ce bloc est toujours valide (aka contient au moins une ligne "valide"), car il se peut que cette ligne qui va être retirée était la seule "valide" de ce bloc (et que ce bloc avait des lignes "vides" comme par exemple de simples sauts de ligne).

						bool at_least_one_valid_ln_remain = false;
						for(int i2 = some_block.header_line_index+1; i2 <= (some_block.header_line_index+some_block.lines_count) and i2 >= 0 and i2 < m_lines_depth.size(); ++i2)
						{
							if(i2 == line_index)
								continue; //On ne prend donc pas en compte la ligne qui va être retirée.

							if(m_lines_depth.at(i2) != Blocks_Folder::INVALID_DEPTH)
							{
								at_least_one_valid_ln_remain = true;
								break;
							}
						}

						if(not at_least_one_valid_ln_remain)
							vanished_blocks.push_back({i,j});
					}

				}
				else
				if(line_index <= some_block.header_line_index)
					some_block.header_line_index--;

			}
		}



		m_lines_depth.erase(m_lines_depth.begin()+line_index);



		for(int i=line_index-1; i>=0; --i) //Recherche de la première ligne "valide" qui précède.
		{
			if(m_lines_depth.at(i) != Blocks_Folder::INVALID_DEPTH)
			{
				insertSomeBlock(i, m_lines_depth.at(i)); //Tentative.
				break;
			}
		}


		//Mise à jour des blocs qui en ont besoins.
		try{

		//On commence par étendre les blocs qui le peuvent maintenant.
		extendExtedableBlocks(vanished_blocks, can_be_extended_blocks);

		}                           //debug
		catch(std::exception& excep){std::cout << "Excep 7 _ 'Blocks_Folder::takeEdit()' :"<<excep.what() << "\n";}

		//Suppresion des blocs qui ne sont plus ('vanished_blocks').
		try{

		removeVanishedBlocks(vanished_blocks);

		}                           //debug
		catch(std::exception& excep){std::cout << "Excep 8 _ 'Blocks_Folder::takeEdit()' :"<<excep.what() << "\n";}


		m_need_to_map_blocks_ranges = true;
	}

	//else if(processed_edit == NEW_TEXT){} non.

	//debug
	if(0)
	{
		for(const Blocks_By_Depth& some_depth_lvl : m_depths)
		for(const Code_Block& some_block : some_depth_lvl.blocks)
		{
			std::cout << some_block.header_line_index+1 << " : "<<some_block.lines_count << "\n";
		}
		std::cout << "---------------------------------------------------------\n\n";
	}


	beforeExit();

	}
	catch(std::exception& excep)
	{
		gs::lastError("!! excep in 'Blocks_Folder::takeEdit()' : ", excep.what());
	}
}



void Blocks_Folder::setCodeBlockFoldedState(int block_header_line_idx, bool inverse_state, bool otherwise__folded_state)
{
	for(Blocks_By_Depth& some_depth_lvl : m_depths)
	{
		bool done = false;

		for(Code_Block& some_block : some_depth_lvl.blocks)
		{
			if(some_block.header_line_index == block_header_line_idx)
			{
				if(inverse_state)
					some_block.is_folded_block = !some_block.is_folded_block;
				else
					some_block.is_folded_block = otherwise__folded_state;

				done = true;
				break;
			}
		}

		if(done)
		{
			mapBlocksRanges();
			break;
		}
	}
}



float Blocks_Folder::adjustYScroll(float y_scroll, int lines_delta) const
{
	//Le but est de décaller 'y_scroll' d'autant de pixels repliés qui ont une influence sur la nouvelle position visuelle des lignes à l'écran.
	//En effet lorsqu'il y'a des blocs repliés certaines lignes se voient "tirées vers le haut" comparé à lorsque qu'il n'y a pas de bloc replié.


	int non_formated_top_ln_idx = static_cast<int>(y_scroll) / lines_delta; //La première affichée en prennant en compte le scrolling, mais sans prendre en compte les potentiels blocs pliés.
	int scroll_idx_offset = 0;
	std::vector<Vec2i> folded_ranges; //Par ordre croissant.

	for(const Blocks_By_Depth& depth_lvl : m_depths) //Note: les différents niveaux de profondeur sont classés par ordre croissant.
	for(const Code_Block& block : depth_lvl.blocks)//Note: les différents blocs sont classés par ordre croissant des indices des lignes d'en-tête.
	{
		if(not block.is_folded_block)
			continue;

		Vec2i block_range = {block.header_line_index+1, block.header_line_index+block.lines_count};

		//On insert (selon l'ordre croissant des bornes min) cette plage si elle n'est pas déjà incluse.
		bool is_ok = false;
		for(int i=0; i<folded_ranges.size(); ++i)
		{
			Vec2i already_in__rg = folded_ranges.at(i);

			if(already_in__rg.x <= block_range.x and block_range.y <= already_in__rg.y) //Si c'est inclus dans une plage déjà enregistrée, c'est bon.
			{
				is_ok = true;
				break;
			}
			else
			if(block_range.x < already_in__rg.x) //Sinon si cette plage commence avant celle qui est courament parcourue, on l'insert à sa place (par et pour l'ordre croissant).
			{
				folded_ranges.insert(folded_ranges.begin()+i, block_range);
				is_ok = true;
				break;
			}
		}

		if(not is_ok) //Sinon on la rajoute à la fin.
			folded_ranges.push_back(block_range);

	}


	//Une fois les plages des blocs pliés déterminées (par ordre croissant des bornes min), on déplace le scrolling en fonction.

	for(Vec2i folded_rg : folded_ranges) //Note: folded_rg.x --> min, folded_rg.y --> max (les deux incluses).
	{
		if(non_formated_top_ln_idx + scroll_idx_offset < folded_rg.x) //Dès la première plage qui n'influence pas le déplacement de la ligne, on arrête (parce que les plages sont dans l'ordre croissant).
			break;

		//Sinon, c'est que le bloc est avant la ligne et donc a une influence sur elle.

		scroll_idx_offset += folded_rg.y-folded_rg.x +1; //Avec ça, dans les différents tours de boucle 'non_formated_top_ln_idx + scroll_idx_offset' est déplacé au fur et à mesure, jusqu'à sa position finale.
	}

	return y_scroll + static_cast<float>(scroll_idx_offset * lines_delta);

}



int Blocks_Folder::adjustClickYPos(int t_win_relative__click_y, float y_scroll, int lines_delta) const
{
	return adjustYScroll(static_cast<float>(t_win_relative__click_y)+y_scroll, lines_delta);
}


void Blocks_Folder::unhideLine(int line_index)
{
	//Le but ici est de rendre (si pliés) ouverts tous les blocs contenant la ligne demandée.

	for(Blocks_By_Depth& some_depth_lvl : m_depths) //Note: les différents niveaux de profondeur sont classés par ordre croissant.
	for(Code_Block& some_block : some_depth_lvl.blocks)//Note: les différents blocs sont classés par ordre croissant des indices des lignes d'en-tête.
	{
		if(not some_block.is_folded_block) //Si c'est déjà ouvert, pas besoin d'ouvrir.
			continue;

		if(line_index <= some_block.header_line_index) //On profite de l'ordre croissant pour optimiser la recherche (en effet les blocs commençant après cette ligne ne peuvent la contenir ; et si cette ligne est l'en-tête de ce bloc, elle n'est pas cachée par ce même bloc, car les blocs ne cachent pas leur ligne d'en-tête (à dernière nouvelle)).
			break;

		int block_min_idx = some_block.header_line_index;
		int block_max_idx = some_block.header_line_index+some_block.lines_count; //Note: '.lines_count' ne comptabilise pas la ligne d'en-tête.

		if(block_min_idx < line_index and line_index <= block_max_idx) //Si le bloc inclut la ligne (hors ligne d'en-tête).
		{
			some_block.is_folded_block = false; //Ouverture du bloc.
			m_need_to_map_blocks_ranges = true; //Si l'on en modifie au moins un, on "fait la demande" d'une mise à jour des plages des blocs de code.
		}
	}
}


void Blocks_Folder::setAllBlocksWithSameState(bool folded_state, int target_depth)
{
	for(Blocks_By_Depth& some_depth_lvl : m_depths) //Note: les différents niveaux de profondeur sont classés par ordre croissant.
	{
		if(target_depth != Blocks_Folder::INVALID_DEPTH and some_depth_lvl.header_depth != target_depth)
			continue;

		for(Code_Block& some_block : some_depth_lvl.blocks)//Note: les différents blocs sont classés par ordre croissant des indices des lignes d'en-tête.
		{
			some_block.is_folded_block = folded_state;
			m_need_to_map_blocks_ranges = true;
		}
	}
}












bool Text_Window::initByText(Font& font, Rect pos_size, const std::string& text, char end_line_char)
{
	try{

	if(m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::initByText()', already init. ");
		return false;
	}


	if(not font.isInit())
	{
		gs::lastError("||Warning|| in 'Text_Window::initByText()', give gs::Font& is not init. ");
		return false;
	}

	m_common_bank.setFont(font, false);

	if(m_common_ptsize == -1) //Si le ptsize n'a pas encore été initialisé, on prend celui du font.
		m_common_ptsize = font.getPtsize();
	else
		font.setPtsize(m_common_ptsize);

	m_lines_delta = font.getFontLineSkip();
	m_end_line_char = end_line_char;
	m_pos.x = pos_size.x;
	m_pos.y = pos_size.y;
	m_size.x = ((pos_size.w < 0) ? 1 : pos_size.w);
	m_size.y = ((pos_size.h < 0) ? 1 : pos_size.h);

	//Distribution des lignes.
	bool is_ok = false;
	std::string rejected_part = text;

	int lines_count_limit = 1000000000; //1_000_000_000, afin d'éviter une boucle infini.
	int i = 0;

	do
	{
		if(i >= lines_count_limit)
			break;

		m_lines.emplace_back();
		m_lines.back().setEndLineChar(m_end_line_char);
		m_lines.back().clearRules(FONT_STYLE_NORMAL, m_common_ptsize, u32RGBA(m_fg_color), 0);

		rejected_part = m_lines.back().setText(rejected_part);

		m_lines.back().setBank(&m_common_bank);
		m_lines.back().setSelectionColor(m_selection_bg_color);
		m_lines.back().setMaxSize( {m_size.x-m_x_margin, static_cast<unsigned int>(m_lines_delta)} ); //~

		m_lines.back().update(m_tab_size);

		if(rejected_part.size() == 0)
			is_ok = true;

		i++;
	}
	while(not is_ok);

	pushAction(te::NEW_TEXT, -1, text);
	setMaxLineW();

	m_blocks_folder.processLinesIndentsMapping(m_lines, m_tab_size);

	m_is_init = true;
	return true;

	}
	catch(std::exception& excep)
	{
		gs::lastError(std::string("!! excep in 'Text_Window::initByText()' : ")+excep.what());
	}

	return false; //En cas d'exception.
}




bool Text_Window::initByFile(Font& font, Rect pos_size, const char* file_path, char end_line_char)
{
	if(m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::initByFile()', already init. ");
		return false;
	}

	if(not font.isInit())
	{
		gs::lastError("||Warning|| in 'Text_Window::initByFile()', give gs::Font& is not init. ");
		return false;
	}

	std::ifstream file;
	file.open(file_path, std::ios::in);

	if(not file.is_open())
	{
		gs::lastError(std::string("||Warning|| in 'Text_Window::initByFile()', cannot open file : \"")+std::string(file_path)+"\". ");
		return false;
	}

	std::string buf, all_lines;

	while(std::getline(file, buf, end_line_char))
	{
		all_lines += buf + end_line_char;
	}

	file.close();


	bool res = initByText(font, pos_size, all_lines, end_line_char);

	if(not res)
	{
		gs::lastError(std::string("from 'Text_Window::initByFile()' : ")+gs::lastError());
		return false;
	}

	return true;
}

bool Text_Window::initByFile(Font& font, Rect pos_size, const std::string& file_path, char end_line_char)
{
	return initByFile(font, pos_size, file_path.c_str(), end_line_char);
}



bool Text_Window::changeFont(Font& font, bool free_bank_prev_memory)
{
	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::openFile()', not yet init. Use Text_Window::initBy...() before. ");
		return false;
	}


	if(not font.isInit())
	{
		gs::lastError("||Warning|| in 'Text_Window::initByText()', give gs::Font& is not init. ");
		return false;
	}

	m_common_bank.setFont(font, free_bank_prev_memory); // m_common_bank.setFont() permet également de reset la banque.
	m_lines_delta = font.getFontLineSkip();

	return true;
}


void Text_Window::resetBank(bool free_memory)
{
	m_common_bank.clear(free_memory);
}



bool Text_Window::openFile(const char* file_path)
{
	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::openFile()', not yet init. Use Text_Window::initBy...() before. ");
		return false;
	}

	std::ifstream file;
	file.open(file_path, std::ios::in);

	if(not file.is_open())
	{
		gs::lastError(std::string("||Warning|| in 'Text_Window::openFile()', cannot find/open file : \"")+std::string(file_path)+"\". ");
		return false;
	}

	std::string buf, all_lines;

	while(std::getline(file, buf, m_end_line_char))
		all_lines += buf + m_end_line_char;

	file.close();


	if(not setText(all_lines))
	{
		gs::lastError(std::string("from 'Text_Window::openFile()' : ")+gs::lastError());
		return false;
	}

	return true;
}



bool Text_Window::setText(const std::string& text, bool record_this_action, Text_Win_Action* out__done_action)
{
	try{

	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::setText()', not yet init. Use Text_Window::initBy...() before. ");
		return false;
	}

	unselect();

	int prev_active_ln_idx = m_curr_active_line_idx;
	int prev_cursor_pos = -1;
	std::string prev_text = this->getText(-1);

	if(prev_active_ln_idx != te::NO_ACTIVE_LINE)
		prev_cursor_pos = m_lines.at(prev_active_ln_idx).getCursorPos();


	//Suppression des précédentes lignes.
	m_lines.clear();
	m_curr_active_line_idx = te::NO_ACTIVE_LINE;
	m_y_scroll = 0.f;
	m_x_scroll = 0.f;

	//Distribution des lignes.
	bool is_ok = false;
	std::string rejected_part = text;

	int lines_count_limit = 1000000000; //1_000_000_000, afin d'éviter une boucle infinie.
	int i = 0;

	do
	{
		if(i >= lines_count_limit)
		{
			gs::lastError("||Warning|| in 'Text_Window::setText()', anti-infinite loop condition passed! Too many lines. ");
			break;
		}

		m_lines.emplace_back();
		m_lines.back().setEndLineChar(m_end_line_char);
		m_lines.back().clearRules(FONT_STYLE_NORMAL, m_common_ptsize, u32RGBA(m_fg_color), 0);

		rejected_part = m_lines.back().setText(rejected_part);


		m_lines.back().setBank(&m_common_bank);
		m_lines.back().setSelectionColor(m_selection_bg_color);
		m_lines.back().setMaxSize( {m_size.x-m_x_margin, static_cast<unsigned int>(m_lines_delta)} ); //~

		m_lines.back().update(m_tab_size);


		if(rejected_part.size() == 0)
			is_ok = true;

		++i;
	}
	while(not is_ok);

	if(record_this_action)
		pushAction(te::NEW_TEXT, -1, text, -1, prev_cursor_pos, {-1,-1}, prev_active_ln_idx, prev_text);

	if(out__done_action) *out__done_action =
		makeAction(te::NEW_TEXT, -1, text, -1, prev_cursor_pos, {-1,-1}, prev_active_ln_idx, prev_text);

	setMaxLineW();

	m_blocks_folder.processLinesIndentsMapping(m_lines, m_tab_size);

	return is_ok;

	}
	catch(std::exception& excep)
	{
		gs::lastError(std::string("!! excep in 'Text_Window::setText()' : ")+excep.what());
	}

	return false; //En cas d'exception.
}


bool Text_Window::insertText(const std::string& str, bool when_not_have_selection__process_multilines_completion, bool record_this_action) //Au niveau du curseur de la ligne courante (si présente).
{
	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::insertText()', not yet init. Use 'Text_Window::initBy...()' before. ");
		return false;
	}


	if(haveSelection()) {
		return replaceSelection(str, record_this_action);
	}
	else
	if(m_curr_active_line_idx != te::NO_ACTIVE_LINE) //Sinon, s'il y a une ligne d'édition courante.
	{
		Text_Line& curr_line = m_lines.at(m_curr_active_line_idx);

		int prv_cursor_pos = curr_line.getCursorPos();

		std::string returned_part;
		std::string cursor_to_end_part;

		//Anticipation: si 'str' va rajouter un caractère de fin de ligne, la partie du texte qui va du curseur à la fin de la ligne sera rejetée.
		if(str.contains(curr_line.getEndLineChar()))
			cursor_to_end_part = curr_line.getTextFromCursorToEnd();

		curr_line.addText(str, &returned_part, true);

		curr_line.update(m_tab_size);

		//Enregistrement de l'action.
		if(record_this_action)
			pushAction(te::INSERT_CHAR, m_curr_active_line_idx, str, curr_line.getCursorPos(), prv_cursor_pos, {-1,-1}, m_curr_active_line_idx, cursor_to_end_part).action_idx = 0;

		m_blocks_folder.takeEdit(te::INSERT_CHAR, m_curr_active_line_idx, &curr_line, m_tab_size);


		unselect();

		if(when_not_have_selection__process_multilines_completion)
		{
			int cursor_pos_of_the_latest_compl_ln_ = getUTF8CharsCount(getLastTextChunk(str, curr_line.getEndLineChar())); //S'il y a des lignes de complétion, ceci permet de mettre le curseur au bon endroit, à la dernière ligne de complétion, au niveau de la fin du text "original" inséré.

			gs::Vec_Bool_Int res = completeTextInsertion(m_curr_active_line_idx+1, returned_part, record_this_action, 1, cursor_pos_of_the_latest_compl_ln_);

			if(record_this_action)
			{
				Text_Win_Action& last_act = pushAction(te::INFO, -1, "");
				last_act.action_idx = res.i;
				last_act.info = te::END_OF_MULTILINES_INSERT;
			}

			return res.b;
		}

		return true;
	}

	unselect();

	gs::lastError("||Warning|| in 'Text_Window::insertText()', there is no current active line. [Try to put the cursor somewhere in some line before.] ");

	return false;
}



Vec_Bool_Int Text_Window::completeTextInsertion(int start_line_idx, std::string str, bool record_this_action, int curr_action_idx, int cursor_pos_of_the_latest_compl_ln) //Utilitaire qui permet d'insérer les lignes complétant une insertion qui introduit plusieurs lignes. Cette méthode rajoute les lignes nécessaires.
{
	if(not m_is_init) //Sécurité
	{
		gs::lastError("||Warning|| in 'Text_Window::completeTextInsertion()', not yet init. ");
		return {false, curr_action_idx};
	}

	if(str.length() == 0) return {true, curr_action_idx};

	if(start_line_idx < 0) //Note: on ne vérifie pas si l'indice dépasse par la fin (soit 'start_line_idx >= m_lines.size()'), car si cela est le cas, 'addLine()' rajoute la ligne à la fin (après la dernière).
	{
		gs::lastError("!! from 'Text_Window::completeTextInsertion()', invalid arg 'start_line_idx' : [", std::to_string(start_line_idx)+"]. ");
		return {false, curr_action_idx};
	}

	std::string returned_part;
	int line_index = start_line_idx;

	while(str.length()) //Tant qu'il y a du text à insérer.
	{
		m_for_addLine__action_idx = curr_action_idx; // 'm_for_addLine__action_idx' est utilisé dans 'addLine'.

		if(not addLine(line_index, str, &returned_part, true, record_this_action)) //On insert une ligne avec le text en question.
		{
			gs::lastError(std::string("!! from 'Text_Window::completeTextInsertion()' : ")+gs::lastError());
			return {false, curr_action_idx};
		}


		if(returned_part.length() == 0) //Si cette ligne ajoutée est la dernière qui complète, on ajuste son curseur selon le paramètre 'cursor_pos_of_the_latest_compl_ln'.
			if(0 <= line_index and line_index < m_lines.size()) //Sécurité.
			{
				int prev_cursor_pos = m_lines.at(line_index).getCursorPos();
				int new_cursor_pos = m_lines.at(line_index).setCursorPos(cursor_pos_of_the_latest_compl_ln);

				if(record_this_action)
				{
					pushAction(te::NEW_CURSOR_POS, line_index, "", new_cursor_pos, prev_cursor_pos, {-1,-1}, line_index).action_idx = curr_action_idx;
					++curr_action_idx;
				}
			}

		str = returned_part; //On récupère la potentielle partie du texte qui a été rejetée (car caractère de fin ligne rencontré), et cette partie servira de texte à la ligne qui va être rajoutée par la suite, et ainsi de suite.
		returned_part.clear();
		++line_index;
		++curr_action_idx;
	}

	return {true, curr_action_idx};
}


std::string Text_Window::getText(int line_index)
{
	std::string text;

	if(line_index < 0)
	{
		for(Text_Line& line : m_lines)
			text += line.getText();
	}
	else
	if(line_index < int(m_lines.size()))
	{
		return m_lines.at(line_index).getText();
	}

	return text;
}




std::string Text_Window::getCursorChar()
{
	if(m_curr_active_line_idx == te::NO_ACTIVE_LINE)
		return std::string();

	m_lines.at(m_curr_active_line_idx).update(m_tab_size);
	return m_lines.at(m_curr_active_line_idx).getCursorChar();
}




std::string Text_Window::getLineBeginBlank(int line_index) const
{
	//C'est une interface pour la méthode 'Text_Line::getTextBeginBlank()'.

	if(line_index < 0 or line_index >= m_lines.size()) //Si l'indice de ligne donné n'est pas correct.
		return std::string();

	return m_lines.at(line_index).getTextBeginBlank();
}






bool Text_Window::show(Renderer* ren, bool highlight_selected_line, std::vector<Rendered_Line_Info>* rendered_lines_info, bool hide_the_after_cursor_part)
{
	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::show()', not yet init. Use 'Text_Window::initBy...()' before. ");
		return false;
	}

	if(not ren)
	{
		gs::lastError(str("||Warning|| in \"")+ m_title +str("\" 'Text_Window::show()', given renderer is nullptr. "));
		return false;
	}

	FRect back = {static_cast<float>(m_pos.x), static_cast<float>(m_pos.y),
						static_cast<float>(m_size.x), static_cast<float>(m_size.y)};
	gs::drawRect(ren, &back, m_bg_color, true);

	float y_scroll = m_blocks_folder.adjustYScroll(m_y_scroll, m_lines_delta); //Prise en compte des lignes pliées

	int begin_x = m_pos.x + m_x_margin; //Dans l'espace global (dans le renderer).
	int curr_y = m_pos.y - int(y_scroll)%m_lines_delta; //Dans l'espace global (dans le renderer).
	int max_y = m_pos.y + m_size.y-1; //Dans l'espace global (dans le renderer).


	textColoring();


	std::string last_error = gs::lastError();
	std::string error_lines; //Afin de présenter la ou les lignes qui ont posées problème. Toutefois le message d'erreur ne concernera que la dernière ligne avec une erreur.

	if(rendered_lines_info) //Ceci n'affecte pas l'affichage, mais permet de renvoyer des données d'affichage en dehors de la méthode.
		rendered_lines_info->clear();

	unsigned int line_max_h = static_cast<unsigned int>(m_lines_delta);

	for(int i = int(y_scroll)/m_lines_delta; i< m_lines.size(); ++i) //Parcours des lignes visibles.
	{
		if(m_blocks_folder.isLineHidden(i)) //On affiche pas les lignes repliées.
			continue;

		Text_Line& line = m_lines.at(i);

		// line.update(m_tab_size);

		line.setPos({begin_x, curr_y});
		line.setMaxSize( {m_size.x-m_x_margin, line_max_h} );

		line.update(m_tab_size);


		if(i == m_curr_active_line_idx and highlight_selected_line)
		{
			Rect line_rect={line.getPos().x, line.getPos().y,
								int(line.getSize().x), int(line.getSize().y)};

			if(line_rect.w == 0 or line_rect.h == 0)
			{
				line_rect.w = m_size.x;
				line_rect.h = m_lines_delta;
			}

			//Gestion des bordures de la représentation (un rectangle) du fond de la ligne sélectionnée.
			// ***** if((line_rect.x+line_rect.w-1) > (m_pos.x+m_size.x-1))
			//
			line_rect.w -= (line_rect.x+line_rect.w-1) - (m_pos.x+m_size.x-1);

			if((line_rect.y+line_rect.h-1) > (m_pos.y+m_size.y-1))
				line_rect.h -= (line_rect.y+line_rect.h-1) - (m_pos.y+m_size.y-1);

			gs::drawRect(ren, frect(line_rect), m_selected_line_bg_color, true);
		}

		if(i == m_curr_active_line_idx and m_cursor_is_hidden)
			line.setShowCursor(false);

		bool success = line.show(ren, int(m_x_scroll), m_tab_size, {m_pos.y, max_y}, hide_the_after_cursor_part);

		if(i == m_curr_active_line_idx and m_cursor_is_hidden)
			line.setShowCursor(true);

		if(not success)
		{
			error_lines += std::to_string(i) + "; ";
		}
		else
		if(rendered_lines_info) //Ceci n'affecte pas l'affichage, mais permet de renvoyer des données d'affichage en dehors de la méthode.
		{
			bool is_folded_block = false;
			rendered_lines_info->push_back({i+1, line.getPos().y, m_blocks_folder.isLineBlockHeader(i, &is_folded_block), is_folded_block}); //Ceci a pour but de permettre d'afficher les numéros des lignes en dehors de la classe. Ca renvoie le numéro des lignes, accompagnés de leur position y. (Note: "i+1" pour passer de "index" à "numéro".)
		}

		if(hide_the_after_cursor_part and i == m_curr_active_line_idx)  //Ce mode permet de n'afficher que ce qu'il y a avant le curseur.
			break;

		curr_y += m_lines_delta;

		if(curr_y > max_y)
			break;
	}


	if(gs::lastError() != last_error)
		gs::lastError(std::string("from \"")+ m_title +str("\" 'Text_Window::show()' [line(s) ") +error_lines+ std::string("] : ")+gs::lastError());

	if(m_reset_last_act_observer_when_show)
		clearLastActions(); //"Effacement" des prorpiétés des dernières actions (de modification) effectuées.

	return true;
}




void Text_Window::examineEvents(float dt)
{
	if(not m_is_init) return;

	int last_actions_count = m_last_actions.size(); //Sert pour 'updateMaxLineW()'.

	std::unordered_map<std::string, bool>& inputs = m_inputs.getInputsMap();

	//Le texte.
	if(m_curr_active_line_idx != te::NO_ACTIVE_LINE)
	for(auto it = inputs.begin(); it != inputs.end(); ++it)
	{
		std::string input_str = it->first;

		if(it->second == false) continue; //L'état de l'entrée.

		//if(input_str == MOUSE_IS_DOWN) continue;
		if(input_str == te::MOUSE_CLICKED) continue;
		if(input_str == te::UP_ARROW) continue;
		if(input_str == te::DOWN_ARROW) continue;
		if(input_str == te::LEFT_ARROW) continue;
		if(input_str == te::RIGHT_ARROW) continue;
		if(input_str == te::LEFT_SHIFT) continue;
		if(input_str == te::RIGHT_SHIFT) continue;




		if(input_str == te::BACK_TAB)
		{
			processBackTab(true);
		}
		else
		if((input_str == "\b" or input_str == te::DELETE) and not this->eraseSelection(true)) //Suppression. Explication, si l'on reçoi ceci : "\b", on appelle d'abord le traitement de la suppression avec sélection, s'il n y a pas de sélection en cours, 'eraseSelection()' renvoie false, ainsi on peut procéder à une suppression classique ici. Sinon s'il y a bien une sélection en cours, 'eraseSelection()' procède et renvoie true (si ça s'est déroulé avec succès, ou du moins que ça a apporté une modification), et on ne traite pas alors cette suppression ici.
		{
			unselect();

			if(input_str == te::DELETE or m_lines.at(m_curr_active_line_idx).getCursorPos() > 0) //Si, dans la ligne active, le curseur n'est pas tout au début, on procède normalement.
			{
				int prv_cursor_pos = m_lines.at(m_curr_active_line_idx).getCursorPos();

				if(not m_lines.at(m_curr_active_line_idx).isUpToDate()) //Mise à jour de la ligne si ce n'est pas le cas, pour ne pas empêcher '.removeFromCursorLeftSide()'.
					m_lines.at(m_curr_active_line_idx).update(m_tab_size);

				std::string rvmd;

				// Le bouton "effacer" (ici '\b') pour effacer depuis la côté gauche du curseur, et le bouton "supprimer" (ici DELETE) pour effacer depuis la côté droit du curseur.
				if(input_str == te::DELETE)
					rvmd = m_lines.at(m_curr_active_line_idx).removeFromCursorRightSide(1); //Relatif à la position du curseur.
				else
					rvmd = m_lines.at(m_curr_active_line_idx).removeFromCursorLeftSide(1); //Relatif à la position du curseur.


				pushAction(te::REMOVE_CHAR, m_curr_active_line_idx, rvmd,
							m_lines.at(m_curr_active_line_idx).getCursorPos(),
							prv_cursor_pos);

				m_blocks_folder.takeEdit(te::REMOVE_CHAR, m_curr_active_line_idx, &m_lines.at(m_curr_active_line_idx), m_tab_size);

					if(m_blocks_folder.isLineHidden(m_curr_active_line_idx)) //Si la ligne qui vient de subir une modification était cachée, on la rend visible (en dépliant tous les blocs la contenant).
						m_blocks_folder.unhideLine(m_curr_active_line_idx);

			}
			else //Sinon si le curseur est au tout début de la ligne courante d'édition, pour supprimer quoi que ce soit, il faut que cette ligne active ne soit pas celle tout en haut.
			if(m_curr_active_line_idx > 0) //Afin que l'on ne puisse pas supprimer la première ligne de cette façon.
			{
				int prev_active_ln_idx = m_curr_active_line_idx;
				int prev_cursor_pos = m_lines.at(prev_active_ln_idx).getCursorPos();

				std::string prev_active_ln_str;
				removeLine(prev_active_ln_idx, &prev_active_ln_str, true); //Note: ici 'action_idx == 0'.

				int now_active_ln_idx = prev_active_ln_idx-1;
				int prev__now_max_cursor_pos = m_lines.at(now_active_ln_idx).setMaxCursorPos(); // #<A>  On le récupère afin de le remettre après, car addText() met le curseur derrière le texte rajouté.

				pushAction(te::NEW_CURSOR_POS, now_active_ln_idx, "",
							prev__now_max_cursor_pos, prev_cursor_pos, {-1,-1}, prev_active_ln_idx).action_idx = 1;

				int last_action_idx = 1;


				if(prev_active_ln_str.size() > 0) //Si la ligne supprimée avait du contenu.
				{
					if(not m_lines.at(now_active_ln_idx).isUpToDate()) //Mise à jour de la ligne si ce n'est pas le cas, pour ne pas empêcher '.addText()'.
						m_lines.at(now_active_ln_idx).update(m_tab_size);

					m_lines.at(now_active_ln_idx).addText(prev_active_ln_str, nullptr, true);
					m_lines.at(now_active_ln_idx).update(m_tab_size);
					int after_addText_cur_pos = m_lines.at(now_active_ln_idx).getCursorPos();

					pushAction(te::INSERT_CHAR, now_active_ln_idx, prev_active_ln_str,
								after_addText_cur_pos,
								prev__now_max_cursor_pos).action_idx = 2; //Ici '.str_2' est ignoré (en vu d'un 'cursor_to_end_part') car le curseur de la ligne qui a fait le '.addText()' était déjà à la fin (voir la ligne avec la balise #<A>).

					m_blocks_folder.takeEdit(te::INSERT_CHAR, now_active_ln_idx, &m_lines.at(now_active_ln_idx), m_tab_size);

						if(m_blocks_folder.isLineHidden(now_active_ln_idx)) //Si la ligne qui vient de subir une modification était cachée, on la rend visible (en dépliant tous les blocs la contenant).
							m_blocks_folder.unhideLine(now_active_ln_idx);


					int cursor_pos_ = m_lines.at(now_active_ln_idx).setCursorPos(prev__now_max_cursor_pos); //On remet le curseur à la précédente position maximale (c-à-d entre le texte initial de la ligne et le texte de la ligne qui vient d'être supprimée).

					pushAction(te::NEW_CURSOR_POS, now_active_ln_idx, "",
								cursor_pos_, after_addText_cur_pos).action_idx = 3;

					last_action_idx = 3;

					//Est ce pertinent de mentionner ces deux dernières actions séparément ?...
				}


				//Sans oublier d'afficher le curseur de la nouvelle ligne d'édition,
				m_lines.at(now_active_ln_idx).setShowCursor(true);

				//et de la définir comme la ligne édition courante.
				m_curr_active_line_idx = now_active_ln_idx;


				//Enfin, enregistrement de l'action globale.
				Text_Win_Action& conclusion_act = pushAction(te::INFO, -1, "");
				conclusion_act.info = te::END_OF_REMOVE_LINE_PROCESS;
				conclusion_act.action_idx = last_action_idx+1;

			}


			//Pour potentiellement remettre la ligne sélectionnée sur le champ de vue.
			updateYScroll();
			updateXScroll();

		}
		else
		if(input_str != "\b" and not this->replaceSelection(input_str, true)) //L'ajout de caractère-s. Le principe est le même que pour la surppression : on tente d'abord de voir si une sélection est en cours pour agir en conséquence..., sinon on traite la chose ici de manière classique.
		{
			unselect();

			std::string returned_part;
			int prv_cursor_pos = m_lines.at(m_curr_active_line_idx).getCursorPos();

			std::string cursor_to_end_part; //Pour l'enregistrement de l'action. Ceci permet d'avoir l'état de la ligne du curseur à la fin, au cas où il y aurait un caractère de fin ligne dans 'input_str', ce qui causerait (généralement) un rejet d'une partie de la ligne.
			if(input_str.contains(m_lines.at(m_curr_active_line_idx).getEndLineChar()))
				cursor_to_end_part = m_lines.at(m_curr_active_line_idx).getTextFromCursorToEnd();

			if(not m_lines.at(m_curr_active_line_idx).isUpToDate()) //Mise à jour de la ligne si ce n'est pas le cas, pour ne pas empêcher '.addText()'.
				m_lines.at(m_curr_active_line_idx).update(m_tab_size);

			m_lines.at(m_curr_active_line_idx).addText(input_str, &returned_part, true); //A la position du curseur.

			pushAction(te::INSERT_CHAR, m_curr_active_line_idx, input_str,
							m_lines.at(m_curr_active_line_idx).getCursorPos(),
							prv_cursor_pos, {-1,-1}, m_curr_active_line_idx, cursor_to_end_part);

			m_blocks_folder.takeEdit(te::INSERT_CHAR, m_curr_active_line_idx, &m_lines.at(m_curr_active_line_idx), m_tab_size);

				if(m_blocks_folder.isLineHidden(m_curr_active_line_idx)) //Si la ligne qui vient de subir une modification était cachée, on la rend visible (en dépliant tous les blocs la contenant).
					m_blocks_folder.unhideLine(m_curr_active_line_idx);


			if(returned_part.size() > 0) //S'il y a une de texte qui en ressort (notamment s'il vient d'y avoir un caractère de fin de ligne,
			{
				//alors on change de ligne : on "quitte" celle qui vient d'être édité,
				m_lines.at(m_curr_active_line_idx).setShowCursor(false);

				//juste après cette dernière, on insert une ligne avec le contenu renvoyé par l'édition,
				m_for_addLine__action_idx = 1;
				addLine(m_curr_active_line_idx+1, returned_part, nullptr, true, true); // Avec l'argument 'incr_active_ln_idx' à 'true', 'm_curr_active_line_idx' est incrémenté.


				//enfin, on représente visuellement que c'est sur cette ligne qu'est le curseur.
				m_lines.at(m_curr_active_line_idx).setShowCursor(true);



				//Enregistrement de l'action globale.
				Text_Win_Action& conclusion_act = pushAction(te::INFO, -1, "");
				conclusion_act.info = te::END_OF_NEW_LINE_PROCESS;
				conclusion_act.action_idx = 2;
			}

			//Pour potentiellement remettre la ligne sélectionnée sur le champ de vue.
			updateYScroll();
			updateXScroll();
		}

		it->second = false;
		break;
	}

	//La souris.
	if(m_check_mouse)
	{
		char clicks_count = -1;
		bool is_double_click_cond_ok = (m_mouse_tracker.isMultiClick(&clicks_count) and (clicks_count%2 == 0) and m_mouse_tracker.last2DownClicksHaveSamePos());

		if(m_inputs[te::MOUSE_CLICKED])
		{
			mouseClickedAt(m_mouse_clicked_pos);
			m_inputs[te::MOUSE_CLICKED] = false;

			if(m_mouse_tracker.isDown() and is_double_click_cond_ok)
			{
				//Si après l'appel de 'mouseClickedAt()', m_curr_active_line_idx est valide, on procède à la sélection par double click.

				if(m_curr_active_line_idx != te::NO_ACTIVE_LINE)
				{
					unselect();

					if(m_lines.at(m_curr_active_line_idx).selectTextPortionByCursorToken(m_ASCII_token_separators))
						setASelectedLine(m_lines.at(m_curr_active_line_idx), m_curr_active_line_idx, false);

				}

			}
		}


		//Si la molette de la souris a été actionnée.
		if(m_wheel.wheeled)
		{
			m_wheel.wheeled = false; //On désactive puisqu'on va prendre en compte l'évènement.

			if(m_last_mouse_click_is_in) //Si le gs::Text_Window est "actif".
			{
				// 'm_wheel.val' est négative si la molette est tournée vers l'arrière (vers soi), positive si c'est vers l'avant.
				short speed = m_wheel.speed;//Environ `speed' lignes par seconde.
				setScrollY(m_y_scroll + float(-speed*m_lines_delta*m_wheel.val)*dt);

				m_wheel.is_on_ending_fade = true;
				m_wheel.fading_timer = 0.225f;
			}

		}
		else
		if(m_wheel.is_on_ending_fade and m_last_mouse_click_is_in)
		{
			if(m_wheel.fading_timer > 0)
			{
				m_wheel.fading_timer-= dt;
				float t = m_wheel.fading_timer;
				short speed = m_wheel.speed*1.16f;//Environ `speed' lignes par seconde.

				setScrollY(m_y_scroll + float(m_lines_delta*(-m_wheel.val*speed/7))*t*dt);

			}
			else
				m_wheel.is_on_ending_fade = false;
		}

		//Côté particulièrement lié à la sélection.
		m_mouse_tracker.update();
		{
			if(m_mouse_tracker.isDown() and (not is_double_click_cond_ok or m_mouse_tracker.moved()))
				processSelection(m_curr_active_line_idx);
		}

	}

	//Les flèches du clavier et le curseur.
	if(m_curr_active_line_idx != te::NO_ACTIVE_LINE)
	{
		if(m_inputs[te::UP_ARROW])
		{
			m_inputs[te::UP_ARROW, false];
			moveCursor(0, -1);
		}
		if(m_inputs[te::DOWN_ARROW])
		{
			m_inputs[te::DOWN_ARROW, false];
			moveCursor(0, 1);
		}

		if(m_inputs[te::LEFT_ARROW])
		{
			m_inputs[te::LEFT_ARROW, false];
			moveCursor(-1, 0);
		}
		if(m_inputs[te::RIGHT_ARROW])
		{
			m_inputs[te::RIGHT_ARROW, false];
			moveCursor(1, 0);
		}
	}


	updateMaxLineW(last_actions_count);

	m_blocks_folder.updateByIndents();

}

void Text_Window::update(float dt)
{
	try{

	return examineEvents(dt);

	}
	catch(std::exception& err)
	{
		gs::lastError(std::string("!! excep from 'Text_Window::update()' which call 'Text_Window::examineEvents()' : ") + err.what());
	}
}



bool Text_Window::mouseClickedAt(const Vec2i& click_pos)
{
	int prev_active_ln_idx = m_curr_active_line_idx;
	int prev_cur_pos = (prev_active_ln_idx == te::NO_ACTIVE_LINE ? -1
						: m_lines.at(prev_active_ln_idx).getCursorPos());


	if(not isPointInTextArea(click_pos))
	{
		//Si, bienque hors de la zone de texte, le click est dans au moins une zone "amie",
		if(isPointInFriendArea(click_pos))
			return true; //on considère la réussite, sans rien faire de plus.


		if(m_curr_active_line_idx != te::NO_ACTIVE_LINE)
			m_lines.at(m_curr_active_line_idx).setShowCursor(false);

		m_curr_active_line_idx = te::NO_ACTIVE_LINE;
		m_last_mouse_click_is_in = false;

		pushAction(te::NEW_CURSOR_POS, m_curr_active_line_idx, "", -1, prev_cur_pos, {-1,-1}, prev_active_ln_idx);

		return false;
	}

	m_last_mouse_click_is_in = true;


	//S'il y avait une ligne précédemment sélectionnée, on affiche plus son curseur.
	if(prev_active_ln_idx != te::NO_ACTIVE_LINE)
		m_lines.at(prev_active_ln_idx).setShowCursor(false);


	int click_y = m_blocks_folder.adjustClickYPos(click_pos.y-m_pos.y, m_y_scroll, m_lines_delta); //Prise en compte des lignes pliées
	m_curr_active_line_idx = click_y / m_lines_delta;

	//Si le click, bien que dans la zone de texte, ne touche pas de ligne.
	if((m_curr_active_line_idx < 0) or (m_curr_active_line_idx >= m_lines.size()))
	{
		m_curr_active_line_idx = te::NO_ACTIVE_LINE;

		pushAction(te::NEW_CURSOR_POS, m_curr_active_line_idx, "", -1, prev_cur_pos, {-1,-1}, prev_active_ln_idx);

		return false;
	}


	//S'il y avait une ligne précédemment active, on affiche plus son curseur.
	//if((prev_active_ln_idx >= 0) and (prev_active_ln_idx < m_lines.size()))
		//m_lines.at(prev_active_ln_idx).setShowCursor(false);


	//Définition de la position du curseur en fonction du click.
	Text_Line& now_active_line = m_lines.at(m_curr_active_line_idx);
	int pixel_pos = click_pos.x - now_active_line.getPos().x  +m_x_scroll;

	if(prev_cur_pos == -1) //S'il n'y avait pas de ligne active avant, on prend la précédente position du curseur de la ligne ciblée comme position précédente du curseur.
		prev_cur_pos = now_active_line.getCursorPos();

	now_active_line.setCursorPosByPixels(pixel_pos);
	now_active_line.setShowCursor(true);

	updateXScroll();

	pushAction(te::NEW_CURSOR_POS, m_curr_active_line_idx, "",
				now_active_line.getCursorPos(), prev_cur_pos, {-1,-1}, prev_active_ln_idx);

	return true;

}




bool Text_Window::save(const char* file_path) const
{
	std::ofstream file;
	file.open(file_path, std::ios::out);

	if(not file.is_open())
	{
		gs::lastError(std::string("||Warning|| in 'Text_Window::save()', cannot open file : \"")+std::string(file_path)+"\". ");
		return false;
	}

	for(const Text_Line& line : m_lines)
	{
		file << line.getText();
	}

	file.close();
	return true;
}





bool Text_Window::addLine(int index, const std::string& text, std::string* out, bool incr_active_ln_idx, bool record_this_action, Text_Win_Action* out__done_action)
{
	try{

	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::addLine()', not yet init. Use 'Text_Window::initBy...()' before. ");
		return false;
	}


	if(index < 0)
	{
		gs::lastError(std::string("||Warning|| in 'Text_Window::addLine()', invalid given index : [") + std::to_string(index) + std::string("]."));
		return false;
	}

	unselect();

	int prev_active_ln_idx = m_curr_active_line_idx;
	setCurrLineOfEdit(te::NO_ACTIVE_LINE); //On "désactive" d'abord la potentielle ligne active.


	std::vector<Text_Line>::iterator lines_it;

	if(index >= int(m_lines.size()))
	{
		lines_it = m_lines.end();
		index = int(m_lines.size());
	}
	else
		lines_it = m_lines.begin()+index;

	m_lines.insert(lines_it, Text_Line()); // !! Point critique : amélioration -> une vérification de la réussite de '.insert()'.
	Text_Line& new_line = m_lines.at(index);

		//Décalage de la ligne courante si demandé.
		if(incr_active_ln_idx and prev_active_ln_idx != te::NO_ACTIVE_LINE)
			setCurrLineOfEdit(prev_active_ln_idx+1);
		else
		if(not incr_active_ln_idx and prev_active_ln_idx != te::NO_ACTIVE_LINE)
			setCurrLineOfEdit(prev_active_ln_idx); //Sinon on remet la potentielle précédente ligne active.


	new_line.setBank(&m_common_bank);
	new_line.setEndLineChar(m_end_line_char);
	new_line.clearRules(FONT_STYLE_NORMAL, m_common_ptsize, u32RGBA(m_fg_color), 0);

	if(out)
		*out = new_line.setText(text);
	else
		new_line.setText(text);

	new_line.setMaxSize( {m_size.x-m_x_margin, static_cast<unsigned int>(m_lines_delta)} ); //~
	new_line.setSelectionColor(m_selection_bg_color);
	new_line.update(m_tab_size);

	if(record_this_action)
		pushAction(te::NEW_LINE, index, new_line.getText(), new_line.getCursorPos(), static_cast<int>(incr_active_ln_idx)).action_idx = m_for_addLine__action_idx;

	if(out__done_action) {*out__done_action =
		makeAction(te::NEW_LINE, index, new_line.getText(), new_line.getCursorPos(), static_cast<int>(incr_active_ln_idx)); out__done_action->action_idx = m_for_addLine__action_idx;}


	m_blocks_folder.takeEdit(te::NEW_LINE, index, &new_line, m_tab_size);

	m_for_addLine__action_idx = 0; //Par sécurité.

	return true;

	}
	catch(std::exception& excep)
	{
		gs::lastError("!! excep in 'Text_Window::addLine()' : ", excep.what());
	}

	return false; //En cas d'exception.
}



bool Text_Window::removeLine(int index, std::string* out__rmvd_line_text, bool record_this_action, Text_Win_Action* out__done_action)
{
	try{

	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::removeLine()', not yet init. Use 'Text_Window::initBy...()' before. ");
		return false;
	}


	if(index < 0 or index >= m_lines.size())
	{
		gs::lastError(std::string("||Warning|| in 'Text_Window::removeLine()', invalid given index : [") + std::to_string(index) + std::string("]."));
		return false;
	}

	//if(m_lines.empty()) return false;


	unselect();

	if(m_curr_active_line_idx != te::NO_ACTIVE_LINE)
	{
		if(index < m_curr_active_line_idx)
		{
			--m_curr_active_line_idx;
		}
		else
		if(index == m_curr_active_line_idx)
			m_curr_active_line_idx = te::NO_ACTIVE_LINE;
	}


	std::string rmvd_line_text = m_lines.at(index).getText();
	int rmvd_line_cursor_pos = m_lines.at(index).getCursorPos();

	m_lines.erase(m_lines.begin()+index);

	//Sécurité.
	if(m_curr_active_line_idx < 0)
		m_curr_active_line_idx = te::NO_ACTIVE_LINE;

	if(record_this_action)
		pushAction(te::REMOVE_LINE, index, rmvd_line_text, -1, rmvd_line_cursor_pos).action_idx = m_for_removeLine__action_idx;

	if(out__done_action) {*out__done_action =
		makeAction(te::REMOVE_LINE, index, rmvd_line_text, -1, rmvd_line_cursor_pos); out__done_action->action_idx = m_for_removeLine__action_idx;}


	m_blocks_folder.takeEdit(te::REMOVE_LINE, index, nullptr, m_tab_size);

	m_for_removeLine__action_idx = 0; //Par sécurité.

	if(out__rmvd_line_text)
		*out__rmvd_line_text = rmvd_line_text;

	return true;

	}
	catch(std::exception& excep)
	{
		gs::lastError("!! excep in 'Text_Window::removeLine()' : ", excep.what());
	}

	return false; //En cas d'exception.
}





bool Text_Window::act(const Text_Win_Action& action, bool record_this_action, Text_Win_Action* out__done_action)
{
	try{

	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::act()', not yet init. Use 'Text_Window::initBy...()' before. ");
		return false;
	}


	if(action.type == te::NONE) return true;

	int line_index = action.line_index;


	if(line_index == te::NO_ACTIVE_LINE and action.type == te::NEW_CURSOR_POS) //En cas de désélection de ligne d'activité.
	{
		unselect();

		int prev_line_index = m_curr_active_line_idx;
		int prev_cur_pos = -1;

		if(m_curr_active_line_idx != te::NO_ACTIVE_LINE)
			prev_cur_pos = m_lines.at(m_curr_active_line_idx).getCursorPos();

		setCurrLineOfEdit(te::NO_ACTIVE_LINE); //Modifie 'm_curr_active_line_idx' en fonction.

		if(record_this_action)
			pushAction(te::NEW_CURSOR_POS, m_curr_active_line_idx, "", prev_cur_pos, prev_cur_pos, {-1,-1}, prev_line_index);

		if(out__done_action) *out__done_action =
			makeAction(te::NEW_CURSOR_POS, m_curr_active_line_idx, "", prev_cur_pos, prev_cur_pos, {-1,-1}, prev_line_index);

		updateYScroll();
		updateXScroll();

		return true;
	}


	if(action.type == te::NEW_TEXT) //Action n'ayant pas besion de 'line_index'. Les testes suivants peuvent bloquer "son passage", de ce fait elle est traitée en amont.
	{
		if(not this->setText(action.str, record_this_action, out__done_action))
			return false;

		updateYScroll();
		updateXScroll();

		return true;
	}


	//Si l'index (de ligne) demandé n'est pas valide, on prend la ligne sélectionnée.
	if(line_index < 0 or line_index >= int(m_lines.size()))
		line_index = m_curr_active_line_idx;

	//S'il n'y a pas non plus de ligne sélectionnée, on quitte.
	if(line_index == te::NO_ACTIVE_LINE)
		return false;

	//Sécurité.
	if(line_index < 0 or line_index >= int(m_lines.size()))
		return false;


	if(action.type == te::INSERT_CHAR)
	{
		unselect();

		if(not m_lines.at(line_index).isUpToDate())
			m_lines.at(line_index).update(m_tab_size);

		//Si la position demandée du curseur est négative, on laisse le curseur tel quel.
		if(action.begin_cursor_pos >= 0)
			m_lines.at(line_index).setCursorPos(action.begin_cursor_pos);

		int prev_cur_pos = m_lines.at(line_index).getCursorPos();

		std::string cursor_to_end_part;
		if(action.str.contains(m_lines.at(line_index).getEndLineChar()))
			cursor_to_end_part = m_lines.at(line_index).getTextFromCursorToEnd();

		m_lines.at(line_index).addText(action.str, nullptr, true);

		m_lines.at(line_index).update(m_tab_size);

		if(action.new_cursor_pos >= 0)
			m_lines.at(line_index).setCursorPos(action.new_cursor_pos); //Afin de respecter ce qui est demandé.


		if(record_this_action)
			pushAction(te::INSERT_CHAR, line_index, action.str,
						m_lines.at(line_index).getCursorPos(), prev_cur_pos, {-1,-1}, line_index, cursor_to_end_part);

		if(out__done_action) *out__done_action =
			makeAction(te::INSERT_CHAR, line_index, action.str,
						m_lines.at(line_index).getCursorPos(), prev_cur_pos, {-1,-1}, line_index, cursor_to_end_part);


		m_blocks_folder.takeEdit(te::INSERT_CHAR, line_index, &m_lines.at(line_index), m_tab_size);

	}
	else
	if(action.type == te::REMOVE_CHAR)
	{
		unselect();

		//Avant:
		//int count = action.begin_cursor_pos - action.new_cursor_pos;
		//if(count <= 0) return false;

		m_lines.at(line_index).setCursorPos(action.begin_cursor_pos); // #<A>
		int prev_cur_pos = m_lines.at(line_index).getCursorPos();

		//Après:
		int count = prev_cur_pos - action.new_cursor_pos; //A ce stade 'prev_cur_pos' est (normalement) le résulat produit depuis la ligne avec la balise #<A>, et si ça s'est passé correctement, alors on a 'prev_cur_pos == action.begin_cursor_pos'. Mais par mesure de sécurité on prend bien 'prev_cur_pos' pour déterminer 'count', au cas où le curseur n'a pas pu se mettre au niveau de 'action.begin_cursor_pos'.
		if(count < 0) return false;


		if(not m_lines.at(line_index).isUpToDate())
			m_lines.at(line_index).update(m_tab_size);

		std::string rvmd;

		if(count == 0) //Quand on veut effacer, mais que le curseur ne doit pas changer de position, on va considérer que l'on demande à effacer depuis le côté droit du curseur.
			rvmd = m_lines.at(line_index).removeFromCursorRightSide(static_cast<int>(getUTF8CharsCount(action.str))); // 'action.str' est attendu ici comme contenant une chaîne dont le nombre de caractères (UTF-8) sert à déterminer le nombre de caractères à effacer. Ceci est particulièrement pratique/interessant lors de l'usage d'un redo sur un effacement côté droit (car la partie effacée, stockée dans 'action.str', pourra de nouveau être affacé comme précédemment).
		else
			rvmd = m_lines.at(line_index).removeFromCursorLeftSide(count);

		m_lines.at(line_index).update(m_tab_size);


		//Ici (pour REMOVE_CHAR) ceci n'est peut-être pas nécessaire.
		if(action.new_cursor_pos >= 0)
			m_lines.at(line_index).setCursorPos(action.new_cursor_pos); //Afin de respecter ce qui est demandé.


		if(record_this_action)
			pushAction(te::REMOVE_CHAR, line_index, rvmd,
						m_lines.at(line_index).getCursorPos(), prev_cur_pos);

		if(out__done_action) *out__done_action =
			makeAction(te::REMOVE_CHAR, line_index, rvmd,
						m_lines.at(line_index).getCursorPos(), prev_cur_pos);


		m_blocks_folder.takeEdit(te::REMOVE_CHAR, line_index, &m_lines.at(line_index), m_tab_size);

	}
	else
	if(action.type == te::NEW_CURSOR_POS)
	{
		//unselect(); !!! Ne pas unselect ici, car 'Text_Window::eraseSelection()' fait appel à cette méthode-ci ('Text_Window::act()') avec une action de type 'NEW_CURSOR_POS'. Si on unselect ici ça perturbe son fonctionnement.

		int prev_line_index = te::NO_ACTIVE_LINE;
		int prev_cur_pos = m_lines.at(line_index).getCursorPos(); //Par défaut on prend la précédente position du curseur de la ligne ciblée, pour définir la précédente position du curseur.

		if(m_curr_active_line_idx != te::NO_ACTIVE_LINE)
		{
			m_lines.at(m_curr_active_line_idx).setShowCursor(false);//Masquage du curseur de la ligne précédemment active.

			prev_line_index = m_curr_active_line_idx;
			prev_cur_pos = m_lines.at(m_curr_active_line_idx).getCursorPos(); //Mais s'il y avait une ligne d'édition active, on la position du curseur de celle-ci comme la précédente position du curseur.
		}


		//Changement de ligne et affichage de son curseur.
		m_curr_active_line_idx = line_index;
		m_lines.at(m_curr_active_line_idx).setShowCursor(true);


		//Position du curseur dans la ligne.
		if(action.new_cursor_pos >= 0)
			m_lines.at(m_curr_active_line_idx).setCursorPos(action.new_cursor_pos);


		if(record_this_action)
			pushAction(te::NEW_CURSOR_POS, line_index, "",
						m_lines.at(line_index).getCursorPos(), prev_cur_pos, {-1,-1}, prev_line_index);

		if(out__done_action) *out__done_action =
			makeAction(te::NEW_CURSOR_POS, line_index, "",
						m_lines.at(line_index).getCursorPos(), prev_cur_pos, {-1,-1}, prev_line_index);

	}
	else
	if(action.type == te::NEW_LINE)
	{
		//Note: ici l'attribut 'Text_Win_Action::begin_cursor_pos' sert à déterminer l'argument 'bool incr_active_ln_idx' de 'Text_Window::addLine()'. Pour 'begin_cursor_pos == 1', 'incr_active_ln_idx' est mis à 'true', et pour tout autre valeur de 'begin_cursor_pos', 'incr_active_ln_idx' est mis à 'false' (afin de favoriser un 'true' intentionnel).

		if(not addLine(line_index, action.str, nullptr, action.begin_cursor_pos==1, record_this_action, out__done_action))
			return false;

		//Gestion du curseur.
		if(action.new_cursor_pos >= 0)
			m_lines.at(line_index).setCursorPos(action.new_cursor_pos);

	}
	else
	if(action.type == te::REMOVE_LINE)
	{
		if(not removeLine(action.line_index, nullptr, record_this_action, out__done_action))
			return false;
	}
	else
	if(action.type == te::ERASE_LINE_SELECTION)
	{
		Text_Line& target_line = m_lines.at(line_index);

		if(not target_line.selectTextPortionByCursorsPos(action.selection_bounds.x, action.selection_bounds.y+1))
			return false;

		if(not target_line.eraseSelection())
		{
			target_line.unselectTextPortion();
			return false;
		}


		Selection_Act_Record s_a_r = target_line.getLastRecordedSelectionAct();

		if(record_this_action)
			if(not s_a_r.it_is_for_replacement)
				pushAction(te::ERASE_LINE_SELECTION, line_index, s_a_r.origin_str, s_a_r.end_cursor_pos, s_a_r.begin_cursor_pos, s_a_r.selection_bounds);


		if(out__done_action)
			if(not s_a_r.it_is_for_replacement) *out__done_action =
				makeAction(te::ERASE_LINE_SELECTION, line_index, s_a_r.origin_str, s_a_r.end_cursor_pos, s_a_r.begin_cursor_pos, s_a_r.selection_bounds);


		m_blocks_folder.takeEdit(te::ERASE_LINE_SELECTION, line_index, &target_line, m_tab_size);

	}
	else
	if(action.type == te::REPLACE_LINE_SELECTION)
	{
		Text_Line& target_line = m_lines.at(line_index);

		if(not target_line.selectTextPortionByCursorsPos(action.selection_bounds.x, action.selection_bounds.y+1))
			return false;

		if(not target_line.replaceSelection(action.str_2, nullptr))
		{
			target_line.unselectTextPortion();
			return false;
		}

		Selection_Act_Record s_a_r = target_line.getLastRecordedSelectionAct();

		if(record_this_action)
			if(s_a_r.it_is_for_replacement)
				pushAction(te::REPLACE_LINE_SELECTION, line_index, s_a_r.origin_str, s_a_r.end_cursor_pos, s_a_r.begin_cursor_pos, s_a_r.selection_bounds, line_index, s_a_r.new_str);

		if(out__done_action)
			if(s_a_r.it_is_for_replacement) *out__done_action =
				makeAction(te::REPLACE_LINE_SELECTION, line_index, s_a_r.origin_str, s_a_r.end_cursor_pos, s_a_r.begin_cursor_pos, s_a_r.selection_bounds, line_index, s_a_r.new_str);


		m_blocks_folder.takeEdit(te::REPLACE_LINE_SELECTION, line_index, &target_line, m_tab_size);

	}
	else
	if(action.type == te::LINE_NEW_TEXT)
	{
		unselect();

		if(not m_lines.at(line_index).isUpToDate())
			m_lines.at(line_index).update(m_tab_size);


		std::string line__prev_text = m_lines.at(line_index).getText();
		int prev_cur_pos = m_lines.at(line_index).getCursorPos();

		m_lines.at(line_index).setText(action.str);
		m_lines.at(line_index).update(m_tab_size);

		if(record_this_action)
			pushAction(te::LINE_NEW_TEXT, line_index, m_lines.at(line_index).getText(),
						m_lines.at(line_index).getCursorPos(), prev_cur_pos, {-1,-1}, line_index, line__prev_text);

		if(out__done_action) *out__done_action =
			makeAction(te::LINE_NEW_TEXT, line_index, m_lines.at(line_index).getText(),
						m_lines.at(line_index).getCursorPos(), prev_cur_pos, {-1,-1}, line_index, line__prev_text);


		m_blocks_folder.takeEdit(te::LINE_NEW_TEXT, line_index, &m_lines.at(line_index), m_tab_size);

	}
	else
	{
		gs::lastError("!! in 'Text_Window::act()', unsupported action : ", std::to_string(static_cast<int>(action.type))+".");
		return false;
	}
	}
	catch(std::exception& excep)
	{
		gs::lastError("!! excep in 'Text_Window::act()' : ", excep.what());
		return false;
	}



	updateYScroll();
	updateXScroll();

	return true;
}







void Text_Window::updateXScroll()
{
	//Rôle : gérer le positionnement de l'affichage du texte en fonction de la position du curseur sur la ligne.

	if(m_x_scroll_is_blocked) //Mais ne procède pas si le scrolling horizontal est blocqué.
		return;

	if(m_curr_active_line_idx != te::NO_ACTIVE_LINE)
	{
		int active_ln_cursor_x = m_lines.at(m_curr_active_line_idx).getCursorPosPixelIndex();
		int active_ln_max_size_x = m_lines.at(m_curr_active_line_idx).getMaxSize().x;
		int padding1 = static_cast<float>(m_size.x)*0.1f; //m_common_ptsize*4; //Choix arbitraire d'une marge.
		int padding2 = static_cast<float>(m_size.x)*0.1f;//m_common_ptsize*4; //Choix arbitraire d'une marge.
		int padding3 = m_common_ptsize*3; //Choix arbitraire d'une marge.

		if(active_ln_cursor_x-m_x_scroll +padding1 > active_ln_max_size_x)//Cursor est sorti (ou s'en rapproche (avec la marge choisie)) de la zone visible à droite.
		{
			m_x_scroll = active_ln_cursor_x-active_ln_max_size_x +padding2;
			//Observation: lorsque padding1 < padding2, l'ajustement du scrolling horizontal se fait par seuil d'entrée dans la zone à scroller. Lorsque padding1 >= padding2, chaque nouveau caractère dans la zone fait avancer le scrolling.
		}
		else
		if(active_ln_cursor_x < (m_x_scroll)) //Cursor est sorti de la zone visible à gauche.
		{
			m_x_scroll = active_ln_cursor_x-padding3;
		}
	}
	else
		m_x_scroll = 0;

	if(m_x_scroll < 0)
		m_x_scroll = 0;
}





void Text_Window::updateYScroll()
{
	if(m_curr_active_line_idx == te::NO_ACTIVE_LINE)
		return;

	int active_ln_y = m_curr_active_line_idx*m_lines_delta
						-m_blocks_folder.getHiddenLinesCount(m_curr_active_line_idx)*m_lines_delta
						-m_y_scroll; //Position, en pixel, de la ligne où se trouve le curseur.

	if(active_ln_y < 0)//Si la ligne où se trouve le curseur est hors champ vers le haut.
	{
		setScrollY(active_ln_y +m_y_scroll); //Je fais '+m_y_scroll' pour retirer le '-m_y_scroll' qui a servi plus haut. Car ici on ne veut pas prendre en compte l'encien niveau de scroll, mais seulement la position de la ligne dans le repère du 'Text_Window', afin d'établir le nouveau niveau de scroll.

		//Note: d'un point de vu externe au 'Text_Window', le scrolling Y ne prend en compte que les lignes non cachées (si je ne me trompe pas).
	}
	else
	if((active_ln_y+m_lines_delta) >= m_size.y) //Si la ligne où se trouve le curseur est hors champ (ou partiellement) vers le bas.
	{
		setScrollY(m_y_scroll + float((active_ln_y+m_lines_delta) - m_size.y));
	}
}




bool Text_Window::setCurrLineOfEdit(int line_index)
{
	try{

	if(not m_is_init) return false;

	//unselect(); ???

	if(line_index == te::NO_ACTIVE_LINE) //Si l'on cherche à ne pas avoir de ligne d'édition courante:
	{
		if(m_curr_active_line_idx == te::NO_ACTIVE_LINE) //Si c'est déjà le cas, succès.
			return true;

		//Sinon, on désélectionne la ligne d'édition courante.
		m_lines.at(m_curr_active_line_idx).setShowCursor(false);
		m_curr_active_line_idx = te::NO_ACTIVE_LINE;

		return true;
	}


	if(line_index < 0 or line_index >= m_lines.size()) //Si l'indice n'est pas valide.
		return false;

	//Sinon:

	if(m_curr_active_line_idx == te::NO_ACTIVE_LINE) //S'il n'y a pas de ligne actuellement active.
		m_curr_active_line_idx = line_index;

	else
	{
		if(m_curr_active_line_idx == line_index) //Si pas de changement nécessaire.
			return true;

		//Sinon on hide le curseur de la ligne précédemment active,
		m_lines.at(m_curr_active_line_idx).setShowCursor(false);

		//on passe à la nouvelle,
		m_curr_active_line_idx = line_index;
	}

	//et affiche son curseur.
	m_lines.at(m_curr_active_line_idx).setShowCursor(true);

	updateYScroll();

	return true;



	}
	catch(std::exception& err){
		gs::lastError(std::string("!! excep in 'Text_Window::setCurrLineOfEdit()' : ")+err.what());

	}
	return false; //En cas d'exception.
}



int Text_Window::getCurrLineOfEdit() const//getSelectedLineIndex() const
{
	return m_curr_active_line_idx;
}


bool Text_Window::haveCurrActiveLine() const
{
	return m_curr_active_line_idx != te::NO_ACTIVE_LINE;
}


bool Text_Window::setCursorPos(int cursor_index)
{
	if(not m_is_init)
	{
		gs::lastError("!! in 'Text_Window::setCursorPos()', not init. [Use 'Text_Window::initBy...' before.]");
		return false;
	}

	if(m_curr_active_line_idx != te::NO_ACTIVE_LINE)
	{
		m_lines.at(m_curr_active_line_idx).update(m_tab_size);

		int final_cursor_pos = m_lines.at(m_curr_active_line_idx).setCursorPos(cursor_index);
		bool res = final_cursor_pos == cursor_index;

		if(not res)
			gs::lastError(std::string("!! in 'Text_Window::setCursorPos()', final cursor pos index is not as expected. [Expectation : ")+std::to_string(cursor_index)+std::string(", got : ")+std::to_string(final_cursor_pos)+"]. ");

		return res;
	}

	gs::lastError("!! in 'Text_Window::setCursorPos()', no current active line.");
	return false;
}


int Text_Window::getCursorPos() const
{
	if(m_curr_active_line_idx != te::NO_ACTIVE_LINE)
		return m_lines.at(m_curr_active_line_idx).getCursorPos();

	return te::NO_ACTIVE_LINE;
}


int Text_Window::getMaxCursorPos() const
{
	if(m_curr_active_line_idx != te::NO_ACTIVE_LINE)
		return m_lines.at(m_curr_active_line_idx).getMaxCursorPos();

	return te::NO_ACTIVE_LINE;
}





bool Text_Window::setPos(gs::Vec2i pos) {m_pos = pos; return true;}

gs::Vec2i Text_Window::getPos() const{return m_pos;}



bool Text_Window::setSize(gs::Vec2u size) {m_size = size; return true;}

bool Text_Window::setSize(gs::Vec2i size) {return setSize(size.to<unsigned int>());}

gs::Vec2u Text_Window::getSize() const{return m_size;}



bool Text_Window::setMarginX(short val)
{
	m_x_margin = val;
	if(m_x_margin < 0)
	{
		m_x_margin = 0;
		return false;
	}
	if(m_x_margin > m_size.x)
	{
		m_x_margin = m_size.x;
		return false;
	}

	return true;
}

short Text_Window::getMarginX() const {return m_x_margin;}


float Text_Window::setScrollY(float val)
{
	int lines_count = m_lines.size();
	int folded_lines_count = m_blocks_folder.getHiddenLinesCount();

	if(folded_lines_count > lines_count)
		folded_lines_count = lines_count; //Sécurité

	// if(lines_count < 1)
	// 	return m_y_scroll;

	m_y_scroll = val;
	if(m_y_scroll < 0)
	{
		m_y_scroll = 0;
		return m_y_scroll;
	}

	if(m_y_scroll > (m_lines_delta * (lines_count-folded_lines_count-1)))
	{
		m_y_scroll = (m_lines_delta * (lines_count-folded_lines_count-1));
		return m_y_scroll;
	}

	return m_y_scroll;
}


float Text_Window::setMaxScrollY()
{
	int lines_count = m_lines.size();
	int folded_lines_count = m_blocks_folder.getHiddenLinesCount();

	m_y_scroll = (m_lines_delta * (lines_count-folded_lines_count-1));
	return m_y_scroll;
}


float Text_Window::getScrollY() //const
{
	//return m_y_scroll;

	//m_y_scroll = m_blocks_folder.adjustYScroll(m_y_scroll, m_lines_delta); //Prise en compte des lignes pliées

	return m_y_scroll;

	//return m_blocks_folder.adjustYScroll(m_y_scroll, m_lines_delta); //Prise en compte des lignes pliées

}



float Text_Window::setScrollX(float val)
{
	if(m_x_scroll_is_blocked)
		return m_x_scroll;

	if(val < 0)
	{
		m_x_scroll = 0;
		return m_x_scroll;
	}

	m_x_scroll = val;
	return m_x_scroll;
}


float Text_Window::getScrollX() const {return m_x_scroll;}

float Text_Window::blockScrollXTo(float val) //Pour blocquer la valeur du srcolling horizontal à une valeur fixe.
{
	float prev_x_scroll = m_x_scroll;

	m_x_scroll_is_blocked = false;
	setScrollX(val);
	m_x_scroll_is_blocked = true;

	return prev_x_scroll;
}


float Text_Window::releaseScrollX() //Pour rendre libre (si blockScrollXTo() a été utilisée) le scrolling horizontal.
{
	m_x_scroll_is_blocked = false;
	return m_x_scroll;
}



bool Text_Window::setTabulationSize(char val)
{
	m_tab_size = val;
	if(m_tab_size < 2)
	{
		m_tab_size = 2;
		return false;
	}

	return true;
}

char Text_Window::getTabulationSize() const {return m_tab_size;}


bool Text_Window::isPointInTextArea(gs::Vec2i pos) const
{
	Point pos_=pos.toPoint();
	Rect text_area={m_pos.x, m_pos.y, static_cast<int>(m_size.x), static_cast<int>(m_size.y)};

	return pointInRect(&pos_, &text_area) == true;
}


bool Text_Window::isPointInFriendArea(gs::Vec2i pos) const
{
	Point pos_=pos.toPoint();

	for(const Rect& friend_area : m_friend_areas)
		if(pointInRect(&pos_, &friend_area))
			return true;

	return false;
}



bool Text_Window::setColor(te::Text_Win_Area area, Color color)
{
	switch(area)
	{
		case te::FOREGROUND: m_fg_color = color; break;
		case te::BACKGROUND: m_bg_color = color; break;
		case te::SELECTED_LINE_BACKGROUND: m_selected_line_bg_color = color; break;
		case te::SELECTION_BACKGROUND: m_selection_bg_color = u32RGBA(color); break;
		default: return false;
	};

	if(area == te::FOREGROUND)
	{
		for(Text_Line& line : m_lines)
		{
			line.clearRules(FONT_STYLE_NORMAL, m_common_ptsize, u32RGBA(m_fg_color), 0);
			line.update(m_tab_size);
		}
	}
	else if(area == te::SELECTION_BACKGROUND)
		for(Text_Line& line : m_lines)
			line.setSelectionColor(m_selection_bg_color);



	return true;
}

Color Text_Window::getColor(te::Text_Win_Area area) const
{
	switch(area)
	{
		case te::FOREGROUND: return m_fg_color;
		case te::BACKGROUND: return m_bg_color;
		case te::SELECTED_LINE_BACKGROUND: return m_selected_line_bg_color; break;
		case te::SELECTION_BACKGROUND: return gs::color(m_selection_bg_color); break;
		default: break;
	};

	return m_bg_color;
}




bool Text_Window::setMouseClickedIn()
{
	bool prev_state = m_last_mouse_click_is_in;
	m_last_mouse_click_is_in = true;
	return prev_state;
}

bool Text_Window::getMouseClickedIn()
{
	return m_last_mouse_click_is_in;
}




bool Text_Window::setTextColorEditor(int (*textColoring_)(const std::vector<Line__Text_Idx>& lines_text,
										std::map<int, std::vector<Text_Print_Rules>> &out_rules, Text_Print_Rules default_rules))
{
	_textColoring = textColoring_;

	if(textColoring_) return true;

	m_text_color_editor_changed_from_ok_to_nullptr = true;
	return false;
}


bool Text_Window::haveTextColorEditor() const {return _textColoring != nullptr;}



void Text_Window::textColoring()
{
	if(not _textColoring)
	{
		if(m_text_color_editor_changed_from_ok_to_nullptr)
		{
			//Reset de toutes les potentielles règles de coloration syntaxique précédentes.
			for(Text_Line& line : m_lines)
				line.clearRules(FONT_STYLE_NORMAL, m_common_ptsize, u32RGBA(m_fg_color), 0u);

			m_text_color_editor_changed_from_ok_to_nullptr = false;
		}

		return;
	}

	std::vector<Line__Text_Idx> lines;
	std::map<int, std::vector<Text_Print_Rules>> rules_by_lines;

	Text_Print_Rules default_rules={0, FONT_STYLE_NORMAL, uint16_t(m_common_ptsize), u32RGBA(m_fg_color), 0u};


	//On selectionne les lignes visibles, afin de limiter la quantité de traitements.
	{
		float y_scroll = m_blocks_folder.adjustYScroll(m_y_scroll, m_lines_delta); //Prise en compte des lignes pliées

		int cur_y = m_pos.y - int(y_scroll)%m_lines_delta;
		int max_y = m_pos.y + m_size.y-1;

		for(int i = int(y_scroll)/m_lines_delta; i< m_lines.size(); ++i)
		{
			if(m_blocks_folder.isLineHidden(i)) //Les lignes repliées ne sont pas affichées.
				continue;

			Text_Line& line = m_lines.at(i);

			//Rétablissement des règles par défaut avant le traitement des couleurs etc, afin que le traitement des règles puisse se faire sans influence de la frame précédente (notamment s'il y a eu des modifications des textes entre temps).
			line.clearRules(FONT_STYLE_NORMAL, m_common_ptsize, u32RGBA(m_fg_color), 0);

			lines.push_back({i, line.getText()});

			cur_y += m_lines_delta;

			if(cur_y > max_y)
				break;
		}
	}


	//On appelle le callback de la coloration syntaxique.
	_textColoring(lines, rules_by_lines, default_rules);


	//Enfin, on applique la coloration reçue.
	int last_line_index = int(m_lines.size())-1;
	for(const std::pair<int, std::vector<Text_Print_Rules>>& curr_line_data : rules_by_lines)
	{
		int line_index  										= curr_line_data.first;
		const std::vector<Text_Print_Rules>& curr_line_rules 	= curr_line_data.second;

		if( not (line_index >= 0 and line_index <= last_line_index)) //Sécurité
			continue;


		Text_Line& curr_line = m_lines.at(line_index);

		for(const Text_Print_Rules& rules : curr_line_rules)
			curr_line.addRules(rules.begin_index, rules.style, m_common_ptsize, rules.fg_color, rules.bg_color);

	}
}





bool Text_Window::moveCursor(int on_x_axis, int on_y_axis)
{
	try{

	if(m_lines.empty()) return false;
	if(m_curr_active_line_idx == te::NO_ACTIVE_LINE) return false;

	int prev_active_ln_idx = m_curr_active_line_idx;
	int prev_cursor_pos = m_lines.at(prev_active_ln_idx).getCursorPos();
	int prev_cursor_pixelpos = m_lines.at(prev_active_ln_idx).getCursorPosPixelIndex();
	int prev_max_cursor_pos = m_lines.at(prev_active_ln_idx).getMaxCursorPos();

	m_lines.at(m_curr_active_line_idx).setShowCursor(false);

	//On commence par traiter les cas particuliers.
	if(on_x_axis < 0 and prev_cursor_pos == 0 and m_curr_active_line_idx > 0) //S'il y a tentative de déplacer le curseur à gauche alors qu'il est déjà au minimum : on remonte d'une ligne et place le curseur à partir de la fin (à reculons) en fonction de la valeur de 'on_x_axis', si possible.
	{
		m_curr_active_line_idx--;

		int new_cursor_pos = m_lines.at(m_curr_active_line_idx).getMaxCursorPos() + (on_x_axis+1); //Note: ici 'on_x_axis' est strictement négative (le '+' permet donc bien de "reculer/aller à gauche" ici); de plus on retranche 1 à sa valeur absolue, ce qui produit que si l'on voulait déplacer de -1, alors le curseur se contenterait de se placer à la position max (comme si le fait de remonter d'une ligne consommait déjà un déplacement "vers la gauche").
		m_lines.at(m_curr_active_line_idx).setCursorPos(GetMax(0, new_cursor_pos)); //Usage de GetMax pour ne pas prendre les valeurs négatives et s'arrêter à la position minimale du curseur (aka 0).
	}
	else
	if(on_x_axis > 0 and prev_cursor_pos == prev_max_cursor_pos and m_curr_active_line_idx+1 < m_lines.size()) //S'il y a tentative de déplacer le curseur à droite tandis qu'il est déjà au maximum : on positionne le curseur au début de la ligne suivante, si possible.
	{
		m_curr_active_line_idx++;

		int new_cursor_pos = 0; //GetMin(on_x_axis, m_lines.at(m_curr_active_line_idx).getMaxCursorPos());
		m_lines.at(m_curr_active_line_idx).setCursorPos(new_cursor_pos);
	}
	else
	//Sinon on traite "normalement".
	if(on_x_axis != 0 or on_y_axis != 0)
	{
		//Haut bas.

		int next_line_index = m_curr_active_line_idx + on_y_axis;

		//Gestion des bords.
		if(next_line_index < 0) //Minimal.
			m_curr_active_line_idx = 0;

		else
		if(next_line_index >= static_cast<int>(m_lines.size())) //Maximal.
			m_curr_active_line_idx = static_cast<int>(m_lines.size())-1;

		else
			m_curr_active_line_idx = next_line_index;



		//Gauche droite.

		m_lines.at(m_curr_active_line_idx).setCursorPosByPixels(prev_cursor_pixelpos); //En pixel.

		int next_cursor_pos = m_lines.at(m_curr_active_line_idx).getCursorPos() + on_x_axis; //Pas en pixel.
		m_lines.at(m_curr_active_line_idx).setCursorPos(next_cursor_pos);

	}

	m_lines.at(m_curr_active_line_idx).setShowCursor(true);

	updateYScroll();
	updateXScroll();

	if(on_x_axis != 0 or on_y_axis != 0)
		pushAction(te::NEW_CURSOR_POS, m_curr_active_line_idx, "",
			m_lines.at(m_curr_active_line_idx).getCursorPos(),
			prev_cursor_pos, {-1,-1}, prev_active_ln_idx);


	}catch(std::exception& excep)
	{
		gs::lastError("!! excep in 'Text_Window::moveCursor()' : ", excep.what(), ". ");
		return false;
	}

	unselect();

	return true;

}


void Text_Window::setShowCursor(bool state) {m_cursor_is_hidden = !state;}

bool Text_Window::isCursorHidden() const {return m_cursor_is_hidden;}



bool Text_Window::setLookAtMouse(bool new_state)
{
	m_check_mouse = new_state;
	return m_check_mouse;
}



std::string Text_Window::setTitle(std::string title)
{
	std::string prev_title = m_title;
	m_title = title;
	return prev_title;
}

std::string Text_Window::getTitle() const
{
	return m_title;
}



size_t Text_Window::getLinesCount() const
{
	return m_lines.size();
}


short Text_Window::getLinesDelta() const
{
	return m_lines_delta;
}





void Text_Window::clearLastActions()
{
	m_last_actions.clear();
}



std::vector<Text_Win_Action> Text_Window::getLastActions() const
{
	return m_last_actions;
}



bool Text_Window::setResetLastActionsObserverWhenShow(bool new_state)
{
	bool prev_state = m_reset_last_act_observer_when_show;
	m_reset_last_act_observer_when_show = new_state;
	return prev_state;
}



Text_Win_Action& Text_Window::pushAction(te::Text_Win_Action_Type type, int line_index, const std::string& str, int new_cursor_pos, int begin_cursor_pos, Vec2i sel_bounds, int prev_line_index, const std::string& str_2)
{
	m_last_actions.emplace_back();

	Text_Win_Action& action = m_last_actions.back();

	action.type = type;
	action.line_index = line_index;
	action.str = str;
	action.new_cursor_pos = new_cursor_pos;
	action.begin_cursor_pos = begin_cursor_pos;
	action.selection_bounds = sel_bounds;
	action.prev_line_index = prev_line_index;
	action.str_2 = str_2;

	return action;
}



Text_Win_Action Text_Window::makeAction(te::Text_Win_Action_Type type, int line_index, const std::string& str, int new_cursor_pos, int begin_cursor_pos, Vec2i sel_bounds, int prev_line_index, const std::string& str_2)
{
	Text_Win_Action action;
	 action.type = type;
	  action.line_index = line_index;
	   action.str = str;
	    action.new_cursor_pos = new_cursor_pos;
	   action.begin_cursor_pos = begin_cursor_pos;
	  action.selection_bounds = sel_bounds;
	 action.prev_line_index = prev_line_index;
	action.str_2 = str_2;

	return action;
}




std::vector<Rect> Text_Window::setFriendAreas(const std::vector<Rect>& areas)
{
	std::vector<Rect> prev_f_a = m_friend_areas;
	m_friend_areas = areas;
	return prev_f_a;
}



std::vector<Rect> Text_Window::getFriendAreas() const
{
	return m_friend_areas;
}




unsigned int Text_Window::getMaxLineW() const
{
	// if(m_max_ln_w.x == 0)
	// 	setMaxLineW();

	return m_max_ln_w.x;
}

void Text_Window::setMaxLineW()
{
	unsigned int max=0;
	unsigned int i=0;
	for(Text_Line& ln : m_lines)
	{
		ln.update(m_tab_size);
		unsigned int curr_w = ln.getTextTotalRequiredWidth();
		if(max <= curr_w)
		{
			max = curr_w;
			m_max_ln_w.x = max;
			m_max_ln_w.y = i;
		}

		++i;
	}
};

void Text_Window::updateMaxLineW(int before_updating__last_actions_count) /// cas non traités (implémentera qui voudra) "NEW_TEXT", "REPLACE_LINE_SELECTION", "ERASE_LINE_SELECTION".
{
	//`before_updating__last_actions_count' est là pour ne traiter que les actions faites depuis la dernière appelle de 'examineEvents()'.

	if(m_max_ln_w.x == 0)
		setMaxLineW();

	int begin_idx = before_updating__last_actions_count;
	if(begin_idx < 0) return;

	if(begin_idx >= int(m_last_actions.size()))
		return;

	std::vector<Text_Win_Action>::iterator acts_it = m_last_actions.begin()+begin_idx;
	std::vector<Text_Win_Action>::iterator acts_end_it = m_last_actions.end();


	int i=0;
	while(acts_it != acts_end_it)
	{
		if(i>= 250) break; //Prévention contre une boucle infinie.

		if(acts_it->type == te::INSERT_CHAR or acts_it->type == te::LINE_NEW_TEXT)
		{
			if(m_max_ln_w.y < m_lines.size()) //Vérification de la validité des indices.
			if(acts_it->line_index >= 0 and acts_it->line_index < m_lines.size())
			{
				m_lines.at(acts_it->line_index).update(m_tab_size);
				// m_lines.at(m_max_ln_w.y).update(m_tab_size);

				unsigned int new_w = m_lines.at(acts_it->line_index).getTextTotalRequiredWidth();
				// unsigned int prev_w = m_lines.at(m_max_ln_w.y).getTextTotalRequiredWidth();
				if(new_w >= m_max_ln_w.x)
				{
					m_max_ln_w.x = new_w;
					m_max_ln_w.y = acts_it->line_index;
				}
				else
				if(acts_it->line_index == m_max_ln_w.y)
					setMaxLineW();

			}
		}
		else
		if(acts_it->type == te::REMOVE_CHAR)
		{
			if(acts_it->line_index == m_max_ln_w.y) //En cas de retrait de caractère-s sur la plus grande ligne.
			if(m_max_ln_w.y < m_lines.size()) //Vérification de la validité de l'indice.
			{
				setMaxLineW();
				// m_max_ln_w.x = 10;
			}
		}
		else
		if(acts_it->type == te::NEW_LINE)
		{
			if(acts_it->line_index <= m_max_ln_w.y)
				++m_max_ln_w.y; //S'il n'y a eu qu'une seule nouvelle ligne (plus haut ou au même niveau) entre-temps (frame), c'est bon, s'il y en a eu plusieurs, cette variable n'est pas fiable ici.


			if(m_max_ln_w.y < m_lines.size()) //Vérification de la validité des indices.
			if(acts_it->line_index >= 0 and acts_it->line_index < m_lines.size())
			{
				m_lines.at(acts_it->line_index).update(m_tab_size);
				// m_lines.at(m_max_ln_w.y).update(m_tab_size);

				unsigned int new_w = m_lines.at(acts_it->line_index).getTextTotalRequiredWidth();
				if(new_w >= m_max_ln_w.x)
				{
					m_max_ln_w.x = new_w;
					m_max_ln_w.y = acts_it->line_index;
				}

			}
		}
		else
		if(acts_it->type == te::REMOVE_LINE)
		{
			if(acts_it->line_index == m_max_ln_w.y) //En cas de retrait de la plus grande ligne.
			if(m_max_ln_w.y < m_lines.size()) //Vérification de la validité de l'indice.
			{
				setMaxLineW();
			}
		}
		else
		if(acts_it->type == te::NEW_TEXT)
		{
			setMaxLineW();
		}

		acts_it++;
	}


}


bool Text_Window::setPtsize(short ptsize)
{
	if(not m_is_init) //Si le 'gs::Text_Window' n'est pas encore initialisé:
	{
		if(ptsize < 1)
			m_common_ptsize = 1;
		else
			m_common_ptsize = ptsize;

		return true;
	}

	//Sinon, c'est que le 'gs::Text_Window' est initialisé (normalement).

	Font* font_ptr = m_common_bank.getFont();

	if(not font_ptr) //Sécurité
	{
		gs::lastError("||Warning|| in 'Text_Window::setPtsize()', don't have a font. ");
		return false;
	}


	if(ptsize < 1)
		m_common_ptsize = 1;
	else
		m_common_ptsize = ptsize;

	//On met également à jour le 'm_lines_delta'.
	font_ptr->setPtsize(m_common_ptsize);
	m_lines_delta = font_ptr->getFontLineSkip();

	//Enfin, reset de toutes les potentielles règles d'affichage précédentes, pour raffraichir avec le nouveau ptsize.
	for(Text_Line& line : m_lines)
		line.clearRules(FONT_STYLE_NORMAL, m_common_ptsize, u32RGBA(m_fg_color), 0u);

	return true;

}

short Text_Window::getPtsize() const {return m_common_ptsize;}



void Text_Window::processSelection(int anchor_line_idx)
{
	try{

	if(anchor_line_idx == te::NO_ACTIVE_LINE)
	{
		//#<A>
		//Cette méthode utilitaire vient après 'mouseClickedAt()' dans l'ordre des appels pour traiter les évènements; et 'mouseClickedAt()' est celle qui gère le click, et met à jour 'm_curr_active_line_idx' en fonction.
		//De ce fait si 'anchor_line_idx' (aka 'm_curr_active_line_idx') ne "pointe" vers aucune ligne, c'est sûrement que le click a été fait dans une zone qui neutralise l'action du 'gs::Text_Window' (notamment si ça ne touche aucune ligne, ni même est dans une zone "amie").
		//Alors on neutralise également la potentielle sélection courante.
		unselect();
		return;
	}

	//'anchor_line_idx' : l'indice de la ligne où s'ancrer pour la sélection.



	if(m_mouse_tracker.isDown())
	{
		//Note: normalement (si j'ai correctement procédé), avec la manière dont les choses sont traitées, arrivé à ce bloc de code, le click n'est pas dans une zone "invalide" à ce traitement (voir les commentaires vers la balise #<A>).

		//A chaque fois que le code arrive ici, on annule la (potentielle) précédente sélection, afin de ne garder que les données le plus à jour possible.
		unselect();


		//Ces positions sont dans l'espace global (dans le renderer).
		Point click_pos = m_mouse_tracker.getPosWhenDown().toPoint();//{m_mouse_tracker.getPosWhenDown().x, m_mouse_tracker.getPosWhenDown().y};
		Vec2i curr_mouse_pos = m_mouse_tracker.getPos().to<int>();



		//float y_scroll = m_y_scroll;
		float y_scroll = m_blocks_folder.adjustYScroll(m_y_scroll, m_lines_delta); //Prise en compte des lignes pliées

		int curr_y = m_pos.y - static_cast<int>(y_scroll)%m_lines_delta; //Position de la première ligne actuelle (la plus haute ligne actuellement visible du gs::Text_Window), en pixel, dans l'espace global (dans le renderer).
		int max_y = m_pos.y + static_cast<int>(m_size.y)-1; //Position, en pixel, de la ligne de pixels la plus basse incluse dans le gs::Text_Window).
		bool anchor_line_found=false;
		//Note: 'curr_y', dans un parcours de haut en bas, représente le haut de la ligne correspodante.


		//Comme la sélection peut se faire de haut en bas comme de bas en haut, on va moduler la boucle en fonction.
		const bool is_up_to_down = (curr_mouse_pos.y - click_pos.y) >= 0;  //'is_up_to_down' peut varier lors d'un même click, entre les différents appels de cette méthode (grosso modo à chaque frame que le click est maintenu) (car la souris peut continuer à bouger avec le click maintenu), cependant lors d'un même passage dans cette méthode la valeur doit rester la même, d'où le 'const' pour prévenir cela.
		int i;


		if(is_up_to_down)
		{
			i = static_cast<int>(y_scroll)/m_lines_delta; //On débute en haut, et la limite c'est le bas.
		}
		else
		{
			//On débute en bas, et la limite c'est le haut.

			for(int j = static_cast<int>(y_scroll)/m_lines_delta; j< m_lines.size(); ++j)
			{
				//Parcours des lignes VISIBLES (de haut en bas) jusqu'au dernier.
				i = j; //Le dernier indice (le dernier 'j') est celui qui sera pris, et donc considéré comme l'indice de la dernière ligne visible actuellement.

				if(not m_blocks_folder.isLineHidden(i))
					curr_y += m_lines_delta; // #<B> //En effet 'curr_y' est incrémenté pour le tour suivant (pour la ligne suivante), or 'i' est affecté avec l'indice de la ligne courante, ce qui à la sortie de la boucle cause un décalage entre l'indice courant et la position (de ligne) courante (qui est celle de la ligne qui est juste en bas). Mais ce décalage est avantageux ici car veut parcourir les lignes de bas en haut, ça facilite les choses (puisqu'ainsi 'curr_y' peut être vu (à 1 pixel près) comme la coo y du bas de la ligne d'indice 'i').

				if(curr_y > max_y)
					break;
			}
		}

		auto isStillLooping = [&]() {
			if(is_up_to_down)
			{
				//Ici la limite c'est la bas.
				return i< m_lines.size() and curr_y <= max_y;
			}
			else
			{
				//Ici la limite c'est le haut.
				return i >= 0 and curr_y >= m_pos.y; //#<C> //Note: lorsque l'on va de bas en haut, on se sert (entre autres) de la coo y du bas des lignes. Voir la ligne avec la balise #<B> un peu plus haut.
			}

			return false;
		};




		while(isStillLooping()) //Note: Il est important dans la boucle de gérer l'avancement de celle-ci, histoire d'éviter une infinite loop, car 'isStillLooping()' ne fait que vérifier l'état.
		{
			if(i==anchor_line_idx) //Si l'on atteint la ligne d'ancrage,
			{
				Rect line_area = m_lines.at(anchor_line_idx).getArea(); //Dans l'espace global (dans le renderer).

				if(pointInRect(&click_pos, &line_area)) //et que le click est bien dessus:
				{
					m_selected_lines.push_back({&m_lines.at(anchor_line_idx), anchor_line_idx, true});
					anchor_line_found = true;
				}
				else
				{
					unselect();
					return;
				}
			}
			else
			if(anchor_line_found)
			{
				bool is_condition_ok=false;

				if(is_up_to_down)
					is_condition_ok = curr_y <= curr_mouse_pos.y;
				else
					is_condition_ok = curr_mouse_pos.y < curr_y; //Si questionnement sur le '<' : voir les lignes (en particulier les commentaires qui vont avec) avec les balises #<B> et #<C> plus haut dans la méthode.

				if(is_condition_ok)
				{
					//Note: il est important de garder une trace de toute les lignes qui sont sélectionnées à un moment ou un autre, pour pouvoir les désélectionner au besoin, et ne pas avoir de "sélection résiduelle". D'où l'intérêt de les rajouter dans 'm_selected_lines' après qu'elles aient subit une sélection.

					if(not m_lines.at(i).isUpToDate())
						m_lines.at(i).update(m_tab_size);

					m_lines.at(i).selectAll(); //Les lignes intermédaires de la sélection (celles qui ne sont pas aux deux extrémités) se voient sélectionnées complètement.

					m_selected_lines.push_back({&m_lines.at(i), i, false});
				}

			}


			//Avancement de la boucle, avec l'orientation gérée en fonction.
			if(is_up_to_down)
			{
				if(not m_blocks_folder.isLineHidden(i))
					curr_y += m_lines_delta;

				++i;
			}
			else
			{
				if(not m_blocks_folder.isLineHidden(i))
					curr_y -= m_lines_delta;

				--i;
			}
		}


		//Traitement après la boucle.
		//Note: avec l'usage de 'unselect();' au début, les données de 'm_selected_lines' sont celles qui viennent (ou pas) d'être rajoutées.

		if(m_selected_lines.size() == 1) //S'il n'y a qu'une seule ligne de sélection, on la sélectionne de manière "précise".
		{
			Line_In_Selection& l_i_s = m_selected_lines.front();

			if(l_i_s.ptr and l_i_s.is_anchor) //Sécurité/vérification
			{
				Rect line_area = l_i_s.ptr->getArea(); //Dans l'espace global (dans le renderer).

				if(not l_i_s.ptr->isUpToDate())
					l_i_s.ptr->update(m_tab_size);

				l_i_s.ptr->selectTextPortionByPixelsPos(click_pos.x -line_area.x +m_x_scroll, curr_mouse_pos.x -line_area.x +m_x_scroll); // 'click_pos', 'curr_mouse_pos' et 'line_area' sont toutes "trois" dans l'espace global (dans le rendu) et sont utilisées pour déterminer les positions des pixels relativement à la ligne (dans l'espace de la ligne). Enfin on ajoute le décalage du scrolling horizontal dans l'espace de la ligne.

				//Si la sélection n'est pas validée.
				if(not l_i_s.ptr->haveSelection())
					m_selected_lines.clear(); //On retire cette ligne de la liste des lignes sélectionnées pour éviter des malentendus.
			}
		}
		else if(m_selected_lines.size() > 1) //S'il y a plus d'une ligne concernée par la sélection, on applique à la première et la dernière ligne sélectionnées les sélections qui leurs sont appropriées en fonction.
		{
			//Note: pour les lignes intermédaires, le traitement a été préalablement effectué avant l'ajout. Ainsi on évite d'avoir une boucle en plus.

			Line_In_Selection& first_l_i_s = m_selected_lines.front();
			Line_In_Selection& last_l_i_s = m_selected_lines.back();


			bool is_it_from_begin = !is_up_to_down; //Pour la première ligne, si l'on descend, la sélection commence de la position du click jusqu'à la fin de la ligne (autrement vu "depuis la fin de la ligne jusqu'à la position du click"), et lorsque l'on monte c'est depuis la position du click jusqu'au début de la ligne (autrement vu "depuis le début jusqu'à la position du click").

			if(first_l_i_s.ptr and first_l_i_s.is_anchor) //Sécurité/vérification
			{
				Rect line_area = first_l_i_s.ptr->getArea(); //Dans l'espace global (dans le renderer).

				if(not first_l_i_s.ptr->isUpToDate())
					first_l_i_s.ptr->update(m_tab_size);

				first_l_i_s.ptr->selectTextPortionByPixelsPos_From(is_it_from_begin, click_pos.x-line_area.x +m_x_scroll);
			}



			is_it_from_begin = is_up_to_down; //Pour la dernière ligne, contrairement à la première, lorsque l'on descend, on sélectionne la ligne depuis le début jusqu'à la dernière position du curseur lors du traitement (voire jusqu'à la fin de la ligne si celle-ci est plus courte que cette position de curseur). Et quand on monte, on sélectionne depuis la fin de la ligne jusqu'à la dernière position du curseur lors du traitement (si possible).

			if(last_l_i_s.ptr and not last_l_i_s.is_anchor) //Sécurité/vérification
			{
				Rect line_area = last_l_i_s.ptr->getArea(); //Dans l'espace global (dans le renderer).

				if(not last_l_i_s.ptr->isUpToDate())
					last_l_i_s.ptr->update(m_tab_size);

				last_l_i_s.ptr->selectTextPortionByPixelsPos_From(is_it_from_begin, curr_mouse_pos.x-line_area.x +m_x_scroll);
			}


			//En cas de sélection-s non validée-s, on retire les lignes concernées de la liste des lignes sélectionnées pour éviter des malentendus.
			if(not last_l_i_s.ptr->haveSelection())
				m_selected_lines.pop_back();

			if(not first_l_i_s.ptr->haveSelection())
				m_selected_lines.erase(m_selected_lines.begin());


			//En somme la "première" et "dernière" lignes dépendent du sens de la sélection (la première peut être celle en haut et la dernière celle en bas et vice versa). En tout cas, peu importe qui est qui, grosso modo celle qui est en haut est traitée comme celle qui est en haut, et pareillement pour celle qui est en bas.
		}

		m_selection_is_up_to_down = is_up_to_down;
	}


	}
	catch(std::exception& err){
		gs::lastError(std::string("!! excep from 'Text_Window::processSelection()' : ")+gs::lastError());
	}

}




bool Text_Window::setASelectedLine(Text_Line& line, int index, bool is_anchor)
{
	if(line.haveSelection() and (0<= index and index < m_lines.size()))
	{
		m_selected_lines.push_back({&line, index, is_anchor});
		return true;
	}

	return false;
}



bool Text_Window::selectAll()
{
	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::selectAll()', not yet init. Use 'Text_Window::initBy...()' before. ");
		return false;
	}

	unselect();

	bool full_success = true;
	m_selection_is_up_to_down = true; //Afin que la copie des lignes puisse se faire dans le bon ordre.

	for(int i=0; i<m_lines.size(); ++i)
	{
		if(not m_lines.at(i).isUpToDate())
			m_lines.at(i).update(m_tab_size);

		if(m_lines.at(i).selectAll())
			m_selected_lines.push_back({&m_lines.at(i), i, false});
		else
		{
			gs::lastError(std::string("||Warning|| in 'Text_Window::selectAll()', couldn't select a line [line index : ")+std::to_string(i)+ "]. ");
			full_success = false;
		}
	}

	return full_success;
}



void Text_Window::unselect()
{
	for(Line_In_Selection& sel_line : m_selected_lines)
	{
		if(checkLineInSelValidity(sel_line))
			sel_line.ptr->unselectTextPortion();
	}

	m_selected_lines.clear();
}




bool Text_Window::haveSelection() const
{
	return m_selected_lines.size() > 0u;
}



std::string Text_Window::getSelection() const
{
	if(not haveSelection())
		return std::string();

	std::string text_cumul;

	//Gestion du sens de la boucle en fonction du sens de la sélection. Ceci est afin de renvoyer le texte sélectionné de haut en bas, même si la sélection part du bas vers le haut.
	const bool is_up_to_down = m_selection_is_up_to_down;
	int i;
	if(is_up_to_down)
		i = 0;
	else
		i = static_cast<int>(m_selected_lines.size())-1;

	auto isStillLooping = [&]() {
		if(is_up_to_down)
			return i < m_selected_lines.size();
		else
			return i >= 0;

		return false;
	};

	while(isStillLooping()) //Il est important dans la boucle de gérer l'avancement de celle-ci, histoire d'éviter une infinite loop, car 'isStillLooping()' ne fait que vérifier l'état.
	{
		const Line_In_Selection& sel_line = m_selected_lines.at(i);

		if(checkLineInSelValidity(sel_line))
			text_cumul += sel_line.ptr->getSelectedText();

		//Avancement de la boucle, avec l'orientation gérée en fonction.
		if(is_up_to_down)
			++i;
		else
			--i;
	}

	return text_cumul;
}



bool Text_Window::checkLineInSelValidity(const Line_In_Selection& l_i_s) const
{
	return (
				l_i_s.ptr != nullptr
				and (0 <= l_i_s.index and l_i_s.index < m_lines.size())
				and (l_i_s.ptr == &m_lines.at(l_i_s.index))
			); //Sécurité, notamment pour prévenir les déplacements/modifications qui se seraient fait dans le 'std::vector' qui contient le lignes.

}



bool Text_Window::eraseSelection(bool record_this_action)
{
	auto recordEraseAction = [&](Line_In_Selection& sel_line, int action_idx) {
		if(not record_this_action) return;

		Selection_Act_Record s_a_r = sel_line.ptr->getLastRecordedSelectionAct();
		if(not s_a_r.it_is_for_replacement)
			pushAction(te::ERASE_LINE_SELECTION, sel_line.index, s_a_r.origin_str, s_a_r.end_cursor_pos, s_a_r.begin_cursor_pos, s_a_r.selection_bounds).action_idx = action_idx;

		m_blocks_folder.takeEdit(te::ERASE_LINE_SELECTION, sel_line.index, sel_line.ptr, m_tab_size);
	};


	try{

	if(not haveSelection())
		return false;

	if(m_selected_lines.size() == 1) //S'il n'y a qu'une ligne concernée dans la sélection.
	{
		Line_In_Selection& sel_line = m_selected_lines.front();

		if(checkLineInSelValidity(sel_line))
		{
			if(not sel_line.ptr->eraseSelection())
				return false; //Si la suppression n'a pas marchée.

			//Sinon, on retire la ligne de la liste des sélectionnées.
			m_selected_lines.clear(); //Note: dans ce bloc on a/avait 'm_selected_lines.size() == 1'.

			recordEraseAction(sel_line, 0);
		}
		else { unselect(); return false; }
	}
	else
	if(m_selected_lines.size() > 1) //Sécurité. Je pense qu'en général là où je ne mets que l'indication "Sécurité", c'est souvent non "strictement nécessaire", c'est pour sécuriser "au cas où".
	{
		int curr_action_idx = 0; //Pour indiquer la position des différents actions dans cette chaîne d'action.


		//Voici ce que j'ai pu observer : quand on supprime une sélection multiligne (et "linéaire"(sans trou)), seule la ligne la plus en haut "survie", en gardant son contenu non sélectionné, auquel on y ajoute le contenu non sélectionné de la ligne la plus en bas.

		int highest_sel_line_idx; //Indices pour localiser dans le conteneur 'm_selected_lines'.
		int lowest_sel_line_idx;

		if(m_selection_is_up_to_down) //Si la sélection est faite de haut en bas, le plus haut c'est le premier de la liste, et le plus bas le dernier.
		{
			highest_sel_line_idx = 0;
			lowest_sel_line_idx = static_cast<int>(m_selected_lines.size())-1;
		}
		else //Sinon, si la sélection est faite de bas en haut, le plus en haut est le dernier de la liste, le plus bas le premier.
		{
			lowest_sel_line_idx = 0;
			highest_sel_line_idx = static_cast<int>(m_selected_lines.size())-1;
		}

		Line_In_Selection& highest_sel_line = m_selected_lines.at(highest_sel_line_idx);
		Line_In_Selection& lowest_sel_line = m_selected_lines.at(lowest_sel_line_idx);


		//On supprime la partie sélectionnée de la ligne haute.
		if(checkLineInSelValidity(highest_sel_line))
		{
			//Si possible, on commence par produire une action "factice" du curseur, pour pemettre à ce que s'il y a une annulation de cette suppression, que le curseur termine là où il a commencé.
			if(record_this_action and m_curr_active_line_idx != te::NO_ACTIVE_LINE)
			{
				int cursor_pos = m_lines.at(m_curr_active_line_idx).getCursorPos();

				Text_Win_Action starting_act;
					starting_act.type = te::NEW_CURSOR_POS;
					starting_act.line_index = m_curr_active_line_idx;
					starting_act.new_cursor_pos = cursor_pos;

				act(starting_act, true); // important de mettre le 'record_this_action' de 'act' à 'true'.
				++curr_action_idx;
			}

			if(not highest_sel_line.ptr->eraseSelection())
				return false;

			recordEraseAction(highest_sel_line, curr_action_idx); ++curr_action_idx;


		} else { unselect(); return false; }



		//On supprime la partie sélectionnée de la ligne basse.
		if(checkLineInSelValidity(lowest_sel_line))
		{
			if(not lowest_sel_line.ptr->eraseSelection())
			{
				unselect(); return true; //On return avec 'true' car on a déjà apporté une/des modification-s. Ainsi on évite de potentiels "malentendus" vers la/les zone-s d'appel de cette méthode.
			}

			recordEraseAction(lowest_sel_line, curr_action_idx); ++curr_action_idx;
		}
		else { unselect(); return true; } //On return avec 'true' car on a déjà apporté une/des modification-s. Ainsi on évite de potentiels "malentendus" vers la/les zone-s d'appel de cette méthode.


		//On rajoute à la ligne haute, le reste de la ligne basse.
		//Note: (normalement) après un 'gs::Text_Line::eraseSelection()' réussi, le curseur se trouve à la bonne position, ce qui facilite l'ajout de texte au niveau du texte supprimé.
		highest_sel_line.ptr->update(m_tab_size);

		int prv_cursor_pos = highest_sel_line.ptr->getCursorPos();

		highest_sel_line.ptr->addText(lowest_sel_line.ptr->getText(), nullptr, true); //#<A>  'true' pour 'allow_cursor_to_be_moved' afin de laisser bouger le curseur après cet ajout, cela aidera à l'enregistrement de l'action.
		int then_cursor_pos = highest_sel_line.ptr->getCursorPos();
		highest_sel_line.ptr->update(m_tab_size);

		if(record_this_action) {pushAction(te::INSERT_CHAR, highest_sel_line.index, lowest_sel_line.ptr->getText(), then_cursor_pos, prv_cursor_pos).action_idx = curr_action_idx; ++curr_action_idx;} //Ici, j'estime que ce n'est pas nécessaire de mentionner '.str_2' (en vu d'un 'cursor_to_end_part') , vu que '*highest_sel_line.ptr' a précédemment (normalement) subit un effacement de sélection, sélection qui allait d'un x position jusqu'à la fin, et qui a mis de ce fait le curseur à la fin de la ligne restante.
		m_blocks_folder.takeEdit(te::INSERT_CHAR, highest_sel_line.index, highest_sel_line.ptr, m_tab_size);

		highest_sel_line.ptr->setCursorPos(prv_cursor_pos); //#<B> cependant, pour aller dans le sens de ce que doit faire cette implémentation, on remet le curseur au niveau où c'était avant l'ajout.
		if(record_this_action) {pushAction(te::NEW_CURSOR_POS, highest_sel_line.index, "", highest_sel_line.ptr->getCursorPos(), then_cursor_pos, {-1,-1}, highest_sel_line.index).action_idx = curr_action_idx; ++curr_action_idx;} //Et on renseigne également ce déplacement du curseur.



		//Maintenant, suppression des lignes intermédaires et celle qui est la plus basse (mais pas la plus haute).
		{
			std::vector<int> lines_to_remove__indices;

			//Récupération des indices de celle-ci.
			for(Line_In_Selection& sel_line : m_selected_lines)
				if(checkLineInSelValidity(sel_line) and sel_line.index != highest_sel_line.index)
					lines_to_remove__indices.push_back(sel_line.index);


			unselect();

			//Classement des indices par ordre décroissant (car supprimer des derniers aux premiers permet de ne pas créer de décalage d'indice sur les éléments qui sont en train d'être retirés (telle une pile d'objet), ça facilite aussi le 'undo').
			std::sort(lines_to_remove__indices.begin(), lines_to_remove__indices.end(), [](int a, int b){return a > b;});

			// ! Important : avant de retirer les lignes a retirer, on s'assure que le ligne active (par 'm_curr_active_line_idx') est bien celle qui est la plus en haut (elle n'est pas concernée par cette suppression).
			//Note: 'm_curr_active_line_idx' indique la ligne sur laquelle apparait le curseur, la ligne qui prend en compte l'édition courante (généralement au travers du clavier). Ce n'est pas "directement" lié au "surlignement".
			setCurrLineOfEdit(highest_sel_line.index); //Et en ce qui concerne la position du curseur sur cette ligne, (normalement) c'est déjà géré : voir les lignes avec les balises #<A> et #<B> un peu plus haut dans la méthode (vers "highest_sel_line.ptr->addText").


			for(int i : lines_to_remove__indices)
			{
				m_for_removeLine__action_idx = curr_action_idx;
				removeLine(i, nullptr, record_this_action);

				++curr_action_idx;
			}

			if(record_this_action)
			{
				Text_Win_Action& last_act = pushAction(te::INFO, -1, "");
				last_act.action_idx = curr_action_idx; ++curr_action_idx;
				last_act.info = te::END_OF_ERASE_MULTILINES_SELECTION;
			}
		}

		m_for_replaceSelection_from_eraseSelection__curr_action_idx = curr_action_idx; //Si 'replaceSelection()' fait appel à 'eraseSelection()', ceci permet de lui faire parvenir la dernière valeur de 'curr_action_idx' afin de pouvoir l'exploiter.

		//A ce stade du code, d'après les observations à l'exécution, la ligne active (par 'm_curr_active_line_idx') est bien la bonne, avec le curseur au bon endroit,
		//soit au niveau de la zone du texte qui vient d'être effacé ; autrement dit devant le caractère juste à gauche de la sélection et derrière le caractère juste à droite de la sélection.
	}


	return true;

	}
	catch(std::exception& err){
		gs::lastError(std::string("!! excep from 'Text_Window::eraseSelection()' : ")+err.what());
	}

	return false; //En cas d'exception.
}



bool Text_Window::replaceSelection(const std::string& str_to_place, bool record_this_action)
{
	try{

	if(not haveSelection())
		return false;

	//Cas à part: entrée d'une tabulation sur une sélection (avec plusieurs lignes) => application d'une tabulation au début de toute les lignes sélectionnées au lieu de faire une remplacement de texte.
	if(str_to_place == "\t" and m_selected_lines.size() > 1)
		return processTabOnSelection(record_this_action);


	//Ici aussi deux cas de figure: une seule ligne concernée, ou, plusieurs lignes concernées.

	std::string returned_part;
	int line_of_the_text_insert__idx = te::NO_ACTIVE_LINE;
	int curr_action_idx=0;
	bool was_one_line_selection;

	if(m_selected_lines.size() == 1) //Si seule une ligne est concernée par la sélection.
	{
		Line_In_Selection& sel_line = m_selected_lines.front();

		if(checkLineInSelValidity(sel_line))
		{
			if(not sel_line.ptr->replaceSelection(str_to_place, &returned_part))
				return false; //Si le remplacement n'a pas marchée.

			line_of_the_text_insert__idx = sel_line.index;

			//Sinon, on retire la ligne de la liste des sélectionnées.
			m_selected_lines.clear(); //Note: dans ce bloc on a/avait 'm_selected_lines.size() == 1'.

			if(record_this_action)
			{
				// #<A>
				Selection_Act_Record s_a_r = sel_line.ptr->getLastRecordedSelectionAct();
				if(s_a_r.it_is_for_replacement)
					pushAction(te::REPLACE_LINE_SELECTION, sel_line.index, s_a_r.origin_str, s_a_r.end_cursor_pos, s_a_r.begin_cursor_pos, s_a_r.selection_bounds, sel_line.index, s_a_r.new_str).action_idx = curr_action_idx;

				++curr_action_idx;

				m_blocks_folder.takeEdit(te::REPLACE_LINE_SELECTION, sel_line.index, sel_line.ptr, m_tab_size);
			}

			was_one_line_selection = true;
		}
		else { unselect(); return false; }

	}
	else if(m_selected_lines.size() > 1) //Si plusieurs lignes sont concernées par la sélection.
	{
		//On commence d'abord par supprimer le contenu de la sélection.
		if(not this->eraseSelection(record_this_action))
			return false;

		curr_action_idx = m_for_replaceSelection_from_eraseSelection__curr_action_idx;


		//Puis on insert ensuite le texte de remplacement.
		//Note: on considère qu'à ce stade (après avoir appelé 'this->eraseSelection()') le curseur est là où on l'attend (au niveau de la zone du texte qui vient d'être effacé ; autrement dit devant le caractère juste à gauche de la sélection et derrière le caractère juste à droite de la sélection).

		if(m_curr_active_line_idx == te::NO_ACTIVE_LINE) //On sécurise tout de même.
			return true; //On return avec 'true' car on a déjà apporté une/des modification-s. Ainsi on évite de potentiels "malentendus" vers la/les zone-s d'appel de cette méthode.


		m_lines.at(m_curr_active_line_idx).update(m_tab_size);
		int prv_cursor_pos = m_lines.at(m_curr_active_line_idx).getCursorPos();


		std::string cursor_to_end_part; //Pour l'enregistrement de l'action. Ceci permet d'avoir l'état de la ligne du curseur à la fin, au cas où il y aurait un caractère de fin ligne dans 'str_to_place', ce qui causerait (généralement) un rejet d'une partie de la ligne.
		if(str_to_place.contains(m_lines.at(m_curr_active_line_idx).getEndLineChar()))
			cursor_to_end_part = m_lines.at(m_curr_active_line_idx).getTextFromCursorToEnd();


		if(not m_lines.at(m_curr_active_line_idx).addText(str_to_place, &returned_part, true))
			return true; //On return avec 'true' car on a déjà apporté une/des modification-s. Ainsi on évite de potentiels "malentendus" vers la/les zone-s d'appel de cette méthode.

		m_lines.at(m_curr_active_line_idx).update(m_tab_size);


		int then_cursor_pos = m_lines.at(m_curr_active_line_idx).getCursorPos();
		if(record_this_action) {pushAction(te::INSERT_CHAR, m_curr_active_line_idx, str_to_place,
											then_cursor_pos, prv_cursor_pos, {-1,-1}, m_curr_active_line_idx, cursor_to_end_part).action_idx = curr_action_idx; ++curr_action_idx;}

		m_blocks_folder.takeEdit(te::INSERT_CHAR, m_curr_active_line_idx, &m_lines.at(m_curr_active_line_idx), m_tab_size);


		line_of_the_text_insert__idx = m_curr_active_line_idx;
		was_one_line_selection = false;
	}


	//Traitement du débordement potentiel.
	int cursor_pos_of_the_latest_compl_ln_=getUTF8CharsCount(getLastTextChunk(str_to_place, m_lines.at(line_of_the_text_insert__idx).getEndLineChar()));
	if(line_of_the_text_insert__idx != te::NO_ACTIVE_LINE)
		completeTextInsertion(line_of_the_text_insert__idx+1, returned_part, record_this_action, curr_action_idx, cursor_pos_of_the_latest_compl_ln_);


	//Identification de l'action en globale :
	if(was_one_line_selection and returned_part.length() == 0) //S'il n'y a eu qu'un remplacement sur une seule ligne, et sans engendrer d'autres lignes, on considère l'action unique 'REPLACE_LINE_SELECTION'.
	{
		//Déjà traité : voir le code vers la balise #<A>.
	}
	else //Sinon, c'est que plusieurs lignes furent concernées, à la sélection, ou au remplacement, voire les deux.
	if(record_this_action)
	{
		Text_Win_Action& last_act = pushAction(te::INFO, -1, "");
		last_act.action_idx = curr_action_idx; ++curr_action_idx;
		last_act.info = te::END_OF_REPLACE_SELECTION_MULTIACTIONS;
	}

	return true;

	}
	catch(std::exception& err){
		gs::lastError(std::string("!! excep from 'Text_Window::replaceSelection()' : ")+err.what());
	}



	return false; //En cas d'exception.
}





bool Text_Window::processTabOnSelection(bool record_this_action)
{
	try{

	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::processTabOnSelection()', not yet init. Use 'Text_Window::initBy...()' before. ");
		return false;
	}


	//Parcours de toutes les lignes sélectionnées, afin de mettre une tabulation au début de celles-ci.

	//Première étape: vérifier la validité de TOUTE la sélection avant de commencer.
	for(Line_In_Selection& sel_line : m_selected_lines)
	{
		if(not checkLineInSelValidity(sel_line))
		{
			unselect();
			return false;
		}
	}



	//Deuxième étape: mettre les tabulations, et renouveler la sélection (par une sélection totale de ces lignes (même pour les deux lignes aux extrêmes de la sélection).
	std::string tab_str = "\t";

	int curr_action_idx=0;

	for(Line_In_Selection& sel_line : m_selected_lines)
	{
		if(not sel_line.ptr) //Sécurité
		{
			gs::lastError("||Warning|| in 'Text_Window::processTabOnSelection()', line ptr error. ");
			unselect();
			return false;
		}

		std::string&& line__prev_text = sel_line.ptr->getText();
		int prev_cur_pos = sel_line.ptr->getCursorPos();

		sel_line.ptr->setText(tab_str+line__prev_text); //Rajout de la tabulation au début du texte (de plus la ligne retire sa sélection (interne) du fait de l'usage de '.setText()').
		sel_line.ptr->update(m_tab_size);
		sel_line.ptr->selectAll(); //Remise de la sélection.

		if(record_this_action)
		{
			pushAction(te::LINE_NEW_TEXT, sel_line.index, sel_line.ptr->getText(),
						sel_line.ptr->getCursorPos(), prev_cur_pos, {-1,-1},
						sel_line.index, line__prev_text).action_idx = curr_action_idx;

			++curr_action_idx;
		}
	}


	if(record_this_action)
	{
		Text_Win_Action& last_act = pushAction(te::INFO, -1, "");
		last_act.action_idx = curr_action_idx; ++curr_action_idx;
		last_act.info = te::END_OF_REPLACE_SELECTION_MULTIACTIONS;
	}

	return true;


	}
	catch(std::exception& err){
		gs::lastError(std::string("!! excep from 'Text_Window::processTabOnSelection()' : ")+err.what());
	}

	return false; //En cas d'exception.
}





bool Text_Window::processBackTab(bool record_this_action)
{
	try{

	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::processBackTab()', not yet init. Use 'Text_Window::initBy...()' before. ");
		return false;
	}



	auto processBackTabOnOneLine = [this, record_this_action](int line_idx, int& curr_action_idx){

		if(not (0<=line_idx and line_idx < m_lines.size())) //La sécurité ^U^
			return false;


		std::string&& from_begin_to_cursor__text = m_lines.at(line_idx).getTextFromBeginToCursor();

		if(from_begin_to_cursor__text.empty())
			return true;

		bool before_cur_is_empty = true; //Détermine s'il y a au moins un caractère (en dehors des espaces et tabulations) entre le début de la ligne et le curseur.

		for(int i=static_cast<int>(from_begin_to_cursor__text.size())-1; i>=0; --i) //Parcours à rebours pour optimiser la recherche (car j'estime qu'en général il y a plus de chance de trouver plus tôt un caractère en partant du curseur plutôt que du début).
		{
			if(from_begin_to_cursor__text.at(i) != ' ' and from_begin_to_cursor__text.at(i) != '\t')
			{
				before_cur_is_empty = false; //A l'initialisation de 'before_cur_is_empty' on considère que c'est bien "vide", mais dès que l'on rencontre un caractère autre que espace ou tabulation c'est que ce n'est pas "vide".
				break;
			}
		}

		int cursor_pos =  m_lines.at(line_idx).getCursorPos();

		if(not before_cur_is_empty and cursor_pos > 0) //Lorsqu'il y a des caractères (autre que ' ' et '\t') à gauche du curseur, voici le comportement du back tab:
			moveCursor(-m_tab_size, 0); //Déplacement du curseur vers la gauche (jusqu'à 'm_tab_size' caractères si possible).

		else
		if(before_cur_is_empty and cursor_pos > 0)
		{
			//Retrait stratégique du "vide" qui se situe juste avant le curseur.
			//En effet, si 'before_cur_is_empty' est à 'true' c'est que 'from_begin_to_cursor__text' n'est constitué que des ' ' et/ou des '\t'.

			//On part du plus proche du curseur (soit la fin de 'from_begin_to_cursor__text').
			//S'il y a immédiatement une tabulation, on la retire, et c'est bon.
			//Sinon s'il y a un espace, on comptabilise jusqu'à 'm_tab_size' espaces (contiguës) à retirer. Si une tabulation s'interpose avant la fin du compte, seuls les espaces comptabilisés seront retirés, afin d'essayer de garder un alignement.

			int chars_to_remove__count = 0;

			for(int i=static_cast<int>(from_begin_to_cursor__text.size())-1; i>=0; --i)
			{
				if(from_begin_to_cursor__text.at(i) == '\t')
				{
					if(chars_to_remove__count == 0)
						chars_to_remove__count=1;

					break;
				}

				++chars_to_remove__count;

				if(chars_to_remove__count >= m_tab_size)
					break;
			}

			if(chars_to_remove__count > 0)
			{
				std::string rvmd = m_lines.at(line_idx).removeFromCursorLeftSide(chars_to_remove__count); //Relatif à la position du curseur.
				m_lines.at(line_idx).update(m_tab_size);

				if(record_this_action)
					pushAction(te::REMOVE_CHAR, line_idx, rvmd,
								m_lines.at(line_idx).getCursorPos(),
								cursor_pos).action_idx = curr_action_idx;

				++curr_action_idx;

				m_blocks_folder.takeEdit(te::REMOVE_CHAR, line_idx, &m_lines.at(line_idx), m_tab_size);
			}
		}

		return true;
	};



	int curr_action_idx=0;

	//Back Tab lorsqu'il n'y a pas de sélection. ("Pas de texte surligné").
	if(not haveSelection() and m_curr_active_line_idx != te::NO_ACTIVE_LINE)
	{
		return processBackTabOnOneLine(m_curr_active_line_idx, curr_action_idx);
	}
	else
	if(haveSelection()) //Back Tab lorsqu'il y a une sélection. ("Présence de texte surligné").
	{
		//Objectif : parcours de toutes les lignes sélectionnées, afin de mettre une back tabulation pour retirer du "vide" en début de ligne si possible.

		//Première étape: vérifier la validité de TOUTE la sélection avant de commencer.
		for(Line_In_Selection& sel_line : m_selected_lines)
		{
			if(not checkLineInSelValidity(sel_line))
			{
				unselect();
				return false;
			}
		}

		//Deuxième étape, mettre les curseurs de ces ligne au niveau où le texte commence [en partant du début de ligne] (car on va utiliser 'processBackTabOnOneLine()' et l'on ne veut pas de déplacement de curseur mais une retrait de "vide").
		//Troisième étape, appliquer 'processBackTabOnOneLine()' pour chacune.
		//Quatrième étape, actualiser la sélection : toutes ces lignes vont être (re)sélectionnées en entier.
		//Ces trois dernières étapes peuvent se faire en une seule boucle sur les lignes sélectionnées.
		for(Line_In_Selection& sel_line : m_selected_lines)
		{
			//Deuxième étape. =======================================================
			int max_cursor_pos = sel_line.ptr->getMaxCursorPos();
			int text_begin__cursor_pos = 0; //Sert à récupérer la position de curseur où le "vrai" texte commence (sans les ' ' et '\t').
			int anti_infinite_loop = 40000;

			do
			{
				if(text_begin__cursor_pos >= max_cursor_pos) //Je mets ici '>=' et non uniquement '>' car si on a 'text_begin__cursor_pos' qui est égal à 'max_cursor_pos', peu importe l'issue de ce qui suit dans la boucle, le "vrai" texte ne pourra pas commencer plus loin que 'max_cursor_pos'.
					break;

				//Voici le principe: parcourir les caractères de la ligne depuis le début, jusqu'à débusquer le premier caractère qui n'est pas ' ' ou '\t'.
				//Cela premettra de déterminer la position de curseur recherchée.

				std::string curr_char = sel_line.ptr->getChar(text_begin__cursor_pos);

				if(curr_char != " " and curr_char != "\t")
					break;

				//Sinon:
				text_begin__cursor_pos++;


				//Sécurité
				anti_infinite_loop--;
				if(anti_infinite_loop < 0)
				{
					std::cout << "||Warning|| in 'Text_Window::processBackTab()', [on a selection], anti_infinite_loop condition reached. \n";
					break;
				}

			}while(true);

			sel_line.ptr->setCursorPos(text_begin__cursor_pos);


			//Troisième et quatrième étapes. =======================================================
			processBackTabOnOneLine(sel_line.index, curr_action_idx);
			sel_line.ptr->selectAll();
		}


		if(record_this_action and curr_action_idx > 0) //En effet, on enregistre la fin d'une chaîne d'actions s'il y'en a eu au moins une (car ici, il se peut qu'aucune des lignes sélectionnées n'a pu être impactée par le back tab).
		{
			Text_Win_Action& last_act = pushAction(te::INFO, -1, "");
			last_act.action_idx = curr_action_idx; ++curr_action_idx;
			last_act.info = te::END_OF_REPLACE_SELECTION_MULTIACTIONS;
		}

	}



	}
	catch(std::exception& err){gs::lastError(std::string("!! excep from 'Text_Window::processBackTab()' : ")+err.what());}

	return false; //En cas d'exception.
}




bool Text_Window::userResponsabilityMethode__replace_inCurrLineOfEdit(int left_cursor_pos, int right_cursor_pos, const std::string& str, std::string* out, bool update_the_line)
{
	if(m_curr_active_line_idx == te::NO_ACTIVE_LINE)
		return false;

	bool success = m_lines.at(m_curr_active_line_idx).replace(left_cursor_pos, right_cursor_pos, str, out);

	if(success)
		m_blocks_folder.takeEdit(te::REPLACE_LINE_SELECTION, m_curr_active_line_idx, &m_lines.at(m_curr_active_line_idx), m_tab_size);

	if(update_the_line)
		m_lines.at(m_curr_active_line_idx).update(m_tab_size);

	return success;
}




std::string Text_Window::copy(bool cut)
{
	//return : si sélection renvoie sélection, sinon si ligne active renvoie ligne active, sinon chaîne vide.  Si erreur renvoie chaîne vide.

	std::string cut_token_val = (cut ? "true" : "false");

	try{

	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::copy(cut==", cut_token_val+")', not yet init. Use 'Text_Window::initBy...()' before. ");
		return std::string();
	}

	if(haveSelection())
	{
		std::string the_copy = getSelection();

		if(cut)
			eraseSelection(true);

		return the_copy;
	}

	if(m_curr_active_line_idx != te::NO_ACTIVE_LINE)
	{
		std::string the_copy = m_lines.at(m_curr_active_line_idx).getText();

		if(cut)
			removeLine(m_curr_active_line_idx, nullptr, true);

		return the_copy;
	}

	}
	catch(std::exception& excep) {gs::lastError("!! excep in 'Text_Window::copy(cut==", cut_token_val+")' : ", excep.what());}


	return std::string();
}




std::string Text_Window::cut()
{
	return this->copy(true);
}




bool Text_Window::past(const std::string& str)
{
	//Comportement (si je ne m'abuse) : si sélection remplace la sélection, sinon si ligne active insert au niveau du curseur de la ligne active.

	try{

	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::past()', not yet init. Use 'Text_Window::initBy...()' before. ");
		return false;
	}

	bool success = insertText(str, true, true);

	if(not success)
		gs::lastError("!!_ from 'Text_Window::past()' : ", gs::lastError());


	return success;

	}
	catch(std::exception& excep) {gs::lastError("!! excep in 'Text_Window::past()' : ", excep.what());}

	return false;
}





bool Text_Window::inverseCodeBlockFoldedState(int block_header_line_idx)
{
	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::inverseCodeBlockFoldedState()', not yet init. Use 'Text_Window::initBy...()' before. ");
		return false;
	}

	m_blocks_folder.setCodeBlockFoldedState(block_header_line_idx, true);

	return true;
}



bool Text_Window::foldAll(bool fold_all, int otherwise__target_depth)
{
	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::foldAll()', not yet init. Use 'Text_Window::initBy...()' before. ");
		return false;
	}

	if(fold_all)
		m_blocks_folder.setAllBlocksWithSameState(true, Blocks_Folder::INVALID_DEPTH);
	else
		m_blocks_folder.setAllBlocksWithSameState(true, otherwise__target_depth);

	return true;
}



bool Text_Window::unfoldAll(bool unfold_all, int otherwise__target_depth)
{
	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::unfoldAll()', not yet init. Use 'Text_Window::initBy...()' before. ");
		return false;
	}

	if(unfold_all)
		m_blocks_folder.setAllBlocksWithSameState(false, Blocks_Folder::INVALID_DEPTH);
	else
		m_blocks_folder.setAllBlocksWithSameState(false, otherwise__target_depth);

	return true;
}



int Text_Window::getFoldedLinesCount()
{
	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Text_Window::getFoldedLinesCount()', not yet init. Use 'Text_Window::initBy...()' before. ");
		return 0;
	}

	return m_blocks_folder.getHiddenLinesCount(-1);
}



}//namespace gs.
