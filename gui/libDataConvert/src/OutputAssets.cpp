﻿
#include "data_Convert/OutputAssets.h"
#include "data_Convert/ProgramDefine.h"
#include "PropertyData/PropertyType.h"
#include "style/Icons.h"

#include <set>
#include <stack>
#include <QMessageBox>
#include <cmath>

namespace raco::dataConvert {
using namespace raco::style;

// A curve is bound by multiple animations.
//std::map<std::string, std::set<std::string>> curveNameAnimation_;
//        curveName ,   animation1, animation2, animation3...
std::map<std::string, std::map<std::string, std::vector<AnimationsSingleCurve>>> curveNameAnimations_;
	//        curveName  ,          [Animation,      [nodeID,   defaultData]]
// A property of a node is bound by different curves of different animations.
std::map<std::string, std::vector<std::map<std::string, std::vector<std::map<std::string, CurvesSingleProp>>>>> pNodePropCurveNames_;
//        pNodeID ,	          <   propertyName,  [ < Animation, CurvesSingleProp > ]	>

std::set<std::string> HasOpacityMaterials_;

float NodeScaleSize_;

std::string delUniformNamePrefix(std::string nodeName) {
	int index = nodeName.rfind("uniforms.");
	if (-1 != index) {
		nodeName = nodeName.substr(9, nodeName.length());
	}
	return nodeName;
}

void OutputPtx::setMeshBaseNode(NodeData* node, HmiScenegraph::TNode* baseNode) {
	std::string nodeName = node->getName();
	std::string baseNodeName = nodeName + "Shape";
	baseNode->set_name(baseNodeName);

	if (node->hasSystemData("scale")) {
		TVector3f* scale = new TVector3f();
		Vec3 scal = std::any_cast<Vec3>(node->getSystemData("scale"));
		scale->set_x(scal.x);
		scale->set_y(scal.y);
		scale->set_z(scal.z);
		baseNode->set_allocated_scale(scale);
	}
	if (node->hasSystemData("rotation")) {
		TVector3f* rotation = new TVector3f();
		Vec3 rota = std::any_cast<Vec3>(node->getSystemData("rotation"));
		rotation->set_x(rota.x);
		rotation->set_y(rota.y);
		rotation->set_z(rota.z);
		baseNode->set_allocated_rotation(rotation);
	}
	if (node->hasSystemData("translation")) {
		TVector3f* translation = new TVector3f();
		Vec3 tran = std::any_cast<Vec3>(node->getSystemData("translation"));
		translation->set_x(tran.x);
		translation->set_y(tran.y);
		translation->set_z(tran.z);
		baseNode->set_allocated_translation(translation);
	}
}

bool uniformCompare(Uniform data, Uniform myUni) {
	bool result = false;
	UniformType dataType = data.getType();
	switch (dataType) {
		case raco::guiData::Null:
			// Do not have
			break;
		case raco::guiData::Bool: {
			uint32_t detemp = std::any_cast<bool>(data.getValue());
			uint32_t mytemp = std::any_cast<bool>(myUni.getValue());
			if (detemp == mytemp) {
				result = true;
			}
			break;
		}
		case raco::guiData::Int: {
			int detemp = std::any_cast<int>(data.getValue());
			int mytemp = std::any_cast<int>(myUni.getValue());
			if (detemp == mytemp) {
				result = true;
			}

			break;
		}
		case raco::guiData::Double: {
			float temp = std::any_cast<double>(data.getValue());
			float mytemp = std::any_cast<double>(myUni.getValue());
			if (temp == mytemp) {
				result = true;
			}
			break;
		}
		case raco::guiData::String:
			// Do not have
			break;
		case raco::guiData::Ref:
			// Do not have
			break;
		case raco::guiData::Table:
			// Do not have
			break;
		case raco::guiData::Vec2f: {
			Vec2 temp = std::any_cast<Vec2>(data.getValue());
			Vec2 mytemp = std::any_cast<Vec2>(myUni.getValue());
			if (temp.x == mytemp.x && temp.y == mytemp.y) {
				result = true;
			}
			break;
		}
		case raco::guiData::Vec3f: {
			Vec3 temp = std::any_cast<Vec3>(data.getValue());
			Vec3 mytemp = std::any_cast<Vec3>(myUni.getValue());
			if (temp.x == mytemp.x && temp.y == mytemp.y && temp.z == mytemp.z) {
				result = true;
			}
			break;
		}
		case raco::guiData::Vec4f: {
			Vec4 temp = std::any_cast<Vec4>(data.getValue());
			Vec4 mytemp = std::any_cast<Vec4>(myUni.getValue());
			if (temp.x == mytemp.x && temp.y == mytemp.y && temp.z == mytemp.z && temp.w == mytemp.w) {
				result = true;
			}
			break;
		}
		case raco::guiData::Vec2i: {
			Vec2int temp = std::any_cast<Vec2int>(data.getValue());
			Vec2int mytemp = std::any_cast<Vec2int>(myUni.getValue());
			if (temp.x == mytemp.x && temp.y == mytemp.y) {
				result = true;
			}
			break;
		}
		case raco::guiData::Vec3i: {
			Vec3int temp = std::any_cast<Vec3int>(data.getValue());
			Vec3int mytemp = std::any_cast<Vec3int>(myUni.getValue());
			if (temp.x == mytemp.x && temp.y == mytemp.y && temp.z == mytemp.z) {
				result = true;
			}
			break;
		}
		case raco::guiData::Vec4i: {
			Vec4int temp = std::any_cast<Vec4int>(data.getValue());
			Vec4int mytemp = std::any_cast<Vec4int>(myUni.getValue());
			if (temp.x == mytemp.x && temp.y == mytemp.y && temp.z == mytemp.z && temp.w == mytemp.w) {
				result = true;
			}
			break;
		}
		case raco::guiData::Struct:
			// Do not have
			break;
		default:
			break;
	}
	return result;
}


void OutputPtx::setPtxTMesh(NodeData* node, HmiScenegraph::TMesh& tMesh) {
	// set baseNode data
	HmiScenegraph::TNode* baseNode = new HmiScenegraph::TNode();
	setMeshBaseNode(node, baseNode);
	tMesh.set_allocated_basenode(baseNode);

	MaterialData materialData;
	NodeMaterial nodeMaterial;
	MeshData meshData;

	if(raco::guiData::MeshDataManager::GetInstance().getMeshData(node->objectID(), meshData)){
		//setMeshUniform(node, meshData);
		// set meshresource
		tMesh.set_meshresource(meshData.getMeshUri());
		// usedAttributes
		if (raco::guiData::MaterialManager::GetInstance().getMaterialData(node->objectID(), materialData)) {
			for (auto& attr : materialData.getUsedAttributes()) {
				HmiScenegraph::TMesh_TAttributeParamteter tempAttr;
				tempAttr.set_name(attr.name);
				HmiScenegraph::TMesh_TAttributeParamteter* itAttr = tMesh.add_attributeparameter();
				*itAttr = tempAttr;
			}
			// if node has material data，so it has nodeMaterial
			raco::guiData::MaterialManager::GetInstance().getNodeMaterial(node->objectID(), nodeMaterial);
			// set material reference name
			tMesh.set_materialreference(materialData.getObjectName());

			// uniforms for mesh
			if (nodeMaterial.isPrivate()) {
				for (auto& uniform : nodeMaterial.getUniforms()) {
					HmiScenegraph::TUniform tUniform;
					uniformTypeValue(uniform, tUniform);
					HmiScenegraph::TUniform* itMesh = tMesh.add_uniform();
					*itMesh = tUniform;
				}
			}
		}
	}
}

void OutputPtx::setPtxTCamera(NodeData* childNode, HmiScenegraph::TNode& hmiNode) {
    // camera
	Q_UNUSED(childNode);
	HmiScenegraph::TCamera* camera = new HmiScenegraph::TCamera();
	camera->set_horizontalfov(0.7);
	camera->set_aspectratio(1.0);
	camera->set_nearplane(0.01);
	camera->set_farplane(100.0); 
	camera->set_projectiontype(HmiScenegraph::TECameraProjectionType::TECameraProjectionType_FOV);
	hmiNode.set_allocated_camera(camera);
}

//void OutputPtx::setMaterialTextureByNodeUniforms(NodeData* childNode, MaterialData& data) {
//	//data.setObjectName(childNode->getName() + "_" + data.getObjectName());
//	TextureData texData;
//	for (auto& textureData : data.getTextures()) {
//		std::string textureProperty = textureData.getUniformName();
//		std::vector<Uniform> Uniforms = childNode->getUniforms();
//		for (auto& un : Uniforms) {
//			if (un.getName() == textureProperty && un.getType() == UniformType::String && un.getValue().type() == typeid(std::string)) {
//				std::string textureName = std::any_cast<std::string>(un.getValue());
//				if (textureName != textureData.getName()) {
//					raco::guiData::MaterialManager::GetInstance().getTexture(textureName, texData);
//					texData.setUniformName(textureProperty);
//
//					data.clearTexture();
//					data.addTexture(texData);
//				}
//				return;
//			}
//		}
//	}
//}
// update ptx node
void OutputPtx::setPtxNode(NodeData* childNode, HmiScenegraph::TNode& hmiNode) {
    std::string nodeName = childNode->getName();
	hmiNode.set_name(nodeName);

	MeshData meshData;
	bool isMeshNode = raco::guiData::MeshDataManager::GetInstance().getMeshData(childNode->objectID(), meshData);

	if (!isMeshNode) {
		if (childNode->hasSystemData("scale")) {
			TVector3f* scale = new TVector3f();
			Vec3 scal = std::any_cast<Vec3>(childNode->getSystemData("scale"));
			scale->set_x(scal.x);
			scale->set_y(scal.y);
			scale->set_z(scal.z);
			if (nodeName == PTW_ROOT_NODE_NAME) {
				NodeScaleSize_ = scal.x;
			}
			hmiNode.set_allocated_scale(scale);
		}
		if (childNode->hasSystemData("rotation")) {
			TVector3f* rotation = new TVector3f();
			Vec3 rota = std::any_cast<Vec3>(childNode->getSystemData("rotation"));
			rotation->set_x(rota.x);
			rotation->set_y(rota.y);
			rotation->set_z(rota.z);
			hmiNode.set_allocated_rotation(rotation);
		}
		if (childNode->hasSystemData("translation")) {
			TVector3f* translation = new TVector3f();
			Vec3 tran = std::any_cast<Vec3>(childNode->getSystemData("translation"));
			translation->set_x(tran.x);
			translation->set_y(tran.y);
			translation->set_z(tran.z);
			hmiNode.set_allocated_translation(translation);
		}
	}else {// meshNode use default data
		TVector3f* scale = new TVector3f();
		scale->set_x(1.0);
		scale->set_y(1.0);
		scale->set_z(1.0);
		hmiNode.set_allocated_scale(scale);

		TVector3f* rotation = new TVector3f();
		rotation->set_x(0.0);
		rotation->set_y(0.0);
		rotation->set_z(0.0);
		hmiNode.set_allocated_rotation(rotation);

		TVector3f* translation = new TVector3f();
		translation->set_x(0.0);
		translation->set_y(0.0);
		translation->set_z(0.0);
		hmiNode.set_allocated_translation(translation);
	}

    // renderorder and childSortOrderRank
    hmiNode.set_renderorder(0);
	hmiNode.set_childsortorderrank(0);

	//MaterialData materialData;
	//if (raco::guiData::MaterialManager::GetInstance().getMaterialData(childNode->objectID(), materialData)) {
	//	setMaterialTextureByNodeUniforms(childNode, materialData);
	//	raco::guiData::MaterialManager::GetInstance().deleteMateialData(childNode->objectID());
	//	raco::guiData::MaterialManager::GetInstance().addMaterialData(childNode->objectID(), materialData);
	//}

    // mesh
	if (isMeshNode) {
		HmiScenegraph::TMesh mesh;
		setPtxTMesh(childNode, mesh);
		HmiScenegraph::TMesh* it = hmiNode.add_mesh();
		*it = mesh;
	}
}

void OutputPtx::setRootSRT(HmiScenegraph::TNode* hmiNode) {
	// scale
	TVector3f* scale = new TVector3f();
	scale->set_x(1.0);
	scale->set_y(1.0);
	scale->set_z(1.0);
	hmiNode->set_allocated_scale(scale);
	// rotation
	TVector3f* rotation = new TVector3f();
	rotation->set_x(0.0);
	rotation->set_y(0.0);
	rotation->set_z(0.0);
	hmiNode->set_allocated_rotation(rotation);
	// translation
	TVector3f* translation = new TVector3f();
	translation->set_x(0.0);
	translation->set_y(0.0);
	translation->set_z(0.0);
	hmiNode->set_allocated_translation(translation);
}


bool isUniformAndIndex(std::string propName, int& index) {
	std::string subLast = propName.substr(propName.length() - 2, propName.length());
	index = -1;
	if (subLast == ".x") {
		index = 0;
	} else if (subLast == ".y") {
		index = 1;
	} else if (subLast == ".z") {
		index = 2;
	} else if (subLast == ".w") {
		index = 3;
	}
	int r = propName.rfind("uniforms.");
	if (-1 == r) {
		return false;
	} else {
		return true;
	}
}


float getUniformValueByType(Uniform data, int index) {
	UniformType type = data.getType();
	switch (type) {
		case raco::guiData::Vec2f: {
			Vec2 temp = std::any_cast<Vec2>(data.getValue());
			switch (index) {
				case 0:
					return temp.x;
				case 1:
					return temp.y;
				default:
					return 0;
			}
		}
		case raco::guiData::Vec3f: {
			Vec3 temp = std::any_cast<Vec3>(data.getValue());
			switch (index) {
				case 0:
					return temp.x;
				case 1:
					return temp.y;
				case 2:
					return temp.z;
				default:
					return 0;
			}
		}
		case raco::guiData::Vec4f: {
			Vec4 temp = std::any_cast<Vec4>(data.getValue());
			switch (index) {
				case 0:
					return temp.x;
				case 1:
					return temp.y;
				case 2:
					return temp.z;
				case 3:
					return temp.w;
				default:
					return 0;
			}
		}
		case raco::guiData::Vec2i: {
			Vec2int temp = std::any_cast<Vec2int>(data.getValue());
			switch (index) {
				case 0:
					return temp.x;
				case 1:
					return temp.y;
				default:
					return 0;
			}
		}
		case raco::guiData::Vec3i: {
			Vec3int temp = std::any_cast<Vec3int>(data.getValue());
			switch (index) {
				case 0:
					return temp.x;
				case 1:
					return temp.y;
				case 2:
					return temp.z;
				default:
					return 0;
			}
		}
		case raco::guiData::Vec4i: {
			Vec4int temp = std::any_cast<Vec4int>(data.getValue());
			switch (index) {
				case 0:
					return temp.x;
				case 1:
					return temp.y;
				case 2:
					return temp.z;
				case 3:
					return temp.w;
				default:
					return 0;
			}
		}
	}
	return 0;
}

float setCurvesPropData(NodeData* pNode, std::string propName) {
	int index;
	bool isUniform = isUniformAndIndex(propName, index);
	if (index != -1 && !isUniform) {
		propName = propName.substr(0, propName.length()-2);
		auto systemProp = pNode->systemDataMapNewRef();
		auto it = systemProp.find(propName);
		if (it != systemProp.end()) {
			Vec3 valueVec = std::any_cast<Vec3>(pNode->getSystemData(propName));
			if (index == 0) {
				return valueVec.x;
			} else if (index == 1) {
				return valueVec.y;
			} else if (index == 2) {
				return valueVec.z;
			} else {
				return 0;
			}
		}
	} 
	if (isUniform) {
		NodeMaterial nodeMaterial;
		raco::guiData::MaterialManager::GetInstance().getNodeMaterial(pNode->objectID(), nodeMaterial);
		propName = delUniformNamePrefix(propName);
		if (index == -1) {
			for (auto un : nodeMaterial.getUniforms()) {
				if (un.getName() == propName) {
					return std::any_cast<double>(un.getValue());
				}
			}
		} else {
			for (auto un : nodeMaterial.getUniforms()) {
				if (un.getName() == propName.substr(0, propName.length() - 2)) {
					return getUniformValueByType(un, index);
				}
			}
		}
	} 
	return 0;
}

void searchRepeatPropInNode(NodeData* pNode, std::string name) {
	auto bindyMap = pNode->NodeExtendRef().curveBindingRef().bindingMap();
	
	std::vector<std::map<std::string, CurvesSingleProp>> curvesName;
	for (auto animation : bindyMap) {
		for (auto prop : animation.second) {
			std::string propName = prop.first;
			if (propName == name) {
				std::map<std::string, CurvesSingleProp> aniProp;
				CurvesSingleProp curveSingleProp;
				curveSingleProp.curveName = prop.second;
				//curveSingleProp.defaultData = setCurvesPropData(pNode, name);
				aniProp.emplace(animation.first, curveSingleProp);
				curvesName.push_back(aniProp);
			}
		}
	}

	if (curvesName.size() > 1) {
		std::vector<std::map<std::string, std::vector<std::map<std::string, CurvesSingleProp>>>> propCurves;
		std::map<std::string, std::vector<std::map<std::string, CurvesSingleProp>>> propCurve;
		double defultData = setCurvesPropData(pNode, name);
		for (int i = 0; i < curvesName.size(); ++i) {
			curvesName[i].begin()->second.defaultData = defultData;
		}
		auto ret = propCurve.emplace(name, curvesName);
		bool a = ret.second;
		propCurves.push_back(propCurve);
		std::string ID = pNode->objectID();
		auto nodeIt = pNodePropCurveNames_.find(ID);
		if (nodeIt != pNodePropCurveNames_.end()) {
			nodeIt->second.push_back(propCurve);
		}
		else {
			pNodePropCurveNames_.emplace(ID, propCurves);
		}
	}
}

void updatePNodePropCurveMap(NodeData* pNode) {
	std::set<std::string> peopertys;
	const std::map<std::string, std::map<std::string, std::string>>& bindyMap = pNode->NodeExtendRef().curveBindingRef().bindingMap();
	for (auto animation : bindyMap) {
		for (auto prop : animation.second) {
			std::string propName = prop.first;
			auto it = peopertys.find(propName);
			if (it == peopertys.end()) {
				peopertys.insert(propName);
				searchRepeatPropInNode(pNode, propName);
			}

		}
	}
}

void DEBUG(QString FILE, QString FUNCTION, int LINE) {
	QMessageBox msgBox;
	msgBox.setWindowTitle("Debug message box");
	QPushButton* okButton = msgBox.addButton("OK", QMessageBox::ActionRole);
	msgBox.setIcon(QMessageBox::Icon::Warning);

	QString info;
	info += FILE + QString(" ") + LINE + QString(" ") + FUNCTION;
	msgBox.setWindowTitle("OutputAssets.cpp Line：emplace error " + info);

	msgBox.setText(info);
	msgBox.exec();

	if (msgBox.clickedButton() == (QAbstractButton*)(okButton)) {
		//isPtwOutputError_ = true;
	}
}

void updateCurveAnimationMap(NodeData* pNode) {
	const std::map<std::string, std::map<std::string, std::string>>& bindyMap = pNode->NodeExtendRef().curveBindingRef().bindingMap();
	for (auto animation : bindyMap) {
		for (auto curve : animation.second) {
			auto it = curveNameAnimations_.find(curve.second);
			std::map<std::string, std::vector<AnimationsSingleCurve>> animations;
			std::vector<AnimationsSingleCurve> datas;
			AnimationsSingleCurve data;
			if (it == curveNameAnimations_.end()) {
				data.pNode = pNode;
				data.property = curve.first;
				datas.push_back(data);
				if (!animations.emplace(animation.first, datas).second) {
					DEBUG(__FILE__, __FUNCTION__, __LINE__);
				}
				if (!curveNameAnimations_.emplace(curve.second, animations).second) {
					DEBUG(__FILE__, __FUNCTION__, __LINE__);
				}
			} else {
				data.pNode = pNode;
				data.property = curve.first;
				// If the same curve is bound to the same node of different animations, it can only be added once. Step 4
				/*for (auto ani : it->second) {
					for (auto nodeTemp : ani.second) {
						if (nodeTemp.pNode->objectID() == pNode->objectID()) {
							return;
						}
					}
				}*/
				auto iter = it->second.find(animation.first);
				if (iter != it->second.end()
					&& (iter->second.at(iter->second.size() - 1)).pNode->objectID() != pNode->objectID()) {  // if animation is found
					iter->second.push_back(data);
				} else { // no animation found
					std::vector<AnimationsSingleCurve> datas;
					datas.push_back(data);
					if (!it->second.emplace(animation.first, datas).second) {
						DEBUG(__FILE__, __FUNCTION__, __LINE__);
					}
				}
			}
		}
	}
}

void cullingData() {
	for (auto it = curveNameAnimations_.begin(); it != curveNameAnimations_.end();) {
		if (it->second.size() < 2) {
			curveNameAnimations_.erase(it++);
		} else {
			it++;
		}
	}
}

void OutputPtx::writeNodePtx(NodeData* pNode, HmiScenegraph::TNode* parent) {
	if (!pNode){
		return;
	}
	HmiScenegraph::TNode hmiNode;
	setPtxNode(pNode, hmiNode);
	updateCurveAnimationMap(pNode);
	updatePNodePropCurveMap(pNode);
	HmiScenegraph::TNode* it = parent->add_child();
	*it = hmiNode;
	parent = const_cast<HmiScenegraph::TNode*>(&(parent->child(parent->child_size() - 1)));
	
	for (auto it = pNode->childMapRef().begin(); it != pNode->childMapRef().end(); ++it) {
		writeNodePtx(&(it->second), parent);
	}
}
TECompareFunction OutputPtx::matchCompareFunction(DepthCompare depthCmp) {
	TECompareFunction result = TECompareFunction::TECompareFunction_Never;
	switch (depthCmp) {
		case raco::guiData::DC_Disabled:
			result = TECompareFunction::TECompareFunction_Never;
			break;
		case raco::guiData::DC_GreaterThan:
			result = TECompareFunction::TECompareFunction_Greater;
			break;
		case raco::guiData::DC_GreaterOrEqualTo:
			result = TECompareFunction::TECompareFunction_Equal;
			break;
		case raco::guiData::DC_LessThan:
			result = TECompareFunction::TECompareFunction_Less;
			break;
		case raco::guiData::DC_LessThanOrEqualTo:
			result = TECompareFunction::TECompareFunction_LessEqual;
			break;
		case raco::guiData::DC_Equal:
			result = TECompareFunction::TECompareFunction_Equal;
			break;
		case raco::guiData::DC_NotEqual:
			result = TECompareFunction::TECompareFunction_NotEqual;
			break;
		case raco::guiData::DC_True:
			result = TECompareFunction::TECompareFunction_Always;
			break;
		case raco::guiData::DC_False:
			result = TECompareFunction::TECompareFunction_Never;
			break;
		default:
			break;
	}
	return result;
}

TEBlendFactor OutputPtx::matchBlendFactor(BlendFactor blendFactor) {
	TEBlendFactor result = TEBlendFactor::TEBlendFactor_Zero;
	switch (blendFactor) {
		case raco::guiData::Zero:
			result = TEBlendFactor::TEBlendFactor_Zero;
			break;
		case raco::guiData::One:
			result = TEBlendFactor::TEBlendFactor_One;
			break;
		case raco::guiData::SrcAlpha:
			result = TEBlendFactor::TEBlendFactor_SourceAlpha;
			break;
		case raco::guiData::OneMinusSrcAlpha:
			result = TEBlendFactor::TEBlendFactor_InverseSourceAlpha;
			break;
		case raco::guiData::DstAlpha:
			result = TEBlendFactor::TEBlendFactor_DestinationAlpha;
			break;
		case raco::guiData::OneMinusDstAlpha:
			result = TEBlendFactor::TEBlendFactor_InverseDestinationAlpha;
			break;
		case raco::guiData::SrcColor:
			result = TEBlendFactor::TEBlendFactor_SourceColor;
			break;
		case raco::guiData::OneMinusSrcColor:
			result = TEBlendFactor::TEBlendFactor_InverseSourceColor;
			break;
		case raco::guiData::DstColor:
			result = TEBlendFactor::TEBlendFactor_DestinationColor;
			break;
		case raco::guiData::OneMinusDstColor:
			result = TEBlendFactor::TEBlendFactor_InverseDestinationColor;
			break;
		case raco::guiData::ConstColor:
			result = TEBlendFactor::TEBlendFactor_ConstantColor;
			break;
		case raco::guiData::OneMinusConstColor:
			result = TEBlendFactor::TEBlendFactor_InverseConstantColor;
			break;
		case raco::guiData::ConstAlpha:
			result = TEBlendFactor::TEBlendFactor_ConstantAlpha;
			break;
		case raco::guiData::OneMinusConstAlpha:
			result = TEBlendFactor::TEBlendFactor_InverseConstantAlpha;
			break;
		case raco::guiData::AlphaSaturated:
			result = TEBlendFactor::TEBlendFactor_SourceAlphaSaturate;
			break;
		default:
			break;
	}
	return result;
}
TEBlendOperation OutputPtx::matchBlendOperation(BlendOperation blendOpera) {
	TEBlendOperation result = TEBlendOperation::TEBlendOperation_None;
	switch (blendOpera) {
		case raco::guiData::BO_None:
			result = TEBlendOperation::TEBlendOperation_None;
			break;
		case raco::guiData::BO_Add:
			result = TEBlendOperation::TEBlendOperation_Add;
			break;
		case raco::guiData::BO_Subtract:
			result = TEBlendOperation::TEBlendOperation_Subtract;
			break;
		case raco::guiData::BO_ReverseSub:
			result = TEBlendOperation::TEBlendOperation_ReverseSubtract;
			break;
		case raco::guiData::BO_Min:
			result = TEBlendOperation::TEBlendOperation_None;
			break;
		case raco::guiData::BO_Max:
			result = TEBlendOperation::TEBlendOperation_None;
			break;
		default:
			break;
	}
	return result;
}

TEFace OutputPtx::matchFaceCulling(Culling cull) {
	TEFace result = TEFace::TEFace_None;
	switch (cull) {
		case raco::guiData::CU_Front:
			result = TEFace::TEFace_Front;
			break;
		case raco::guiData::CU_Back:
			result = TEFace::TEFace_Back;
			break;
		case raco::guiData::CU_FrontAndBack:
			result = TEFace::TEFace_FrontAndBack;
			break;
		case raco::guiData::CU_None:
			result = TEFace::TEFace_None;
			break;
		default:
			break;
	}
	return result;
}

TEWinding OutputPtx::matchWinding(WindingType wind) {
	TEWinding result = TEWinding::TEWinding_ClockWise;
	switch (wind) {
		case raco::guiData::M_TEWinding_ClockWise:
			result = TEWinding::TEWinding_ClockWise;
			break;
		case raco::guiData::M_TEWinding_CounterClockWise:
			result = TEWinding::TEWinding_CounterClockWise;
			break;
		default:
			break;
	}
	return result;
}

void OutputPtx::setMaterialDefaultRenderMode(RenderMode& renderMode, HmiScenegraph::TRenderMode* rRenderMode) {
	// winding and culling
	rRenderMode->set_winding(TEWinding::TEWinding_CounterClockWise);
	rRenderMode->set_culling(TEFace::TEFace_None);
	// tblending
	HmiScenegraph::TBlendMode* tblending = new HmiScenegraph::TBlendMode();
	Blending blending = renderMode.getBlending();
	// operation
	tblending->set_blendoperationcolor(TEBlendOperation::TEBlendOperation_Add);
	tblending->set_blendoperationalpha(TEBlendOperation::TEBlendOperation_Add);
	// factor
	tblending->set_sourcealphafactor(TEBlendFactor::TEBlendFactor_One);
	tblending->set_sourcecolorfactor(TEBlendFactor::TEBlendFactor_SourceAlpha);
	tblending->set_destinationalphafactor(TEBlendFactor::TEBlendFactor_InverseSourceAlpha);
	tblending->set_destinationcolorfactor(TEBlendFactor::TEBlendFactor_InverseSourceAlpha);

	rRenderMode->set_allocated_blending(tblending);
	rRenderMode->set_depthcompare(TECompareFunction::TECompareFunction_Always);

	rRenderMode->set_depthwrite(false);
	// ColorWrite
	HmiScenegraph::TRenderMode_TColorWrite* tColorWrite = new HmiScenegraph::TRenderMode_TColorWrite();
	ColorWrite colorWrite = renderMode.getColorWrite();
	tColorWrite->set_alpha(true);
	tColorWrite->set_blue(true);
	tColorWrite->set_green(true);
	tColorWrite->set_red(true);
	rRenderMode->set_allocated_colorwrite(tColorWrite);
}

void OutputPtx::setMaterialRenderMode(RenderMode& renderMode, HmiScenegraph::TRenderMode* rRenderMode) {
	// winding and culling
	rRenderMode->set_winding(matchWinding(renderMode.getWindingType()));
	rRenderMode->set_culling(matchFaceCulling(renderMode.getCulling()));
	// tblending
	HmiScenegraph::TBlendMode* tblending = new HmiScenegraph::TBlendMode();
	Blending blending = renderMode.getBlending();
	// operation
	tblending->set_blendoperationcolor(matchBlendOperation(blending.getBlendOperationColor()));
	tblending->set_blendoperationalpha(matchBlendOperation(blending.getBlendOperationAlpha()));
	// factor
	tblending->set_sourcealphafactor(matchBlendFactor(blending.getSrcAlphaFactor()));
	tblending->set_sourcecolorfactor(matchBlendFactor(blending.getSrcColorFactor()));
	tblending->set_destinationalphafactor(matchBlendFactor(blending.getDesAlphaFactor()));
	tblending->set_destinationcolorfactor(matchBlendFactor(blending.getDesColorFactor()));

	rRenderMode->set_allocated_blending(tblending);
	rRenderMode->set_depthcompare(matchCompareFunction(renderMode.getDepthCompare()));

	rRenderMode->set_depthwrite(renderMode.getDepthWrite());
	// ColorWrite
	HmiScenegraph::TRenderMode_TColorWrite* tColorWrite = new HmiScenegraph::TRenderMode_TColorWrite();
	ColorWrite colorWrite = renderMode.getColorWrite();
	// If the colorwrite property value is false, the content will not be displayed in Hmi.
	tColorWrite->set_alpha(true);
	tColorWrite->set_blue(true);
	tColorWrite->set_green(true);
	tColorWrite->set_red(true);
	rRenderMode->set_allocated_colorwrite(tColorWrite);
}

void OutputPtx::uniformTypeValue(Uniform data, HmiScenegraph::TUniform& tUniform) {
	tUniform.set_name(data.getName());
	TNumericValue* tNumericValue = new TNumericValue();
	UniformType dataType = data.getType();
	switch (dataType) {
		case raco::guiData::Null:
			// Do not have
			break;
		case raco::guiData::Bool: {
			uint32_t temp = std::any_cast<bool>(data.getValue());
			tNumericValue->set_uint(temp);
			tUniform.set_allocated_value(tNumericValue);
			tUniform.set_type(TENumericType::TENumericType_unsignedInteger);
			break;
		}
		case raco::guiData::Int: {
			int temp = std::any_cast<int>(data.getValue());
			tNumericValue->set_int_(temp);
			tUniform.set_allocated_value(tNumericValue);
			tUniform.set_type(TENumericType::TENumericType_unsignedInteger);
			break;
		}
		case raco::guiData::Double: {
			float temp = std::any_cast<double>(data.getValue());
			tNumericValue->set_float_(temp);
			tUniform.set_allocated_value(tNumericValue);
			tUniform.set_type(TENumericType::TENumericType_float);
			break;
		}
		case raco::guiData::String:
			// Do not have
			break;
		case raco::guiData::Ref:
			// Do not have
			break;
		case raco::guiData::Table:
			// Do not have
			break;
		case raco::guiData::Vec2f: {
			Vec2 temp = std::any_cast<Vec2>(data.getValue());
			TVector2f* tVec2f = new TVector2f();
			tVec2f->set_x(temp.x);
			tVec2f->set_y(temp.y);
			tNumericValue->set_allocated_vec2f(tVec2f);
			tUniform.set_allocated_value(tNumericValue);
			tUniform.set_type(TENumericType::TENumericType_floatVector2);
			break;
		}
		case raco::guiData::Vec3f: {
			Vec3 temp = std::any_cast<Vec3>(data.getValue());
			TVector3f* tVec3f = new TVector3f();
			tVec3f->set_x(temp.x);
			tVec3f->set_y(temp.y);
			tVec3f->set_z(temp.z);
			tNumericValue->set_allocated_vec3f(tVec3f);
			tUniform.set_allocated_value(tNumericValue);
			tUniform.set_type(TENumericType::TENumericType_floatVector3);
			break;
		}
		case raco::guiData::Vec4f: {
			Vec4 temp = std::any_cast<Vec4>(data.getValue());
			TVector4f* tVec4f = new TVector4f();
			tVec4f->set_x(temp.x);
			tVec4f->set_y(temp.y);
			tVec4f->set_z(temp.z);
			tVec4f->set_w(temp.w);
			tNumericValue->set_allocated_vec4f(tVec4f);
			tUniform.set_allocated_value(tNumericValue);
			tUniform.set_type(TENumericType::TENumericType_floatVector4);
			break;
		}
		case raco::guiData::Vec2i: {
			Vec2int temp = std::any_cast<Vec2int>(data.getValue());
			TVector2i* tVec2i = new TVector2i();
			tVec2i->set_x(temp.x);
			tVec2i->set_y(temp.y);
			tNumericValue->set_allocated_vec2i(tVec2i);
			tUniform.set_allocated_value(tNumericValue);
			tUniform.set_type(TENumericType::TENumericType_intVector2);
			break;
		}
		case raco::guiData::Vec3i: {
			Vec3int temp = std::any_cast<Vec3int>(data.getValue());
			TVector3i* tVec3i = new TVector3i();
			tVec3i->set_x(temp.x);
			tVec3i->set_y(temp.y);
			tVec3i->set_z(temp.z);
			tNumericValue->set_allocated_vec3i(tVec3i);
			tUniform.set_allocated_value(tNumericValue);
			tUniform.set_type(TENumericType::TENumericType_intVector3);
			break;
		}
		case raco::guiData::Vec4i: {
			Vec4int temp = std::any_cast<Vec4int>(data.getValue());
			TVector4i* tVec4i = new TVector4i();
			tVec4i->set_x(temp.x);
			tVec4i->set_y(temp.y);
			tVec4i->set_z(temp.z);
			tVec4i->set_w(temp.w);
			tNumericValue->set_allocated_vec4i(tVec4i);
			tUniform.set_allocated_value(tNumericValue);
			tUniform.set_type(TENumericType::TENumericType_intVector4);
			break;
		}
		case raco::guiData::Struct:
			// Do not have
			break;
		default:
			break;
	}
}

TETextureFilter OutputPtx::matchFilter(Filter filter) {
	TETextureFilter result = TETextureFilter::TETextureFilter_Linear;
	switch (filter) {
		case raco::guiData::Nearest:
			result = TETextureFilter::TETextureFilter_Nearest;
			break;
		case raco::guiData::Linear:
			result = TETextureFilter::TETextureFilter_Linear;
			break;
		case raco::guiData::NearestMipMapNearest:
			result = TETextureFilter::TETextureFilter_NearestMipMapNearest;
			break;
		case raco::guiData::NearestMipMapLinear:
			result = TETextureFilter::TETextureFilter_NearestMipMapLinear;
			break;
		case raco::guiData::LinearMipMapNearest:
			result = TETextureFilter::TETextureFilter_LinearMipMapNearest;
			break;
		case raco::guiData::LinearMipMapLinear:
			result = TETextureFilter::TETextureFilter_LinearMipMapLinear;
			break;
		default:
			break;
	}

	return result;
}

TETextureWrapMode OutputPtx::matchWrapMode(WrapMode mode) {
	TETextureWrapMode result = TETextureWrapMode::TETextureWrapMode_ClampToEdge;
	switch (mode) {
		case raco::guiData::Clamp:
			result = TETextureWrapMode::TETextureWrapMode_ClampToEdge;
			break;
		case raco::guiData::Repeat:
			result = TETextureWrapMode::TETextureWrapMode_Repeat;
			break;
		case raco::guiData::Mirror:
			result = TETextureWrapMode::TETextureWrapMode_MirroredRepeat;
			break;
		default:
			break;
	}
	return result;
}

bool OutputPtx::mkdir(QString path) {
	QString dest = path;
	QDir dir;
	if (!dir.exists(dest)) {
		dir.mkpath(dest);
		return true;
	} else {
		return false;
	}
}

bool OutputPtx::isStored(std::string name, std::set<std::string>& nameArr) {
	auto it = nameArr.find(name);
	if (it == nameArr.end()) {
		nameArr.emplace(name);
		return false;
	}
	return true;
}

std::string OutputPtx::getShaderPtxNameByShaderName(std::string name) {
	std::map<std::string, Shader> shaderMap = raco::guiData::MaterialManager::GetInstance().getShaderDataMap();
	auto it = shaderMap.find(name);
	if (it == shaderMap.end()) {
		return std::string("");
	} else {
		return it->second.getPtxShaderName();
	}
}

void OutputPtx::messageBoxError(std::string materialName, int type) {
	if (isPtxOutputError_) {
		return;
	}

	QMessageBox customMsgBox;
	customMsgBox.setWindowTitle("Warning message box");
	QPushButton* okButton = customMsgBox.addButton("OK", QMessageBox::ActionRole);
	//QPushButton* cancelButton = customMsgBox.addButton(QMessageBox::Cancel);
	customMsgBox.setIcon(QMessageBox::Icon::Warning);
	QString text;
	if (1 == type) {
		text = QString::fromStdString(materialName) + "\" has an empty texture !";
		text = "Warning: Material \"" + text;
	} else if (2 == type) {
		text = QString::fromStdString(materialName) + "\" generated by mesh node has the same name. !";
		text = "Warning: The private material \"" + text;
	}
	customMsgBox.setText(text);
	customMsgBox.exec();

	if (customMsgBox.clickedButton() == (QAbstractButton *)(okButton)) {
		isPtxOutputError_ = true;
	}
}

bool updateHasOpacityMaterial(std::vector<Uniform> uniforms, std::string materialName) {
	for (auto& uniform : uniforms) {
		if (uniform.getName() == PTW_FRAG_CTRL_OPACITY) {
			auto it = HasOpacityMaterials_.find(materialName);
			if (it == HasOpacityMaterials_.end()) {
				HasOpacityMaterials_.emplace(materialName);
			}
			return true;
		}
	}
	return false;
}

void OutputPtx::writeMaterial2MaterialLib(HmiScenegraph::TMaterialLib* materialLibrary) {
	std::map<std::string, MaterialData> materialMap = raco::guiData::MaterialManager::GetInstance().getMaterialDataMap();
	std::set<std::string> setNameArr;
	for (auto& material : materialMap) {
		MaterialData data = material.second;
		HmiScenegraph::TMaterial tMaterial;

		// whether it has been stored?
		if (isStored(data.getObjectName(), setNameArr)) {
			continue;
		}
		// name
		tMaterial.set_name(data.getObjectName());

		// RenderMode
		HmiScenegraph::TRenderMode* rRenderMode = new HmiScenegraph::TRenderMode();
		RenderMode renderMode = data.getRenderMode();
		// setRenderMode
		setMaterialRenderMode(renderMode, rRenderMode);
		tMaterial.set_allocated_rendermode(rRenderMode);

		// shaderReference
		std::string shaderPtxName = getShaderPtxNameByShaderName(data.getShaderRef());
		tMaterial.set_shaderreference(shaderPtxName);

		for (auto& textureData : data.getTextures()) {
			HmiScenegraph::TTexture tTextture;
			if (textureData.getName() == "empty") {
				messageBoxError(data.getObjectName(), 1);
			}
			tTextture.set_name(textureData.getName());
			tTextture.set_bitmapreference(textureData.getBitmapRef());
			tTextture.set_minfilter(matchFilter(textureData.getMinFilter()));
			tTextture.set_magfilter(matchFilter(textureData.getMagFilter()));
			tTextture.set_anisotropicsamples(textureData.getAnisotropicSamples());
			tTextture.set_wrapmodeu(matchWrapMode(textureData.getWrapModeU()));
			tTextture.set_wrapmodev(matchWrapMode(textureData.getWrapModeV()));
			tTextture.set_uniformname(textureData.getUniformName());
			HmiScenegraph::TTexture* textureIt = tMaterial.add_texture();
			*textureIt = tTextture;
		}
		// uniforms
		for (auto& uniform : data.getUniforms()) {
			HmiScenegraph::TUniform tUniform;
			uniformTypeValue(uniform, tUniform);
			HmiScenegraph::TUniform* tUniformIt = tMaterial.add_uniform();
			*tUniformIt = tUniform;
		}

		updateHasOpacityMaterial(data.getUniforms(), data.getObjectName());

		HmiScenegraph::TMaterial* materialIt = materialLibrary->add_material();
		*materialIt = tMaterial;
	}
}

void OutputPtx::writeShaders2MaterialLib(QString& filePath, QString& oldPath, HmiScenegraph::TMaterialLib* materialLibrary) {
	mkdir(filePath + "/shaders");
	std::map<std::string, Shader> shaderMap = raco::guiData::MaterialManager::GetInstance().getShaderDataMap();
	std::set<std::string> shaderNameArr;
	for (auto& shader : shaderMap) {
		// whether it has been stored?
		if (isStored(shader.second.getPtxShaderName(), shaderNameArr)) {
			continue;
		}

		HmiScenegraph::TShader tShader;
		tShader.set_name(shader.second.getPtxShaderName());

		QString shaderPath = oldPath + "/" + QString::fromStdString(shader.second.getVertexShader());
		QFileInfo fileinfo(shaderPath);
		QString shadersPathName = "shaders/" + fileinfo.fileName();
		qDebug() << shadersPathName;
		QString desPath = filePath + "/" + shadersPathName;
		qDebug() << desPath;
		if (!QFile::copy(shaderPath, desPath)) {
			qDebug() << " copy [" << fileinfo.fileName() << " ] failed!";
		}
		tShader.set_vertexshader(shadersPathName.toStdString());
		shaderPath = oldPath + "/" + QString::fromStdString(shader.second.getFragmentShader());
		fileinfo = QFileInfo(shaderPath);
		shadersPathName = "shaders/" + fileinfo.fileName();
		desPath = filePath + "/" + shadersPathName;
		if (!QFile::copy(shaderPath, desPath)) {
			qDebug() << " copy [" << fileinfo.fileName() << " ] failed!";
		}
		tShader.set_fragmentshader(shadersPathName.toStdString());
		HmiScenegraph::TShader* tShaderIt = materialLibrary->add_shader();
		*tShaderIt = tShader;
	}
}

void OutputPtx::writeBitmap2MaterialLib(QString& filePath, QString& oldPath, HmiScenegraph::TMaterialLib* materialLibrary) {
	mkdir(filePath + "./bitmaps");
	std::map<std::string, Bitmap> bitMaps = raco::guiData::MaterialManager::GetInstance().getBitmapDataMap();
	std::set<std::string> bitmapNameArr;
	for (auto& bitData : bitMaps) {
		// whether it has been stored?
		if (isStored(bitData.second.getName(), bitmapNameArr)) {
			continue;
		}
		HmiScenegraph::TBitmap tBitMap;
		if (bitData.second.getName() != "" && bitData.second.getResource() != "") {
			tBitMap.set_name(bitData.second.getName());
			QString bitmapPath = oldPath + "/" + QString::fromStdString(bitData.second.getResource());
			QFileInfo fileinfo(bitmapPath);
			QString bitmapPathName = "bitmaps/" + fileinfo.fileName();
			QString desPath = filePath + "/" + bitmapPathName;
			if (!QFile::copy(bitmapPath, desPath)) {
				qDebug() << " copy [" << fileinfo.fileName() << " ] failed!";
			}

			tBitMap.set_resource(bitmapPathName.toStdString());
			tBitMap.set_generatemipmaps(bitData.second.getGenerateMipmaps());
			HmiScenegraph::TBitmap* tBitMapIt = materialLibrary->add_bitmap();
			*tBitMapIt = tBitMap;
		}
	}
}

void OutputPtx::writeMaterialLib2Ptx(QString& filePath, QString& oldPath, HmiScenegraph::TMaterialLib* materialLibrary) {
	// add materials
	writeMaterial2MaterialLib(materialLibrary);

	// add shaders
	writeShaders2MaterialLib(filePath, oldPath, materialLibrary);

	// add bitmaps
	writeBitmap2MaterialLib(filePath, oldPath, materialLibrary);
}

bool OutputPtx::writeProgram2Ptx(std::string filePathStr, QString oldPath) {
	filePathStr = filePathStr.substr(0, filePathStr.find(".rca"));
	QString filePath = QString::fromStdString(filePathStr);
	mkdir(filePath);

	QFile file(filePath + "/scene.ptx");
	if (!file.open(QIODevice::ReadWrite)) {
		return false;
	}
	file.resize(0);
	nodeWithMaterial_.clear();
	curveNameAnimations_.clear();
	pNodePropCurveNames_.clear();
	HasOpacityMaterials_.clear();
	// root
	NodeData* rootNode = &(raco::guiData::NodeDataManager::GetInstance().root());
	HmiScenegraph::TScene scene;

    HmiScenegraph::TNode* tRoot = new HmiScenegraph::TNode();
	tRoot->set_name(PTX_SCENE_NAME.toStdString());
	setRootSRT(tRoot);
	int rootChildIndex = 0;
	NodeScaleSize_ = 0;
    for (auto& child : rootNode->childMapRef()) {
		NodeData* childNode = &(child.second);
		if (-1 != childNode->getName().find("PerspectiveCamera")) {
			continue;
		}

		writeNodePtx(childNode, tRoot);

	}
    scene.set_allocated_root(tRoot);

	// materiallibrary
	HmiScenegraph::TMaterialLib* materialLibrary = new HmiScenegraph::TMaterialLib();
	writeMaterialLib2Ptx(filePath, oldPath, materialLibrary);
	scene.set_allocated_materiallibrary(materialLibrary);

    std::string output;
	google::protobuf::TextFormat::PrintToString(scene, &output);

    QByteArray byteArray = QByteArray::fromStdString(output);
	file.write(byteArray);
	file.close();

	if (isPtxOutputError_) {
		QFile::remove(filePath + "/scene.ptx");
		isPtxOutputError_ = false;
		return false;
	}
	return true;
}

void addAnimationSwitchType2Operation(TOperation* operation) {
	operation->set_operator_(TEOperatorType_Switch);
	operation->add_datatype(TEDataType_Identifier);
	// operand -> key
	TIdentifier* key = new TIdentifier;
	key->set_valuestring(PTW_USED_ANIMATION_NAME);
	// operand -> provider
	TDataProvider* provider = new TDataProvider;
	provider->set_source(TEProviderSource_ExtModelValue);
	// operation.operand add key,provider
	auto operand = operation->add_operand();
	operand->set_allocated_key(key);
	operand->set_allocated_provider(provider);
}

void addAnimationSwitchCase2Operation(TOperation* operation, std::string anName, bool isDefault = false) {
	if (!isDefault) {
		operation->add_datatype(TEDataType_Identifier);
		operation->add_datatype(TEDataType_Float);

		auto operandAn = operation->add_operand();
		TDataProvider* providerAn = new TDataProvider;
		TVariant* variantAn = new TVariant;
		TIdentifier* identifierAn = new TIdentifier;
		identifierAn->set_valuestring(anName);
		variantAn->set_allocated_identifier(identifierAn);
		variantAn->set_identifiertype(TEIdentifierType_ParameterValue);
		providerAn->set_allocated_variant(variantAn);
		operandAn->set_allocated_provider(providerAn);

		auto operandIn = operation->add_operand();
		TIdentifier* key = new TIdentifier;
		key->set_valuestring(anName + PTW_SUFFIX_CURVE_INETERAL);
		operandIn->set_allocated_key(key);
		TDataProvider* providerIn = new TDataProvider;
		providerIn->set_source(TEProviderSource_IntModelValue);
		operandIn->set_allocated_provider(providerIn);
	} else {
		operation->add_datatype(TEDataType_Float);

		auto operandIn = operation->add_operand();
		TIdentifier* key = new TIdentifier;
		key->set_valuestring(anName + PTW_SUFFIX_CURVE_INETERAL);
		operandIn->set_allocated_key(key);
		TDataProvider* providerIn = new TDataProvider;
		providerIn->set_source(TEProviderSource_IntModelValue);
		operandIn->set_allocated_provider(providerIn);
	}
}

// Only when multiple animations bind one curve
void OutputPtw::switchAnimation(HmiWidget::TWidget* widget) {
	for (auto curve : curveNameAnimations_) {
		if (curve.second.size() > 1) {
			PTWSwitchData switchData;
			switchData.outPutKey = curve.first + "_interal_switch";
			switchData.dataType = TEDataType_Float;
			switchData.condition.key = PTW_USED_ANIMATION_NAME;
			switchData.condition.src = TEProviderSource_ExtModelValue;

			// add switch case to operation
			for (auto anName : curve.second) {
				Case caseData;
				caseData.Identifier = anName.first;
				caseData.IdentifierType = TEIdentifierType_ParameterValue;
				caseData.key = anName.first + PTW_SUFFIX_CURVE_INETERAL;
				caseData.src = TEProviderSource_IntModelValue;
				switchData.caseArr.push_back(caseData);
			}
			auto internalModelValue = widget->add_internalmodelvalue();
			assetsFun_.PTWSwitch(internalModelValue, switchData);
		}
	}
}

void OutputPtw::ConvertAnimationInfo(HmiWidget::TWidget* widget) {
	std::string animation_interal;
	auto animationList = raco::guiData::animationDataManager::GetInstance().getAnitnList();
	if (0 == animationList.size()) {
		messageBoxError("", 5);
	}

	// add first animation name
	HmiWidget::TExternalModelParameter* externalModelValue = widget->add_externalmodelvalue();
	TIdentifier* key = new TIdentifier;
	key->set_valuestring(PTW_USED_ANIMATION_NAME);
	externalModelValue->set_allocated_key(key);
	TVariant* variant = new TVariant;
	std::string* asciistring = new std::string(animationList.begin()->first);
	variant->set_allocated_asciistring(asciistring);
	externalModelValue->set_allocated_variant(variant);


	for (auto animation : animationList) {
		auto internalModelValue = widget->add_internalmodelvalue();
		TIdentifier* key_int = new TIdentifier;
		animation_interal = animation.first + PTW_SUFFIX_CURVE_INETERAL;
		key_int->set_valuestring(animation_interal);
		internalModelValue->set_allocated_key(key_int);
		TDataBinding* binding = new TDataBinding;
		TDataProvider* provider = new TDataProvider;
		TOperation* operation = new TOperation;

		operation->set_operator_(TEOperatorType_Mul);
		operation->add_datatype(TEDataType_Float);
		operation->add_datatype(TEDataType_Float);

		auto operand1 = operation->add_operand();
		TIdentifier* key = new TIdentifier;
		TDataProvider* provider1 = new TDataProvider;
		if (addTrigger_) {
			key->set_valuestring(animation.first + PTW_SUFFIX_ANIMATION_DOMAIN);
			provider1->set_source(TEProviderSource_IntModelValue);
		} else {
			key->set_valuestring(animation.first + "_extenal");
			provider1->set_source(TEProviderSource_ExtModelValue);
		}
		operand1->set_allocated_key(key);
		operand1->set_allocated_provider(provider1);

		auto operand2 = operation->add_operand();
		TDataProvider* provider2 = new TDataProvider;
		TVariant* variant1 = new TVariant;
		TNumericValue* numeric = new TNumericValue;
		numeric->set_float_(float(animation.second.GetEndTime() - animation.second.GetStartTime()));
		variant1->set_allocated_numeric(numeric);
		provider2->set_allocated_variant(variant1);

		operand2->set_allocated_provider(provider2);
		provider->set_allocated_operation(operation);
		binding->set_allocated_provider(provider);
		internalModelValue->set_allocated_binding(binding);

		addAnimationDomain(widget, animation.first);
	}
	switchAnimation(widget);
}

void addUsedAnimationSwitchType2Operation(TOperation* operation) {
	operation->set_operator_(TEOperatorType_Switch);
	operation->add_datatype(TEDataType_Identifier);
	// operand -> key
	TIdentifier* key = new TIdentifier;
	key->set_valuestring(PTW_USED_ANIMATION_NAME);
	// operand -> provider
	TDataProvider* provider = new TDataProvider;
	provider->set_source(TEProviderSource_ExtModelValue);
	// operation.operand add key,provider
	auto operand = operation->add_operand();
	operand->set_allocated_key(key);
	operand->set_allocated_provider(provider);
}

void addDomainSwitchCase2Operation(TOperation* operation, std::string anName, bool isDefault = false) {
	if (!isDefault) {
		operation->add_datatype(TEDataType_Identifier);
		operation->add_datatype(TEDataType_Float);

		auto operandAn = operation->add_operand();
		TDataProvider* providerAn = new TDataProvider;
		TVariant* variantAn = new TVariant;
		TIdentifier* identifierAn = new TIdentifier;
		identifierAn->set_valuestring(anName);
		variantAn->set_allocated_identifier(identifierAn);
		variantAn->set_identifiertype(TEIdentifierType_ParameterValue);
		providerAn->set_allocated_variant(variantAn);
		operandAn->set_allocated_provider(providerAn);

		auto operandIn = operation->add_operand();
		TIdentifier* key = new TIdentifier;
		key->set_valuestring("domain");
		operandIn->set_allocated_key(key);
		TDataProvider* providerIn = new TDataProvider;
		providerIn->set_source(TEProviderSource_IntModelValue);
		operandIn->set_allocated_provider(providerIn);
	} else {
		operation->add_datatype(TEDataType_Float);

		auto operandIn = operation->add_operand();
		TDataProvider* providerAn = new TDataProvider;
		TVariant* variantAn = new TVariant;

		TNumericValue* nuneric = new TNumericValue;
		nuneric->set_float_(0.0);
		variantAn->set_allocated_numeric(nuneric);
		providerAn->set_allocated_variant(variantAn);
		operandIn->set_allocated_provider(providerAn);
	}
}

void OutputPtw::addAnimationDomain(HmiWidget::TWidget* widget, std::string animationName) {
	// internalModelValue
	auto internalModelValue = widget->add_internalmodelvalue();
	TIdentifier* key = new TIdentifier;
	key->set_valuestring(animationName + PTW_SUFFIX_ANIMATION_DOMAIN);
	internalModelValue->set_allocated_key(key);
	// add binding
	TDataBinding* binding = new TDataBinding;
	TDataProvider* provider = new TDataProvider;
	TOperation* operation = new TOperation;

	addUsedAnimationSwitchType2Operation(operation);
	addDomainSwitchCase2Operation(operation, animationName);
	addDomainSwitchCase2Operation(operation, animationName, true);

	provider->set_allocated_operation(operation);
	binding->set_allocated_provider(provider);
	internalModelValue->set_allocated_binding(binding);
}

void addCompositeAnimation(HmiWidget::TWidget* widget) {
	// compositeAnimation
	auto compositeAnimation = widget->add_compositeanimation();
	TIdentifier* compositeidentifier = new TIdentifier;
	compositeidentifier->set_valuestring("compositeAnimation");
	compositeAnimation->set_allocated_compositeidentifier(compositeidentifier);

	// returnValue
	// returnValue.key
	auto returnValue = compositeAnimation->add_returnvalue();
	TIdentifier* key = new TIdentifier;
	key->set_valuestring("compositeAnimation.output");
	returnValue->set_allocated_key(key);

	// returnValue.animation
	auto animation = returnValue->add_animation();
	// returnValue.animation.Identifier
	auto anIdentifier = new TIdentifier;
	anIdentifier->set_valuestring("animation");
	animation->set_allocated_identifier(anIdentifier);

	// returnValue.animation.WidgetAnimation
	auto widgetAnimation = new HmiWidget::TWidgetAnimation;
	auto startValue = new TNumericValue;
	startValue->set_float_(0.0);
	widgetAnimation->set_allocated_startvalue(startValue);
	auto endValue = new TNumericValue;
	endValue->set_float_(1.0);
	widgetAnimation->set_allocated_endvalue(endValue);
	widgetAnimation->set_durationvalue(8000);
	widgetAnimation->set_interpolator(TEAnimationInterpolator::TEAnimationInterpolator_Linear);
	widgetAnimation->set_returntype(TEDataType::TEDataType_Float);
	widgetAnimation->set_loopcount(0);
	widgetAnimation->set_updateinterval(33);
	animation->set_allocated_widgetanimation(widgetAnimation);

	// returnValue.animation.trigger
	auto triggerIter = animation->add_trigger();
	triggerIter->set_action(HmiWidget::TEAnimationSlot::TEAnimationSlot_SlotAnimationStart);
	returnValue->set_returntype(TEDataType::TEDataType_Float);
}

void addTrigger(HmiWidget::TWidget* widget) {
	auto trigger = widget->add_trigger();
	TIdentifier* triggeridentifier = new TIdentifier;
	triggeridentifier->set_valuestring("StartStopCompositeAnimation");
	trigger->set_allocated_identifier(triggeridentifier);

	auto conditionalTrigger = new HmiWidget::TConditionalTrigger;
	auto condition = new TDataBinding;
	auto key = new TIdentifier;
	key->set_valuestring("Play");
	condition->set_allocated_key(key);

	TDataProvider* provider = new TDataProvider;
	provider->set_source(TEProviderSource_ExtModelValue);
	condition->set_allocated_provider(provider);

	conditionalTrigger->set_allocated_condition(condition);

	auto commond = new HmiWidget::TCommand;
	auto antrigger = new HmiWidget::TAnimationTrigger;
	auto animation = new TIdentifier;
	animation->set_valuestring("compositeAnimation");
	antrigger->set_allocated_animation(animation);
	antrigger->set_action(HmiWidget::TEAnimationAction::TEAnimationAction_Start);
	commond->set_allocated_animationtrigger(antrigger);
	conditionalTrigger->set_allocated_command(commond);
	
	auto elseCommond = new HmiWidget::TCommand;
	auto antriggerElse = new HmiWidget::TAnimationTrigger;
	auto animationElse = new TIdentifier;
	animationElse->set_valuestring("compositeAnimation");
	antriggerElse->set_allocated_animation(animationElse);
	antriggerElse->set_action(HmiWidget::TEAnimationAction::TEAnimationAction_Stop);
	elseCommond->set_allocated_animationtrigger(antriggerElse);
	conditionalTrigger->set_allocated_elsecommand(elseCommond);

	trigger->set_allocated_conditionaltrigger(conditionalTrigger);
}

void addPlayDomain(HmiWidget::TWidget* widget) {
	auto play = widget->add_externalmodelvalue();
	TIdentifier* playKey = new TIdentifier;
	playKey->set_valuestring("Play");
	play->set_allocated_key(playKey);
	TVariant* variant = new TVariant;
	variant->set_bool_(true);
	play->set_allocated_variant(variant);

	auto domain = widget->add_internalmodelvalue();
	TIdentifier* domainKey = new TIdentifier;
	domainKey->set_valuestring("domain");
	domain->set_allocated_key(domainKey);

	auto binding = new TDataBinding;
	auto bindKey = new TIdentifier;
	bindKey->set_valuestring("compositeAnimation.output");
	binding->set_allocated_key(bindKey);
	TDataProvider* provider = new TDataProvider;
	provider->MutableExtension(HmiWidget::animation)->set_valuestring("compositeAnimation");
	binding->set_allocated_provider(provider);
	domain->set_allocated_binding(binding);
}

void OutputPtw::triggerByInternalModel(HmiWidget::TWidget* widget) {
	addCompositeAnimation(widget);

	addTrigger(widget);

	addPlayDomain(widget);
}

void OutputPtw::messageBoxError(std::string curveName,int errorNum) {
	if (isPtwOutputError_) {
		return;
	}

	QMessageBox customMsgBox;
	customMsgBox.setWindowTitle("Warning message box");
	QPushButton* okButton = customMsgBox.addButton("OK", QMessageBox::ActionRole);
	customMsgBox.setIcon(QMessageBox::Icon::Warning);
	QString text;
	if (errorNum == 1) {
		text = QString::fromStdString(curveName) + "\" components is less than 3 !";
		text = "Warning: The number of Rotation Curve \"" + text;
	} else if (errorNum == 2) {
		text = QString::fromStdString(curveName) + "\" do not match !";
		text = "Warning: The length in \"" + text;
	} else if (errorNum == 3) {
		text = QString::fromStdString(curveName) + "\" do not match !";
		text = "Warning: The keyframe points in \"" + text;
	} else if (errorNum == 4) {
		text = QString::fromStdString(curveName) + "\" is neither linear nor hermite/beiser !";
		text = "Warning: The type of curve  \"" + text;
	} else if (errorNum == 5) {
		text = "No animation information !";
	}
	customMsgBox.setText(text);
	customMsgBox.exec();

	if (customMsgBox.clickedButton() == (QAbstractButton*)(okButton)) {
		isPtwOutputError_ = true;
	}
}

bool getAnimationInteral(std::string curveName, std::string& animationInteral) {
	auto it = curveNameAnimations_.find(curveName);
	if (it != curveNameAnimations_.end()) {
		auto animations = it->second;
		if (animations.size() > 1) {
			animationInteral = curveName + "_interal_switch";
		} else {
			// bugs
			animationInteral = animations.begin()->first + PTW_SUFFIX_CURVE_INETERAL;
		}
		return true;
	} else {
		return false;
	}
}

void OutputPtw::ConvertCurveInfo(HmiWidget::TWidget* widget, std::string animation_interal) {
	for (auto curveData : raco::guiData::CurveManager::GetInstance().getCurveList()) {
		std::string animation_interal;
		if (!getAnimationInteral(curveData->getCurveName(), animation_interal)) {
			continue;
		}
		auto curve = widget->add_curve();
		TIdentifier* curveIdentifier = new TIdentifier;
		curveIdentifier->set_valuestring(curveData->getCurveName());
		curve->set_allocated_curveidentifier(curveIdentifier);
		TDataBinding* samplePosition = new TDataBinding;
		TIdentifier* key = new TIdentifier;
		key->set_valuestring(animation_interal);
		samplePosition->set_allocated_key(key);
		TDataProvider* provider = new TDataProvider;
		provider->set_source(TEProviderSource_IntModelValue);
		samplePosition->set_allocated_provider(provider);
		curve->set_allocated_sampleposition(samplePosition);
		TCurveDefinition* curveDefinition = new TCurveDefinition;
		curveDefinition->set_curvevaluetype(TENumericType_float);
		for (auto pointData : curveData->getPointList()) {
			auto point = curveDefinition->add_point();
			TMultidimensionalPoint* pot = new TMultidimensionalPoint;
			TNumericValue* value = new TNumericValue;
			value->set_float_(std::any_cast<double>(pointData->getDataValue()));
			pot->set_domain(pointData->getKeyFrame());
			pot->set_allocated_value(value);
			point->set_allocated_point(pot);

			if (pointData->getInterPolationType() == raco::guiData::LINER) {
				TCurvePointInterpolation* incommingInterpolation = new TCurvePointInterpolation;
				incommingInterpolation->set_interpolation(TCurvePointInterpolationType_Linear);
				point->set_allocated_incomminginterpolation(incommingInterpolation);
				TCurvePointInterpolation* outgoingInterpolation = new TCurvePointInterpolation;
				outgoingInterpolation->set_interpolation(TCurvePointInterpolationType_Linear);
				point->set_allocated_outgoinginterpolation(outgoingInterpolation);
			} else if (pointData->getInterPolationType() == raco::guiData::BESIER_SPLINE) {	 // HERMIT_SPLINE
				TCurvePointInterpolation* incommingInterpolation = new TCurvePointInterpolation;
				incommingInterpolation->set_interpolation(TCurvePointInterpolationType_Hermite);
				TMultidimensionalPoint* lefttangentVector = new TMultidimensionalPoint;
				lefttangentVector->set_domain(pointData->getLeftKeyFrame());
				TNumericValue* leftValue = new TNumericValue;
				leftValue->set_float_(std::any_cast<double>(pointData->getLeftData()));
				lefttangentVector->set_allocated_value(leftValue);
				incommingInterpolation->set_allocated_tangentvector(lefttangentVector);
				point->set_allocated_incomminginterpolation(incommingInterpolation);

				TCurvePointInterpolation* outgoingInterpolation = new TCurvePointInterpolation;
				outgoingInterpolation->set_interpolation(TCurvePointInterpolationType_Hermite);
				TMultidimensionalPoint* RighttangentVector = new TMultidimensionalPoint;
				RighttangentVector->set_domain(pointData->getRightKeyFrame());
				TNumericValue* RightValue = new TNumericValue;
				//double right = std::any_cast<double>(pointData->getRightTagent());
				RightValue->set_float_(std::any_cast<double>(pointData->getRightData()));
				RighttangentVector->set_allocated_value(RightValue);
				outgoingInterpolation->set_allocated_tangentvector(RighttangentVector);
				point->set_allocated_outgoinginterpolation(outgoingInterpolation);
			} else {
				messageBoxError(curveData->getCurveName(), 4);
			}
		}
		curve->set_allocated_curvedefinition(curveDefinition);
	}
}

void OutputPtw::ConvertBind(HmiWidget::TWidget* widget, raco::guiData::NodeData& node) {
	if (0 != node.getBindingySize()) {
		HmiWidget::TNodeParam* nodeParam = widget->add_nodeparam();
		TIdentifier* identifier = new TIdentifier;
		NodeMaterial nodeMaterial;
		if (raco::guiData::MaterialManager::GetInstance().getNodeMaterial(node.objectID(), nodeMaterial)) {
			identifier->set_valuestring(node.getName() + "Shape");
		} else {
			identifier->set_valuestring(node.getName());
		}
		if (0 == node.getMaterialName().compare("")) {
			identifier->set_valuestring(node.getName());
		} else {
			identifier->set_valuestring(node.getName() + "Shape");
		}
		nodeParam->set_allocated_identifier(identifier);
		TDataBinding* paramnode = new TDataBinding;
		TDataProvider* provider = new TDataProvider;
		TVariant* variant = new TVariant;
		if (raco::guiData::MaterialManager::GetInstance().getNodeMaterial(node.objectID(), nodeMaterial)) {
			variant->set_asciistring(node.getName() + "Shape");
		} else {
			variant->set_asciistring(node.getName());
		}

		provider->set_allocated_variant(variant);
		paramnode->set_allocated_provider(provider);
		nodeParam->set_allocated_node(paramnode);
		auto animationList = node.NodeExtendRef().curveBindingRef().bindingMap();
		for (auto cuvebindList : animationList) {
			for (auto curveProP : cuvebindList.second) {
				if (curveProP.first.find("translation") == 0) {
					if (nodeParam->has_transform()) {
						auto transform = nodeParam->mutable_transform();
						if (transform->has_translation()) {
							ModifyTranslation(curveProP, transform, node);
						} else {
							CreateTranslation(curveProP, transform, node);
						}
					} else {
						HmiWidget::TNodeTransform* transform = new HmiWidget::TNodeTransform;
						CreateTranslation(curveProP, transform, node);
						nodeParam->set_allocated_transform(transform);
					}
				} else if (curveProP.first.find("rotation") == 0) {
					if (nodeParam->has_transform()) {
						auto transform = nodeParam->mutable_transform();
						if (transform->has_rotation()) {
							ModifyRotation(curveProP, transform);
						} else {
							CreateRotation(curveProP, transform, node);
						}
					} else {
						HmiWidget::TNodeTransform* transform = new HmiWidget::TNodeTransform;
						CreateRotation(curveProP, transform, node);
						nodeParam->set_allocated_transform(transform);
					}
				} else if (curveProP.first.find("scale") == 0) {
					if (nodeParam->has_transform()) {
						auto transform = nodeParam->mutable_transform();
						if (transform->has_scale()) {
							ModifyScale(curveProP, transform);
						} else {
							CreateScale(curveProP, transform, node);
						}
					} else {
						HmiWidget::TNodeTransform* transform = new HmiWidget::TNodeTransform;
						CreateScale(curveProP, transform, node);
						nodeParam->set_allocated_transform(transform);
					}
				} else {
					AddUniform(widget, curveProP, nodeParam, &node);
				}
			}
		}

		for (auto cuvebindList : animationList) {
			for (auto curveProP : cuvebindList.second) {
				std::vector<std::map<std::string, CurvesSingleProp>> curves;
				bool hasMultiCurveSingleProp = hasMultiCurveOneProp(curveProP.first, &node, curves);
				if (!hasMultiCurveSingleProp) {
					continue;
				}
				if (curveProP.first.find("scale") == 0 || curveProP.first.find("translation") == 0 || curveProP.first.find("rotation") == 0) {
					auto transform = nodeParam->mutable_transform();
					modifyMultiCurveTransform(widget, transform, curveProP.first, curves);
				}
			}
		}

		for (auto cuvebindList : animationList) {
			for (auto curveProP : cuvebindList.second) {
				std::string animaitionName;
				AnimationsSingleCurve aniSingleCurve;
				bool hasMultiAnimationSingleCurve = hasMultiAnimationOneCurve(curveProP.second, &node, aniSingleCurve, animaitionName);
				if (!hasMultiAnimationSingleCurve) {
					continue;
				}
				if (curveProP.first.find("scale") == 0 || curveProP.first.find("translation") == 0 || curveProP.first.find("rotation") == 0) {
					auto transform = nodeParam->mutable_transform();

					modifyMultiAnimaTransform(widget, transform, curveProP.first, curveProP.second, aniSingleCurve, animaitionName);
				}
			}
		}
	}

	if (node.getChildCount() != 0) {
		for (auto childNode : node.childMapRef()) {
			ConvertBind(widget, childNode.second);
		}
	} else {
		return;
	}
}

void OutputPtw::WriteAsset(std::string filePath) {
	filePath = filePath.substr(0, filePath.find(".rca"));
	nodeIDUniformsName_.clear();

	addTrigger_ = true;

	HmiWidget::TWidgetCollection widgetCollection;
	HmiWidget::TWidget* widget = widgetCollection.add_widget();
	WriteBasicInfo(widget);
	externalScaleData(widget);
	externalColorData(widget);
	if (addTrigger_) {
		triggerByInternalModel(widget);
	}
	ConvertAnimationInfo(widget);
	std::string animation_interal = "";
	ConvertCurveInfo(widget, animation_interal);
	ConvertBind(widget, NodeDataManager::GetInstance().root());
	externalOpacityData(widget);

	std::string output;
	google::protobuf::TextFormat::PrintToString(widgetCollection, &output);

	QDir* folder = new QDir;
	if (!folder->exists(QString::fromStdString(filePath))) {
		bool ok = folder->mkpath(QString::fromStdString(filePath));
	}
	delete folder;
	std::ofstream outfile;
	outfile.open(filePath + "/widget.ptw", std::ios_base::out | std::ios_base::trunc);
	outfile << output << std::endl;
	outfile.close();

	if (isPtwOutputError_) {
		QFile::remove(QString::fromStdString(filePath) + "/widget.ptw");
		isPtwOutputError_ = false;
	}
	addTrigger_ = false;
}

void OutputPtw::WriteBasicInfo(HmiWidget::TWidget* widget) {
	TIdentifier* type = new TIdentifier;
	type->set_valuestring("eWidgetType_Generate");
	widget->set_allocated_type(type);
	TIdentifier* prototype = new TIdentifier;
	prototype->set_valuestring("eWidgetType_Model");
	widget->set_allocated_prototype(prototype);
	HmiWidget::TExternalModelParameter* externalModelValue = widget->add_externalmodelvalue();

	TIdentifier* key = new TIdentifier;
	key->set_valuestring("WidgetNameHint");
	externalModelValue->set_allocated_key(key);
	TVariant* variant = new TVariant;
	variant->set_asciistring("WIDGET_SCENE");
	externalModelValue->set_allocated_variant(variant);
	externalModelValue = widget->add_externalmodelvalue();

	TIdentifier* key1 = new TIdentifier;
	key1->set_valuestring("eParam_ModelResourceId");
	externalModelValue->set_allocated_key(key1);
	TVariant* variant1 = new TVariant;
	variant1->set_resourceid("scene.ptx");
	externalModelValue->set_allocated_variant(variant1);
	externalModelValue = widget->add_externalmodelvalue();

	TIdentifier* key2 = new TIdentifier;
	key2->set_valuestring("eParam_ModelRootId");
	externalModelValue->set_allocated_key(key2);
	TVariant* variant2 = new TVariant;
	variant2->set_resourceid("");
	externalModelValue->set_allocated_variant(variant2);
}

void addMultiCurveBindingSwitchType2Operation(TOperation* operation) {
	operation->set_operator_(TEOperatorType_Switch);
	operation->add_datatype(TEDataType_Identifier);
	// operand -> key
	TIdentifier* key = new TIdentifier;
	key->set_valuestring(PTW_USED_ANIMATION_NAME);
	// operand -> provider
	TDataProvider* provider = new TDataProvider;
	provider->set_source(TEProviderSource_ExtModelValue);
	// operation.operand add key,provider
	auto operand = operation->add_operand();
	operand->set_allocated_key(key);
	operand->set_allocated_provider(provider);
}

void addMultiCurveBindingSwitchCase2Operation(TOperation* operation, std::map<std::string, CurvesSingleProp> anName, bool isDefault = false) {
	if (!isDefault) {
		operation->add_datatype(TEDataType_Identifier);
		operation->add_datatype(TEDataType_Float);

		auto operandAn = operation->add_operand();
		TDataProvider* providerAn = new TDataProvider;
		TVariant* variantAn = new TVariant;
		TIdentifier* identifierAn = new TIdentifier;
		identifierAn->set_valuestring(anName.begin()->first);
		variantAn->set_allocated_identifier(identifierAn);
		variantAn->set_identifiertype(TEIdentifierType_ParameterValue);
		providerAn->set_allocated_variant(variantAn);
		operandAn->set_allocated_provider(providerAn);

		auto operandIn = operation->add_operand();
		TIdentifier* key = new TIdentifier;
		key->set_valuestring(anName.begin()->second.curveName + ".output");
		operandIn->set_allocated_key(key);
		TDataProvider* providerIn = new TDataProvider;

		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(anName.begin()->second.curveName);
		providerIn->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
		operandIn->set_allocated_provider(providerIn);
	} else {
		operation->add_datatype(TEDataType_Float);
		auto operandIn = operation->add_operand();
		TDataProvider* providerAn = new TDataProvider;
		TVariant* variantAn = new TVariant;

		TNumericValue* nuneric = new TNumericValue;
		nuneric->set_float_(anName.begin()->second.defaultData);
		variantAn->set_allocated_numeric(nuneric);
		providerAn->set_allocated_variant(variantAn);
		operandIn->set_allocated_provider(providerAn);
	}
}

void multiCurveBindingSinglePropSwitch(HmiWidget::TWidget* widget, std::string propName, std::vector<std::map<std::string, CurvesSingleProp>> curves) {
	auto internalModelValue = widget->add_internalmodelvalue();
	// add result key
	TIdentifier* key_re = new TIdentifier;
	key_re->set_valuestring(PTW_PREFIX_CURVES_SINGLE_PROP + (curves.at(0).begin())->second.curveName);
	internalModelValue->set_allocated_key(key_re);
	// add binding
	TDataBinding* binding = new TDataBinding;
	TDataProvider* provider = new TDataProvider;
	TOperation* operation = new TOperation;
	// add switch condition to operation
	addMultiCurveBindingSwitchType2Operation(operation);
	// add switch case to operation
	for (auto anName : curves) {
		addMultiCurveBindingSwitchCase2Operation(operation, anName);
	}
	addMultiCurveBindingSwitchCase2Operation(operation, *curves.begin(), true);
	provider->set_allocated_operation(operation);
	binding->set_allocated_provider(provider);
	internalModelValue->set_allocated_binding(binding);
}

void modifyMultiCurveTranslation(HmiWidget::TNodeTransform* transform, std::string propName, std::string curveName) {
	auto translation = transform->mutable_translation();
	auto provider = translation->mutable_provider();
	auto operation = provider->mutable_operation();

	if (propName.compare("translation.x") == 0) {
		TDataBinding* operand = operation->mutable_operand(0);
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* key = new TIdentifier;
		key->set_valuestring(curveName);
		TDataProvider* provider = new TDataProvider;
		provider->set_source(TEProviderSource_IntModelValue);
		operand->set_allocated_key(key);
		operand->set_allocated_provider(provider);
	} else if (propName.compare("translation.y") == 0) {
		TDataBinding* operand = operation->mutable_operand(1);
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* key = new TIdentifier;
		key->set_valuestring(curveName);
		TDataProvider* provider = new TDataProvider;
		provider->set_source(TEProviderSource_IntModelValue);
		operand->set_allocated_key(key);
		operand->set_allocated_provider(provider);
	} else if (propName.compare("translation.z") == 0) {
		TDataBinding* operand = operation->mutable_operand(2);
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* key = new TIdentifier;
		key->set_valuestring(curveName);
		TDataProvider* provider = new TDataProvider;
		provider->set_source(TEProviderSource_IntModelValue);
		operand->set_allocated_key(key);
		operand->set_allocated_provider(provider);
	}
}

void modifyMultiCurveRotation(HmiWidget::TNodeTransform* transform, std::string propName, std::string curveName) {
	auto rotation = transform->mutable_rotation();
	auto provider = rotation->mutable_provider();
	auto operation = provider->mutable_operation();

	if (propName.compare("rotation.x") == 0) {
		TDataBinding* operand = operation->mutable_operand(0);
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* key = new TIdentifier;
		key->set_valuestring(curveName);
		TDataProvider* provider = new TDataProvider;
		provider->set_source(TEProviderSource_IntModelValue);
		operand->set_allocated_key(key);
		operand->set_allocated_provider(provider);
	} else if (propName.compare("rotation.y") == 0) {
		TDataBinding* operand = operation->mutable_operand(1);
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* key = new TIdentifier;
		key->set_valuestring(curveName);
		TDataProvider* provider = new TDataProvider;
		provider->set_source(TEProviderSource_IntModelValue);
		operand->set_allocated_key(key);
		operand->set_allocated_provider(provider);
	} else if (propName.compare("rotation.z") == 0) {
		TDataBinding* operand = operation->mutable_operand(2);
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* key = new TIdentifier;
		key->set_valuestring(curveName);
		TDataProvider* provider = new TDataProvider;
		provider->set_source(TEProviderSource_IntModelValue);
		operand->set_allocated_key(key);
		operand->set_allocated_provider(provider);
	}
}

void modifyMultiCurveScale(HmiWidget::TNodeTransform* transform, std::string propName, std::string curveName) {
	auto scale = transform->mutable_scale();
	auto provider = scale->mutable_provider();
	auto operation = provider->mutable_operation();

	if (propName.compare("scale.x") == 0) {
		TDataBinding* operand = operation->mutable_operand(0);
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* key = new TIdentifier;
		key->set_valuestring(curveName);
		TDataProvider* provider = new TDataProvider;
		provider->set_source(TEProviderSource_IntModelValue);
		operand->set_allocated_key(key);
		operand->set_allocated_provider(provider);
	} else if (propName.compare("scale.y") == 0) {
		TDataBinding* operand = operation->mutable_operand(1);
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* key = new TIdentifier;
		key->set_valuestring(curveName);
		TDataProvider* provider = new TDataProvider;
		provider->set_source(TEProviderSource_IntModelValue);
		operand->set_allocated_key(key);
		operand->set_allocated_provider(provider);
	} else if (propName.compare("scale.z") == 0) {
		TDataBinding* operand = operation->mutable_operand(2);
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* key = new TIdentifier;
		key->set_valuestring(curveName);
		TDataProvider* provider = new TDataProvider;
		provider->set_source(TEProviderSource_IntModelValue);
		operand->set_allocated_key(key);
		operand->set_allocated_provider(provider);
	}
}

void OutputPtw::modifyMultiCurveTransform(HmiWidget::TWidget* widget, HmiWidget::TNodeTransform* transform, std::string propName, std::vector<std::map<std::string, CurvesSingleProp>> curves) {
	std::string curveName = std::string(PTW_PREFIX_CURVES_SINGLE_PROP) + (curves.at(0).begin())->second.curveName;
	if (propName.find("translation") == 0) {
		modifyMultiCurveTranslation(transform, propName, curveName);
	} else if (propName.find("rotation") == 0) {
		modifyMultiCurveRotation(transform, propName, curveName);
	} else if (propName.find("scale") == 0) {
		modifyMultiCurveScale(transform, propName, curveName);
	}
	multiCurveBindingSinglePropSwitch(widget, propName, curves);
}

// more normal
void addCurvesSwitchCase2Operation(TOperation* operation, std::string AnimationName,std::string curveName, AnimationsSingleCurve nodePropData, bool isDefault = false) {
	if (!isDefault) {
		operation->add_datatype(TEDataType_Identifier);
		operation->add_datatype(TEDataType_Float);

		auto operandAn = operation->add_operand();
		TDataProvider* providerAn = new TDataProvider;
		TVariant* variantAn = new TVariant;
		TIdentifier* identifierAn = new TIdentifier;
		identifierAn->set_valuestring(AnimationName);
		variantAn->set_allocated_identifier(identifierAn);
		variantAn->set_identifiertype(TEIdentifierType_ParameterValue);
		providerAn->set_allocated_variant(variantAn);
		operandAn->set_allocated_provider(providerAn);
		// different
		auto operandIn = operation->add_operand();
		TIdentifier* curveReference = new TIdentifier;
		TDataProvider* providerIn = new TDataProvider;
		curveReference->set_valuestring(curveName);
		providerIn->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
		operandIn->set_allocated_provider(providerIn);
	} else {
		operation->add_datatype(TEDataType_Float);

		auto operandIn = operation->add_operand();
		TDataProvider* providerAn = new TDataProvider;
		TVariant* variantAn = new TVariant;

		TNumericValue* nuneric = new TNumericValue;
		nuneric->set_float_(nodePropData.defaultData);
		variantAn->set_allocated_numeric(nuneric);
		providerAn->set_allocated_variant(variantAn);
		operandIn->set_allocated_provider(providerAn);
	}
}

// refer to addAnimationDomain
void OutputPtw::addAnimationCurveSwitch(HmiWidget::TWidget* widget, std::string animationName, std::string curveName, AnimationsSingleCurve aniSingleCurve) {
	// internalModelValue
	auto internalModelValue = widget->add_internalmodelvalue();
	TIdentifier* key = new TIdentifier;
	key->set_valuestring(animationName + "-"+ curveName);
	internalModelValue->set_allocated_key(key);
	// add binding
	TDataBinding* binding = new TDataBinding;
	TDataProvider* provider = new TDataProvider;
	TOperation* operation = new TOperation;

	addUsedAnimationSwitchType2Operation(operation);
	addCurvesSwitchCase2Operation(operation, animationName, curveName, aniSingleCurve);
	addCurvesSwitchCase2Operation(operation, animationName, curveName, aniSingleCurve, true);

	provider->set_allocated_operation(operation);
	binding->set_allocated_provider(provider);
	internalModelValue->set_allocated_binding(binding);
}

void OutputPtw::modifyMultiAnimaTransform(HmiWidget::TWidget* widget, HmiWidget::TNodeTransform* transform, std::string propName, std::string curve, AnimationsSingleCurve aniSingleCurve, std::string animationName) {
	std::string curveName = animationName + "-" + curve;
	if (propName.find("translation") == 0) {
		modifyMultiCurveTranslation(transform, propName, curveName);
	} else if (propName.find("rotation") == 0) {
		modifyMultiCurveRotation(transform, propName, curveName);
	} else if (propName.find("scale") == 0) {
		modifyMultiCurveScale(transform, propName, curveName);
	}
	//multiCurveBindingSinglePropSwitch(widget, propName, animations);
	addAnimationCurveSwitch(widget, animationName, curve, aniSingleCurve);
}

// std::map<std::string, std::vector<std::map<std::string, std::vector<std::map<std::string, std::string>>>>> pNodePropCurveNames_;
//        pNodeID ,	          <   propertyName,  [ < Animation, curveName > ]	>
bool OutputPtw::hasMultiCurveOneProp(std::string prop, NodeData* pNode, std::vector<std::map<std::string, CurvesSingleProp>>& curves) {
	curves.clear();
	std::map<std::string, std::vector<std::map<std::string, std::vector<std::map<std::string, CurvesSingleProp>>>>> test = pNodePropCurveNames_;
	auto it = pNodePropCurveNames_.find(pNode->objectID());
	if (it != pNodePropCurveNames_.end()) {
		std::vector<std::map<std::string, std::vector<std::map<std::string, CurvesSingleProp>>>>& propCurves = it->second;
		for (auto& propCurve : propCurves) {
			auto itProp = propCurve.find(prop);
			if (itProp != propCurve.end()) {
				curves = itProp->second;
				propCurve.erase(itProp->first);
				return true;
			}
		}
	}
	return false;
}

// std::map<std::string, std::map<std::string, std::vector<AnimationsSingleCurve>>> curveNameAnimations_;
//        curveName  ,          [Animation,      [nodeID,   defaultData]]
bool OutputPtw::hasMultiAnimationOneCurve(std::string curveName, NodeData* pNode, AnimationsSingleCurve& aniSingleCurv, std::string& animationName) {
	for (auto it = curveNameAnimations_.begin(); it != curveNameAnimations_.end(); it++) {
		if (it->second.size() > 1 && it->first == curveName) {
			for (auto& animation : it->second) {
				for (auto& aniSingleCurve : animation.second) {
					if (pNode->objectID() == aniSingleCurve.pNode->objectID()) {
						aniSingleCurve.defaultData = setCurvesPropData(aniSingleCurve.pNode, aniSingleCurve.property);
						aniSingleCurv = aniSingleCurve;
						animationName = animation.first;
						return true;
					}
				}
			}
		}
	}
	return false;
}

void OutputPtw::ModifyTranslation(std::pair<std::string, std::string> curveProP, HmiWidget::TNodeTransform* transform, NodeData& node) {
	auto translation = transform->mutable_translation();
	auto provider = translation->mutable_provider();
	auto operation = provider->mutable_operation();

	if (curveProP.first.compare("translation.x") == 0) {
		TDataBinding* operand = operation->mutable_operand(0);
		// has multi curves binding
		if (operand->has_key() && operand->has_provider()) {
			return;
		}
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
	} else if (curveProP.first.compare("translation.y") == 0) {
		TDataBinding* operand = operation->mutable_operand(1);
		// has multi curves binding
		if (operand->has_key() && operand->has_provider()) {
			return;
		}
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
	} else if (curveProP.first.compare("translation.z") == 0) {
		TDataBinding* operand = operation->mutable_operand(2);
		// has multi curves binding
		if (operand->has_key() && operand->has_provider()) {
			return;
		}
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
	}

}

void OutputPtw::CreateTranslation(std::pair<std::string, std::string> curveProP, HmiWidget::TNodeTransform* transform , raco::guiData::NodeData& node) {
	TDataBinding* translation = new TDataBinding;
	TDataProvider* provider = new TDataProvider;
	TOperation* operation = new TOperation;
	operation->set_operator_(TEOperatorType_MuxVec3);
	operation->add_datatype(TEDataType_Float);
	operation->add_datatype(TEDataType_Float);
	operation->add_datatype(TEDataType_Float);
	auto operand1 = operation->add_operand();
	auto operand2 = operation->add_operand();
	auto operand3 = operation->add_operand();
	if (curveProP.first.compare("translation.x") == 0) {
		TDataProvider* provide = new TDataProvider;
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
		operand1->set_allocated_provider(provide);
	} else {
		TDataProvider* provide = new TDataProvider;
		TVariant* variant = new TVariant;
		TNumericValue* nuneric = new TNumericValue;
		nuneric->set_float_(std::any_cast<Vec3>(node.getSystemData("translation")).x);
		variant->set_allocated_numeric(nuneric);
		provide->set_allocated_variant(variant);
		operand1->set_allocated_provider(provide);
	}

	if (curveProP.first.compare("translation.y") == 0) {
		TDataProvider* provide = new TDataProvider;
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
		operand2->set_allocated_provider(provide);
	} else {
		TDataProvider* provide = new TDataProvider;
		TVariant* variant = new TVariant;
		TNumericValue* nuneric = new TNumericValue;
		nuneric->set_float_(std::any_cast<Vec3>(node.getSystemData("translation")).y);
		variant->set_allocated_numeric(nuneric);
		provide->set_allocated_variant(variant);
		operand2->set_allocated_provider(provide);
	}

	if (curveProP.first.compare("translation.z") == 0) {
		TDataProvider* provide = new TDataProvider;
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
		operand3->set_allocated_provider(provide);
	} else {
		TDataProvider* provide = new TDataProvider;
		TVariant* variant = new TVariant;
		TNumericValue* nuneric = new TNumericValue;
		nuneric->set_float_(std::any_cast<Vec3>(node.getSystemData("translation")).z);
		variant->set_allocated_numeric(nuneric);
		provide->set_allocated_variant(variant);
		operand3->set_allocated_provider(provide);
	}

	provider->set_allocated_operation(operation);
	translation->set_allocated_provider(provider);
	transform->set_allocated_translation(translation);
}
void OutputPtw::ModifyScale(std::pair<std::string, std::string> curveProP, HmiWidget::TNodeTransform* transform) {
	auto scale = transform->mutable_scale();
	auto provider = scale->mutable_provider();
	auto operation = provider->mutable_operation();
	if (curveProP.first.compare("scale.x") == 0) {
		TDataBinding* operand = operation->mutable_operand(0);
		// has multi curves binding
		if (operand->has_key() && operand->has_provider()) {
			return;
		}
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
	} else if (curveProP.first.compare("scale.y") == 0) {
		TDataBinding* operand = operation->mutable_operand(1);
		// has multi curves binding
		if (operand->has_key() && operand->has_provider()) {
			return;
		}
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);

	} else if (curveProP.first.compare("scale.z") == 0) {
		TDataBinding* operand = operation->mutable_operand(2);
		// has multi curves binding
		if (operand->has_key() && operand->has_provider()) {
			return;
		}
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
	}
}
void OutputPtw::CreateScale(std::pair<std::string, std::string> curveProP, HmiWidget::TNodeTransform* transform, raco::guiData::NodeData node) {
	TDataBinding* scale = new TDataBinding;
	TDataProvider* provider = new TDataProvider;
	TOperation* operation = new TOperation;
	operation->set_operator_(TEOperatorType_MuxVec3);
	operation->add_datatype(TEDataType_Float);
	operation->add_datatype(TEDataType_Float);
	operation->add_datatype(TEDataType_Float);
	auto operand1 = operation->add_operand();
	auto operand2 = operation->add_operand();
	auto operand3 = operation->add_operand();
	if (curveProP.first.compare("scale.x") == 0) {
		TDataProvider* provide = new TDataProvider;
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
		operand1->set_allocated_provider(provide);
	} else {
		TDataProvider* provide = new TDataProvider;
		TVariant* variant = new TVariant;
		TNumericValue* nuneric = new TNumericValue;
		nuneric->set_float_(std::any_cast<Vec3>(node.getSystemData("scale")).x);
		variant->set_allocated_numeric(nuneric);
		provide->set_allocated_variant(variant);
		operand1->set_allocated_provider(provide);
	}

	if (curveProP.first.compare("scale.y") == 0) {
		TDataProvider* provide = new TDataProvider;
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
		operand2->set_allocated_provider(provide);
	} else {
		TDataProvider* provide = new TDataProvider;
		TVariant* variant = new TVariant;
		TNumericValue* nuneric = new TNumericValue;
		nuneric->set_float_(std::any_cast<Vec3>(node.getSystemData("scale")).y);
		variant->set_allocated_numeric(nuneric);
		provide->set_allocated_variant(variant);
		operand2->set_allocated_provider(provide);
	}

	if (curveProP.first.compare("scale.z") == 0) {
		TDataProvider* provide = new TDataProvider;
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
		operand3->set_allocated_provider(provide);
	} else {
		TDataProvider* provide = new TDataProvider;
		TVariant* variant = new TVariant;
		TNumericValue* nuneric = new TNumericValue;
		nuneric->set_float_(std::any_cast<Vec3>(node.getSystemData("scale")).z);
		variant->set_allocated_numeric(nuneric);
		provide->set_allocated_variant(variant);
		operand3->set_allocated_provider(provide);
	}

	provider->set_allocated_operation(operation);
	scale->set_allocated_provider(provider);
	transform->set_allocated_scale(scale);
}

void OutputPtw::ModifyRotation(std::pair<std::string, std::string> curveProP, HmiWidget::TNodeTransform* transform) {
	auto rotation = transform->mutable_rotation();
	auto provider = rotation->mutable_provider();
	auto operation = provider->mutable_operation();
	if (curveProP.first.compare("rotation.x") == 0) {
		TDataBinding* operand = operation->mutable_operand(0);
		// has multi curves binding
		if (operand->has_key() && operand->has_provider()) {
			return;
		}
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
	} else if (curveProP.first.compare("rotation.y") == 0) {
		TDataBinding* operand = operation->mutable_operand(1);
		// has multi curves binding
		if (operand->has_key() && operand->has_provider()) {
			return;
		}
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);

	} else if (curveProP.first.compare("rotation.z") == 0) {
		TDataBinding* operand = operation->mutable_operand(2);
		// has multi curves binding
		if (operand->has_key() && operand->has_provider()) {
			return;
		}
		operand->clear_provider();
		TDataProvider* provide = operand->mutable_provider();
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
	}
}

void OutputPtw::CreateRotation(std::pair<std::string, std::string> curveProP, HmiWidget::TNodeTransform* transform, raco::guiData::NodeData node) {
	TDataBinding* rotation = new TDataBinding;
	TDataProvider* provider = new TDataProvider;
	TOperation* operation = new TOperation;
	operation->set_operator_(TEOperatorType_MuxVec3);
	operation->add_datatype(TEDataType_Float);
	operation->add_datatype(TEDataType_Float);
	operation->add_datatype(TEDataType_Float);
	auto operand1 = operation->add_operand();
	auto operand2 = operation->add_operand();
	auto operand3 = operation->add_operand();
	if (curveProP.first.compare("rotation.x") == 0) {
		TDataProvider* provide = new TDataProvider;
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
		operand1->set_allocated_provider(provide);
	} else {
		TDataProvider* provide = new TDataProvider;
		TVariant* variant = new TVariant;
		TNumericValue* nuneric = new TNumericValue;
		nuneric->set_float_(std::any_cast<Vec3>(node.getSystemData("rotation")).x);
		variant->set_allocated_numeric(nuneric);
		provide->set_allocated_variant(variant);
		operand1->set_allocated_provider(provide);
	}

	if (curveProP.first.compare("rotation.y") == 0) {
		TDataProvider* provide = new TDataProvider;
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
		operand2->set_allocated_provider(provide);
	} else {
		TDataProvider* provide = new TDataProvider;
		TVariant* variant = new TVariant;
		TNumericValue* nuneric = new TNumericValue;
		nuneric->set_float_(std::any_cast<Vec3>(node.getSystemData("rotation")).y);
		variant->set_allocated_numeric(nuneric);
		provide->set_allocated_variant(variant);
		operand2->set_allocated_provider(provide);
	}

	if (curveProP.first.compare("rotation.z") == 0) {
		TDataProvider* provide = new TDataProvider;
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveProP.second);
		provide->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
		operand3->set_allocated_provider(provide);
	} else {
		TDataProvider* provide = new TDataProvider;
		TVariant* variant = new TVariant;
		TNumericValue* nuneric = new TNumericValue;
		nuneric->set_float_(std::any_cast<Vec3>(node.getSystemData("rotation")).z);
		variant->set_allocated_numeric(nuneric);
		provide->set_allocated_variant(variant);
		operand3->set_allocated_provider(provide);
	}

	provider->set_allocated_operation(operation);
	rotation->set_allocated_provider(provider);
	transform->set_allocated_rotation(rotation);
}

size_t getArrIndex(std::string name) {
	std::string suffix = name.substr(name.length() - 2, 2);
	if (suffix == ".x") {
		return 0;
	} else if (suffix == ".y") {
		return 1;
	} else if (suffix == ".z") {
		return 2;
	} else if (suffix == ".w") {
		return 3;
	}
	return -1;
}

void OutputPtw::addOperandCurveRef2Operation(TOperation* operation, std::string curveName, std::string multiCurveName) {
	auto operand = operation->add_operand();
	if (multiCurveName == "") {
		TDataProvider* provider = new TDataProvider;
		TIdentifier* curveReference = new TIdentifier;
		curveReference->set_valuestring(curveName);
		provider->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
		operand->set_allocated_provider(provider);
	} else {
		TDataProvider* provider = new TDataProvider;
		TIdentifier* key = new TIdentifier;
		key->set_valuestring(multiCurveName);
		provider->set_source(TEProviderSource_IntModelValue);
		operand->set_allocated_key(key);
		operand->set_allocated_provider(provider);
	}
}


void OutputPtw::setUniformOperationByType(raco::guiData::Uniform& vecUniform, TOperation* operation, std::string* curveNameArr, std::string multiCurveOrAnimationName, bool isMultiAnimationSingleCurve) {
	auto usedUniformType = vecUniform.getType();
	switch (usedUniformType) {
		case raco::guiData::Vec2f:
			operation->set_operator_(TEOperatorType_MuxVec2);
			for (int i = 0; i < 2; ++i) {
				operation->add_datatype(TEDataType_Float);
				if (curveNameArr[i] == "") {
					float data = getUniformValueByType(vecUniform, i);
					addOperandOne2Operation(operation, data);
				} else {
					if (isMultiAnimationSingleCurve) {
						addOperandCurveRef2Operation(operation, curveNameArr[i], multiCurveOrAnimationName + "-" + curveNameArr[i]);
					}
					else {
						addOperandCurveRef2Operation(operation, curveNameArr[i], multiCurveOrAnimationName);
					}
				}
			}
			break;
		case raco::guiData::Vec3f:
			operation->set_operator_(TEOperatorType_MuxVec3);
			for (int i = 0; i < 3; ++i) {
				operation->add_datatype(TEDataType_Float);
				if (curveNameArr[i] == "") {
					float data = getUniformValueByType(vecUniform, i);
					addOperandOne2Operation(operation, data);
				} else {
					if (isMultiAnimationSingleCurve) {
						addOperandCurveRef2Operation(operation, curveNameArr[i], multiCurveOrAnimationName + "-" + curveNameArr[i]);
					} else {
						addOperandCurveRef2Operation(operation, curveNameArr[i], multiCurveOrAnimationName);
					}
				}
			}
			break;
		case raco::guiData::Vec4f:
			operation->set_operator_(TEOperatorType_MuxVec4);
			for (int i = 0; i < 4; ++i) {
				operation->add_datatype(TEDataType_Float);
				if (curveNameArr[i] == "") {
					float data = getUniformValueByType(vecUniform, i);
					addOperandOne2Operation(operation, data);
				} else {
					if (isMultiAnimationSingleCurve) {
						addOperandCurveRef2Operation(operation, curveNameArr[i], multiCurveOrAnimationName + "-" + curveNameArr[i]);
					} else {
						addOperandCurveRef2Operation(operation, curveNameArr[i], multiCurveOrAnimationName);
					}
				}
			}
			break;
		case raco::guiData::Vec2i:
			operation->set_operator_(TEOperatorType_MuxVec2);
			for (int i = 0; i < 2; ++i) {
				operation->add_datatype(TEDataType_Int);
				if (curveNameArr[i] == "") {
					float data = getUniformValueByType(vecUniform, i);
					addOperandOne2Operation(operation, data);
				} else {
					if (isMultiAnimationSingleCurve) {
						addOperandCurveRef2Operation(operation, curveNameArr[i], multiCurveOrAnimationName + "-" + curveNameArr[i]);
					} else {
						addOperandCurveRef2Operation(operation, curveNameArr[i], multiCurveOrAnimationName);
					}
				}
			}
			break;
		case raco::guiData::Vec3i:
			operation->set_operator_(TEOperatorType_MuxVec3);
			for (int i = 0; i < 3; ++i) {
				operation->add_datatype(TEDataType_Int);
				if (curveNameArr[i] == "") {
					float data = getUniformValueByType(vecUniform, i);
					addOperandOne2Operation(operation, data);
				} else {
					if (isMultiAnimationSingleCurve) {
						addOperandCurveRef2Operation(operation, curveNameArr[i], multiCurveOrAnimationName + "-" + curveNameArr[i]);
					} else {
						addOperandCurveRef2Operation(operation, curveNameArr[i], multiCurveOrAnimationName);
					}
				}
			}
			break;
		case raco::guiData::Vec4i:
			operation->set_operator_(TEOperatorType_MuxVec4);
			for (int i = 0; i < 4; ++i) {
				operation->add_datatype(TEDataType_Int);
				if (curveNameArr[i] == "") {
					float data = getUniformValueByType(vecUniform, i);
					addOperandOne2Operation(operation, data);
				} else {
					if (isMultiAnimationSingleCurve) {
						addOperandCurveRef2Operation(operation, curveNameArr[i], multiCurveOrAnimationName + "-" + curveNameArr[i]);
					} else {
						addOperandCurveRef2Operation(operation, curveNameArr[i], multiCurveOrAnimationName);
					}
				}
			}
			break;
		default:
			break;
	}
}

bool OutputPtw::isAddedUniform(std::string name, raco::guiData::NodeData* node) {
	auto re = nodeIDUniformsName_.find(node->objectID());
	if (re != nodeIDUniformsName_.end()) {
		for (auto& unName : re->second) {
			if (unName == name) {
				return true;
			}
		}
		re->second.push_back(name);
		return false;
	}
	std::vector<std::string> names;
	names.push_back(name);
	nodeIDUniformsName_.emplace(node->objectID(), names);
	return false;
}

bool findFromUniform(std::string property, std::string name) {
	QStringList propArr = QString::fromStdString(property).split(".");
	if (propArr[propArr.size() - 2].toStdString() == name) {
		return true;
	}
	return false;
}

void OutputPtw::addVecValue2Uniform(HmiWidget::TWidget* widget, std::pair<std::string, std::string> curveProP, HmiWidget::TNodeParam* nodeParam, raco::guiData::NodeData* node) {
	// set uniform name
	std::string uniformName = curveProP.first.substr(9, curveProP.first.length() - 11);

	if (isAddedUniform(uniformName, node)) {
		return;
	}

	// get uniforms
	bool isInUniforms = false;
	NodeMaterial nodeMaterial;
	raco::guiData::MaterialManager::GetInstance().getNodeMaterial(node->objectID(), nodeMaterial);
	// get weights
	raco::guiData::Uniform vecUniform;
	vecUniform.setType(UniformType::Null);
	std::vector<Uniform> uniforms = nodeMaterial.getUniforms();
	for (auto& un : uniforms) {
		if (un.getName() == uniformName) {
			vecUniform = un;
			isInUniforms = true;
			break;
		}
	}
	if (!isInUniforms) {
		return;
	}

	auto uniform = nodeParam->add_uniform();

	TDataBinding* name = new TDataBinding;
	TDataProvider* namePrivder = new TDataProvider;
	TVariant* variant = new TVariant;
	variant->set_asciistring(delUniformNamePrefix(uniformName));
	namePrivder->set_allocated_variant(variant);
	name->set_allocated_provider(namePrivder);
	uniform->set_allocated_name(name);

	// set uniform value
	TDataProvider* valProvder = new TDataProvider;
	TOperation* operation = new TOperation;
	TDataBinding* value = new TDataBinding;

	// get which weight used
	std::string curveNameArr[4] = {""};
	std::map<std::string, std::map<std::string, std::string>>& map = node->NodeExtendRef().curveBindingRef().bindingMap();
	for (auto& an : map) {
		for (auto& prop : an.second) {
			int index = -1;
			if (findFromUniform(prop.first, uniformName) && -1 != (index = getArrIndex(prop.first))) {
				curveNameArr[index] = prop.second;
			}
		}
	}
	// step2
	std::vector<std::map<std::string, CurvesSingleProp>> curves;
	bool hasMultiCurveSingleProp = hasMultiCurveOneProp(curveProP.first, node, curves);
	// step3
	std::string animaitionName;
	AnimationsSingleCurve aniSingleCurve;
	bool hasMultiAnimationSingleCurve = hasMultiAnimationOneCurve(curveProP.second, node, aniSingleCurve, animaitionName);

	if (!hasMultiCurveSingleProp && !hasMultiAnimationSingleCurve) {
		// set operation
		setUniformOperationByType(vecUniform, operation, curveNameArr);
	} else if (hasMultiCurveSingleProp && !hasMultiAnimationSingleCurve) {
		std::string multiCurveName = "Multi-" + (curves.at(0).begin())->second.curveName;
		setUniformOperationByType(vecUniform, operation, curveNameArr, multiCurveName);
		multiCurveBindingSinglePropSwitch(widget, curveProP.first, curves);
	} else if (!hasMultiCurveSingleProp && hasMultiAnimationSingleCurve) {
		std::string curveName = animaitionName + "-" + curveProP.second;
		bool isMultiAnimationSingleCurve = true;
		setUniformOperationByType(vecUniform, operation, curveNameArr, animaitionName, true);
		for (int i = 0; i < 4; ++i) {
			if (curveNameArr[i] != "") {
				addAnimationCurveSwitch(widget, animaitionName, curveNameArr[i], aniSingleCurve);
			}
		}
	}
	else { // Step4
		DEBUG(__FILE__, __FUNCTION__, __LINE__);
	}

	// add to value
	valProvder->set_allocated_operation(operation);
	value->set_allocated_provider(valProvder);
	uniform->set_allocated_value(value);
}

bool OutputPtw::isVecUniformValue(std::string name) {
	std::string suffix = name.substr(name.length() - 2, 2);
	if (suffix == ".x" || suffix == ".y" || suffix == ".z" || suffix == ".w") {
		return true;
	}
	return false;
}

void OutputPtw::addOperandOne2Operation(TOperation* operation, float data) {
	auto operand = operation->add_operand();
	TDataProvider* provider = new TDataProvider;
	TVariant* variant = new TVariant;
	TNumericValue* numeric = new TNumericValue;
	numeric->set_float_(data);
	variant->set_allocated_numeric(numeric);
	provider->set_allocated_variant(variant);

	operand->set_allocated_provider(provider);
}

void OutputPtw::AddUniform(HmiWidget::TWidget* widget,std::pair<std::string, std::string> curveProP, HmiWidget::TNodeParam* nodeParam, raco::guiData::NodeData* node) {
	auto uniforms = nodeParam->uniform();
	for (auto un : uniforms) {
		std::string nameBinding = un.name().provider().variant().asciistring();
		if (nameBinding == delUniformNamePrefix(curveProP.first)) {
			return;
		}
	}
	if (!isVecUniformValue(curveProP.first)) {
		auto uniform = nodeParam->add_uniform();
		// set uniform name
		TDataBinding* name = new TDataBinding;
		TDataProvider* namePrivder = new TDataProvider;
		TVariant* variant = new TVariant;
		variant->set_asciistring(delUniformNamePrefix(curveProP.first));
		namePrivder->set_allocated_variant(variant);
		name->set_allocated_provider(namePrivder);
		uniform->set_allocated_name(name);

		std::vector<std::map<std::string, CurvesSingleProp>> curves;
		bool hasMultiCurveSingleProp = hasMultiCurveOneProp(curveProP.first, node, curves);

		std::string animaitionName;
		AnimationsSingleCurve aniSingleCurve;
		bool hasMultiAnimationSingleCurve = hasMultiAnimationOneCurve(curveProP.second, node, aniSingleCurve, animaitionName);

		if (!hasMultiCurveSingleProp && !hasMultiAnimationSingleCurve) {
			// set uniform value
			TDataBinding* value = new TDataBinding;
			TDataProvider* privder = new TDataProvider;
			TIdentifier* curveReference = new TIdentifier;
			curveReference->set_valuestring(curveProP.second);
			privder->MutableExtension(HmiWidget::curve)->set_allocated_curvereference(curveReference);
			value->set_allocated_provider(privder);
			uniform->set_allocated_value(value);
		} else if (hasMultiCurveSingleProp && !hasMultiAnimationSingleCurve) {
			TDataBinding* value = new TDataBinding;
			TIdentifier* key = new TIdentifier;
			TDataProvider* provider = new TDataProvider;
			key->set_valuestring("Multi-" + (curves.at(0).begin())->second.curveName);
			provider->set_source(TEProviderSource_IntModelValue);
			value->set_allocated_key(key);
			value->set_allocated_provider(provider);
			uniform->set_allocated_value(value);

			multiCurveBindingSinglePropSwitch(widget, curveProP.first, curves);
		} else if (!hasMultiCurveSingleProp && hasMultiAnimationSingleCurve) {
			std::string curveName = animaitionName + "-" + curveProP.second;
			TDataBinding* value = new TDataBinding;
			TIdentifier* key = new TIdentifier;
			TDataProvider* provider = new TDataProvider;
			key->set_valuestring(curveName);
			provider->set_source(TEProviderSource_IntModelValue);
			value->set_allocated_key(key);
			value->set_allocated_provider(provider);
			uniform->set_allocated_value(value);
			addAnimationCurveSwitch(widget, animaitionName, curveProP.second, aniSingleCurve);
		} else {
			DEBUG(__FILE__, __FUNCTION__, __LINE__);
			qDebug() << "Step4";
		}
	} else {
		addVecValue2Uniform(widget, curveProP, nodeParam, node);
	}
}

// add exteral scale
void OutputPtw::externalScale(HmiWidget::TWidget* widget) {
	HmiWidget::TExternalModelParameter* externalModelValue = widget->add_externalmodelvalue();
	externalModelValue->set_allocated_key(assetsFun_.Key(PTW_EX_SCALE_NAME));
	externalModelValue->set_allocated_variant(assetsFun_.VariantNumeric(300.0));
}

void OutputPtw::externalScaleData(HmiWidget::TWidget* widget) {
	if (!NodeScaleSize_) {
		DEBUG(__FILE__, __FUNCTION__, __LINE__);
	}

	//  add exteral scale
	externalScale(widget);

	// add scale Div 300
	auto internalModelValue = widget->add_internalmodelvalue();
	internalModelValue->set_allocated_key(assetsFun_.Key(PTW_SCALE_DIVIDE_VALUE));
	internalModelValue->set_allocated_binding(assetsFun_.BindingValueStrNumericOperatorType(PTW_EX_SCALE_NAME, TEProviderSource_ExtModelValue, 300, TEOperatorType_Div));

	// add scale Mul NodeScaleSize_
	internalModelValue = widget->add_internalmodelvalue();
	internalModelValue->set_allocated_key(assetsFun_.Key(PTW_SCALE_DIV_RESULT_MUL_VALUE));
	internalModelValue->set_allocated_binding(assetsFun_.BindingValueStrNumericOperatorType(PTW_SCALE_DIVIDE_VALUE, TEProviderSource_IntModelValue, NodeScaleSize_, TEOperatorType_Mul));

	// add nodeParam of Node scale
	HmiWidget::TNodeParam* nodeParam = widget->add_nodeparam();
	assetsFun_.NodeParamAddIdentifier(nodeParam, PTW_ROOT_NODE_NAME);
	assetsFun_.NodeParamAddNode(nodeParam, PTW_ROOT_NODE_NAME);
	auto transform = nodeParam->mutable_transform();
	TDataBinding operandX;
	assetsFun_.OperandKeySrc(operandX, PTW_SCALE_DIV_RESULT_MUL_VALUE, TEProviderSource_IntModelValue);
	TDataBinding operandY;
	assetsFun_.OperandKeySrc(operandY, PTW_SCALE_DIV_RESULT_MUL_VALUE, TEProviderSource_IntModelValue);
	TDataBinding operandZ;
	assetsFun_.OperandKeySrc(operandZ, PTW_SCALE_DIV_RESULT_MUL_VALUE, TEProviderSource_IntModelValue);
	assetsFun_.TransformCreateScale(transform, operandX, operandY, operandZ);
}

// add exteral Opacity
void OutputPtw::externalOpacity(HmiWidget::TWidget* widget) {
	HmiWidget::TExternalModelParameter* externalModelValue = widget->add_externalmodelvalue();
	externalModelValue->set_allocated_key(assetsFun_.Key(PTW_EX_OPACITY_NAME));
	externalModelValue->set_allocated_variant(assetsFun_.VariantNumeric(1.0));
}

void OutputPtw::createResourceParam(HmiWidget::TWidget* widget, std::string materialName) {
	// identifier
	HmiWidget::TResourceParam* resParam = widget->add_resourceparam();
	assetsFun_.ResourceParamAddIdentifier(resParam, materialName + "_Opacity");	 // test
	// resource
	assetsFun_.ResourceParamAddResource(resParam, materialName);
	// appearance
	HmiWidget::TAppearanceParam* appearance = new HmiWidget::TAppearanceParam;
	assetsFun_.AddUniform2Appearance(appearance, PTW_FRAG_CTRL_OPACITY, PTW_EX_OPACITY_NAME, TEProviderSource_ExtModelValue);
	resParam->set_allocated_appearance(appearance);
}

void OutputPtw::externalOpacityData(HmiWidget::TWidget* widget) {
	if (HasOpacityMaterials_.empty()) {
		return ;
	}
	// external Opacity
	externalOpacity(widget);

	// resource Param
	for (auto materialName : HasOpacityMaterials_) {
		createResourceParam(widget, materialName);
	}
}

void OutputPtw::externalColorData(HmiWidget::TWidget* widget) {
	int ExColorNum = 6;
	for (int i = 0; i < ExColorNum; ++i) {
		// CONTENT..._Ci
		auto externalModelValueCi = widget->add_externalmodelvalue();
		assetsFun_.ColorExternal(externalModelValueCi, PTW_CONTENT_IPA_ICON_TRANSFORMATION + std::to_string(i + 1));
	}

	for (int i = 0; i < ExColorNum; ++i) {
		// HUD_CONTENT..._Ci
		auto HUD_ExternalModelValueCi = widget->add_externalmodelvalue();
		assetsFun_.ColorExternal(HUD_ExternalModelValueCi, std::string(PTW_EX_HUB_NAME) + PTW_CONTENT_IPA_ICON_TRANSFORMATION + std::to_string(i + 1));
	}

	// HUB
	auto externalModelValue = widget->add_externalmodelvalue();
	externalModelValue->set_allocated_key(assetsFun_.Key(PTW_EX_HUB_NAME));
	externalModelValue->set_allocated_variant(assetsFun_.VariantNumeric(0));

	// CONTENT..._Ci_V4
	for (int i = 0; i < ExColorNum; ++i) {
		auto externalModelValueCiV4 = widget->add_externalmodelvalue();
		assetsFun_.ColorExternal(externalModelValueCiV4, PTW_CONTENT_IPA_ICON_TRANSFORMATION + std::to_string(i + 1) + "_V4", PTW_CONTENT_IPA_ICON_TRANSFORMATION + std::to_string(i + 1));
	}
	//	HUD_CONTENT..._Ci_V4
	for (int i = 0; i < ExColorNum; ++i) {
		auto HUB_ExternalModelValueCiV4 = widget->add_externalmodelvalue();
		assetsFun_.ColorExternal(HUB_ExternalModelValueCiV4, std::string(PTW_EX_HUB_NAME) + PTW_CONTENT_IPA_ICON_TRANSFORMATION + std::to_string(i + 1) + "_V4", std::string(PTW_EX_HUB_NAME) + PTW_CONTENT_IPA_ICON_TRANSFORMATION + std::to_string(i + 1));
	}
}

}  // namespace raco::dataConvert
