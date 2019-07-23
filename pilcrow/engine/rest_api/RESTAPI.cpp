#include <algorithm>
#include <codecvt>  // conver to std::string
#include <fstream>
#include <iostream>
#include <locale>  // convert to std::string
#include <random>
#include <sstream>
#include <sstream>  // string parsing
#include <string>
#include <vector>
#include <set>

#include <cpprest/asyncrt_utils.h>
#include <cpprest/json.h>
#include <cpprest/uri.h>


#include "pilcrow/engine/core/Simulation.hpp"

#include "pilcrow/engine/rest_api/RESTAPI.h"

REST_VM::REST_VM(Simulation &sim, const utility::string_t &url)
  : m_listener(url), m_simulation(sim) {
  m_listener.support(
    std::bind(&REST_VM::handle_request, this, std::placeholders::_1));
}
REST_VM::~REST_VM() {
  // dtor
}

void REST_VM::handle_error(pplx::task<void> &t) {
  try {
    t.get();
  } catch(...) {
    // Ignore the error, Log it if a logger is available
    std::cout << "An error occurred.\n";
  }
}

REST_VM::RequestHandler::RequestHandler(Simulation &            sim,
                                        web::http::http_request request)
  : m_Request(request)
  , m_RequestPath(web::http::uri::split_path(web::http::uri::decode(m_Request.relative_uri().path())))
  , m_Simulation(sim) {
}
void REST_VM::RequestHandler::BadRequest(const std::string &response) {
  m_Request.reply(web::http::status_codes::BadRequest, response);
}
void REST_VM::RequestHandler::BadRequest() {
  m_Request.reply(web::http::status_codes::BadRequest);
}
void REST_VM::RequestHandler::GetWorlds() {
  std::vector<web::json::value> worldsAsJson
    = VecToJsonArray(m_Simulation.Worlds());
  web::json::value response{};
  response[U("Worlds")] = web::json::value::array(worldsAsJson);
  m_Request.reply(web::http::status_codes::OK, response);
}
void REST_VM::RequestHandler::PostWorld(const std::string &world) {
  m_Simulation.CreateWorld(world);
  web::json::value response;
  response[U("World")] = web::json::value::string(m_RequestPath[1]);
  m_Request.reply(web::http::status_codes::Created, response);
}
void REST_VM::RequestHandler::DeleteWorld(const std::string &world) {
  m_Simulation.DeleteWorld(world);
  m_Request.reply(web::http::status_codes::OK);
}
void REST_VM::RequestHandler::WorldEntities(const std::string &world) {
  // Path size is >= 4
  // Current path is /Worlds/<world>/Entities/
  EntityID ID{GetEntityID(m_RequestPath[3])};

  Entity *entity{m_Simulation.GetWorld(world).GetEntity(ID)};
  if(entity == nullptr) {
    web::json::value response;
    web::json::value IDJson;
    IDJson[U("Index")]      = ID.first;
    IDJson[U("Generation")] = ID.second;
    response[U("ID")]       = IDJson;
    m_Request.reply(web::http::status_codes::NotFound, response);
    return;
  }
  EntityRef ER{ID, &m_Simulation.GetWorld(world)};
  
  if(m_RequestPath.size() == 4) {
    if(m_Request.method() == web::http::methods::DEL) {
      ER.Kill();
      m_Request.reply(web::http::status_codes::OK);
      return;
    } else if(m_Request.method() == web::http::methods::PATCH) {
      PatchWorldEntity(ER);
    }
  } else if(m_RequestPath[4] == U("Components")) {
    if(m_RequestPath.size() == 5) { 
      GetEntityComponents(entity, ER);
      return;
    } else if(m_RequestPath.size() == 6) {
      EntityComponents(ER);
      return;
    }
  }
}

void REST_VM::RequestHandler::WorldSystems(const std::string &world) {
  // Path size is >= 4
  // Current path is /Worlds/<world>/Systems/

}
void REST_VM::RequestHandler::GetEntityComponents(const Entity *entity, EntityRef ER) {
  auto             components{VecToJsonArray(entity->GetComponentTypes())};
  web::json::value entityJson{EntityRefJson(ER)};
  entityJson[U("Components")] = web::json::value::array(components);
  m_Request.reply(web::http::status_codes::OK, entityJson);
}
void REST_VM::RequestHandler::EntityComponents(EntityRef entity) {

}
void REST_VM::RequestHandler::PatchWorldEntity(EntityRef entity) {
  auto queryParams
    = m_Request.relative_uri().split_query(m_Request.relative_uri().query());
  if(auto it{queryParams.find(U("Name"))}; it != queryParams.end()) {
    std::string name(it->second.begin(), it->second.end());
    auto        str_find_replace = [](std::string &      subject,
                               const std::string &search,
                               const std::string &replace) {
      size_t pos = 0;
      while((pos = subject.find(search, pos)) != std::string::npos) {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
      }
    };
    str_find_replace(name, std::string{"%20"}, std::string{" "});
    entity.Name(name);
    // @@TODO: Send a response
    m_Request.reply(web::http::status_codes::OK, EntityRefJson(entity));
  }
}
void REST_VM::RequestHandler::GetWorldSystems(const std::string &world) {
  std::vector<std::string> systems{m_Simulation.GetWorld(world).Systems()};
  std::vector<web::json::value> systemsJson;
  for(const auto &system : systems) {
    systemsJson.emplace_back(utility::string_t(system.begin(), system.end()));
  }
  web::json::value response;
  response[U("World")] = web::json::value::string(utility::string_t(world.begin(), world.end()));
  response[U("Systems")] = web::json::value::array(systemsJson);
  m_Request.reply(web::http::status_codes::OK, response);
}
void REST_VM::RequestHandler::GetWorldEntities(const std::string &world) {
  std::vector<web::json::value> entities;
  for (const auto& entity : m_Simulation.GetWorld(world).GetEntities()) {
    web::json::value entityJson;
    entityJson[U("Name")] = web::json::value::string(
      utility::string_t(entity.second.begin(), entity.second.end()));
    web::json::value id;
    id[U("Index")]      = entity.first.first;
    id[U("Generation")] = entity.first.second;
    entityJson[U("ID")] = id;
    entities.push_back(entityJson);
  }

  web::json::value response;
  response[U("World")]
    = web::json::value::string(utility::string_t(world.begin(), world.end()));
  response[U("Entities")] = web::json::value::array(entities);
  m_Request.reply(web::http::status_codes::OK, response);
}
void REST_VM::RequestHandler::PostWorldEntities(const std::string &world) {
  EntityRef entity{m_Simulation.GetWorld(world).Spawn()};
  m_Request.reply(web::http::status_codes::Created, EntityRefJson(entity));
}
void REST_VM::RequestHandler::Worlds() {
  if(m_RequestPath.size() == 1) {
    if(m_Request.method() == web::http::methods::GET) GetWorlds();
    else {
      BadRequest();
    }
    return;
  }
  std::string world{m_RequestPath[1].begin(), m_RequestPath[1].end()};
  if(m_RequestPath.size() == 2) {
    // Create or delete a world
    if(m_Request.method() == web::http::methods::POST)
      PostWorld(world);
    else if(m_Request.method() == web::http::methods::DEL)
      DeleteWorld(world);
    else
      BadRequest();
  } else if(m_RequestPath.size() == 3) {
    // Get properties of a World
    std::string property{m_RequestPath[2].begin(), m_RequestPath[2].end()};
    if(property == "Entities") {
      if(m_Request.method() == web::http::methods::GET)
        GetWorldEntities(world);
      else if(m_Request.method() == web::http::methods::POST)
        PostWorldEntities(world);
    } else if(property == "Systems") {
      GetWorldSystems(world);
    } else BadRequest();
  } else if(m_RequestPath.size() >= 4) {
    std::string property{m_RequestPath[2].begin(), m_RequestPath[2].end()};
    if(property == "Entities") {
      WorldEntities(world);
    } else if(property == "Systems") {
      WorldSystems(world);
    }
  
  }
}
EntityID REST_VM::RequestHandler::GetEntityID(const utility::string_t &token) {
  EntityID ID;

  utility::istringstream_t iss(token);
  iss >> ID.first;
  iss.get();  // pull out the comma
  iss >> ID.second;
  return ID;
}
std::vector<web::json::value>
REST_VM::RequestHandler::VecToJsonArray(const std::vector<std::string> &vec) {
  std::vector<web::json::value> result;
  result.reserve(vec.size());
  for (const auto& str : vec) {
    result.emplace_back(utility::string_t(str.begin(), str.end()));
  }
  return result;
}
void REST_VM::RequestHandler::Systems(const std::string &world) {
  if(m_RequestPath.size() == 1) { GetSystems();
  }

}
void REST_VM::RequestHandler::GetSystems() {
  auto                  worlds = m_Simulation.Worlds();
  std::set<std::string> systems;
  for(const auto &world : worlds) {
    auto worldSystems = m_Simulation.GetWorld(world).Systems();
    for(const auto system : worldSystems) {
      systems.insert(system);
    }
  }
  std::vector<web::json::value> systemsJson;
  for(const auto &system : systems) {
    systemsJson.push_back(web::json::value::string(
      utility::string_t(system.begin(), system.end())));
  }
  web::json::value response;
  response[U("Systems")] = web::json::value::array(systemsJson);
  m_Request.reply(web::http::status_codes::OK, response);
}
void REST_VM::RequestHandler::Components() {
// @@TODO: Return list of components
}
void REST_VM::RequestHandler::Handle() {
  if(m_RequestPath.empty()) {
    m_Request.reply(web::http::status_codes::BadRequest, "Empty Request.");
  } else if(m_RequestPath[0] == U("Worlds")) {
    Worlds();
  } else if(m_RequestPath[0] == U("Systems")) {
    Systems();
  } else if(m_RequestPath[0] == U("Components")) {
    Components();
  }
}
web::json::value
REST_VM::RequestHandler::EntityRefJson(EntityRef entity) {
  web::json::value entityJson;
  entityJson[U("Name")] = web::json::value::string(
    utility::string_t(entity.Name().begin(), entity.Name().end()));
  web::json::value id;
  id[U("index")]      = entity.ID().first;
  id[U("generation")] = entity.ID().second;
  entityJson[U("ID")] = id;
  return entityJson;
}
/*
  else if(m_RequestPath.size() > 1) {
          utility::istringstream_t iss(m_RequestPath[1]);
          EntityID                 ID;
          iss >> ID.first;
          iss.get();  // pull out the comma
          iss >> ID.second;
          Entity *entity{world.GetEntity(ID)};
          if(entity != nullptr) {
            m_Request.reply(web::http::status_codes::OK,
                            U("The Entity with ID {") + m_RequestPath[1]
                              + U("} is named \"")
                              + utility::string_t(entity->Name().begin(),
                                                  entity->Name().end())
                              + U("\""));
          } else {
            m_Request.reply(web::http::status_codes::NotFound,
                            "Entity requested does not exist.");
          }
        }
      }
*/

void REST_VM::handle_request(const web::http::http_request &message) {
  using web::http::uri;
  ucout << message.to_string() << std::endl;
  RequestHandler rh(m_simulation, message);
  rh.Handle();
}
