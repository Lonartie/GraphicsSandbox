#pragma once
#include "Component.h"
#include <QBuffer>
#include <QColor>
#include <QImage>
#include <QMap>
#include <QOpenGLTexture>
#include <QQuaternion>
#include <QVariant>
#include <QVector2D>
#include <QVector3D>
#include <map>

struct MaterialComponent : Component {
   static inline auto Name = u"Material"_s;

   using Component::Component;

   /// Properties can either be float, int, bool, QImage, QColor, QVector2D, QVector3D, QSizeF
   struct Property {
      QString type;
      QVariant value;

      QJsonObject toJson() const {
         QJsonObject json;
         //         json["type"] = type;
         //         json["value"] = QJsonValue::fromVariant(value);
         json["type"] = type;
         if (type == "bool") json["value"] = value.toBool();
         else if (type == "int")
            json["value"] = value.toInt();
         else if (type == "float")
            json["value"] = value.toFloat();
         else if (type == "QImage")
            json["value"] = ImageToBase64(value.value<QImage>());
         else if (type == "QColor")
            json["value"] = value.value<QColor>().name();
         else if (type == "QVector2D") {
            auto vec = value.value<QVector2D>();
            json["value"] = QJsonArray{vec.x(), vec.y()};
         } else if (type == "QVector3D") {
            auto vec = value.value<QVector3D>();
            json["value"] = QJsonArray{vec.x(), vec.y(), vec.z()};
         } else if (type == "QSizeF") {
            auto vec = value.value<QSizeF>();
            json["value"] = QJsonArray{vec.width(), vec.height()};
         }
         return json;
      }

      static Property fromJson(const QJsonObject& json) {
         Property prop;
//         prop.type = json["type"].toString();
//         prop.value = json["value"].toVariant();
         prop.type = json["type"].toString();
         if (prop.type == "bool") prop.value = json["value"].toBool();
         else if (prop.type == "int")
            prop.value = json["value"].toInt();
         else if (prop.type == "float")
            prop.value = json["value"].toDouble();
         else if (prop.type == "QImage")
            prop.value = Base64ToImage(json["value"].toString());
         else if (prop.type == "QColor")
            prop.value = QColor(json["value"].toString());
         else if (prop.type == "QVector2D") {
            auto arr = json["value"].toArray();
            prop.value = QVector2D(arr[0].toDouble(), arr[1].toDouble());
         } else if (prop.type == "QVector3D") {
            auto arr = json["value"].toArray();
            prop.value = QVector3D(arr[0].toDouble(), arr[1].toDouble(), arr[2].toDouble());
         } else if (prop.type == "QSizeF") {
            auto arr = json["value"].toArray();
            prop.value = QSizeF(arr[0].toDouble(), arr[1].toDouble());
         }
         return prop;
      }

      static QString ImageToBase64(const QImage& image) {
         QByteArray byteArray;
         QBuffer buffer(&byteArray);
         buffer.open(QIODevice::WriteOnly);
         image.save(&buffer, "PNG");
         return QString::fromUtf8(byteArray.toBase64());
      }

      static QImage Base64ToImage(const QString& base64) {
         QImage image;
         image.loadFromData(QByteArray::fromBase64(base64.toUtf8()), "PNG");
         return image;
      }
   };

   QString shader;
   std::map<QString, Property> properties;

   QJsonObject toJson() const override {
      QJsonObject json;
      json["shader"] = QJsonValue::fromVariant(shader);
      for (auto& [name, prop]: properties) {
         json[name] = prop.toJson();
      }
      return json;
   }

   void fromJson(const QJsonObject& json) override {
      shader = json["shader"].toString();
      for (auto it = json.begin(); it != json.end(); ++it) {
         if (it.key() != "shader") {
            properties[it.key()] = Property::fromJson(it.value().toObject());
         }
      }
   }

   void prepare(QOpenGLShaderProgram* program) override {
      if (m_textures.empty() || isDirty()) {
         for (auto& [name, tex]: m_textures) {
            delete tex;
         }
         m_textures.clear();

         for (auto& [name, prop]: properties) {
            if (prop.type == "QImage") {
               auto image = prop.value.value<QImage>();
               if (image.isNull()) continue;
               auto texture = new QOpenGLTexture(image.mirrored());
               texture->setMinificationFilter(QOpenGLTexture::Linear);
               texture->setMagnificationFilter(QOpenGLTexture::Linear);
               texture->setWrapMode(QOpenGLTexture::DirectionS, QOpenGLTexture::ClampToEdge);
               texture->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::ClampToEdge);
               m_textures.emplace(name, texture);
            }
         }
         clean();
      }
   }

   void bind(QOpenGLShaderProgram* program) override {
      for (auto& [name, prop]: properties) {
         if (prop.type == "bool") program->setUniformValue(name.toStdString().c_str(), prop.value.toBool());
         else if (prop.type == "int")
            program->setUniformValue(name.toStdString().c_str(), prop.value.toInt());
         else if (prop.type == "float")
            program->setUniformValue(name.toStdString().c_str(), prop.value.toFloat());
         else if (prop.type == "QImage") {
            // handled at the end
         } else if (prop.type == "QColor")
            program->setUniformValue(name.toStdString().c_str(), prop.value.value<QColor>());
         else if (prop.type == "QVector2D") {
            auto vec = prop.value.value<QVector2D>();
            program->setUniformValue(name.toStdString().c_str(), vec);
         } else if (prop.type == "QVector3D") {
            auto vec = prop.value.value<QVector3D>();
            program->setUniformValue(name.toStdString().c_str(), vec);
         } else if (prop.type == "QSizeF") {
            auto vec = prop.value.value<QSizeF>();
            program->setUniformValue(name.toStdString().c_str(), vec);
         }
      }

      int texID = 0;
      for (auto& [name, texture]: m_textures) {
         texture->bind(texID);
         program->setUniformValue(name.toStdString().c_str(), texID);
         texID++;
      }
   }

   void release(QOpenGLShaderProgram* program) override {
      for (auto& [_, texture]: m_textures) {
         texture->release();
      }

      for (auto& [name, _]: properties) {
         program->setUniformValue(name.toStdString().c_str(), 0);
      }
   }

private:
   std::unordered_map<QString, QOpenGLTexture*, QtHasher<QString>> m_textures;
};