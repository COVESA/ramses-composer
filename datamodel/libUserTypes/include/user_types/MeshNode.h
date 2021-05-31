/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/GENIVI/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "user_types/Node.h"
#include "user_types/Mesh.h"
#include "user_types/Material.h"
#include "core/Handles.h"

#include <tuple>
#include <map>

namespace raco::user_types {

class MeshNode : public Node {
public:
	static inline const TypeDescriptor typeDescription = { "MeshNode", false };
	TypeDescriptor const& getTypeDescription() const override {
		return typeDescription;
	}

	MeshNode(MeshNode const& other) : Node(other), mesh_(other.mesh_), materials_(other.materials_) {
		fillPropertyDescription();
	}

	MeshNode(std::string name = std::string(), std::string id = std::string())
		: Node(name, id)
	{
		fillPropertyDescription();
	}

	void fillPropertyDescription() {
		properties_.emplace_back("mesh", &mesh_);
		properties_.emplace_back("materials", &materials_);
		properties_.emplace_back("instanceCount", &instanceCount_);
	}

	void onBeforeDeleteObject(Errors& errors) const override;

	void onAfterContextActivated(BaseContext& context) override;
	void onAfterReferencedObjectChanged(BaseContext& context, ValueHandle const& changedObject) override;
	void onAfterValueChanged(BaseContext& context, ValueHandle const& value) override;

	size_t numMaterialSlots();

	SMaterial getMaterial(size_t materialSlot);
	Table* getUniformContainer(size_t materialSlot);
	
	ValueHandle getMaterialHandle(size_t materialSlot);
	ValueHandle getUniformContainerHandle(size_t materialSlot);
	ValueHandle getMaterialOptionsHandle(size_t materialSlot);

    Property<SMesh, DisplayNameAnnotation> mesh_{nullptr, DisplayNameAnnotation("Mesh")};
    
    Property<Table, DisplayNameAnnotation> materials_{{}, DisplayNameAnnotation("Materials")};
	Property<int, DisplayNameAnnotation, RangeAnnotation<int>> instanceCount_{1, DisplayNameAnnotation("Instance Count"), RangeAnnotation<int>(1, 20)};

private:
	std::vector<std::string> getMaterialNames();
	
	void createMaterialSlot(std::string const& name);
	void updateMaterialSlots(BaseContext& context, std::vector<std::string> const& materialNames);

	void updateUniformContainer(BaseContext& context, const std::string& materialName, const Table* src, ValueHandle& destUniforms);

	void checkMeshMaterialAttributMatch(BaseContext& context);
	
	// Cached values of outdated uniforms.
	// Indexed by <material name, uniform name, uniform type> tuple
	std::map<std::tuple<std::string, std::string, raco::core::EnginePrimitive>, std::unique_ptr<ValueBase> > cachedUniformValues_;
};

using SMeshNode = std::shared_ptr<MeshNode>;

}
