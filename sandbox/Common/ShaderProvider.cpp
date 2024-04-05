#include "ShaderProvider.h"
#include <QDirIterator>

ShaderProvider& ShaderProvider::instance() {
   static ShaderProvider instance;
   return instance;
}

ShaderProvider::ShaderProvider() {
   // watch every single file and folder
   m_watcher.addPath(QDir(ShadersLocation).canonicalPath());
   for (auto& file: allContents(ShadersLocation)) {
      m_watcher.addPath(file);
   }

   connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, &ShaderProvider::dirChanged);
   connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, &ShaderProvider::fileChanged);

   for (auto& shader: subDirs(ShadersLocation)) {
      const auto name = QDir(shader).dirName();
      invalidateShader(name);
   }
}

QStringList ShaderProvider::allContents(const QString& location) {
   // recursively search for all files in the location
   QDirIterator it(location, QDir::Files | QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot, QDirIterator::Subdirectories);
   QStringList files;
   while (it.hasNext()) {
      it.next();
      files << it.filePath();
   }
   return files;
}

QStringList ShaderProvider::subDirs(const QString& location) {
   QDirIterator it(location, QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot);
   QStringList dirs;
   while (it.hasNext()) {
      it.next();
      dirs << it.filePath();
   }
   return dirs;
}

QStringList ShaderProvider::subFiles(const QString& location) {
   QDirIterator it(location, QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
   QStringList files;
   while (it.hasNext()) {
      it.next();
      files << it.filePath();
   }
   return files;
}

std::optional<QString> ShaderProvider::getFileType(const QStringList& collection, const QString& ext) {
   for (const auto& file: collection) {
      if (file.endsWith(ext)) {
         return file;
      }
   }
   return std::nullopt;
}

void ShaderProvider::dirChanged(const QString& path) {
   if (QDir(path).canonicalPath() == QDir(ShadersLocation).canonicalPath()) {
      // invalidate all shaders
      m_emitShaderChanged = false;
      for (auto& shader: subDirs(ShadersLocation)) {
         const auto name = QDir(shader).dirName();
         invalidateShader(name);
      }
      emit shadersChanged(getShaderNames());
      m_emitShaderChanged = true;
   } else {
      // invalidate only the shader that was changed
      const auto name = QDir(path).dirName();
      invalidateShader(name);
   }

   for (auto& file: allContents(ShadersLocation)) {
      m_watcher.addPath(file);
   }
}

void ShaderProvider::fileChanged(const QString& path) {
   // invalidate only the shader that was changed
   auto fileDir = QFileInfo(path).dir();
   auto parentDir = fileDir;
   parentDir.cdUp();
   if (parentDir.canonicalPath() == QDir(ShadersLocation).canonicalPath()) {
      // yes it is a shader
      const auto name = fileDir.dirName();
      invalidateShader(name);
   }
}

void ShaderProvider::invalidateShader(const QString& name) {
   auto shader = new QOpenGLShaderProgram();


   auto files = subFiles(QString(ShadersLocation) + "/" + name);
   if (auto vertFile = getFileType(files, ".vert")) {
      if (!shader->addShaderFromSourceFile(QOpenGLShader::Vertex, *vertFile)) {
         emit fileError(*vertFile, shader->log());
         delete shader;
         return;
      }
   }
   if (auto tescFile = getFileType(files, ".tesc")) {
      if (!shader->addShaderFromSourceFile(QOpenGLShader::TessellationControl, *tescFile)) {
         emit fileError(*tescFile, shader->log());
         delete shader;
         return;
      }
   }
   if (auto teseFile = getFileType(files, ".tese")) {
      if (!shader->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, *teseFile)) {
         emit fileError(*teseFile, shader->log());
         delete shader;
         return;
      }
   }
   if (auto geomFile = getFileType(files, ".geom")) {
      if (!shader->addShaderFromSourceFile(QOpenGLShader::Geometry, *geomFile)) {
         emit fileError(*geomFile, shader->log());
         delete shader;
         return;
      }
   }
   if (auto fragFile = getFileType(files, ".frag")) {
      if (!shader->addShaderFromSourceFile(QOpenGLShader::Fragment, *fragFile)) {
         emit fileError(*fragFile, shader->log());
         delete shader;
         return;
      }
   }
   if (!shader->link()) {
      emit fileError(name, shader->log());
      delete shader;
      return;
   }

   delete m_shaders[name];
   m_shaders[name] = shader;
   if (m_emitShaderChanged) {
      emit shaderChanged(name, shader);
   }

   qDebug() << "loaded " << name;
}

QStringList ShaderProvider::getShaderNames() const {
   QStringList names;
   for (const auto& [name, _]: m_shaders) {
      names << name;
   }
   return names;
}

const std::unordered_map<QString, QOpenGLShaderProgram*, QtHasher<QString>>& ShaderProvider::getShaders() const {
   return m_shaders;
}
