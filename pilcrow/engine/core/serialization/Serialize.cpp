#include "pilcrow/engine/core/serialization/Serialize.hpp"

#include "pilcrow/engine/core/ReflectedGlm.hpp"

///////////////////////////////
// Float
///////////////////////////////
inline float ValueAsFloat(web::json::value* aValue)
{
  if (aValue->is_double())
  {
    return static_cast<float>(aValue->as_double());
  }
  else if (aValue->is_string())
  {
    auto hex = aValue->as_string();
  
    unsigned int x = std::stoul(hex, nullptr, 16);
    return *reinterpret_cast<float*>(&x);
  }
  
  return 0.0f;
}

inline double ValueAsDouble(web::json::value* aValue)
{
  if (aValue->is_double())
  {
    return aValue->as_double();
  }
  else if (aValue->is_string())
  {
    auto hex = aValue->as_string();

    unsigned long long x = std::stoull(hex, nullptr, 16);
    return *reinterpret_cast<double*>(&x);
  }

  return 0.0f;
}

inline utility::string_t FloatToString(float aFloat)
{
  utility::ostringstream_t stm;
  stm << std::hex << std::uppercase << *reinterpret_cast<unsigned int*>(&aFloat);

  return stm.str();
}

inline utility::string_t DoubleToString(double aDouble)
{
  utility::ostringstream_t stm;
  stm << std::hex << std::uppercase << *reinterpret_cast<unsigned long long*>(&aDouble);
  
  return stm.str();
}

inline void FloatAsValue(web::json::value& aValue, float aFloat)
{
  aValue = web::json::value::string(FloatToString(aFloat));
}

inline web::json::value FloatAsValue(float aFloat)
{
  web::json::value value = web::json::value::string(FloatToString(aFloat));
  return std::move(value);
}

inline void DoubleAsValue(web::json::value& aValue, double aDouble)
{
  aValue = web::json::value::string(DoubleToString(aDouble));
}

inline web::json::value DoubleAsValue(double aDouble)
{
  web::json::value value = web::json::value::string(DoubleToString(aDouble));
  return std::move(value);
}

///////////////////////////////
// Real2
///////////////////////////////
inline glm::vec2 ValueAsReal2(web::json::value* aValue)
{
  auto real4 = aValue->at(U("Vector2"));
  auto x = ValueAsFloat(&real4.at(U("x")));
  auto y = ValueAsFloat(&real4.at(U("y")));
  return glm::vec2{ x, y };
}

inline void Real2AsValue(web::json::value& aValue, glm::vec2 aVector)
{
  //aValue.SetObject();
  //
  //RSValue subObject;
  //subObject.SetObject();
  //subObject.AddMember("x", FloatAsValue(aVector.x, aAllocator), aAllocator);
  //subObject.AddMember("y", FloatAsValue(aVector.y, aAllocator), aAllocator);
  //
  //aValue.AddMember("Vector2", subObject, aAllocator);
}

///////////////////////////////
// Real3
///////////////////////////////
inline glm::vec3 ValueAsReal3(web::json::value* aValue)
{
  auto real4 = aValue->at(U("Vector3"));
  auto x = ValueAsFloat(&real4.at(U("x")));
  auto y = ValueAsFloat(&real4.at(U("y")));
  auto z = ValueAsFloat(&real4.at(U("z")));
  return glm::vec3{ x, y, z };
}

inline void Real3AsValue(web::json::value& aValue, glm::vec3 aVector)
{
  //aValue.SetObject();
  //
  //RSValue subObject;
  //subObject.SetObject();
  //subObject.AddMember("x", FloatAsValue(aVector.x, aAllocator), aAllocator);
  //subObject.AddMember("y", FloatAsValue(aVector.y, aAllocator), aAllocator);
  //subObject.AddMember("z", FloatAsValue(aVector.z, aAllocator), aAllocator);
  //
  //aValue.AddMember("Vector3", subObject, aAllocator);
}

///////////////////////////////
// Real4
///////////////////////////////
inline glm::vec4 ValueAsReal4(web::json::value* aValue)
{
  auto real4 = aValue->at(U("Vector4"));
  auto x = ValueAsFloat(&real4.at(U("x")));
  auto y = ValueAsFloat(&real4.at(U("y")));
  auto z = ValueAsFloat(&real4.at(U("z")));
  auto w = ValueAsFloat(&real4.at(U("w")));
  return glm::vec4{ x, y, z, w };
}

inline void Real4AsValue(web::json::value& aValue, glm::vec4 aVector)
{
  aValue = web::json::value::object();
  aValue[U("x")]    = web::json::value::number(aVector.x);
  aValue[U("y")]    = web::json::value::number(aVector.y);
  aValue[U("z")]    = web::json::value::number(aVector.z);
  aValue[U("w")]    = web::json::value::number(aVector.w);
}

///////////////////////////////
// Quaternion
///////////////////////////////
inline glm::quat ValueAsQuaternion(web::json::value* aValue)
{
  auto real4 = aValue->at(U("Quaternion"));
  auto x = ValueAsFloat(&real4.at(U("x")));
  auto y = ValueAsFloat(&real4.at(U("y")));
  auto z = ValueAsFloat(&real4.at(U("z")));
  auto w = ValueAsFloat(&real4.at(U("w")));
  return glm::quat{ w, x, y, z };
}

inline void QuaternionAsValue(web::json::value& aValue, glm::quat aVector)
{
  //web::json::value subObject;
  //subObject.SetObject();
  //subObject.AddMember("x", FloatAsValue(aVector.x));
  //subObject.AddMember("y", FloatAsValue(aVector.y));
  //subObject.AddMember("z", FloatAsValue(aVector.z));
  //subObject.AddMember("w", FloatAsValue(aVector.w));
  //
  //aValue.AddMember("Quaternion", subObject, aAllocator);
}



void Deserialize(void* aObject, srefl::Type* aType)
{

}

void Serialize(void* aObject, srefl::OrderedMultiMap<std::string, std::unique_ptr<srefl::Property>>& aMap, web::json::value &blob)
{
  for (auto const& [name, property] : aMap)
  {
    auto& propertyName = property->GetName();

    // Set up the get so we can serialize its value.
    auto getter = property->GetGetter();
    auto any = getter->Invoke(aObject);
    auto propertyType = getter->GetReturnType()->GetMostBasicType();

    web::json::value propertyValue;

    // Type is a float
    if (propertyType == srefl::TypeId<float>())
    {
      auto value = any.As<float>();
      FloatAsValue(propertyValue, value);
    }
    else if (propertyType == srefl::TypeId<double>())
    {
      auto value = any.As<double>();
      DoubleAsValue(propertyValue, value);
    }
    else if (propertyType->GetEnumOf())
    {
      // TODO (Josh): Fix Enums
      __debugbreak();
      //Call call(property->Get, aState);
      //call.SetHandle(This, aSelf);
      //call.Invoke(reportForEnum);
      //u32 enumAsInt = call.Get<u32>(Return);
      //
      //String enumAsStr = property->PropertyType->GenericToString((byte*)&enumAsInt);
      //aBuilder.Value(enumAsStr);
    }
    // Type is an int.
    else if (propertyType == srefl::TypeId<std::uint32_t>())
    {
      propertyValue = web::json::value::number(any.As<std::uint32_t>());
    }
    else if (propertyType == srefl::TypeId<std::uint64_t>())
    {
      propertyValue = web::json::value::number(any.As<std::uint64_t>());
    }
    else if (propertyType == srefl::TypeId<std::int64_t>())
    {
      propertyValue = web::json::value::number(any.As<std::int64_t>());
    }
    else if (propertyType == srefl::TypeId<std::int32_t>())
    {
      propertyValue = web::json::value::number(any.As<std::int32_t>());
    }
    // Type is a string.
    else if (propertyType == srefl::TypeId<std::string>())
    {
      auto& value = any.As<std::string>();
      propertyValue = web::json::value::string(
        utility::string_t(value.begin(), value.end()));
    }
    // Type is a Boolean.
    else if(propertyType == srefl::TypeId<bool>())
    {
      propertyValue = web::json::value::boolean(any.As<bool>());
    }
    // Type is a Real2.
    else if(propertyType == srefl::TypeId<glm::vec2>())
    {
      auto value = any.As<glm::vec2>();
      Real2AsValue(propertyValue, value);
    }
    // Type is a Real3.
    else if(propertyType == srefl::TypeId<glm::vec3>())
    {
      auto value = any.As<glm::vec3>();
      Real3AsValue(propertyValue, value);
    }
    // Type is a Real4.
    else if(propertyType == srefl::TypeId<glm::vec4>())
    {
      auto value = any.As<glm::vec4>();
      Real4AsValue(propertyValue, value);
    }
    // Type is a Quaternion.
    else if(propertyType == srefl::TypeId<glm::quat>())
    {
      auto value = any.As<glm::quat>();
      QuaternionAsValue(propertyValue, value);
    }

    utility::string_t propertyNameValue{
      utility::string_t(propertyName.begin(), propertyName.end())};

    blob[propertyNameValue] = propertyValue;
  }
}

void Serialize(void* aObject, srefl::Type* aType)
{

}