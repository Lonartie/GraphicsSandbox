#include "AssimpImporter.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <ranges>
#include <vector>
#include <QMatrix3x3>
#include <QDir>
#include <QFileInfo>
#include <QImageReader>

#include "Model/Components/MaterialComponent.h"
#include "Model/Components/MeshComponent.h"
#include "Common/AssetProvider.h"

namespace {
   std::unordered_map<QString, uint64_t> LOADED;
   uint64_t OBJECT_COUNT = 0;
   uint64_t IMAGE_COUNT = 0;
}

static QUuid importNode(const QDir& root,
                        const aiNode* assimpNode,
                        const aiScene* assimpScene,
                        Scene& scene,
                        aiMatrix4x4 transform);

static QUuid createObject(const QDir& root,
                          const aiNode* assimpNode,
                          aiMesh* assimpMesh,
                          const aiScene* assimpScene,
                          Scene& scene,
                          aiMatrix4x4* transform);

static std::vector<uint64_t> loadMaterialTextures(const QDir& root,
                                                  const aiMaterial* mat,
                                                  aiTextureType type,
                                                  const aiScene* scene);

QUuid AssimpImporter::loadInto(const QString& path, Scene& scene) {
   OBJECT_COUNT = 0;
   IMAGE_COUNT = 0;

   Assimp::Importer importer;
   const auto root = QFileInfo(path).absoluteDir();
   const aiScene* assimpScene = importer.ReadFile(
      path.toStdString(),
      aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);

   if (!assimpScene || assimpScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !assimpScene->mRootNode) {
      return {};
   }

   // preload textures
   for (unsigned int i = 0; i < assimpScene->mNumMaterials; ++i) {
      auto material = assimpScene->mMaterials[i];
      for (int j = 0; j <= 17; ++j) {
         loadMaterialTextures(root, material, (aiTextureType) j, assimpScene);
      }
   }

   auto id = importNode(root, assimpScene->mRootNode, assimpScene, scene, aiMatrix4x4());
   GS_DEBUG() << "Imported" << OBJECT_COUNT << "objects and " << LOADED.size() << "images";
   return id;
}

QUuid importNode(
   const QDir& root,
   const aiNode* assimpNode,
   const aiScene* assimpScene,
   Scene& scene,
   aiMatrix4x4 transform) {
   std::vector<QUuid> result;

   aiMatrix4x4 globalMatrix = assimpNode->mTransformation * transform;

   QUuid first = createObject(root, assimpNode, nullptr, assimpScene, scene, &globalMatrix);

   // process meshes
   for (unsigned int i = 0; i < assimpNode->mNumMeshes; i++) {
      aiMesh* assimpMesh = assimpScene->mMeshes[assimpNode->mMeshes[i]];
      auto id = createObject(root, assimpNode, assimpMesh, assimpScene, scene, nullptr);
      auto parentObj = scene.findObject(first);
      auto childObj = scene.findObject(id);
      if (parentObj && childObj) {
         scene.addChild(**parentObj, **childObj);
      }
      result.push_back(id);
   }

   // process children
   for (unsigned int i = 0; i < assimpNode->mNumChildren; i++) {
      auto child = importNode(root, assimpNode->mChildren[i], assimpScene, scene, globalMatrix);
      auto parentObj = scene.findObject(first);
      auto childObj = scene.findObject(child);
      if (parentObj && childObj) {
         scene.addChild(**parentObj, **childObj);
      }
   }
   return first;
}

QUuid createObject(
   const QDir& root,
   const aiNode* assimpNode,
   aiMesh* assimpMesh,
   const aiScene* assimpScene,
   Scene& scene,
   aiMatrix4x4* transform) {
   auto obj = Object::create(scene);
   obj->setName(QString::fromStdString(assimpNode->mName.C_Str()));
   auto& trans = obj->getComponent<TransformComponent>();
   OBJECT_COUNT++;

   if (transform) {
      aiVector3D position, scaling;
      aiQuaternion rotation;
      transform->Decompose(scaling, rotation, position);
      trans.position = QVector3D(position.x, position.y, position.z);
      trans.rotation = QQuaternion(rotation.w, rotation.x, rotation.y, rotation.z);
      trans.scale = QVector3D(scaling.x, scaling.y, scaling.z);
   }

   if (assimpMesh) {
      auto& mesh = obj->addComponent<MeshComponent>();
      auto& mat = obj->addComponent<MaterialComponent>();
      mat.shader = "Default";
      mat.properties.emplace(
         "solidColor",
         MaterialComponent::Property{
            .type = "QColor", .value = QColor(Qt::magenta)
         });

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

      // process material
      auto materialIndex = assimpMesh->mMaterialIndex;
      auto materials = assimpScene->mMaterials;

      if (materialIndex < assimpScene->mNumMaterials) {
         auto material = materials[materialIndex];
         std::vector<uint64_t> albedoMaps = loadMaterialTextures(root, material, aiTextureType_DIFFUSE, assimpScene);

         if (!albedoMaps.empty()) {
            mat.shader = "Material";
            mat.properties.emplace(
               "albedo",
               MaterialComponent::Property{
                  .type = "QImage", .value = albedoMaps.at(0)
               });

            std::vector<uint64_t> normalMaps = loadMaterialTextures(root, material, aiTextureType_HEIGHT, assimpScene);
            if (!normalMaps.empty()) {
               mat.properties.emplace(
                  "normal",
                  MaterialComponent::Property{
                     .type = "QImage", .value = normalMaps.at(0)
                  });
            }
         }
      }
   }

   const auto id = obj->id();
   scene.addObject(std::move(obj));
   return id;
}

static std::vector<uint64_t> loadMaterialTextures(const QDir& root,
                                                  const aiMaterial* mat,
                                                  aiTextureType type,
                                                  const aiScene* scene) {
   std::vector<uint64_t> result;
   for (unsigned int i = 0; i < mat->GetTextureCount(type); i++) {
      aiString str;
      mat->GetTexture(type, i, &str);
      QString path = root.absoluteFilePath(QString::fromStdString(str.C_Str()));
      if (LOADED.contains(path)) {
         result.push_back(LOADED.at(path));
         continue;
      }
      QImageReader reader(path);
      QImage image = reader.read();
      if (image.isNull()) {
         GS_DEBUG() << "Failed to load texture:" << reader.errorString();
         GS_DEBUG() << "Trying to load a PNG variant";
         QFileInfo info(path);
         auto pngPath = root.absoluteFilePath(info.baseName() + ".png");
         QImageReader pngReader(pngPath);
         image = pngReader.read();
         if (image.isNull()) {
            GS_DEBUG() << "Failed to load PNG texture:" << pngReader.errorString();
         } else {
            LOADED.emplace(path, AssetProvider::instance().add<QImage>(std::move(image)));
            result.push_back(LOADED.at(path));
         }
      } else {
         LOADED.emplace(path, AssetProvider::instance().add<QImage>(std::move(image)));
         result.push_back(LOADED.at(path));
      }
   }
   return result;
}
