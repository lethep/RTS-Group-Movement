#ifndef __j1PATH_MANAGER_H__
#define __j1PATH_MANAGER_H__

#include "j1Module.h"
#include "p2Point.h"

#include <list>
#include <vector>
#include <algorithm>
using namespace std;

class Entity;
class PathPlanner;

class j1PathManager : public j1Module
{
public:

	j1PathManager(double msSearchPerUpdate);

	// Destructor
	~j1PathManager();

	// Called before quitting
	bool CleanUp();

	bool Update(float dt);

	// Every time this is called, the total amount of search cycles available will be shared out
	// equally between all the active path requests. If a search completes successfully or fails,
	// the method will notify the relevant bot
	void UpdateSearches();

	// A path planner should call this method to register a search with the manager
	// (this method checks to ensure the path planner is only registered once)
	void Register(PathPlanner* pathPlanner);

	// An agent can use this method to remove a search request
	void UnRegister(PathPlanner* pathPlanner);

private:

	list<PathPlanner*> searchRequests; // a container of all the active search requests
	
	// total ms to spend on search cycles each update allocated to the manager
	// each update step these are divided equally among all registered path requests
	double msSearchPerUpdate = 0.0f;

	j1PerfTimer timer; // timer to keep track of the ms spent on each update
};

class PathPlanner 
{
public:

	PathPlanner(Entity* owner);

	~PathPlanner();

	// Creates an instance of the A* time-sliced search and registers it with the path manager
	bool RequestAStar(iPoint origin, iPoint destination);

	// Creates an instance of the Dijkstra time-sliced search and registers it with the path manager
	bool RequestDijkstra(iPoint origin, iPoint destination);

	// Initializes all the PathPlanner variables for the next search
	void GetReadyForNewSearch();

	// The PathManager calls this to iterate once though the search cycle of the currently
	// asigned search algorithm. When a search is terminated the method messages the owner
	PathfindingStatus CycleOnce();

	// Called by an agent after it has been notified that the A* search has terminated successfully
	// to request the path found
	vector<iPoint> GetAStarPath() const;

	// Called by an agent after it has been notified that the Dijkstra search has terminated successfully
	// to request the tile found
	iPoint GetDijkstraTile() const;

private:

	Entity* entity = nullptr; // a pointer to the owner of this class

	
	// a local reference to the navgraph

	j1PathFinding* currentSearch = nullptr;

	vector<iPoint> path; // A* path
	iPoint tile = { -1,-1 }; // Dijkstra tile
};

#endif //__j1PATH_MANAGER_H__