/**
 * \file advanced_color_selector.hpp
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
#ifndef ADVANCED_COLOR_SELECTOR_HPP
#define ADVANCED_COLOR_SELECTOR_HPP

#include "colorwidgets_global.hpp"

#include <QWidget>

namespace color_widgets {

class QCP_EXPORT AdvancedColorSelector : public QWidget
{
    Q_OBJECT

    // TODO: define Q_PROPERTYs

public:
    explicit AdvancedColorSelector(QWidget* parent = nullptr);
    ~AdvancedColorSelector();

public:
    QColor color() const;

public Q_SLOTS:
    void setColor(QColor c);
    void setBaseColor(QColor c);

Q_SIGNALS:
    void colorChanged(QColor);

private:
    class Private;
    QScopedPointer<Private> const p;
};

} // namespace color_widgets

#endif
