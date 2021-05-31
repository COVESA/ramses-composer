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

#include "core/Context.h"
#include "ramses_adaptor/ObjectAdaptor.h"
#include "components/DataChangeDispatcher.h"
#include "ramses_adaptor/utilities.h"
#include "user_types/Mesh.h"
#include <unordered_map>

namespace raco::ramses_adaptor {

class SceneAdaptor;
using VertexDataMap = std::unordered_map<std::string, raco::ramses_base::RamsesArrayResource>;

class MeshAdaptor final : public ObjectAdaptor {
public:
	explicit MeshAdaptor(SceneAdaptor* sceneAdaptor, user_types::SMesh mesh);

	raco::ramses_base::RamsesArrayResource indicesPtr();
	const VertexDataMap& vertexData() const;
	bool isValid();
	
	core::SEditorObject baseEditorObject() noexcept override;
	const core::SEditorObject baseEditorObject() const noexcept override;

	bool sync(core::Errors* errors) override;

private:
	user_types::SMesh editorObject_;
	VertexDataMap vertexDataMap_;
	raco::ramses_base::RamsesArrayResource indices_;
	core::FileChangeMonitor::UniqueListener meshFileChangeListener_;
	components::Subscription subscription_;
	components::Subscription nameSubscription_;
};

};	// namespace raco::ramses_adaptor
