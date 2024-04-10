#pragma once
#include <QString>
#include <assimp/mesh.h>

#include "Model/Hierarchy/Scene.h"
#include "Model/Hierarchy/Object.h"

struct aiNode;
struct aiScene;
struct aiMesh;

class AssimpImporter {
public:
   static QUuid loadInto(const QString& path, Scene& scene);
};
