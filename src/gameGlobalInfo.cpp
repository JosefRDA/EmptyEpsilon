#include <i18n.h>
#include "gameGlobalInfo.h"
#include "preferenceManager.h"
#include "resources.h"
#include "scienceDatabase.h"

P<GameGlobalInfo> gameGlobalInfo;

REGISTER_MULTIPLAYER_CLASS(GameGlobalInfo, "GameGlobalInfo")
GameGlobalInfo::GameGlobalInfo()
: MultiplayerObject("GameGlobalInfo")
{
    assert(!gameGlobalInfo);

    callsign_counter = 0;
    victory_faction = -1;
    gameGlobalInfo = this;

    for(int n=0; n<max_player_ships; n++)
    {
        playerShipId[n] = -1;
        registerMemberReplication(&playerShipId[n]);
    }

    for(int n=0; n<max_nebulas; n++)
    {
        nebula_info[n].vector = sf::Vector3f(random(-1, 1), random(-1, 1), random(-1, 1));
        nebula_info[n].textureName = "Nebula" + string(irandom(1, 3));
        registerMemberReplication(&nebula_info[n].vector);
        registerMemberReplication(&nebula_info[n].textureName);
    }

    global_message_timeout = 0.0;
    player_warp_jump_drive_setting = PWJ_ShipDefault;
    scanning_complexity = SC_Normal;
    hacking_difficulty = 2;
    hacking_games = HG_All;
    use_beam_shield_frequencies = true;
    use_system_damage = true;
    allow_main_screen_tactical_radar = true;
    allow_main_screen_long_range_radar = true;
    gm_control_code = "";
    elapsed_time = 0.0f;
    intercept_all_comms_to_gm = false;

    registerMemberReplication(&scanning_complexity);
    registerMemberReplication(&hacking_difficulty);
    registerMemberReplication(&hacking_games);
    registerMemberReplication(&global_message);
    registerMemberReplication(&global_message_timeout, 1.0);
    registerMemberReplication(&banner_string);
    registerMemberReplication(&victory_faction);
    registerMemberReplication(&use_beam_shield_frequencies);
    registerMemberReplication(&use_system_damage);
    registerMemberReplication(&allow_main_screen_tactical_radar);
    registerMemberReplication(&allow_main_screen_long_range_radar);
    registerMemberReplication(&gm_control_code);
    registerMemberReplication(&elapsed_time, 0.1);
    registerMemberReplication(&locals_name, 1.0);

    for(unsigned int n=0; n<factionInfo.size(); n++)
        reputation_points.push_back(0);
    registerMemberReplication(&reputation_points, 1.0);
}

//due to a suspected compiler bug this deconstructor needs to be explicitly defined
GameGlobalInfo::~GameGlobalInfo()
{
}

P<PlayerSpaceship> GameGlobalInfo::getPlayerShip(int index)
{
    assert(index >= 0 && index < max_player_ships);
    if (game_server)
        return game_server->getObjectById(playerShipId[index]);
    return game_client->getObjectById(playerShipId[index]);
}

void GameGlobalInfo::setPlayerShip(int index, P<PlayerSpaceship> ship)
{
    assert(index >= 0 && index < max_player_ships);
    assert(game_server);

    if (ship)
        playerShipId[index] = ship->getMultiplayerId();
    else
        playerShipId[index] = -1;
}

int GameGlobalInfo::findPlayerShip(P<PlayerSpaceship> ship)
{
    for(int n=0; n<max_player_ships; n++)
        if (getPlayerShip(n) == ship)
            return n;
    return -1;
}

int GameGlobalInfo::insertPlayerShip(P<PlayerSpaceship> ship)
{
    for(int n=0; n<max_player_ships; n++)
    {
        if (!getPlayerShip(n))
        {
            setPlayerShip(n, ship);
            return n;
        }
    }
    return -1;
}

void GameGlobalInfo::update(float delta)
{
    if (global_message_timeout > 0.0)
    {
        global_message_timeout -= delta;
    }
    if (my_player_info)
    {
        //Set the my_spaceship variable based on the my_player_info->ship_id
        if ((my_spaceship && my_spaceship->getMultiplayerId() != my_player_info->ship_id) || (my_spaceship && my_player_info->ship_id == -1) || (!my_spaceship && my_player_info->ship_id != -1))
        {
            if (game_server)
                my_spaceship = game_server->getObjectById(my_player_info->ship_id);
            else
                my_spaceship = game_client->getObjectById(my_player_info->ship_id);
        }
    }
    elapsed_time += delta;
}

string GameGlobalInfo::getNextShipCallsign()
{
    callsign_counter += 1;
    switch(irandom(0, 9))
    {
    case 0: return "S" + string(callsign_counter);
    case 1: return "NC" + string(callsign_counter);
    case 2: return "CV" + string(callsign_counter);
    case 3: return "SS" + string(callsign_counter);
    case 4: return "VS" + string(callsign_counter);
    case 5: return "BR" + string(callsign_counter);
    case 6: return "CSS" + string(callsign_counter);
    case 7: return "UTI" + string(callsign_counter);
    case 8: return "VK" + string(callsign_counter);
    case 9: return "CCN" + string(callsign_counter);
    }
    return "SS" + string(callsign_counter);
}

void GameGlobalInfo::addScript(P<Script> script)
{
    script_list.update();
    script_list.push_back(script);
}

void GameGlobalInfo::reset()
{
    gm_callback_functions.clear();

    foreach(GameEntity, e, entityList)
        e->destroy();
    foreach(SpaceObject, o, space_object_list)
        o->destroy();
    if (engine->getObject("scenario"))
        engine->getObject("scenario")->destroy();

    foreach(Script, s, script_list)
    {
        s->destroy();
    }
    for(unsigned int n=0; n<reputation_points.size(); n++)
        reputation_points[n] = 0;
    elapsed_time = 0.0f;
    callsign_counter = 0;
    victory_faction = -1;
}

void GameGlobalInfo::startScenario(string filename)
{
    reset();

    i18n::reset();
    i18n::load("locale/" + PreferencesManager::get("language", "en") + ".po");
    i18n::load("locale/" + filename.replace(".lua", "." + PreferencesManager::get("language", "en") + ".po"));

    flushDatabaseData();
    fillDefaultDatabaseData();

    P<ScriptObject> scienceInfoScript = new ScriptObject("science_db.lua");
    if (scienceInfoScript->getError() != "") exit(1);
    scienceInfoScript->destroy();

    P<ScriptObject> script = new ScriptObject();
    script->run(filename);
    engine->registerObject("scenario", script);

    if (PreferencesManager::get("game_logs", "1").toInt())
    {
        state_logger = new GameStateLogger();
        state_logger->start();
    }
}

void GameGlobalInfo::destroy()
{
    reset();
    MultiplayerObject::destroy();
    if (state_logger)
        state_logger->destroy();
}

string playerWarpJumpDriveToString(EPlayerWarpJumpDrive player_warp_jump_drive)
{
    switch(player_warp_jump_drive)
    {
    case PWJ_ShipDefault:
        return "Ship default";
    case PWJ_WarpDrive:
        return "Warp-drive";
    case PWJ_JumpDrive:
        return "Jump-drive";
    case PWJ_WarpAndJumpDrive:
        return "Both";
    default:
        return "?";
    }
}

string getSectorName(sf::Vector2f position, int scale_magnitude)
{
    string x;
    string y;
    string name = "";

    if (PreferencesManager::get("advanced_sector_system","0") == "1")
    {
        if (scale_magnitude != 0 && scale_magnitude != 2 && scale_magnitude != 4)
            return "";
        
        // Section (scale_magnitude == 0)
        int sector_x = floorf(position.x / GameGlobalInfo::sector_size);
        int sector_y = floorf(position.y / GameGlobalInfo::sector_size);
        
        // Draw area only if first sector of the area
        if (scale_magnitude == 2 && (sector_x % (8*8) != 0 || sector_y % (8*8) != 0))
            return "";

        // Draw region only if first area of the region
        if (scale_magnitude == 4 && (sector_x % (8*8*8*8) != 0 || sector_y % (8*8*8*8) != 0))
            return "";
        
        // Area (scale_magnitude == 2)
        int area_factor = std::pow(8,2) * GameGlobalInfo::sector_size;
        int area_x = floorf((position.x) / area_factor);
        int area_y = floorf((position.y) / area_factor);

        // Region (scale_magnitude == 5)
        int region_factor = std::pow(8,4) * GameGlobalInfo::sector_size;
        int region_x = floorf((position.x) / region_factor);
        int region_y = floorf((position.y) / region_factor);

        // Refactor sector based on area
        sector_x = sector_x - floorf((area_x * area_factor)/ GameGlobalInfo::sector_size);
        sector_y = sector_y - floorf((area_y * area_factor)/ GameGlobalInfo::sector_size);
        
        // Refactor area based on region
        area_x = area_x - floorf((region_x * region_factor)/ (GameGlobalInfo::sector_size * 64));
        area_y = area_y - floorf((region_y * region_factor)/ (GameGlobalInfo::sector_size * 64));
        
        x = string(sector_x + 1);
        if (sector_y >= 26)
            y = string(char('A' + floorf(sector_y / 26) - 1 )) + string(char('A' + (sector_y % 26)));
        else
            y = string(char('A' + (sector_y)));
        
        string sector_name = y + x;
        string area_name = string(area_x) + "." + string(area_y);
        string region_name = string(region_x) + "/" + string(region_y);
        
        string local_sector_name = sector_name;
        string local_area_name = area_name;
        string local_region_name = region_name;
        
        for(std::pair<string, string>& local_name : gameGlobalInfo->locals_name)
        {
            if (region_name == local_name.first)
                local_region_name = local_name.second;
            if (region_name + " " + area_name == local_name.first)
                local_area_name = local_name.second;
            if (region_name + " " + area_name + " " + sector_name == local_name.first)
                local_sector_name = local_name.second;
        }
        
        if (scale_magnitude == 0)
            return local_sector_name;
        if (scale_magnitude == 2)
            return local_area_name;
        if (scale_magnitude == 4)
            return local_region_name;
        return "";
    }
    else
    {
        int sector_x = floorf(position.x / GameGlobalInfo::sector_size) + 5;
        int sector_y = floorf(position.y / GameGlobalInfo::sector_size) + 5;

        if (sector_y >= 0)
            y = string(char('A' + (sector_y)));
        else
            y = string(char('z' + sector_y / 26)) + string(char('z' + 1 + (sector_y % 26)));
        if (sector_x >= 0)
            x = string(sector_x);
        else
            x = string(100 + sector_x);
        return y + x;
    }
}

static int addLocalName(lua_State* L)
{
    gameGlobalInfo->locals_name.emplace_back(luaL_checkstring(L, 1), luaL_checkstring(L, 2));    
    return 0;
}
/// addLocalName(string,string)
/// Personalize the name of a sector/area/region.
REGISTER_SCRIPT_FUNCTION(addLocalName);

static int victory(lua_State* L)
{
    gameGlobalInfo->setVictory(luaL_checkstring(L, 1));
    if (engine->getObject("scenario"))
        engine->getObject("scenario")->destroy();
    engine->setGameSpeed(0.0);
    return 0;
}
/// victory(string)
/// Called with a faction name as parameter, sets a certain faction as victor and ends the game.
REGISTER_SCRIPT_FUNCTION(victory);

static int globalMessage(lua_State* L)
{
    gameGlobalInfo->global_message = luaL_checkstring(L, 1);
    gameGlobalInfo->global_message_timeout = 5.0;
    return 0;
}
/// globalMessage(string)
/// Show a global message on the main screens of all active player ships.
/// The message is shown for 5 sec; new messages replace the old immediately.
REGISTER_SCRIPT_FUNCTION(globalMessage);

static int setBanner(lua_State* L)
{
    gameGlobalInfo->banner_string = luaL_checkstring(L, 1);
    return 0;
}
/// setBanner(string)
/// Show a scrolling banner containing this text on the cinematic and top down views.
REGISTER_SCRIPT_FUNCTION(setBanner);

static int getMissionTime(lua_State* L)
{
    lua_pushnumber(L, gameGlobalInfo->elapsed_time);
    return 1;
}
/// getMissionTime()
/// Return the elapsed time of the mission.
REGISTER_SCRIPT_FUNCTION(getMissionTime);

static int getPlayerShip(lua_State* L)
{
    int index = luaL_checkinteger(L, 1);
    if (index == -1)
    {
        for(index = 0; index<GameGlobalInfo::max_player_ships; index++)
        {
            P<PlayerSpaceship> ship = gameGlobalInfo->getPlayerShip(index);
            if (ship)
                return convert<P<PlayerSpaceship> >::returnType(L, ship);
        }
        return 0;
    }
    if (index < 1 || index > GameGlobalInfo::max_player_ships)
        return 0;
    P<PlayerSpaceship> ship = gameGlobalInfo->getPlayerShip(index - 1);
    if (!ship)
        return 0;
    return convert<P<PlayerSpaceship> >::returnType(L, ship);
}
/// getPlayerShip(index)
/// Return the player's ship, use -1 to get the first active player ship.
REGISTER_SCRIPT_FUNCTION(getPlayerShip);

static int getObjectsInRadius(lua_State* L)
{
    float x = luaL_checknumber(L, 1);
    float y = luaL_checknumber(L, 2);
    float r = luaL_checknumber(L, 3);

    sf::Vector2f position(x, y);

    PVector<SpaceObject> objects;
    PVector<Collisionable> objectList = CollisionManager::queryArea(position - sf::Vector2f(r, r), position + sf::Vector2f(r, r));
    foreach(Collisionable, obj, objectList)
    {
        P<SpaceObject> sobj = obj;
        if (sobj && (sobj->getPosition() - position) < r)
            objects.push_back(sobj);
    }

    return convert<PVector<SpaceObject> >::returnType(L, objects);
}
/// getObjectsInRadius(x, y, radius)
/// Return a list of all space objects at the x,y location within a certain radius.
REGISTER_SCRIPT_FUNCTION(getObjectsInRadius);

static int getAllObjects(lua_State* L)
{
    return convert<PVector<SpaceObject> >::returnType(L, space_object_list);
}
/// getAllObjects()
/// Return a list of all space objects. (Use with care, this could return a very long list which could slow down the game when called every update)
REGISTER_SCRIPT_FUNCTION(getAllObjects);

static int getScenarioVariation(lua_State* L)
{
    lua_pushstring(L, gameGlobalInfo->variation.c_str());
    return 1;
}
/// getScenarioVariation()
/// Returns the currently used scenario variation.
REGISTER_SCRIPT_FUNCTION(getScenarioVariation);

/** Short lived object to do a scenario change on the update loop. See "setScenario" for details */
class ScenarioChanger : public Updatable
{
public:
    ScenarioChanger(string script_name, string variation)
    : script_name(script_name), variation(variation)
    {
    }

    virtual void update(float delta)
    {
        gameGlobalInfo->variation = variation;
        gameGlobalInfo->startScenario(script_name);
        destroy();
    }
private:
    string script_name;
    string variation;
};

static int setScenario(lua_State* L)
{
    string script_name = luaL_checkstring(L, 1);
    string variation = luaL_optstring(L, 2, "");
    //This could be called from a currently active scenario script.
    // Calling GameGlobalInfo::startScenario is unsafe at this point,
    // as this will destroy the lua state that this function is running in.
    //So use the ScenarioChanger object which will do the change in the update loop. Which is safe.
    new ScenarioChanger(script_name, variation);
    return 0;
}
/// setScenario(script_name, variation_name)
/// Change the current scenario to a different one.
REGISTER_SCRIPT_FUNCTION(setScenario);

static int shutdownGame(lua_State* L)
{
    engine->shutdown();
    return 0;
}
/// Shutdown the game.
/// Calling this function will close the game. Mainly usefull for a headless server setup.
REGISTER_SCRIPT_FUNCTION(shutdownGame);

static int pauseGame(lua_State* L)
{
    engine->setGameSpeed(0.0);
    return 0;
}
/// Pause the game
/// Calling this function will pause the game. Mainly usefull for a headless server setup.
REGISTER_SCRIPT_FUNCTION(pauseGame);

static int unpauseGame(lua_State* L)
{
    engine->setGameSpeed(1.0);
    return 0;
}
/// Pause the game
/// Calling this function will pause the game. Mainly usefull for a headless server setup. As the scenario functions are not called when paused.
REGISTER_SCRIPT_FUNCTION(unpauseGame);

static int playSoundFile(lua_State* L)
{
    soundManager->playSound(luaL_checkstring(L, 1));
    return 0;
}
/// Play a sound file on the server. Will work with any file supported by SFML (.wav, .ogg, .flac)
/// Note that the sound is only played on the server. Not on any of the clients.
REGISTER_SCRIPT_FUNCTION(playSoundFile);

static int onNewPlayerShip(lua_State* L)
{
    int idx = 1;
    convert<ScriptSimpleCallback>::param(L, idx, gameGlobalInfo->on_new_player_ship);
    return 0;
}
/// Register a callback function that is called when a new ship is created on the ship selection screen.
REGISTER_SCRIPT_FUNCTION(onNewPlayerShip);

static int allowNewPlayerShips(lua_State* L)
{
    gameGlobalInfo->allow_new_player_ships = lua_toboolean(L, 1);
    return 0;
}
/// Set if the server is allowed to create new player ships from the ship creation screen.
/// allowNewPlayerShip(false) -- disallow new player ships to be created
REGISTER_SCRIPT_FUNCTION(allowNewPlayerShips);
