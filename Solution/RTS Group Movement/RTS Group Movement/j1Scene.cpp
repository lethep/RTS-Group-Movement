#include "Defs.h"
#include "p2Log.h"
#include <time.h>

#include "j1App.h"

#include "j1Input.h"
#include "j1Textures.h"
#include "j1Render.h"
#include "j1Window.h"
#include "j1Map.h"
#include "j1Scene.h"
#include "j1Pathfinding.h"
#include "j1Movement.h"
#include "j1EntityFactory.h"
#include "j1Collision.h"
#include "j1Particles.h"

#include"Brofiler\Brofiler.h"

j1Scene::j1Scene() : j1Module()
{
	name.assign("scene");
}

// Destructor
j1Scene::~j1Scene()
{}

// Called before render is available
bool j1Scene::Awake(pugi::xml_node& config)
{
	bool ret = true;

	LOG("Loading scene");

	// Load maps
	pugi::xml_node maps = config.child("maps");

	warcraftMap = maps.child("warcraft").attribute("name").as_string();
	warcraftActive = maps.child("warcraft").attribute("active").as_bool();
	warcraftTexName = maps.child("warcraft").attribute("tex").as_string();

	return ret;
}

// Called before the first frame
bool j1Scene::Start()
{
	bool ret = false;

	srand(time(NULL));

	// Save camera info
	App->win->GetWindowSize(width, height);
	scale = App->win->GetScale();

	// Load a warcraft-based map
	if (warcraftActive) {
		ret = App->map->Load(warcraftMap.data());
		debugTex = App->tex->Load(warcraftTexName.data());
	}

	// Create walkability map
	if (ret)
	{
		int w, h;
		uchar* data = NULL;
		if (App->map->CreateWalkabilityMap(w, h, &data))
			App->pathfinding->SetMap(w, h, data);

		RELEASE_ARRAY(data);
	}

	return ret;
}

// Called each loop iteration
bool j1Scene::PreUpdate()
{
	bool ret = true;

	// Save mouse position (world and map coords)
	int x, y;
	App->input->GetMousePosition(x, y);
	iPoint mousePos = App->render->ScreenToWorld(x, y);
	iPoint mouseTile = App->map->WorldToMap(mousePos.x, mousePos.y);
	iPoint mouseTilePos = App->map->MapToWorld(mouseTile.x, mouseTile.y);

	// ---------------------------------------------------------------------

	// Entities info
	/// Entity
	iPoint size = { App->map->data.tile_width,App->map->data.tile_height };
	uint maxLife = 30;
	int currLife = (int)maxLife;

	/// DynamicEntity
	UnitInfo unitInfo;
	unitInfo.damage = 2;
	unitInfo.priority = 1; // TODO: change to 3 or so

	/// Footman
	FootmanInfo footmanInfo;

	/// Grunt
	GruntInfo gruntInfo;

	/// Sheep
	CritterSheepInfo critterSheepInfo;
	critterSheepInfo.restoredHealth = 5;

	/// Boar
	CritterBoarInfo critterBoarInfo;
	critterBoarInfo.restoredHealth = 10;

	// Entities creation

	// 1: spawn a Footman with priority 1
	unitInfo.sightRadius = 6;
	unitInfo.attackRadius = 2;
	unitInfo.maxSpeed = 80.0f;

	if (App->input->GetKey(SDL_SCANCODE_1) == KEY_DOWN) {

		iPoint tile = { 10,10 };

		// Make sure that there are no entities on the spawn tile and that the tile is walkable
		if (App->entities->IsEntityOnTile(tile) != nullptr || !App->pathfinding->IsWalkable(tile))

			tile = FindClosestValidTile(tile);

		// Make sure that the spawn tile is valid
		//if (tile.x != -1 && tile.y != -1) {  // TODO: uncomment this line

			iPoint tilePos = App->map->MapToWorld(tile.x, tile.y);
			//fPoint pos = { (float)tilePos.x,(float)tilePos.y }; // TODO: uncomment this line

			fPoint pos = { (float)mouseTilePos.x,(float)mouseTilePos.y }; // TODO: delete this debug
			App->entities->AddDynamicEntity(DynamicEntityType_Footman, pos, size, currLife, maxLife, unitInfo, (EntityInfo&)footmanInfo);
		//}
	}

	// 2: spawn a Grunt with priority 1
	unitInfo.sightRadius = 5;
	unitInfo.attackRadius = 2;
	unitInfo.maxSpeed = 50.0f;

	maxLife = 20;
	currLife = (int)maxLife;

	if (App->input->GetKey(SDL_SCANCODE_2) == KEY_DOWN) {

		iPoint tile = { 15,11 };

		// Make sure that there are no entities on the spawn tile and that the tile is walkable
		if (App->entities->IsEntityOnTile(tile) != nullptr || !App->pathfinding->IsWalkable(tile))

			tile = FindClosestValidTile(tile);

		// Make sure that the spawn tile is valid
		//if (tile.x != -1 && tile.y != -1) { // TODO: uncomment this line

			iPoint tilePos = App->map->MapToWorld(tile.x, tile.y);
			//fPoint pos = { (float)tilePos.x,(float)tilePos.y }; // TODO: uncomment this line

			fPoint pos = { (float)mouseTilePos.x,(float)mouseTilePos.y }; // TODO: delete this debug
			App->entities->AddDynamicEntity(DynamicEntityType_Grunt, pos, size, currLife, maxLife, unitInfo, (EntityInfo&)gruntInfo);
		//}
	}

	// 5: spawn a group of Footmans
	if (App->input->GetKey(SDL_SCANCODE_5) == KEY_DOWN) {

		list<DynamicEntity*> units;
		uint maxFootmans = 4;

		for (uint i = 0; i < maxFootmans; ++i) {

			iPoint tile = { rand() % App->map->data.width,rand() % App->map->data.height };

			// Make sure that there are no entities on the spawn tile and that the tile is walkable
			if (App->entities->IsEntityOnTile(tile) != nullptr || !App->pathfinding->IsWalkable(tile))

				tile = FindClosestValidTile(tile);

			// Make sure that the spawn tile is valid
			if (tile.x != -1 && tile.y != -1) {

				iPoint tilePos = App->map->MapToWorld(tile.x, tile.y);
				fPoint pos = { (float)tilePos.x,(float)tilePos.y };

				DynamicEntity* dynEnt = App->entities->AddDynamicEntity(DynamicEntityType_Footman, pos, size, currLife, maxLife, unitInfo, (EntityInfo&)footmanInfo);

				if (dynEnt != nullptr)
					units.push_back(dynEnt);
			}
		}

		if (units.size() > 0)
			App->movement->CreateGroupFromUnits(units);
	}

	// 6: spawn a group of Grunts
	if (App->input->GetKey(SDL_SCANCODE_6) == KEY_DOWN) {

		list<DynamicEntity*> units;
		uint maxGrunts = 4;

		for (uint i = 0; i < maxGrunts; ++i) {

			iPoint tile = { rand() % App->map->data.width,rand() % App->map->data.height };

			// Make sure that there are no entities on the spawn tile and that the tile is walkable
			if (App->entities->IsEntityOnTile(tile) != nullptr || !App->pathfinding->IsWalkable(tile))

				tile = FindClosestValidTile(tile);

			// Make sure that the spawn tile is valid
			if (tile.x != -1 && tile.y != -1) {

				iPoint tilePos = App->map->MapToWorld(tile.x, tile.y);
				fPoint pos = { (float)tilePos.x,(float)tilePos.y };

				DynamicEntity* dynEnt = App->entities->AddDynamicEntity(DynamicEntityType_Grunt, pos, size, currLife, maxLife, unitInfo, (EntityInfo&)gruntInfo);

				if (dynEnt != nullptr)
					units.push_back(dynEnt);
			}
		}

		if (units.size() > 0)
			App->movement->CreateGroupFromUnits(units);
	}

	fPoint pos = { (float)mouseTilePos.x,(float)mouseTilePos.y };

	// 3: spawn a Sheep
	unitInfo.sightRadius = 0;
	unitInfo.attackRadius = 0;
	unitInfo.priority = 1;
	maxLife = 10;
	currLife = (int)maxLife;

	if (App->input->GetKey(SDL_SCANCODE_3) == KEY_DOWN)
		App->entities->AddDynamicEntity(DynamicEntityType_CritterSheep, pos, size, currLife, maxLife, unitInfo, (EntityInfo&)critterSheepInfo);

	// 4: spawn a Boar
	unitInfo.sightRadius = 0;
	unitInfo.attackRadius = 0;
	unitInfo.priority = 2;
	maxLife = 20;
	currLife = (int)maxLife;

	if (App->input->GetKey(SDL_SCANCODE_4) == KEY_DOWN)
		App->entities->AddDynamicEntity(DynamicEntityType_CritterBoar, pos, size, currLife, maxLife, unitInfo, (EntityInfo&)critterBoarInfo);

	return ret;
}

// Called each loop iteration
bool j1Scene::Update(float dt)
{
	BROFILER_CATEGORY(__FUNCTION__, Profiler::Color::Orchid);

	bool ret = true;

	// Save mouse position (world and map coords)
	int x, y;
	App->input->GetMousePosition(x, y);
	iPoint mousePos = App->render->ScreenToWorld(x, y);
	iPoint mouseTile = App->map->WorldToMap(mousePos.x, mousePos.y);
	iPoint mouseTilePos = App->map->MapToWorld(mouseTile.x, mouseTile.y);

	// ---------------------------------------------------------------------

	if (App->input->GetKey(SDL_SCANCODE_F1) == KEY_DOWN)
		isFrameByFrame = !isFrameByFrame;

	if (App->input->GetKey(SDL_SCANCODE_F2) == KEY_DOWN)
		debugDrawMovement = !debugDrawMovement;

	if (App->input->GetKey(SDL_SCANCODE_F3) == KEY_DOWN)
		debugDrawPath = !debugDrawPath;

	if (App->input->GetKey(SDL_SCANCODE_F4) == KEY_DOWN)
		debugDrawMap = !debugDrawMap;

	if (App->input->GetKey(SDL_SCANCODE_F5) == KEY_DOWN)
		debugDrawAttack = !debugDrawAttack;

	// Draw
	App->map->Draw(); // map
	App->particles->Draw(); // particles
	App->entities->Draw(); // entities

	if (debugDrawAttack)
		App->collision->DebugDraw(); // debug draw collisions

	if (debugDrawMovement)
		App->movement->DebugDraw(); // debug draw movement

	App->render->Blit(debugTex, mouseTilePos.x, mouseTilePos.y); // tile under the mouse pointer

	// Select units by mouse click
	if (App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_DOWN) {
		startRectangle = mousePos;

		Entity* entity = App->entities->IsEntityOnTile(mouseTile);

		if (entity != nullptr)
			App->entities->SelectEntity(entity);
		else
			App->entities->UnselectAllEntities();
	}

	int width = mousePos.x - startRectangle.x;
	int height = mousePos.y - startRectangle.y;

	/// SELECT UNITS
	// Select units by rectangle drawing
	if (abs(width) >= RECTANGLE_MIN_AREA && abs(height) >= RECTANGLE_MIN_AREA && App->input->GetMouseButtonDown(SDL_BUTTON_LEFT) == KEY_REPEAT) {

		// Draw the rectangle
		SDL_Rect mouseRect = { startRectangle.x, startRectangle.y, width, height };
		App->render->DrawQuad(mouseRect, 255, 255, 255, 255, false);

		// Select units within the rectangle
		if (width < 0) {
			mouseRect.x = mousePos.x;
			mouseRect.w *= -1;
		}
		if (height < 0) {
			mouseRect.y = mousePos.y;
			mouseRect.h *= -1;
		}

		App->entities->SelectEntitiesWithinRectangle(mouseRect);
	}

	list<DynamicEntity*> units = App->entities->GetLastUnitsSelected();

	if (units.size() > 0) {

		UnitGroup* group = App->movement->GetGroupByUnits(units);

		if (group == nullptr)

			// Selected units will now behave as a group
			group = App->movement->CreateGroupFromUnits(units);

		if (group != nullptr) {

			/// COMMAND PATROL
			if (App->input->GetKey(SDL_SCANCODE_P) == KEY_DOWN)

				App->entities->CommandToUnits(units, UnitCommand_Patrol);

			/// STOP UNIT (FROM WHATEVER THEY ARE DOING)
			if (App->input->GetKey(SDL_SCANCODE_S) == KEY_DOWN)

				App->entities->CommandToUnits(units, UnitCommand_Stop);

			/// COMMAND ATTACK
			/// Enemy
			Entity* target = App->entities->IsEntityOnTile(mouseTile, EntityType_NoType, EntitySide_Enemy);

			if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN && target != nullptr) {

				// All the group is issued to attack this enemy (and other enemies if seen when arrived at destination)
				list<DynamicEntity*>::const_iterator it = units.begin();

				bool isTarget = true;

				while (it != units.end()) {

					if (!(*it)->SetCurrTarget(target))
						isTarget = false;

					it++;
				}

				if (isTarget)

					App->entities->CommandToUnits(units, UnitCommand_AttackTarget);
			}

			/// Critter
			Entity* critter = App->entities->IsEntityOnTile(mouseTile, EntityType_NoType, EntitySide_Neutral);

			if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_DOWN && critter != nullptr) {

				// All the group is issued to attack this enemy (and other enemies if seen when arrived at destination)
				list<DynamicEntity*>::const_iterator it = units.begin();

				bool isTarget = true;

				while (it != units.end()) {

					if (!(*it)->SetCurrTarget(critter))
						isTarget = false;

					it++;
				}

				if (isTarget)

					App->entities->CommandToUnits(units, UnitCommand_AttackTarget);
			}

			/// SET GOAL (COMMAND MOVE TO POSITION)
			// Draw a shaped goal
			if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_REPEAT)

				group->DrawShapedGoal(mouseTile);

			// Set a normal or shaped goal
			if (App->input->GetMouseButtonDown(SDL_BUTTON_RIGHT) == KEY_UP) {

				bool isGoal = false;

				if (group->GetShapedGoalSize() <= 1) {
				
					group->ClearShapedGoal();

					if (group->SetGoal(mouseTile)) /// normal goal

						isGoal = true;
				}
				else if (group->SetShapedGoal()) /// shaped goal

					isGoal = true;

				if (isGoal) {

					uint isPatrol = 0;

					list<DynamicEntity*>::const_iterator it = units.begin();

					while (it != units.end()) {

						if ((*it)->GetUnitCommand() == UnitCommand_Patrol)
							isPatrol++;

						it++;
					}

					/// If all units are in the Patrol command or the AttackTarget command, do not set the MoveToPosition command
					bool isFull = false;

					if (isPatrol == units.size() || target != nullptr || critter != nullptr)
						isFull = true;

					if (!isFull)
						App->entities->CommandToUnits(units, UnitCommand_MoveToPosition);
				}
			}
		}
	}

	return ret;
}

// Called each loop iteration
bool j1Scene::PostUpdate()
{
	bool ret = true;

	if (App->input->GetKey(SDL_SCANCODE_ESCAPE) == KEY_DOWN)
		ret = false;

	return ret;
}

// Called before quitting
bool j1Scene::CleanUp()
{
	bool ret = true;

	LOG("Freeing scene");

	// Unload the map
	App->map->UnLoad();

	// Free all textures
	App->tex->UnLoad(debugTex);

	return ret;
}

iPoint j1Scene::FindClosestValidTile(iPoint tile) const 
{
	// Perform a BFS
	queue<iPoint> queue;
	list<iPoint> visited;

	iPoint curr = tile;
	queue.push(curr);

	while (queue.size() > 0) {

		curr = queue.front();
		queue.pop();

		if (!App->entities->IsEntityOnTile(curr) && App->pathfinding->IsWalkable(curr))
			return curr;

		iPoint neighbors[8];
		neighbors[0].create(curr.x + 1, curr.y + 0);
		neighbors[1].create(curr.x + 0, curr.y + 1);
		neighbors[2].create(curr.x - 1, curr.y + 0);
		neighbors[3].create(curr.x + 0, curr.y - 1);
		neighbors[4].create(curr.x + 1, curr.y + 1);
		neighbors[5].create(curr.x + 1, curr.y - 1);
		neighbors[6].create(curr.x - 1, curr.y + 1);
		neighbors[7].create(curr.x - 1, curr.y - 1);

		for (uint i = 0; i < 8; ++i)
		{
			if (App->pathfinding->IsWalkable(neighbors[i])) {

				if (find(visited.begin(), visited.end(), neighbors[i]) == visited.end()) {

					queue.push(neighbors[i]);
					visited.push_back(neighbors[i]);
				}
			}
		}
	}

	return { -1,-1 };
}

// -------------------------------------------------------------
// -------------------------------------------------------------

// Save
bool j1Scene::Save(pugi::xml_node& save) const
{
	bool ret = true;

	return ret;
}

// Load
bool j1Scene::Load(pugi::xml_node& save)
{
	bool ret = true;

	return ret;
}