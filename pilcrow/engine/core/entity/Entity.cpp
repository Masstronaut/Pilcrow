#include <typeinfo>
#include <utility>
#include <vector>

//#include <SimpleReflection/Type.hpp>

#include "pilcrow/engine/core/entity/Entity.hpp"
#include "pilcrow/engine/core/entity/EntityRef.hpp"

#include "pilcrow/engine/core/World.hpp"


Entity::Entity(World &world, EntityID id)
  : m_World(world), m_ID(std::move(id)), m_Name("Nameless Entity") {}
Entity::Entity(Entity &&rhs)
  : m_World(rhs.m_World)
  , m_Components(std::move(rhs.m_Components))
  , m_ID(rhs.m_ID)
  , m_Name(std::move(rhs.m_Name)) {}
Entity &Entity::operator=(Entity &&rhs) {
  this->SafelyDisposeComponents();
  m_Components = std::move(rhs.m_Components);
  m_ID         = rhs.m_ID;
  m_Name       = std::move(rhs.m_Name);
  return *this;
}
Entity::~Entity() { this->SafelyDisposeComponents(); }

bool Entity::Has(std::type_index component_type) const {
  return m_Components.count(component_type) > 0;
}

void *Entity::Get(const std::string &Component) { 
  std::string componentName;
  for (const auto& kv : m_Components) {
    componentName = kv.first.name();
	if (auto idx{ componentName.find(Component) }; idx != std::string::npos) {
      return this->m_World.GetComponentPool(kv.first)->Get(kv.second);
	}
  }
  return nullptr; 
}

void Entity::SafelyDisposeComponents() {
  for(auto &component : m_Components) {
    m_World.GetComponentPool(component.first)->Erase(component.second);
  }
  m_Components.clear();
}

EntityID Entity::Clone(World &world, Entity &entity) const {
  for(const auto &comp : m_Components) {
    entity.m_Components.emplace(
      m_World.GetComponentPool(comp.first)->Clone(comp.second, world));
  }
  world.Emit<EntitySpawnedEvent>(
    EntitySpawnedEvent(EntityRef(entity.ID(), &world)));
  return entity.ID();
}

EntityID Entity::ID() const { return m_ID; }

const std::string &Entity::Name() const { return m_Name; }
Entity &           Entity::Name(const std::string &name) {
  m_Name = name;
  return *this;
}

std::vector<std::string> Entity::GetComponentTypes() const {
  std::vector<std::string> result;
  for (const auto& kv : m_Components) {
    result.push_back(kv.first.name());
  }
  for (std::string& component : result) {
    if (auto it{ component.find(' ') }; it != std::string::npos) {
      component = component.substr(it + 1, std::string::npos);
    }
  }
  return result;
}

void *Entity::Get(std::type_index component) {
  return m_World.GetComponent(m_Components[component], component);
  return nullptr;
}
