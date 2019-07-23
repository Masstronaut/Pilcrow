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
  //aValue.SetObject();
  //
  //RSValue subObject;
  //subObject.SetObject();
  //subObject.AddMember("x", FloatAsValue(aVector.x, aAllocator), aAllocator);
  //subObject.AddMember("y", FloatAsValue(aVector.y, aAllocator), aAllocator);
  //subObject.AddMember("z", FloatAsValue(aVector.z, aAllocator), aAllocator);
  //subObject.AddMember("w", FloatAsValue(aVector.w, aAllocator), aAllocator);
  //
  //aValue.AddMember("Vector4", subObject, aAllocator);
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

void Serialize(void* aObject, srefl::OrderedMultiMap<std::string, std::unique_ptr<srefl::Property>>& aMap)
{
  for (auto const& [name, property] : aMap)
  {
    auto& propertyName = property->GetName();

    // If the bound field/property does not have the Property Attribute, do nothing.
    if (!property->GetAttribute<Serializable>())
    {
      continue;
    }

    if (auto redirectAttribute = property->GetAttribute<RedirectObject>();
      nullptr != redirectAttribute)
    {
      auto object = redirectAttribute->Serialize(aAllocator, aSelf);

      RSValue propName;
      propName.SetString(redirectAttribute->GetName().c_str(),
        static_cast<RSSizeType>(redirectAttribute->GetName().size()),
        aAllocator);

      aValue.AddMember(propName, object, aAllocator);
      continue;
    }

    // Set up the get so we can serialize its value.
    auto getter = property->GetGetter();
    auto any = getter->Invoke(aSelf);
    auto propertyType = getter->GetReturnType()->GetMostBasicType();

    RSValue propertyValue;

    // Type is a float
    if (propertyType == TypeId<float>())
    {
      auto value = any.As<float>();
      FloatAsValue(propertyValue, value, aAllocator);
    }
    else if (propertyType == TypeId<double>())
    {
      auto value = any.As<double>();
      DoubleAsValue(propertyValue, value, aAllocator);
    }
    else if (propertyType->GetEnumOf())
    {
      // TODO (Josh): Fix Enums
      debugbreak();
      //Call call(property->Get, aState);
      //call.SetHandle(This, aSelf);
      //call.Invoke(reportForEnum);
      //u32 enumAsInt = call.Get<u32>(Return);
      //
      //String enumAsStr = property->PropertyType->GenericToString((byte*)&enumAsInt);
      //aBuilder.Value(enumAsStr);
    }
    // Type is an int.
    else if (propertyType == TypeId<u32>())
    {
      propertyValue.SetUint(any.As<u32>());
    }
    else if (propertyType == TypeId<u64>())
    {
      propertyValue.SetUint64(any.As<u64>());
    }
    else if (propertyType == TypeId<i64>())
    {
      propertyValue.SetInt64(any.As<i64>());
    }
    else if (propertyType == TypeId<i32>())
    {
      propertyValue.SetInt(any.As<i32>());
    }
    // Type is a string.
    else if (propertyType == TypeId<String>())
    {
      auto& value = any.As<String>();
      propertyValue.SetString(value.c_str(), static_cast<RSSizeType>(value.Size()), aAllocator);
    }
    else if (propertyType == TypeId<std::string>())
    {
      auto& value = any.As<std::string>();
      propertyValue.SetString(value.c_str(), static_cast<RSSizeType>(value.size()), aAllocator);
    }
    // Type is a Boolean.
    else if (propertyType == TypeId<bool>())
    {
      propertyValue.SetBool(any.As<bool>());
    }
    // Type is a Real2.
    else if (propertyType == TypeId<glm::vec2>())
    {
      auto value = any.As<glm::vec2>();
      Real2AsValue(propertyValue, value, aAllocator);
    }
    // Type is a Real3.
    else if (propertyType == TypeId<glm::vec3>())
    {
      auto value = any.As<glm::vec3>();
      Real3AsValue(propertyValue, value, aAllocator);
    }
    // Type is a Real4.
    else if (propertyType == TypeId<glm::vec4>())
    {
      auto value = any.As<glm::vec4>();
      Real4AsValue(propertyValue, value, aAllocator);
    }
    // Type is a Quaternion.
    else if (propertyType == TypeId<glm::quat>())
    {
      auto value = any.As<glm::quat>();
      QuaternionAsValue(propertyValue, value, aAllocator);
    }

    RSValue propertyNameValue;
    propertyNameValue.SetString(propertyName.c_str(),
      static_cast<RSSizeType>(propertyName.size()),
      aAllocator);

    aValue.AddMember(propertyNameValue, propertyValue, aAllocator);
  }
}

void Serialize(void* aObject, srefl::Type* aType)
{

}