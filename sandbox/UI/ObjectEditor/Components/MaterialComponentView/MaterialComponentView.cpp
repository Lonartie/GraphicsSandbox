#include "MaterialComponentView.h"
#include "Common/ShaderProvider.h"
#include "ui_MaterialComponentView.h"
#include <QCheckBox>
#include <QColorDialog>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QPushButton>

MaterialComponentView::MaterialComponentView(QWidget* parent)
    : QWidget(parent), m_ui(new Ui::MaterialComponentView) {
   REG_ASSERT(Registered);

   m_ui->setupUi(this);

   connect(&ShaderProvider::instance(), &ShaderProvider::shadersChanged, [this](auto shaders) {
      QSignalBlocker block(m_ui->shader);
      auto selected = m_ui->shader->currentText();
      m_ui->shader->clear();
      for (auto& shader: shaders) {
         m_ui->shader->addItem(shader);
      }
      m_ui->shader->setCurrentText(selected);
   });

   for (auto& shader: ShaderProvider::instance().getShaderNames()) {
      m_ui->shader->addItem(shader);
   }

   connect(&ShaderProvider::instance(), &ShaderProvider::shadersChanged, this, &MaterialComponentView::init);
   connect(m_ui->shader, QOverload<int>::of(&QComboBox::currentIndexChanged), [=, this](int index) {
      auto& mat = m_obj->getComponent<MaterialComponent>();
      auto shader = m_ui->shader->itemText(index);
      mat.shader = shader;
      selectShader(shader, mat);
   });
}

MaterialComponentView::~MaterialComponentView() {
   delete m_ui;
}

QWidget* MaterialComponentView::asWidget() {
   return this;
}

void MaterialComponentView::init() {
   auto& mat = m_obj->getComponent<MaterialComponent>();
   if (mat.shader.isEmpty()) mat.shader = "Default";
   selectShader(mat.shader, mat);
}

void MaterialComponentView::updateValues(const QString& name, const QString& type, const QVariant& value) {
   auto& mat = m_obj->getComponent<MaterialComponent>();
   mat.properties[name] = MaterialComponent::Property(type, value);
   mat.dirty();
   emit objectChanged();
}

void MaterialComponentView::selectShader(QString name, MaterialComponent& mat) {
   while (!m_ui->settings->isEmpty()) {
      auto widget = m_ui->settings->takeAt(0)->widget();
      m_ui->settings->removeWidget(widget);
      delete widget;
   }
   m_widgets.clear();
   QSignalBlocker blocker(m_ui->shader);
   m_ui->shader->setCurrentText(name);

   if (name == "Default") {
      createColorField("solidColor", mat);
   } else if (name == "Material") {
      createImageField("albedo", mat);
      createImageField("normal", mat);
   }

   mat.dirty();
}

void MaterialComponentView::createColorField(const QString& name, MaterialComponent& mat) {
   // set default value if it has none
   if (!mat.properties.contains(name)) {
      mat.properties[name].type = "QColor";
      mat.properties[name].value = QColor(Qt::black);
   }

   // build up ui for a color field
   auto* layout = new QHBoxLayout();
   auto* label = new QLabel(name);
   auto* preview = new QLabel();
   preview->setStyleSheet(QString("background-color: %1").arg(mat.properties.at(name).value.value<QColor>().name()));
   auto* button = new QPushButton("select");
   connect(button, &QPushButton::clicked, [this, preview, name, obj = &mat.parent()] {
      auto& mat = obj->getComponent<MaterialComponent>();
      auto before = mat.properties.at(name).value.value<QColor>();
      QColorDialog diag(before, this);
      connect(&diag, &QColorDialog::currentColorChanged, [this, preview, name](const QColor& color) {
         preview->setStyleSheet(QString("background-color: %1").arg(color.name()));
         updateValues(name, "QColor", color);
      });
      if (diag.exec() == QDialog::Rejected) {
         preview->setStyleSheet(QString("background-color: %1").arg(before.name()));
      }
   });

   // compose widget and add it to ui and m_widgets
   layout->addWidget(label);
   layout->addWidget(preview);
   layout->addWidget(button);
   auto containerWidget = new QWidget();
   containerWidget->setLayout(layout);
   m_ui->settings->addWidget(containerWidget);
   m_widgets.push_back(containerWidget);
}

void MaterialComponentView::createFloatField(const QString& name, MaterialComponent& mat) {
   // set default value if it has none
   if (!mat.properties.contains(name)) {
      mat.properties[name].type = "float";
      mat.properties[name].value = 0.0f;
   }

   // build up ui for a float field
   auto* layout = new QHBoxLayout();
   auto* label = new QLabel(name);
   auto* field = new QDoubleSpinBox();
   field->setRange(-1e6, 1e6);
   field->setSingleStep(0.1);
   field->setValue(mat.properties.at(name).value.toDouble());
   connect(field, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=, this](double value) {
      updateValues(name, "float", value);
   });

   // compose widget and add it to ui and m_widgets
   layout->addWidget(label);
   layout->addWidget(field);
   auto containerWidget = new QWidget();
   containerWidget->setLayout(layout);
   m_ui->settings->addWidget(containerWidget);
   m_widgets.push_back(containerWidget);
}

void MaterialComponentView::createIntField(const QString& name, MaterialComponent& mat) {
   // set default value if it has none
   if (!mat.properties.contains(name)) {
      mat.properties[name].type = "int";
      mat.properties[name].value = 0;
   }

   // build up ui for an int field
   auto* layout = new QHBoxLayout();
   auto* label = new QLabel(name);
   auto* field = new QSpinBox();
   field->setRange(-1e6, 1e6);
   field->setValue(mat.properties.at(name).value.toInt());
   connect(field, QOverload<int>::of(&QSpinBox::valueChanged), [=, this](int value) {
      updateValues(name, "int", value);
   });

   // compose widget and add it to ui and m_widgets
   layout->addWidget(label);
   layout->addWidget(field);
   auto containerWidget = new QWidget();
   containerWidget->setLayout(layout);
   m_ui->settings->addWidget(containerWidget);
   m_widgets.push_back(containerWidget);
}

void MaterialComponentView::createBoolField(const QString& name, MaterialComponent& mat) {
   // set default value if it has none
   if (!mat.properties.contains(name)) {
      mat.properties[name].type = "bool";
      mat.properties[name].value = false;
   }

   // build up ui for a bool field
   auto* layout = new QHBoxLayout();
   auto* label = new QLabel(name);
   auto* field = new QCheckBox();
   field->setChecked(mat.properties.at(name).value.toBool());
   connect(field, &QCheckBox::toggled, [=, this](bool value) {
      updateValues(name, "bool", value);
   });

   // compose widget and add it to ui and m_widgets
   layout->addWidget(label);
   layout->addWidget(field);
   auto containerWidget = new QWidget();
   containerWidget->setLayout(layout);
   m_ui->settings->addWidget(containerWidget);
   m_widgets.push_back(containerWidget);
}

void MaterialComponentView::createImageField(const QString& name, MaterialComponent& mat) {
   // set default value if it has none
   if (!mat.properties.contains(name)) {
      mat.properties[name].type = "QImage";
      mat.properties[name].value = QImage();
   }

   // build up ui for an image field
   auto* layout = new QHBoxLayout();
   auto* label = new QLabel(name);
   auto* preview = new QLabel();
   auto* button = new QPushButton("select");
   auto* button2 = new QPushButton("clear");
   auto image = mat.properties.at(name).value.value<QImage>();
   if (!image.isNull()) {
      preview->setPixmap(QPixmap::fromImage(image).scaled(64, 64, Qt::KeepAspectRatio));
   }
   connect(button, &QPushButton::clicked, [=, this] {
      auto path = QFileDialog::getOpenFileName(this, "Select Image", "", "Images (*.png *.jpg *.bmp)");
      if (path.isEmpty()) return;
      QImage image(path);
      if (image.isNull()) return;
      preview->setPixmap(QPixmap::fromImage(image).scaled(64, 64, Qt::KeepAspectRatio));
      updateValues(name, "QImage", image);
   });
   connect(button2, &QPushButton::clicked, [=, this] {
      preview->clear();
      updateValues(name, "QImage", QImage());
   });

   // compose widget and add it to ui and m_widgets
   layout->addWidget(label);
   layout->addWidget(preview);
   layout->addWidget(button);
   layout->addWidget(button2);
   auto containerWidget = new QWidget();
   containerWidget->setLayout(layout);
   m_ui->settings->addWidget(containerWidget);
   m_widgets.push_back(containerWidget);
}

void MaterialComponentView::createVector2DField(const QString& name, MaterialComponent& mat) {
   // set default value if it has none
   if (!mat.properties.contains(name)) {
      mat.properties[name].type = "QVector2D";
      mat.properties[name].value = QVector2D();
   }

   // build up ui for a vector2d field
   auto* layout = new QHBoxLayout();
   auto* label = new QLabel(name);
   auto* fieldX = new QDoubleSpinBox();
   fieldX->setRange(-1e6, 1e6);
   fieldX->setSingleStep(0.1);
   fieldX->setValue(mat.properties.at(name).value.value<QVector2D>().x());
   connect(fieldX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=, this](double value) {
      auto vec = mat.properties.at(name).value.value<QVector2D>();
      vec.setX(value);
      updateValues(name, "QVector2D", vec);
   });
   auto* fieldY = new QDoubleSpinBox();
   fieldY->setRange(-1e6, 1e6);
   fieldY->setSingleStep(0.1);
   fieldY->setValue(mat.properties.at(name).value.value<QVector2D>().y());
   connect(fieldY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=, this](double value) {
      auto vec = mat.properties.at(name).value.value<QVector2D>();
      vec.setY(value);
      updateValues(name, "QVector2D", vec);
   });

   // compose widget and add it to ui and m_widgets
   layout->addWidget(label);
   layout->addWidget(fieldX);
   layout->addWidget(fieldY);
   auto containerWidget = new QWidget();
   containerWidget->setLayout(layout);
   m_ui->settings->addWidget(containerWidget);
   m_widgets.push_back(containerWidget);
}

void MaterialComponentView::createVector3DField(const QString& name, MaterialComponent& mat) {
   // set default value if it has none
   if (!mat.properties.contains(name)) {
      mat.properties[name].type = "QVector3D";
      mat.properties[name].value = QVector3D();
   }

   // build up ui for a vector3d field
   auto* layout = new QHBoxLayout();
   auto* label = new QLabel(name);
   auto* fieldX = new QDoubleSpinBox();
   fieldX->setRange(-1e6, 1e6);
   fieldX->setSingleStep(0.1);
   fieldX->setValue(mat.properties.at(name).value.value<QVector3D>().x());
   connect(fieldX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=, this](double value) {
      auto vec = mat.properties.at(name).value.value<QVector3D>();
      vec.setX(value);
      updateValues(name, "QVector3D", vec);
   });
   auto* fieldY = new QDoubleSpinBox();
   fieldY->setRange(-1e6, 1e6);
   fieldY->setSingleStep(0.1);
   fieldY->setValue(mat.properties.at(name).value.value<QVector3D>().y());
   connect(fieldY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=, this](double value) {
      auto vec = mat.properties.at(name).value.value<QVector3D>();
      vec.setY(value);
      updateValues(name, "QVector3D", vec);
   });
   auto* fieldZ = new QDoubleSpinBox();
   fieldZ->setRange(-1e6, 1e6);
   fieldZ->setSingleStep(0.1);
   fieldZ->setValue(mat.properties.at(name).value.value<QVector3D>().z());
   connect(fieldZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=, this](double value) {
      auto vec = mat.properties.at(name).value.value<QVector3D>();
      vec.setZ(value);
      updateValues(name, "QVector3D", vec);
   });

   // compose widget and add it to ui and m_widgets
   layout->addWidget(label);
   layout->addWidget(fieldX);
   layout->addWidget(fieldY);
   layout->addWidget(fieldZ);
   auto containerWidget = new QWidget();
   containerWidget->setLayout(layout);
   m_ui->settings->addWidget(containerWidget);
   m_widgets.push_back(containerWidget);
}

void MaterialComponentView::createSizeField(const QString& name, MaterialComponent& mat) {
   // set default value if it has none
   if (!mat.properties.contains(name)) {
      mat.properties[name].type = "QSizeF";
      mat.properties[name].value = QSizeF();
   }

   // build up ui for a size field
   auto* layout = new QHBoxLayout();
   auto* label = new QLabel(name);
   auto* fieldW = new QDoubleSpinBox();
   fieldW->setRange(-1e6, 1e6);
   fieldW->setSingleStep(0.1);
   fieldW->setValue(mat.properties.at(name).value.value<QSizeF>().width());
   connect(fieldW, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=, this](double value) {
      auto vec = mat.properties.at(name).value.value<QSizeF>();
      vec.setWidth(value);
      updateValues(name, "QSizeF", vec);
   });
   auto* fieldH = new QDoubleSpinBox();
   fieldH->setRange(-1e6, 1e6);
   fieldH->setSingleStep(0.1);
   fieldH->setValue(mat.properties.at(name).value.value<QSizeF>().height());
   connect(fieldH, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [=, this](double value) {
      auto vec = mat.properties.at(name).value.value<QSizeF>();
      vec.setHeight(value);
      updateValues(name, "QSizeF", vec);
   });

   // compose widget and add it to ui and m_widgets
   layout->addWidget(label);
   layout->addWidget(fieldW);
   layout->addWidget(fieldH);
   auto containerWidget = new QWidget();
   containerWidget->setLayout(layout);
   m_ui->settings->addWidget(containerWidget);
   m_widgets.push_back(containerWidget);
}
