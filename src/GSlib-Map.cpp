//#define USE_RAYLIB_GSlib // à décommenter ou rajouter au niveau de la compilation au besoin.
#include "GSlib-Map.hpp"
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <format>
#include <cassert>

//Version modifiée le 16/07/26 (à moins d'oubli de modifier cette ligne ou autre).



namespace fs = std::filesystem;

namespace gs
{

using namespace ma;


void Tile_Anim::initBegin()
{
	frame_timer=one_frame_duration;
	curr_frame=0;
}

short Tile_Anim::getCurrFrameTileVal(bool& is_ok) const
{
	if(curr_frame >= 0 and curr_frame < frames.size())
	{
		short val = frames.at(curr_frame);
		is_ok = true;
		return val;
	}

	is_ok = false;
	return tile_val; //Cas d'erreur
}

void Tile_Anim::update(float dt)
{
	if(frame_timer <= 0.f) //Si le temps d'une frame s'est écoulé, on passe à la suivante, ou au début si l'on était à la dernière.
	{
		frame_timer = one_frame_duration; //Réinitialisation du timer.

		short max_frame = short(frames.size())-1; //en indice

		if(curr_frame == max_frame)//Si l'on était à la dernière frame
			curr_frame = 0; //On retourne au début
		else
		if(curr_frame+1 == max_frame) //Si l'on était à l'avant dernière frame
		{
			curr_frame++; //On passe à la dernière frame,
			frame_timer += new_cycle_delay; //et pour celle-ci on compte, en plus de sa durée, la durée d'attente avant de passer à un nouveau cycle.
		}
		else
			curr_frame++; //Pour les autres frames on avance simplement au suivant.

		//Note: dans le cas où il n'y aurait qu'une seule frame dans l'animation (ce qui est à mon avis un cas d'usage peu fréquent, néanmoins techniquement possible), et bien, le 'new_cycle_delay' n'est pas pris en compte d'après cette implémentation, mais cela est tout à fait OK car qu'il soit pris en compte ou non, la frame (tuile) à afficher reste la même.
	}

	frame_timer -= dt; //Et dans tous les cas il est important de prendre en compte le temps écoulé, que l'on vienne de passer à la frame suivante ou non.
}




bool Tileset::load(Renderer* ren, const std::string& pth, const Tileset_Loading_Args& tl_args)
{
	Tileset_Loading_Out_Info info; //Pour récupérer certaines infos sur l'atlas produit.
	Texture* new_atlas = genAtlas(ren, pth, tl_args, &info); //Génération d'un atlas de tuiles, à partir de l'atlas de tuiles attendu dans le fichier image à "pth". Ceci sert en particulier à pouvoir rajouter un padding entre les tuiles si l'atlas original n'en a pas (c'est en lutte contre le 'texture-bleeding').

	if(not new_atlas)
	{
		gs::lastError("from 'Tileset::load()' : ", gs::lastError());
		return false;
	}

	load__finalStep(new_atlas, tl_args, info);
	return true;
}


bool Tileset::load(Renderer* ren, Surface* src_img, const Tileset_Loading_Args& tl_args)
{
	Tileset_Loading_Out_Info info; //Pour récupérer certaines infos sur l'atlas produit.
	Texture* new_atlas = genAtlas(ren, src_img, tl_args, &info); //Génération d'un atlas de tuiles, à partir de l'atlas de tuiles attendu dans le fichier image à "pth". Ceci sert en particulier à pouvoir rajouter un padding entre les tuiles si l'atlas original n'en a pas (c'est en lutte contre le 'texture-bleeding').

	if(not new_atlas)
	{
		gs::lastError("from 'Tileset::load()' : ", gs::lastError());
		return false;
	}

	load__finalStep(new_atlas, tl_args, info);
	return true;
}


void Tileset::load__finalStep(Texture* new_atlas, const Tileset_Loading_Args& tl_args, const Tileset_Loading_Out_Info& info)
{
	if(m_atlas)
		gs::destroyTexture(m_atlas);

	m_atlas = new_atlas;
	m_tile_src_size = tl_args.tile_src_size;
	m_number_of_columns = info.nb_of_columns;
	m_tile_src_padding = info.used_padding;	
}




Texture* Tileset::genAtlas(Renderer* ren, const std::string& pth, const Tileset_Loading_Args& tl_args, Tileset_Loading_Out_Info* out)
{
	Surface* tmp = gs::loadSurface(pth);

	if(not tmp)
	{
		gs::lastError("from 'Tileset::genAtlas(path)' : ", gs::lastError());
		return nullptr;
	}

	Texture* out_atlas_texture = genAtlas(ren, tmp, tl_args, out);
	gs::destroySurface(tmp);

	if(not out_atlas_texture)
		gs::lastError("from 'Tileset::genAtlas(path)' : ", gs::lastError());

	return out_atlas_texture;
}


Texture* Tileset::genAtlas(Renderer* ren, Surface* src_img, const Tileset_Loading_Args& tl_args, Tileset_Loading_Out_Info* out)
{
	//______________Les vérifications_________________________________________________
	{
		if(not src_img)
		{
			gs::lastError("||Warning|| in 'Tileset::genAtlas(src_img)', invalid given image ptr.");
			return nullptr;
		}

		if(tl_args.tile_src_size.x <= 0 or tl_args.tile_src_size.y <= 0)
		{
			gs::lastError("||Warning|| in 'Tileset::genAtlas(src_img)', invalid given tile src size, must not be empty, nor negative.");
			return nullptr;
		}

		Vec2f s = gs::getSurfaceSize(src_img);
		if(s.x < tl_args.tile_src_size.x or s.y < tl_args.tile_src_size.y)
		{
			gs::lastError("||Warning|| in 'Tileset::genAtlas(src_img)', given image is too small according to the given tile size.");
			return nullptr;
		}

		if(tl_args.src_padding_to_add < 0)
		{
			gs::lastError("||Warning|| in 'Tileset::genAtlas(src_img)', invalid given padding to add, must be >= 0.");
			return nullptr;
		}
	}
	//_____________________________________________________________________________


	//Si aucun padding n'est demandé, l'atlas est alors déjà prêt.
	if(tl_args.src_padding_to_add == 0)
	{
		Texture* out_atlas_texture = gs::surfaceToTexture(ren, src_img);

		if(not out_atlas_texture)
			gs::lastError("from 'Tileset::genAtlas(src_img)' : ", gs::lastError());
		else
		{
			gs::addToClearAtTheEnd(out_atlas_texture);

			if(out)
			{
				out->nb_of_columns = int(gs::getTextureSize(out_atlas_texture).x)/tl_args.tile_src_size.x;
				out->used_padding = tl_args.src_padding_to_add;
			}
		}

		return out_atlas_texture;
	}

	
	int padding_to_add = tl_args.src_padding_to_add;
	if(padding_to_add > GS_TILESET_ATLAS_MAX_SRC_PADDING)
	{
		gs::lastError("||Info|| in 'Tileset::genAtlas(src_img)', given padding is larger than allowed, so, max allowed padding will be used instead.");
		padding_to_add = GS_TILESET_ATLAS_MAX_SRC_PADDING;
	}


	//Calcul de la taille de l'atlas dont le padding est pris en compte ----------------------------
	
	//D'abord, on détermine combien il y a de tuile effectives (complètes) dans l'image reçue
	Vec2i src_img_size = gs::getSurfaceSize(src_img).to<int>();
	Vec2i nb_of_tiles = src_img_size / tl_args.tile_src_size; // Note : les zéros sont prévenus par les vérifications plus haut.
	Vec2i new_cell_size = tl_args.tile_src_size + Vec2i(padding_to_add*2, padding_to_add*2); //On rajoute à la taille d'une tuile deux fois le padding car le padding est sur le côté min, et, sur le côté max.

	Vec2i new_atlas_size = new_cell_size * nb_of_tiles; //'new_cell_size' est l'ensemble de l'espace occupé par une tuile dans le nouvel atlas (taille de la tuile + 2*padding).
	

	//Création du buffer image du nouvel atlas -----------------------------------------------------

	Surface* atlas_buf = gs::createSurface(new_atlas_size, {0,0,0,0});
	
	if(not atlas_buf)
	{
		gs::lastError("from 'Tileset::genAtlas(src_img)' [when creating atlas buffer] : ", gs::lastError());
		return nullptr;
	}


	//Copie des tuiles dans le buffer ---------------------------------------------------------------
	
	for(int row = 0; row < nb_of_tiles.y; row++) //Parcours des tuiles
	for(int column = 0; column < nb_of_tiles.x; column++)
	{
		Rect src_rect{column*tl_args.tile_src_size.x, row*tl_args.tile_src_size.y,
					tl_args.tile_src_size.x, tl_args.tile_src_size.y}; //Là où l'on prend la tuile dans l'atlas d'origine.

		Rect dest_rect{column*new_cell_size.x +padding_to_add, row*new_cell_size.y +padding_to_add,
					tl_args.tile_src_size.x, tl_args.tile_src_size.y}; //Là où l'on va copier la tuile dans le nouvel atlas

		gs::drawSurfaceToSurface(src_img, &src_rect, atlas_buf, &dest_rect);

		if(tl_args.duplicate_borders) //Gestion de ce qu'il y a dans le padding
		{
			//Bordure gauche
			Rect border_src{src_rect.x, src_rect.y, 1, src_rect.h}; //'border_src' sert à contenir, pour les quatre côtés chacun à son tour, la bande de pixels du bord. (bande récupérée depuis 'src_img')
			Rect border_dest{dest_rect.x -padding_to_add, dest_rect.y, padding_to_add, dest_rect.h}; //'border_dest' sert à contenir, pour les quatre côtés chacun à son tour, la zone où sera dessinée la bande (à savoir sur le padding) dans 'atlas_buf'.
			gs::drawSurfaceToSurface(src_img, &border_src, atlas_buf, &border_dest);

			//Bordure droite
			border_src.x += src_rect.w-1; //attention ces deux modifications impilquent que la bordure gauche soit celle traitée avant celle-ci.
			border_dest.x = dest_rect.x+dest_rect.w;
			gs::drawSurfaceToSurface(src_img, &border_src, atlas_buf, &border_dest);

			//Bordure haute
			border_src = Rect(src_rect.x, src_rect.y, src_rect.w, 1);
			border_dest = Rect(dest_rect.x, dest_rect.y -padding_to_add, dest_rect.w, padding_to_add);
			gs::drawSurfaceToSurface(src_img, &border_src, atlas_buf, &border_dest);

			//Bordure basse
			border_src.y += src_rect.h-1; //attention ces deux modifications impilquent que la bordure haute soit celle traitée avant celle-ci.
			border_dest.y = dest_rect.y+dest_rect.h;
			gs::drawSurfaceToSurface(src_img, &border_src, atlas_buf, &border_dest);


			//Enfin, il reste à combler les coins.

			//Coin supérieur gauche
			border_src = Rect(src_rect.x, src_rect.y, 1, 1);
			border_dest = Rect(dest_rect.x -padding_to_add, dest_rect.y -padding_to_add, padding_to_add, padding_to_add);
			gs::drawSurfaceToSurface(src_img, &border_src, atlas_buf, &border_dest);

			//Coin supérieur droit
			border_src.x += src_rect.w-1;
			border_dest.x = dest_rect.x + dest_rect.w;
			gs::drawSurfaceToSurface(src_img, &border_src, atlas_buf, &border_dest);

			//Coin inférieur droit
			border_src.y += src_rect.h-1;
			border_dest.y = dest_rect.y+dest_rect.h;
			gs::drawSurfaceToSurface(src_img, &border_src, atlas_buf, &border_dest);

			//Coin inférieur gauche
			border_src.x = src_rect.x;
			border_dest.x = dest_rect.x -padding_to_add;
			gs::drawSurfaceToSurface(src_img, &border_src, atlas_buf, &border_dest);
		}	
	}


	Texture* out_atlas_texture = gs::surfaceToTexture(ren, atlas_buf);
	gs::destroySurface(atlas_buf);

	if(not out_atlas_texture)
	{
		gs::lastError("from 'Tileset::genAtlas(src_img)' [final stage : surface to texture] : ", gs::lastError());
		return nullptr;
	}


	if(out)
	{
		out->nb_of_columns = nb_of_tiles.x;
		out->used_padding = padding_to_add;
	}

	gs::addToClearAtTheEnd(out_atlas_texture);
	return out_atlas_texture;
}




short Tileset::getCurrentFrameOf(short tile_val) const
{
	bool skip_arg;
	std::unordered_map<short, Tile_Anim>::const_iterator c_it = animated_tiles.find(tile_val);
	if(c_it != animated_tiles.end())
		return c_it->second.getCurrFrameTileVal(skip_arg); //S'il y a erreur, je laisse faire le comportement de 'Tile_Anim', d'où le fait de ne pas traiter ici le 'is_ok'.

	return tile_val; //Si la tuile n'est pas dans les animations, elle reste inchangée.
}



Vec2i Tileset::getTileSrcPos(short tile_val) const
{
	if(m_number_of_columns <= 0) return Vec2i(-1,-1); //En cas d'erreur

	Vec2i tile_cell_size = m_tile_src_size + m_tile_src_padding*2; //La taille utile + la taille due au padding = taille totale d'une tuile dans l'atlas
	Vec2i tile_pos;
	tile_pos.x = (tile_val % m_number_of_columns) * tile_cell_size.x +m_tile_src_padding;
	tile_pos.y = (tile_val / m_number_of_columns) * tile_cell_size.y +m_tile_src_padding;

	return tile_pos;
}




Collision_Mask::Collision_Mask(std::vector<std::vector<char>>& grid)
: m_grid(grid)
{
	//Histoire d'éviter que la grille ne soit pas rectangulaire, on met tout le monde à la même taille.
	size_t max_w=0;
	for(std::vector<char>& line : m_grid) //Premier parcours pour déterminer la longueur de la ligne la plus grande
		max_w = GetMax(line.size(), max_w);

	for(std::vector<char>& line : m_grid) //Deuxième parcours pour mettre à niveau celles qui ne sont pas à cette taille.
		if(line.size() < max_w)
			line.resize(max_w, 0); //Avec des zéros pour ne pas rajouter des collisions là où il y en a pas.

	//C'est par sécurité, pour uniformiser le tableau afin d'en faciliter le traitement.
}

bool Collision_Mask::isCollisionTile(int x, int y) const //Les coo invalides seront comptées comme non collision.
{
	//Vérifications de la validité des coordonnées reçues.
	if(m_grid.empty())
		return false;

	int size_y = m_grid.size();
	if(y < 0 or y >= size_y) return false;

	int size_x = m_grid.at(y).size();
	if(x < 0 or x >= size_x) return false;

	return m_grid.at(y).at(x) != 0;
}

Vec2i Collision_Mask::getTilesCount() const
{
	if(m_grid.empty())
		return Vec2i(0,0);

	return Vec2i(m_grid.at(0).size(), m_grid.size());
}












bool Background::incrStartX(float val)
{
	double coef = m_sensibility.x / 100.f;
	val *= coef;

	m_start.x+= val;
	m_param.current_pos.x = m_param.begin_pos.x + m_start.x;


	if(std::abs(m_start.x) >= m_param.current_dest_rect.w)
		m_start.x = 0;

	return true;
}


bool Background::incrStartY(float val)
{
	double coef = m_sensibility.y / 100.f;
	val *= coef;

	m_start.y+= val;
	m_param.current_pos.y = m_param.begin_pos.y + m_start.y;


	if(std::abs(m_start.y) >= m_param.current_dest_rect.h)
		m_start.y = 0;

	return true;
}



/******************************************************************************************/





















bool Map_Layer::initTheMapArray()
{
	if(m_is_init)
	{
		gs::lastError("||Warning|| in 'Map_Layer::initTheMapArray()', already initialized");
		return false;
	}

	m_map_array.clear();
	m_map_array.resize(m_tiles_count.y);

	for(int i=0; i< m_tiles_count.y; i++)
	{
		m_map_array[i].resize(m_tiles_count.x);
	}

	m_is_init = true;
	return m_is_init;
}




void Map_Layer::setWorldStartX(float val, short dest_bloc, unsigned short render_w)
{
	/*on prend le pourcentage de la sensibilité*/
	float coef = m_sensibility.x/100.f;
	val *= coef;
	m_world_start.x = val;

	{
		float max_start = m_tiles_count.x * dest_bloc;
			max_start -= render_w;

		if(m_world_start.x >= max_start)
		{
			m_world_start.x = max_start;
			m_right_scrolling_limit_reached=true;
		}
		else
			m_right_scrolling_limit_reached=false;

		if(m_world_start.x <= 0)
		{
			m_world_start.x = 0;
			m_left_scrolling_limit_reached=true;
		}
		else
			m_left_scrolling_limit_reached=false;

		if(m_sensibility.x == 0)
		{
			//Afin que ceux qui ne bougent pas quoi qu'il arrive sur l'instant, ne bloquent pas les autres si la synchronisation est activée.
			// m_up_scrolling_limit_reached=false;
			// m_down_scrolling_limit_reached=false;
			m_left_scrolling_limit_reached=false;
			m_right_scrolling_limit_reached=false;
		}

	}

	//removed updateLinkedBodiesPos(true, false);
}

void Map_Layer::setWorldStartY(float val, short dest_bloc, unsigned short render_h)
{
	float coef = m_sensibility.y/100.f;
	val *= coef;
	m_world_start.y = val;

	{
		float max_start = m_tiles_count.y * dest_bloc;
			max_start -= render_h;

		if(m_world_start.y >= max_start)
		{
			m_world_start.y = max_start;
			m_down_scrolling_limit_reached=true;
		}
		else
			m_down_scrolling_limit_reached=false;

		if(m_world_start.y <= 0)
		{
			m_world_start.y = 0;
			m_up_scrolling_limit_reached=true;
		}
		else
			m_up_scrolling_limit_reached=false;

		if(m_sensibility.y == 0)
		{
			m_up_scrolling_limit_reached=false;
			m_down_scrolling_limit_reached=false;
			// m_left_scrolling_limit_reached=false;
			// m_right_scrolling_limit_reached=false;
		}
	}

	//removed updateLinkedBodiesPos(false, true);
}

void Map_Layer::incrWorldStartX(float val, short dest_bloc, unsigned short render_w)
{
	float coef = m_sensibility.x/100.f;
	val *= coef;
	m_world_start.x += val;

	{
		float max_start = m_tiles_count.x * dest_bloc;
			max_start -= render_w;

		if(m_world_start.x >= max_start)
		{
			m_world_start.x = max_start;
			m_right_scrolling_limit_reached=true;
		}
		else
			m_right_scrolling_limit_reached=false;

		if(m_world_start.x <= 0)
		{
			m_world_start.x = 0;
			m_left_scrolling_limit_reached=true;
		}
		else
			m_left_scrolling_limit_reached=false;

		if(m_sensibility.x == 0)
		{
		// 	m_up_scrolling_limit_reached=false;
		// 	m_down_scrolling_limit_reached=false;
			m_left_scrolling_limit_reached=false;
			m_right_scrolling_limit_reached=false;
		}
	}

	//removed updateLinkedBodiesPos(true, false);
}


void Map_Layer::incrWorldStartY(float val, short dest_bloc, unsigned short render_h)
{
	float coef = m_sensibility.y/100.f;
	val *= coef;
	m_world_start.y += val;

	{
		float max_start = m_tiles_count.y * dest_bloc;
			max_start -= render_h;

		if(m_world_start.y >= max_start)
		{
			m_world_start.y = max_start;
			m_down_scrolling_limit_reached=true;
		}
		else
			m_down_scrolling_limit_reached=false;

		if(m_world_start.y <= 0)
		{
			m_world_start.y = 0;
			m_up_scrolling_limit_reached=true;
		}
		else
			m_up_scrolling_limit_reached=false;

		if(m_sensibility.y == 0)
		{
			m_up_scrolling_limit_reached=false;
			m_down_scrolling_limit_reached=false;
			// m_left_scrolling_limit_reached=false;
			// m_right_scrolling_limit_reached=false;
		}
	}

	//removed updateLinkedBodiesPos(false, true);
}








/******************************************************************************************/





















Map::Map(Renderer* ren, Vec2i dest_bloc, std::string title, short idle_value)
: 	m_ren(ren), m_destBLOC_custom_w_h(dest_bloc), m_title(title), m_idle_value(idle_value), m_is_init(true)
{
	/*Constructeur le plus complet en terme de renseignements*/
}



Map::Map()
{/*Constructeur tout par défaut.*/}



Map::Map(Vec2i dest_bloc, std::string title, short idle_value)
: 	m_destBLOC_custom_w_h(dest_bloc), m_title(title), m_idle_value(idle_value), m_is_init(true)
{}



Map::Map(Renderer* ren)
: m_ren(ren)
{/*Constructeur avec seule la gs::Game. */}



bool Map::init(Renderer* ren, Vec2i dest_bloc, std::string title, short idle_value)
{
	/*Comme le constructeur complet.*/

	m_ren = ren;
	m_destBLOC_custom_w_h = dest_bloc;
	m_title = title;
	setIdleValue(idle_value, true);
	m_is_init = true;

	return true;
}



Map::~Map()
{
	//if(m_destroy_ptrs_in_destructor)
		clearTilesets();
	//else
	//	::gs::addToClearAtTheEnd(m_tileset);
};































//________________________________________________________________________________________
//___Globals______________________________________________________________________________



void Map::update(float dt)
{
	//Mis à jour des animations des tuiles
	for(auto& [tileset_id, tileset] : m_tilesets)
	{
		for(auto& [tile_val, anim_info] : tileset.animated_tiles)
			anim_info.update(dt);
	}
}




bool Map::setTileset(int tileset_id, const std::string& pth, const Tileset_Loading_Args& tl_args)
{
	//Etape 1 : chargement du tileset
	Tileset tileset_buff;
	if(not tileset_buff.load(m_ren, pth, tl_args))
	{
		gs::lastError(std::format("from 'Map::setTileset()' of '{}' : ", m_title), gs::lastError());
		return false;
	}

	//Etape 2 : rajout du tileset
	setTileset__finalStep(tileset_id, tileset_buff);

	return true;
}


bool Map::setTilesetFromSurface(int tileset_id, Surface* atlas_surf, const Tileset_Loading_Args& tl_args)
{
	//Etape 1 : chargement du tileset
	Tileset tileset_buff;
	if(not tileset_buff.load(m_ren, atlas_surf, tl_args))
	{
		gs::lastError(std::format("from 'Map::setTilesetFromSurface()' of '{}' : ", m_title), gs::lastError());
		return false;
	}

	//Etape 2 : rajout du tileset
	setTileset__finalStep(tileset_id, tileset_buff);

	return true;
}


void Map::setTileset__finalStep(int tileset_id, Tileset& tileset_buff)
{
	//On cherche le tileset avec l'ID correspondant.
	std::unordered_map<int, Tileset>::iterator it = m_tilesets.find(tileset_id);
	if(it != m_tilesets.end()) //si trouvé
	{
		if(it->second.m_atlas) //S'il y a une texture, on la destroy.
			gs::destroyTexture(it->second.m_atlas);

		it->second = tileset_buff;
	}
	else //Sinon on crée le tileset et l'initialise.
		m_tilesets[tileset_id] = tileset_buff;
}





bool Map::setTileset_noOverwrite(int tileset_id, const std::string& pth, const Tileset_Loading_Args& tl_args)
{
	//On cherche le tileset avec l'ID correspondant.
	std::unordered_map<int, Tileset>::iterator it = m_tilesets.find(tileset_id);
	if(it == m_tilesets.end() //Si non trouvé
	or not it->second.m_atlas //ou si trouvé, mais qu'il n'y a pas de texture
	)
		return setTileset(tileset_id, pth, tl_args);

	return true;
}

bool Map::setTilesetFromSurface_noOverwrite(int tileset_id, Surface* atlas_surf, const Tileset_Loading_Args& tl_args)
{
	//On cherche le tileset avec l'ID correspondant.
	std::unordered_map<int, Tileset>::iterator it = m_tilesets.find(tileset_id);
	if(it == m_tilesets.end() //Si non trouvé
	or not it->second.m_atlas //ou si trouvé, mais qu'il n'y a pas de texture
	)
		return setTilesetFromSurface(tileset_id, atlas_surf, tl_args);

	return true;
}




bool Map::addAnimtedTiles(int tileset_id, const std::vector<Tile_Anim>& animated_tiles, bool overwrite_existing_ones)
{
	//On cherche le tileset avec l'ID correspondant.
	std::unordered_map<int, Tileset>::iterator it = m_tilesets.find(tileset_id);
	if(it == m_tilesets.end()) //Si NON trouvé
	{
		gs::lastError(std::format("||Warning|| in Map::addAnimtedTiles()' of '{}', tileset not found. [given tileset_id : {}]. ", m_title, tileset_id));
		return false;
	}

	Tileset& tileset = it->second;

	for(const Tile_Anim& a_t : animated_tiles) //Parcours des "tuiles" reçues pour pouvoir les ajouter.
	{
		//Recherche de la tuile courante, dans les tuiles animées du tileset.
		std::unordered_map<short, Tile_Anim>::iterator it2 = tileset.animated_tiles.find(a_t.tile_val);

		if(it2 != tileset.animated_tiles.end()) //Si l'on trouve cette tuile,
		{
			if(overwrite_existing_ones) // on la rajoute uniquement si le remplacement est autorisé.
			{
				it2->second = a_t;
				it2->second.initBegin();
			}
		}
		else //Si elle n'y est pas, on l'ajoute librement
		{
			tileset.animated_tiles.try_emplace(a_t.tile_val, a_t);
			tileset.animated_tiles[a_t.tile_val].initBegin();
		}
	}

	return true;
}





void Map::setScreenOffset(Vec2f val) {m_screen_offset = val;}
void Map::setScreenOffsetX(float val) {m_screen_offset.x = val;}
void Map::setScreenOffsetY(float val) {m_screen_offset.y = val;}
void Map::incrScreenOffsetX(float val) {m_screen_offset.x += val;}
void Map::incrScreenOffsetY(float val) {m_screen_offset.y += val;}




void Map::setDestBLOC(Vec2i vals)
{
	vals.x = vals.x < 1 ? 1 : vals.x;
	vals.y = vals.y < 1 ? 1 : vals.y;
	m_destBLOC_custom_w_h = vals;
}

void Map::setDestBLOCW(unsigned short val) {m_destBLOC_custom_w_h.x = (val < 1 ? 1 : val);}

void Map::setDestBLOCH(unsigned short val) {m_destBLOC_custom_w_h.y = (val < 1 ? 1 : val);}



void Map::setZoom(float val) { m_zoom = val; if(m_zoom < 0.01f) m_zoom=0.01f;}
void Map::incrZoom(float val) { m_zoom += val; if(m_zoom < 0.01f) m_zoom=0.01f;}


void Map::setRen(Renderer* ren) { m_ren = ren; }



void Map::clearTilesets()
{
	for(auto& [tileset_id, tileset] : m_tilesets)
	{
		if(tileset.m_atlas)
		{
			gs::destroyTexture(tileset.m_atlas);
			tileset.m_atlas = nullptr;
		}

		tileset.animated_tiles.clear();
	}
}



void Map::setSynchronization(bool new_state)
{
	m_synchronized = new_state;
}








Vec2f Map::getScreenOffset() const {return m_screen_offset;}
int Map::getScreenOffsetX_i() const {return static_cast<int>(m_screen_offset.x);}
int Map::getScreenOffsetY_i() const {return static_cast<int>(m_screen_offset.y);}



Vec2i Map::getDestBLOC() const {return m_destBLOC_custom_w_h;}


int Map::getDestBLOCW() const {return m_destBLOC_custom_w_h.x;}
int Map::getDestBLOCH() const {return m_destBLOC_custom_w_h.y;}





Vec2i Map::getTileSrcSize(int tileset_id) const
{
	//On cherche le tileset avec l'ID correspondant.
	std::unordered_map<int, Tileset>::const_iterator c_it = m_tilesets.find(tileset_id);
	if(c_it != m_tilesets.cend()) //Si trouvé
		return c_it->second.m_tile_src_size;

	return Vec2i(0,0);
}


float Map::getZoom() const {return m_zoom;}

Renderer* Map::getRen() {return m_ren;}

const Tileset& Map::getTileset(int tileset_id, bool& is_ok) const
{
	is_ok = true;

	//On cherche le tileset avec l'ID correspondant.
	std::unordered_map<int, Tileset>::const_iterator c_it = m_tilesets.find(tileset_id);

	if(c_it != m_tilesets.cend()) //si trouvé
		return c_it->second;

	static Tileset default_return;
	is_ok = false;
	return default_return;
}


bool Map::isTilesetLoaded(int tileset_id) const
{
	//On cherche le tileset avec l'ID correspondant.
	std::unordered_map<int, Tileset>::const_iterator c_it = m_tilesets.find(tileset_id);

	if(c_it != m_tilesets.cend()) //si trouvé
		return c_it->second.m_atlas != nullptr;

	return false;
}


int Map::getNumberOfTilesetColumn(int tileset_id) const
{
	//On cherche le tileset avec l'ID correspondant.
	std::unordered_map<int, Tileset>::const_iterator c_it = m_tilesets.find(tileset_id);

	if(c_it != m_tilesets.cend()) //si trouvé
		return c_it->second.m_number_of_columns;

	return INT_ERR_VAL;
}


Vec2f Map::getTilesetWH(int tileset_id) const
{
	bool is_ok;
	const Tileset& tileset = getTileset(tileset_id, is_ok);
	Vec2f res(-1.f,-1.f);

	if(not is_ok or not tileset.m_atlas)
	{
		gs::lastError(std::format("||Warning|| in Map::getTilesetWH()' of '{}', tileset not init. [given tileset_id : {}]. ", m_title, tileset_id));
		return res;
	}

	getTextureSize(tileset.m_atlas, &res.x, &res.y);
	return res;
}



float Map::getTilesetW(int tileset_id) const
{
	bool is_ok;
	const Tileset& tileset = getTileset(tileset_id, is_ok);

	if(not is_ok or not tileset.m_atlas)
	{
		gs::lastError(std::format("||Warning|| in Map::getTilesetW()' of '{}', tileset not init. [given tileset_id : {}]. ", m_title, tileset_id));
		return -1.f;
	}

	float w=0.f;
	getTextureSize(tileset.m_atlas, &w, nullptr);
	return w;
}



float Map::getTilesetH(int tileset_id) const
{
	bool is_ok;
	const Tileset& tileset = getTileset(tileset_id, is_ok);

	if(not is_ok or not tileset.m_atlas)
	{
		gs::lastError(std::format("||Warning|| in Map::getTilesetH()' of '{}', tileset not init. [given tileset_id : {}]. ", m_title, tileset_id));
		return -1.f;
	}

	float h=0.f;
	getTextureSize(tileset.m_atlas, nullptr, &h);
	return h;
}



bool Map::isRenValid() const { return m_ren != nullptr; }


int Map::m_getRenW() const
{
	int w=-1;
	getCurrentRenderOutputSize(m_ren, &w, nullptr);
	return w;
}


int Map::m_getRenH() const
{
	int h=-1;
	getCurrentRenderOutputSize(m_ren, nullptr, &h);
	return h;
}

float Map::m_getRenWz()
{
	if(m_zoom <= 0.f) m_zoom = 1.f;
	int w=-1;
	getCurrentRenderOutputSize(m_ren, &w, nullptr);

	return static_cast<float>(w)/m_zoom;
}


float Map::m_getRenHz()
{
	if(m_zoom <= 0.f) m_zoom = 1.f;
	int h=-1;
	getCurrentRenderOutputSize(m_ren, nullptr, &h);
	return static_cast<float>(h)/m_zoom;
}

Texture* Map::m_loadTexture(const std::string& path)
{
	if(not isRenValid())
	{
		gs::lastError("from 'Map::m_loadTexture()', invalid renderer. ");
		return nullptr;
	}

	return gs::loadTexture(m_ren, path);
}



// Rect* Map::getBackgroundSrc() {return &backgroundSrc_;}


// Rect* Map::gatBackgroundDest() {return &backgroundDest_;}



bool Map::getSynchronization() const {return m_synchronized;}


int Map::getNumberOfLayers() const
{
	if(m_layers.size() != m_layers_count)
	{
		gs::lastError("||Fatal Error|| (found when called 'Map::getNumberOfLayers()'), m_layers.size() != m_layers_count. Alert the dev if you can or if you want please.");
		return -1;
	}

	return m_layers_count;
}

int Map::getLayersCount() const
{
	if(m_layers.size() != m_layers_count)
	{
		gs::lastError("||Fatal Error|| (found when called 'Map::getLayersCount()'), m_layers.size() != m_layers_count. Alert the dev if you can or if you want please.");
		return -1;
	}

	return m_layers_count;
}


std::string Map::getTitle() const{return m_title;}






















//__________________________________________________________________________________________________________
//____Layers________________________________________________________________________________________________


void Map::setNextLayersTileset(int tileset_id) {m_next_layers_tileset_id=tileset_id;}


bool Map::addLayer(const std::string& file_path, const std::string& layer_str_ID, Vec2f sensibility, bool is_the_main_layer)
{
	std::string error_msg_begin = gs::str("||Error|| in 'Map::addLayer()' of '") + m_title;
	// std::string warning_msg_begin = gs::str("||Warning|| in 'Map::addLayer()' of '") + m_title;

	fs::path path(file_path);

	if(not exists(path))
	{
		gs::lastError(error_msg_begin +gs::str("', invalid file path : \"")+path.string()+gs::str("\". "));
		return false;
	}


	std::ifstream file(file_path, std::ios::in);

	if(not file.is_open())
	{
		gs::lastError(error_msg_begin +gs::str("', cannot open file. [")+path.string()+std::string("]"));
		return false;
	}


	bool layer_found = false;
	std::string str_ID = std::string("#") + layer_str_ID;

	while(not file.eof())
	{
		std::string token;

		file >> token;

		if(token == str_ID)
		{
			layer_found = true;
			break;
		}
	}

	if(not layer_found)
	{
		gs::lastError(error_msg_begin +gs::str("', cannot find the layer by the string ID. Make sure ")
					+ gs::str("there is the good string ID in the file or in the source code. ")
					+ gs::str("(In the file, have to have a '#' in the begining of the ID (e.g #MyLayer1). In the source ")
					+ gs::str("code, have to ingore the '#' (e.g map.addLayer(\"path\", \"MyLayer1\") ) )"));
		return false;
	}


	m_layers.push_back(Map_Layer());
	m_layers_count++;
	int layer_index = m_layers_count - 1;


	if(is_the_main_layer)
	{
		if(not m_have_main_layer)
		{
			m_layers[layer_index].m_is_the_main_layer = true;
			m_main_layer_index = layer_index;
			m_have_main_layer = true;

			sensibility = Vec2f(100.f, 100.f);
		}
	}



	//Lecture des limites (nombre de tuiles par axe).
	file >> m_layers.at(layer_index).m_tiles_count.x;
	file >> m_layers.at(layer_index).m_tiles_count.y;

	if(m_layers.at(layer_index).m_tiles_count.x <= 0 or
		m_layers.at(layer_index).m_tiles_count.y <= 0)
	{
		gs::lastError(std::format("||Error|| in 'Map::addLayer()' of '{}', invalid map limits (map sizes). [given values : in x {}, in y {}]. ",
									m_title, m_layers.at(layer_index).m_tiles_count.x, m_layers.at(layer_index).m_tiles_count.y));
		popBackLayer();
		return false;
	}

	//Initialisation du layer (du tableau des tuiles).
	m_layers.at(layer_index).initTheMapArray();

	//Remplissage avec les valeurs.
		//Les valeurs invalides seront transformées en celle ci dessous, et la tile sera considérée comme inactive.
	const short DEFAULT_VALUE = 0;
	const int limit_y = m_layers[layer_index].m_tiles_count.y;
	const int limit_x = m_layers[layer_index].m_tiles_count.x;

	try
	{
		for(int y=0; y< limit_y; y++)
		{
			for(int x=0; x< limit_x; x++)
			{
				if(file.eof())
				{
					gs::lastError(error_msg_begin +gs::str("', end of file reached without finish collecting datas"));
					popBackLayer(); // m_layers.pop_back();
					return false;
				}

				short val;
				bool active_state=true;

				file >> val;

				if(val < 0)
				{
					val = DEFAULT_VALUE;
					active_state = false;
				}
				else
				if(val == m_idle_value)
					active_state = false;


				Tile tile;
					tile.m_value = val;
					tile.m_active = active_state;

				m_layers[layer_index].m_map_array[y][x] = tile;
			}
		}
	}
	catch(std::exception& ex)
	{
		gs::lastError(gs::str("||Error|| occurred in 'gs::Map::addLayer()' of '") + m_title +gs::str("', when read datas ; ")
					+ gs::str(ex.what()));
		popBackLayer(); // m_layers.pop_back();
		return false;
	}


	if(sensibility.x < 0)
		sensibility.x = 0;
	if(sensibility.y < 0)
		sensibility.y = 0;

	m_layers[layer_index].m_sensibility = sensibility;
	m_layers[layer_index].m_tileset_id = m_next_layers_tileset_id;

	return true;
}



bool Map::addLayer(const Vec2ui& size, int fill_val, int border_val, Vec2f sensibility, bool is_the_main_layer)
{
	m_layers.push_back(Map_Layer());
	m_layers_count ++;
	Map_Layer& new_layer = m_layers.back();
	int layer_index = m_layers_count - 1;

	if(is_the_main_layer)
	{
		if(not m_have_main_layer)
		{
			new_layer.m_is_the_main_layer = true;
			m_main_layer_index = layer_index;
			m_have_main_layer = true;

			sensibility = Vec2f(100.f, 100.f);
		}
	}

	//Application des dimensions de la map.
	new_layer.m_tiles_count.x = size.x;
	new_layer.m_tiles_count.y = size.y;
	new_layer.initTheMapArray(); //Initialisation du tableau des tuiles.

	//Remplissage avec la valeur de remplissage si est valide.
	const short DEFAULT_VALUE = 0;
	bool active_state = true;
	bool boder_active_state = true;

	if(fill_val < 0)
	{
		fill_val = DEFAULT_VALUE;
		active_state = false; //Si 'fill_val' est invalid on désactive toutes les tuiles.
	}
	if(border_val < 0)
	{
		border_val = DEFAULT_VALUE;
		boder_active_state = false;
	}

	for(int y=0; y < size.y; ++y)
	{
		for(int x=0; x < size.x; ++x)
		{
			if(x==0 or y==0 or x==int(size.x)-1 or y==int(size.y)-1) //Les bords.
			{
				new_layer.m_map_array[y][x].m_value = border_val;
				new_layer.m_map_array[y][x].m_active = boder_active_state;
			}
			else
			{
				new_layer.m_map_array[y][x].m_value = fill_val;
				new_layer.m_map_array[y][x].m_active = active_state;
			}


		}
	}

	//Attribution des sensibilités.
	if(sensibility.x < 0)
		sensibility.x = 0;
	if(sensibility.y < 0)
		sensibility.y = 0;

	new_layer.m_sensibility = sensibility;
	new_layer.m_tileset_id = m_next_layers_tileset_id;

	return true;
}



bool Map::addLayer(const Vec2ui& size, const std::vector<std::vector<int>>& tiles_grid, Vec2f sensibility, bool is_the_main_layer)
{
	//Vérification de la validité des dimensions.
	bool size_is_ok = false;
	if(tiles_grid.size() >= size.y)
	{
		size_is_ok = true;

		for(int i=0; i< size.y; ++i)
		{
			//Vérification de la taille en x (il faut qu'il y'ait au moins 'size.x' sur toutes les lignes concernées).
			if(tiles_grid.at(i).size() < size.x)
			{
				size_is_ok = false;
				break;
			}
		}
	}

	if(not size_is_ok)
	{
		gs::lastError("||Warning|| in 'Map::addLayer(size, tiles_grid)', 'size' and dimensions of given 'tiles_grid' don't match. [Dimensions of 'tiles_grid' must be >= 'size'.] ");
		return false;
	}


	m_layers.push_back(Map_Layer());
	m_layers_count ++;
	Map_Layer& new_layer = m_layers.back();
	int layer_index = m_layers_count - 1;

	if(is_the_main_layer and not m_have_main_layer)
	{
		new_layer.m_is_the_main_layer = true;
		m_main_layer_index = layer_index;
		m_have_main_layer = true;

		sensibility = Vec2f(100.f, 100.f);
	}

	//Application des dimensions de la map.
	new_layer.m_tiles_count.x = size.x;
	new_layer.m_tiles_count.y = size.y;
	new_layer.initTheMapArray(); //Initialisation du tableau des tuiles.


	//Remplissage avec la valeur de remplissage si est valide.
	const short DEFAULT_VALUE = 0;

	for(int y=0; y < size.y; ++y)
	{
		for(int x=0; x < size.x; ++x)
		{
			bool active_state = true;
			int val = tiles_grid[y][x];

			if(val < 0)
			{
				val = DEFAULT_VALUE;
				active_state = false;
			}

			new_layer.m_map_array[y][x].m_value = val;
			new_layer.m_map_array[y][x].m_active = active_state;
		}
	}

	//Attribution des sensibilités.
	if(sensibility.x < 0)
		sensibility.x = 0;
	if(sensibility.y < 0)
		sensibility.y = 0;

	new_layer.m_sensibility = sensibility;
	new_layer.m_tileset_id = m_next_layers_tileset_id;

	return true;
}


void Map::setLayerTileset(int layer_index, int tileset_id)
{
	if(isInvalidLayerIndex(layer_index))
		gs::lastError("||Error|| in 'gs::Map::setLayerTileset()' given layer index [ ", std::to_string(layer_index)+" ] doesn't exist. ");
	else
		m_layers.at(layer_index).m_tileset_id = tileset_id;
}


bool Map::drawLayer(int layer_index, Vec2f offset, float brightness, Rect area)
{
	//'offset' = décalage sur l'écran.

	std::string error_msg_begin = gs::str("||Error|| in 'gs::Map::drawLayer()' [layer_index is ")+std::to_string(layer_index)+std::string("] of '") + m_title;
	std::string warning_msg_begin = gs::str("||Warning|| in 'gs::Map::drawLayer()' [layer_index is ")+std::to_string(layer_index)+std::string("] of '") + m_title;

	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(error_msg_begin + gs::str("', given layer index doesn't exist."));
		return false;
	}

	if(not m_layers.at(layer_index).m_is_visible)
		return true;

	if(not isRenValid())
	{
		gs::lastError(warning_msg_begin + gs::str("', cannot draw : missing a valid 'Renderer*'"));
		return false;
	}

	bool tileset_is_ok;
	const Tileset& tileset = getTileset(m_layers.at(layer_index).m_tileset_id, tileset_is_ok);
	if(not tileset_is_ok or not tileset.m_atlas)
	{
		gs::lastError(warning_msg_begin + gs::str("', cannot draw : missing a valid tileset. Current layer [index==")+std::to_string(layer_index)+", tileset_id=="
			+std::to_string(m_layers.at(layer_index).m_tileset_id)+"] ");
		return false;
	}

	//nombre maximal de tiles sur chaque axe
	float limit_x = m_layers[layer_index].m_tiles_count.x;
	float limit_y = m_layers[layer_index].m_tiles_count.y;


	//prise en compte du zoom, qui baisse/augmente la taille du bloc de destination
	// int destBLOC_z = static_cast<float>(m_destBLOC) * m_zoom;
	float dest_bloc_w = float(getDestBLOCW()) *m_zoom;
	float dest_bloc_h = float(getDestBLOCH()) *m_zoom;
	Vec2f start = getWorldStart(layer_index) *m_zoom;

	//On prend en compte la sous-zone mentionnée (ou non).
	{
		start.x+=area.x*dest_bloc_w *m_zoom; //Pour la position de cette sous zone on reste flexible.
		start.y+=area.y*dest_bloc_h *m_zoom;

		if(area.w > 0 and area.h > 0) //Pour la taille on impose une zone non vide pour la prendre en compte.
		{
			limit_x = GetMin(limit_x, area.x+area.w);
			limit_y = GetMin(limit_y, area.y+area.h);
		}
	}

	//d'où l'on commence à lire (sur les cases du tableau)
	int mapX = std::floor(start.x / dest_bloc_w); int mapX_starting_val = mapX;
	int mapY = std::floor(start.y / dest_bloc_h);


	//à partir d'où (au pixel près) on va commencer à blitter
	int x1 = getScreenOffset().x - std::fmod(start.x, dest_bloc_w);
	int y1 = getScreenOffset().y - std::fmod(start.y, dest_bloc_h);

	{
		//Traitements liés à l'ajout de 'offset'.
		x1 += offset.x;
		y1 += offset.y;
		if(x1 < getScreenOffsetX_i()-(dest_bloc_w-1)) //Si un ou plusieurs blocs sont affichés hors écran, alors on passe directement au 'moment' où il y'a quelque-chose de visible, afin d'éviter des tours de boucle dans le vide.
		{
			int dest_b_w_int = std::floor(dest_bloc_w);
			int x1_int = std::floor(x1);

			mapX += std::abs(x1_int) / dest_b_w_int;  mapX_starting_val=mapX;
			x1 += dest_b_w_int * (std::abs(x1_int)/dest_b_w_int);
		}
		if(y1 < getScreenOffsetY_i()-(dest_bloc_h-1))
		{
			int dest_b_h_int = std::floor(dest_bloc_h);
			int y1_int = std::floor(y1);

			mapY += std::abs(y1_int) / dest_b_h_int;
			y1 += dest_b_h_int * (std::abs(y1_int)/dest_b_h_int);
		}
	}


	//à quelle pixel de l'écran on arrête de blitter:
	//pour commencer on considère qu'on s'arrête au tout dernier pixel (de la map), même si est hors écran,
	//puis on apporte une correction si nécessaire.
	float x2 = x1 + limit_x * dest_bloc_w;        x2+=(offset.x < 0 ? offset.x : 0); //Prise compte du décalage apporté par 'offset'.
	float y2 = y1 + limit_y * dest_bloc_h;        y2+=(offset.y < 0 ? offset.y : 0);

		//si x2 < largeur du rendu : tout va bien, sinon on l'arrête à la bordure
		float x2_max = m_getRenW() + (offset.x < 0 ? offset.x : 0);
		if(x2 > x2_max)
			x2 = x2_max;

		float y2_max = m_getRenH() + (offset.y < 0 ? offset.y : 0);
		if(y2 > y2_max)
			y2 = y2_max;


	//Protection contre le crache.
	if(mapX < 0) mapX = 0;
	if(mapY < 0) mapY = 0;

	//Gestion de la luminosité du layer : 1=100% -> rien n'est fait en plus, 0=0% -> le layer est tout noir, ]0; 1[ = ]0%; 100%[ -> un rectangle noire est dessiné sur le layer avec une certaine oppacité en fonction du pourcentage de luminosité.
	bool zero_brightness=false;
	FColor brightness_filter{0,0,0,0};
	{
		if(brightness < 0) brightness = 0; //Les négatives sont comptés comme nul.
		else if(brightness > 1) brightness = 1; //Les >1 sont comptés comme 1.

		zero_brightness = (brightness == 0);
		brightness_filter.a = (1.f-brightness)*255.f;
	}

	FRect src, dest;
	src.w=tileset.getTileSrcSize().x; src.h=tileset.getTileSrcSize().y; 
	dest.w = dest_bloc_w; dest.h = dest_bloc_h;

	if(not zero_brightness)
	if(x1 < x2) //Cette condition permet d'éviter des parcours inutiles de la permière boucle.
	for(float y = y1; y < y2; y+= dest_bloc_h)
	{
		for(float x = x1; x< x2; x+= dest_bloc_w)
		{
			if(mapX >= limit_x || mapY >= limit_y)
			{
				//removed std::string witch=std::string("(mapX : ")+std::to_string(mapX)+std::string("; limit_x : ")+std::to_string(limit_x)+std::string(").");
				//removed if(not (mapX >= limit_x))
				//removed 	witch = std::string("(mapY : ")+std::to_string(mapY)+std::string("; limit_y : ")+std::to_string(limit_y)+std::string(").");

				//removed gs::lastError(error_msg_begin+ gs::str("', reached out of limits when drawing layer ")
				//removed 			+ std::to_string(layer_index)+std::string(". ")+witch);

				break; //On passe à la prochaine ligne.
			}

			if(m_layers[layer_index].m_map_array[mapY][mapX].m_active)
			{
				dest.x = x;
				dest.y = y;

				int tile_val = m_layers[layer_index].m_map_array[mapY][mapX].m_value;

				tile_val = tileset.getCurrentFrameOf(tile_val); //Prise en compte de l'animation
				Vec2i tile_src_pos = tileset.getTileSrcPos(tile_val);

				src.x = tile_src_pos.x;
				src.y = tile_src_pos.y;

				bool res = drawTile(m_ren, nullptr, tileset.m_atlas, src, dest, false); //false pour dessiner sur la target courante et pas forcément sur le render de base.
				
				if(not res)
					gs::lastError("from 'Map::drawLayer()' : ", gs::lastError());
			}

			mapX ++;
		}

		//remise du "curseur" des x au début
		mapX = mapX_starting_val;//start.x / dest_bloc_w;
		mapY ++;

		if(mapY >= limit_y)
			break; //On a fini de parcourir les lignes.
	}

	//Finition de la partie luminosité.
	Blend_Mode bm = BLENDMODE_BLEND;
	bool success = true;
	if(brightness_filter.a > 0) //Si a==0, c'est que brightness était à 1 (ou suffisament proche), donc on ne fait rien de plus.
		success = gs::drawRect(m_ren, FRect(x1,y1, ((x2-x1)/dest_bloc_w +1)*dest_bloc_w, ((y2-y1)/dest_bloc_h +1)*dest_bloc_h), color(brightness_filter), true,&bm);

	if(not success)
		gs::lastError(gs::str("from 'gs::Map::drawLayer()', on brightness filter, [layer_index is ")+std::to_string(layer_index)+std::string("] of '") + m_title+std::string(" : ") +gs::lastError());

	return success;
}



bool Map::drawLayer(Vec2f offset, float brightness, Rect area)
{
	//Ce drawLayer ci, sans 'layer_index', dessine tous les layers.

	bool success = true;

	for(int index=0; index< m_layers_count; index++)
	{
		success = (success & drawLayer(index, offset, brightness, area));
	}

	return success;
}



void Map::popBackLayer()
{
	short last_index = m_layers_count - 1;

	if(isInvalidLayerIndex(last_index))
		return;

	if(m_layers[last_index].m_is_the_main_layer)
	{
		m_have_main_layer = false;
		m_main_layer_index = -1; // = non défini
	}

	for(size_t i=0; i< m_collision_layers.size(); i++)
	{
		Vec_Int_VecSInt& collision_lay = m_collision_layers.at(i);

		if(collision_lay.m_id == last_index)
		{
			m_collision_layers.erase(m_collision_layers.begin()+i);
			break;
		}
	}

	m_layers.pop_back();

	m_layers_count --;
}


const Map_Layer& Map::getLayer(int layer_index, bool& is_ok) const
{
	static Map_Layer default_return;

	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError("!! in 'Map::getLayer()', given layer index doesn't exist. [given layer index : ", std::to_string(layer_index)+"]");
		is_ok = false;
		return default_return;
	}

	is_ok = true;
	return m_layers.at(layer_index);
}































std::vector<Map_Layer*>* Map::INTERNAL_USE_getTargetLayers(std::string function_name, int layer_index)
{
	bool all_layers = false;
	if(layer_index == -1)
		all_layers = true;
	else
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(std::format("||Warning|| in 'Map::{}' of '{}', invalid layer index. [given index : {}, max index : {}]", function_name, m_title, layer_index, m_layers_count-1));
		return nullptr;
	}

	m_helper_return_layers_array.clear();

	if(not all_layers)
		m_helper_return_layers_array.push_back(&m_layers.at(layer_index));
	else
	{
		for(int i=0; i< m_layers.size(); i++)
			m_helper_return_layers_array.push_back(&m_layers.at(i));
	}

	return &m_helper_return_layers_array;
}









bool Map::setWorldStartX(int layer_index, float val)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError("||Warning|| in 'Map::setWorldStartX()', given layer index doesn't exist");
		return false;
	}

	if(not isRenValid())
	{
		gs::lastError("||Warning|| in 'Map::setWorldStartX()', invalid 'Renderer*' ");
		return false;
	}


	m_layers[layer_index].setWorldStartX(val, getDestBLOCW(), m_getRenWz());
	return true;
}





bool Map::setWorldStartY(int layer_index, float val)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError("||Warning|| in 'Map::setWorldStartY()', given layer index doesn't exist");
		return false;
	}

	if(not isRenValid())
	{
		gs::lastError("||Warning|| in 'Map::setWorldStartY()', invalid 'Renderer*' ");
		return false;
	}


	m_layers[layer_index].setWorldStartY(val, getDestBLOCH(), m_getRenHz());
	return true;
}




bool Map::incrWorldStartX(int layer_index, float val)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError("||Warning|| in 'Map::incrWorldStartX()', given layer index doesn't exist");
		return false;
	}

	if(not isRenValid())
	{
		gs::lastError("||Warning|| in 'Map::incrWorldStartX()', invalid 'Renderer*' ");
		return false;
	}


	m_layers[layer_index].incrWorldStartX(val, getDestBLOCW(), m_getRenWz());

	return true;
}


bool Map::incrWorldStartY(int layer_index, float val)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError("||Warning|| in 'Map::incrWorldStartY()', given layer index doesn't exist");
		return false;
	}

	if(not isRenValid())
	{
		gs::lastError("||Warning|| in 'Map::incrWorldStartY()', invalid 'Renderer*' ");
		return false;
	}


	m_layers[layer_index].incrWorldStartY(val, getDestBLOCH(), m_getRenHz());

	return true;
}




bool Map::setWorldStartX(float val)
{
	//Pour cette fonction qui est un setter sur tous, la synchronisation des 'incr' ne rentre pas en compte.
	bool success = true;
	for(int index = 0; index < m_layers_count; index++)
	{
		success = (success & setWorldStartX(index, val));
	}

	return success;
}



bool Map::setWorldStartY(float val)
{
	bool success = true;
	for(int index = 0; index < m_layers_count; index++)
	{
		success = (success & setWorldStartY(index, val));
	}

	return success;
}





bool Map::incrWorldStartX(float val)
{
	//incrWorldStartX / Y (sans layer_index) = pour tous les layers.

	//En mode synchronisé, les layers ne peuvent bouger que si eux tous peuvent bouger (dans tel ou tel sens)

	bool success = true;

	//Calculs préparatoires pour savoir si tout le monde peut scroller.
	if(m_synchronized && val != 0)
	{
		bool there_is_one_who_cannot = false;
		bool go_to_left = (val < 0 ? true : false);

		if(go_to_left)
		{
			for(const Map_Layer& layer : m_layers)
			{
				if(layer.m_left_scrolling_limit_reached)
				{
					there_is_one_who_cannot = true;
					break;
				}
			}
		}
		else // = go to right
		{
			for(const Map_Layer& layer : m_layers)
			{
				if(layer.m_right_scrolling_limit_reached)
				{
					there_is_one_who_cannot = true;
					break;
				}
			}
		}

		if(there_is_one_who_cannot)
			return false;
	}


	for(int index = 0; index < m_layers_count; index++)
	{
		success = (success & incrWorldStartX(index, val));
	}

	return success;
}





bool Map::incrWorldStartY(float val)
{
	bool success = true;

	if(m_synchronized && val != 0)
	{
		bool there_is_one_who_cannot = false;
		bool go_to_up = (val < 0 ? true : false);

		if(go_to_up)
		{
			for(const Map_Layer& layer : m_layers)
			{
				if(layer.m_up_scrolling_limit_reached)
				{
					there_is_one_who_cannot = true;
					break;
				}
			}
		}
		else // = go to down
		{
			for(const Map_Layer& layer : m_layers)
			{
				if(layer.m_down_scrolling_limit_reached)
				{
					there_is_one_who_cannot = true;
					break;
				}
			}
		}

		if(there_is_one_who_cannot)
			return true;
	}


	for(int index = 0; index < m_layers_count; index++)
	{
		success = (success & incrWorldStartY(index, val));
	}

	return success;
}



void Map::setIdleValue(short val, bool restore_previous)
{
	/*
	Cette fonction attribue une valeur de tuile (pour tous les layers) qui sera considérée comme inactif.
	C-à-d que toute les tuiles ayant comme valeur cette valeur seront inactives (ignorées lors de l'affichage ou/et des collisions).
	`restore_previous' permet de si jamais on avait utilisé cette fonction précédemment, de ne pas garder les tuiles
	concernées par l'encienne valeur de `m_idle_value' incatives, alors qu'elles ne sont potentiellement plus concernées par la
	nouvelle valeur. Par ailleurs l'utilisateur peut interdire cette restauration en mentionnant un 'false' ; 'true' est par défaut.
	*/

	if(restore_previous)
	{
		for(int i=0; i< m_layers_count; ++i)
		{
			setActiveStateOfTilesOf(m_idle_value, i, true);
		}
	}

	m_idle_value = val;

	for(int i=0; i< m_layers_count; ++i)
	{
		setActiveStateOfTilesOf(m_idle_value, i, false);
	}
}


bool Map::setActiveStateOfTilesOf(short tile_val, int layer_index, bool new_state)
{
	/*
	Pour un layer donné, cette fonction attribue un état actif ou inactif donné par `new_state', à toutes les tuiles ayant la même valeur que `tile_val'.
	*/
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError("||Warning|| in 'Map::setActiveStateOfTilesOf()', given layer index doesn't exist. ");
		return false;
	}

	if(not m_layers.at(layer_index).m_is_init)
		return false;

	short limit_x = m_layers[layer_index].m_tiles_count.x;
	short limit_y = m_layers[layer_index].m_tiles_count.y;

	for(int i=0; i< limit_y; i++)
	{
		for(int j=0; j< limit_x; j++)
		{
			Tile& tile = m_layers[layer_index].m_map_array[i][j];

			if(tile.m_value == tile_val)
				tile.m_active = new_state;
		}
	}

	return true;
}


bool Map::setActiveStateOfTilesOf(short tile_val, bool new_state)
{
	bool success = true;
	for(int i=0; i< m_layers_count; i++)
	{
		success = (success & setActiveStateOfTilesOf(tile_val, i, new_state));
	}

	return success;
}



bool Map::setTileActiveStateAt(short x, short y, int layer_index, bool new_state)
{
	if(not INTERNAL_USE_check_tile_access_is_ok(gs::str("`Map::setTileActiveStateAt()'"), x, y, layer_index))
		return false;

	m_layers.at(layer_index).m_map_array[y][x].m_active = new_state;

	return true;
}



bool Map::setTileValueAt(short x, short y, int layer_index, short new_value)
{
	if(not INTERNAL_USE_check_tile_access_is_ok(gs::str("`Map::setTileValueAt()'"), x, y, layer_index))
		return false;

	if(new_value < 0)
	{
		gs::lastError(gs::str("||Warning|| in 'Map::setTileValueAt()' of '")+m_title+gs::str("', tile value cannot be negative. [given value : ")+std::to_string(new_value)+gs::str("]. "));
		return false;
	}

	m_layers.at(layer_index).m_map_array[y][x].m_value = new_value;

	return true;
}




bool Map::setTileValueAndActiveStateAt(short x, short y, int layer_index, short new_value, bool new_state)
{
	bool success = true;
	success = success & setTileValueAt(x, y, layer_index, new_value);
	success = success & setTileActiveStateAt(x, y, layer_index, new_state);

	if(not success)
		gs::lastError(gs::str("from 'Map::setTileValueAndActiveStateAt()' : ")+gs::lastError());

	return success;
}




bool Map::setSensibilityX(int layer_index, float new_sensibility)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError("||Warning|| in 'Map::setSensibilityX()', given layer index doesn't exist. ");
		return false;
	}

	if(m_layers.at(layer_index).m_is_the_main_layer)
	{
		gs::lastError("||Warning|| in 'Map::setSensibilityX()', connot change the sensibility of the main layer. ");
		return false;
	}

	if(new_sensibility < 0)
		new_sensibility = 0;

	m_layers.at(layer_index).m_sensibility.x = new_sensibility;

	return true;
}


bool Map::setSensibilityY(int layer_index, float new_sensibility)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError("||Warning|| in 'Map::setSensibilityY()', given layer index doesn't exist. ");
		return false;
	}

	if(m_layers.at(layer_index).m_is_the_main_layer)
	{
		gs::lastError("||Warning|| in 'Map::setSensibilityY()', connot change the sensibility of the main layer. ");
		return false;
	}

	if(new_sensibility < 0)
		new_sensibility = 0;

	m_layers.at(layer_index).m_sensibility.y = new_sensibility;

	return true;
}




bool Map::setSensibilityXY(int layer_index, Vec2f new_sensibility)
{
	bool success = true;
	success = success & setSensibilityX(layer_index, new_sensibility.x);
	success = success & setSensibilityY(layer_index, new_sensibility.y);

	if(not success)
		gs::lastError(gs::str("from 'setSensibilityXY()' : ")+gs::lastError());

	return success;
}




bool Map::setShowLayer(unsigned int layer_index, bool new_state)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(gs::str("||Warning|| in 'gs::Map::setShowLayer()' of '")+m_title+gs::str("', given layer index doesn't exist. [given index : ")+std::to_string(layer_index)+gs::str("] "));
		return false;
	}

	m_layers.at(layer_index).m_is_visible = new_state;

	return true;
}




bool Map::setShowLayers(bool new_state)
{
	if(m_layers.empty())
	{
		gs::lastError(gs::str("||Warning|| in 'gs::Map::setShowLayers()' of '")+m_title+gs::str("', no have any layer in this 'gs::Map'. "));
		return false;
	}

	bool success = true;
	for(int i=0; i< m_layers.size(); i++)
	{
		success = (success & setShowLayer(i, new_state));
	}

	if(not success)
		gs::lastError(gs::str("from 'gs::Map::setShowLayers' : ")+ gs::lastError());

	return success;
}





bool Map::setMainLayer(unsigned int layer_index)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(gs::str("||Warning|| in 'Map::setMainLayer()' of '") +m_title
					+ gs::str("', given layer index doesn't exist. "));
		return false;
	}

	if(m_have_main_layer)
	{
		m_layers[m_main_layer_index].m_is_the_main_layer = false;
	}

	m_layers[layer_index].m_is_the_main_layer = true;
	m_layers[layer_index].m_sensibility = Vec2f(100.f, 100.f);
	m_main_layer_index = layer_index;

	return true;
}






bool Map::replaceTilesValue(int layer_index, short target_tile_value, short new_value)
{
	/*
	Dans cette fonction, pour les layers demandés, toutes les tuiles ayant comme valeur `target_tile_value' auront désormais `new_value' comme valeur.
	*/
	std::vector<Map_Layer*>*
	target_layers = INTERNAL_USE_getTargetLayers("replaceTilesValue()", layer_index);

	if(not target_layers)
		return false;

	for(Map_Layer* layer_ptr : (*target_layers))
	{
		if(not layer_ptr) continue;

		for(int y=0; y< layer_ptr->m_tiles_count.y; ++y)
		{
			for(int x=0; x< layer_ptr->m_tiles_count.x; ++x)
			{
				Tile& current_tile = layer_ptr->m_map_array.at(y).at(x);

				if(current_tile.m_value == target_tile_value)
					current_tile.m_value = new_value;
			}
		}
	}

	return true;
}


//removed
/*
bool Map::linkBodyToLayer(int layer_index, Body& body)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError("||Warning|| in 'Map::linkBodyToLayer()', invalid given layer index.");
		return false;
	}

	for(Map_Layer& layer : m_layers) //On vérifie que le gs::Body qu'on veut rajouter n'y est pas déjà.
	{
		for(Body* body_ptr : layer.m_linked_bodies)
		{
			if(body_ptr == &body)
			{
				gs::lastError("||Warning|| in 'Map::linkBodyToLayer()', this gs::Body is already linked.");
				return false;
			}
		}
	}

	if(body.INTERNAL_USE_isLinkedToALayer())
	{
		gs::lastError("||Warning|| in 'Map::linkBodyToLayer()', this gs::Body seems to be already linked to a layer.");
		return false;
	}


	Map_Layer& layer = m_layers[layer_index];
	layer.m_linked_bodies.push_back(&body);

	body.INTERNAL_USE_linkToALayer(&layer);

	//Mise à jour de la position "physique" du gs::Body par rapport au layer auquel il vient d'être lié.
	{
		body.setX(body.getVX_f()-layer.m_world_start.x);
		body.setY(body.getVY_f()-layer.m_world_start.y);
	}

	return true;
}




bool Map::unlinkBodyToLayer(int layer_index, Body& body)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(gs::str("||Warning|| in 'Map::unlinkBodyToLayer()', invalid given layer index [-> ")+std::to_string(layer_index)+gs::str("]."));
		return false;
	}

	bool success = m_layers[layer_index].unlinkBodyToLayer(body);

	if(not success)
		gs::lastError(gs::str("||Warning|| in 'Map::unlinkBodyToLayer()', it seems that this gs::Body is not linked to this layer. [given layer index : ")+std::to_string(layer_index)+gs::str("]"));

	return success;
}
*/


















Vec2i Map::getWorldStart_i(int layer_index) const
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError("||Warning|| in 'Map::getWorldStart_i()', given layer index doesn't exist");
		return Vec2i(INT_ERR_VAL, INT_ERR_VAL);
	}

	return m_layers[layer_index].m_world_start.to<int>();
}




Vec2f Map::getWorldStart(int layer_index) const
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError("||Warning|| in 'Map::getWorldStart()', given layer index doesn't exist");
		return Vec2f(INT_ERR_VAL, INT_ERR_VAL);
	}

	return m_layers[layer_index].m_world_start;
}




Vec2i Map::getWorldStart_i() const
{
	if(m_have_main_layer)
		return getWorldStart_i(m_main_layer_index);
	return getWorldStart_i(0);
}


Vec2f Map::getWorldStart() const
{
	if(m_have_main_layer)
		return getWorldStart(m_main_layer_index);
	return getWorldStart(0);
}


Vec2i Map::getTilesCount(int layer_index) const
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError("||Warning|| in 'Map::getTilesCount()', given layer index doesn't exist. ");
		return Vec2i(INT_ERR_VAL, INT_ERR_VAL);
	}

	return m_layers.at(layer_index).m_tiles_count;
}




//removed short Map::getLayerW(int layer_index) const
//removed {
//removed 	short return_val = getTilesCountX(layer_index);

//removed 	if(return_val == INT_ERR_VAL)
//removed 		gs::lastError(std::format("from 'Map::getLayerW()' : {}", gs::lastError()));

//removed 	return return_val;
//removed }



//removed short Map::getLayerH(int layer_index) const
//removed {
//removed 	short return_val = getTilesCountY(layer_index);

//removed 	if(return_val == INT_ERR_VAL)
//removed 		gs::lastError(std::format("from 'Map::getLayerH()' : {}", gs::lastError()));

//removed 	return return_val;
//removed }


Vec2i Map::getLastTileXY(int layer_index) const
{
	Vec2i val = getTilesCount(layer_index);
	if(val.x == INT_ERR_VAL or val.y == INT_ERR_VAL)
	{
		gs::lastError(std::format("from 'Map::getLastTileXY()' : {} ", gs::lastError()));
		return Vec2i(INT_ERR_VAL, INT_ERR_VAL);
	}
	return val-Vec2i(1,1);
}



Vec2f Map::getLayerPixelSize(int layer_index) const
{
	Vec2i tiles_count = getTilesCount(layer_index);

	if(tiles_count == Vec2i(INT_ERR_VAL, INT_ERR_VAL))
	{
		gs::lastError("from 'Map::getLayerPixelSize()' : ", gs::lastError());
		return Vec2f(INT_ERR_VAL, INT_ERR_VAL);
	}

	return tiles_count.to<float>() * getDestBLOC().to<float>();

}






short Map::getIdleValue() const {return m_idle_value;}


bool Map::getTileActiveStateAt(int x, int y, int layer_index) const
{

	if(not INTERNAL_USE_check_tile_access_is_ok("Map::getTileActiveStateAt()", x, y, layer_index))
		return false;

	return m_layers[layer_index].m_map_array[y][x].m_active;
}


short Map::getTileValueAt(int x, int y, int layer_index) const
{
	if(not INTERNAL_USE_check_tile_access_is_ok("Map::getTileValueAt()", x, y, layer_index))
		return INT_ERR_VAL;

	return m_layers[layer_index].m_map_array[y][x].m_value;
}






Vec2f Map::getSensibility(int layer_index)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError("||Warning|| in 'Map::getSensibility()', given layer index doesn't exist");
		return Vec2f(INT_ERR_VAL, INT_ERR_VAL);
	}

	return m_layers[layer_index].m_sensibility;
}





bool Map::getShowLayer(int layer_index)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(gs::str("||Warning|| in 'gs::Map::getShowLayer()' of '")+m_title+gs::str("', given layer index doesn't exist. [given index : ")+std::to_string(layer_index)+gs::str("] "));
		return false;
	}

	return m_layers.at(layer_index).m_is_visible;
}



bool Map::isVisibleLayer(int layer_index)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(gs::str("||Warning|| in 'gs::Map::isVisibleLayer()' of '")+m_title+gs::str("', given layer index doesn't exist. [given index : ")+std::to_string(layer_index)+gs::str("] "));
		return false;
	}

	return m_layers.at(layer_index).m_is_visible;
}



bool Map::haveMainLayer() const
{
	return m_have_main_layer;
}


short Map::getMainLayerIndex() const
{
	if(m_have_main_layer)
	{
		if(m_main_layer_index < 0)
			return 0;

		return m_main_layer_index;
	}

	return 0;
}



bool Map::isInvalidLayerIndex(int layer_index) const
{
	short max_layer_index = m_layers_count -1;
	if(layer_index < 0 or layer_index > max_layer_index)
		return true;
	return false;
}



bool Map::INTERNAL_USE_check_tile_access_is_ok(std::string function_name, int x, int y, int layer_index) const
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(gs::str("||Error|| in function '")+function_name+gs::str("' of '")+m_title+gs::str("', given layer index doesn't exist. [given layer : ")+std::to_string(layer_index)+gs::str("]. "));
		return false;
	}

	if(not m_layers[layer_index].m_is_init)
	{
		gs::lastError(gs::str("||Error|| in function '")+function_name+gs::str("' of '")+m_title+gs::str("', this layer seems not yet init. [given layer : ")+std::to_string(layer_index)+gs::str("]. "));
		return false;
	}

	const int limit_x = m_layers[layer_index].m_tiles_count.x;
	const int limit_y = m_layers[layer_index].m_tiles_count.y;

	if(x< 0 or x >= limit_x)
	{
		lastError(gs::str("||Error|| in function '")+function_name+gs::str("' of '")+m_title+gs::str("', given parameter `x' is invalid for layer ")+std::to_string(layer_index)+gs::str(". [given x : ")+std::to_string(x)+gs::str("]. "));
		return false;
	}
	if(y< 0 or y >= limit_y)
	{
		lastError(gs::str("||Error|| in function '")+function_name+gs::str("' of '")+m_title+gs::str("', given parameter `y' is invalid for layer ")+std::to_string(layer_index)+gs::str(". [given y : ")+std::to_string(y)+gs::str("]. "));
		return false;
	}

	return true;
}


































//__________________________________________________________________________________________________________
//_____Collisions___________________________________________________________________________________________

bool Map::setAlsoAsCollisionLayer(int layer_index, std::vector<short> collision_tiles_value)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(gs::str("||Warning|| in 'gs::Map::setAlsoAsCollisionLayer()' of '")+m_title+gs::str("', given layer index doesn't exist. "));
		return false;
	}

	for(auto& collision_lay : m_collision_layers)
	{
		if(collision_lay.m_id == layer_index)
		{
			gs::lastError(gs::str("||Warning|| in 'gs::Map::setAlsoAsCollisionLayer()' of '")+m_title+gs::str("', layer ")+std::to_string(layer_index)+gs::str(" is already set as a collision layer. "));
			return false;
		}
	}

	if(collision_tiles_value.empty())
	{
		gs::lastError(gs::str("||Warning|| in 'gs::Map::setAlsoAsCollisionLayer()' of '")+m_title+gs::str("', cannot add a collision layer without collision tile values. Given 'collision_tiles_value' is empty. "));
		return false;
	}

	//afin d'éviter les doubles valeurs
	{
		std::vector<short> filtered_values{collision_tiles_value.front()};

		for(short& val : collision_tiles_value)
		{
			std::vector<short>::iterator it;
			it = std::find(filtered_values.begin(), filtered_values.end(), val);

			bool already_in = (it == filtered_values.end() ? false : true);

			if(not already_in)
				filtered_values.push_back(val);
		}

		collision_tiles_value.swap(filtered_values);
	}

	Vec_Int_VecSInt layer_info;
	layer_info.m_id = layer_index;
	layer_info.m_values = collision_tiles_value;
	m_collision_layers.push_back(layer_info);

	return true;
}











bool Map::isCollisionTile(int x, int y, int layer_index, int* out__tile_val)
{
	m_helper__error_at_last_call_of_isCollisionTile=false;

	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(gs::str("||Warning|| in 'gs::Map::isCollisionTile()' of '")+m_title+gs::str("', given layer index doesn't exist. "));
		m_helper__error_at_last_call_of_isCollisionTile=true;
		return false;
	}


	bool found = false;
	Vec_Int_VecSInt* collision_lay_ptr=nullptr;

	for(Vec_Int_VecSInt& collision_lay : m_collision_layers)
	{
		if(collision_lay.m_id == layer_index)
		{
			collision_lay_ptr = &collision_lay;
			found = true;
			break;
		}
	}

	if(not found or collision_lay_ptr == nullptr)
	{
		gs::lastError(gs::str("||Warning|| in 'gs::Map::isCollisionTile()' of '")+m_title+gs::str("', layer ")+std::to_string(layer_index)+gs::str(" seems not set as a collision layer. "));
		m_helper__error_at_last_call_of_isCollisionTile=true;
		return false;
	}

	short tile_value_at_xy = getTileValueAt(x, y, layer_index);

	if(tile_value_at_xy == INT_ERR_VAL)
	{
		gs::lastError(gs::str("from 'gs::Map::isCollisionTile()' : ")+gs::lastError());
		m_helper__error_at_last_call_of_isCollisionTile=true;
		return false;
	}

	if(getTileActiveStateAt(x, y, layer_index) == false)
		return false;

	for(const int& collision_tile_val : collision_lay_ptr->m_values)
	{
		if(tile_value_at_xy == collision_tile_val)
		{
			if(out__tile_val) *out__tile_val = tile_value_at_xy;

			return true;
		}
	}

	return false;
}













bool Map::INTERNAL_USE_addNewValueInVector(std::vector<short>* target, short val)
{
	if(target)
	{
		for(short& in_val : (*target) )
		{
			if(in_val == val)
				return false;
		}

		target->push_back(val);
		return true;
	}

	return false;
}









bool Map::hitCollisionTile(FRect ent_rect, int layer_index, std::vector<short>* out__tile_vals, Rect area) // layer_index = -1 par défaut pour signifier de prendre le premier layer de collision (si présent)
{
	m_helper__error_at_last_call_of_hitCollisionTile = false;
	bool isCollisionTile_called_one_time_or_more_here = false;

	if(std::empty(m_collision_layers))
	{
		gs::lastError(gs::str("||Warning|| in 'gs::Map::hitCollisionTile()' of '")+m_title+gs::str("', no have any collision layer in this 'gs::Map'."));
		m_helper__error_at_last_call_of_hitCollisionTile = true;
		return false;
	}
	else
	if(layer_index == -1)
	{
		try //Avec 'layer_index' par défaut (==-1), on essaye de prendre le premier layer de collision.
		{
			layer_index = m_collision_layers.at(0).m_id;
		}
		catch(std::out_of_range& _er) //normalement avec la vérification ci-dessus, ce bloc catch ne devrait jamais être exécuté
		{
			gs::lastError(std::format("||Error|| Exception occurred in 'gs::Map::hitCollisionTile()' of '{}' about indexing the collision layer. It may be an internal error. The exception : {}", m_title, _er.what()));
			return false;
		}
	}
	else
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(gs::str("||Warning|| in 'gs::Map::hitCollisionTile()' of '")+m_title+gs::str("', given layer index doesn't exist. "));
		m_helper__error_at_last_call_of_hitCollisionTile = true;
		return false;
	}
	else
	{
		bool found = false;
		for(gs::Vec_Int_VecSInt& collision_lay : m_collision_layers)
		{
			if(layer_index == collision_lay.m_id)
			{
				found = true;
				break;
			}
		}

		if(not found)
		{
			gs::lastError(gs::str("||Warning|| in 'gs::Map::hitCollisionTile()' of '")+m_title+gs::str("', given layer index doesn't exist as a collision layer. "));
			m_helper__error_at_last_call_of_hitCollisionTile = true;
			return false;
		}
	}


	//Pour se faire, premièrement on va calculer la position de chacun de ses 4 points dans la tile-map et voir si au moins un est dans une tile de collision. (pratique de vérifier seulement les quatres coins dans le cas des hitbox plus petits qu'une tuile)

		//La position dans la tile-map est : 'position à l'écran (au pixel près, sur la cible de rendu)' + 'décalage de la "caméra" (startX ou Y)'.

		/*Petit mémo :
			pos_à_écran (en pixel) 			= pos_dans_map (en pixel) - startX ou Y (en pixel)
			pos_dans_map 					= pos_à_écran + startX ou Y
			case_dans_map ou tuile_dans_map = pos_dans_map / bloc (taille d'une tuile)

			en ce qui concerne startX ou Y : dépend de la taille du bloc : plus le bloc est petit plus il impacte dans l'avancée dans la map (notamment au niveau des cases/tuiles);
											plus le bloc est grand, moins il impacte.
											Dépend de la taille du bloc et son impact varie, car son unité ne change pas (reste au pixel) tandis que la taille du bloc peut changer.
											On pourrait utiliser un startXY non plus au pixel, mais en pourcentage, ainsi il restera constant peu importe la taille du bloc.
											À voir si j'opterai pour cette solution/optimisation à l'avenir une fois que je serai carré là-dessus.

		*/

	//Nouveauté : 'area', permet de réduire "virtuellement" le layer à la sous-zone définie par 'area'. Ainsi 'ent_rect' sera considéré relatif à cette zone.
	// L'unité de 'area.xywz' : en tuile.


	gs::Vec2i limit = m_layers.at(layer_index).m_tiles_count; //nombre max de tuiles sur les abscisses et les ordonnés

	short dest_bloc_w = getDestBLOCW();
	short dest_bloc_h = getDestBLOCH();


	// g = gauche, d = droit, h = haut, b = bas, hg = point haut gauche :)
	Point hg, hd, bg, bd;


	hg.x = ent_rect.x;
	hg.y = ent_rect.y;

	//On prend en compte la sous-zone mentionnée (ou non).
	{
		hg.x+=area.x*dest_bloc_w; //Pour la position de cette sous zone on reste flexible.
		hg.y+=area.y*dest_bloc_h;

		if(area.w > 0 and area.h > 0) //Pour la taille on impose une zone non vide pour la prendre en compte.
		{
			limit.x = GetMin(limit.x, area.x+area.w);
			limit.y = GetMin(limit.y, area.y+area.h);
		}
	}

    hd.x = hg.x + ent_rect.w-1;
	hd.y = hg.y;

	bg.x = hg.x;
    bg.y = hg.y + ent_rect.h-1;

    bd.x = hg.x + ent_rect.w-1;
    bd.y = hg.y + ent_rect.h-1;

	std::vector<Point> points{hg, hd, bg, bd};
	bool have_one_who_hit = false;

	for(const Point& pt : points)
	{
		short px = pt.x/dest_bloc_w;
		short py = pt.y/dest_bloc_h;

		if(px < 0 or py < 0 or limit.x <= px or limit.y <= py)
				continue;	//Il n'y a aucune collision à l'extérieur de la map, donc on ignore le point.

		isCollisionTile_called_one_time_or_more_here = true;
		if(isCollisionTile(px, py, layer_index)) 	// /getDestBLOCZ() = conversion de pos_dans_map à tuile_dans_map
        {
			have_one_who_hit = true;

			if(out__tile_vals)
				INTERNAL_USE_addNewValueInVector(out__tile_vals, getTileValueAt(px, py, layer_index));
			else
				break;
		}
	}



	//Deuxième partie potentielle : si pas de collision encore détécté et qu'un côté est plus grand que le bloc de destination on vérifie aussi les tuiles entre les deux points des côtés.
	//Note : le bloc de destination est la taille en pixel d'une tuile sur la cible de rendu. getDestBLOC() donne la valeur initiale, getDestBLOCZ() donne la valeur réelle à traiter (où l'influence du zoom est incluse).

	if(not have_one_who_hit or out__tile_vals != nullptr)
	{
		std::vector<Point> other_points;

		if( std::abs(hg.y - bg.y) > dest_bloc_h ) // = côté gauche et droit de l'hitblox sont plus grands que getDestBLOCZ()
		{
			//ici les points sont directement traité en case
			//Note : j'utilise 'case' et 'tuile' sans distinction :)
			Point min_side_tile_pos = {hg.x/dest_bloc_w, hg.y/dest_bloc_h};
			Point max_side_tile_pos = {bg.x/dest_bloc_w, bg.y/dest_bloc_h};

			for(int y = min_side_tile_pos.y+1; y< max_side_tile_pos.y; y++) // = collecte des y des tuiles entre les deux points. 'min_side_tile_pos.y' est déjà vérifié plus haut, c'est pour ça que l'on commence avec le potentiel suivant. De même pour 'max_side_tile_pos.y', on ne va pas jusqu'à lui car déjà traité.
			{
				//point sur le côté gauche
				Point intern_lpt={min_side_tile_pos.x, y};
				//point sur le côté droit
				Point intern_rpt={hd.x/dest_bloc_w, y};

				other_points.push_back(intern_lpt);
				other_points.push_back(intern_rpt);
			}
		}

		if( std::abs(hg.x - hd.x) > dest_bloc_w ) // = côté du haut et du bas de l'hitbox sont plus grands que le getDestBLOCZ()
		{
			Point min_side_tile_pos = {hg.x/dest_bloc_w, hg.y/dest_bloc_h};
			Point max_side_tile_pos = {hd.x/dest_bloc_w, hd.y/dest_bloc_h};

			for(int x = min_side_tile_pos.x+1; x< max_side_tile_pos.x; x++)
			{
				//point sur le côté haut
				Point intern_upt = {x, min_side_tile_pos.y};
				//point sur le côté bas
				Point intern_dpt = {x, bg.y/dest_bloc_h};

				other_points.push_back(intern_upt);
				other_points.push_back(intern_dpt);
			}
		}

		for(const auto& pt : other_points)
		{
			if(pt.x < 0 or pt.y < 0 or limit.x <= pt.x or limit.y <= pt.y)
				continue;	//Il n'y a aucune collision à l'extérieur de la map, donc on ignore le point.

			isCollisionTile_called_one_time_or_more_here = true;
			if(isCollisionTile(pt.x, pt.y, layer_index))
			{
				have_one_who_hit = true;

				if(out__tile_vals)
					INTERNAL_USE_addNewValueInVector(out__tile_vals, getTileValueAt(pt.x, pt.y, layer_index));
				else
					break;
			}
		}
	}




	//Troisième cas potentiel : parcour des points interieurs.
	if(not have_one_who_hit or out__tile_vals != nullptr)
	if(std::abs(hg.y - bg.y) > dest_bloc_h or std::abs(hg.x - hd.x) > dest_bloc_w)
	{
		std::vector<Point> other_points;

		Point top_left = {hg.x/dest_bloc_w, hg.y/dest_bloc_h}; 	//la première tuile
		Point low_right = {bd.x/dest_bloc_w, bd.y/dest_bloc_h};	//la dernière tuile

		//ici seuls les tuiles de l'interieur sont vétifiés (aucune tuiles ici ne touche un bord de l'hitbox puisque ces points sont déjà vérifiés plus haut)
		for(int y=top_left.y+1; y< low_right.y; y++)
		{
			for(int x=top_left.x+1; x< low_right.x; x++)
			{
				Point pt = {x, y};
				other_points.push_back(pt);
			}
		}

		for(const auto& pt : other_points)
		{
			if(pt.x < 0 or pt.y < 0 or limit.x <= pt.x or limit.y <= pt.y)
				continue;	//Il n'y a aucune collision à l'extérieur de la map, donc on ignore le point.

			isCollisionTile_called_one_time_or_more_here = true;
			if(isCollisionTile(pt.x, pt.y, layer_index))
			{
				have_one_who_hit = true;

				if(out__tile_vals)
					INTERNAL_USE_addNewValueInVector(out__tile_vals, getTileValueAt(pt.x, pt.y, layer_index));
				else
					break;
			}
		}
	}

	//Note : on aurait pu très bien vérifier toutes les tuiles que touche le personnage d'un seul coup ; c'est une autre alternative ;)

	if(isCollisionTile_called_one_time_or_more_here)
		if(m_helper__error_at_last_call_of_isCollisionTile)
		{
			gs::lastError(gs::str("from 'gs::Map::hitCollisionTile()' : ")+gs::lastError());
			m_helper__error_at_last_call_of_hitCollisionTile = true;
		}

	return have_one_who_hit;
}


bool Map::willHitCollisionTile(FRect ent_rect, Vec2f total_pixel_incr, int layer_index, Vec2<float*> dist_before_hit__out,
                                    std::vector<short>* out__tile_vals, Rect area)
{
    //Recyclage d'un ancien code.

	/*
		total_pixel_incr_xy sont : de combien réellement la position va changer sur tel axe dans la frame. E.g 2.6 pixel en x, 1.7 en y.
        (Biensur allieurs, pour trouver à quelle pixel cela correspond on retire la partie décimale. Utiliser des float pour les pixels
		permet de bouger de moins d'un pixel par frame, permettant des pouvement plus lents). Ainsi l'utilisateur à le choix de mettre un décalge
		qui est la vitesse de son personnage, ou de faire correspondre cette vitesse avec le delta time par une multiplication, ou de mettre n'importe quelle valeur
		que son coeur désir.
		Le tout ici est d'avoir 'de combien' précisément il va bouger de pixels.

		Par ailleurs cette fonction ne traite pas la trajectoire, seulement la destination : si une collision se trouve
		entre le body et la destination mais que la destination est n'est pas une collision, le retour sera 'false'.
		Autrement dit attention au total_pixel_incr_xy > à la taille du bloc de destination.
	*/


    gs::FRect ent_move_rect = ent_rect;
	ent_move_rect.x += total_pixel_incr.x;
	ent_move_rect.y += total_pixel_incr.y;
	bool hit = hitCollisionTile(ent_move_rect, layer_index, out__tile_vals, area);

	if(m_helper__error_at_last_call_of_hitCollisionTile)
	{
		gs::lastError(gs::str("from 'gs::Map::willHitCollisionTile()' : ")+gs::lastError());
		return false;
	}


	//Calculs de la distance restante entre le gs::Body et la tuile qui va être cognée.
	if(hit) // = oui collision si mouvement efféctué.
	{
		short dest_bloc_w = getDestBLOCW();
		short dest_bloc_h = getDestBLOCH();

		if(dist_before_hit__out.x != nullptr)
		{
			bool go_to_left = (total_pixel_incr.x < 0 ? true : false);

            float body_pos_x = std::floor(ent_rect.x);

			if(not go_to_left)
                body_pos_x += ent_rect.w;	//pour prendre en compte son côté droit plutôt

			int collision_tile_pos_x = static_cast<int>(body_pos_x + total_pixel_incr.x);
				collision_tile_pos_x /= dest_bloc_w;	//pour trouver la colonne de tuile correspondante
				collision_tile_pos_x *= dest_bloc_w;	//pour trouver le premier pixel correspondant à cette colonne

			if(go_to_left)
			{
				collision_tile_pos_x += dest_bloc_w;	//pour décaler vers la droite de cette colonne puis que l'on va conger sa droite
				*dist_before_hit__out.x = body_pos_x - static_cast<float>(collision_tile_pos_x);
			}
			else
				*dist_before_hit__out.x = static_cast<float>(collision_tile_pos_x) - body_pos_x;

			if(*dist_before_hit__out.x < 0 or total_pixel_incr.x == 0)
				*dist_before_hit__out.x = 0.f;

		}


		if(dist_before_hit__out.y != nullptr)
		{
			bool go_to_up = (total_pixel_incr.y < 0 ? true : false);

            float body_pos_y = std::floor(ent_rect.y);

			if(not go_to_up)
                body_pos_y += ent_rect.h;	//pour prendre en compte son côté inférieur plutôt

			int collision_tile_pos_y = static_cast<int>(body_pos_y + total_pixel_incr.y);
				collision_tile_pos_y /= dest_bloc_h;	//pour trouver la ligne de tuile correspondante
				collision_tile_pos_y *= dest_bloc_h;	//pour trouver le premier pixel correspondant à cette ligne

			if(go_to_up)
			{
				collision_tile_pos_y += dest_bloc_h;	//pour décaler vers le bas de cette ligne puis que l'on va conger son bas
				*dist_before_hit__out.y = body_pos_y - static_cast<float>(collision_tile_pos_y);
			}
			else
				*dist_before_hit__out.y = static_cast<float>(collision_tile_pos_y) - body_pos_y;

			if(*dist_before_hit__out.y < 0 or total_pixel_incr.y == 0)
				*dist_before_hit__out.y = 0.f;

		}
	}
	else
	{
		if(dist_before_hit__out.x != nullptr)
			*dist_before_hit__out.x = 0.f;

		if(dist_before_hit__out.y != nullptr)
			*dist_before_hit__out.y = 0.f;

		// 0 puisqu'il n'y a pas eu de collision en vu.
	}

	return hit;
}









bool Map::addCollisionTilesValues(int layer_index, std::vector<short> values)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(gs::str("||Warning|| in 'addCollisionTilesValue()' of '")+m_title+gs::str("', given layer index doesn't exist"));
		return false;
	}

	bool found = false;
	Vec_Int_VecSInt* collision_lay_ptr=nullptr;

	for(Vec_Int_VecSInt& collision_lay : m_collision_layers)
	{
		if(collision_lay.m_id == layer_index)
		{
			found = true;
			collision_lay_ptr = &collision_lay;
			break;
		}
	}

	if(not found or collision_lay_ptr == nullptr)
	{
		gs::lastError(gs::str("||Warning|| in 'addCollisionTilesValue()' of '")+m_title+gs::str("', layer ")+std::to_string(layer_index)+gs::str(" seems not set as a collision layer. "));
		return false;
	}


	{//Petit algorithme pour ne pas rajouter de doublons.

		std::vector<short>& lay_coll_values = collision_lay_ptr->m_values;

		for(short& val : values)
		{
			std::vector<short>::iterator it;
			it = std::find(lay_coll_values.begin(), lay_coll_values.end(), val);

			if( it == lay_coll_values.end() )
			{
				lay_coll_values.push_back(val);
			}
		}
	}

	return true;
}







bool Map::removeCollisionTilesValues(int layer_index, std::vector<short> values)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(gs::str("||Warning|| in 'removeCollisionTilesValue()' of '")+m_title+gs::str("', given layer index doesn't exist"));
		return false;
	}

	bool found = false;
	Vec_Int_VecSInt* collision_lay_ptr=nullptr;

	for(Vec_Int_VecSInt& collision_lay : m_collision_layers)
	{
		if(collision_lay.m_id == layer_index)
		{
			found = true;
			collision_lay_ptr = &collision_lay;
			break;
		}
	}

	if(not found or collision_lay_ptr == nullptr)
	{
		gs::lastError(gs::str("||Warning|| in 'removeCollisionTilesValue()' of '")+m_title+gs::str("', layer ")+std::to_string(layer_index)+gs::str(" seems not set as a collision layer. "));
		return false;
	}


	{
		std::vector<short>& lay_coll_values = collision_lay_ptr->m_values;
		std::vector<short>::iterator it;

		for(const short& val : values)
		{
			it = std::find(lay_coll_values.begin(), lay_coll_values.end(), val);

			if(*it == val)
			{
				lay_coll_values.erase(it);
			}
		}
	}

	return true;
}






std::vector<short> Map::getCollisionTilesValues(int layer_index)
{
	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(gs::str("||Warning|| in 'getCollisionTilesValue()' of '")+m_title+gs::str("', given layer index doesn't exist"));
		return std::vector<short>();
	}

	bool found = false;
	Vec_Int_VecSInt* collision_lay_ptr=nullptr;

	for(Vec_Int_VecSInt& collision_lay : m_collision_layers)
	{
		if(collision_lay.m_id == layer_index)
		{
			found = true;
			collision_lay_ptr = &collision_lay;
			break;
		}
	}

	if(not found or collision_lay_ptr == nullptr)
	{
		gs::lastError(gs::str("||Warning|| in 'getCollisionTilesValue()' of '")+m_title+gs::str("', layer ")+std::to_string(layer_index)+gs::str(" seems not set as a collision layer. "));
		return std::vector<short>();
	}

	return collision_lay_ptr->m_values;
}


Collision_Mask Map::getCollisionMask(int layer_index)
{
	std::vector<std::vector<char>> grid;
	Collision_Mask err(grid);

	if(isInvalidLayerIndex(layer_index))
	{
		gs::lastError(gs::str("||Warning|| in 'getCollisionMask()' of '")+m_title+gs::str("', given layer index doesn't exist"));
		return err;
	}

	std::vector<short>&& coll_tiles_values = getCollisionTilesValues(layer_index); //Récupération des valeurs pour lesquelles les tuiles sont des tuiles de collision.

	if(coll_tiles_values.empty())
	{
		gs::lastError(gs::str("from 'getCollisionMask()' : ")+gs::lastError());
		return err;
	}

	//A ce stade le 'layer_index' indique bien un layer associé à de la collision, grâce à 'getCollisionTilesValues()'.

	//Initialisation de la grille.
	Vec2i layer_size = getTilesCount(layer_index);
	grid.resize(layer_size.y);
	for(std::vector<char>& ln : grid)
		ln.resize(layer_size.x);

	//Remplissage de la grille, avec des 1 (pour collision) et des 0 (pour non collision).
	Map_Layer& layer = m_layers.at(layer_index);
	for(int _x=0; _x<layer_size.x; ++_x)
	for(int _y=0; _y<layer_size.y; ++_y)
	{
		int tile_val = layer.m_map_array[_y][_x].m_value;

		for(const short& coll_val : coll_tiles_values) //Parcours des valeurs de collision.
		{
			if(tile_val == coll_val) //Si la tuile courante a une valeur de collision.
			{
				grid[_y][_x] = 1;
				break;
			}
			else
				grid[_y][_x] = 0;
		}
	}


	return Collision_Mask(grid);
}




































//__________________________________________________________________________________________________________
//_____Backgrounds__________________________________________________________________________________________

bool Map::addBackgroundStack(uint16_t count)
{
	for(int i=0; i< count; i++)
		m_backgrounds_stacks.emplace_back();
	return true;
}



bool Map::popBackBackgroundStack()
{
	if(m_backgrounds_stacks.empty())
	{
		gs::lastError(std::format("||Warning|| in 'popBackBackgroundStack()' of '{}', no have any background stack to pop. ", m_title));
		return false;
	}

	m_backgrounds_stacks.pop_back();
	return true;
}


bool Map::popBackBackground(short stack_index)
{
	bool all_stacks=false;
	if(not INTERNAL_USE_stackIndexIsOk("popBackBackground()", stack_index, all_stacks))
		return false;

	if(all_stacks)
	{
		for(std::vector<Background>& backgrounds_stack : m_backgrounds_stacks)
			backgrounds_stack.pop_back();
	}
	else
		m_backgrounds_stacks.at(stack_index).pop_back();

	return true;
}




bool Map::isInvalidBackgroundStackIndex(short stack_index) const
{
	if(stack_index < 0 or stack_index >= m_backgrounds_stacks.size())
		return true;
	return false;
}

bool Map::isInvalidBackgroundIndex(short stack_index, short background_index)
{
	if(isInvalidBackgroundStackIndex(stack_index))
		return true;

	std::vector<Background>& backgrounds_stack = m_backgrounds_stacks.at(stack_index);

	if(background_index < 0 or (background_index > (int)backgrounds_stack.size()-1))	//attention à ne pas oublier la conversion en int car le retour de size() est size_t (= pas de négatif). Autre solution, faire un supérieur ou égal.
		return true;
	return false;
}


bool Map::INTERNAL_USE_stackIndexIsOk(std::string function_name, short stack_index, bool& is_default_index) const
{
	if(m_backgrounds_stacks.empty())
	{
		gs::lastError(std::format("||Warning|| in 'Map::{}' of '{}', no have any background stack in this 'gs::Map'. ", function_name, m_title));
		return false;
	}

	is_default_index = false;
	if(stack_index == -1)
		is_default_index = true;

	else
	if(isInvalidBackgroundStackIndex(stack_index))
	{
		gs::lastError(std::format("||Warning|| in 'Map::{}' of '{}', invalid stack index. [given index : {}, max index : {}]. ", function_name, m_title, stack_index, (int)m_backgrounds_stacks.size()-1));
		return false;
	}

	return true;
}



bool Map::INTERNAL_USE_backgroundCooAreOk(std::string function_name, short stack_index, bool& is_default_stack_index, short background_index, bool& is_default_background_index)
{
	bool stack_is_ok = INTERNAL_USE_stackIndexIsOk(function_name, stack_index, is_default_stack_index);
	if(not stack_is_ok)
		return false;

	is_default_background_index = false;

	if(not is_default_stack_index)
	{
		std::vector<Background>& backgrounds_stack = m_backgrounds_stacks.at(stack_index);

		if(backgrounds_stack.empty())
		{
			gs::lastError(std::format("||Warning|| in 'Map::{}' of '{}', no have any background in the stack {}. ", function_name, m_title, stack_index));
			return false;
		}

		if(background_index == -1)
			is_default_background_index = true;

		else
		if(background_index < 0 or (background_index > (int)backgrounds_stack.size()-1))	//attention à ne pas oublier la conversion en int car le retour de size() est size_t (= pas de négatif). Autre solution, faire un supérieur ou égal.
		{
			gs::lastError(std::format("||Warning|| in 'Map::{}' of '{}', invalid background index in the stack {}. [given index : {}, max index : {}]. ", function_name, m_title, stack_index, background_index, (int)backgrounds_stack.size()-1));
			return false;
		}
	}
	else
	{
		int st_i = 0;
		for(auto& backgrounds_stack : m_backgrounds_stacks)
		{

			if(background_index == -1)
				is_default_background_index = true;

			else // if(background_index != -1)
			{
				if(backgrounds_stack.empty())
				{
					gs::lastError(std::format("||Warning|| in 'Map::{}' of '{}', required background index {} in all stacks, but the stack {} is empty. ", function_name, m_title, background_index, st_i));
					return false;
				}

				if(background_index < 0 or (background_index > (int)backgrounds_stack.size()-1))	//attention à ne pas oublier la conversion en int car le retour de size() est size_t (= pas de négatif). Autre solution, faire un supérieur ou égal.
				{
					gs::lastError(std::format("||Warning|| in 'Map::{}' of '{}', required background index {} in all stacks, but is invalid in the stack {} witch max index is {}. ", function_name, m_title, background_index, st_i, (int)backgrounds_stack.size()-1));
					return false;
				}
			}

			st_i++;
		}
	}

	return true;
}






bool Map::addBackground(short stack_index, const std::string& file_path, Background_Mode mode, Vec2f sensibility, Vec2b scrolling_allowed)
{
	if(not isRenValid())
	{
		gs::lastError(gs::str("||Warning|| in 'addBackground()' of '")+m_title+gs::str("', cannot add : missing a valid 'Renderer*'. Make sure you have set a valid 'Renderer*'."));
		return false;
	}

	bool _skip_arg_;
	if(not INTERNAL_USE_stackIndexIsOk("addBackground()", stack_index, _skip_arg_))
	{
		gs::lastError(gs::lastError()+ gs::str("Try to add stack-s of background, or check the stack index if there are already stack-s in. "));
		return false;
	}


	std::vector<Background>& backgrounds_stack = m_backgrounds_stacks.at(stack_index);
	backgrounds_stack.emplace_back();
	Background& new_background = backgrounds_stack.back();


	new_background.m_image = m_loadTexture(file_path);

	if(not new_background.m_image)
	{
		gs::lastError(std::format("from 'addBackground()' of '{}' : {}", m_title, gs::lastError()));
		return false;
	}

	Vec2f tmp;
	getTextureSize(new_background.m_image, &tmp.x, &tmp.y);
	new_background.m_w = tmp.x;
	new_background.m_h = tmp.y;
	new_background.m_1st_image_file_path = file_path;
	new_background.m_mode=mode;
	new_background.m_sensibility = sensibility;
	new_background.m_scrolling_allowed = scrolling_allowed;
	new_background.m_src.x = 0;
	new_background.m_src.y = 0;
	new_background.m_src.w = new_background.m_w;
	new_background.m_src.h = new_background.m_h;
	new_background.m_is_init = true;



	return true;
}













bool Map::drawBackground(short stack_index, short background_index)	// -1 par défaut pour signifier de tout dessiner (dans l'ordre d'ajout)
{
	if(not isRenValid())
	{
		gs::lastError(std::format("||Warning|| in 'drawBackground()' of '{}', cannot add : missing a valid 'Renderer*'. Make sure you have set a valid 'Renderer*'. ", m_title));
		return false;
	}

	bool all_stacks=false;
	bool draw_all=false; 	//par stack
	if(not INTERNAL_USE_backgroundCooAreOk("drawBackground()", stack_index, all_stacks, background_index, draw_all))
		return false;


	std::vector<short> stack_indexes;
	if(not all_stacks)
		stack_indexes.push_back(stack_index);

	else
		for(int i=0; i< m_backgrounds_stacks.size(); i++)
			stack_indexes.push_back(i);



	for(short& st_i : stack_indexes)
	{
		std::vector<Background>& backgrounds_stack = m_backgrounds_stacks.at(st_i);


		std::vector<short> background_indexes;	//collecte des index à dessiner
		if(not draw_all)
			background_indexes.push_back(background_index);

		else
			for(size_t i=0; i< backgrounds_stack.size(); i++)
				background_indexes.push_back(i);



		for(short& i : background_indexes)
		{
			Background& bg = backgrounds_stack.at(i);

			if(bg.m_mode == FILL)
			{
				if(not drawTexture(m_ren, bg.m_image, &bg.m_src, nullptr))
				{
					gs::lastError(std::format("||Warning|| in 'drawBackground()' of '{}', [stack index : {}, background index : {}, first texture from '{}'], cannot draw : {}. ", m_title, st_i, i, bg.m_1st_image_file_path, Backend_GetError()));
					return false;
				}
			}
			else
			if(bg.m_mode == PARALLAX)
			{
				bg.m_param.current_dest_rect.x = bg.m_param.current_pos.x;
				bg.m_param.current_dest_rect.y = bg.m_param.current_pos.y;

				Vec2f tx_size;
				getTextureSize(bg.m_image, &tx_size.x, &tx_size.y);

				if(bg.m_param.mode == FILL_SCREEN)
				{
					bg.m_param.current_dest_rect.w = m_getRenW();
					bg.m_param.current_dest_rect.h = m_getRenH();
				}
				else
				if(bg.m_param.mode == ORIGINAL_SIZE)
				{
					bg.m_param.current_dest_rect.w = tx_size.x;
					bg.m_param.current_dest_rect.h = tx_size.y;
				}
				else
				if(bg.m_param.mode == CUSTOM_SIZE)
				{
					bg.m_param.current_dest_rect.w = bg.m_param.custom_dest_rect.w;
					bg.m_param.current_dest_rect.h = bg.m_param.custom_dest_rect.h;
				}
				else
				if(bg.m_param.mode == FILL_SCREEN_IN_W)
				{
					bg.m_w = tx_size.x; bg.m_h = tx_size.y;

					double coef = double(m_getRenW()) / double(bg.m_w);
					bg.m_param.current_dest_rect.w = std::round(double(bg.m_w) * coef);
					bg.m_param.current_dest_rect.h = std::round(double(bg.m_h) * coef);
				}
				else
				if(bg.m_param.mode == FILL_SCREEN_IN_H)
				{
					bg.m_w = tx_size.x; bg.m_h = tx_size.y;

					double coef = double(m_getRenH()) / double(bg.m_h);
					bg.m_param.current_dest_rect.w = std::round(double(bg.m_w) * coef);
					bg.m_param.current_dest_rect.h = std::round(double(bg.m_h) * coef);
				}

				if(not drawTexture(m_ren, bg.m_image, &bg.m_src, &bg.m_param.current_dest_rect))
				{
					gs::lastError(std::format("||Warning|| in 'drawBackground()' of '{}', [stack index : {}, background index : {}, first texture from '{}'], cannot draw : {}. ", m_title, st_i, i, bg.m_1st_image_file_path, Backend_GetError()));
					return false;
				}
				else
				//partie de la mise en place de l'illusion (= remplissage des trous)
				{
					//Le but de l'algorithme est de remplir toute la bande x et/ou y par l'image.
					//C-à-d que lorsque l'image est dans une position où elle ne rempli pas toute la bande (soit car elle est trop petite, soit car elle scroll) on complète les trous (que se soit à gauche, à droite ou les deux pour la bande x, et pareilement avec le haut et le bas dans le cas de la bande y).
					//Note : on peut traiter le scrolling dans les deux axes en même temps, ça ne gène pas.

					std::vector<int> bonus_render_copy_x; 	//Stocke les x des RenderCopy bonus pour combler les trous. Ainsi s'il y a également le scrolling y, on répétera ces RenderCopy pour chaque y de comblage.

					if(bg.m_scrolling_allowed.x)
					{
						FRect dest_rect = bg.m_param.current_dest_rect;
						float min_x = dest_rect.x;
						float max_x = dest_rect.x+dest_rect.w-1;

						//combler les potentiels trous à gauche
						for(int i=0; min_x > 0 and dest_rect.w > 0; i++)	//dest_rect.w > 0 sinon boucle infini
						{
							FRect tmp = {min_x-dest_rect.w, dest_rect.y, dest_rect.w, dest_rect.h};
							drawTexture(m_ren, bg.m_image, &bg.m_src, &tmp);

							bonus_render_copy_x.push_back(tmp.x);
							min_x = tmp.x;

							if(i >= 20)
								break;	//Pour éviter une boucle trop grand on stoppe la boucle quoi qu'il arrive à ce stade car généralement avec un background de taille raisonable i tourne au alentours de 0 et 2. Si le programme vient en arriver à là c'est qu'il faut probablement revoir les dimensions du backgrounds car trop petites pour le rendu (du point du vu où c'est sensé recouvrir une grande surface).
						}

						int ren_w = m_getRenW();

						//combler les potentiels trous à droite
						for(int i=0; max_x < ren_w-1 and dest_rect.w > 0; i++)	//dest_rect.w > 0 sinon boucle infini
						{
							FRect tmp = {max_x+1, dest_rect.y, dest_rect.w, dest_rect.h};
							drawTexture(m_ren, bg.m_image, &bg.m_src, &tmp);

							bonus_render_copy_x.push_back(tmp.x);
							max_x = tmp.x+tmp.w-1;

							if(i >= 20)
								break;
						}


						if(bg.m_move_when_draw)
							moveBackground(bg.m_speed.x, 0.f, st_i, i);


					}





					if(bg.m_scrolling_allowed.y)
					{
						FRect dest_rect = bg.m_param.current_dest_rect;
						float min_y = dest_rect.y;
						float max_y = dest_rect.y+dest_rect.h-1;

						//combler les potentiels trous en haut
						for(int i=0; min_y > 0 and dest_rect.h > 0; i++)	//dest_rect.h > 0 sinon boucle infini
						{
							FRect tmp = {dest_rect.x, min_y-dest_rect.h, dest_rect.w, dest_rect.h};
							drawTexture(m_ren, bg.m_image, &bg.m_src, &tmp);

								for(int& _x_ : bonus_render_copy_x)
								{
									FRect tmp2 = {float(_x_), tmp.y, dest_rect.w, dest_rect.h};
									drawTexture(m_ren, bg.m_image, &bg.m_src, &tmp2);
								}

							min_y = tmp.y;

							if(i >= 20)
								break;	//Pour éviter une boucle trop grand on stoppe la boucle quoi qu'il arrive à ce stade car généralement avec un background de taille raisonable i tourne au alentours de 0 et 2. Si le programme vient en arriver à là c'est qu'il faut probablement revoir les dimensions du backgrounds car trop petites pour le rendu (du point du vu où c'est sensé recouvrir une grande surface).
						}


						int ren_h = m_getRenH();

						//combler les potentiels trous en bas
						for(int i=0; max_y < ren_h-1 and dest_rect.h > 0; i++)	//dest_rect.h > 0 sinon boucle infini
						{
							FRect tmp = {dest_rect.x, max_y+1, dest_rect.w, dest_rect.h};
							drawTexture(m_ren, bg.m_image, &bg.m_src, &tmp);

								for(int& _x_ : bonus_render_copy_x)
								{
									FRect tmp2 = {float(_x_), tmp.y, dest_rect.w, dest_rect.h};
									drawTexture(m_ren, bg.m_image, &bg.m_src, &tmp2);
								}

							max_y = tmp.y+tmp.h-1;

							if(i >= 20)
								break;
						}


						if(bg.m_move_when_draw)
							moveBackground(0.f, bg.m_speed.y, st_i, i);

					}


				}
			}
		}
	}

	return true;
}







bool Map::moveBackground(short stack_index, short background_index, float x_pix, float y_pix)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("moveBackground()", stack_index, background_index);

	if(not given_bg_s)
		return false;

	for(Background* bg_ptr : *given_bg_s)
	{
		bg_ptr->incrStartX(x_pix);
		bg_ptr->incrStartY(y_pix);
	}

	return true;
}


















std::vector<Background*>* Map::INTERNAL_USE_getTargetBackgrounds(std::string function_name, short stack_index, short background_index)
{
	bool all_stacks = false;
	bool edit_all = false; // = à l'échelle d'une stack
	if(not INTERNAL_USE_backgroundCooAreOk(function_name, stack_index, all_stacks, background_index, edit_all))
		return nullptr;

	m_helper_return_bg_array.clear();

	std::vector<int> stack_indexes;
	if(not all_stacks)
		stack_indexes.push_back(stack_index);

	else
		for(int i=0; i< m_backgrounds_stacks.size(); i++)
			stack_indexes.push_back(i);


	for(int& st_i : stack_indexes)
	{
		std::vector<Background>& backgrounds_stack = m_backgrounds_stacks.at(st_i);

		std::vector<short> background_indexes;
		if(not edit_all)
			background_indexes.push_back(background_index);

		else
			for(size_t i=0; i< backgrounds_stack.size(); i++)
				background_indexes.push_back(i);


		for(short& i : background_indexes)
		{
			Background& bg = backgrounds_stack.at(i);

			m_helper_return_bg_array.push_back(&bg);
		}
	}

	return &m_helper_return_bg_array;
}






bool Map::changeBackgroundMode(short stack_index, short background_index, Background_Mode mode)
{
	bool all_stacks=false;
	bool change_all=false;
	if(not INTERNAL_USE_backgroundCooAreOk("changeBackgroundMode()", stack_index, all_stacks, background_index, change_all))
		return false;


	std::vector<short> stack_indexes;
	if(not all_stacks)
		stack_indexes.push_back(stack_index);

	else
		for(int i=0; i< m_backgrounds_stacks.size(); i++)
			stack_indexes.push_back(i);



	for(short& st_i : stack_indexes)
	{
		std::vector<Background>& backgrounds_stack = m_backgrounds_stacks.at(st_i);


		std::vector<short> background_indexes;
		if(not change_all)
			background_indexes.push_back(background_index);

		else
			for(size_t i=0; i< backgrounds_stack.size(); i++)
				background_indexes.push_back(i);

		for(short& i : background_indexes)
		{
			Background& bg = backgrounds_stack.at(i);

			bg.m_mode = mode;
		}
	}

	return true;
}





bool Map::setBackgroundMoveWhenDraw(short stack_index, short background_index, bool new_state)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("setBackgroundMoveWhenDraw()", stack_index, background_index);

	if(not given_bg_s)
		return false;

	for(Background* bg_ptr : *given_bg_s)
		bg_ptr->m_move_when_draw = new_state;

	return true;
}





















bool Map::setBackgroundSpeedX(short stack_index, short background_index, float val)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("setBackgroundSpeedX()", stack_index, background_index);

	if(not given_bg_s)
		return false;

	for(Background* bg_ptr : *given_bg_s)
		bg_ptr->m_speed.x = val;

	return true;
}




bool Map::setBackgroundSpeedY(short stack_index, short background_index, float val)
{


	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("setBackgroundSpeedY()", stack_index, background_index);

	if(not given_bg_s)
		return false;

	for(Background* bg_ptr : *given_bg_s)
		bg_ptr->m_speed.y = val;

	return true;
}





bool Map::setBackgroundSpeedXY(short stack_index, short background_index, float val_x, float val_y)
{
	bool success = true;
	success = success & setBackgroundSpeedX(stack_index, background_index, val_x);
	success = success & setBackgroundSpeedY(stack_index, background_index, val_y);

	if(not success)
		gs::lastError(gs::str("from 'setBackgroundSpeedXY()' : ")+gs::lastError());

	return success;
}




bool Map::editBackgroundParallaxParam(
	short 					stack_index, short background_index,
	Parallax_Param_Field 	field, 	int solo_val, Rect non_solo_vals)
{

	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("editBackgroundParallaxParam()", stack_index, background_index);

	if(not given_bg_s)
		return false;

	bool success = true;

	for(Background* bg_ptr : *given_bg_s)
	{
		Background& bg = *bg_ptr;

		if(bg.m_mode != PARALLAX) // On reste rigoureux ;)
		{
			gs::lastError(std::format("||Warning|| in 'editBackgroundParallaxParam()' of '{}', cannot edit : this background is not on paralax mode. Try to change it. ", m_title));
			return false;
		}

		Parallax_Param& param = bg.m_param;

		/* début des traitements */
		switch(field)
		{
			default :
				success = false;
				gs::lastError(std::format("||Warning|| in 'editBackgroundParallaxParam()' of '{}', no parameter field correspond with given field. ", m_title));
				break;

			case APPEARANCE_MODE :
				try{
					param.mode = (Parallax_Dest_Appearance)solo_val;
				}
				catch(std::exception& er)
				{
					success = false;
					gs::lastError(std::format("||Warning|| in 'editBackgroundParallaxParam()' of '{}', error occured when convert to `Paralax_Dest_Appearance' : {}", m_title, er.what()));
				}
				break;

			case CUSTOM_DEST_RECT_X :
				param.custom_dest_rect.x = solo_val;
				break;

			case CUSTOM_DEST_RECT_Y :
				param.custom_dest_rect.y = solo_val;
				break;

			case CUSTOM_DEST_RECT_W :
				param.custom_dest_rect.w = solo_val;
				break;

			case CUSTOM_DEST_RECT_H :
				param.custom_dest_rect.h = solo_val;
				break;

			case CURRENT_DEST_RECT_X :
				param.current_dest_rect.x = solo_val;
				break;

			case CURRENT_DEST_RECT_Y :
				param.current_dest_rect.y = solo_val;
				break;

			case CURRENT_DEST_RECT_W :
				param.current_dest_rect.w = solo_val;
				break;

			case CURRENT_DEST_RECT_H :
				param.current_dest_rect.h = solo_val;
				break;

			case BEGIN_POS_X :
				param.begin_pos.x = solo_val;
				break;

			case BEGIN_POS_Y :
				param.begin_pos.y = solo_val;
				break;

			case CURRENT_POS_X :
				param.current_pos.x = solo_val;
				break;

			case CURRENT_POS_Y :
				param.current_pos.y = solo_val;
				break;

			case CUSTOM_DEST_RECT :
				param.custom_dest_rect = frect(non_solo_vals);
				break;

			case CURRENT_DEST_RECT :
				param.current_dest_rect = frect(non_solo_vals);
				break;

			case BEGIN_POS_XY :
				param.begin_pos.x = non_solo_vals.x;
				param.begin_pos.y = non_solo_vals.y;
				break;

			case CURRENT_POS_XY :
				param.current_pos.x = non_solo_vals.x;
				param.current_pos.y = non_solo_vals.y;
				break;

			case CUSTOM_DEST_RECT_WH :
				param.custom_dest_rect.w = non_solo_vals.x;	//Histoire de n'avoir qu'à remplir le deux primiers champs lors de l'appel de la fonction : e.g editBackgroundParalaxParam(..WH, 0, {800, 480});
				param.custom_dest_rect.h = non_solo_vals.y;
				break;

			case CURRENT_DEST_RECT_WH :
				param.current_dest_rect.w = non_solo_vals.x;
				param.current_dest_rect.h = non_solo_vals.y;
				break;


		}//fin du switch

	}

	return success;
}








bool Map::setBackgroundSensibilityX(short stack_index, short background_index, float val)
{

	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("setBackgroundSensibilityX()", stack_index, background_index);

	if(not given_bg_s)
		return false; //Les messages d'erreur sont déjà traités dans une fonction de la fonction précédente.

	for(Background* bg_ptr : *given_bg_s)
		bg_ptr->m_sensibility.x = val;

	return true;
}





bool Map::setBackgroundSensibilityY(short stack_index, short background_index, float val)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("setBackgroundSensibilityY()", stack_index, background_index);

	if(not given_bg_s)
		return false;

	for(Background* bg_ptr : *given_bg_s)
		bg_ptr->m_sensibility.y = val;

	return true;
}




bool Map::setBackgroundSensibilityXY(short stack_index, short background_index, float val_x, float val_y)
{
	bool success = true;
	success = success & setBackgroundSensibilityX(stack_index, background_index, val_x);
	success = success & setBackgroundSensibilityY(stack_index, background_index, val_y);

	if(not success)
		gs::lastError(gs::str("from 'setBackgroundSensibilityXY()' : ")+gs::lastError());

	return success;
}







bool Map::setBackgroundScrollingXActiveState(short stack_index, short background_index, bool new_state)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("setBackgroundScrollingXActiveState()", stack_index, background_index);

	if(not given_bg_s)
		return false;

	for(Background* bg_ptr : *given_bg_s)
		bg_ptr->m_scrolling_allowed.x = new_state;

	return true;
}




bool Map::setBackgroundScrollingYActiveState(short stack_index, short background_index, bool new_state)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("setBackgroundScrollingYActiveState()", stack_index, background_index);

	if(not given_bg_s)
		return false;

	for(Background* bg_ptr : *given_bg_s)
		bg_ptr->m_scrolling_allowed.y = new_state;

	return true;
}



bool Map::setBackgroundScrollingXYActiveState(short stack_index, short background_index, bool new_state_x, bool new_state_y)
{
	bool success = true;
	success = success & setBackgroundScrollingXActiveState(stack_index, background_index, new_state_x);
	success = success & setBackgroundScrollingYActiveState(stack_index, background_index, new_state_y);

	if(not success)
		gs::lastError(gs::str("from 'setBackgroundScrollingXYActiveState()' : ")+gs::lastError());

	return success;
}



bool Map::setBackgroundImage(short stack_index, short background_index, Texture* texture, bool destroy_previous)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("setBackgroundImage()", stack_index, background_index);

	if(not given_bg_s)
		return false;

	for(Background* bg_ptr : *given_bg_s)
	{
		if(destroy_previous)
		{
			if(bg_ptr->m_image)
				gs::destroyTexture(bg_ptr->m_image);
		}
		else
			::gs::addToClearAtTheEnd(bg_ptr->m_image);

		if(texture)
			bg_ptr->m_image = texture;
		else
		{
			gs::lastError(std::format("||Warning|| in 'setBackgroundImage()' of '{}', given texture is invalid. ", m_title));
			return false;
		}
	}

	return true;
}








bool Map::setBackgroundImage(short stack_index, short background_index, const std::string& file_path, bool destroy_previous)
{
	if(not isRenValid())
	{
		gs::lastError(std::format("||Warning|| in 'setBackgroundImage() [with file path]' of '{}', cannot set : missing a valid 'Renderer*'. Make sure you have set a valid 'Renderer*'. ", m_title));
		return false;
	}

	Texture* tex = m_loadTexture(file_path);

	if(not tex)
	{
		gs::lastError(std::format("||Warning|| in 'setBackgroundImage() [with file path]' of '{}', image load failed : {} ", m_title, gs::lastError()));
		return false;
	}

	return setBackgroundImage(stack_index, background_index, tex, destroy_previous);
}






bool Map::setBackgroundSrc(short stack_index, short background_index, FRect vals)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("setBackgroundSrc()", stack_index, background_index);

	if(not given_bg_s)
		return false;

	if( vals.x < 0 || vals.y < 0 || vals.w < 0 || vals.h < 0 )
	{
		gs::lastError(std::format("||Error|| in 'setBackgroundSrc()' of '{}', one of given rect values [x,y,w,h] is < 0. Src accepts only positive values. ", m_title));
		return false;
	}

	for(Background* bg_ptr : *given_bg_s)
		bg_ptr->m_src = vals;

	return true;

}

bool Map::setBackgroundSrc(short stack_index, short background_index, int* x, int* y, int* w, int* h)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("setBackgroundSrc()", stack_index, background_index);

	if(not given_bg_s)
		return false;

	if( x || y || w || h )
	{
		for(Background* bg_ptr : *given_bg_s)
		{
			if(x)
			{
				if((*x) < 0)
					gs::lastError(std::format("||Warning|| in 'setBackgroundSrc()' of '{}', cannot accpet a negative x value. [given x : {}] ", m_title, *x));
				else
					bg_ptr->m_src.x = *x;
			}

			if(y)
			{
				if((*y) < 0)
					gs::lastError(std::format("||Warning|| in 'setBackgroundSrc()' of '{}', cannot accpet a negative y value. [given y : {}] ", m_title, *y));
				else
					bg_ptr->m_src.y = *y;
			}

			if(w)
			{
				if((*w) < 0)
					gs::lastError(std::format("||Warning|| in 'setBackgroundSrc()' of '{}', cannot accpet a negative w value. [given w : {}] ", m_title, (*w)));
				else
					bg_ptr->m_src.w = *w;
			}

			if(h)
			{
				if((*h) < 0)
					gs::lastError(std::format("||Warning|| in 'setBackgroundSrc()' of '{}', cannot accpet a negative h value. [given h : {}] ", m_title, *h));
				else
					bg_ptr->m_src.h = *h;
			}
		}
	}
	else
	{
		gs::lastError(std::format("||Litle Warning|| in 'setBackgroundSrc()' of '{}', there is no given values. ", m_title));
		return false;
	}

	return true;
}





// *************************************************************************
//	 *******************************************************************
//	 	***********************************************************
//	 		***************************************************
//	 			*******************************************
//	 			*******************************************
//	 			*******************************************
//	 		***************************************************
//	 	***********************************************************
//	 *******************************************************************
// **************************************************************************





short Map::getNbOfBackgroundStacks() const
{
	return static_cast<short>(m_backgrounds_stacks.size());
}


short Map::getNbOfBackgrounds(short stack_index) const
{
	if(isInvalidBackgroundStackIndex(stack_index))
	{
		gs::lastError(std::format("||Warning|| in 'getNbOfBackgrounds()' of '{}', invalid stack index. [given index : {}, max index : {}]. ", m_title, stack_index, (int)m_backgrounds_stacks.size()-1));
		return 0;
	}

	return static_cast<short>(m_backgrounds_stacks.at(stack_index).size());
}


Parallax_Param* Map::getBackgroundParallaxParam(short stack_index, short background_index)
{
	bool _skip_arg_, _skip_arg2;
	if(not INTERNAL_USE_backgroundCooAreOk("getBackgroundParallaxParam()", stack_index, _skip_arg_, background_index, _skip_arg2))
		return nullptr;

	return &m_backgrounds_stacks.at(stack_index).at(background_index).m_param;
}



FRect Map::getBackgroundParallaxParam(short stack_index, short background_index, Parallax_Param_Field field)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("getBackgroundParallaxParam()", stack_index, background_index);

	FRect return_val = {0,0,0,0};

	if(not given_bg_s)
		return {0.f,0.f,-1.f,-1.f};

	bool success = true;

	/*
	Pour les valeurs seules : le retour est stocké dans le 'x'; les autres champs seront nulles. E.g : getBackgroundParallaxParam(0, 2, gs::ma::BEGIN_POS_Y).x;
	pour les valeurs duo : les retours sont stockés dans les champs 'x' et 'y';
	Pour les rectangles complets : la valeur retourné est le rectangle demandé tout simplement.
	*/


	for(Background* bg_ptr : *given_bg_s)
	{
		Background& bg = *bg_ptr;

		if(bg.m_mode != PARALLAX) // On reste rigoureux ;)
		{
			gs::lastError(std::format("||Warning|| in 'getBackgroundParallaxParam()' of '{}', this background is not on paralax mode. Try to change it. ", m_title));
			return {0,0,-1,-1};
		}

		Parallax_Param& param = bg.m_param;

		/* début des traitements */
		switch(field)
		{
			default :
				success = false;
				gs::lastError(std::format("||Warning|| in 'getBackgroundParallaxParam()' of '{}', no parameter field correspond with given field. ", m_title));
				break;

			case APPEARANCE_MODE :
				return_val.x = (int)param.mode;
				break;

			case CUSTOM_DEST_RECT_X :
				return_val.x = param.custom_dest_rect.x;
				break;

			case CUSTOM_DEST_RECT_Y :
				return_val.x = param.custom_dest_rect.y;
				break;

			case CUSTOM_DEST_RECT_W :
				return_val.x = param.custom_dest_rect.w;
				break;

			case CUSTOM_DEST_RECT_H :
				return_val.x = param.custom_dest_rect.h;
				break;

			case CURRENT_DEST_RECT_X :
				return_val.x = param.current_dest_rect.x;
				break;

			case CURRENT_DEST_RECT_Y :
				return_val.x = param.current_dest_rect.y;
				break;

			case CURRENT_DEST_RECT_W :
				return_val.x = param.current_dest_rect.w;
				break;

			case CURRENT_DEST_RECT_H :
				return_val.x = param.current_dest_rect.h;
				break;

			case BEGIN_POS_X :
				return_val.x = param.begin_pos.x;
				break;

			case BEGIN_POS_Y :
				return_val.x = param.begin_pos.y;
				break;

			case CURRENT_POS_X :
				return_val.x = param.current_pos.x;
				break;

			case CURRENT_POS_Y :
				return_val.x = param.current_pos.y;
				break;

			case CUSTOM_DEST_RECT :
				return_val = param.custom_dest_rect;
				break;

			case CURRENT_DEST_RECT :
				return_val = param.current_dest_rect;
				break;

			case BEGIN_POS_XY :
				return_val.x = param.begin_pos.x;
				return_val.y = param.begin_pos.y;
				break;

			case CURRENT_POS_XY :
				return_val.x = param.current_pos.x;
				return_val.y = param.current_pos.y;
				break;

			case CUSTOM_DEST_RECT_WH :
				return_val.x = param.custom_dest_rect.w ;
				return_val.y = param.custom_dest_rect.h ;
				break;

			case CURRENT_DEST_RECT_WH :
				return_val.x = param.current_dest_rect.w;
				return_val.y = param.current_dest_rect.h;
				break;


		}//fin du switch

	}

	if(not success)
		return {0.f,0.f,-1.f,-1.f};

	return return_val;
}








bool Map::getBackgroundMoveWhenDraw(short stack_index, short background_index)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("getBackgroundMoveWhenDraw()", stack_index, background_index);

	if(not given_bg_s)
		return false;

	Background* bg_ptr = given_bg_s->front();
	return bg_ptr->m_move_when_draw;
}



Vec2f Map::getBackgroundSpeed(short stack_index, short background_index)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("getBackgroundSpeed()", stack_index, background_index);

	if(not given_bg_s)
		return Vec2f(0.f, 0.f);

	Background* bg_ptr = given_bg_s->front();
	return bg_ptr->m_speed;
}





Vec2f Map::getBackgroundSensibility(short stack_index, short background_index)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("getBackgroundSensibility()", stack_index, background_index);

	if(not given_bg_s)
		return Vec2f(INT_ERR_VAL, INT_ERR_VAL);

	Background* bg_ptr = given_bg_s->front();
	return bg_ptr->m_sensibility;
}






Vec2b Map::getBackgroundScrollingActiveState(short stack_index, short background_index)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("getBackgroundScrollingActiveState()", stack_index, background_index);

	if(not given_bg_s)
		return Vec2b(false,false);

	Background* bg_ptr = given_bg_s->front();
	return bg_ptr->m_scrolling_allowed;
}





Texture* Map::getBackgroundImage(short stack_index, short background_index)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("getBackgroundImage()", stack_index, background_index);

	if(not given_bg_s)
		return nullptr;

	Background* bg_ptr = given_bg_s->front();
	return bg_ptr->m_image;
}




FRect Map::getBackgroundSrc(short stack_index, short background_index)
{
	std::vector<Background*>*
	given_bg_s = INTERNAL_USE_getTargetBackgrounds("getBackgroundSrc()", stack_index, background_index);

	if(not given_bg_s)
		return {-1,-1,-1,-1};

	Background* bg_ptr = given_bg_s->front();
	return bg_ptr->m_src;
}




// ****************************Boundless_Map******************************** //

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

// ****************************Boundless_Map******************************** //



bool Boundless_Map::init(Renderer* ren, gs::Vec2i dest_bloc, std::string title, short idle_value)
{
	m_ren = ren;
	m_is_init = m_chunks.init(m_ren, dest_bloc, title, idle_value);

	if(not m_is_init)
		gs::lastError(std::string("from 'Boundless_Map::init()' : ")+gs::lastError());

	return m_is_init;
}

Vec2i Boundless_Map::getChunkLayerIndex(Vec2i chunk_pos)
{

	int is_present = 0;
	int index = 0;

	if(m_chunks_register.contains(chunk_pos.x))
		if(m_chunks_register.at(chunk_pos.x).contains(chunk_pos.y))
		{
			index = m_chunks_register.at(chunk_pos.x).at(chunk_pos.y);
			is_present = 1;
		}

	//Gestion de la requête d'un chunk.
	if(is_present == 0)
	{
		//Vérification de la non préalable présence de cette requête.
		bool already_in = false;
		for(const Vec2i& rqst : m_required_chunks)
			if(chunk_pos == rqst)
			{
				already_in = true;
				break;
			}

		if(not already_in)
			m_required_chunks.push_back(chunk_pos);
	}

	return Vec2i(is_present, index);
}

bool Boundless_Map::setTileset(int tileset_id, const std::string& path, const Tileset_Loading_Args& tl_args)
{
	if(not m_chunks.setTileset(tileset_id, path, tl_args))
	{
		gs::lastError(std::string("from 'Boundless_Map::setTileset()' : ")+gs::lastError());
		return false;
	}

	return true;
}


bool Boundless_Map::setChunkRequester(std::vector<std::vector<int>> (*chunkRequester)(const Vec2i& begin_tile__grid_pos, const Vec2i& last_tile__grid_pos, bool& is_ok))
{
	mm_chunkRequester = chunkRequester;
	return chunkRequester!=nullptr;
}


Vec2f Boundless_Map::setCameraPos(const Vec2f& pos)
{
	Vec2f prev_pos = m_cam_global_pos;
	m_cam_global_pos = pos;

	return prev_pos;
}

Vec2f Boundless_Map::incrCameraPos(const Vec2f& vals)
{
	m_cam_global_pos += vals;
	return m_cam_global_pos;
}

bool Boundless_Map::setChunkSize(const Vec2i& size)
{
	if(size.x <= 0 or size.y <= 0)
	{
		gs::lastError(std::string("||Warnig|| in 'Boundless_Map::setChunkSize()' of \"")+m_chunks.getTitle()+std::string("\", 'size.x' and 'size.y' must be > 0."));
		return false;
	}

	m_chunk_size = size;
	return true;
}

bool Boundless_Map::setChunk(const Vec2i& chunk_pos, const std::vector<std::vector<int>>& tiles_grid)
{
	bool success = true;

	//Plusieurs cas de figure : le chunk a déjà été chargé, ou, le chunk n'a pas encore été chargé.

	bool already_loaded = false;
	if(m_chunks_register.contains(chunk_pos.x))
		already_loaded = m_chunks_register.at(chunk_pos.x).contains(chunk_pos.y);

	if(already_loaded)
	{
		//Vérification de la conformité des dimensions de la grille reçue.
		success = false;
		if(tiles_grid.size() >= m_chunk_size.y)
		{
			success = true;
			for(const std::vector<int>& ln : tiles_grid)
				if(ln.size() < m_chunk_size.x)
				{
					success = false;
					break;
				}
		}

		if(not success)
		{
			gs::lastError(std::string("||Warnig|| in 'Boundless_Map::setChunk()' of \"")+m_chunks.getTitle()+std::string("\", invalide size of the given 'tiles_grid'. It must be >= chunk size."));
			return false;
		}


		//Remplacements des précédentes tuiles par les nouvelles.
		int corresponding_lay_idx = m_chunks_register[chunk_pos.x][chunk_pos.y];
		char DEFAULT_VALUE = 0;
		for(int x=0; x<m_chunk_size.x; ++x)
		for(int y=0; y<m_chunk_size.y; ++y)
		{
			int val = tiles_grid[y][x]; //Note : pour 'm_chunks_register' (qui est un 'std::unordered_map<int, std::unordered_map<int, int>>') c'est bien [x] puis [y], c'est le choix d'organisation que j'ai fait. Cependant avec 'tiles_grid' (qui est un 'std::vector<std::vector<int>>' soit un tableau de tableaux) c'est bien [y] puis [x], en effet la manière choisie de représenter une grille est : ligne par ligne. E.g: [[1,2,3], [4,5,6], [7,8,9] etc]. De ce fait il faut d'abord localiser la ligne (soit la coo y), puis la colonne de cette ligne (soit la coo x), d'où l'ordre [y][x].
			bool active_state = true;

			if(val < 0)
			{
				val = DEFAULT_VALUE;
				active_state = false;
			}

			success = success & m_chunks.setTileValueAndActiveStateAt(x, y, corresponding_lay_idx, val, active_state);
		}
	}
	else
	{
		success = m_chunks.addLayer(m_chunk_size.to<unsigned int>(), tiles_grid);
		if(success)
			m_chunks_register[chunk_pos.x][chunk_pos.y] = m_chunks.getNumberOfLayers()-1;
	}

	if(not success)
		gs::lastError(gs::str("from 'Boundless_Map::setChunk()' of \"")+m_chunks.getTitle()+gs::str("\" : ")+gs::lastError());


	return success;
}



Vec2i Boundless_Map::getChunkSize() const
{
	return m_chunk_size;
}

Vec2f Boundless_Map::getCameraPos() const
{
	return m_cam_global_pos;
}


int Boundless_Map::getCurrentLoadedChunksCount() const
{
	return m_chunks.getNumberOfLayers();
}




void Boundless_Map::update()
{
	Vec2f dest_bloc = m_chunks.getDestBLOC().to<float>();
	float chunk_total_pixel_x = dest_bloc.x*float(m_chunk_size.x);
	float chunk_total_pixel_y = dest_bloc.y*float(m_chunk_size.y);

	m_curr_chunk.x = std::floor(m_cam_global_pos.x/chunk_total_pixel_x);
	m_curr_chunk.y = std::floor(m_cam_global_pos.y/chunk_total_pixel_y);

	//Ici va être fait la gestion des requêtes de chunks.

	if(m_required_chunks.empty() or not mm_chunkRequester)
		return;

	short rqst_nb = m_required_chunks.size();
	char max_rqst = 8;
	int loop = rqst_nb <= max_rqst ? rqst_nb : max_rqst; //Limitation à 'max_rqst' requête-s par appel de 'Boundless_Map::update()'.

	for(int i=0; i<rqst_nb; ++i)
	{
		Vec2i& chunk_pos = m_required_chunks.at(i);

		Vec2i from = chunk_pos * m_chunk_size.to<int>(); //{chunk_pos.x * int(m_chunk_size.x), chunk_pos.y * int(m_chunk_size.y)};
		Vec2i to = (chunk_pos+1)*m_chunk_size.to<int>() -1; //{(chunk_pos.x+1) * int(m_chunk_size.x) -1, (chunk_pos.y+1) * int(m_chunk_size.y) -1};

		bool is_ok = false;
		std::vector<std::vector<int>> tiles_grid = mm_chunkRequester(from, to, is_ok);

		if(not is_ok)
			continue; //On laisse ça.

		//
		is_ok = m_chunks.addLayer(m_chunk_size.to<unsigned int>(), tiles_grid);
		if(is_ok)
		{
			m_chunks_register[chunk_pos.x][chunk_pos.y] = m_chunks.getNumberOfLayers()-1;
		}

	}

	m_required_chunks.clear(); //Afin de ne traiter que les requêtes à jour. Tant pis pour les requêtes qui n'ont pas eu de réponse favorable.
}


bool Boundless_Map::drawLayer()
{
	if(not m_is_init or not isRenValid())
	{
		gs::lastError(std::string("||Warnig|| in 'Boundless_Map::drawLayer()' of \"")+m_chunks.getTitle()+std::string("\", not yet init. [Use 'Boundless_Map::init()' before.]"));
		return false;
	}

	Vec2f dest_bloc = m_chunks.getDestBLOC().to<float>();

	int ren_w = m_getRenW(); //En pixel.
	int ren_h = m_getRenH();
	int layer_w =  m_chunk_size.x*dest_bloc.x; //En pixel.
	int layer_h =  m_chunk_size.y*dest_bloc.y;

	//Note : tous les chunks sont sensés avoir les mêmes dimensions.
	int max_start_x = (layer_w > ren_w ? layer_w - ren_w : 0);
	int max_start_y = (layer_h > ren_h ? layer_h - ren_h : 0);

	//Position de la "caméra" dans le chunk courant.
	Vec2f cam_local_pos = m_cam_global_pos;

	if(m_cam_global_pos.x < 0)
		cam_local_pos.x =  std::abs(float(dest_bloc.x*m_chunk_size.x*m_curr_chunk.x)) - std::abs(m_cam_global_pos.x);
	else
	if(m_cam_global_pos.x >= float(dest_bloc.x*m_chunk_size.x))
		cam_local_pos.x = m_cam_global_pos.x - float(dest_bloc.x*m_chunk_size.x*m_curr_chunk.x);

	if(m_cam_global_pos.y < 0)
		cam_local_pos.y =  std::abs(float(dest_bloc.y*m_chunk_size.y*m_curr_chunk.y)) - std::abs(m_cam_global_pos.y);
	else
	if(m_cam_global_pos.y >= float(dest_bloc.y*m_chunk_size.y))
		cam_local_pos.y = m_cam_global_pos.y - float(dest_bloc.y*m_chunk_size.y*m_curr_chunk.y);


	//Traitements de l'offset : si le start_x_y vient à son max, alors on actionne l'offset en plus.
	Vec2f offset;

	cam_local_pos.x -= ren_w/2; //Afin que la position de la "caméra" s'affiche au centre du rendu (non pas au coin supérieur gauche du rendu).
	cam_local_pos.y -= ren_h/2; //Et ça marche x).

	Vec2i res = getChunkLayerIndex(m_curr_chunk);
	bool have_center_chunk = res.x;
	int c = res.y; //'center'. En index.

	if(cam_local_pos.x > max_start_x)
	{
		if(have_center_chunk)
			m_chunks.setWorldStartX(c, max_start_x);
		offset.x = max_start_x - cam_local_pos.x; //Note : startxy "déplace" la map de manière opposée à ce que fait offset.
		//Note : startxy déplace la "caméra" (donc "virtuellement"), alors que offset déplace le rendu à l'écran.
	}
	else
	if(cam_local_pos.x < 0)
	{
		if(have_center_chunk)
			m_chunks.setWorldStartX(c, 0);
		offset.x = -cam_local_pos.x;
	}
	else
	{
		if(have_center_chunk)
			m_chunks.setWorldStartX(c, cam_local_pos.x);
		offset.x = 0;
	}


	if(cam_local_pos.y > max_start_y)
	{
		if(have_center_chunk)
			m_chunks.setWorldStartY(c, max_start_y);
		offset.y = max_start_y - cam_local_pos.y;
	}
	else
	if(cam_local_pos.y < 0)
	{
		if(have_center_chunk)
			m_chunks.setWorldStartY(c, 0);
		offset.y = -cam_local_pos.y;
	}
	else
	{
		if(have_center_chunk)
			m_chunks.setWorldStartY(c, cam_local_pos.y);
		offset.y = 0;
	}



	if(have_center_chunk)
		m_chunks.drawLayer(c, offset);

	int c_start_x = have_center_chunk ? m_chunks.getWorldStart_i(c).x : 0; //'center start x'.
	int c_start_y = have_center_chunk ? m_chunks.getWorldStart_i(c).y : 0; //'center start x'.

	int on_x=0;
	int on_y=0;

	/////////////////////////////////////////
	res = getChunkLayerIndex(m_curr_chunk+Vec2i(0,-1));
	if(res.x)
	{
		int t=res.y; //'top'.
		on_y = (layer_h < ren_h ? layer_h : ren_h);
		m_chunks.setWorldStartX(t, c_start_x);
		m_chunks.setWorldStartY(t, max_start_y);

		m_chunks.drawLayer(t, offset+Vec2f(0.f, -on_y));
	}

	//****************************************
	res = getChunkLayerIndex(m_curr_chunk+Vec2i(1,-1));
	if(res.x)
	{
		int tr=res.y; //'top right'.
		on_y = (layer_h < ren_h ? layer_h : ren_h);
		on_x = (layer_w < ren_w ? layer_w : ren_w);
		m_chunks.setWorldStartX(tr, 0);
		m_chunks.setWorldStartY(tr, max_start_y);

		m_chunks.drawLayer(tr, offset+Vec2f(+on_x, -on_y));
	}

	//////////////////////////////////////////////////////
	res = getChunkLayerIndex(m_curr_chunk+Vec2i(1,0));
	if(res.x)
	{
		int r=res.y; //'right'.
		on_x = (layer_w < ren_w ? layer_w : ren_w);
		m_chunks.setWorldStartX(r, 0);
		m_chunks.setWorldStartY(r, c_start_y);

		m_chunks.drawLayer(r, offset+Vec2f(on_x, 0));
	}

	//*************************************************
	res = getChunkLayerIndex(m_curr_chunk+Vec2i(1,1));
	if(res.x)
	{
		int br=res.y; //'bottom right'.
		on_x = (layer_w < ren_w ? layer_w : ren_w);
		on_y = (layer_h < ren_h ? layer_h : ren_h);
		m_chunks.setWorldStartX(br, 0);
		m_chunks.setWorldStartY(br, 0);

		m_chunks.drawLayer(br, offset+Vec2f(on_x, on_y));
	}

	///////////////////////////////////////////////////
	res = getChunkLayerIndex(m_curr_chunk+Vec2i(0,1));
	if(res.x)
	{
		int b=res.y; //'bottom'.
		on_y = (layer_h < ren_h ? layer_h : ren_h);
		m_chunks.setWorldStartX(b, c_start_x);
		m_chunks.setWorldStartY(b, 0);

		m_chunks.drawLayer(b, offset+Vec2f(0, on_y));
	}

	//***********************************************
	res = getChunkLayerIndex(m_curr_chunk+Vec2i(-1,1));
	if(res.x)
	{
		int bl=res.y; //'bottom left'.
		on_x = (layer_w < ren_w ? layer_w : ren_w);
		on_y = (layer_h < ren_h ? layer_h : ren_h);
		m_chunks.setWorldStartX(bl, max_start_x);
		m_chunks.setWorldStartY(bl, 0);

		m_chunks.drawLayer(bl, offset+Vec2f(-on_x, on_y));
	}

	////////////////////////////////////////////////
	res = getChunkLayerIndex(m_curr_chunk+Vec2i(-1,0));
	if(res.x)
	{
		int l=res.y; //'left'.
		on_x = (layer_w < ren_w ? layer_w : ren_w);
		m_chunks.setWorldStartX(l, max_start_x);
		m_chunks.setWorldStartY(l, c_start_y);

		m_chunks.drawLayer(l, offset+Vec2f(-on_x, 0));
	}

	//******************************************
	res = getChunkLayerIndex(m_curr_chunk+Vec2i(-1,-1));
	if(res.x)
	{
		int tl=res.y; //'top left'.
		on_x = (layer_w < ren_w ? layer_w : ren_w);
		on_y = (layer_h < ren_h ? layer_h : ren_h);
		m_chunks.setWorldStartX(tl, max_start_x);
		m_chunks.setWorldStartY(tl, max_start_y);

		m_chunks.drawLayer(tl, offset+Vec2f(-on_x, -on_y));
	}

	return true;
}



int Boundless_Map::m_getRenW() const
{
	int w=-1;
	getCurrentRenderOutputSize(m_ren, &w, nullptr);
	return w;
}

int Boundless_Map::m_getRenH() const
{
	int h=-1;
	getCurrentRenderOutputSize(m_ren, nullptr, &h);
	return h;
}














void Camera2D::updateMapView(Map& map, int base_layer_idx)
{
	//Note: Ceci est une simulation de caméra dans un monde virtuel 2D.
	//Les coordonnées mondes sont celles dont le zoom a un impact, contrairement aux coordonnées écrans.
	//On peut considérer 'map' comme étant le "hardware", qui en jouant sur ses propriétés, permet d'avoir
	//Le rendu voulu par l'abstraction 'Camera2D'.

	//Note: les modifications apportées à 'map' visent tous les layers, cependant, c'est seulement sur l'évolution du layer indiqué par 'base_layer_id' que se font les ajustements ( -> tous les autres layers sont dirigés en fonction de ce layer).

	const float zoom = getZoom();
	map.setZoom(zoom);

	gs::Vec2f w_total = m_world_pos+m_world_offset; //Note: 'w_' pour 'world'

	if(w_total.x >= 0.f)
	{
		map.setWorldStartX(w_total.x);
		map.setScreenOffsetX(0.f); //Important, pour éviter les incohérences d'affichage lorsqu'il y a des "téléportations".

		float wx = map.getWorldStart(base_layer_idx).x; //Pour pouvoir suivre/garder la correspondance avec les bornes max globales du layer de référence.
		if(wx < w_total.x) // -> si la map n'est pas allée aussi loin que demandé, par ses limitations internes.
		{
			if(not allow_out_right)
			{
				m_world_pos.x=wx;
				m_world_offset.x=0.f;
			}
			else
				map.setScreenOffsetX(-(w_total.x-wx)*zoom); //Puisqu'on veut décaler l'affichage de la map vers la gauche, il faut mentionner une valeur négative, or 'w_total.x-wx' est ici positive, d'où le signe '-'.
		}
	}
	else
	{
		if(not allow_out_left)
		{
			m_world_pos.x = 0.f; //Pour limiter la caméra à ne pas sortir à gauche de la map.
			m_world_offset.x = 0.f;
			w_total.x = 0.f;
		}

		map.setWorldStartX(0.f);
		map.setScreenOffsetX(-w_total.x*zoom); //Puisqu'on veut décaler l'affichage de la map vers la droite, il faut mentionner une valeur positive, et comme 'w_total.x' arrive négative dans ce bloc, on y rajoute un signe '-'.
	}

	//_________________________________________________________________//
	//_________________________________________________________________//

	if(w_total.y >= 0.f)
	{
		map.setWorldStartY(w_total.y);
		map.setScreenOffsetY(0.f); //Important, pour éviter les incohérences d'affichage lorsqu'il y a des "téléportations".

		float wy = map.getWorldStart(base_layer_idx).y; //Pour pouvoir suivre/garder la correspondance avec les bornes max globales du layer de référence.
		if(wy < w_total.y)
		{
			if(not allow_out_down)
			{
				m_world_pos.y=wy;
				m_world_offset.y=0.f; // = 0.f -shaking.y (on annule aussi le shaking)
			}
			else
				map.setScreenOffsetY(-(w_total.y-wy)*zoom); //Puisqu'on veut décaler l'affichage de la map vers le haut, il faut mentionner une valeur négative, or 'w_total.y-wy' est ici positive, d'où le signe '-'.
		}
	}
	else
	{
		if(not allow_out_up)
		{
			m_world_pos.y = 0.f; //Pour limiter la caméra à ne pas sortir en haut de la map.
			m_world_offset.y = 0.f;
			w_total.y = 0.f;
		}

		map.setWorldStartY(0.f);
		map.setScreenOffsetY(-w_total.y*zoom); //Puisqu'on veut décaler l'affichage de la map vers le bas, il faut mentionner une valeur positive, et comme 'w_total.y' arrive négative dans ce bloc, on y rajoute un signe '-'.
	}






	//Au tour du décalage écran ------------

	gs::Vec2f shaking = getCurrScreenShaking();
	gs::Vec2f screen_total = (m_screen_offset+shaking)*getScreenValModifier() /zoom; //Décalage écran à prendre en compte, ramené au coordonnées monde

	gs::Vec2f cam_w_pos = map.getWorldStart(base_layer_idx) - map.getScreenOffset()/zoom; // "camera world position" -> Coordonnées mondes de la caméra (coin supérieur gauche)
	gs::Vec2f new_cam_w_pos = cam_w_pos - screen_total; //Calcule de la nouvelle position de la caméra dans le monde, dû au décalage écran. Soustraction car la caméra bouge dans le sens opposé à celui du décalage (par exemple : décaler la map vers la droite, à l'écran <=> décaler la caméra vers la gauche "dans le monde").

	auto processScreenOffset_1D = [zoom](
		float cam_w_pos,
		float new_cam_w_pos,
		float shaking,
		float screen_val_modifier,
		bool allow_out_min, //left pour les X, up pour les Y
		bool allow_out_max, //right pour les X, down pour les Y
		std::function<void(float val)> map_setScreenOffset,
		std::function<void(float val)> map_incrScreenOffset,
		std::function<void(float val)> map_setWorldStart,
		std::function<float()> map_getScreenOffset,
		std::function<float()> map_getWorldStart,
		float& m_screen_offset
	)
	{
		//Note: les commentaires ci-dessous sont pour le cas des X - puisque les deux axes suivent les mêmes règles, j'ai décidé a posteriori de généraliser en une seule fonction, appliquée aux deux, notamment pour éviter de dupliquer un bloc de code assez conséquant.

		if(cam_w_pos < 0.f) //S'il y a initialement une sortie de l'origine de la caméra vers la gauche de la map, notamment quand 'allow_out_left' est à 'true'.
		{
			if(new_cam_w_pos < 0.f) //Si la nouvelle position reste dans les négatives (aka à l'extérieur gauche de la map)
				map_setScreenOffset(-new_cam_w_pos*zoom); //Note: 'new_cam_w_pos.x' est en coordonnées mondes, '*zoom' permet de le faire passer en coordonnées écrans (afin d'être correst avec '.setScreenOffsetX()'), et le '-' car les postions, de la caméra (-> mondes) et de la map (-> "matériel"/pixel/écran/affichage), vont dans des sens opposés.

			else //Si la caméra n'est plus à l'extérieur gauche
			{
				map_setScreenOffset(0.f);
				map_setWorldStart(new_cam_w_pos); //Note: 'new_cam_w_pos.x' est déjà en coordonnées mondes, tout à fait ce qui est attendu par '.setWorldStartX()', et ici pas de '-' car on passe en argument des coordonnées mondes dans une méthode qui reçoit des coordonnées mondes (les deux sont de même unité (le "pixelmonde" en quelques sortes) et de même sens).

				float set = map_getWorldStart();
				if(set < new_cam_w_pos) //On vérifie si la map a atteint sa limite droite, et n'a donc pas pu prendre en compte 'new_cam_w_pos.x' complètement.
				{
					if(allow_out_max)
						map_setScreenOffset(-(new_cam_w_pos-set)*zoom);
					else
						m_screen_offset = (cam_w_pos - set -shaking)*zoom /screen_val_modifier;
				}
			}
		}
		else //Pas de sortie à gauche initialement
		{
			if(new_cam_w_pos < cam_w_pos) //Si la caméra se déplace vers la gauche
			{
				if(new_cam_w_pos >= 0.f) //Si la caméra reste dans les positives
				{
					float map_scr_off = map_getScreenOffset()/zoom;
					if(map_scr_off < 0.f) //S'il y avait déjà sortie à l'extérieur droit (du côté droit de la caméra)
					{
						float gap = cam_w_pos - new_cam_w_pos;

						if(map_scr_off + gap < 0.f) //Si ça laisse la caméra à l'extérieur droit
							map_incrScreenOffset(gap*zoom);

						else //Si la caméra n'est plus à l'extérieur droit
						{
							map_setScreenOffset(0.f);
							map_setWorldStart(new_cam_w_pos);
						}
					}
					else //S'il n'y avait pas déjà sortie à l'extérieur droit (du côté droit de la caméra)
						map_setWorldStart(new_cam_w_pos);
				}
				else //Si la caméra passe dans les négatives (à l'extérieur gauche)
				{
					map_setWorldStart(0.f);

					if(allow_out_min)
						map_setScreenOffset(-new_cam_w_pos*zoom);
					else
						m_screen_offset = (cam_w_pos -shaking)*zoom /screen_val_modifier;
				}
			}
			else
			if(new_cam_w_pos > cam_w_pos) //Si la caméra se déplace vers la droite
			{
				map_setWorldStart(new_cam_w_pos); //Deux possibilités : soit ça passe, soit ça se limite au bord.

				float set = map_getWorldStart();
				if(set < new_cam_w_pos) //On vérifie si la map a atteint sa limite droite, et n'a donc pas pu prendre en compte 'new_cam_w_pos.x' complètement.
				{
					if(allow_out_max)
						map_setScreenOffset(-(new_cam_w_pos-set)*zoom);
					else
						m_screen_offset = (cam_w_pos - set -shaking)*zoom /screen_val_modifier;
				}

			}
		}
	};

	processScreenOffset_1D(cam_w_pos.x, new_cam_w_pos.x, shaking.x, getScreenValModifier().x, allow_out_left, allow_out_right,
		[&map](float v){map.setScreenOffsetX(v);}, [&map](float v){map.incrScreenOffsetX(v);},
		[&map](float v){map.setWorldStartX(v);}, [&map](){return map.getScreenOffset().x;},
		[&map, base_layer_idx](){return map.getWorldStart(base_layer_idx).x;}, m_screen_offset.x
	);

	processScreenOffset_1D(cam_w_pos.y, new_cam_w_pos.y, shaking.y, getScreenValModifier().y, allow_out_up, allow_out_down,
		[&map](float v){map.setScreenOffsetY(v);}, [&map](float v){map.incrScreenOffsetY(v);},
		[&map](float v){map.setWorldStartY(v);}, [&map](){return map.getScreenOffset().y;},
		[&map, base_layer_idx](){return map.getWorldStart(base_layer_idx).y;}, m_screen_offset.y
	);
}










}; //Fin du namespace gs.
