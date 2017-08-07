/**
 * \file advanced_color_selector.cpp
 * \brief Advanced combined color selector widget
 *
 * \author caryoscelus
 *
 * \copyright Copyright (C) 2017 caryoscelus
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include <QLayout>
#include <QToolButton>
#include <QTabWidget>
#include <QEvent>
#include <QButtonGroup>
#include <QDebug>

#include "color_wheel.hpp"
#include "color_2d_slider.hpp"
#include "hue_slider.hpp"
#include "advanced_color_selector.hpp"

namespace color_widgets {

class AdvancedColorSelector::Private : public QObject
{
public:
    Private(AdvancedColorSelector* parent) :
        wheel(new ColorWheel()),
        rectangle(new Color2DSlider()),
        hue_slider(new HueSlider(Qt::Vertical)),
        harmony_buttons(new QButtonGroup()),
        parent(parent)
    {
        addColorWidget(wheel);
        addColorWidget(rectangle);
        addColorWidget(hue_slider);

        auto harmony_none = newToolButton(
            "media-playback-start",
            [this]() {
                wheel->clearHarmonies();
            }
        );
        harmony_none->setChecked(true);
        harmony_buttons->addButton(
            harmony_none
        );
        harmony_buttons->addButton(
            newToolButton(
                "media-playback-start",
                [this]() {
                    wheel->clearHarmonies();
                    wheel->addHarmony(0.5, false);
                }
            )
        );
        harmony_buttons->addButton(
            newToolButton(
                "media-playback-start",
                [this]() {
                    wheel->clearHarmonies();
                    auto a = wheel->addHarmony(0.125, true);
                    wheel->addSymmetricHarmony(a);
                }
            )
        );
    }
    ~Private() = default;
public:
//     bool eventFilter(QObject* object, QEvent* event) override {
//         if (event->type() == QEvent::Resize)
//         {
//             qDebug() << "resized";
//         }
//         return false;
//     }
public:
    /**
     * Adds color widget with proper signals & slots
     *
     * TODO: more info
     */
    void addColorWidget(QObject* widget) {
        widgets.push_back(widget);
        connect(widget, SIGNAL(colorChanged(QColor)), parent, SIGNAL(colorChanged(QColor)));
    }
    void removeColorWidget(QObject* widget) {
        widgets.removeAll(widget);
        disconnect(widget, SIGNAL(colorChanged(QColor)), parent, SIGNAL(colorChanged(QColor)));
    }
    template <typename F>
    QToolButton* newToolButton(QString icon, F callback) const {
        auto button = new QToolButton();
        button->setCheckable(true);
        button->resize(32, 32);
        button->setIcon(QIcon::fromTheme(icon));
        connect(button, &QToolButton::toggled, callback);
        return button;
    }
    void setColor(QColor c) {
        for (auto widget : widgets) {
            auto oldState = widget->blockSignals(true);
            QMetaObject::invokeMethod(widget, "setColor", Q_ARG(QColor, c));
            widget->blockSignals(oldState);
        }
    }
public:
    ColorWheel* wheel;
    Color2DSlider* rectangle;
    HueSlider* hue_slider;
    QButtonGroup* harmony_buttons;
private:
    AdvancedColorSelector * const parent;
    QVector<QObject*> widgets;
};

AdvancedColorSelector::AdvancedColorSelector(QWidget* parent) :
    QWidget(parent),
    p(new Private(this))
{
    auto main_layout = new QVBoxLayout();

    auto tabs_widget = new QTabWidget();
    main_layout->addWidget(tabs_widget);

    auto wheel_layout = new QVBoxLayout();
    auto wheel_widget = new QWidget();
    wheel_widget->setLayout(wheel_layout);
//     wheel_widget->installEventFilter(p.data());

    auto form_button = new QToolButton(/*wheel_widget*/);
    form_button->setCheckable(true);
    form_button->resize(32, 32);
    connect(form_button, &QToolButton::toggled, [this, form_button](bool square) {
        if (square)
        {
            form_button->setIcon(QIcon::fromTheme("draw-triangle3"));
            p->wheel->setDisplayFlags(ColorWheel::SHAPE_SQUARE | ColorWheel::ANGLE_FIXED);
        }
        else
        {
            form_button->setIcon(QIcon::fromTheme("draw-rectangle"));
            p->wheel->setDisplayFlags(ColorWheel::SHAPE_TRIANGLE | ColorWheel::ANGLE_ROTATING);
        }
    });
    form_button->setChecked(true);

    auto harmony_none = new QToolButton(/*wheel_widget*/);
    harmony_none->setCheckable(true);
    harmony_none->resize(32, 32);

    auto harmony_layout = new QHBoxLayout();
    harmony_layout->addWidget(form_button);

    for (auto button : p->harmony_buttons->buttons())
        harmony_layout->addWidget(button);

    auto harmony_widget = new QWidget();
    harmony_widget->setLayout(harmony_layout);

    wheel_layout->addWidget(harmony_widget);
    wheel_layout->addWidget(p->wheel, 1.0);

    tabs_widget->addTab(wheel_widget, tr("Wheel"));

    auto rectangle_layout = new QHBoxLayout();
    rectangle_layout->addWidget(p->rectangle);
    rectangle_layout->addWidget(p->hue_slider);
    auto rectangle_widget = new QWidget();
    rectangle_widget->setLayout(rectangle_layout);
    tabs_widget->addTab(rectangle_widget, tr("Rectangle"));

    setLayout(main_layout);

    connect(this, SIGNAL(colorChanged(QColor)), this, SLOT(setColor(QColor)));
}

AdvancedColorSelector::~AdvancedColorSelector()
{
}

void AdvancedColorSelector::setColor(QColor c)
{
    p->setColor(c);
}

} // namespace color_widgets
