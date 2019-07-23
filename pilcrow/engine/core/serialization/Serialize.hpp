#include <string>
#include <cpprest/json.h>

#include "SimpleReflection/Meta.hpp"
#include "SimpleReflection/Type.hpp"


void Deserialize(void* aObject, srefl::Type* aType);
void Serialize(void* aObject, srefl::Type* aType);