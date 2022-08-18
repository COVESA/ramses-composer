/*
 * SPDX-License-Identifier: MPL-2.0
 *
 * This file is part of Ramses Composer
 * (see https://github.com/bmwcarit/ramses-composer).
 *
 * This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
 * If a copy of the MPL was not distributed with this file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

namespace raco::application {
class RaCoApplication;
}

namespace raco::python_api {

bool preparePythonEnvironment(std::wstring argv0, bool searchPythonFolderForTest = false);
void setup(raco::application::RaCoApplication* app);
int runPythonScript(raco::application::RaCoApplication* app, const std::wstring& applicationPath, const std::string& pythonScriptPath, const std::vector<const char*>& pos_argv_cp);
}
