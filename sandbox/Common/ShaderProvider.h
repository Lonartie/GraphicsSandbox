#pragma once
#include "Common.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QOpenGLShaderProgram>
#include <QFileSystemWatcher>
#include <unordered_map>

static constexpr auto ShadersLocation = "Shaders";

class ShaderProvider : public QObject {
   Q_OBJECT

public:
   static ShaderProvider& instance();

   QStringList getShaderNames() const;
   const std::unordered_map<QString, QOpenGLShaderProgram*, QtHasher<QString>>& getShaders() const;

signals:
   void shadersChanged(QStringList updatesCollection);
   void shaderChanged(QString shaderName, QOpenGLShaderProgram* shaderProgram);

   void fileError(const QString& path, const QString& error);

private slots:
   void dirChanged(const QString& path);
   void fileChanged(const QString& path);

   void invalidateShader(const QString& name);

private:
   ShaderProvider();
   ~ShaderProvider() override = default;

   static QStringList allContents(const QString& location);
   static QStringList subDirs(const QString& location);
   static QStringList subFiles(const QString& location);
   static std::optional<QString> getFileType(const QStringList& collection, const QString& ext);

private:
   QFileSystemWatcher m_watcher;
   std::unordered_map<QString, QOpenGLShaderProgram*, QtHasher<QString>> m_shaders;
   bool m_emitShaderChanged = true;
};
