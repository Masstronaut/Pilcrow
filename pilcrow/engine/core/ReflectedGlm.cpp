#include "pilcrow/engine/core/ReflectedGlm.hpp"

sreflDefineExternalType(glm::mat2)
{
  RegisterType<glm::mat2>();
  TypeBuilder<glm::mat2> builder;
}

sreflDefineExternalType(glm::mat3)
{
  RegisterType<glm::mat3>();
  TypeBuilder<glm::mat3> builder;
}

sreflDefineExternalType(glm::mat4)
{
  RegisterType<glm::mat4>();
  TypeBuilder<glm::mat4> builder;
}

sreflDefineExternalType(glm::mat4x3)
{
  RegisterType<glm::mat4x3>();
  TypeBuilder<glm::mat4x3> builder;
}

sreflDefineExternalType(glm::ivec2)
{
  RegisterType<glm::ivec2>();
  TypeBuilder<glm::ivec2> builder;

  builder.Field<&glm::ivec2::x>("x", PropertyBinding::GetSet);
  builder.Field<&glm::ivec2::y>("y", PropertyBinding::GetSet);
}

sreflDefineExternalType(glm::ivec3)
{
  RegisterType<glm::ivec3>();
  TypeBuilder<glm::ivec3> builder;
  builder.Field<&glm::ivec3::x>("x", PropertyBinding::GetSet);
  builder.Field<&glm::ivec3::y>("y", PropertyBinding::GetSet);
  builder.Field<&glm::ivec3::y>("z", PropertyBinding::GetSet);
}

sreflDefineExternalType(glm::ivec4)
{
  RegisterType<glm::ivec4>();
  TypeBuilder<glm::ivec4> builder;
  builder.Field<&glm::ivec4::x>("x", PropertyBinding::GetSet);
  builder.Field<&glm::ivec4::y>("y", PropertyBinding::GetSet);
  builder.Field<&glm::ivec4::y>("z", PropertyBinding::GetSet);
  builder.Field<&glm::ivec4::w>("w", PropertyBinding::GetSet);
}

sreflDefineExternalType(glm::vec2)
{
  RegisterType<glm::vec2>();
  TypeBuilder<glm::vec2> builder;

  builder.Field<&glm::vec2::x>("x", PropertyBinding::GetSet);
  builder.Field<&glm::vec2::y>("y", PropertyBinding::GetSet);
}

sreflDefineExternalType(glm::vec3)
{
  RegisterType<glm::vec3>();
  TypeBuilder<glm::vec3> builder;
  builder.Field<&glm::vec3::x>("x", PropertyBinding::GetSet);
  builder.Field<&glm::vec3::y>("y", PropertyBinding::GetSet);
  builder.Field<&glm::vec3::y>("z", PropertyBinding::GetSet);
}

sreflDefineExternalType(glm::vec4)
{
  RegisterType<glm::vec4>();
  TypeBuilder<glm::vec4> builder;
  builder.Field<&glm::vec4::x>("x", PropertyBinding::GetSet);
  builder.Field<&glm::vec4::y>("y", PropertyBinding::GetSet);
  builder.Field<&glm::vec4::y>("z", PropertyBinding::GetSet);
  builder.Field<&glm::vec4::w>("w", PropertyBinding::GetSet);
}

sreflDefineExternalType(glm::quat)
{
  RegisterType<glm::quat>();
  TypeBuilder<glm::quat> builder;
  builder.Field<&glm::quat::x>("x", PropertyBinding::GetSet);
  builder.Field<&glm::quat::y>("y", PropertyBinding::GetSet);
  builder.Field<&glm::quat::y>("z", PropertyBinding::GetSet);
  builder.Field<&glm::quat::w>("w", PropertyBinding::GetSet);
}

namespace GlmReflection
{
  void InitializeReflection()
  {
    srefl::InitializeType<glm::mat2>();
    srefl::InitializeType<glm::mat3>();
    srefl::InitializeType<glm::mat4>();
    srefl::InitializeType<glm::mat4x3>();
    srefl::InitializeType<glm::vec2>();
    srefl::InitializeType<glm::vec3>();
    srefl::InitializeType<glm::vec4>();
    srefl::InitializeType<glm::quat>();
    srefl::InitializeType<glm::ivec2>();
    srefl::InitializeType<glm::ivec3>();
    srefl::InitializeType<glm::ivec4>();
  }
}  // namespace GlmReflection