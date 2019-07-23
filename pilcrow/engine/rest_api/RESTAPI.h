#pragma once

#include <vector>
#include <string>
#include <cpprest/asyncrt_utils.h>
#include <cpprest/http_listener.h>
#include <cpprest/json.h>

class Simulation;
class Entity;
class World;

class REST_VM {
public:
  REST_VM(Simulation &sim, const utility::string_t &url);
  REST_VM(const REST_VM &) = delete;
  REST_VM(REST_VM &&)      = delete;
  REST_VM &operator=(const REST_VM &) = delete;
  REST_VM &operator=(REST_VM &&) = delete;

  ~REST_VM();

  pplx::task<void> Open() { return m_listener.open(); }
  pplx::task<void> Close() { return m_listener.close(); }

  class RequestHandler {
  public:
    RequestHandler(Simulation &sim, web::http::http_request request);
    void Handle();
    web::json::value EntityRefJson(EntityRef entity);
  private:
    static EntityID GetEntityID(const utility::string_t &token);
    static std::vector<web::json::value>
         VecToJsonArray(const std::vector<std::string> &vec);
    void BadRequest();
    void BadRequest(const std::string &response);

    Entity *GetEntity(const utility::string_t &name);

    void    Systems(const std::string &world = "");
    void    GetSystems();

    void    Components();

    void    Worlds();
    void    GetWorlds();
    void    GetWorldSystems(const std::string &world);
    void    GetWorldEntities(const std::string &world);
    void    PostWorldEntities(const std::string &world);
    void    PostWorld(const std::string &world);
    void    DeleteWorld(const std::string &world);
    void    WorldEntities(const std::string &world);
    void    WorldSystems(const std::string &world);
    void    GetEntityComponents(const Entity *entity, EntityRef ER);
    void    EntityComponents(Entity *entity, EntityRef ER);

    void PatchWorldEntity(EntityRef entity);


    web::http::http_request        m_Request;
    std::vector<utility::string_t> m_RequestPath;
    Simulation &                   m_Simulation;
  };

private:
  void handle_request(const web::http::http_request &message);
  void handle_error(pplx::task<void> &t);
  web::http::experimental::listener::http_listener m_listener;
  Simulation &                                     m_simulation;
};

