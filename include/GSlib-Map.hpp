#ifndef _GSlib_MAP_HPP_
#define _GSlib_MAP_HPP_
//Version modifiée le 16/07/26 (à moins d'oubli de modifier cette ligne ou autre).

/*
Mises à jour (tout n'est pas mentionné) :
22/07/23: 	dissociation désormais possible du w et h dans l'utilisation du destBLOC et obligation pour srcBLOC.
23/07/23:	ajout de la possibilité de manipuler le rectangle source des backgrounds, ajout de la récupération des dimensions du tileset.
26/07/23: 	ajout de 'setTileValueAndActiveStateAt()', 'setSensibilityXY()'
27/07/23:	mise à jour des fonctions 'hitCollisionTile()' 'willHitCollisionTile()'. Ajout de 'replaceTilesValue()'
02/08/23:	ajout de 'getLastTileX/Y()'
07/08/23:	Ajout de 'getLayerW/H()', des alias pour 'getLimitX/Y()'.
22/01/25:	Ajout de 'linkBodyToLayer()' des modifications qui vont avec.
17/02/25:	Ajout de 'getTitle()'.
15/03/25:	Ajout de 'Collision_Mask'.
14/03/26:	Retrait de la dépendence à gs::Game.
23/04/26:	Changements de noms:  'getLimitX/Y' -> 'getTilesCountX/Y' (et retrait de 'getLayerW/H()'), 'm_limit_x/y' -> 'm_tiles_count',
				'...BeginX/Y...' -> '...ScreenOffsetX/Y...', 'm_beginX/Y' -> 'm_screen_offset'.
			Remise de 'hitCollisionTile()'.
			Rajout du mécanisme de sous-zone dans les layers (collisions et affichage).
24/04/26:	Rajout de la possibilité d'utiliser plusieurs tilesets à la fois.
24/05/26:	Rajout de 'Map::getLayer()'.
16/07/26:	Rajout des tuiles animées.

*/

	#include "GSlib.hpp"
	#include <unordered_map>





namespace gs /*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/
{

class Map;
struct Camera2D : public Camera2D_Base
{
	void updateMapView(Map& map, int base_layer_idx);
};


struct Vec_Int_VecSInt
{
	int m_id=0;
	std::vector<short> m_values;
};

struct Size2i : public Vec2i
{
	int& w; int& h; Size2i() : w(x), h(y) {};
};

struct Size2f : public Vec2f
{
	float& w; float& h; Size2f() : w(x), h(y) {};
};

struct Tile
{
	short m_value=0;
	bool m_active=true;
	Tile() = default;
	~Tile() {}
};



struct Tile_Anim //Décrit les animations d'une tuile
{
	//Les paramètres (ce que l'utilisateur doit mentionner)
	short tile_val=0; //Numéro de la tuile concernée
	std::vector<short> frames; //Contient les valeurs des tuiles qui composent l'animation de le tuile indiquée. Note: les tuiles affichées ne dépendent plus de la tuile originale (aka 'tile_val') mais uniquement du contenu de 'frames', SAUF en cas d'erreur. Bien sûr, rien n'empêche de mentionner la tuile originale dans 'frames'.
	float one_frame_duration=0.8f; //Durée d'une frame de l'animation
	float new_cycle_delay=0.f; //Durée à attendre avant de boucler l'animation (à la fin de la dernière frame).

	//L'usage
	float frame_timer=0.f; //Timer à rebours (sert aussi pour le 'new_cycle_delay')
	short curr_frame=0; //indice dans 'frames'
	void initBegin();
	short getCurrFrameTileVal(bool& is_ok) const;
	void update(float dt);
};

#define GS_TILESET_ATLAS_MAX_SRC_PADDING 4
struct Tileset_Loading_Args
{
	Vec2i tile_src_size={32,32}; //En pixel - taille réelle d'une tuile dans l'atlas -- Important : doit ça rester en type entier (et non floatant) pour assurer les divisions euclidiennes requises.
	int src_padding_to_add=1; //En pixel - autour d'une tuile, dans l'atlas créé
	bool duplicate_borders=true; //Pour remplir ou non l'espace du padding avec les bords des tuiles.
};
	struct Tileset_Loading_Out_Info
	{
		int nb_of_columns=0;
		int used_padding=0;
	};


class Tileset
{
	friend Map;

	public:
		//~Tileset(); !!! Ne pas destroy "'m_atlas'" avec 'Tileset' - à laisser être gérée par 'Map', ou par l'utilisateur (s'il utilise 'Tileset' de son côté). En effet, dans mon implémentation de 'Map', il y a apparition de buffer de 'Tileset', ainsi détruire la texture à la destruction de 'Tileset' ferait que le 'Tileset' temporaire détruirait la texture fraichement préparée.
		void destroyAtlasTexture(); //Pour une destruction manuelle de la texture

		bool load(Renderer* ren, const std::string& pth, const Tileset_Loading_Args& tl_args);
		bool load(Renderer* ren, Surface* src_img, const Tileset_Loading_Args& tl_args);
	  	static Texture* genAtlas(Renderer* ren, const std::string& pth, const Tileset_Loading_Args& tl_args, Tileset_Loading_Out_Info* out=nullptr);
	  	static Texture* genAtlas(Renderer* ren, Surface* src_img, const Tileset_Loading_Args& tl_args, Tileset_Loading_Out_Info* out=nullptr);
		short getCurrentFrameOf(short tile_val) const; // type de retour : une valeur de tuile
		Vec2i getTileSrcSize() const {return m_tile_src_size;}
		Vec2i getTileSrcPos(short tile_val) const;
		short getNumberOfColumn() const {return m_number_of_columns;}
		int getTileSrcPadding() const {return m_tile_src_padding;}
		Texture* getAtlas() const {return m_atlas;}

	private:
		void load__finalStep(Texture* new_atlas, const Tileset_Loading_Args& tl_args, const Tileset_Loading_Out_Info& info);

	protected:

		Texture* m_atlas=nullptr;
		short m_number_of_columns=0; //Nombre de colonnes dans l'atlas - est déduit
		Vec2i m_tile_src_size{1,1}; //En pixel - taille réelle d'une tuile dans l'atlas - n'est pas destiné à changer une fois l'atlas chargé
		int m_tile_src_padding=1; //En pixel - padding (uniforme) extérieur d'une tuile, dans l'atlas. Exemple: si le padding vaut 1, alors les tuiles occupent un espace "taille+{2,2}", "taille" étant les dimensions réelles de la tuile, et {2,2} -> padding à gauche, à droite, et padding en haut, en bas.

		std::unordered_map<short, Tile_Anim> animated_tiles; //Indique le tuiles animées, de ce tileset.
};


class Collision_Mask
{
	public:

		Collision_Mask() = default;
		Collision_Mask(std::vector<std::vector<char>>& grid);
		~Collision_Mask() = default;

		Collision_Mask(const Collision_Mask&)=default;
		Collision_Mask& operator=(const Collision_Mask&)=default;

		Collision_Mask(Collision_Mask&&)=default;
		Collision_Mask& operator=(Collision_Mask&&)=default;

		bool isCollisionTile(int x, int y) const; //Les coo invalides seront comptées comme non collision.
		Vec2i getTilesCount() const;
		Vec2f getTilesCount_f() const {return getTilesCount().to<float>();}
		const std::vector<std::vector<char>>& getGrid() const {return m_grid;}

	protected:
		std::vector<std::vector<char>> m_grid; //0 -> tuile de non collision, autre -> tuile de collision.
};

namespace ma
{

	enum Background_Mode{FILL, PARALLAX};

	enum Parallax_Dest_Appearance{FILL_SCREEN, ORIGINAL_SIZE, CUSTOM_SIZE, FILL_SCREEN_IN_W, FILL_SCREEN_IN_H};



/*

FILL_SCREEN_IN_W et FILL_SCREEN_IN_H
agrandissent ou réduisent l'image en gardant les mêmes proportions.
FILL_SCREEN_IN_W redimensionne de manière à remplir toute la largeur du rendu.
FILL_SCREEN_IN_H redimensionne de manière à remplir toute la longueur du rendu.

*/

	enum Parallax_Param_Field
	{
		APPEARANCE_MODE,
		CUSTOM_DEST_RECT_X,
		CUSTOM_DEST_RECT_Y,
		CUSTOM_DEST_RECT_W,
		CUSTOM_DEST_RECT_H,
		CURRENT_DEST_RECT_X,
		CURRENT_DEST_RECT_Y,
		CURRENT_DEST_RECT_W,
		CURRENT_DEST_RECT_H,
		BEGIN_POS_X,
		BEGIN_POS_Y,
		CURRENT_POS_X,
		CURRENT_POS_Y,
		CUSTOM_DEST_RECT,
		CURRENT_DEST_RECT,
		BEGIN_POS_XY,
		CURRENT_POS_XY,
		CUSTOM_DEST_RECT_WH,
		CURRENT_DEST_RECT_WH,
	};

};


struct Parallax_Param
{
	ma::Parallax_Dest_Appearance mode=ma::FILL_SCREEN;
	FRect custom_dest_rect={0, 0, 800, 480};
	FRect current_dest_rect={0, 0, 800, 480};
	Point begin_pos={0, 0};
	Point current_pos={0, 0};


	Parallax_Param() = default;
	~Parallax_Param() = default;
};

struct Background
{
	Texture* 		    	m_image=nullptr;
	bool  					m_destroy_ptrs_in_destructor=false;
	ma::Background_Mode 	m_mode=ma::FILL;
	int 					m_w=0, m_h=0;
	bool  					m_is_init=false;
	std::string   			m_1st_image_file_path;
	FRect 				    m_src;

	//pour le mode parallax
	Parallax_Param 			m_param;
	Vec2f 					m_start={0.f,0.f};
	Vec2f					m_sensibility{100.f,100.f};
	Vec2b 					m_scrolling_allowed={true,true};
	bool 					m_move_when_draw=true;	//Active ou désactive l'incrémention de la position lors de l'affichage. Si désactivé, le mouvement doit être fait manuellement par l'utilisateur (si voulu).
	Vec2f 					m_speed={0.f,0.f};	//Pour incrémenter/décrémenter m_start.xy.



	Background() = default;
	~Background()
	{
		if(m_destroy_ptrs_in_destructor)
		{
			if(m_image) destroyTexture(m_image);
		}
		else
			::gs::addToClearAtTheEnd(m_image);
	}
	bool incrStartX(float val);
	bool incrStartY(float val);

};



struct Map_Layer
{
	/*
	Cette structure est le model de base pour les maps ; il est destiné à être gérér par gs::Map.
	*/

	std::vector<std::vector<Tile>> m_map_array;

	Vec2i 			m_tiles_count{0,0}; 	//nombre max de tuiles sur les abscisses et les ordonnés
	Vec2f 			m_world_start{0.f,0.f}; //debut de lecture des pixel virtuels de la map (pixel près, mais pas par rapport à l écran, mais à la map)
	Vec2f 			m_sensibility{100.f,100.f};	//sensibilité d'adaptation aux modifications de startX/Y (e.g: pour 50 % de sensibilité et une incrémentation de 14 au startX, seulement 7 sera ajouté)
	bool 			m_is_init=false;
	bool 			m_is_the_main_layer=false;
	bool 			m_is_visible=true;
	int             m_tileset_id=0; //L'identifiant du tileset rattaché

	//Les variables qui suivent sont du point de vue de la "camera / nous".
	//C-à-d par exemple que left scrolling c'est quand la "camera" se dirige vers la gauche.
	bool m_up_scrolling_limit_reached=false;
	bool m_down_scrolling_limit_reached=false;
	bool m_left_scrolling_limit_reached=false;
	bool m_right_scrolling_limit_reached=false;


	bool initTheMapArray();
	void setWorldStartX(float val, short dest_bloc, unsigned short render_w);
	void setWorldStartY(float val, short dest_bloc, unsigned short render_h);
	void incrWorldStartX(float val, short dest_bloc, unsigned short render_w);
	void incrWorldStartY(float val, short dest_bloc, unsigned short render_h);

	Vec2f getWorldStartXY() {return m_world_start;}
	//removed bool unlinkBodyToLayer(Body& body);

	protected :
		//removed void updateLinkedBodiesPos(bool in_x, bool in_y);
		// void applyIncrStartXYForBodies(float val_x, float val_y); //Pour l'instant ce n'est pas très réussi car ça cause des problèmes sur les collisions.
		// Vec2f m_incr_cumul={0,0}; //Permet de voir le cumul des incrémentations de startX Y, afin de ne rajouter que des valeurs entièreq à chaque fois.
};





class Map
{
	public :

	  //Note: "bloc" == "tile size".

		Map(Renderer* ren, Vec2i dest_bloc, std::string title="No_titled_map", short idle_value=-1);
		Map()=default;
		Map(Vec2i dest_bloc, std::string title="No_titled_map", short idle_value=-1);
		Map(Renderer* ren);
		bool init(Renderer* ren, Vec2i dest_bloc, std::string title="No_titled_map", short idle_value=-1);

		~Map();



	  /*-------Globals--------------------------------------------------------------*/

	  	void update(float dt);

	  	bool setTileset(int tileset_id, const std::string& pth, const Tileset_Loading_Args& tl_args);
		bool setTilesetFromSurface(int tileset_id, Surface* atlas_surf, const Tileset_Loading_Args& tl_args);
		bool setTileset_noOverwrite(int tileset_id, const std::string& pth, const Tileset_Loading_Args& tl_args);
		bool setTilesetFromSurface_noOverwrite(int tileset_id, Surface* atlas_surf, const Tileset_Loading_Args& tl_args);


		//Note: Les animations des tuiles sont rattachées aux tilesets. Un tileset peut avoir des animations différentes d'un autres tileset.
		bool addAnimtedTiles(int tileset_id, const std::vector<Tile_Anim>& animated_tiles, bool overwrite_existing_ones=true);
		//bool removeAnimatedTiles(int tileset_id, const std::vector<short>& animated_tiles);
		//bool setAnimtedTiles(int tileset_id, const std::vector<Tile_Anim>& animated_tiles); //Ecrase tout le contenu pour le remplacer par le nouveau (si le nouveau contenu est vide, ça donne le même résultat que 'removeAllAnimatedTiles()')
		//bool removeAllAnimatedTiles(int tileset_id);

		void setScreenOffset(Vec2f val);
		void setScreenOffsetX(float val);
		void setScreenOffsetY(float val);
		void incrScreenOffsetX(float val);
		void incrScreenOffsetY(float val);

		void setDestBLOC(Vec2i vals);
		void setDestBLOCW(unsigned short val);
		void setDestBLOCH(unsigned short val);

		void setZoom(float val);
		void incrZoom(float val);

		void setRen(Renderer* ren);

		void setSynchronization(bool new_state);
		//
		//
		Vec2f getScreenOffset() const;
		int getScreenOffsetX_i() const;
		int getScreenOffsetY_i() const;

		Vec2i getDestBLOC() const;
		int getDestBLOCW() const;
		int getDestBLOCH() const;

		Vec2i getTileSrcSize(int tileset_id) const;

		float getZoom() const;

		Renderer* getRen();

		const Tileset& getTileset(int tileset_id, bool& is_ok) const;
		bool isTilesetLoaded(int tileset_id) const;
		int getNumberOfTilesetColumn(int tileset_id) const;
		Vec2f getTilesetWH(int tileset_id) const;
		float getTilesetW(int tileset_id) const;
		float getTilesetH(int tileset_id) const;

		bool getSynchronization() const;

		int getNumberOfLayers() const;
		int getLayersCount() const;

		std::string getTitle() const;






	  /*-------Layers--------------------------------------------------------------*/

	  	void setNextLayersTileset(int tileset_id); //Détermine à quel tileset seront rattachés les prochains layers qui vont être chargés (par 'addLayer()').

		bool addLayer(const std::string& file_path, const std::string& layer_str_ID, Vec2f sensibility=Vec2f(100.f,100.f), bool is_the_main_layer=false);
		bool addLayer(const Vec2ui& size, int fill_val=0, int border_val=1, Vec2f sensibility=Vec2f(100.f,100.f), bool is_the_main_layer=false);
		bool addLayer(const Vec2ui& size, const std::vector<std::vector<int>>& tiles_grid, Vec2f sensibility=Vec2f(100.f,100.f), bool is_the_main_layer=false);

		void setLayerTileset(int layer_index, int tileset_id); //Permet de changer le tileset auquel est rattaché un layer.

		bool drawLayer(int layer_index, Vec2f offset={0.f,0.f}, float brightness=1, Rect area={0,0,0,0});
		bool drawLayer(Vec2f offset={0.f,0.f}, float brightness=1.f, Rect area={0,0,0,0});

		void popBackLayer();
		const Map_Layer& getLayer(int layer_index, bool& is_ok) const;
		//
		//
		//
		bool setWorldStartX(int layer_index, float val); //Scrolling de la "caméra du layer".
		bool setWorldStartY(int layer_index, float val);
		bool incrWorldStartX(int layer_index, float val);
		bool incrWorldStartY(int layer_index, float val);

		bool setWorldStartX(float val); //Ces 'set' sans layer_index visent tous les layers.
		bool setWorldStartY(float val);
		bool incrWorldStartX(float val);
		bool incrWorldStartY(float val);

		void setIdleValue(short val, bool restore_previous=true);

		bool setActiveStateOfTilesOf(short tile_val, int layer_index, bool new_state);
		bool setActiveStateOfTilesOf(short tile_val, bool new_state);
		bool setTileActiveStateAt(short x, short y, int layer_index, bool new_state);
		bool setTileValueAt(short x, short y, int layer_index, short new_value);
		bool setTileValueAndActiveStateAt(short x, short y, int layer_index, short new_value, bool new_state);

		bool setSensibilityX(int layer_index, float new_sensibility);
		bool setSensibilityY(int layer_index, float new_sensibility);
		bool setSensibilityXY(int layer_index, Vec2f new_sensibility);

		bool setShowLayer(unsigned int layer_index, bool new_state);
		bool setShowLayers(bool new_state);

		bool setMainLayer(unsigned int layer_index);

		bool replaceTilesValue(int layer_index, short target_tile_value, short new_value);

		Vec2i getWorldStart_i(int layer_index) const;
		Vec2f getWorldStart(int layer_index) const;

		Vec2i getWorldStart_i() const; //Ces 'get' sans layer_index visent le main_layer, si pas de main_layer alors le premier layer.
		Vec2f getWorldStart() const;

		Vec2i getTilesCount(int layer_index=0) const; //Nombre de tuiles sur la largeur et la longueur.
		Vec2f getTilesCount_f() const {return getTilesCount().to<float>();}
		Vec2i getLastTileXY(int layer_index=0) const; //En unité de tuile et non en pixel.

		Vec2f getLayerPixelSize(int layer_index=0) const;

		short getIdleValue() const;

		bool getTileActiveStateAt(int x, int y, int layer_index=0) const;
		short getTileValueAt(int x, int y, int layer_index=0) const;

		Vec2f getSensibility(int layer_index=0);

		bool getShowLayer(int layer_index);
		bool isVisibleLayer(int layer_index);

		bool haveMainLayer() const;
		short getMainLayerIndex() const;

		bool isInvalidLayerIndex(int layer_index) const;






	  /*-------Collisions--------------------------------------------------------------*/

		bool setAlsoAsCollisionLayer(int layer_index, std::vector<short> collision_tiles_value);

		bool isCollisionTile(int x, int y, int layer_index, int* out__tile_val=nullptr);
		bool hitCollisionTile(FRect ent_rect, int layer_index=-1, std::vector<short>* out__tile_vals=nullptr, Rect area={0,0,0,0}); // layer_index = -1 par défaut pour signifier de prendre le premier layer de collision (si présent)
		bool willHitCollisionTile(FRect ent_rect, Vec2f total_pixel_incr, int layer_index=-1, gs::Vec2<float*> remaining_before_hit__out={nullptr,nullptr}, std::vector<short>* out__tile_vals=nullptr, Rect area={0,0,0,0});

		bool addCollisionTilesValues(int layer_index, std::vector<short> values);
		bool removeCollisionTilesValues(int layer_index, std::vector<short> values);

		std::vector<short> getCollisionTilesValues(int layer_index);
		Collision_Mask getCollisionMask(int layer_index);






	  /*-------Backgrounds--------------------------------------------------------------*/

		bool addBackgroundStack(uint16_t count=1);
		bool popBackBackgroundStack();
		bool popBackBackground(short stack_index);

		bool isInvalidBackgroundStackIndex(short stack_index) const;
		bool isInvalidBackgroundIndex(short stack_index, short background_index);

		bool addBackground(short stack_index, const std::string& file_path, ma::Background_Mode mode=ma::FILL, Vec2f sensibility=Vec2f(100.f,100.f), Vec2b scrolling_allowed=Vec2b(true,true));

		bool drawBackground(short stack_index=-1, short background_index=-1);

		bool moveBackground(short stack_index, short background_index, float x_pix, float y_pix);
		//
		//
		//
		bool changeBackgroundMode(short stack_index, short background_index, ma::Background_Mode mode);
		bool setBackgroundMoveWhenDraw(short stack_index, short background_index, bool new_state);

		bool setBackgroundSpeedX(short stack_index, short background_index, float val);
		bool setBackgroundSpeedY(short stack_index, short background_index, float val);
		bool setBackgroundSpeedXY(short stack_index, short background_index, float val_x, float val_y);

		bool editBackgroundParallaxParam(short stack_index, short background_index, ma::Parallax_Param_Field field, int solo_val, Rect non_solo_vals={0, 0, 100, 100});

		bool setBackgroundSensibilityX(short stack_index, short background_index, float val);
		bool setBackgroundSensibilityY(short stack_index, short background_index, float val);
		bool setBackgroundSensibilityXY(short stack_index, short background_index, float val_x, float val_y);

		bool setBackgroundScrollingXActiveState(short stack_index, short background_index, bool new_state);
		bool setBackgroundScrollingYActiveState(short stack_index, short background_index, bool new_state);
		bool setBackgroundScrollingXYActiveState(short stack_index, short background_index, bool new_state_x, bool new_state_y);

		bool setBackgroundImage(short stack_index, short background_index, Texture* texture, bool destroy_previous=false);
		bool setBackgroundImage(short stack_index, short background_index, const std::string& file_path, bool destroy_previous=false);

		bool setBackgroundSrc(short stack_index, short background_index, FRect vals);
		bool setBackgroundSrc(short stack_index, short background_index, int* x=nullptr, int* y=nullptr, int* w=nullptr, int* h=nullptr);
		//
		//
		short getNbOfBackgroundStacks() const;
		short getNbOfBackgrounds(short stack_index) const;

		Parallax_Param* getBackgroundParallaxParam(short stack_index=0, short background_index=0);
		FRect getBackgroundParallaxParam(short stack_index, short background_index, ma::Parallax_Param_Field field);

		bool getBackgroundMoveWhenDraw(short stack_index, short background_index);
		Vec2f getBackgroundSpeed(short stack_index, short background_index);
		Vec2f getBackgroundSensibility(short stack_index, short background_index);
		Vec2b getBackgroundScrollingActiveState(short stack_index, short background_index);

		Texture* getBackgroundImage(short stack_index, short background_index);
		FRect getBackgroundSrc(short stack_index, short background_index);




	protected :

		Texture* m_loadTexture(const std::string& path);
		int m_getRenW() const;
		int m_getRenH() const;
		float m_getRenWz(); //Retourne la taille qu'à la fenêtre de rendu, dans le monde virtuelle, en tenant compte du zoom.
		float m_getRenHz();
		bool isRenValid() const;
		void clearTilesets();



		std::vector<Map_Layer*>* INTERNAL_USE_getTargetLayers(std::string function_name, int layer_index);
		std::vector<Map_Layer*> m_helper_return_layers_array;
		bool INTERNAL_USE_stackIndexIsOk(std::string function_name, short stack_index, bool& is_default_index) const;
		bool INTERNAL_USE_backgroundCooAreOk(std::string function_name, short stack_index, bool& is_default_stack_index, short background_index, bool& is_default_background_index);
		std::vector<Background*>* INTERNAL_USE_getTargetBackgrounds(std::string function_name, short stack_index, short background_index);
		std::vector<Background*> m_helper_return_bg_array;
		bool INTERNAL_USE_addNewValueInVector(std::vector<short>* target, short val);

		bool INTERNAL_USE_backgroundBasicVeririf(std::string function_name, short background_index, bool& is_default_index);
		bool INTERNAL_USE_check_tile_access_is_ok(std::string function_name, int x, int y, int layer_index) const;
		bool m_helper__error_at_last_call_of_isCollisionTile=false;
		bool m_helper__error_at_last_call_of_hitCollisionTile=false;
		//bool m_destroy_ptrs_in_destructor=false;

		void setTileset__finalStep(int tileset_id, Tileset& tileset_buff);


		bool m_is_init=false;
		bool m_synchronized=true;

		bool m_have_main_layer=false;
		int m_main_layer_index=-1;

		Renderer* m_ren=nullptr;

		std::string m_title{"No_titled_map"};

		std::unordered_map<int, Tileset> m_tilesets;
		int m_next_layers_tileset_id=0;

		std::vector<Map_Layer> m_layers;
		int m_layers_count=0;

		//traitement des backgrounds (avec possibilité d'avoir plusieurs lots de backgrounds)
		std::vector<std::vector<Background>> m_backgrounds_stacks;

		//les infos pour traiter les collisions
		std::vector<Vec_Int_VecSInt> m_collision_layers;

		//la valeur de tuile pour laquelle, à l'initialisation, la tuile est notée comme inactive
		short m_idle_value=-1;

		//à partir d'où sur l'écran (la fenêtre ou la target) on va se référer comme point de départ des calculs de blittage.
		gs::Vec2f m_screen_offset{0.f,0.f};

		//taille pour les blocs de destinations et source (au pixel près)
		Vec2i m_destBLOC_custom_w_h={32,32};


		//zoom de la map qui viendra se multiplier au destBLOC pour l agrandir ou le retrecire
		float m_zoom=1.f;


		//destBLOC et srcBLOC sont par defaut pour des blocs carrés, qui ont la même taille des quatre cotés

};


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
/////
/////
/////
//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////


class Boundless_Map //Map (carte de jeu) psuedo infinie.
{
	public :

		bool init(Renderer* ren, gs::Vec2i dest_bloc, std::string title="No_titled_boudless_map", short idle_value=-1);

		bool setTileset(int tileset_id, const std::string& path, const Tileset_Loading_Args& tl_args);

		bool setChunkRequester(std::vector<std::vector<int>> (*chunkRequester)(const Vec2i& begin_tile__grid_pos, const Vec2i& last_tile__grid_pos, bool& is_ok));

		Vec2f setCameraPos(const Vec2f& pos); //Renvoie la postition précédente.

		Vec2f incrCameraPos(const Vec2f& vals); //Renvoie la position une fois incrémentée.

		bool setChunkSize(const Vec2i& size);

		bool setChunk(const Vec2i& chunk_pos, const std::vector<std::vector<int>>& tiles_grid);


		//----------------------------------------------//
		Vec2i getChunkSize() const;

		Vec2f getCameraPos() const;

		int getCurrentLoadedChunksCount() const;
		//----------------------------------------------//


		void update();

		bool drawLayer();


	protected :

		bool isRenValid() const { return m_ren != nullptr; }
		int m_getRenW() const;
		int m_getRenH() const;

		//Voici le choix fait : 'm_chunks_register' est tel un tableau, une grille de morceaux de carte (faisant référence à des layers de gs::Map).
		//un chunk est représenté dedans par son index dans 'm_chunks' par ses coordonnées x et y ().

		//Voici le choix fait : 'm_chunks_register' est tel un tableau à double entrée, donnant l'index référant au layer correspondant dans 'm_chunks', d'après le chunck de coordonnées x,y. Ce chunk peut ne pas être présent.
		//Le premier 'int' servira à retrouver le chunk par sa coordonée 'x', le deuxième 'int' par sa coordonée 'y', et le troisième 'int' donne l'index du layer correspondant rangé dans 'm_chunks'.
		//Notes : un chunk à des coordonnées en tableau, du style [0;0], [2;-5], [456;37], [-803;11] etc. Cela ne correspond pas à des pixels ou des tuiles. (Un chunk regoupe des tuiles, qui elles-même regroupent des pixels. Par exemple avec une taille de chunk de 10(tuiles), le chunk[2,-1] regroupe les tuiles de l'intervalle [20; 29] en x, et [-1; -10] en y.)
		std::unordered_map<int, std::unordered_map<int, int>> m_chunks_register;
		gs::Map m_chunks;

		Vec2i m_chunk_size={80,80}; //Représente les dimensions d'un chunk, nombres de tuiles par chunk en x et en y.

		bool m_is_init=false;
		Renderer* m_ren=nullptr;

		Vec2f m_cam_global_pos={0,0}; //Point d'ancrage (position) de la "caméra" dans la map sans bordures, en pixel.
		Vec2i m_curr_chunk={0,0};

		std::vector<Vec2i> m_required_chunks; //Permet de stocker les coordonnées des chunks non chargés.

		Vec2i getChunkLayerIndex(Vec2i chunk_pos); //Renvoie : {layer présent ou non, indice du layer}.
		std::vector<std::vector<int>> (*mm_chunkRequester)(const Vec2i& begin_tile__grid_pos, const Vec2i& last_tile__grid_pos, bool& is_ok)=nullptr;

};

};/*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*_*/


/*choses à faire : pourquoi ne pas rajouter une sorte de replaceTileValue() (une sorte de setTileValueAt() pour toutes celles qui ont une certaine valeur donnée). A réflechir.



*/

#endif //_GSlib_MAP_HPP_
