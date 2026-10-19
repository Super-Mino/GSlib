#include "backends/SDL3/GSlib_SDL3_backend.hpp"
#include "GSlib.hpp"
#include "GSlib-Geo.hpp"
#include "GSlib-Text.hpp"
#include "GSlib-Widget.hpp"
#include <iostream>
#include <vector>
#include <string>

#define STB_IMAGE_IMPLEMENTATION
#define STBI_NO_FAILURE_STRINGS
#define STBI_FAILURE_USERMSG
#include "backends/SDL3/GSlib_stb_image.h"

namespace gs
{

void lastError(const std::string& str_info, const std::string& str_info_2, const std::string& str_info_3);
bool get_show_comments();
void addToClearAtTheEnd(Texture* tex);
void addToClearAtTheEnd(Renderer* renderer);


SDL_Color _prev_ren_draw_col{0u,0u,0u,255u};
SDL_BlendMode _prev_ren_blend_mode=SDL_BLENDMODE_BLEND;

std::vector<std::string> paths_loaded_with_gsloadTexture{};

//====================================================================//



Vec2f getMousePos(float* x, float* y)
{
	Vec2f pos;
	SDL_GetMouseState(&pos.x, &pos.y);
	if(x) *x=pos.x;
	if(y) *y=pos.y;
	return pos;
}


void drawBegin(Renderer* ren, bool clear) {if(clear) SDL_RenderClear(ren);}

void drawEnd(Renderer* ren) {SDL_RenderPresent(ren);}



SDL_Texture* loadTexture(SDL_Renderer* renderer, const std::string& path, bool nearest)
{
	if(not renderer)
	{
		gs::lastError("||Warning|| in 'loadTexture()', [path: ", path+"], invalid renderer");
		return nullptr;
	}

	stbi_convert_iphone_png_to_rgb(1);

	//Chargement des pixels à l'aide de stb
	Vec2i size;
	int channels_in_file;
	int desired_channels = 4; //Pour avoir un RGBA
	uint8_t* pixels = stbi_load(path.c_str(), &size.x, &size.y, &channels_in_file, desired_channels);

	if(not pixels)
	{
		gs::lastError("||Warning|| in 'loadTexture()', [path: ", path+"], pixel loading failed. ["+stbi_failure_reason(), "]");
		return nullptr;
	}

	//Création d'une texture SDL
	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STATIC, size.x, size.y); //Note: SDL_PIXELFORMAT_RGBA32 = (SDL_PIXELFORMAT_RGBA8888 sur big-endian), (SDL_PIXELFORMAT_ABGR8888 sur little-endian)
	if(not texture)
	{
		gs::lastError("||Warning|| in 'loadTexture()', [path: ", path+"], texture creation failed. ["+SDL_GetError(), "]");
		stbi_image_free(pixels); //Important
		return nullptr;
	}

	//Remplissage de la texture avec les pixels chargé précédemment
	int pitch = desired_channels*size.x; //Note: d'après la documention de stb_image, il n'y aurait pas de padding, ni entre les lignes, ni entre les pixels, si j'ai bien compris. Ainsi le pitch reviendrait à "channels*width".
	if(not SDL_UpdateTexture(texture, nullptr, (const void*)pixels, pitch))
	{
		gs::lastError("||Warning|| in 'loadTexture()', [path: ", path+"], texture pixels update failed. ["+SDL_GetError(), "]");
		SDL_DestroyTexture(texture); //Important
		stbi_image_free(pixels); //Important
		return nullptr;
	}

	//Post traitement
	if(nearest)
		SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_NEAREST); //Pour la rendre "nearest" (rendu au pixel près, pas de flou) lors du zoom.
	else
		SDL_SetTextureScaleMode(texture, SDL_SCALEMODE_LINEAR);

	SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	addToClearAtTheEnd(texture);

	paths_loaded_with_gsloadTexture.push_back(path);

	if(show_comments)
		std::cout << "\t|______Texture loaded : '" << path << "'______|\n";

	stbi_image_free(pixels); //Important

	return texture;
}



SDL_Surface* loadSurface(const std::string& img_path)
{
	SDL_Surface* surface = SDL_LoadSurface(img_path.c_str()); //BMP et PNG (voir JPEG si version suffisament récente)

	if(not surface)
		gs::lastError("||Warning|| in 'loadSurface()', [path: ", img_path+"], loading failed : "+SDL_GetError());

	return surface;

	//gs::lastError("||Warning|| in 'loadSurface()', function not availlable.");
	//return nullptr;
}



Texture_Target* createTextureTarget(Renderer* ren, Vec2i size)
{
	if(not ren)
	{
		gs::lastError("||Warning|| in 'createTextureTarget()', invalid renderer.");
		return nullptr;
	}
	if(size.x <= 0 or size.y <= 0)
	{
		gs::lastError("||Warning|| in 'createTextureTarget()', invalid size (x and/or y <= 0).");
		return nullptr;
	}

	Texture* t = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, size.x, size.y);

	if(not t)
	{
		gs::lastError("||Warning|| in 'createTextureTarget()', create texture error. [", SDL_GetError(),"]");
		return nullptr;
	}

	addToClearAtTheEnd(t);

	return t;
}


Surface* createSurface(Vec2i size, Color fill_color)
{
	Surface* surf = SDL_CreateSurface(size.x, size.y, SDL_PIXELFORMAT_RGBA32);
	if(not surf)
	{
		gs::lastError("||Warning|| in 'createSurface()' : ", SDL_GetError());
		return nullptr;
	}

	if(fill_color.r!=0 or fill_color.g!=0 or fill_color.b!=0 or fill_color.a!=0) //Note: la doc de 'SDL_CreateSurface', au moment où j'écris ce commentaire, indique que les pixels sont initialisés à zéro, ainsi si la couleur demandée est "zéro" on évite une opération.
	{
		uint32_t col = SDL_MapSurfaceRGBA(surf, fill_color.r, fill_color.g, fill_color.b, fill_color.a);
		SDL_FillSurfaceRect(surf, nullptr, col);
	}

	return surf;
}



bool drawTile(Renderer* ren, Texture_Target* target, Texture* tileset,
					FRect src, FRect dest, bool takeInToAccountTheTarget)
{
	bool success = true;

	SDL_Texture* previousTarget = SDL_GetRenderTarget(ren);

	if(takeInToAccountTheTarget)
		SDL_SetRenderTarget(ren, target);


	if(not SDL_RenderTexture(ren, tileset, &src, &dest))
	{
		lastError(std::string("||Warnig|| in `gs::drawTile()' , impossible to draw the tile to [")
			+std::to_string(dest.x)
			+std::string(" ; ")
			+std::to_string(dest.y)
			+std::string("], : ")
			+std::string(SDL_GetError()));

		success = false;
	}

	if(takeInToAccountTheTarget)
		SDL_SetRenderTarget(ren, previousTarget);

	return success;

}


bool drawSurfaceToSurface(Surface* src, Rect* src_rect, Surface* dest, Rect* dest_rect)
{
	if(not SDL_BlitSurfaceScaled(src, src_rect, dest, dest_rect, SDL_SCALEMODE_NEAREST))
	{
		gs::lastError("||Warning|| in 'drawSurfaceToSurface()', ", SDL_GetError());
		return false;
	}

	return true;
}



void setClearColor(Renderer* ren, Color col) {setRenderDrawColor(ren, col);}
Color getClearColor(Renderer* ren) {return getRenderDrawColor(ren);}


bool drawRect(SDL_Renderer* ren, const SDL_FRect* rect, const SDL_Color* col, bool fill, Blend_Mode* b_mode)
{
	if(not ren)
	{
		gs::lastError("in 'drawRect()', invalid renderer. ");
		return false;
	}

	SDL_Color previous_c;
	if(col)
	{
		SDL_GetRenderDrawColor(ren, &previous_c.r, &previous_c.g, &previous_c.b, &previous_c.a);
		SDL_SetRenderDrawColor(ren, col->r, col->g, col->b, col->a);
	}

	SDL_BlendMode previous_blend_mode;
	if(b_mode)
	{
		SDL_GetRenderDrawBlendMode(ren, &previous_blend_mode);
		SDL_SetRenderDrawBlendMode(ren, *b_mode);
	}

	bool res;

	if(fill)
		res = SDL_RenderFillRect(ren, rect);
	else
		res = SDL_RenderRect(ren, rect);

	if(b_mode)
		SDL_SetRenderDrawBlendMode(ren, previous_blend_mode);

	if(col)
		SDL_SetRenderDrawColor(ren, previous_c.r, previous_c.g, previous_c.b, previous_c.a);

	if(!res)
	{
		gs::lastError(gs::str("in 'drawRect()' : ")+SDL_GetError());
		return false;
	}

	return true;
}

bool drawLine(SDL_Renderer* ren, Vec2f pt1, Vec2f pt2, const SDL_Color& col, Blend_Mode* b_mode)
{
	if(not ren)
	{
		gs::lastError("in 'drawLine()', invalid renderer. ");
		return false;
	}

	SDL_Color previous_c;
	SDL_GetRenderDrawColor(ren, &previous_c.r, &previous_c.g, &previous_c.b, &previous_c.a);
	SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);

	SDL_BlendMode previous_blend_mode;

	if(b_mode)
	{
		SDL_GetRenderDrawBlendMode(ren, &previous_blend_mode);
		SDL_SetRenderDrawBlendMode(ren, *b_mode);
	}

	bool res = SDL_RenderLine(ren, pt1.x, pt1.y, pt2.x, pt2.y);

	if(b_mode)
		SDL_SetRenderDrawBlendMode(ren, previous_blend_mode);

	SDL_SetRenderDrawColor(ren, previous_c.r, previous_c.g, previous_c.b, previous_c.a);

	if(!res)
	{
		gs::lastError(std::string("In 'drawLine()' : ")+SDL_GetError());
		return false;
	}

	return true;
}

bool drawPoint(SDL_Renderer* ren, int x, int y, const SDL_Color* col, Blend_Mode* b_mode)
{
	if(not ren)
	{
		gs::lastError("In 'drawPoint()', invalid renderer. ");
		return false;
	}

	SDL_Color previous_c;
	if(col)
	{
		SDL_GetRenderDrawColor(ren, &previous_c.r, &previous_c.g, &previous_c.b, &previous_c.a);
		SDL_SetRenderDrawColor(ren, col->r, col->g, col->b, col->a);
	}

	SDL_BlendMode previous_blend_mode;
	if(b_mode)
	{
		SDL_GetRenderDrawBlendMode(ren, &previous_blend_mode);
		SDL_SetRenderDrawBlendMode(ren, *b_mode);
	}

	bool res = SDL_RenderPoint(ren, x, y);

	if(b_mode)
		SDL_SetRenderDrawBlendMode(ren, previous_blend_mode);

	if(col)
		SDL_SetRenderDrawColor(ren, previous_c.r, previous_c.g, previous_c.b, previous_c.a);

	if(!res)
	{
		gs::lastError(gs::str("In 'drawPoint()' : ")+SDL_GetError());
		return false;
	}

	return true;
}

int randint(int min, int max) //distribution uniforme
{
	return int(random(float(min), float(max)+0.99f));
}

float _INTERNAL_GS_dt_ = 0.f;

float dt() {return _INTERNAL_GS_dt_;}

void processDeltaTime()
{
	static uint64_t _last_time_ns=0;

	if(_last_time_ns == 0) _last_time_ns  = SDL_GetTicksNS();

	uint64_t now_time_ns = SDL_GetTicksNS();
	_INTERNAL_GS_dt_ = static_cast<float>(now_time_ns - _last_time_ns)*0.000000001f; // Conversion en secondes.
	_last_time_ns = now_time_ns;
}



//___________________________________________


void addToClearAtTheEnd_backend(Texture* tex, void* texturesToClearAtTheEnd_ptr)
{
	if(not texturesToClearAtTheEnd_ptr) return;

	std::vector<gs::Texture*>& texturesToClearAtTheEnd = *((std::vector<gs::Texture*>*)texturesToClearAtTheEnd_ptr);

	for(Texture* t : texturesToClearAtTheEnd)
		if(t == tex)
			return;
	if(tex)
		texturesToClearAtTheEnd.push_back(tex);
}


Vec2<float> getTextureSize(Texture* texture, float* w, float* h)
{
	Vec2f size{0.f, 0.f};
	SDL_GetTextureSize(texture, &size.x, &size.y);

	if(w) *w = size.x;
	if(h) *h = size.y;

	return size;
}


Vec2<float> getSurfaceSize(Surface* surface, float* w, float* h)
{
	Vec2f size{0.f, 0.f};

	if(surface)
	{
		size.x = surface->w;
		size.y = surface->h;
	}

	if(w) *w = size.x;
	if(h) *h = size.y;

	return size;
}



Vec2<int> getCurrentRenderOutputSize(Renderer* ren, int* w, int* h)
{
	Vec2i size;
	SDL_GetCurrentRenderOutputSize(ren, &size.x, &size.y);

	if(w) *w = size.x;
	if(h) *h = size.y;

	return size;
}

Vec2<int> getRenderOutputSize(Renderer* ren, int* w, int* h)
{
	Vec2i size;
	SDL_GetRenderOutputSize(ren, &size.x, &size.y);

	if(w) *w = size.x;
	if(h) *h = size.y;

	return size;
}

SDL_Color getRenderDrawColor(SDL_Renderer* ren)
{
	SDL_Color c;
	SDL_GetRenderDrawColor(ren, &c.r, &c.g, &c.b, &c.a);
	return c;
}

bool setRenderDrawColor(SDL_Renderer* ren, SDL_FColor col)
{
	_prev_ren_draw_col = getRenderDrawColor(ren);
	SDL_SetRenderDrawColorFloat(ren, col.r, col.g, col.b, col.a);

	return ren!=nullptr;
}


bool setRenderDrawColor(SDL_Renderer* ren, SDL_Color col)
{
	_prev_ren_draw_col = getRenderDrawColor(ren);
	SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);

	return ren!=nullptr;
}

SDL_Color restaurePrevRenderDrawColor(SDL_Renderer* ren)
{
	SDL_Color target = _prev_ren_draw_col;
	setRenderDrawColor(ren, target);
	return target;
}


SDL_BlendMode getRenderBlendMode(SDL_Renderer* ren)
{
	SDL_BlendMode curr_b_mode;
	SDL_GetRenderDrawBlendMode(ren, &curr_b_mode);
	return curr_b_mode;
}


bool setRenderBlendMode(SDL_Renderer* ren, SDL_BlendMode blend_mode)
{
	_prev_ren_blend_mode = getRenderBlendMode(ren);
	SDL_SetRenderDrawBlendMode(ren, blend_mode);
	return ren != nullptr;
}


SDL_BlendMode restaurePrevRenderBlendMode(SDL_Renderer* ren)
{
	SDL_BlendMode target = _prev_ren_blend_mode;
	setRenderBlendMode(ren, target);
	return target;
}


bool setRenderTarget(Renderer* ren, Texture_Target* texture_target)
{
	bool res = SDL_SetRenderTarget(ren, texture_target);
	if(not res)
		gs::lastError("||Warning|| in 'setRenderTarget()' : [", SDL_GetError(),"]");
	return res;
}



SDL_Surface* textureToSurface(Renderer* ren, Texture* tex)
{
	if(not tex or not ren) return nullptr;

	//Je pose une limite de taille pour la texture à traiter.
	int max_w = 3000;
	int max_h = 3000;

	Vec2f texture_size;
	SDL_GetTextureSize(tex, &texture_size.x, &texture_size.y);

	if(texture_size.x > max_w or texture_size.y > max_h) //Limite de taille
	{
		gs::lastError("||Error|| in 'textureToSurface()', given texture'size is too big according to the limits imposed to this function. [Max allowed (",
					std::to_string(max_w)+" x "+std::to_string(max_h)+"), given ("+std::to_string(texture_size.x)+" x "+std::to_string(texture_size.y)+")] ");

		return nullptr;
	}

	static SDL_Texture* target_buffer=nullptr;
	if(not target_buffer) //Génération du buffer si non chargé
	{
		target_buffer = SDL_CreateTexture(ren, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, max_w, max_h);

		if(not target_buffer)
		{
			gs::lastError("||Error|| in 'textureToSurface()', failed to load the buffer. ", SDL_GetError());
			return nullptr;
		}
		else
			gs::addToClearAtTheEnd(target_buffer);
	}

	//----------------------------------------------------------//
	//Le traitement peut commencer

	SDL_Texture* prev_render_target = SDL_GetRenderTarget(ren); //Récupération du précédent render target afin de pouvoir le restaurer une fois le traitement fini.

	SDL_SetRenderTarget(ren, target_buffer); //Mise en place de notre buffer afin d'y dessiner dessus.
	FRect dest{0.f,0.f,texture_size.x, texture_size.y}; //Avec les testes fait plus tôt, ce rectangle, à la fois matchant la texture, rentre complètement dans le buffer.

	SDL_Surface* output=nullptr;

	if(SDL_RenderTexture(ren, tex, nullptr, &dest))
	{
		Rect dest_i{int(dest.x),int(dest.y),int(dest.w),int(dest.h)};
		output = SDL_RenderReadPixels(ren, &dest_i);

		if(not output)
			gs::lastError("||Error|| in 'textureToSurface()', SDL_RenderReadPixels() error, ", SDL_GetError());
	}
	else
	{
		gs::lastError("||Error|| in 'textureToSurface()', SDL_RenderTexture() error, ", SDL_GetError());
		//On ne quitte pas ici, afin de rétablir le précédent render target. Le retour sera toute de même 'nullptr' car 'output' n'est pas touchée dans ce cas.
	}

	SDL_SetRenderTarget(ren, prev_render_target); //Ni vu ni connu ^-^

	return output; //Note: la surface générée est maintenant sous la responsabilité de l'extérieur de la fonction pour sa libération en mémoire.
}



Texture* surfaceToTexture(Renderer* ren, Surface* surface)
{
	if(not ren)
	{
		gs::lastError("||Warning|| in 'surfaceToTexture()', invalid renderer.");
		return nullptr;
	}

	if(not surface)
	{
		gs::lastError("||Warning|| in 'surfaceToTexture()', invalid surface ptr.");
		return nullptr;
	}

	Texture* t = SDL_CreateTextureFromSurface(ren, surface);

	if(not t)
		gs::lastError("||Warning|| in 'surfaceToTexture()', ", SDL_GetError());
	else
		SDL_SetTextureScaleMode(t, SDL_SCALEMODE_NEAREST);

	return t;
}




bool drawLines_INTERNAL(Renderer* ren, const FPoint* points, int pts_count, const Color& col, Blend_Mode* b_mode)
{
	if(not ren)
	{
		gs::lastError("in 'drawLines()', invalid renderer. ");
		return false;
	}

	setRenderDrawColor(ren, col);
	if(b_mode)
		setRenderBlendMode(ren, *b_mode);

	bool res = SDL_RenderLines(ren, points, pts_count);

	restaurePrevRenderDrawColor(ren);
	if(b_mode)
		restaurePrevRenderBlendMode(ren);

	if(!res)
	{
		gs::lastError(std::string("In 'drawLines()' : ")+SDL_GetError());
		return false;
	}

	return true;
}















void Backend__Mouse_Tracker::processEvent(const SDL_Event& ev)
{
	Uint8 target_btn = SDL_BUTTON_LEFT;
	if(not m_target_btn_is_left)
		target_btn = SDL_BUTTON_RIGHT;

	if(ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN and ev.button.button == target_btn)
	{
		m_is_down = true;

		m_prev_down_pos.x = m_pos_when_down.x;
		m_prev_down_pos.y = m_pos_when_down.y;

		m_pos_when_down.x = ev.button.x;
		m_pos_when_down.y = ev.button.y;

		if(ev.button.clicks >= 2)
			m_is_multi_click = true;
		else
			m_is_multi_click = false;

		m_clicks_count = ev.button.clicks;
	}
	else
	if(ev.type == SDL_EVENT_MOUSE_BUTTON_UP and ev.button.button == target_btn)
	{
		m_is_down = false;
		m_is_multi_click = false;
		m_clicks_count = 0;

		m_pos_when_up.x = ev.button.x;
		m_pos_when_up.y = ev.button.y;
	}
	else
	if(ev.type == SDL_EVENT_MOUSE_WHEEL)
	{
		if(ev.wheel.x) m_wheel_buff.x = ev.wheel.x;
		if(ev.wheel.y) m_wheel_buff.y = ev.wheel.y;
	}
}



void Backend__Game::backendDestr()
{
    if(ren_ != nullptr)
		SDL_DestroyRenderer(ren_); ren_ = nullptr;

	if(win_ != nullptr)
		SDL_DestroyWindow(win_);   win_ = nullptr;


	//if(mixActive_)
		//Mix_CloseAudio();


	if(win_icon != nullptr)
		SDL_DestroySurface(win_icon);

	if(is_main_game)
		SDL_Quit();
	else
		std::atexit(SDL_Quit);

}

//____________________________________________________________________________________
//____________________________________________________________________________________

std::string chooseRendererName()
{
	//Tentative de récupération d'un backend de rendu selon un ordre de préférence, sur Windows.
	std::string chosen_ren_name;
	#ifdef _WIN32

	std::vector<std::string> available_ones;
	std::vector<std::string> choices{"gpu", "direct3d12", "vulkan", "direct3d11", "opengl",  "opengles2", "direct3d"};

	//Récupération des noms des backends disponibles.
	int available_rens_count = SDL_GetNumRenderDrivers();
	for(int i=0; i < available_rens_count; ++i)
		available_ones.push_back(std::string(SDL_GetRenderDriver(i)));


	for(const std::string& ren_name : choices) //On parcours ensuite selon l'ordre établi dans 'choices'.
	{
		for(const std::string& available_ren : available_ones) //Puis on regarde s'il est présent
		{
			if(ren_name == available_ren)
			{
				chosen_ren_name = available_ren;
				break;
			}
		}

		if(chosen_ren_name.size()) break; //On quitte dès la première trouvaille
	}

	#endif

	return chosen_ren_name; //Si le retour est vide, alors on laisse à SDL de choisir
}


bool Backend__Game::InitSDLEngine(int window_w, int window_h, const char* window_title, uint32_t win_flags, const char* renderer_name, bool try_enable_VSYNC)
{
	if(isInit) return true;

	if(not SDL_Init(SDL_INIT_VIDEO))
	{
		std::string error = std::string("||Fatal Error|| in 'gs::Backend__Game::InitSDLEngine()', 'SDL_Init()' failed : ")+std::string(SDL_GetError());
		throw std::runtime_error(error);
	};

	win_ = SDL_CreateWindow(window_title, window_w, window_h, win_flags);
	if(win_ == nullptr)
	{
		SDL_Quit();
		// std::exit(-1);
		std::string error = std::string("||Fatal Error|| in 'gs::Backend__Game::InitSDLEngine()', 'SDL_CreateWindow()' failed : ")+std::string(SDL_GetError());
		throw std::runtime_error(error);
	};


	std::string ren_name;
	if(renderer_name)
		ren_name = std::string(renderer_name);
	else
	if(allow_GSlib_try_choose_ren_when_no_given_ren_name)
		ren_name = chooseRendererName(); //Une petite manipulation pour le choix du backend du renderer lorsqu'il n'y a pas une de choix explicite reçu en argument.

	if(ren_name.size())
		ren_ = SDL_CreateRenderer(win_, ren_name.c_str());
	else
		ren_ = SDL_CreateRenderer(win_, nullptr);

	if(ren_ == nullptr)
	{
		SDL_DestroyWindow(win_);
		SDL_Quit();
		// std::exit(-1);
		std::string error = std::string("||Fatal Error|| in 'gs::Backend__Game::InitSDLEngine()', 'SDL_CreateRenderer()' failed : ")+std::string(SDL_GetError())
								+(renderer_name ? std::string(" - Maybe given renderer name '")+std::string(renderer_name)+"' is invalid. " : (ren_name.size() ? std::string(" - [Internal : chosen renderer name '")+ren_name+"']" : ""));
		throw std::runtime_error(error);
	};

	if(try_enable_VSYNC)
		if(not SDL_SetRenderVSync(ren_, SDL_RENDERER_VSYNC_ADAPTIVE))
		{
			if(not skip_vsync_error_msg)
				gs::lastError("||Warning|| in 'gs::Backend__Game::InitSDLEngine()', failed to enable VSYNC : ", SDL_GetError(), "");
		}

	//Pour les primitives.
	setRenderBlendMode(ren_, SDL_BLENDMODE_BLEND);
	setRenderDrawColor(ren_, SDL_Color(0,0,0,255));

	if(get_show_comments())
	std::cout<<"|______SDL and Backend__Game init succes______|" << std::endl;

	Win_h = window_h;
	Win_w = window_w;
	isInit = true;
	return true;
}



//____________________________________________________________________________________
//____________________________________________________________________________________



bool Backend__Game::InitByAssociateTo(SDL_Window *W, SDL_Renderer *newR )
{
	if(not isInit)
	{
		isInit = true;
		win_ = W;
		changeRenderer(newR);

		if(get_show_comments())
		std::cout<<"\tBackend__Game launched" << std::endl;
	}
	else
	{
		if(get_show_comments())
		std::cout <<"\tBackend__Game already launched" << std::endl;

		return false;
	};

	return true;
};



//____________________________________________________________________________________
//____________________________________________________________________________________

		/*simple geters*/
//----------------------------------------------
SDL_Renderer *Backend__Game::ren() const {return ren_;};
//----------------------------------------------
SDL_Window *Backend__Game::win() const {return win_;};



//____________________________________________________________________________________
//____________________________________________________________________________________

uint32_t Backend__Game::getWinID() const
{
	if(win_ == nullptr)
		return 0;

	return SDL_GetWindowID(win());
}



bool Backend__Game::isCloseWinEvent(SDL_Event& ev)
{
	if(ev.type == SDL_EVENT_QUIT)
		return true;

	if(ev.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED)
		if(getWinID() == ev.window.windowID)
			return true;

	return false;
}



void Backend__Game::changeRenderer(SDL_Renderer *newR, bool destroy)
{
	if(destroy)
		if(ren_ != nullptr)
			SDL_DestroyRenderer(ren_);

	if(not destroy)
		addToClearAtTheEnd(ren_);

	ren_ = newR;
};



//____________________________________________________________________________________
//____________________________________________________________________________________

bool Game::InitTheWindow(int window_w, int window_h, const char* window_title, uint32_t win_flags, const char* renderer_name, bool try_enable_VSYNC)
{
	return InitSDLEngine(window_w, window_h, window_title, win_flags, renderer_name, try_enable_VSYNC);
}


//----------------------------------------------
int Game::getWinW()
{
	SDL_GetWindowSize(win(), &Win_w, &Win_h);
	return Win_w;
};
//----------------------------------------------
int Game::getWinH()
{
	SDL_GetWindowSize(win(), &Win_w, &Win_h);
	return Win_h;
};
//----------------------------------------------
int Game::getRenW()
{
	SDL_GetCurrentRenderOutputSize(ren(), &m_ren_w, &m_ren_h);
	return m_ren_w;
};
//----------------------------------------------
int Game::getRenH()
{
	SDL_GetCurrentRenderOutputSize(ren(), &m_ren_w, &m_ren_h);
	return m_ren_h;
};


float Game::dt() const {return ::gs::dt();}


void Game::fpsProcessBegin()
{
	m_frame_begin_ticks = SDL_GetTicks();
	m_fpsProcessBegin_is_ok = true;
}

void Game::fpsProcessEnd(uint32_t fps)
{
	if(not m_fpsProcessBegin_is_ok)
	{
		gs::lastError("!! in 'Game::fpsProcessEnd()', a call of 'Game::fpsProcessBegin()' is needed before calling 'Game::fpsProcessEnd()', as many times as 'Game::fpsProcessEnd()' is called. ");
		return;
	}

	if(fps == 0)
		fps = 1;

	float FPS = static_cast<float>(fps);
	float one_frame_duration = 1.0/FPS; //En sec
	uint64_t one_frame_ms_duration = std::lround(one_frame_duration * 1000.0);

	m_frame_end_ticks = SDL_GetTicks();

	uint64_t delta = m_frame_end_ticks - m_frame_begin_ticks;

	if(delta < one_frame_ms_duration)
		SDL_Delay(one_frame_ms_duration - delta);

	m_fpsProcessBegin_is_ok = false;
}


void Game::processDeltaTime() {::gs::processDeltaTime();}


void Game::setWinIcon(const std::string& path)
{
	if(not isInit)
	{
		gs::lastError("||Warning|| in 'Game::setWinIcon()', not yet init. ");
		return;
	}

	if(win_icon != nullptr)
		SDL_DestroySurface(win_icon);

	win_icon = loadSurface(path.c_str());

	SDL_SetWindowIcon(win(), win_icon);
}


void Game::setWinTitle(const std::string& new_title)
{
	if(win_ != nullptr)
		SDL_SetWindowTitle(win_, new_title.c_str());
}

void Game::renPresent() {SDL_RenderPresent(ren());}

void Game::renClear() {SDL_RenderClear(ren());}


SDL_Color Game::setDrawColor(SDL_Color new_color)
{
	//return : Renvoie la couleur précédente.

	SDL_Color curr_col;

	if(not isInit)
	{
		gs::lastError("||Warning|| in 'Game::setDrawColor()', not yet init. ");
		return curr_col;
	}

	SDL_GetRenderDrawColor(ren_, &curr_col.r, &curr_col.g, &curr_col.b, &curr_col.a);
	SDL_SetRenderDrawColor(ren_, new_color.r, new_color.g, new_color.b, new_color.a);

	return curr_col;
}


SDL_Color Game::getDrawColor()
{
	SDL_Color curr_col;

	if(not isInit)
	{
		gs::lastError("||Warning|| in 'Game::getDrawColor()', not yet init. ");
		return curr_col;
	}

	SDL_GetRenderDrawColor(ren_, &curr_col.r, &curr_col.g, &curr_col.b, &curr_col.a);
	return curr_col;
}














//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//_______________________________________GEO__________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________




bool Triangle::draw(Renderer* ren, Color col, bool fill, const Blend_Mode* b_mode)
{
	if(!ren)
	{
		gs::lastError("||Warning|| in 'Triangle::draw()', invalid renderer. ");
		return false;
	}

	SDL_BlendMode previous_blend_mode;
	if(b_mode)
	{
		SDL_GetRenderDrawBlendMode(ren, &previous_blend_mode);
		SDL_SetRenderDrawBlendMode(ren, *b_mode);
	}

	Color previous_c;
	SDL_FColor color_;
	//if(col) //Application de la couleur demandée.
	{
		SDL_GetRenderDrawColor(ren,&previous_c.r,&previous_c.g,&previous_c.b,&previous_c.a);
		SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
		color_.r=col.r/255.f; color_.g=col.g/255.f; color_.b=col.b/255.f; color_.a=col.a/255.f;
	}


	bool success = true;
	bool res=true, res1=true, res2=true, res3=true;
	std::string err, err1, err2, err3;

	if(fill)
	{
		SDL_Vertex common_fields;
		common_fields.color = color_;
		common_fields.tex_coord.x = 1;
		common_fields.tex_coord.y = 1;

		std::vector<SDL_Vertex> vertices={common_fields, common_fields, common_fields};
		vertices.at(0).position.x = a.x; vertices.at(0).position.y = a.y;
		vertices.at(1).position.x = b.x; vertices.at(1).position.y = b.y;
		vertices.at(2).position.x = c.x; vertices.at(2).position.y = c.y;

		res = SDL_RenderGeometry(ren, nullptr, vertices.data(), int(vertices.size()), nullptr, 0);
			if(!res) err = std::string(SDL_GetError());
	}
	else
	{
		res1 = SDL_RenderLine(ren, (int)a.x, (int)a.y, (int)b.x, (int)b.y);
			if(!res1) err1 = std::string(SDL_GetError());
		res2 = SDL_RenderLine(ren, (int)b.x, (int)b.y, (int)c.x, (int)c.y);
			if(!res2) err2 = std::string(SDL_GetError());
		res3 = SDL_RenderLine(ren, (int)c.x, (int)c.y, (int)a.x, (int)a.y);
			if(!res3) err3 = std::string(SDL_GetError());
	}

	if(!res or !res1 or !res2 or !res3)
	{
		using std::operator ""s;
		gs::lastError("||Warning|| in 'Triangle::draw()' : [0: "s + err+" | 1: "+ err1+" | 2: "+ err2+" | 3: "+ err3+" ]");
		success = false;
	}

	//if(col) //Restauration de la couleur de dessin précédente.
		SDL_SetRenderDrawColor(ren,previous_c.r,previous_c.g,previous_c.b,previous_c.a);

	if(b_mode) //Restauration du b_mode précédente.
		SDL_SetRenderDrawBlendMode(ren, previous_blend_mode);

	return success;

}



bool drawRect(Renderer* ren, const FRect* rect, Color col, bool fill, /*short thickness,*/ Blend_Mode* b_mode, Rotation_P rotation)
{
	if(not ren)
	{
		gs::lastError("In 'drawRect()', invalid renderer. ");
		return false;
	}

	float rot_rad = rotation.rot_rad;
	Vec2f anchor = rotation.anchor;
	bool clockwise = rotation.clockwise;


	FRect rect_cpy={0.f, 0.f, 0.f, 0.f};

	if(rect) //S'il y'a un rectangle de donné.
		rect_cpy = *rect;

	else //Sinon on prend le rectangle de l'affichage.
	{
		int w, h;
		SDL_GetCurrentRenderOutputSize(ren, &w, &h);
		rect_cpy.w = float(w);
		rect_cpy.h = float(h);
	}


	//Les quatres points du rectangle
	//
	// nw       ne
	//
	// sw       se
	gs::Vec2f nw, ne, se, sw;
	nw.x = rect_cpy.x;
	nw.y = rect_cpy.y;
	ne.x = nw.x + rect_cpy.w-1;
	ne.y = nw.y;
	se.x = ne.x;
	se.y = ne.y + rect_cpy.h-1;
	sw.x = nw.x;
	sw.y = se.y;



	//Centre de rotation.
	gs::Vec2f rot_center;
	rot_center.x = rect_cpy.x + (rect_cpy.w*anchor.x);
	rot_center.y = rect_cpy.y + (rect_cpy.h*anchor.y);


	//Calculs des coordonnées des différents points par rapport au centre de rotation après rotation.
	if(rot_rad != 0)
	{
		nw = rotatePoint(rot_center.x, rot_center.y, nw.x, nw.y, rot_rad, clockwise).to<float>();
		ne = rotatePoint(rot_center.x, rot_center.y, ne.x, ne.y, rot_rad, clockwise).to<float>();
		se = rotatePoint(rot_center.x, rot_center.y, se.x, se.y, rot_rad, clockwise).to<float>();
		sw = rotatePoint(rot_center.x, rot_center.y, sw.x, sw.y, rot_rad, clockwise).to<float>();
	}

	//Préparation au dessin
	SDL_BlendMode previous_blend_mode;
	if(b_mode)
	{
		SDL_GetRenderDrawBlendMode(ren, &previous_blend_mode);
		SDL_SetRenderDrawBlendMode(ren, SDL_BlendMode(*b_mode));
	}

	SDL_Color previous_c;
	SDL_FColor color_;
	//if(col)
	{
		SDL_GetRenderDrawColor(ren, &previous_c.r, &previous_c.g, &previous_c.b, &previous_c.a);
		SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);
		color_.r=col.r/255.f; color_.g=col.g/255.f; color_.b=col.b/255.f; color_.a=col.a/255.f;
	}


	//Dessin
	if(fill)
	{
		SDL_Vertex common_fields;
		common_fields.color = color_;
		common_fields.tex_coord.x = 1;
		common_fields.tex_coord.y = 1;

		std::vector<SDL_Vertex> vertices{common_fields, common_fields, common_fields};
		vertices.at(0).position.x = nw.x;
		vertices.at(0).position.y = nw.y;
		vertices.at(1).position.x = ne.x;
		vertices.at(1).position.y = ne.y;
		vertices.at(2).position.x = se.x;
		vertices.at(2).position.y = se.y;

		bool res1 = SDL_RenderGeometry(ren, nullptr, vertices.data(), int(vertices.size()), nullptr, 0);
		vertices.at(1).position.x = sw.x;
		vertices.at(1).position.y = sw.y;
		bool res2 = SDL_RenderGeometry(ren, nullptr, vertices.data(), int(vertices.size()), nullptr, 0);
	}
	else
	{
		std::vector<FPoint> points{nw.toFPoint(), ne.toFPoint(),
										se.toFPoint(), sw.toFPoint(),
										nw.toFPoint()};

		SDL_RenderLines(ren, points.data(), 5);
	}


	//Rétablissement des précédents paramètres.
	//if(col)
		SDL_SetRenderDrawColor(ren, previous_c.r, previous_c.g, previous_c.b, previous_c.a);

	if(b_mode)
		SDL_SetRenderDrawBlendMode(ren, previous_blend_mode);


	return true;
}



bool drawRoundedRect(Renderer* ren, FRect rect, float percent, Color col, bool fill, Blend_Mode* b_mode)
{
	if(!ren)
	{
		gs::lastError("||Error|| in 'drawRoundedRect()', invalid renderer. ");
		return false;
	}

	if(SDL_RectEmptyFloat(&rect)) return true;

	if(percent < 0.f) percent = 0.f;
	if(percent > 0.5f) percent = 0.5f;

	float min_side_size = SDL_min(rect.w, rect.h);
	float margin = min_side_size * percent;

	FRect main = {rect.x+margin, rect.y, rect.w-margin*2, rect.h};
	FRect left_side = {rect.x, rect.y+margin, margin, rect.h-margin*2};
	FRect right_side = {main.x+main.w, rect.y+margin, margin, rect.h-margin*2};


	//Préparation au dessin
	SDL_BlendMode previous_blend_mode;
	if(b_mode)
	{
		SDL_GetRenderDrawBlendMode(ren, &previous_blend_mode);
		SDL_SetRenderDrawBlendMode(ren, *b_mode);
	}

	bool success = true;

	if(fill)
	{
		success = success & drawRect(ren, &main, col, fill);
		success = success & drawRect(ren, &left_side, col, fill);
		success = success & drawRect(ren, &right_side, col, fill);
	}
	else
	{
		Vec2f line_1_pt1 = Vec2f(main.x, main.y);
		Vec2f line_1_pt2 = Vec2f(main.x+main.w-1, main.y);
		Vec2f line_2_pt1 = Vec2f(right_side.x+margin-1, right_side.y);
		Vec2f line_2_pt2 = Vec2f(right_side.x+margin-1, right_side.y+right_side.h-1);
		Vec2f line_3_pt1 = Vec2f(main.x, main.y+main.h-1);
		Vec2f line_3_pt2 = Vec2f(main.x+main.w-1, main.y+main.h-1);
		Vec2f line_4_pt1 = Vec2f(left_side.x, left_side.y);
		Vec2f line_4_pt2 = Vec2f(left_side.x, left_side.y+left_side.h-1);

		success = success & drawLine(ren, line_1_pt1, line_1_pt2, col);
		success = success & drawLine(ren, line_2_pt1, line_2_pt2, col);
		success = success & drawLine(ren, line_3_pt1, line_3_pt2, col);
		success = success & drawLine(ren, line_4_pt1, line_4_pt2, col);
	}

	Vec2f margin_margin = Vec2f(margin, margin);
	success = success & drawEllipse(ren, Vec2f(rect.x+margin, rect.y+margin), margin_margin, col, fill, gs::rad(90), gs::rad(180));
	success = success & drawEllipse(ren, Vec2f(right_side.x-(int(!fill)), right_side.y), margin_margin, col, fill, gs::rad(90), gs::rad(270));
	success = success & drawEllipse(ren, Vec2f(right_side.x-(int(!fill)), right_side.y+right_side.h-(int(!fill))), margin_margin, col, fill, gs::rad(90), gs::rad(0));
	success = success & drawEllipse(ren, Vec2f(rect.x+margin, left_side.y+left_side.h-(int(!fill))), margin_margin, col, fill, gs::rad(90), gs::rad(90));


	if(not success)
		gs::lastError(gs::str("||Warning|| in 'drawRoundedRect()', error occured : ")+gs::lastError());


	if(b_mode)
		SDL_SetRenderDrawBlendMode(ren, previous_blend_mode);

	return success;
}



bool drawEllipse(Renderer* ren, Vec2f center, Vec2f radii, Color col, bool fill, float total_angle, float begin_angle)
{
	//Dessine une ellipse par décompositon en triangles (ou en segements si ce n'est pas plein).

	//center = (center.to<int>()).to<float>();
	//radii = (radii.to<int>()).to<float>();

	if(!ren)
	{
		gs::lastError("||Error|| in 'drawEllipse()', invalid renderer. ");
		return false;
	}

	if(total_angle <= 0)
	{
		// gs::lastError("||Warning|| in 'drawEllipse()', 'total_angle' must be '>= 0' ");
		return true; // return false;
	}
	if(total_angle > 2*_pi_)
		total_angle = 2*_pi_;

	static const char qualities_nb=8;
	static const float qualities[qualities_nb] = {12.f, 16.f, 30.f, 60.f, 100.f, 120.f, 140.f, 160.f}; // -> en combien de triangle on va découper le cerlce.
	static const short limits[qualities_nb-1] = {10, 30, 100, 220, 300, 400, 500};

	float nb_of_triangles = qualities[qualities_nb-1];
	int max_radius = SDL_max(radii.x, radii.y);

	for(int i=0; i < qualities_nb-1; ++i)
	{
		if(max_radius < limits[i])
		{
			nb_of_triangles = qualities[i];
			break;
		}
	}



	float percent_angle = total_angle / (2.0*M_PI);
	nb_of_triangles = float(int(nb_of_triangles * percent_angle));

	FPoint circle_center={float(center.x), float(center.y)};
	float delta_angle = total_angle / nb_of_triangles; //2*M_PI / nb_of_triangles;

	float curr_angle = begin_angle;
	std::vector<SDL_Vertex> vertices;

	SDL_Vertex common_fields;
		common_fields.color = fcolor(col);
		common_fields.tex_coord.x = 1;
		common_fields.tex_coord.y = 1;

		//L'arg 'col' est attendu au format [0; 255], cependant 'SDL_Vertex.color' est attendu au format [0.0; 1.0], d'où la conversion.
		common_fields.color.r/=255.f; common_fields.color.g/=255.f; common_fields.color.b/=255.f; common_fields.color.a/=255.f;


	FPoint last_point; // Va être init avec le 1er point.
	bool break_next = false;

		last_point.x = std::cos(curr_angle) * float(radii.x) + circle_center.x;
		last_point.y = std::sin(curr_angle) * float(radii.y) + circle_center.y;

		//On tourne l'angle.
		if((curr_angle + delta_angle) > total_angle +begin_angle)
		{
			curr_angle = total_angle +begin_angle;
			break_next = true;
		}
		else
			curr_angle += delta_angle;



	do
	{
		FPoint curr_pt;

		curr_pt.x = std::cos(curr_angle) * float(radii.x) +circle_center.x;
		curr_pt.y = std::sin(curr_angle) * float(radii.y) +circle_center.y;

		if(fill)
		{
			vertices.push_back(common_fields);
				vertices.back().position = circle_center; // Point n°1.
		}

		vertices.push_back(common_fields);
			vertices.back().position = last_point; // Point n°2.

		vertices.push_back(common_fields);
			vertices.back().position = curr_pt; // Point n°3.


		last_point = curr_pt;


		if(break_next) break;

		// On tourne l'angle.
		if((curr_angle + delta_angle) > total_angle +begin_angle)
		{
			curr_angle = total_angle +begin_angle;
			break_next = true;
		}
		else
			curr_angle += delta_angle;
	}
	while(curr_angle <= total_angle +begin_angle);


	bool res;

	if(fill)
		res = SDL_RenderGeometry(ren, nullptr, vertices.data(), int(vertices.size()), nullptr, 0);
	else
	{
		std::vector<FPoint> points;

		for(SDL_Vertex& ver : vertices)
			points.push_back(ver.position);


		Color previous_c;
		SDL_GetRenderDrawColor(ren, &previous_c.r, &previous_c.g, &previous_c.b, &previous_c.a);
		SDL_SetRenderDrawColor(ren, col.r, col.g, col.b, col.a);

		res = SDL_RenderLines(ren, points.data(), points.size());

		SDL_SetRenderDrawColor(ren, previous_c.r, previous_c.g, previous_c.b, previous_c.a);
	}

	if(!res)
	{
		gs::lastError(gs::str("||Error|| in 'drawEllipse()' : ") + SDL_GetError());
		return false;
	}

	return true;
}



bool drawEllipseThickness(Renderer* ren, Vec2f center, Vec2f radii, int thickness, Color col, bool fill, float total_angle, float begin_angle)
{
	//center = (center.to<int>()).to<float>();
	//radii = (radii.to<int>()).to<float>();

	if(!ren)
	{
		gs::lastError("||Error|| in 'drawEllipseThickness()', invalid renderer. ");
		return false;
	}

	if(total_angle < 0)
	{
		gs::lastError("||Warning|| in 'drawEllipseThickness()', 'total_angle' must be '>= 0'. ");
		return false;
	}
	else
	if(total_angle > 2*_pi_)
		total_angle = 2*_pi_;


float quality = 50;

	if(thickness == 0)
	{
		gs::lastError("||Warning|| in 'drawEllipseThickness()', 'thickness' must be '!= 0'. ");
		return false;
	}
	else
	if(thickness == 1 or thickness == -1)
	{
		radii += (thickness == -1 ? 1 : 0);
		bool success = drawEllipse(ren, center, radii, col, false, total_angle, begin_angle);
		if(not success)
			gs::lastError(gs::str("from 'drawEllipseThickness()' : ")+gs::lastError());

		return success;
	}
	else
	if(thickness >= SDL_min(radii.x, radii.y) and fill) //Si l'épaisseur dépasse le rayon minimum, l'épaisseur ne compte plus.
	{
		bool success = drawEllipse(ren, center, radii, col, true, total_angle, begin_angle);
		if(not success)
			gs::lastError(gs::str("from 'drawEllipseThickness()' : ")+gs::lastError());

		return success;
	}

	if(thickness < 0) //Si l'épaisseur donnée est négative, on considère une épaisseur vers l'extérieur.
	{
		thickness = std::abs(thickness);
		radii.x += thickness;
		radii.y += thickness;
	}


	if(thickness >= SDL_min(radii.x, radii.y)) //Si ce teste passe, alors c'est que fill==false.
		thickness = SDL_min(radii.x, radii.y);

	//Les rayons internes.
	float min_radius_x = radii.x - thickness;
	float min_radius_y = radii.y - thickness;

	float current_angle = begin_angle;
	float final_angle = begin_angle + total_angle;

	float delta_angle = (2*M_PI / quality);
	gs::Vec2f offset = {float(center.x), float(center.y)};

	std::vector<SDL_Vertex> vertices;
	SDL_Vertex common_fields;
		common_fields.color = SDL_FColor(col.r/255.f, col.g/255.f, col.b/255.f, col.a/255.f);
		common_fields.tex_coord.x = 1;
		common_fields.tex_coord.y = 1;

	int count = 0;
	bool break_next = false;
	gs::Vec2f prev_border_out;
	gs::Vec2f prev_border_in;

	if(not fill)
		goto _not_fill;

	while(current_angle <= final_angle)
	{
		gs::Vec2f border_out;
		border_out.x = std::cos(current_angle) * radii.x;
		border_out.y = std::sin(current_angle) * radii.y;

		gs::Vec2f border_in;
		border_in.x = std::cos(current_angle) * min_radius_x;
		border_in.y = std::sin(current_angle) * min_radius_y;

		border_out += offset;
		border_in += offset;

		if(count > 0) // = ce n'est pas le premier tour de boucle.
		{
			// Triangle 1.
			vertices.push_back(common_fields);
				vertices.back().position.x = prev_border_out.x;
				vertices.back().position.y = prev_border_out.y;

			vertices.push_back(common_fields);
				vertices.back().position.x = prev_border_in.x;
				vertices.back().position.y = prev_border_in.y;

			vertices.push_back(common_fields);
				vertices.back().position.x = border_out.x;
				vertices.back().position.y = border_out.y;

			// Triangle 2.
			vertices.push_back(common_fields);
				vertices.back().position.x = prev_border_in.x;
				vertices.back().position.y = prev_border_in.y;

			vertices.push_back(common_fields);
				vertices.back().position.x = border_out.x;
				vertices.back().position.y = border_out.y;

			vertices.push_back(common_fields);
				vertices.back().position.x = border_in.x;
				vertices.back().position.y = border_in.y;


		}

		count+=1;

		prev_border_out = border_out;
		prev_border_in = border_in;

		if(break_next) break;

		if((current_angle + delta_angle) > final_angle)
		{
			current_angle = final_angle;
			break_next = true;
		}
		else
			current_angle += delta_angle;
	}


	SDL_RenderGeometry(ren, nullptr, vertices.data(), int(vertices.size()), nullptr, 0);


	_not_fill:
	if(not fill)
	{
		//Ellipse externe.
		drawEllipse(ren, center, radii, col, false, total_angle, begin_angle);
		//Ellipse interne.
		drawEllipse(ren, center, {(min_radius_x), (min_radius_y)}, col, false, total_angle, begin_angle);

		//La fermeture des bords, si nécessaire.
		if(total_angle < 2*_pi_)
		{
			Vec2f pt1, pt2;
			pt1.x = std::cos(begin_angle)*min_radius_x +offset.x;
			pt1.y = std::sin(begin_angle)*min_radius_y +offset.y;
			pt2.x = std::cos(begin_angle)*radii.x +offset.x;
			pt2.y = std::sin(begin_angle)*radii.y +offset.y;
			drawLine(ren, pt1, pt2, col);

			pt1.x = std::cos(final_angle)*min_radius_x +offset.x;
			pt1.y = std::sin(final_angle)*min_radius_y +offset.y;
			pt2.x = std::cos(final_angle)*radii.x +offset.x;
			pt2.y = std::sin(final_angle)*radii.y +offset.y;
			drawLine(ren, pt1, pt2, col);
		}
	}

	return true;
}






bool drawCircle(Renderer* ren, Vec2f center, float radius, Color col, bool fill, float total_angle, float begin_angle)
{
	bool success = drawEllipse(ren, center, {radius, radius}, col, fill, total_angle, begin_angle);

	if(not success)
		gs::lastError(gs::str("from 'drawCircle()' : ")+gs::lastError());
	return success;
}

bool drawCircleThickness(Renderer* ren, Vec2f center, float radius, int thickness, Color col, bool fill, float total_angle, float begin_angle)
{
	bool success = drawEllipseThickness(ren, center, {radius, radius}, thickness, col, fill, total_angle, begin_angle);

	if(not success)
		gs::lastError(gs::str("from 'drawCircleThickness()' : ")+gs::lastError());
	return success;
}








//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//______________________________________TEXT__________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________




int Backend__Font::getFontLineSkip() {return TTF_GetFontLineSkip(m_SDL_font);}


bool Backend__Font::SDL_TTF__init_by_gs_Font = false;
int Backend__Font::remaining_gs_Font__count = 0;


Backend__Font::~Backend__Font()
{
	if(m_SDL_font)
		TTF_CloseFont(m_SDL_font);

	//if(m_close_TTF_at_my_exit)
	//{
	//	if(TTF_WasInit())
	//		TTF_Quit();
	//}

	remaining_gs_Font__count--;

	if(remaining_gs_Font__count <= 0 and SDL_TTF__init_by_gs_Font)
	{
		if(TTF_WasInit())
		{
			TTF_Quit();

			if(show_comments)
				std::cout << "|______SDL_TTF Exit success!______|\n";
		}

		SDL_TTF__init_by_gs_Font = false;
	}

}



bool Font::init(Renderer* ren, const std::string& font_path, size_t ptsize, te::Font_Rendering_Type default_type)
{
	if(m_is_init)
	{
		gs::lastError("||Warning|| in 'Font::init()', cannot init if it is already init.");
		return false;
	}

	if(not TTF_WasInit())
	{
		if(not TTF_Init())
		{
			gs::lastError(std::string("||Error|| in  'Font::init()', cannot init SDL_TTF : ")+ SDL_GetError());

			m_is_init = false;
			return false;
		}
		else
			SDL_TTF__init_by_gs_Font = true;
	}


	{
		m_default_type 			= default_type;
		m_ptsize 				= ptsize;
	}


	m_SDL_font = TTF_OpenFont(font_path.c_str(), ptsize);

	if(m_SDL_font == nullptr)
	{
		gs::lastError(std::string("||Error|| in 'Font::init()', TTF_OpenFont failed : ")
							+ SDL_GetError()
							+ std::string(". [path : \"")+font_path+"\"] ");

		m_is_init = false;
		return false;
	}

	TTF_SetFontHinting(m_SDL_font, TTF_HINTING_NORMAL);

	m_ren_ref = ren;
	m_is_init = true;
	m_font_style = TTF_GetFontStyle(m_SDL_font);
	return true;
}



bool Font::try_changeFontSource(const std::string& target_font_path)
{
	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Font::try_changeFontSource()', this 'gs::Font' is not yet init");
		return false;
	}

	BK_Font_t* try_font = TTF_OpenFont(target_font_path.c_str(), m_ptsize);

	if(not try_font)
	{
		gs::lastError(std::string("||Error|| in 'Font::try_changeFontSource()', TTF_OpenFont failed : ")
							+ SDL_GetError()
							+ std::string(". [path : \"")+target_font_path+"\"] ");
		return false;
	}

	if(m_SDL_font)
		TTF_CloseFont(m_SDL_font);

	m_SDL_font = try_font;

	return true;
}


void Font::setPtsize(size_t ptsize)
{
	if(not m_is_init) return;
	if(ptsize < 1) ptsize = 1;

	TTF_SetFontSize(m_SDL_font, ptsize);
	m_ptsize = ptsize;
}



gs::Vec2i Font::getTextPixelSize(const std::string& str) //const
{
	Vec2i output{0,0};

	if(not m_is_init)
		return output;

	TTF_GetStringSize(m_SDL_font, str.c_str(), str.length(), &output.x, &output.y);

	if(str == " " and m_wider_space_token_mode)
		output.x = output.x*m_wider_space_factor;

	return output;
}


BK_Font_t* Font::getFont() {return m_SDL_font;}


int Font::getFontStyle() const {return m_font_style;}

void Font::setFontStyle(int new_font_style)
{
	m_font_style = new_font_style;
	TTF_SetFontStyle(m_SDL_font, m_font_style);
}



Surface* Font::hackRenderedWidth(Surface* src, float scale_factor) const //Ce hack permet d'étirer le str généré.
{
	if(not src)
		return nullptr;

	//Pré-traitement purement technique
	if(src->format == SDL_PIXELFORMAT_INDEX8)
	{
		//Après usage, on observe que le format SDL_PIXELFORMAT_INDEX8 n'est pas supporté par 'SDL_BlitSurfaceScaled()' dans la version où j'écris ceci (SDL 3.4.4)
		//Alors : tentative de conversion de la surface dans un autre format.
		Surface* converted_surface = SDL_ConvertSurface(src, SDL_PIXELFORMAT_RGBA8888);
		if(not converted_surface)
		{
			gs::lastError("||Warning|| in 'Font::hackRenderedWidth(), src->format == SDL_PIXELFORMAT_INDEX8, so tried to \"convert surface format\", but error occured, [ SDL_ConvertSurface() ] : ", SDL_GetError());
			return src;
		}

		//On libère l'encienne surface et récupère la nouvelle
		SDL_DestroySurface(src);
		src = converted_surface;
	}

	int dest_w = src->w*scale_factor;
	int dest_h = src->h;

	Surface* dest=nullptr;
	dest = SDL_CreateSurface(dest_w, dest_h, src->format);

	if(not dest)
	{
		gs::lastError("||Warning|| in 'Font::hackRenderedWidth(), SDL_CreateSurface() for output failed : ", SDL_GetError());
		return src;
	}

	if(not SDL_BlitSurfaceScaled(src, nullptr, dest, nullptr, (SDL_ScaleMode)m_scale_mode))
	{
		gs::lastError("||Warning|| in 'Font::hackRenderedWidth(), SDL_BlitSurfaceScaled() failed : ", SDL_GetError());
		//std::cout << SDL_GetPixelFormatName(src->format) << " src fmt\n";
		//std::cout << SDL_GetPixelFormatName(dest->format) << " dst fmt\n";
		SDL_DestroySurface(dest); //Important
		return src;
	}

	SDL_DestroySurface(src); //Important

	return dest;
}




Texture* Font::render(const std::string& text, Color text_color,te::Font_Rendering_Type rendering_type,
						Color* bg_color,
						Vec2f* text_size_out) // const
{
	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Font::render()', cannot render : not yet init");
		return nullptr;
	}

	if(m_ren_ref == nullptr)
	{
		gs::lastError("||Warning|| in 'Font::render()' cannot do without a valid renderer (try to set a valid render)");
		return nullptr;
	}

	te::Font_Rendering_Type type = rendering_type;

	if(type == te::SELF_DEFAULT)
		type = m_default_type;

	Color bg_color_cpy = m_bg_color;
	if(bg_color == nullptr)
		bg_color = &bg_color_cpy;


	Surface* surface = nullptr;

	switch(type)
	{
		case te::SOLID: surface = TTF_RenderText_Solid(m_SDL_font, text.c_str(), 0, text_color); break;
		case te::SHADED: surface = TTF_RenderText_Shaded(m_SDL_font, text.c_str(), 0, text_color, *bg_color); break;
		case te::BLENDED: surface = TTF_RenderText_Blended(m_SDL_font, text.c_str(), 0, text_color); break;
		case te::DEFAULT: surface = TTF_RenderText_Solid(m_SDL_font, text.c_str(), 0, text_color); break;
		case te::SELF_DEFAULT: surface = TTF_RenderText_Solid(m_SDL_font, text.c_str(), 0, text_color); break;

		default :
			gs::lastError("||Error|| in 'Font::render()' invalid rendering type. It may be an internal error");
			return nullptr;
	}

	if(surface == nullptr)
	{
		gs::lastError(std::string("||Error|| in 'Font::render()', create text surface failed : ")+ SDL_GetError());
		return nullptr;
	}


	if(not m_show_background)
	{
		if(type == te::SHADED)
		{
			bool res = SDL_SetSurfaceColorKey(surface, true, ((uint32_t*)surface->pixels)[0]);

			if(not res)
				gs::lastError(std::string("||Warning|| in 'Font::render()', cannot hide background, error occured : ") + SDL_GetError());
		}
	}


	if(text == " " and m_wider_space_token_mode)
		surface = hackRenderedWidth(surface, m_wider_space_factor);


	if(m_userInterventionCallback)
		m_userInterventionCallback(surface, text);


	Texture* textTexture = SDL_CreateTextureFromSurface(m_ren_ref, surface);

	if(textTexture == nullptr)
	{
		gs::lastError(std::string("||Error|| in 'Font::render()', create texture failed : ")+ SDL_GetError());
		return nullptr;
	}

	SDL_SetTextureBlendMode(textTexture, (SDL_BlendMode)m_blend_mode);
	SDL_SetTextureScaleMode(textTexture, (SDL_ScaleMode)m_scale_mode);


	if(text_size_out != nullptr)
	{
		text_size_out->x = surface->w;
		text_size_out->y = surface->h;
	}

	SDL_DestroySurface(surface);

	addToClearAtTheEnd(textTexture);
	return textTexture;
}





Texture* Font::renderMultiLines(const std::vector<std::string>& text, Color text_color,
					te::Paragraph_Lines_Type paragraph_lines_type,
					te::Font_Rendering_Type rendering_type,
					Color* bg_color,
					Vec2f* text_size_out) //const
{

	if(not m_is_init)
	{
		gs::lastError("||Warning|| in 'Font::renderMultiLines()', cannot render : not yet init");
		return nullptr;
	}

	if(m_ren_ref == nullptr)
	{
		gs::lastError("||Warning|| in 'Font::renderMultiLines()', cannot do without a valid renderer (try to set a valid render)");
		return nullptr;
	}

	if(std::empty(text))
	{
		gs::lastError("||Warning|| in 'Font::renderMultiLines()', given empty text. ");
		return nullptr;
	}


	int max_w=0;
	int max_h=0;
	int w_buf=0;
	int line_y_space = TTF_GetFontLineSkip(m_SDL_font); //Espace recommandé entre les lignes.

	//Détermination de la taille totale.
	for(const std::string& text_line : text)
	{
		TTF_GetStringSize(m_SDL_font, text_line.c_str(), 0, &w_buf, nullptr);

		if(w_buf > max_w)
			max_w = w_buf;

		max_h += line_y_space;
	}

	Texture* return_texture=nullptr;
	return_texture = SDL_CreateTexture(m_ren_ref, SDL_PIXELFORMAT_RGBA8888,
									SDL_TEXTUREACCESS_TARGET, max_w, max_h);

	if(not return_texture)
	{
		gs::lastError(std::string("||Warning|| in 'Font::renderMultiLines()', create texture failed : ")+SDL_GetError());
		return nullptr;
	}




	Texture* previousTarget = SDL_GetRenderTarget(m_ren_ref);
	SDL_SetRenderTarget(m_ren_ref, return_texture);


	//Afin de rendre la texture transparente.
	{
		SDL_SetTextureBlendMode(return_texture, SDL_BLENDMODE_BLEND);

		Color previousC;
	    SDL_GetRenderDrawColor(m_ren_ref, &previousC.r,&previousC.g,&previousC.b,&previousC.a);
	    SDL_SetRenderDrawColor(m_ren_ref, 0, 0, 0, 0); //C'est le dernier zéro qui compte.
	   	SDL_RenderFillRect(m_ren_ref, nullptr);
	    SDL_SetRenderDrawColor(m_ren_ref, previousC.r,previousC.g,previousC.b,previousC.a);
	}


	short current_y=0;
	Texture* tmp_line_texture=nullptr;

	for(const std::string& text_line : text)
	{
		Vec2f size_buf;
		tmp_line_texture = render(text_line.c_str(), text_color,
							rendering_type, bg_color, &size_buf);

		if(not tmp_line_texture)
			goto faillure;

		FRect dest_rect = {0.f, static_cast<float>(current_y), size_buf.x, size_buf.y};

		switch(paragraph_lines_type) {

			case te::LEFT_BEGIN:
				break; // car (x == 0) == true

			case te::RIGHT_BEGIN:
				dest_rect.x = max_w - dest_rect.w;
				break;

			case te::CENTERED:
				dest_rect.x = max_w/2 - dest_rect.w/2;
				break;

			default:
				break;
		}

		bool res = SDL_RenderTexture(m_ren_ref, tmp_line_texture, nullptr, &dest_rect);

		SDL_DestroyTexture(tmp_line_texture);
		tmp_line_texture = nullptr;

		if(!res) {
			gs::lastError(SDL_GetError());
			goto faillure;
		}

		current_y += line_y_space;
	}


	SDL_SetRenderTarget(m_ren_ref, previousTarget);


	if(text_size_out) {
		text_size_out->x = max_w;
		text_size_out->y = max_h;
	}

	SDL_SetTextureScaleMode(return_texture, (SDL_ScaleMode)m_scale_mode);

	return return_texture;


	faillure:

	gs::lastError(std::string("from 'Font::renderMultiLines()' : ")+gs::lastError());

	SDL_SetRenderTarget(m_ren_ref, previousTarget);

	if(return_texture)
		SDL_DestroyTexture(return_texture);
	if(tmp_line_texture)
		SDL_DestroyTexture(tmp_line_texture);


	return nullptr;
}






bool Backend__Text_Window::processEvent(SDL_Event& event, SDL_Window* win)
{
	//Récupération des membres requis ---------------------------------
	INTERNAL__membersTransfer mb;
	INTERNAL_USE_getMembersPtrs_forEventProcessing(mb);
	if( !mb.m_is_init or
		!mb.m_mouse_tracker or
		!mb.m_check_mouse or
		!mb.m_mouse_clicked_pos or
		!mb.m_wheel or
		!mb.m_curr_active_line_idx or
		!mb.m_inputs)
	{
		gs::lastError("||INTERAL|| : in 'Backend__Text_Window::processEvent()', after 'INTERNAL_USE_getMembersPtrs_forEventProcessing()', have invalid ptr.");
		return false;
	}
	bool& m_is_init = *mb.m_is_init;
	Mouse_Tracker& m_mouse_tracker = *mb.m_mouse_tracker;
	bool& m_check_mouse = *mb.m_check_mouse;
	Vec2<int>& m_mouse_clicked_pos = *mb.m_mouse_clicked_pos;
	te::Wheel_Details& m_wheel = *mb.m_wheel;
	int& m_curr_active_line_idx = *mb.m_curr_active_line_idx;
	Input& m_inputs = *mb.m_inputs;
	//------------------------------------------------------------------



	if(not m_is_init) return false;

	if(not SDL_TextInputActive(win))
		SDL_StartTextInput(win);

	m_mouse_tracker.processEvent(event);

	if(m_check_mouse)
	{
		if(event.type == SDL_EVENT_MOUSE_BUTTON_DOWN)
		{
			if(event.button.button == SDL_BUTTON_LEFT)
			{
				m_mouse_clicked_pos.x = event.button.x;
				m_mouse_clicked_pos.y = event.button.y;
				m_inputs[te::MOUSE_CLICKED, true];
				//m_inputs[MOUSE_IS_DOWN, true];


			}
		}
		else if(event.type == SDL_EVENT_MOUSE_BUTTON_UP)
		{
			//m_inputs[MOUSE_IS_DOWN, false];
		}
		else if(event.type == SDL_EVENT_MOUSE_WHEEL)
		{
			m_wheel.wheeled = true;
			m_wheel.val = event.wheel.y;
		}

	}

	if(m_curr_active_line_idx == te::NO_ACTIVE_LINE) return false; //Pour le clavier on ne traite que s'il y a une ligne active.

	if(event.type == SDL_EVENT_TEXT_INPUT)
	{
		m_inputs[str(event.text.text), true];
	}
	else if(event.type == SDL_EVENT_KEY_DOWN)
	{
		switch(event.key.key)
		{
			case SDLK_BACKSPACE : m_inputs["\b", true]; break;
		    case SDLK_RETURN 	: m_inputs["\n", true]; break;
		    case SDLK_KP_ENTER 	: m_inputs["\n", true]; break;
		    case SDLK_DELETE    : m_inputs[te::DELETE, true]; break;
			case SDLK_TAB 		:
				if(not(m_inputs[te::LEFT_SHIFT] or m_inputs[te::RIGHT_SHIFT]))
					m_inputs["\t", true];
				else
					m_inputs[te::BACK_TAB, true];
				break;



		    case SDLK_UP : m_inputs[te::UP_ARROW, true]; break;
		    case SDLK_DOWN : m_inputs[te::DOWN_ARROW, true]; break;
		    case SDLK_LEFT : m_inputs[te::LEFT_ARROW, true]; break;
		    case SDLK_RIGHT : m_inputs[te::RIGHT_ARROW, true]; break;



			case SDLK_LSHIFT : m_inputs[te::LEFT_SHIFT, true]; break;
			case SDLK_RSHIFT : m_inputs[te::RIGHT_SHIFT, true]; break;

			default:break;
		}

	}
	else if(event.type == SDL_EVENT_KEY_UP)
	{
		switch(event.key.key)
		{
			case SDLK_LSHIFT : m_inputs[te::LEFT_SHIFT, false]; break;
			case SDLK_RSHIFT : m_inputs[te::RIGHT_SHIFT, false]; break;

			default:break;
		}

	}

	return true;
}









//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________WIDGET__________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________
//____________________________________________________________________________________

std::vector<Button*>& INTERNAL_USE_getButtonsVec();
std::vector<Scrollbar*>& INTERNAL_USE_getScrollbarsVec();

void widgetsProcessEvent(SDL_Event& ev)
{
	std::vector<Button*>& buttons_vec = INTERNAL_USE_getButtonsVec();
	std::vector<Scrollbar*>& scrollbars_vec = INTERNAL_USE_getScrollbarsVec();

	FPoint mouse_curr_pos;

	SDL_GetMouseState(&mouse_curr_pos.x, &mouse_curr_pos.y);


	for(Button* btn : buttons_vec)
	{
		if(not btn)
			continue;

		Rect btn_area = rect(btn->getArea());
		Point mouse_curr_pos_i = Point(static_cast<int>(mouse_curr_pos.x), static_cast<int>(mouse_curr_pos.y));

		if(SDL_PointInRect(&mouse_curr_pos_i, &btn_area))
		{
			INTERNAL_USE_editState(*btn, wi::MOUSE_IN, true);

			if(ev.type == btn->getTargetEvent())
			{
				if(ev.button.button == btn->getTargetButtonEvent())
					INTERNAL_USE_editState(*btn, wi::CLICKED_IN, true);
				else
					INTERNAL_USE_editState(*btn, wi::CLICKED_IN, false);

			}
			else
				INTERNAL_USE_editState(*btn, wi::CLICKED_IN, false);

		}
		else
		{
			INTERNAL_USE_editState(*btn, wi::MOUSE_IN, false);
			INTERNAL_USE_editState(*btn, wi::CLICKED_IN, false);
		}
	}


	for(Scrollbar* bar : scrollbars_vec)
		if(bar)
			bar->processEvent(ev);

}




void Backend__Scrollbar::processEvent(const SDL_Event& ev)
{
	//Récupération des membres requis ---------------------------------
	INTERNAL__membersTransfer mb;
	INTERNAL_USE_getMembersPtrs_forEventProcessing(mb);
	if( !mb.m_mouse_clicked_in or
		!mb.m_orient or
		!mb.m_to_move_buf
		)
	{
		gs::lastError("||INTERAL|| : in 'Backend__Scrollbar::processEvent()', after 'INTERNAL_USE_getMembersPtrs_forEventProcessing()', have invalid ptr.");
		return;
	}
	bool& m_mouse_clicked_in = *mb.m_mouse_clicked_in;
	wi::Scrollbar_Orient& m_orient = *((wi::Scrollbar_Orient*)mb.m_orient);
	short& m_to_move_buf = *mb.m_to_move_buf;
	//------------------------------------------------------------------


	static Rect area;
	static Rect moveable_thing_area;

	//Définition de la zone globale de la scrollbar, et de l'élément amovible.
	setAreaRects(area, moveable_thing_area);


	if(ev.type == SDL_EVENT_MOUSE_BUTTON_UP and ev.button.button == SDL_BUTTON_LEFT)
		m_mouse_clicked_in = false;
	else
	if(ev.type == SDL_EVENT_MOUSE_BUTTON_DOWN and ev.button.button == SDL_BUTTON_LEFT)
	{
		Point click_pos={static_cast<int>(ev.button.x), static_cast<int>(ev.button.y)};
		if(SDL_PointInRect(&click_pos, &moveable_thing_area))
			m_mouse_clicked_in = true;
	}
	else
	if(m_mouse_clicked_in)
	{
		if(ev.type == SDL_EVENT_MOUSE_MOTION)
		{
			if(m_orient == wi::VERTICAL)
				m_to_move_buf = ev.motion.yrel;
			else
				m_to_move_buf = ev.motion.xrel;
		}
	}

}










bool Trail::draw(Renderer* ren, Texture* texture, Color tint, const Camera2D_Base& cam)
{
    Trail_Vertices d=getVertices(tint, cam);

    return SDL_RenderGeometry(ren, texture, d.vertices.data(), d.vertices.size(), d.indices.data(), d.indices.size());
}




} //namespace gs
