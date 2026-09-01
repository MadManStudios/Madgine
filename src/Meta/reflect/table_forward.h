#pragma once

namespace Engine::Scene::Entity {
struct EntityComponentBase;
}

namespace Engine::Behavior {
struct BehaviorSender;
}

DLL_IMPORT_VARIABLE(const Engine::Reflect::MetaTable, table, SINGLE_ARG(Engine::Concepts::NoneOf<Engine::Void, Engine::Reflect::ScopePtr, Engine::Reflect::Value, Engine::Reflect::Result, std::reference_wrapper<Engine::Behavior::BehaviorSender>>));
