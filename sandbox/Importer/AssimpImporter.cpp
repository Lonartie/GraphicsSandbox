#include "AssimpImporter.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <ranges>
#include <vector>
#include <QMatrix3x3>

#include "Model/Components/MaterialComponent.h"
#include "Model/Components/MeshComponent.h"

static QUuid importNode(const aiNode* assimpNode,
                                     const aiScene* assimpScene,
                                     Scene& scene,
                                     aiMatrix4x4 transform);

static QUuid createObject(const aiNode* assimpNode,
                          aiMesh* assimpMesh,
                          const aiScene* assimpScene,
                          Scene& scene,
                          aiMatrix4x4* transform);

QUuid AssimpImporter::loadInto(const QString& path, Scene& scene) {
   Assimp::Importer importer;
   const aiScene* assimpScene = importer.ReadFile(
      path.toStdString(),
      aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

   if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode) {
      return {};
   }

   return importNode(assimpScene->mRootNode, assimpScene, scene, aiMatrix4x4());
}

QUuid importNode(
   const aiNode* assimpNode,
   const aiScene* assimpScene,
   Scene& scene,
   aiMatrix4x4 transform) {
   std::vector<QUuid> result;

   aiMatrix4x4 globalMatrix = assimpNode->mTransformation * transform;

   QUuid first = createObject(assimpNode, nullptr, assimpScene, scene, &globalMatrix);

   // process meshes
   for (unsigned int i = 0; i < assimpNode->mNumMeshes; i++) {
      aiMesh* assimpMesh = assimpScene->mMeshes[assimpNode->mMeshes[i]];
      auto id = createObject(assimpNode, assimpMesh, assimpScene, scene, nullptr);
      auto parentObj = scene.findObject(first);
      auto childObj = scene.findObject(id);
      if (parentObj && childObj) {
         scene.addChild(**parentObj, **childObj);
      }
      result.push_back(id);
   }

   // process children
   for (unsigned int i = 0; i < assimpNode->mNumChildren; i++) {
      auto child = importNode(assimpNode->mChildren[i], assimpScene, scene, globalMatrix);
      auto parentObj = scene.findObject(first);
      auto childObj = scene.findObject(child);
      if (parentObj && childObj) {
         scene.addChild(**parentObj, **childObj);
      }
   }
   return first;
}

QUuid createObject(
   const aiNode* assimpNode,
   aiMesh* assimpMesh,
   const aiScene* assimpScene,
   Scene& scene,
   aiMatrix4x4* transform) {
   auto obj = Object::create(scene);
   obj->setName(QString::fromStdString(assimpNode->mName.C_Str()));
   auto& trans = obj->getComponent<TransformComponent>();
   auto& mesh = obj->addComponent<MeshComponent>();
   auto& mat = obj->addComponent<MaterialComponent>();

   if (transform) {
      aiVector3D position, scaling;
      aiQuaternion rotation;
      transform->Decompose(scaling, rotation, position);
      trans.position = QVector3D(position.x, position.y, position.z);
      trans.rotation = QQuaternion(rotation.w, rotation.x, rotation.y, rotation.z);
      trans.scale = QVector3D(scaling.x, scaling.y, scaling.z);
   }

   mat.shader = "Default";
   mat.properties.emplace(
      "solidColor",
      MaterialComponent::Property{
         .type = "QColor",
         .value = QColor(Qt::white)
      });

   if (assimpMesh) {
      // process vertices and normals
      for (unsigned int i = 0; i < assimpMesh->mNumVertices; i++) {
         mesh.vertices.emplace_back(
            assimpMesh->mVertices[i].x,
            assimpMesh->mVertices[i].y,
            assimpMesh->mVertices[i].z);

         mesh.normals.emplace_back(
            assimpMesh->mNormals[i].x,
            assimpMesh->mNormals[i].y,
            assimpMesh->mNormals[i].z);

         // process texture coordinates (uvs)
         if (assimpMesh->mTextureCoords[0]) {
            mesh.uvs.emplace_back(
               assimpMesh->mTextureCoords[0][i].x,
               assimpMesh->mTextureCoords[0][i].y);
         }
      }

      // process indices
      for (unsigned int i = 0; i < assimpMesh->mNumFaces; i++) {
         auto& face = assimpMesh->mFaces[i];
         for (unsigned int j = 0; j < face.mNumIndices; j++) {
            mesh.indices.push_back(face.mIndices[j]);
         }
      }
   }

   const auto id = obj->id();
   scene.addObject(std::move(obj));
   return id;
}
