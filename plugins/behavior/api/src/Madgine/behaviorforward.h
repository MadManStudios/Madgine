#pragma once

#include "Generic/fixed_string.h"

namespace Engine {

struct Behavior;
struct BehaviorStateBase;
struct BehaviorReceiver;
struct BehaviorError;
struct BehaviorList;

struct ParameterTuple;

struct BehaviorFactoryBase;

template <fixed_string Name, typename T>
struct Binding;

struct BindingDescriptor;

struct HandlerBase;
struct HandlerManager;

}
