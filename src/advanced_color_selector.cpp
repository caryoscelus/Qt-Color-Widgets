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

#include <memory>
#include <cmath>

#include "color_wheel.hpp"
#include "color_2d_slider.hpp"
#include "hue_slider.hpp"
#include "color_line_edit.hpp"
#include "swatch.hpp"
#include "advanced_color_selector.hpp"

namespace color_widgets {

class HarmonyButton : public QWidget {
public:
    HarmonyButton(AdvancedColorSelector* parent, unsigned n) :
        parent(parent),
        n(n),
        widget(new ColorLineEdit(this))
    {
        widget->setPreviewColor(true);
        auto layout = new QHBoxLayout();
        layout->addWidget(widget);
        layout->setContentsMargins(0, 0, 0, 0);
        setLayout(layout);
        widget->installEventFilter(this);
    }
    inline void setColor(const QColor& color) {
        widget->setColor(color);
    }
    inline void setReadOnly(bool ro) {
        widget->setReadOnly(ro);
    }
    void setSelected(bool active) {
        setStyleSheet(active ? "font-weight: bold" : "");
    }
    bool eventFilter(QObject* object, QEvent* event) override {
        if (dynamic_cast<ColorLineEdit*>(object))
        {
            if (event->type() == QEvent::MouseButtonPress)
            {
                parent->setHarmony(n);
            }
        }
        return false;
    }
private:
    AdvancedColorSelector* parent;
    unsigned n;
    ColorLineEdit* widget;
};

class AdvancedColorSelector::Private : public QObject
{
public:
    Private(AdvancedColorSelector* parent) :
        wheel(new ColorWheel()),
        rectangle(new Color2DSlider()),
        hue_slider(new HueSlider(Qt::Vertical)),
        color_history(new Swatch()),
        harmony_buttons(new QButtonGroup()),
        wheel_layout(new QVBoxLayout()),
        parent(parent)
    {
        addColorWidget(wheel);
        addColorWidget(rectangle);
        addColorWidget(hue_slider);

        auto harmony_none = newToolButton(
            "media-playback-start",
            [this]() {
                wheel->clearHarmonies();
                updateColors();
            }
        );
        harmony_buttons->addButton(
            harmony_none
        );
        harmony_buttons->addButton(
            newToolButton(
                "media-playback-start",
                [this]() {
                    wheel->clearHarmonies();
                    wheel->addHarmony(0.5, false);
                    updateColors();
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
                    updateColors();
                }
            )
        );
        auto main_layout = new QVBoxLayout();

        auto tabs_widget = new QTabWidget();
        main_layout->addWidget(tabs_widget);

        auto wheel_widget = new QWidget();
        wheel_widget->setLayout(wheel_layout);
//         wheel_widget->installEventFilter(p.data());

        auto form_button = new QToolButton(/*wheel_widget*/);
        form_button->setCheckable(true);
        form_button->resize(32, 32);
        connect(form_button, &QToolButton::toggled, [this, form_button](bool square) {
            if (square)
            {
                form_button->setIcon(QIcon::fromTheme("draw-triangle3"));
                wheel->setDisplayFlags(ColorWheel::SHAPE_SQUARE | ColorWheel::ANGLE_FIXED);
            }
            else
            {
                form_button->setIcon(QIcon::fromTheme("draw-rectangle"));
                wheel->setDisplayFlags(ColorWheel::SHAPE_TRIANGLE | ColorWheel::ANGLE_ROTATING);
            }
        });
        form_button->setChecked(true);

        auto harmony_layout = new QHBoxLayout();
        harmony_layout->addWidget(form_button);

        for (auto button : harmony_buttons->buttons())
            harmony_layout->addWidget(button);

        auto harmony_widget = new QWidget();
        harmony_widget->setLayout(harmony_layout);

        wheel_layout->addWidget(harmony_widget);
        wheel_layout->addWidget(wheel, 1.0);

        tabs_widget->addTab(wheel_widget, tr("Wheel"));

        auto rectangle_layout = new QHBoxLayout();
        rectangle_layout->addWidget(rectangle);
        rectangle_layout->addWidget(hue_slider);
        auto rectangle_widget = new QWidget();
        rectangle_widget->setLayout(rectangle_layout);
        tabs_widget->addTab(rectangle_widget, tr("Rectangle"));

        main_layout->addWidget(color_history);
        main_layout->setStretchFactor(tabs_widget, 1);
        color_history->setForcedColumns(12);
        color_history->setColorSizePolicy(Swatch::Minimum);
        color_history->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

        parent->setLayout(main_layout);

        connect(color_history, &Swatch::colorSelected, this, &Private::setColor);
        connect(wheel, &ColorWheel::harmonyChanged, this, &Private::updateColors);
        harmony_none->setChecked(true);
        setHarmony(0);
    }
    ~Private() = default;
public:
    /**
     * Adds color widget with proper signals & slots
     *
     * TODO: more info
     */
    void addColorWidget(QObject* widget) {
        widgets.push_back(widget);
        connect(widget, SIGNAL(colorChanged(QColor)), parent, SLOT(setBaseColor(QColor)));
    }
    void removeColorWidget(QObject* widget) {
        widgets.removeAll(widget);
        disconnect(widget, SIGNAL(colorChanged(QColor)), parent, SLOT(setBaseColor(QColor)));
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
        auto baseHue = c.hueF() - color().hueF() + baseColor().hueF();
        baseHue -= std::floor(baseHue);
        setBaseColor(QColor::fromHsvF(baseHue, c.saturationF(), c.valueF()));
    }
    void setBaseColor(QColor c) {
        for (auto widget : widgets) {
            auto oldState = widget->blockSignals(true);
            QMetaObject::invokeMethod(widget, "setColor", Q_ARG(QColor, c));
            widget->blockSignals(oldState);
        }
    }
    void updateColors() {
        auto count = wheel->harmonyCount();
        auto colors = wheel->harmonyColors();
        if (harmony_colors_layout == nullptr || (unsigned) harmony_colors_layout->count() != count) {
            harmony_colors_widget.reset(new QWidget());
            harmony_colors_layout = new QHBoxLayout();
            harmony_colors_widget->setLayout(harmony_colors_layout);
            harmony_colors_widget->setMaximumHeight(32);
            wheel_layout->addWidget(harmony_colors_widget.get());
            harmony_colors_widgets.clear();
            for (unsigned i = 0; i < count; ++i)
            {
                auto button = new HarmonyButton(parent, i);
                harmony_colors_layout->addWidget(button);
                harmony_colors_widgets.append(button);
            }
        }
        unsigned i = 0;
        for (auto widget : harmony_colors_widgets)
        {
            widget->setColor(colors[i]);
            widget->setReadOnly(true);
            widget->setSelected((int)i == selected_harmony);
            ++i;
        }
        Q_EMIT parent->colorChanged(color());
    }
    void setHarmony(int i) {
        if (i < 0 || i >= (int)wheel->harmonyCount())
            i = 0;
        selected_harmony = i;
        int j = 0;
        for (auto widget : harmony_colors_widgets)
        {
            widget->setSelected(j == selected_harmony);
            ++j;
        }
        Q_EMIT parent->colorChanged(color());
    }
    QColor color() const {
        auto i = selected_harmony;
        if (i < 0 || i >= (int)wheel->harmonyCount())
            i = 0;
        return wheel->harmonyColors()[i];
    }
    QColor baseColor() const {
        return wheel->color();
    }
public:
    ColorWheel* wheel;
    Color2DSlider* rectangle;
    HueSlider* hue_slider;
    Swatch* color_history;
    QButtonGroup* harmony_buttons;
    QVBoxLayout* wheel_layout;
private:
    AdvancedColorSelector * const parent;
    QVector<QObject*> widgets;
    std::unique_ptr<QWidget> harmony_colors_widget = nullptr;
    QHBoxLayout* harmony_colors_layout = nullptr;
    QVector<HarmonyButton*> harmony_colors_widgets;
    int selected_harmony = 0;
};

AdvancedColorSelector::AdvancedColorSelector(QWidget* parent) :
    QWidget(parent),
    p(new Private(this))
{
}

AdvancedColorSelector::~AdvancedColorSelector()
{
}

QColor AdvancedColorSelector::color() const
{
    return p->color();
}

void AdvancedColorSelector::setColor(QColor c)
{
    // NOTE: QColors with different models compare unequal!
    if (c.toRgb() == p->color().toRgb())
        return;
    p->setColor(c);
    Q_EMIT colorChanged(color());
}

void AdvancedColorSelector::setBaseColor(QColor c)
{
    p->setBaseColor(c);
    Q_EMIT colorChanged(color());
}

void AdvancedColorSelector::setHarmony(unsigned harmony)
{
    p->setHarmony(harmony);
}

void AdvancedColorSelector::saveToHistory() {
    p->color_history->palette().insertColor(0, color());
    p->color_history->updateGeometry();
}

} // namespace color_widgets
